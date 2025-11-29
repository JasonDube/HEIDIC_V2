# EDEN ENGINE (Heidic V2) Development Log

**Date:** November 29, 2025
**AI Assistant:** Gemini 3-Pro
n
## Project Goal
Rebuild the HEIDIC language compiler ("Heidic V2") from scratch, focusing on a clean architecture and integrating a custom game engine ("EDEN ENGINE") with Vulkan, GLFW, GLM, and ImGui support.

## Development Journey

### 1. Compiler Setup (Nov 29, 2025)
- Reused core Lexer, Parser, AST, and Type Checker from original HEIDIC.
- Cleaned up the codebase to remove legacy dependencies.
- Configured Codegen to emit C++ code compatible with modern C++17.

### 2. Graphics Integration (Nov 29, 2025)
- Created a `stdlib/` directory for standard library headers.
- Implemented `eden_vulkan_helpers` to abstract verbose Vulkan initialization.
- Integrated `GLFW` for window management.

### 3. The "Spinning Triangle" Challenge (Nov 29, 2025)
We encountered several significant hurdles getting the basic 3D example to run:

#### Phase 1: Build System & Linking
- **Challenge**: The C++ compiler (`g++`) failed to link GLFW and Vulkan correctly on Windows.
- **Solution**: Identified correct library paths (`C:\glfw-3.4\build\src` for MinGW `libglfw3.a`) and Vulkan SDK paths. Created a robust build command.

#### Phase 2: The "Black Screen" of Doom
After successful compilation, the window appeared but remained black. We diagnosed multiple rendering pipeline issues:

1.  **Coordinate System Mismatch**:
    *   *Issue*: GLM defaults to OpenGL coordinates (Y-up, Z range -1 to 1). Vulkan uses Y-down and Z range 0 to 1. This caused the geometry to be projected upside-down and, more critically, **clipped** because the negative Z values from standard GLM projection were outside Vulkan's [0, 1] depth range.
    *   *Fix*:
        1.  Inverted the Y-axis in the projection matrix: `proj[1][1] *= -1`.
        2.  Updated `stdlib/math.h` to use `glm::perspectiveRH_ZO` (Zero-to-One) instead of the default `glm::perspective`.

2.  **Shader Pipeline Mismatch**:
    *   *Issue*: The initial fragment shader expected a texture sampler (Binding 1), but our simple renderer only set up a Uniform Buffer (Binding 0). This caused undefined behavior/rendering failure.
    *   *Fix*: Created a simplified `frag_3d.glsl` that relies solely on interpolated vertex colors, removing the texture dependency.

3.  **Data Alignment (The Final Boss)**:
    *   *Issue*: Our C++ wrapper struct `Mat4` contained both the `glm::mat4` data AND a cached float array `m[16]`, making it 128 bytes. The shader expected a standard 64-byte `mat4`. Sending this struct to the GPU meant the shader read garbage values for View and Projection matrices, putting the triangle at infinity.
    *   *Fix*: Redefined the `UniformBufferObject` struct in C++ to use `glm::mat4` directly, ensuring the data layout matched the shader exactly (192 bytes total).

### 4. Top-Down Example & ImGui Integration (Nov 29, 2025)

#### ImGui Backend & Header Conflict
- **Challenge**: ImGui compilation failed with obscure "undeclared identifier" errors (`ImGuiIO`, `ImGui`).
- **Diagnosis**: Circular dependency in include paths. Our `stdlib/imgui.h` wrapper (intended for Heidic generated code) shadowed the *actual* `imgui.h` library file because `-Istdlib` was included before `-Ithird_party/imgui`. The ImGui backend files thus included the wrapper, which included the backend header, creating a cycle where `ImGui` namespace was never defined.
- **Fix**: Renamed `stdlib/imgui.h` to `stdlib/eden_imgui.h` to resolve the name collision.

#### C++ vs Extern "C" String ABI
- **Challenge**: The program crashed immediately with an ImGui assertion `name != NULL`.
- **Diagnosis**: The Heidic compiler generated C++ code that passed `std::string` by value to functions declared as `extern "C"`. The C++ implementation of these helper functions accepted `const char*`. This is an ABI mismatch: the caller pushed a `std::string` object (struct) onto the stack, but the callee read it as a `const char*` pointer, interpreting internal struct data as a memory address (garbage).
- **Fix**: Updated `codegen.rs` to detect `ExternFunction` declarations. For these functions, `Type::String` is now generated as `const char*` in the C++ declaration instead of `std::string`. Since the call sites in `top_down.hd` used string literals (e.g., `"Debug Panel"`), they compiled correctly as `const char*` and the ABI mismatch was resolved.

