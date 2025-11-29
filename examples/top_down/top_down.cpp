#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <cstdint>

// EDEN ENGINE Standard Library
#include "stdlib/vulkan.h"
#include "stdlib/glfw.h"
#include "stdlib/math.h"
#include "stdlib/eden_imgui.h"

extern "C" {
    void heidic_glfw_vulkan_hints();
}
extern "C" {
    int32_t heidic_init_renderer(GLFWwindow* window);
}
extern "C" {
    void heidic_cleanup_renderer();
}
extern "C" {
    int32_t heidic_window_should_close(GLFWwindow* window);
}
extern "C" {
    void heidic_poll_events();
}
extern "C" {
    int32_t heidic_is_key_pressed(GLFWwindow* window, int32_t key);
}
extern "C" {
    void heidic_begin_frame();
}
extern "C" {
    void heidic_end_frame();
}
extern "C" {
    void heidic_draw_cube(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
}
extern "C" {
    void heidic_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b);
}
extern "C" {
    void heidic_draw_model_origin(float x, float y, float z, float rx, float ry, float rz, float length);
}
extern "C" {
    void heidic_update_camera(float px, float py, float pz, float rx, float ry, float rz);
}
extern "C" {
    void heidic_imgui_begin(const char* name);
}
extern "C" {
    void heidic_imgui_end();
}
extern "C" {
    void heidic_imgui_text(const char* text);
}
extern "C" {
    void heidic_imgui_text_float(const char* label, float value);
}
extern "C" {
    float heidic_imgui_drag_float(const char* label, float v, float speed);
}
extern "C" {
    int32_t heidic_glfw_init();
}
extern "C" {
    GLFWwindow* heidic_create_window(int32_t width, int32_t height, const char* title);
}
extern "C" {
    void heidic_glfw_terminate();
}
extern "C" {
    void heidic_destroy_window(GLFWwindow* window);
}
extern "C" {
    void heidic_set_window_should_close(GLFWwindow* window, int32_t value);
}
extern "C" {
    int32_t heidic_get_key(GLFWwindow* window, int32_t key);
}
extern "C" {
    float heidic_convert_degrees_to_radians(float degrees);
}
extern "C" {
    float heidic_sin(float radians);
}
extern "C" {
    float heidic_cos(float radians);
}

int heidic_main();

