#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <cstdint>

// EDEN ENGINE Standard Library
#include "stdlib/glfw.h"
#include "stdlib/math.h"

// Frame-Scoped Memory Allocator (FrameArena)
// Automatically frees all allocations at frame end
class FrameArena {
private:
    struct Block {
        void* ptr;
        size_t size;
    };
    std::vector<Block> blocks;
    size_t current_offset;
    static constexpr size_t BLOCK_SIZE = 1024 * 1024; // 1MB blocks
    std::vector<uint8_t> current_block;

public:
    FrameArena() : current_offset(0) {
        current_block.resize(BLOCK_SIZE);
    }

    ~FrameArena() {
        // All memory is automatically freed when blocks go out of scope
    }

    template<typename T>
    std::vector<T> alloc_array(size_t count) {
        size_t size_needed = count * sizeof(T);
        size_t aligned_size = (size_needed + alignof(T) - 1) & ~(alignof(T) - 1);
        
        // Check if we need a new block
        if (current_offset + aligned_size > current_block.size()) {
            // Save current block
            blocks.push_back({current_block.data(), current_block.size()});
            // Allocate new block
            current_block.resize(BLOCK_SIZE);
            current_offset = 0;
        }
        
        // Allocate from current block
        void* ptr = current_block.data() + current_offset;
        current_offset += aligned_size;
        
        // Construct vector with custom allocator that uses our memory
        std::vector<T> result;
        result.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            new (static_cast<T*>(ptr) + i) T();
            result.push_back(*static_cast<T*>(ptr) + i);
        }
        
        return result;
    }

    void reset() {
        // Reset for next frame
        blocks.clear();
        current_offset = 0;
        current_block.resize(BLOCK_SIZE);
    }
};


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
    void heidic_update_camera_with_far(float px, float py, float pz, float rx, float ry, float rz, float far_plane);
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
    float heidic_get_fps();
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
extern "C" {
    Vec3 heidic_vec3(float x, float y, float z);
}
extern "C" {
    Vec3 heidic_vec3_add(Vec3 a, Vec3 b);
}
extern "C" {
    Vec3 heidic_vec_copy(Vec3 src);
}
extern "C" {
    Vec3 heidic_attach_camera_translation(Vec3 player_translation);
}
extern "C" {
    Vec3 heidic_attach_camera_rotation(Vec3 player_rotation);
}

int heidic_main();

