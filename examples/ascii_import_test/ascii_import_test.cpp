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
    void heidic_update_camera(float px, float py, float pz, float rx, float ry, float rz);
}
extern "C" {
    int32_t heidic_load_ascii_model(const char* filename);
}
extern "C" {
    void heidic_draw_mesh(int32_t mesh_id, float x, float y, float z, float rx, float ry, float rz);
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

int heidic_main();

int heidic_main() {
        std::cout << "Initializing GLFW...\n" << std::endl;
        if ((heidic_glfw_init() == 0)) {
            return 0;
        }
        heidic_glfw_vulkan_hints();
        GLFWwindow* window = heidic_create_window(1280, 720, "EDEN ENGINE - ASCII Import Test");
        if ((heidic_init_renderer(window) == 0)) {
            heidic_glfw_terminate();
            return 0;
        }
        std::cout << "Loading model...\n" << std::endl;
        int32_t mesh_id = heidic_load_ascii_model("../models/test.txt");
        if ((mesh_id < 0)) {
            std::cout << "Failed to load model!\n" << std::endl;
            heidic_cleanup_renderer();
            heidic_destroy_window(window);
            heidic_glfw_terminate();
            return 0;
        }
        std::cout << "Model loaded successfully!\n" << std::endl;
        float cam_x = 0;
        float cam_y = 0;
        float cam_z = 300;
        float cam_rx = 0;
        float cam_ry = 0;
        float cam_rz = 0;
        float model_x = 0;
        float model_y = 0;
        float model_z = 0;
        float model_rx = 0;
        float model_ry = 0;
        float model_rz = 0;
        float rot_speed = 1;
        std::cout << "Starting loop...\n" << std::endl;
        while ((heidic_window_should_close(window) == 0)) {
            heidic_poll_events();
            if ((heidic_is_key_pressed(window, 256) == 1)) {
                heidic_set_window_should_close(window, 1);
            }
            model_ry = (model_ry + rot_speed);
            heidic_begin_frame();
            heidic_update_camera(cam_x, cam_y, cam_z, cam_rx, cam_ry, cam_rz);
            heidic_draw_mesh(mesh_id, model_x, model_y, model_z, model_rx, model_ry, model_rz);
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