### 5. Line Drawing & Input Handling (Nov 29, 2025)
- **Immediate Mode Lines**: Implemented `heidic_draw_line` and `heidic_draw_model_origin` in the Vulkan backend using a dynamic vertex buffer and `VK_PRIMITIVE_TOPOLOGY_LINE_LIST`.
- **F1 Toggle**: Added input handling logic in `top_down.hd` to toggle the ImGui debug panel visibility using the F1 key.
- **Model Origin**: The `top_down` example now draws Red (X), Green (Y), and Yellow (Z) axes extending from the cube's origin, transforming with the object.

### 6. WASD Player Movement System (Dec 2024)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** December 2024

Implemented a complete player movement system with rotation-relative controls:

#### Challenges & Solutions

**Challenge 1: Missing Trigonometric Functions**
- **Problem**: Heidic language had no built-in `sin` or `cos` functions needed to calculate rotation-relative direction vectors.
- **Solution**: Added `heidic_sin` and `heidic_cos` wrapper functions to `eden_vulkan_helpers.cpp` that call `std::sin` and `std::cos` from `<cmath>`. These functions are declared as `extern "C"` to be callable from Heidic code.

**Challenge 2: Initial Movement in Wrong Direction**
- **Problem**: Initially implemented WASD with fixed world-space directions (W = negative Z, S = positive Z, A = negative X, D = positive X). This meant movement was always in the same direction regardless of rotation, which felt unnatural for a player controller.
- **Solution**: Switched to rotation-relative movement by calculating forward and right direction vectors based on the cube's Y-axis rotation angle.

**Challenge 3: Forward Vector Calculation**
- **Problem**: The forward vector calculation was incorrect. Initially tried `forward = (sin(angle), 0, -cos(angle))`, which caused the cube to move in the X-axis direction (red line) instead of the Z-axis direction (yellow line) when rotated.
- **Debugging Process**:
  1. Added `heidic_imgui_text_float` function to display float values in the debug panel
  2. Added real-time display of forward and right direction vectors to visualize what was being calculated
  3. User reported that at 40° rotation, forward X was 0.643 and forward Z was -0.766, but movement was going in the positive X direction (red line) instead of the Z direction (yellow line)
