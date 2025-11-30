# Raycasting System

## Overview

The EDEN Engine now supports raycasting from mouse position to world-space objects. This enables click detection, object selection, and interaction systems.

## Implementation Status

### ✅ Completed

1. **Mouse Position Access**
   - `heidic_get_mouse_x(window)`: Returns mouse X coordinate in screen space
   - `heidic_get_mouse_y(window)`: Returns mouse Y coordinate in screen space

2. **Mouse-to-Ray Conversion**
   - `heidic_get_mouse_ray_origin(window)`: Returns world-space ray origin (camera position)
   - `heidic_get_mouse_ray_dir(window)`: Returns normalized world-space ray direction
   - Uses unproject algorithm with current camera view/projection matrices

3. **Ray-AABB Intersection**
   - Möller-Trumbore slab method implementation
   - Fast, accurate intersection testing for axis-aligned bounding boxes
   - Used internally for cube raycasting

4. **Cube Raycasting**
   - `heidic_raycast_cube_hit(window, x, y, z, sx, sy, sz)`: Returns 1 if ray hits cube, 0 otherwise
   - `heidic_raycast_cube_hit_point(window, x, y, z, sx, sy, sz)`: Returns world-space hit point if ray hits cube

## Usage Example

```heidic
include "stdlib/eden.hd";

fn main(): void {
    let window: GLFWwindow = heidic_create_window(1920, 1080, "Raycast Test");
    heidic_init_renderer(window);
    
    // Game loop
    loop {
        heidic_poll_events();
        
        // Get mouse position
        let mouseX: f32 = heidic_get_mouse_x(window);
        let mouseY: f32 = heidic_get_mouse_y(window);
        
        // Test raycast against a cube at (0, 0, 0) with size (100, 100, 100)
        let hit: i32 = heidic_raycast_cube_hit(window, 0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
        
        if hit == 1 {
            // Get hit point
            let hitPoint: Vec3 = heidic_raycast_cube_hit_point(window, 0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
            heidic_imgui_text("Cube hit!");
            heidic_imgui_text_float("Hit X", hitPoint.x);
            heidic_imgui_text_float("Hit Y", hitPoint.y);
            heidic_imgui_text_float("Hit Z", hitPoint.z);
        }
        
        heidic_begin_frame();
        heidic_draw_cube(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
        heidic_end_frame();
    }
}
```

## Technical Details

### Unproject Algorithm

The mouse-to-ray conversion uses the standard unproject algorithm:

1. Convert screen coordinates to normalized device coordinates (NDC): [-1, 1]
2. Create clip-space positions for near and far planes
3. Transform to eye space using inverse projection matrix
4. Transform to world space using inverse view matrix
5. Calculate ray direction from near to far point

### Ray-AABB Intersection

Uses the Möller-Trumbore slab method:

1. Calculate inverse ray direction
2. Compute intersection intervals for each axis
3. Find overlap of all intervals
4. Return true if overlap exists and is in front of ray origin

### Performance

- **Mouse position**: O(1) - Direct GLFW call
- **Ray calculation**: O(1) - Matrix inversions and vector operations
- **AABB intersection**: O(1) - Constant-time slab test
- **Overall**: Very fast, suitable for per-frame use

## Future Enhancements

### Pending

1. **Triangle Intersection (Möller-Trumbore)**
   - For accurate model raycasting
   - Needed for complex meshes

2. **Embree Integration**
   - High-performance raycasting library (Intel)
   - Supports BVH acceleration structures
   - 1M+ triangles per millisecond
   - Required for complex scenes with many models

3. **AABB Struct in HEIDIC**
   - Expose AABB type to HEIDIC language
   - Allow custom AABB definitions
   - Enable more flexible raycasting

4. **Spatial Acceleration**
   - Octree for many cubes (1M+)
   - BVH for models
   - Grid-based spatial partitioning

## API Reference

### Mouse Functions

```heidic
extern fn heidic_get_mouse_x(window: GLFWwindow): f32;
extern fn heidic_get_mouse_y(window: GLFWwindow): f32;
```

### Ray Functions

```heidic
extern fn heidic_get_mouse_ray_origin(window: GLFWwindow): Vec3;
extern fn heidic_get_mouse_ray_dir(window: GLFWwindow): Vec3;
```

### Raycasting Functions

```heidic
// Test if mouse ray hits a cube
extern fn heidic_raycast_cube_hit(
    window: GLFWwindow,
    cubeX: f32, cubeY: f32, cubeZ: f32,
    cubeSx: f32, cubeSy: f32, cubeSz: f32
): i32;  // Returns 1 if hit, 0 if miss

// Get hit point if ray hits cube
extern fn heidic_raycast_cube_hit_point(
    window: GLFWwindow,
    cubeX: f32, cubeY: f32, cubeZ: f32,
    cubeSx: f32, cubeSy: f32, cubeSz: f32
): Vec3;  // Returns hit point in world space
```

## Notes

- Camera matrices are automatically updated when `heidic_update_camera()` is called
- Raycasting uses the current frame's camera matrices
- All coordinates are in world space
- Cube raycasting assumes axis-aligned bounding boxes (no rotation support yet)