int heidic_main() {
        std::cout << "Initializing GLFW...\n" << std::endl;
        if ((heidic_glfw_init() == 0)) {
            return 0;
        }
        heidic_glfw_vulkan_hints();
        GLFWwindow* window = heidic_create_window(1280, 720, "EDEN ENGINE - FPS Camera");
        if ((heidic_init_renderer(window) == 0)) {
            heidic_glfw_terminate();
            return 0;
        }
        Vec3 player_pos = heidic_vec3(0, 0, 0);
        Vec3 player_rot = heidic_vec3(0, 0, 0);
        Vec3 camera_pos = heidic_vec3(0, 100, 0);
        Vec3 camera_rot = heidic_vec3(0, 0, 0);
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
        int32_t camera_mode = 1;
        int32_t c_was_pressed = 0;
        Vec3 topdown_cam_pos = heidic_vec3(0, 10000, 0);
        Vec3 topdown_cam_rot = heidic_vec3(-90, 0, 0);
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
            int32_t c_is_pressed = heidic_is_key_pressed(window, 67);
            if ((c_is_pressed == 1)) {
                if ((c_was_pressed == 0)) {
                    if ((camera_mode == 0)) {
                        camera_mode = 1;
                    } else {
                        camera_mode = 0;
                    }
                    c_was_pressed = 1;
                }
            } else {
                c_was_pressed = 0;
            }
            float rot_y_rad = heidic_convert_degrees_to_radians(player_rot.y);
            float forward_x = -(heidic_sin(rot_y_rad));
            float forward_z = -(heidic_cos(rot_y_rad));
            float right_x = heidic_cos(rot_y_rad);
            float right_z = -(heidic_sin(rot_y_rad));
            if ((heidic_is_key_pressed(window, 87) == 1)) {
                player_pos.x = (player_pos.x + (forward_x * move_speed));
                player_pos.z = (player_pos.z + (forward_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 83) == 1)) {
                player_pos.x = (player_pos.x - (forward_x * move_speed));
                player_pos.z = (player_pos.z - (forward_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 65) == 1)) {
                player_pos.x = (player_pos.x - (right_x * move_speed));
                player_pos.z = (player_pos.z - (right_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 68) == 1)) {
                player_pos.x = (player_pos.x + (right_x * move_speed));
                player_pos.z = (player_pos.z + (right_z * move_speed));
            }
            if ((heidic_is_key_pressed(window, 81) == 1)) {
                player_rot.y = (player_rot.y + rot_speed);
            }
            if ((heidic_is_key_pressed(window, 69) == 1)) {
                player_rot.y = (player_rot.y - rot_speed);
            }
            if ((camera_mode == 1)) {
                Vec3 offset = heidic_vec3(0, 100, 0);
                Vec3 offset_pos = heidic_vec3_add(player_pos, offset);
                camera_pos = heidic_attach_camera_translation(offset_pos);
                camera_rot = heidic_attach_camera_rotation(player_rot);
            } else {
                camera_pos = topdown_cam_pos;
                camera_rot = topdown_cam_rot;
            }
            cube_x = player_pos.x;
            cube_y = player_pos.y;
            cube_z = player_pos.z;
            cube_rx = player_rot.x;
            cube_ry = player_rot.y;
            cube_rz = player_rot.z;
            heidic_begin_frame();
            if ((camera_mode == 0)) {
                heidic_update_camera_with_far(camera_pos.x, camera_pos.y, camera_pos.z, camera_rot.x, camera_rot.y, camera_rot.z, 50000);
            } else {
                heidic_update_camera(camera_pos.x, camera_pos.y, camera_pos.z, camera_rot.x, camera_rot.y, camera_rot.z);
            }
            heidic_draw_cube(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, cube_sx, cube_sy, cube_sz);
            heidic_draw_model_origin(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, 100);
            float cube_spacing = 2000;
            float cube_height = 1000;
            float cube_size = 200;
            heidic_draw_cube(-(cube_spacing), (cube_height / 2), -(cube_spacing), 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(0, (cube_height / 2), -(cube_spacing), 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(cube_spacing, (cube_height / 2), -(cube_spacing), 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(-(cube_spacing), (cube_height / 2), 0, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(0, (cube_height / 2), 0, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(cube_spacing, (cube_height / 2), 0, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(-(cube_spacing), (cube_height / 2), cube_spacing, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(0, (cube_height / 2), cube_spacing, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube(cube_spacing, (cube_height / 2), cube_spacing, 0, 0, 0, cube_size, cube_height, cube_size);
            heidic_draw_cube((cube_spacing * 2), (cube_height / 2), 0, 0, 0, 0, cube_size, cube_height, cube_size);
            if ((show_debug == 1)) {
                heidic_imgui_begin("Debug Panel (F1 to Toggle)");
                float fps = heidic_get_fps();
                heidic_imgui_text_float("FPS", fps);
                heidic_imgui_text("Camera Mode (C to Toggle): ");
                if ((camera_mode == 0)) {
                    heidic_imgui_text("Top-Down");
                } else {
                    heidic_imgui_text("FPS");
                }
                heidic_imgui_text("Camera Transform (1 unit = 1 cm)");
                camera_pos.x = heidic_imgui_drag_float("Cam X", camera_pos.x, 1);
                camera_pos.y = heidic_imgui_drag_float("Cam Y", camera_pos.y, 1);
                camera_pos.z = heidic_imgui_drag_float("Cam Z", camera_pos.z, 1);
                camera_rot.x = heidic_imgui_drag_float("Cam Rot X", camera_rot.x, 1);
                camera_rot.y = heidic_imgui_drag_float("Cam Rot Y", camera_rot.y, 1);
                camera_rot.z = heidic_imgui_drag_float("Cam Rot Z", camera_rot.z, 1);
                heidic_imgui_text("Player Transform");
                player_pos.x = heidic_imgui_drag_float("Player X", player_pos.x, 1);
                player_pos.y = heidic_imgui_drag_float("Player Y", player_pos.y, 1);
                player_pos.z = heidic_imgui_drag_float("Player Z", player_pos.z, 1);
                player_rot.x = heidic_imgui_drag_float("Player Rot X", player_rot.x, 1);
                player_rot.y = heidic_imgui_drag_float("Player Rot Y", player_rot.y, 1);
                player_rot.z = heidic_imgui_drag_float("Player Rot Z", player_rot.z, 1);
                cube_sx = heidic_imgui_drag_float("Cube Scale X", cube_sx, 1);
                cube_sy = heidic_imgui_drag_float("Cube Scale Y", cube_sy, 1);
                cube_sz = heidic_imgui_drag_float("Cube Scale Z", cube_sz, 1);
                heidic_imgui_text("Direction Vectors (W moves in Forward direction)");
                float debug_rot_y_rad = heidic_convert_degrees_to_radians(player_rot.y);
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