- **Solution**: After several iterations, determined the correct formula:
  - `forward = (-sin(angle), 0, -cos(angle))`
  - This ensures that at 0° rotation, forward = (0, 0, -1) = negative Z (the model's forward direction)
  - When rotated, the forward vector correctly follows the yellow line (Z-axis) direction
  - The right vector is calculated as `right = (cos(angle), 0, -sin(angle))` for proper strafing

**Challenge 4: Understanding Rotation Conventions**
- **Problem**: Confusion about which direction the forward vector should point after rotation. The question was: if the cube rotates to 30°, should forward be at 30° or 210° (30° + 180°)?
- **Solution**: Forward should be at the same angle as the rotation (30°), not 180° opposite. The forward vector represents where the cube is "facing" after rotation, which matches the visual rotation of the yellow line (Z-axis).

#### Math Functions
- Added `heidic_sin` and `heidic_cos` trigonometric functions to `eden_vulkan_helpers` for direction vector calculations.
- Added `heidic_imgui_text_float` helper function to display float values in the debug panel.

#### Rotation-Relative Movement
- **Forward Vector Calculation**: The forward direction follows the yellow line (Z-axis) direction after rotation.
  - Formula: `forward = (-sin(angle), 0, -cos(angle))`
  - At 0° rotation: forward = (0, 0, -1) = negative Z (model's forward at start)
  - The forward vector correctly rotates with the cube's Y-axis rotation.

#### Controls
- **W (Key 87)**: Move forward in the direction the cube is facing
- **S (Key 83)**: Move backward (opposite of forward)
- **A (Key 65)**: Strafe left (negative right direction)
- **D (Key 68)**: Strafe right (positive right direction)
- **Q (Key 81)**: Rotate Y-axis up (increase rotation)
- **E (Key 69)**: Rotate Y-axis down (decrease rotation)

#### Debug Visualization
- Added direction vector display to ImGui debug panel showing:
  - Forward X/Z: The direction W moves
  - Right X/Z: The direction A/D moves
- Real-time updates as rotation changes
- This debug visualization was crucial for diagnosing the forward vector calculation issue

#### Implementation Details
- Movement speed: 5.0 units (cm) per frame
- Rotation speed: 2.0 degrees per frame
- Movement is calculated relative to the cube's current Y-axis rotation
- The forward vector correctly follows the yellow line (Z-axis) direction after rotation

### 7. ASCII Model Import & Texture Pipeline (Dec 2024)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** December 2024

Implemented ASCII model file format loading and a complete Vulkan texture pipeline to apply textures with transparency support.

#### Goal
Create an `ascii_import_test` example that:
- Loads 3D models from ASCII text files (`test.txt` format)
- Applies a texture (`test.png`) to the imported model
- Supports transparency (alpha channel) for proper rendering

#### Challenges & Solutions

**Challenge 1: Missing Image Loading Library**
- **Problem**: Vulkan requires raw pixel data to create textures, but we had no way to load PNG files.
- **Solution**: Integrated `stb_image.h` (single-header image loader) from `third_party/`. Added `#define STB_IMAGE_IMPLEMENTATION` before including the header to enable the implementation.

**Challenge 2: Vertex Format Mismatch**
- **Problem**: The existing `Vertex` struct used `float color[3]` for RGB colors, but textures require UV coordinates (`float uv[2]`). The ASCII model format includes UV data, but our vertex structure couldn't store it.
- **Solution**: 
  - Modified `Vertex` struct from `{ float pos[3]; float color[3]; }` to `{ float pos[3]; float uv[2]; }`
  - Updated `Vertex::getAttributeDescriptions()` to bind location 1 as `VK_FORMAT_R32G32_SFLOAT` (vec2) instead of `VK_FORMAT_R32G32B32_SFLOAT` (vec3)
  - Updated all vertex creation code (cube, lines, ASCII model loader) to use UVs instead of colors

**Challenge 3: Shader Pipeline Updates**
- **Problem**: Existing shaders expected vertex colors, but we needed texture sampling with UV coordinates.
- **Solution**:
  - **Vertex Shader** (`vert_cube.glsl`): Changed input from `inColor` (vec3) to `inUV` (vec2), output `fragUV` instead of `fragColor`
  - **Fragment Shader** (`frag_cube.glsl`): 
    - Added `layout(binding = 1) uniform sampler2D uTexture;`
    - Changed from using `fragColor` to sampling texture: `vec4 tex = texture(uTexture, fragUV);`
    - Added transparency support: `if (tex.a < 0.01) discard;` to discard transparent pixels

**Challenge 4: Complete Vulkan Texture Pipeline**
- **Problem**: Vulkan requires multiple objects to use textures: `VkImage`, `VkDeviceMemory`, `VkImageView`, `VkSampler`, and descriptor set updates. None of this existed.
- **Solution**: Implemented full texture pipeline in `createTextureAndDescriptors()`:
  1. **Image Loading**: Use `stbi_load()` to load PNG into CPU memory
  2. **Staging Buffer**: Create host-visible buffer, copy pixel data
  3. **VkImage Creation**: Create device-local image with `VK_FORMAT_R8G8B8A8_UNORM`
  4. **Memory Allocation**: Allocate device memory and bind to image
  5. **Layout Transitions**: Transition from `UNDEFINED` → `TRANSFER_DST_OPTIMAL` → `SHADER_READ_ONLY_OPTIMAL`
  6. **Buffer-to-Image Copy**: Use `vkCmdCopyBufferToImage` to transfer data
  7. **ImageView Creation**: Create view for shader access
  8. **Sampler Creation**: Configure filtering and addressing modes
  9. **Descriptor Set Update**: Add `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` at binding 1

**Challenge 5: ASCII Model Format Parsing**
- **Problem**: The ASCII model format (`test.txt`) contains vertices, triangles, skin points, and skin triangles. We needed to parse this and create Vulkan vertex buffers.
- **Solution**: Implemented `heidic_load_ascii_model()`:
  - Parses file line-by-line, extracting vertex positions and UVs
  - Scales positions by `100.0f` to convert from meters to centimeters (EDEN units: 1 unit = 1 cm)
  - Flips V coordinate for UVs (`1.0 - v`) to match Vulkan's coordinate system
  - Creates vertex buffer using the same staging buffer pattern as the cube
  - Returns mesh ID for later drawing

**Challenge 6: Texture File Path Resolution**
- **Problem**: The texture loading code used a hardcoded path `../models/test.png`, but the executable could be run from different directories, causing file not found errors.
- **Initial Error**: Program crashed with "Failed to load texture image" and "failed to allocate buffer memory!" because the code continued execution with uninitialized `texWidth`/`texHeight` values after a failed load.
- **Solution**: 
  - Implemented multiple path attempts: `../models/test.png`, `../../models/test.png`, `models/test.png`, `../test.png`
  - Added fallback texture creation: If all paths fail, create a 1x1 white texture in memory
  - Proper cleanup: Use `delete[]` for fallback texture, `stbi_image_free()` for loaded images
  - This ensures the program always has a valid texture, even if the file is missing

**Challenge 7: Compilation Errors from Vertex Format Change**
- **Problem**: After changing `Vertex` struct, existing code (cube creation, line drawing) still used the old `{pos, color}` format, causing compilation errors.
- **Solution**:
  - Updated `createCube()` to use UV coordinates instead of colors (using simple UV mapping)
  - Updated `heidic_draw_line()` to use zero UVs `{0.0f, 0.0f}` since lines don't need textures
  - Fixed brace initializer errors by ensuring all vertex creation matches the new struct layout

**Challenge 8: Missing Helper Function Declarations**
- **Problem**: `createBuffer()` and `findMemoryType()` were called in `createTextureAndDescriptors()` but declared later in the file, causing "not declared in this scope" errors.
- **Solution**: Moved texture global variable declarations (`g_textureImage`, `g_textureImageMemory`, etc.) to before `createTextureAndDescriptors()`, and ensured helper functions are declared/defined before use.

#### Implementation Details
- **Model Scaling**: ASCII models are scaled by 100x (1 meter → 100 cm) to match EDEN's unit system
- **UV Coordinate System**: V coordinate is flipped (`1.0 - v`) to account for Vulkan's Y-down vs. typical Y-up image formats
- **Transparency**: Fragment shader discards pixels with alpha < 0.01 for proper transparency rendering
- **Back-face Culling**: Disabled `VK_CULL_MODE_BACK_BIT` in pipeline to ensure models render regardless of triangle winding
- **Texture Fallback**: If texture file is missing, a 1x1 white texture is created automatically, allowing the program to run without errors

#### New Functions
- `heidic_load_ascii_model(const char* filename)`: Loads ASCII model file, returns mesh ID
- `heidic_draw_mesh(int mesh_id, float x, float y, float z, float rx, float ry, float rz)`: Draws loaded mesh with transform

#### Example
The `ascii_import_test` example demonstrates:
- Loading a model from `../models/test.txt`
- Applying `test.png` texture with transparency
- Camera at (0, 0, 300) looking at origin
- Model rotating around Y-axis at origin

### 8. Validation Layers & Debug Infrastructure (Dec 2024)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** December 2024

Implemented Phase 1 of the debug architecture: Vulkan validation layers and debug messenger.

#### Goal
Add comprehensive debugging infrastructure to catch Vulkan API errors early, detect memory leaks, and provide better error messages.

#### Implementation

**Challenge 1: No Validation Layers**
- **Problem**: The Vulkan instance was created without validation layers, meaning errors would manifest as silent failures or crashes without clear messages.
- **Solution**: 
  - Added validation layer checking function `checkValidationLayerSupport()`
  - Enabled `VK_LAYER_KHRONOS_validation` in debug builds
  - Automatically disabled in release builds (`NDEBUG`)

**Challenge 2: Debug Message Routing**
- **Problem**: Validation layers need a way to send messages to the application.
- **Solution**: 
  - Implemented `VK_EXT_debug_utils` extension support
  - Created debug messenger with custom callback function
  - Messages filtered by severity (ERROR, WARNING, INFO, VERBOSE)
  - Console output with clear prefixes: `[Vulkan Validation] ERROR/WARNING/INFO`

**Challenge 3: Extension Management**
- **Problem**: Debug utils extension must be enabled at instance creation, but only if validation layers are available.
- **Solution**: 
  - Dynamically build extension list (GLFW extensions + debug utils)
  - Check for validation layer availability before enabling
  - Graceful fallback if layers aren't available

#### Features Implemented

1. **Validation Layer Support**:
   - `VK_LAYER_KHRONOS_validation` (all-in-one validation layer)
   - Automatic detection and enabling in debug builds
   - Disabled in release builds for performance

2. **Debug Messenger**:
   - Custom callback function `debugCallback()` for handling validation messages
   - Severity filtering (shows errors/warnings in release, everything in debug)
   - Clear console output with message type prefixes

3. **Extension Support**:
   - `VK_EXT_debug_utils` extension enabled when validation layers are active
   - Proper cleanup of debug messenger on shutdown

#### Code Structure

**New Functions**:
- `checkValidationLayerSupport()`: Verifies validation layers are available
- `debugCallback()`: Handles validation layer messages
- `CreateDebugUtilsMessengerEXT()`: Wrapper for debug messenger creation
- `DestroyDebugUtilsMessengerEXT()`: Wrapper for debug messenger destruction
- `setupDebugMessenger()`: Initializes debug messenger after instance creation

**Modified Functions**:
- `heidic_init_renderer()`: Now enables validation layers and debug utils extension
- `heidic_cleanup_renderer()`: Now cleans up debug messenger

#### Benefits

1. **Early Error Detection**: Catches Vulkan API misuse immediately during development
2. **Better Debugging**: Clear error messages instead of silent failures
3. **Memory Leak Detection**: Validation layers track resource lifetime
4. **Synchronization Validation**: Catches race conditions and sync errors
5. **Zero Performance Impact in Release**: Automatically disabled in release builds

#### Bugs Caught by Validation Layers

The validation layers immediately caught critical synchronization bugs that would have caused crashes or undefined behavior:

**Bug 1: Semaphore Reuse with Swapchain**
- **Problem**: Using a single semaphore for all swapchain images. When the same semaphore was used for different swapchain images, Vulkan detected that a semaphore was being reused while still in use by a previous presentation operation.
- **Error**: `vkQueueSubmit(): pSubmits[0].pSignalSemaphores[0] is being signaled by VkQueue, but it may still be in use by VkSwapchainKHR`
- **Fix**: Created per-swapchain-image semaphores (`g_imageAvailableSemaphores` and `g_renderFinishedSemaphores` vectors), using `imageIndex` to select the correct semaphore for each swapchain image.

**Bug 2: Missing Synchronization with Acquire**
- **Problem**: `vkAcquireNextImageKHR` was called with both semaphore and fence as `VK_NULL_HANDLE`, which is invalid. Vulkan requires at least one synchronization primitive.
- **Error**: `vkAcquireNextImageKHR(): semaphore and fence are both VK_NULL_HANDLE`
- **Fix**: Use `g_imageAvailableFence` with `vkAcquireNextImageKHR` to properly synchronize image acquisition.

**Bug 3: Semaphore Mismatch**
- **Problem**: Attempting to wait on a semaphore in `vkQueueSubmit` that was never signaled by `vkAcquireNextImageKHR`, causing a deadlock.
- **Error**: `vkQueueSubmit(): pWaitSemaphores[0] is waiting on semaphore that has no way to be signaled`
- **Fix**: Use fence-based synchronization for acquire, then use the correct render-finished semaphore (based on `imageIndex`) for presentation.

#### Understanding Vulkan Synchronization: Semaphores vs Fences

**Semaphores** are GPU-to-GPU synchronization primitives:
- Used to synchronize operations between different GPU queues or between GPU and presentation engine
- Binary state: unsignaled → signaled (one-way, can't be reset by CPU)
- Example: "Wait for image to be acquired" → "Signal when rendering is done" → "Present waits for signal"
- **Key Point**: Each swapchain image needs its own semaphore to avoid reuse conflicts

**Fences** are GPU-to-CPU synchronization primitives:
- Used to know when the CPU can safely access GPU resources
- Can be waited on by CPU (`vkWaitForFences`)
- Can be reset by CPU (`vkResetFences`)
- Example: "Wait for previous frame to finish before starting new one"
- **Key Point**: Fences block the CPU, semaphores coordinate GPU operations

**In Our Render Loop**:
1. **Fence** (`g_inFlightFence`): Ensures CPU waits for previous frame to complete
2. **Fence** (`g_imageAvailableFence`): Ensures image acquisition is complete before using it
3. **Semaphore** (`g_renderFinishedSemaphores[imageIndex]`): Signals presentation engine that rendering is done for this specific swapchain image

**Why Per-Image Semaphores?**
- Swapchain has multiple images (typically 3) that rotate
- Each image can be at different stages: acquiring, rendering, presenting
- Using the same semaphore for different images causes conflicts
- Solution: One semaphore per swapchain image, selected by `imageIndex`

#### Next Steps (Future Phases)

- **Phase 2**: Object naming for better error messages (name buffers, images, pipelines)
- **Phase 3**: Performance markers for GPU profiling
- **Phase 4**: Enhanced error reporting with ImGui debug panel

### 8. Colored Geometry Rendering Fix (Dec 2024)

After adding vertex color support to the `Vertex` structure to enable textured models, we encountered an issue where the `top_down` example's colored cube faces and axes lines appeared black or invisible.

#### The Problem

When we added `color[3]` to the `Vertex` structure to support both colored geometry (cube, lines) and textured meshes, we updated the shaders to pass vertex colors through the pipeline. However, the fragment shader was always sampling a texture and multiplying it by the vertex color:

```glsl
vec4 tex = texture(uTexture, fragUV);
outColor = vec4(tex.rgb * fragColor, tex.a);
```

For colored geometry like the cube and axes:
- UVs were set to `(0, 0)` (not intended for texture sampling)
- Vertex colors were set to the desired RGB values
- The texture sampled at `(0, 0)` was either black or uninitialized
- Result: `black * color = black` → geometry appeared black/invisible

#### The Solution

We modified the fragment shader (`frag_cube.glsl`) to detect when UVs are `(0, 0)` and use vertex color directly without texture sampling:

```glsl
// Check if UVs are essentially (0,0) - this indicates colored geometry
if (abs(fragUV.x) < 0.01 && abs(fragUV.y) < 0.01) {
    // Colored geometry - use vertex color directly (no texture sampling)
    outColor = vec4(fragColor, 1.0);
} else {
    // Textured mesh - sample texture and multiply by vertex color
    vec4 tex = texture(uTexture, fragUV);
    if (tex.a < 0.01)
        discard;
    // For textured meshes, vertex color is white (1,1,1), so texture * white = texture
    outColor = vec4(tex.rgb * fragColor, tex.a);
}
```

**How It Works:**
- **Colored Geometry** (cube, lines): UVs are `(0, 0)`, so vertex color is used directly. The fallback white texture at `(0, 0)` ensures `white * color = color`.
- **Textured Meshes**: UVs are valid, so texture is sampled. Vertex colors are set to white `(1, 1, 1)` for textured meshes, so `texture * white = texture`.

#### Challenges Overcome

1. **Shader Recompilation**: After modifying the shaders, we had to recompile both vertex and fragment shaders. The validation layers caught the mismatch when the SPIR-V files were out of date, showing errors like "fragColor at Location 1 not an Output declared in VK_SHADER_STAGE_VERTEX_BIT".

2. **Unified Shader Approach**: We chose to use a single shader for both colored and textured geometry rather than separate pipelines, which simplifies the codebase and reduces pipeline state management.

3. **Vertex Structure Updates**: The `Vertex` structure now includes `pos[3]`, `uv[2]`, and `color[3]`, requiring updates to:
   - `getAttributeDescriptions()` to include location 2 for color
   - `createCube()` to assign original RGB colors to vertices
   - `heidic_draw_line()` to pass `r, g, b` as vertex colors
   - `heidic_load_ascii_model()` to set vertex colors to white for textured meshes

#### Result

The `top_down` example now correctly displays:
- **Colored cube faces**: Each face has its original distinct color (red, green, blue, yellow, cyan, magenta)
- **Colored axes lines**: X (red), Y (green), Z (blue) axes display with their intended colors
- **Textured meshes**: ASCII-imported models continue to render correctly with textures

This fix demonstrates the importance of understanding the interaction between vertex attributes, shader logic, and texture sampling in a unified rendering pipeline.

## Current Status (Dec 2024)
- **Compiler**: Working with updated C ABI support for strings.
- **Renderer**: Vulkan pipeline fully functional for 3D primitives (Triangles and Lines), with complete texture support including transparency. Unified shader supports both colored geometry and textured meshes.
- **Model Loading**: ASCII model format parser with automatic unit conversion and UV mapping.
- **Debug Infrastructure**: Validation layers and debug messenger enabled in debug builds for comprehensive error detection.
- **Tools**: ImGui integrated with toggleable visibility and float display helpers.
- **Examples**: 
  - `examples/top_down`: Programmable cube with colored faces, colored debug axes, camera controls, and full WASD player movement system with rotation-relative controls.
  - `examples/ascii_import_test`: ASCII model import with texture application and transparency support.
  - `examples/spinning_triangle`: Basic triangle rendering test.
