#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../stdlib/math.h"

extern "C" {
    // Window/Input Wrappers (to avoid signature conflicts in generated C++)
    int heidic_glfw_init();
    void heidic_glfw_terminate();
    GLFWwindow* heidic_create_window(int width, int height, const char* title);
    void heidic_destroy_window(GLFWwindow* window);
    void heidic_set_window_should_close(GLFWwindow* window, int value);
    int heidic_get_key(GLFWwindow* window, int key);
    void heidic_set_video_mode(int windowed); // 0 = fullscreen, 1 = windowed

    // Helpers
    void heidic_glfw_vulkan_hints();
    int heidic_init_renderer(GLFWwindow* window);
    void heidic_cleanup_renderer();
    int heidic_window_should_close(GLFWwindow* window);
    void heidic_poll_events();
    int heidic_is_key_pressed(GLFWwindow* window, int key);
    int heidic_is_mouse_button_pressed(GLFWwindow* window, int button);

    // Frame Control
    void heidic_begin_frame();
    void heidic_end_frame();
    
    // Drawing
    void heidic_draw_cube(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
    void heidic_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b);
    void heidic_draw_model_origin(float x, float y, float z, float rx, float ry, float rz, float length);
    void heidic_update_camera(float px, float py, float pz, float rx, float ry, float rz);
    void heidic_update_camera_with_far(float px, float py, float pz, float rx, float ry, float rz, float far_plane);
    Camera heidic_create_camera(Vec3 pos, Vec3 rot, float clip_near, float clip_far);
    void heidic_update_camera_from_struct(Camera camera);
    
    // Mesh Loading
    int heidic_load_ascii_model(const char* filename);
    void heidic_draw_mesh(int mesh_id, float x, float y, float z, float rx, float ry, float rz);

    // ImGui
    void heidic_imgui_init(GLFWwindow* window);
    void heidic_imgui_begin(const char* name);
    void heidic_imgui_end();
    void heidic_imgui_text(const char* text);
    void heidic_imgui_text_float(const char* label, float value);
    bool heidic_imgui_drag_float3(const char* label, Vec3* v, float speed);
    Vec3 heidic_imgui_drag_float3_val(const char* label, Vec3 v, float speed);
    float heidic_imgui_drag_float(const char* label, float v, float speed);
    float heidic_get_fps();
    
    // Vector Operations
    Vec3 heidic_vec3(float x, float y, float z);
    Vec3 heidic_vec3_add(Vec3 a, Vec3 b);
    Vec3 heidic_vec_copy(Vec3 src);
    Vec3 heidic_attach_camera_translation(Vec3 player_translation);
    Vec3 heidic_attach_camera_rotation(Vec3 player_rotation);
    
    // Math Helpers
    float heidic_convert_degrees_to_radians(float degrees);
    float heidic_convert_radians_to_degrees(float radians);
    float heidic_sin(float radians);
    float heidic_cos(float radians);

    // Utility
    void heidic_sleep_ms(int ms);
    
    // Raycasting
    float heidic_get_mouse_x(GLFWwindow* window);
    float heidic_get_mouse_y(GLFWwindow* window);
    float heidic_get_mouse_scroll_y(GLFWwindow* window); // Get vertical scroll delta
    Vec3 heidic_get_mouse_ray_origin(GLFWwindow* window);
    Vec3 heidic_get_mouse_ray_dir(GLFWwindow* window);
    int heidic_raycast_cube_hit(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz);
    Vec3 heidic_raycast_cube_hit_point(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz);
    void heidic_draw_cube_wireframe(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
    void heidic_draw_ground_plane(float size, float r, float g, float b);
    int heidic_raycast_ground_hit(float x, float y, float z, float maxDistance);
    Vec3 heidic_raycast_ground_hit_point(float x, float y, float z, float maxDistance);
    
    // Debug: Print raycast info to console
    void heidic_debug_print_ray(GLFWwindow* window);
    // Draw mouse ray for visual debugging
    void heidic_draw_ray(GLFWwindow* window, float length, float r, float g, float b);

    // Gizmos
    Vec3 heidic_gizmo_translate(GLFWwindow* window, float x, float y, float z);
    int heidic_gizmo_is_interacting();
    
    // Dynamic Cube Storage System
    int heidic_create_cube(float x, float y, float z, float sx, float sy, float sz);
    int heidic_get_cube_count();  // Returns count of active cubes
    int heidic_get_cube_total_count();  // Returns total count (including deleted)
    float heidic_get_cube_x(int index);
    float heidic_get_cube_y(int index);
    float heidic_get_cube_z(int index);
    float heidic_get_cube_sx(int index);
    float heidic_get_cube_sy(int index);
    float heidic_get_cube_sz(int index);
    int heidic_get_cube_active(int index);
    void heidic_set_cube_pos(int index, float x, float y, float z);
    void heidic_set_cube_pos_f(float index, float x, float y, float z);  // Float index version
    void heidic_delete_cube(int index);
    int heidic_find_next_active_cube_index(int start_index);  // Returns -1 if no more
    float heidic_int_to_float(int value);  // Convert i32 to f32
}
