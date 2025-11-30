# EDEN ENGINE (Heidic V2) Development Log

**Date:** November 29, 2025
**AI Assistant:** Gemini 3-Pro

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
  - This ensures that at 0° rotation, forward = (0, 0, -1) = negative Z (model's forward at start)
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

### 9. Build System Fixes & FPS Camera Example (Nov 30, 2025)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** November 30, 2025

Fixed critical batch script parsing issues and successfully created a working build system for the `fps_camera` example.

#### Goal
Create a new `fps_camera` example by cloning the `top_down` example, and fix the build script to successfully compile and link the example with all dependencies (Vulkan, GLFW, ImGui).

#### Challenges & Solutions

**Challenge 1: Batch Script Parsing Errors**
- **Problem**: The build script (`build_fps_camera.bat`) failed with `"... was unexpected at this time."` errors when trying to evaluate conditional statements with delayed expansion variables.
- **Root Cause**: Windows batch files have complex parsing rules when mixing delayed expansion (`!VAR!`) with normal expansion (`%VAR%`) in `if` statements. The `if !GLFW_FOUND! equ 0` syntax was causing parser errors.
- **Solution**: Restructured the conditional logic to use `goto` labels instead of nested `if/else` blocks. This avoids batch parser issues entirely:
  ```batch
  if "!GLFW_FOUND!"=="0" goto :glfw_not_found
  goto :glfw_found
  
  :glfw_not_found
  REM Handle GLFW not found case
  goto :glfw_check_done
  
  :glfw_found
  REM Handle GLFW found case
  goto :glfw_check_done
  
  :glfw_check_done
  ```

**Challenge 2: Variable Scope Loss After `endlocal`**
- **Problem**: Variables set with delayed expansion were being lost after calling `endlocal`, causing the link command to be built without object files, includes, or library paths.
- **Root Cause**: Attempted to save variables to `_SAVE` variants before `endlocal`, but these `_SAVE` variables were also in the local scope and got destroyed.
- **Solution**: Removed the problematic `endlocal` call in the middle of the script. Variables are now used directly with delayed expansion throughout the entire script, with only one `endlocal` at the very end for cleanup. This keeps all variables (`GLFW_FOUND`, `GLFW_LIB_PATH`, `VULKAN_SDK_PATH`, `INCLUDES`, `LIBPATHS`, `OBJ_FILES`) available throughout the build process.

**Challenge 3: Path Handling with Backslashes**
- **Problem**: Windows paths with backslashes (`C:\glfw-3.4\build\src`) can cause issues in batch scripts and with `g++` linking.
- **Solution**: Convert GLFW library paths to forward slashes (`C:/glfw-3.4/build/src`) when adding to the link command using `set "GLFW_LIB_PATH_FORWARD=!GLFW_LIB_PATH:\=/!"`.

**Challenge 4: GCC Internal Compiler Error with ImGui**
- **Problem**: GCC 15.2.0 (from WinLibs) crashes with a segmentation fault when compiling ImGui files with `-O3` optimization, specifically in `avx512fintrin.h` when processing AVX-512 intrinsics.
- **Status**: Known GCC bug. The build still succeeds because ImGui object files from previous builds are reused. This is a compiler issue, not a build script issue.
- **Future Fix**: Could lower optimization for ImGui files (`-O2` or `-O1`) or skip compilation if object files already exist.

#### Implementation Details

**Build Script Structure**:
1. **HEIDIC Compilation**: Compiles `.hd` file to `.cpp` using `cargo run -- compile`
2. **Path Detection**: Automatically finds Vulkan SDK and GLFW installation
3. **Object Compilation**: Compiles `eden_vulkan_helpers.cpp`, ImGui files, and the generated `.cpp`
4. **Linking**: Builds complete link command with all object files, includes, library paths, and libraries

**Variable Management**:
- Uses `setlocal enabledelayedexpansion` at the start
- All variables use delayed expansion (`!VAR!`) for safe handling of special characters
- Single `endlocal` at the end for cleanup
- No variable saving/restoring needed

**Library Detection**:
- **Vulkan SDK**: Checks `VULKAN_SDK` environment variable, falls back to `C:\VulkanSDK\<version>`
- **GLFW**: Checks `GLFW_PATH` environment variable, falls back to `C:\glfw-3.4`, then recursively searches for library files (`libglfw3.a`, `glfw3.a`, `libglfw3.dll.a`)

**Link Command Construction**:
- Base: `g++ -std=c++17 -O3`
- Includes: Project root, stdlib, GLFW, Vulkan SDK, ImGui
- Library Paths: Vulkan SDK lib directory
- Object Files: All compiled `.o` files
- Libraries: `-lvulkan-1`, `-L<glfw_path> -lglfw3`, Windows system libraries (`-lgdi32 -luser32 -lshell32`)

#### Debug Output

Added comprehensive debug output to trace:
- Variable values before/after operations
- Step-by-step link command construction
- Final complete link command
- Exit codes for each compilation/linking step

This debug output was crucial for identifying the variable scope loss issue.

#### Result

The `fps_camera` example now builds successfully:
- ✅ HEIDIC compilation works
- ✅ C++ compilation works (except ImGui GCC bug, but uses cached objects)
- ✅ Linking succeeds with all dependencies
- ✅ Executable is created: `fps_camera.exe`

The build script is now robust and can be used as a template for future examples.

#### Files Created/Modified

- **Created**: `examples/fps_camera/fps_camera.hd` (cloned from `top_down.hd`)
- **Created**: `examples/fps_camera/fps_camera.cpp` (generated from `fps_camera.hd`)
- **Created**: `examples/fps_camera/build_fps_camera.bat` (build script with all fixes)
- **Created**: `examples/fps_camera/frag_cube.spv` and `vert_cube.spv` (copied from `top_down`)

### 10. Gateway Editor v1: Raycasting, Selection, and Camera System (Nov 29, 2025)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** November 29, 2025

Created a new `gateway_editor_v1` example project with comprehensive raycasting, mouse selection, ground plane, and enhanced camera controls.

#### Goal
Build an editor-like example with mouse picking, object selection, ground plane visualization, and improved camera controls (top-down zoom, video mode toggle).

#### Features Implemented

**1. Mouse Raycasting System**
- **Implementation**: CPU-based raycasting using Möller-Trumbore slab method for AABB intersection
- **Functions Added**:
  - `heidic_get_mouse_x/y()`: Get mouse screen coordinates
  - `heidic_get_mouse_ray_origin/dir()`: Get world-space ray from mouse position
  - `heidic_raycast_cube_hit()`: Test ray against cube AABB
  - `heidic_raycast_cube_hit_point()`: Get hit point on cube surface
  - `heidic_raycast_ground_hit()`: Test ray against ground plane
  - `heidic_raycast_ground_hit_point()`: Get ground hit point
- **Technical Details**:
  - Converts mouse screen coordinates to NDC (Normalized Device Coordinates)
  - Unprojects NDC to world-space ray using inverse projection and view matrices
  - Ray origin is camera position, direction calculated from near/far plane points
  - AABB intersection test with epsilon handling for division by zero
  - Precision improvements for distance calculations

**2. Object Selection System**
- **Visual Feedback**: Black wireframe overlay on selected cubes using `heidic_draw_cube_wireframe()`
- **Closest-Hit Detection**: Tests all cubes and selects the closest hit point
- **Mouse Button Input**: Added `heidic_is_mouse_button_pressed()` for left-click selection
- **Selection State**: Tracks selected cube position, size, and selection status

**3. Ground Plane System**
- **Solid Ground Plane**: 100m × 100m × 1m gray cube at y = -5m
- **Grid Overlay**: Toggleable grid pattern (press 'G' to toggle) at y = -3m
- **Ground Detection**: Raycast straight down from player position to detect ground contact
- **Visual Feedback**: Green line when grounded, red line when in air

**4. Camera System Enhancements**
- **Top-Down Camera Zoom**: Mouse scroll wheel adjusts camera height (10m to 500m range)
  - Scroll up = zoom in (lower height)
  - Scroll down = zoom out (higher height)
  - Only active when not hovering over ImGui windows
- **Video Mode Toggle**: Shift+Enter toggles between fullscreen and windowed mode
  - Starts in windowed mode by default
  - Uses `glfwSetWindowMonitor()` for mode switching
- **Camera Modes**: 'C' key toggles between FPS camera (follows player) and top-down camera
- **Far Plane Control**: Top-down mode uses 50km far plane, FPS uses default 5km

**5. Debug Visualization**
- **Yellow Ray Line**: Visual debug line from player to mouse ray end point (500m along ray, or hit point if selected)
- **Debug Panel**: Comprehensive raycasting diagnostics:
  - Mouse screen coordinates
  - Ray origin and direction (world space)
  - Ray end point coordinates
  - Distance calculations (origin to end, player to end, camera to origin)
- **Real-time Updates**: All values update in real-time as mouse moves

**6. HEIDIC Language Improvements**
- **Include Directive**: Implemented `include "path";` for modularizing extern declarations
  - Supports `stdlib/` paths (resolved relative to project root via Cargo.toml)
  - Supports relative paths (resolved relative to current file)
  - Prevents circular includes
  - Merges included items into current file's AST
- **Standard Library Header**: Created `stdlib/eden.hd` with common extern function declarations
- **Camera Type**: Added `Camera` struct type support in parser and type checker

**7. Engine Function Additions**
- **Vector Operations**: `heidic_vec3()`, `heidic_vec3_add()`, `heidic_vec_copy()`
- **Camera Functions**: `heidic_update_camera_with_far()`, `heidic_create_camera()`, `heidic_update_camera_from_struct()`
- **Utility Functions**: `heidic_get_fps()`, `heidic_set_video_mode()`, `heidic_get_mouse_scroll_y()`
- **Raycasting Functions**: Complete suite of mouse ray and hit detection functions
- **Drawing Functions**: `heidic_draw_cube_wireframe()`, `heidic_draw_ground_plane()`

#### Challenges & Solutions

**Challenge 1: Ray Direction Y Inversion**
- **Problem**: Mouse ray tracked correctly side-to-side but was inverted up/down
- **Root Cause**: Incorrect NDC coordinate mapping in `screenToNDC()` function
- **Solution**: Fixed mapping to correctly convert screen Y=0 (top) to NDC Y=1 (top), screen Y=height (bottom) to NDC Y=-1 (bottom)

**Challenge 2: Selection Only Works at Close Range**
- **Problem**: Cubes could only be selected when very close, despite ray visually hitting them from far away
- **Root Cause**: Precision issues in ray-AABB intersection test and potential ray direction normalization problems
- **Solution**: 
  - Added explicit ray direction normalization in `rayAABB()` function
  - Added epsilon for floating-point comparison to handle precision at distance
  - Made hit test more lenient with `tMax >= tMin - epsilon` instead of strict equality

**Challenge 3: Zoom Not Working**
- **Problem**: Mouse scroll wheel didn't zoom top-down camera
- **Root Cause**: ImGui was consuming scroll events before game code could read them
- **Solution**: Use ImGui's `MouseWheel` value directly, which is 0 if ImGui consumed it, or the scroll delta if available

**Challenge 4: Player Spawning Inside Blocks**
- **Problem**: Player spawned at origin (0,0,0) which was inside a reference cube
- **Solution**: Moved spawn position to (3000, 100, 3000) - 30m away from origin, 1m above ground

**Challenge 5: Fullscreen Mode Unresponsiveness**
- **Problem**: When toggling to fullscreen, application became unresponsive (white screen, no input)
- **Status**: Documented in `docs/KNOWN_ISSUES.md` as known issue
- **Likely Cause**: Swapchain recreation needed when changing display modes
- **Workaround**: Start in windowed mode, toggle to fullscreen only when needed

#### Implementation Details

**Raycasting Pipeline**:
1. Get mouse screen coordinates from GLFW
2. Convert to NDC using `screenToNDC()` (handles Y-axis flip)
3. Unproject NDC to world-space ray using inverse projection and view matrices
4. Test ray against AABB using Möller-Trumbore slab method
5. Return hit status and hit point

**Selection Algorithm**:
1. On mouse click, test ray against all cubes
2. For each hit, calculate distance from ray origin to hit point
3. Select cube with minimum distance (closest hit)
4. Draw wireframe overlay on selected cube

**Ground Plane**:
- Solid cube: 100m × 100m × 1m at y = -5m
- Grid: 200m × 200m pattern at y = -3m (toggleable with 'G')
- Ground detection: Raycast straight down from player position

**Camera Zoom**:
- Scroll delta read from ImGui's `MouseWheel`
- Adjusts `topdown_cam_height` (clamped 10m-500m)
- Updates camera position each frame

#### Files Created/Modified

- **Created**: `examples/gateway_editor_v1/gateway_editor_v1.hd` (main game logic)
- **Created**: `examples/gateway_editor_v1/build_gateway_editor_v1.bat` (build script)
- **Created**: `stdlib/eden.hd` (standard library header with extern declarations)
- **Created**: `docs/RAYCASTING.md` (raycasting API documentation)
- **Created**: `docs/KNOWN_ISSUES.md` (known issues tracking)
- **Modified**: `vulkan/eden_vulkan_helpers.cpp/h` (raycasting, ground plane, zoom, video mode functions)
- **Modified**: `src/parser.rs` (include directive support)
- **Modified**: `src/lexer.rs` (Include and Camera tokens)
- **Modified**: `src/ast.rs` (Include item and Camera type)
- **Modified**: `src/type_checker.rs` (Camera type compatibility)
- **Modified**: `src/codegen.rs` (Camera type code generation)

### 11. PICKING HELL: 11/29/2025 (Nov 29, 2025)
**AI Assistant:** Auto (Claude Sonnet 4.5)
**Date:** November 29, 2025

Resolved critical ray picking issues that caused inverted Y-axis tracking and inability to select objects at a distance.

#### The Problem
Selection worked somewhat when very close to objects but failed completely at distance. Additionally, the ray tracked the mouse correctly horizontally but was inverted vertically (moving mouse up moved ray down).

#### The Diagnosis
The root cause was a fundamental mismatch between coordinate spaces (Vulkan vs OpenGL vs Screen) and how the projection matrix was configured.

1. **Vulkan NDC Y**: Our projection matrix `glm::perspectiveRH_ZO` combined with `proj[1][1] *= -1` creates a clip space where Y points **down** (top is -1, bottom is +1).
2. **Screen to NDC**: We were using an OpenGL-style mapping (`1.0 - ...`) which produced Y=1 at the top. Since our projection expects Y=-1 at the top, this inverted the ray direction.
3. **Unprojection Z**: We were using `[0, 1]` for the clip space Z range. However, `glm::unproject` logic (and manual unprojection) expects the canonical view volume to be unprojected from `[-1, 1]` in clip space, regardless of the depth range used by the projection matrix (`_ZO`). Using `0` for the near plane caused the unprojected point to be wrong, leading to a ray that pointed backwards or sideways.

#### The Fix

**1. Correct Screen-to-NDC Mapping**
```cpp
// Map screen Y=0 (top) to NDC Y=-1 (top)
// Map screen Y=height (bottom) to NDC Y=1 (bottom)
float ndcY = (2.0f * screenY / height) - 1.0f;
```
This correctly feeds the Y-down projection matrix.

**2. Correct Unprojection Z Range**
```cpp
// Use full clip space range [-1, 1] for unprojection
glm::vec4 clipNear = glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);
glm::vec4 clipFar = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
```
Even though Vulkan depth is [0, 1], the clip space math for unprojection requires the standard homogenous clip volume definition.

**3. Robust Ray Origin**
Instead of using the unprojected near-plane point (which can suffer from precision issues if near plane is very close to 0), we now explicitly use the camera position as the ray origin:
```cpp
rayOrigin = g_currentCamPos;
rayDir = glm::normalize(worldFarPoint - rayOrigin);
```

#### Result
- **Pixel-Perfect Picking**: The ray now exactly matches the mouse cursor position.
- **Infinite Distance**: Selection works correctly at any distance.
- **Correct Orientation**: No more inverted Y-axis or backward rays.

#### The Final Boss: The "10 Meter" Wall
Even after fixing the ray direction, we hit one last wall: selection stopped working beyond ~10 meters.
- **The Culprit:** In `gateway_editor_v1.hd`, `closest_dist` was initialized to `999999.0`.
- **The Math:** This variable tracks *squared distance*. `sqrt(999999) ≈ 1000` units = **10 meters**.
- **The Fix:** Initialized `closest_dist` to `100,000,000,000.0` (approx 3.16 km range).

...and Gemini 3 saves the day AGAIN! Give that robot a pat on the back.

### 12. GIZMO TRANSLATION & DYNAMIC CUBE CREATION: 11/29/2025 (Nov 29, 2025)
**AI Assistant:** Claude Sonnet 4.5
**Date:** November 29, 2025

Implemented object manipulation via translation gizmos and a dynamic cube creation system that supports unlimited level assets.

#### The Problem
After getting ray picking working, we needed:
1. **Visual feedback for selection** - Users needed to see which object was selected
2. **Object manipulation** - Selected objects needed to be moveable via a gizmo
3. **Level building** - Developers needed to create arbitrary numbers of cubes, not just a fixed set

#### The Solution

**1. Translation Gizmo Implementation**

Created a 3-axis translation gizmo (RGB = XYZ) that allows dragging objects in 3D space:

```cpp
Vec3 heidic_gizmo_translate(GLFWwindow* window, float x, float y, float z);
int heidic_gizmo_is_interacting();  // Returns 1 if gizmo is being dragged
```

**Gizmo Features:**
- **RGB Axes**: Red (X), Green (Y), Blue (Z) lines extending from object center
- **Hover Detection**: Axes highlight when mouse hovers over them
- **Drag Interaction**: Click and drag an axis to move the object along that axis
- **Closest Distance Algorithm**: Uses line-to-line distance calculation to determine which axis is being dragged
- **Interaction Lock**: Prevents new selections while dragging the gizmo

**Implementation Details:**
- Gizmo state stored in static variables (`g_gizmoActiveAxis`, `g_gizmoInitialPos`, `g_gizmoDragOffset`)
- Uses `closestDistanceBetweenLines()` helper to find closest points between mouse ray and gizmo axis lines
- Returns new position as `Vec3` that updates the selected object

**2. Selection Wireframe Overlay**

Added visual feedback for selected objects:
```cpp
void heidic_draw_cube_wireframe(float x, float y, float z, float rx, float ry, float rz, 
                                 float sx, float sy, float sz, float r, float g, float b);
```
- Draws black wireframe outline around selected cube (1% larger than actual cube)
- Provides clear visual indication of what's selected

**3. Dynamic Cube Storage System**

Replaced the limited variable-based cube system (max 5 cubes) with a C++ vector-based dynamic storage:

**C++ Storage:**
```cpp
struct CreatedCube {
    float x, y, z;
    float sx, sy, sz;  // size
    int active;  // 1 = exists, 0 = deleted
};
static std::vector<CreatedCube> g_createdCubes;
```

**HEIDIC API:**
```heidi
extern fn heidic_create_cube(x: f32, y: f32, z: f32, sx: f32, sy: f32, sz: f32): i32;
extern fn heidic_get_cube_count(): i32;  // Active cubes
extern fn heidic_get_cube_total_count(): i32;  // Total (including deleted)
extern fn heidic_get_cube_x(index: i32): f32;
extern fn heidic_get_cube_y(index: i32): f32;
extern fn heidic_get_cube_z(index: i32): f32;
extern fn heidic_set_cube_pos_f(index: f32, x: f32, y: f32, z: f32): void;
extern fn heidic_delete_cube(index: i32): void;
```

**4. Spacebar Cube Creation**

Implemented cube creation via spacebar:
- Press **Spacebar** → Creates cube at mouse ray hit point on ground plane
- If no ground hit, places cube 50 units along the ray
- Cubes placed 1 meter above ground when ground is hit
- Each cube gets a unique index (stored in C++ vector)
- No limit on number of cubes

**5. Type System Challenges**

HEIDIC's strict type system required helper functions for type conversions:
- Added `heidic_int_to_float(int value): float` to convert `i32` to `f32`
- Created `heidic_set_cube_pos_f()` overload that accepts `f32` index (since `selected_cube_index` is `f32`)
- Fixed multiple type mismatch errors during implementation

**6. Selection & Gizmo Integration**

**Selection Index Mapping:**
- Player Cube: Index `0.0`
- Reference Cube 1: Index `1.0`
- Created Cubes: Indices `2.0+` (cube storage index + 2.0)

**Gizmo Update Logic:**
```heidi
if selected_cube_index >= 2.0 {
    let cube_storage_index: f32 = selected_cube_index - 2.0;
    heidic_set_cube_pos_f(cube_storage_index, selected_cube_x, selected_cube_y, selected_cube_z);
}
```

**Drawing Loop:**
All created cubes are drawn each frame via a `while` loop iterating through the storage vector:
```heidi
let cube_draw_index: i32 = 0;
let total_cubes: i32 = heidic_get_cube_total_count();
while cube_draw_index < total_cubes {
    if heidic_get_cube_active(cube_draw_index) == 1 {
        // Draw cube at heidic_get_cube_x/y/z(cube_draw_index)
    }
    cube_draw_index = cube_draw_index + 1;
}
```

**Selection Loop:**
All created cubes are tested for raycast hits in a similar loop, finding the closest hit.

#### Result
- **Unlimited Cubes**: Developers can now create as many level assets as needed
- **Visual Selection**: Clear wireframe feedback shows what's selected
- **Intuitive Manipulation**: RGB gizmo allows precise 3-axis translation
- **Seamless Integration**: Created cubes work identically to pre-placed cubes (selection, gizmo, etc.)

#### Files Modified

- **`vulkan/eden_vulkan_helpers.cpp/h`**: 
  - Added `heidic_gizmo_translate()` and `heidic_gizmo_is_interacting()`
  - Added `heidic_draw_cube_wireframe()`
  - Added dynamic cube storage system (vector + all getter/setter functions)
  - Added `heidic_int_to_float()` helper for type conversion
- **`stdlib/eden.hd`**: Added extern declarations for all new functions
- **`examples/gateway_editor_v1/gateway_editor_v1.hd`**: 
  - Replaced variable-based cube system with dynamic storage
  - Added spacebar cube creation
  - Integrated gizmo for all selectable objects
  - Added selection wireframe drawing
  - Updated drawing and selection loops to iterate through all cubes

#### Technical Notes

**Gizmo Algorithm:**
The gizmo uses a closest-distance-between-lines algorithm to determine which axis is being dragged. For each axis, it calculates the closest points between:
1. The mouse ray (from camera through mouse cursor)
2. The gizmo axis line (from object center along X/Y/Z)

The axis with the smallest distance is considered "hovered" or "dragged".

**Index Mapping:**
The `selected_cube_index` system uses floats to distinguish between different object types:
- `0.0` = Player cube
- `1.0` = Reference cube 1
- `2.0+` = Created cubes (mapped to storage index by subtracting 2.0)

This allows a single selection system to handle all object types while maintaining type safety.

### Path Forward & Preservation

To ensure this victory cannot be undone:

1.  **Do NOT Change `unproject` Z-Range:** Keep `clipNear.z = -1.0f` and `clipFar.z = 1.0f` in `eden_vulkan_helpers.cpp`. This is the magic combo for our current projection matrix setup.
2.  **Do NOT Revert `screenToNDC`:** Screen Y=0 must map to NDC Y=-1.
3.  **Do NOT Lower `closest_dist`:** In HEIDIC scripts, always use a massive number (or `FLT_MAX` equivalent) for closest-hit search variables when comparing squared distances.
4.  **Reference `docs/RAYCASTING.md`:** This file now documents the working API. Any changes to raycasting logic should update this doc first.

### Recovery Plan (If things go wrong)
If selection breaks again:
1.  Check `eden_vulkan_helpers.cpp`: Search for `unproject`. Ensure `clipNear.z` is `-1.0f`.
2.  Check `gateway_editor_v1.hd`: Search for `closest_dist`. Ensure it's `> 100,000,000,000.0`.
3.  Check `screenToNDC`: Ensure `ndcY` calculation maps 0 to -1.

## Current Status (Nov 30, 2025)
- **Compiler**: Working with updated C ABI support for strings.
- **Renderer**: Vulkan pipeline fully functional.
- **Raycasting**: Fully working mouse picking and selection system (infinite range).
- **Examples**:
  - `examples/gateway_editor_v1`: Advanced editor example with ray picking and gizmos.
