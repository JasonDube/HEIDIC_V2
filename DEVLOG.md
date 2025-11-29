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

## Current Status (Dec 2024)
- **Compiler**: Working with updated C ABI support for strings.
- **Renderer**: Vulkan pipeline fully functional for 3D primitives (Triangles and Lines).
- **Tools**: ImGui integrated with toggleable visibility and float display helpers.
- **Example**: `examples/top_down` runs successfully, featuring a programmable cube with debug axes, camera controls, and full WASD player movement system with rotation-relative controls.