int heidic_main() {
        std::cout << "Initializing GLFW...\n" << std::endl;
        if ((heidic_glfw_init() == 0)) {
            return 0;
        }
        heidic_glfw_vulkan_hints();
        GLFWwindow* window = heidic_create_window(1280, 720, "EDEN ENGINE - Top Down");
        if ((heidic_init_renderer(window) == 0)) {
            heidic_glfw_terminate();
            return 0;
        }
        float cam_x = 0;
        float cam_y = 1000;
        float cam_z = 0;
        float cam_rx = -90;
        float cam_ry = 0;
        float cam_rz = 0;
        float cube_x = 0;
        float cube_y = 0;
        float cube_z = 0;
        float cube_rx = 0;
        float cube_ry = 0;
        float cube_rz = 0;
        float cube_sx = 100;
        float cube_sy = 100;
        float cube_sz = 100;
        float move_speed = 5;
        float rot_speed = 2;
        int32_t show_debug = 1;
        int32_t f1_was_pressed = 0;
        std::cout << "Starting loop...\n" << std::endl;
        while ((heidic_window_should_close(window) == 0)) {
            heidic_poll_events();
            if ((heidic_is_key_pressed(window, 256) == 1)) {
                heidic_set_window_should_close(window, 1);
            }
            int32_t f1_is_pressed = heidic_is_key_pressed(window, 290);
            if ((f1_is_pressed == 1)) {
                if ((f1_was_pressed == 0)) {
                    if ((show_debug == 1)) {
                        show_debug = 0;
                    } else {
                        show_debug = 1;
                    }
                    f1_was_pressed = 1;
                }
            } else {
                f1_was_pressed = 0;
            }
            float rot_y_rad = heidic_convert_degrees_to_radians(cube_ry);
            float forward_x = -(heidic_sin(rot_y_rad));
            float forward_z = -(heidic_cos(rot_y_rad));
            float right_x = heidic_cos(rot_y_rad);
            float right_z = -(heidic_sin(rot_y_rad));
            if ((heidic_is_key_pressed(window, 87) == 1)) {
                cube_x = (cube_x + (forward_x * move_speed));
                cube_z = (cube_z + (forward_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 83) == 1)) {
                cube_x = (cube_x - (forward_x * move_speed));
                cube_z = (cube_z - (forward_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 65) == 1)) {
                cube_x = (cube_x - (right_x * move_speed));
                cube_z = (cube_z - (right_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 68) == 1)) {
                cube_x = (cube_x + (right_x * move_speed));
                cube_z = (cube_z + (right_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 81) == 1)) {
                cube_ry = (cube_ry + rot_speed);
            }
            if ((heidic_is_key_pressed(window, 69) == 1)) {
                cube_ry = (cube_ry - rot_speed);
            }
            heidic_begin_frame();
            heidic_update_camera(cam_x, cam_y, cam_z, cam_rx, cam_ry, cam_rz);
            heidic_draw_cube(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, cube_sx, cube_sy, cube_sz);
            heidic_draw_model_origin(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, 100);
            if ((show_debug == 1)) {
                heidic_imgui_begin("Debug Panel (F1 to Toggle)");
                heidic_imgui_text("Camera Transform (1 unit = 1 cm)");
                cam_x = heidic_imgui_drag_float("Cam X", cam_x, 1);
                cam_y = heidic_imgui_drag_float("Cam Y", cam_y, 1);
                cam_z = heidic_imgui_drag_float("Cam Z", cam_z, 1);
                cam_rx = heidic_imgui_drag_float("Cam Rot X", cam_rx, 1);
                cam_ry = heidic_imgui_drag_float("Cam Rot Y", cam_ry, 1);
                heidic_imgui_text("Cube Transform");
                cube_x = heidic_imgui_drag_float("Cube X", cube_x, 1);
                cube_y = heidic_imgui_drag_float("Cube Y", cube_y, 1);
                cube_z = heidic_imgui_drag_float("Cube Z", cube_z, 1);
                cube_rx = heidic_imgui_drag_float("Cube Rot X", cube_rx, 1);
                cube_ry = heidic_imgui_drag_float("Cube Rot Y", cube_ry, 1);
                cube_rz = heidic_imgui_drag_float("Cube Rot Z", cube_rz, 1);
                cube_sx = heidic_imgui_drag_float("Cube Scale X", cube_sx, 1);
                cube_sy = heidic_imgui_drag_float("Cube Scale Y", cube_sy, 1);
                cube_sz = heidic_imgui_drag_float("Cube Scale Z", cube_sz, 1);
                heidic_imgui_text("Direction Vectors (W moves in Forward direction)");
                float debug_rot_y_rad = heidic_convert_degrees_to_radians(cube_ry);
                float debug_forward_x = -(heidic_sin(debug_rot_y_rad));
                float debug_forward_z = -(heidic_cos(debug_rot_y_rad));
                float debug_right_x = heidic_cos(debug_rot_y_rad);
                float debug_right_z = -(heidic_sin(debug_rot_y_rad));
                heidic_imgui_text_float("Forward X", debug_forward_x);
                heidic_imgui_text_float("Forward Z", debug_forward_z);
                heidic_imgui_text_float("Right X", debug_right_x);
                heidic_imgui_text_float("Right Z", debug_right_z);
                heidic_imgui_end();
            }
            heidic_end_frame();
        }
        heidic_cleanup_renderer();
        heidic_destroy_window(window);
        heidic_glfw_terminate();
        return 0;
}

int main(int argc, char* argv[]) {
    heidic_main();
    return 0;
}
