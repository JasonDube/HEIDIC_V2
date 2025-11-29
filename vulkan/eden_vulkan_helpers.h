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

    // Helpers
    void heidic_glfw_vulkan_hints();
    int heidic_init_renderer(GLFWwindow* window);
    void heidic_cleanup_renderer();
    int heidic_window_should_close(GLFWwindow* window);
    void heidic_poll_events();
    int heidic_is_key_pressed(GLFWwindow* window, int key);

    // Frame Control
    void heidic_begin_frame();
    void heidic_end_frame();
    
    // Drawing
    void heidic_draw_cube(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
    void heidic_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b);
    void heidic_draw_model_origin(float x, float y, float z, float rx, float ry, float rz, float length);
    void heidic_update_camera(float px, float py, float pz, float rx, float ry, float rz);

    // ImGui
    void heidic_imgui_init(GLFWwindow* window);
    void heidic_imgui_begin(const char* name);
    void heidic_imgui_end();
    void heidic_imgui_text(const char* text);
    void heidic_imgui_text_float(const char* label, float value);
    bool heidic_imgui_drag_float3(const char* label, Vec3* v, float speed);
    Vec3 heidic_imgui_drag_float3_val(const char* label, Vec3 v, float speed);
    float heidic_imgui_drag_float(const char* label, float v, float speed);
    
    // Math Helpers
    float heidic_convert_degrees_to_radians(float degrees);
    float heidic_convert_radians_to_degrees(float radians);
    float heidic_sin(float radians);
    float heidic_cos(float radians);

    // Utility
    void heidic_sleep_ms(int ms);
}
