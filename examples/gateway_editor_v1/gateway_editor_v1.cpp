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
    int32_t heidic_glfw_init();
}
extern "C" {
    void heidic_glfw_terminate();
}
extern "C" {
    GLFWwindow* heidic_create_window(int32_t width, int32_t height, const char* title);
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
    void heidic_set_video_mode(int32_t windowed);
}
extern "C" {
    void heidic_glfw_vulkan_hints();
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
    int32_t heidic_is_mouse_button_pressed(GLFWwindow* window, int32_t button);
}
extern "C" {
    int32_t heidic_init_renderer(GLFWwindow* window);
}
extern "C" {
    void heidic_cleanup_renderer();
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
    void heidic_draw_cube_grey(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
}
extern "C" {
    void heidic_draw_cube_blue(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
}
extern "C" {
    void heidic_draw_cube_colored(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
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
    Camera heidic_create_camera(Vec3 pos, Vec3 rot, float clip_near, float clip_far);
}
extern "C" {
    void heidic_update_camera_from_struct(Camera camera);
}
extern "C" {
    int32_t heidic_load_ascii_model(const char* filename);
}
extern "C" {
    void heidic_draw_mesh(int32_t mesh_id, float x, float y, float z, float rx, float ry, float rz);
}
extern "C" {
    void heidic_imgui_init(GLFWwindow* window);
}
extern "C" {
    void heidic_imgui_begin(const char* name);
}
extern "C" {
    void heidic_imgui_begin_docked_with(const char* name, const char* dock_with_name);
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
    int32_t heidic_imgui_begin_main_menu_bar();
}
extern "C" {
    void heidic_imgui_end_main_menu_bar();
}
extern "C" {
    void heidic_imgui_setup_dockspace();
}
extern "C" {
    void heidic_imgui_load_layout(const char* ini_path);
}
extern "C" {
    void heidic_imgui_save_layout(const char* ini_path);
}
extern "C" {
    int32_t heidic_imgui_begin_menu(const char* label);
}
extern "C" {
    void heidic_imgui_end_menu();
}
extern "C" {
    int32_t heidic_imgui_menu_item(const char* label);
}
extern "C" {
    void heidic_imgui_separator();
}
extern "C" {
    int32_t heidic_imgui_button(const char* label);
}
extern "C" {
    int32_t heidic_save_level_str_wrapper(const char* filepath);
}
extern "C" {
    int32_t heidic_load_level_str_wrapper(const char* filepath);
}
extern "C" {
    int32_t heidic_show_save_dialog();
}
extern "C" {
    int32_t heidic_show_open_dialog();
}
extern "C" {
    float heidic_get_fps();
}
extern "C" {
    float heidic_convert_degrees_to_radians(float degrees);
}
extern "C" {
    float heidic_convert_radians_to_degrees(float radians);
}
extern "C" {
    float heidic_sin(float radians);
}
extern "C" {
    float heidic_cos(float radians);
}
extern "C" {
    float heidic_atan2(float y, float x);
}
extern "C" {
    float heidic_asin(float value);
}
extern "C" {
    Vec3 heidic_vec3(float x, float y, float z);
}
extern "C" {
    Vec3 heidic_vec3_add(Vec3 a, Vec3 b);
}
extern "C" {
    Vec3 heidic_vec3_sub(Vec3 a, Vec3 b);
}
extern "C" {
    float heidic_vec3_distance(Vec3 a, Vec3 b);
}
extern "C" {
    Vec3 heidic_vec3_mul_scalar(Vec3 v, float s);
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
extern "C" {
    void heidic_sleep_ms(int32_t ms);
}
extern "C" {
    float heidic_get_mouse_x(GLFWwindow* window);
}
extern "C" {
    float heidic_get_mouse_y(GLFWwindow* window);
}
extern "C" {
    float heidic_get_mouse_scroll_y(GLFWwindow* window);
}
extern "C" {
    float heidic_get_mouse_delta_x(GLFWwindow* window);
}
extern "C" {
    float heidic_get_mouse_delta_y(GLFWwindow* window);
}
extern "C" {
    void heidic_set_cursor_mode(GLFWwindow* window, int32_t mode);
}
extern "C" {
    Vec3 heidic_get_mouse_ray_origin(GLFWwindow* window);
}
extern "C" {
    Vec3 heidic_get_mouse_ray_dir(GLFWwindow* window);
}
extern "C" {
    int32_t heidic_raycast_cube_hit(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz);
}
extern "C" {
    Vec3 heidic_raycast_cube_hit_point(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz);
}
extern "C" {
    void heidic_draw_cube_wireframe(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
}
extern "C" {
    void heidic_draw_ground_plane(float size, float r, float g, float b);
}
extern "C" {
    int32_t heidic_raycast_ground_hit(float x, float y, float z, float maxDistance);
}
extern "C" {
    Vec3 heidic_raycast_ground_hit_point(float x, float y, float z, float maxDistance);
}
extern "C" {
    void heidic_debug_print_ray(GLFWwindow* window);
}
extern "C" {
    void heidic_draw_ray(GLFWwindow* window, float length, float r, float g, float b);
}
extern "C" {
    Vec3 heidic_gizmo_translate(GLFWwindow* window, float x, float y, float z);
}
extern "C" {
    int32_t heidic_gizmo_is_interacting();
}
extern "C" {
    int32_t heidic_create_cube(float x, float y, float z, float sx, float sy, float sz);
}
extern "C" {
    int32_t heidic_create_cube_with_color(float x, float y, float z, float sx, float sy, float sz, float r, float g, float b);
}
extern "C" {
    int32_t heidic_get_cube_count();
}
extern "C" {
    int32_t heidic_get_cube_total_count();
}
extern "C" {
    float heidic_get_cube_x(int32_t index);
}
extern "C" {
    float heidic_get_cube_y(int32_t index);
}
extern "C" {
    float heidic_get_cube_z(int32_t index);
}
extern "C" {
    float heidic_get_cube_sx(int32_t index);
}
extern "C" {
    float heidic_get_cube_sy(int32_t index);
}
extern "C" {
    float heidic_get_cube_sz(int32_t index);
}
extern "C" {
    float heidic_get_cube_r(int32_t index);
}
extern "C" {
    float heidic_get_cube_g(int32_t index);
}
extern "C" {
    float heidic_get_cube_b(int32_t index);
}
extern "C" {
    int32_t heidic_get_cube_active(int32_t index);
}
extern "C" {
    void heidic_set_cube_pos(int32_t index, float x, float y, float z);
}
extern "C" {
    void heidic_set_cube_pos_f(float index, float x, float y, float z);
}
extern "C" {
    void heidic_delete_cube(int32_t index);
}
extern "C" {
    int32_t heidic_find_next_active_cube_index(int32_t start_index);
}
extern "C" {
    float heidic_int_to_float(int32_t value);
}
extern "C" {
    int32_t heidic_float_to_int(float value);
}
extern "C" {
    float heidic_random_float();
}

int heidic_main();

int heidic_main() {
        int32_t VISIBLE = 1;
        int32_t INVISIBLE = 0;
        std::cout << "Initializing GLFW...\n" << std::endl;
        if ((heidic_glfw_init() == 0)) {
            return 0;
        }
        heidic_glfw_vulkan_hints();
        GLFWwindow* window = heidic_create_window(1280, 720, "EDEN ENGINE - Gateway Editor v1");
        if ((heidic_init_renderer(window) == 0)) {
            heidic_glfw_terminate();
            return 0;
        }
        heidic_imgui_load_layout("");
        Vec3 player_pos = heidic_vec3(3000, 100, 3000);
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
        float mouse_sensitivity = 0.1;
        float pitch_max = 90;
        float pitch_min = -90;
        int32_t show_debug = 1;
        int32_t f1_was_pressed = 0;
        int32_t camera_mode = 0;
        int32_t c_was_pressed = 0;
        int32_t player_cube_visible = VISIBLE;
        int32_t mouse_mode = 1;
        int32_t mouse_mode_left_was_pressed = 0;
        int32_t mouse_mode_right_was_pressed = 0;
        heidic_set_cursor_mode(window, 0);
        int32_t show_grid = 1;
        int32_t g_was_pressed = 0;
        int32_t video_mode = 1;
        int32_t shift_enter_was_pressed = 0;
        float selected_cube_x = 0;
        float selected_cube_y = 0;
        float selected_cube_z = 0;
        float selected_cube_sx = 0;
        float selected_cube_sy = 0;
        float selected_cube_sz = 0;
        int32_t has_selection = 0;
        float selected_cube_index = -1;
        int32_t is_grounded = 0;
        int32_t mouse_left_was_pressed = 0;
        int32_t mouse_middle_was_pressed = 0;
        int32_t delete_was_pressed = 0;
        float topdown_cam_height = 10000;
        float topdown_cam_pan_x = 0;
        float topdown_cam_pan_z = 0;
        Vec3 topdown_cam_pos = heidic_vec3(0, topdown_cam_height, 0);
        Vec3 topdown_cam_rot = heidic_vec3(-90, 0, 0);
        float dolly_orbit_azimuth = 0;
        float dolly_orbit_elevation = 45;
        float dolly_orbit_distance = 2000;
        int32_t in_orbit_mode = 0;
        int32_t alt_was_pressed = 0;
        int32_t space_was_pressed = 0;
        float created_cube_size = 200;
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
                        if ((mouse_mode == 0)) {
                            heidic_set_cursor_mode(window, 2);
                        } else {
                            heidic_set_cursor_mode(window, 0);
                        }
                    } else {
                        camera_mode = 0;
                        if ((mouse_mode == 0)) {
                            heidic_set_cursor_mode(window, 1);
                        } else {
                            heidic_set_cursor_mode(window, 0);
                        }
                    }
                    c_was_pressed = 1;
                }
            } else {
                c_was_pressed = 0;
            }
            int32_t left_alt_pressed = heidic_is_key_pressed(window, 342);
            int32_t right_alt_pressed = heidic_is_key_pressed(window, 346);
            int32_t alt_pressed = 0;
            if ((left_alt_pressed == 1)) {
                alt_pressed = 1;
            }
            if ((right_alt_pressed == 1)) {
                alt_pressed = 1;
            }
            if (((alt_pressed == 1) && (mouse_mode == 0))) {
                if ((has_selection == 1)) {
                    if ((alt_was_pressed == 0)) {
                        Vec3 target_pos = heidic_vec3(selected_cube_x, selected_cube_y, selected_cube_z);
                        Vec3 current_cam_pos = camera_pos;
                        Vec3 to_camera = heidic_vec3_sub(current_cam_pos, target_pos);
                        float dist = heidic_vec3_distance(current_cam_pos, target_pos);
                        if ((dist > 0.001)) {
                            float dir_x = (to_camera.x / dist);
                            float dir_y = (to_camera.y / dist);
                            float dir_z = (to_camera.z / dist);
                            float azimuth_rad = heidic_atan2(dir_x, dir_z);
                            dolly_orbit_azimuth = heidic_convert_radians_to_degrees(azimuth_rad);
                            float elevation_rad = heidic_asin(dir_y);
                            dolly_orbit_elevation = heidic_convert_radians_to_degrees(elevation_rad);
                            if (((dist > 10) && (dist < 100000))) {
                                dolly_orbit_distance = dist;
                            }
                        }
                    }
                    float dolly_delta_x = heidic_get_mouse_delta_x(window);
                    float dolly_delta_y = heidic_get_mouse_delta_y(window);
                    float orbit_sensitivity = 0.3;
                    dolly_orbit_azimuth = (dolly_orbit_azimuth - (dolly_delta_x * orbit_sensitivity));
                    dolly_orbit_elevation = (dolly_orbit_elevation + (dolly_delta_y * orbit_sensitivity));
                    if ((dolly_orbit_elevation > 89)) {
                        dolly_orbit_elevation = 89;
                    }
                    if ((dolly_orbit_elevation < -89)) {
                        dolly_orbit_elevation = -89;
                    }
                    Vec3 target_pos = heidic_vec3(selected_cube_x, selected_cube_y, selected_cube_z);
                    float azimuth_rad = heidic_convert_degrees_to_radians(dolly_orbit_azimuth);
                    float elevation_rad = heidic_convert_degrees_to_radians(dolly_orbit_elevation);
                    float cos_elev = heidic_cos(elevation_rad);
                    float sin_elev = heidic_sin(elevation_rad);
                    float cos_azim = heidic_cos(azimuth_rad);
                    float sin_azim = heidic_sin(azimuth_rad);
                    float offset_x = ((dolly_orbit_distance * cos_elev) * sin_azim);
                    float offset_y = (dolly_orbit_distance * sin_elev);
                    float offset_z = ((dolly_orbit_distance * cos_elev) * cos_azim);
                    Vec3 offset = heidic_vec3(offset_x, offset_y, offset_z);
                    Vec3 new_cam_pos = heidic_vec3_add(target_pos, offset);
                    in_orbit_mode = 1;
                    if ((camera_mode == 0)) {
                        topdown_cam_pos = new_cam_pos;
                        camera_pos = new_cam_pos;
                        Vec3 to_target = heidic_vec3_sub(target_pos, new_cam_pos);
                        float dist_to_target = heidic_vec3_distance(new_cam_pos, target_pos);
                        if ((dist_to_target > 0.001)) {
                            float dir_x = (to_target.x / dist_to_target);
                            float dir_y = (to_target.y / dist_to_target);
                            float dir_z = (to_target.z / dist_to_target);
                            float yaw_rad = heidic_atan2(dir_x, dir_z);
                            float yaw_deg = heidic_convert_radians_to_degrees(yaw_rad);
                            float pitch_rad = heidic_asin(dir_y);
                            float pitch_deg = heidic_convert_radians_to_degrees(pitch_rad);
                            topdown_cam_rot.x = pitch_deg;
                            topdown_cam_rot.y = yaw_deg;
                            topdown_cam_rot.z = 0;
                        } else {
                            topdown_cam_rot.x = -(dolly_orbit_elevation);
                            topdown_cam_rot.y = (dolly_orbit_azimuth + 180);
                            topdown_cam_rot.z = 0;
                        }
                        camera_rot = topdown_cam_rot;
                    } else {
                        player_pos = new_cam_pos;
                        Vec3 to_target = heidic_vec3_sub(target_pos, new_cam_pos);
                        float dist_to_target = heidic_vec3_distance(new_cam_pos, target_pos);
                        if ((dist_to_target > 0.001)) {
                            float dir_x = (to_target.x / dist_to_target);
                            float dir_y = (to_target.y / dist_to_target);
                            float dir_z = (to_target.z / dist_to_target);
                            float yaw_rad = heidic_atan2(dir_x, dir_z);
                            float yaw_deg = heidic_convert_radians_to_degrees(yaw_rad);
                            float pitch_rad = heidic_asin(dir_y);
                            float pitch_deg = heidic_convert_radians_to_degrees(pitch_rad);
                            player_rot.y = yaw_deg;
                            player_rot.x = pitch_deg;
                        } else {
                            player_rot.y = (dolly_orbit_azimuth + 180);
                            player_rot.x = -(dolly_orbit_elevation);
                        }
                    }
                } else {
                    float dolly_delta_x = heidic_get_mouse_delta_x(window);
                    float dolly_delta_y = heidic_get_mouse_delta_y(window);
                    float dolly_speed = 20;
                    if ((camera_mode == 0)) {
                        float yaw_rad = heidic_convert_degrees_to_radians(topdown_cam_rot.y);
                        float forward_x = heidic_sin(yaw_rad);
                        float forward_z = heidic_cos(yaw_rad);
                        float right_x = heidic_cos(yaw_rad);
                        float right_z = -(heidic_sin(yaw_rad));
                        float move_x = (((right_x * dolly_delta_x) * dolly_speed) + ((forward_x * dolly_delta_y) * dolly_speed));
                        float move_z = (((right_z * dolly_delta_x) * dolly_speed) + ((forward_z * dolly_delta_y) * dolly_speed));
                        topdown_cam_pan_x = (topdown_cam_pan_x + move_x);
                        topdown_cam_pan_z = (topdown_cam_pan_z + move_z);
                    } else {
                        float yaw_rad = heidic_convert_degrees_to_radians(player_rot.y);
                        float forward_x = heidic_sin(yaw_rad);
                        float forward_z = heidic_cos(yaw_rad);
                        float right_x = heidic_cos(yaw_rad);
                        float right_z = -(heidic_sin(yaw_rad));
                        float move_x = (((right_x * dolly_delta_x) * dolly_speed) + ((forward_x * dolly_delta_y) * dolly_speed));
                        float move_z = (((right_z * dolly_delta_x) * dolly_speed) + ((forward_z * dolly_delta_y) * dolly_speed));
                        player_pos.x = (player_pos.x + move_x);
                        player_pos.z = (player_pos.z + move_z);
                    }
                }
                alt_was_pressed = 1;
            } else {
                alt_was_pressed = 0;
            }
            if ((((camera_mode == 1) && (mouse_mode == 0)) && (alt_pressed == 0))) {
                float mouse_delta_x = heidic_get_mouse_delta_x(window);
                float mouse_delta_y = heidic_get_mouse_delta_y(window);
                player_rot.y = (player_rot.y - (mouse_delta_x * mouse_sensitivity));
                player_rot.x = (player_rot.x - (mouse_delta_y * mouse_sensitivity));
                if ((player_rot.x > pitch_max)) {
                    player_rot.x = pitch_max;
                }
                float pitch_check = (pitch_min - player_rot.x);
                if ((pitch_check > 0)) {
                    player_rot.x = pitch_min;
                }
            }
            int32_t g_is_pressed = heidic_is_key_pressed(window, 71);
            if ((g_is_pressed == 1)) {
                if ((g_was_pressed == 0)) {
                    if ((show_grid == 1)) {
                        show_grid = 0;
                    } else {
                        show_grid = 1;
                    }
                    g_was_pressed = 1;
                }
            } else {
                g_was_pressed = 0;
            }
            int32_t space_is_pressed = heidic_is_key_pressed(window, 32);
            if ((space_is_pressed == 0)) {
                space_is_pressed = heidic_is_key_pressed(window, 57);
            }
            if ((space_is_pressed == 1)) {
                if ((space_was_pressed == 0)) {
                    std::cout << "SPACEBAR PRESSED! Creating cube at ray hit point\n" << std::endl;
                    float default_cube_size = 100;
                    Vec3 create_ray_origin = heidic_get_mouse_ray_origin(window);
                    Vec3 create_ray_dir = heidic_get_mouse_ray_dir(window);
                    Vec3 create_pos = heidic_vec3(0, 0, 0);
                    int32_t found_hit = 0;
                    float closest_dist = 100000000000;
                    float hit_cube_x = 0;
                    float hit_cube_y = 0;
                    float hit_cube_z = 0;
                    float hit_cube_sx = 0;
                    float hit_cube_sy = 0;
                    float hit_cube_sz = 0;
                    Vec3 hit_point = heidic_vec3(0, 0, 0);
                    int32_t is_ground_plane = 0;
                    float ground_cube_x = 0;
                    float ground_cube_y = -500;
                    float ground_cube_z = 0;
                    float ground_cube_sx = 10000;
                    float ground_cube_sy = 100;
                    float ground_cube_sz = 10000;
                    int32_t ground_cube_hit = heidic_raycast_cube_hit(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                    if ((ground_cube_hit == 1)) {
                        Vec3 ground_cube_hit_point = heidic_raycast_cube_hit_point(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                        float ground_cube_dist = ((((ground_cube_hit_point.x - create_ray_origin.x) * (ground_cube_hit_point.x - create_ray_origin.x)) + ((ground_cube_hit_point.y - create_ray_origin.y) * (ground_cube_hit_point.y - create_ray_origin.y))) + ((ground_cube_hit_point.z - create_ray_origin.z) * (ground_cube_hit_point.z - create_ray_origin.z)));
                        if ((ground_cube_dist < closest_dist)) {
                            closest_dist = ground_cube_dist;
                            create_pos = ground_cube_hit_point;
                            hit_point = ground_cube_hit_point;
                            hit_cube_x = ground_cube_x;
                            hit_cube_y = ground_cube_y;
                            hit_cube_z = ground_cube_z;
                            hit_cube_sx = ground_cube_sx;
                            hit_cube_sy = ground_cube_sy;
                            hit_cube_sz = ground_cube_sz;
                            is_ground_plane = 1;
                            found_hit = 1;
                        }
                    }
                    int32_t cube_test_index = 0;
                    int32_t total_cubes_test = heidic_get_cube_total_count();
                    while ((cube_test_index < total_cubes_test)) {
                        if ((heidic_get_cube_active(cube_test_index) == 1)) {
                            float test_cube_x = heidic_get_cube_x(cube_test_index);
                            float test_cube_y = heidic_get_cube_y(cube_test_index);
                            float test_cube_z = heidic_get_cube_z(cube_test_index);
                            float test_cube_sx = heidic_get_cube_sx(cube_test_index);
                            float test_cube_sy = heidic_get_cube_sy(cube_test_index);
                            float test_cube_sz = heidic_get_cube_sz(cube_test_index);
                            int32_t cube_hit = heidic_raycast_cube_hit(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                            if ((cube_hit == 1)) {
                                Vec3 test_hit_point = heidic_raycast_cube_hit_point(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                float dist = ((((test_hit_point.x - create_ray_origin.x) * (test_hit_point.x - create_ray_origin.x)) + ((test_hit_point.y - create_ray_origin.y) * (test_hit_point.y - create_ray_origin.y))) + ((test_hit_point.z - create_ray_origin.z) * (test_hit_point.z - create_ray_origin.z)));
                                if ((dist < closest_dist)) {
                                    closest_dist = dist;
                                    create_pos = test_hit_point;
                                    hit_point = test_hit_point;
                                    hit_cube_x = test_cube_x;
                                    hit_cube_y = test_cube_y;
                                    hit_cube_z = test_cube_z;
                                    hit_cube_sx = test_cube_sx;
                                    hit_cube_sy = test_cube_sy;
                                    hit_cube_sz = test_cube_sz;
                                    is_ground_plane = 0;
                                    found_hit = 1;
                                }
                            }
                        }
                        cube_test_index = (cube_test_index + 1);
                    }
                    if ((found_hit == 1)) {
                        if ((is_ground_plane == 1)) {
                            float hit_top_y = (hit_cube_y + (hit_cube_sy / 2));
                            create_pos.y = (hit_top_y + (default_cube_size / 2));
                            create_pos.x = hit_point.x;
                            create_pos.z = hit_point.z;
                            std::cout << "Hit ground plane! Stacking on top\n" << std::endl;
                        } else {
                            float cube_min_x = (hit_cube_x - (hit_cube_sx / 2));
                            float cube_max_x = (hit_cube_x + (hit_cube_sx / 2));
                            float cube_min_y = (hit_cube_y - (hit_cube_sy / 2));
                            float cube_max_y = (hit_cube_y + (hit_cube_sy / 2));
                            float cube_min_z = (hit_cube_z - (hit_cube_sz / 2));
                            float cube_max_z = (hit_cube_z + (hit_cube_sz / 2));
                            float dist_to_left = (hit_point.x - cube_min_x);
                            float dist_to_right = (cube_max_x - hit_point.x);
                            float dist_to_bottom = (hit_point.y - cube_min_y);
                            float dist_to_top = (cube_max_y - hit_point.y);
                            float dist_to_back = (hit_point.z - cube_min_z);
                            float dist_to_front = (cube_max_z - hit_point.z);
                            float min_dist = dist_to_left;
                            int32_t hit_face = 0;
                            if ((dist_to_right < min_dist)) {
                                min_dist = dist_to_right;
                                hit_face = 1;
                            }
                            if ((dist_to_bottom < min_dist)) {
                                min_dist = dist_to_bottom;
                                hit_face = 2;
                            }
                            if ((dist_to_top < min_dist)) {
                                min_dist = dist_to_top;
                                hit_face = 3;
                            }
                            if ((dist_to_back < min_dist)) {
                                min_dist = dist_to_back;
                                hit_face = 4;
                            }
                            if ((dist_to_front < min_dist)) {
                                min_dist = dist_to_front;
                                hit_face = 5;
                            }
                            if ((hit_face == 0)) {
                                create_pos.x = (cube_min_x - (default_cube_size / 2));
                                create_pos.y = hit_cube_y;
                                create_pos.z = hit_cube_z;
                                std::cout << "Hit left face! Stacking to the left\n" << std::endl;
                            } else {
                                if ((hit_face == 1)) {
                                    create_pos.x = (cube_max_x + (default_cube_size / 2));
                                    create_pos.y = hit_cube_y;
                                    create_pos.z = hit_cube_z;
                                    std::cout << "Hit right face! Stacking to the right\n" << std::endl;
                                } else {
                                    if ((hit_face == 2)) {
                                        create_pos.x = hit_cube_x;
                                        create_pos.y = (cube_min_y - (default_cube_size / 2));
                                        create_pos.z = hit_cube_z;
                                        std::cout << "Hit bottom face! Stacking below\n" << std::endl;
                                    } else {
                                        if ((hit_face == 3)) {
                                            create_pos.x = hit_cube_x;
                                            create_pos.y = (cube_max_y + (default_cube_size / 2));
                                            create_pos.z = hit_cube_z;
                                            std::cout << "Hit top face! Stacking on top\n" << std::endl;
                                        } else {
                                            if ((hit_face == 4)) {
                                                create_pos.x = hit_cube_x;
                                                create_pos.y = hit_cube_y;
                                                create_pos.z = (cube_min_z - (default_cube_size / 2));
                                                std::cout << "Hit back face! Stacking to the back\n" << std::endl;
                                            } else {
                                                create_pos.x = hit_cube_x;
                                                create_pos.y = hit_cube_y;
                                                create_pos.z = (cube_max_z + (default_cube_size / 2));
                                                std::cout << "Hit front face! Stacking to the front\n" << std::endl;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        create_pos = heidic_vec3_add(create_ray_origin, heidic_vec3_mul_scalar(create_ray_dir, 500));
                        std::cout << "No hit, placing cube along ray\n" << std::endl;
                    }
                    float cube_r = heidic_random_float();
                    float cube_g = heidic_random_float();
                    float cube_b = heidic_random_float();
                    int32_t cube_index = heidic_create_cube_with_color(create_pos.x, create_pos.y, create_pos.z, default_cube_size, default_cube_size, default_cube_size, cube_r, cube_g, cube_b);
                    if ((cube_index >= 0)) {
                        std::cout << "Created cube at index: " << std::endl;
                        std::cout << cube_index << std::endl;
                        std::cout << "\n" << std::endl;
                        has_selection = 1;
                        selected_cube_index = (heidic_int_to_float(cube_index) + 2);
                        selected_cube_x = create_pos.x;
                        selected_cube_y = create_pos.y;
                        selected_cube_z = create_pos.z;
                        selected_cube_sx = default_cube_size;
                        selected_cube_sy = default_cube_size;
                        selected_cube_sz = default_cube_size;
                    } else {
                        std::cout << "Failed to create cube\n" << std::endl;
                    }
                    space_was_pressed = 1;
                }
            } else {
                space_was_pressed = 0;
            }
            int32_t delete_is_pressed = heidic_is_key_pressed(window, 261);
            if ((delete_is_pressed == 1)) {
                if ((delete_was_pressed == 0)) {
                    if (((has_selection == 1) && (selected_cube_index >= 2))) {
                        int32_t cube_vector_index = heidic_float_to_int((selected_cube_index - 2));
                        heidic_delete_cube(cube_vector_index);
                        std::cout << "Deleted cube at index " << std::endl;
                        std::cout << "\n" << std::endl;
                        has_selection = 0;
                        selected_cube_index = -1;
                    }
                    delete_was_pressed = 1;
                }
            } else {
                delete_was_pressed = 0;
            }
            int32_t enter_is_pressed = heidic_is_key_pressed(window, 257);
            int32_t left_shift_is_pressed = heidic_is_key_pressed(window, 340);
            int32_t right_shift_is_pressed = heidic_is_key_pressed(window, 344);
            int32_t shift_is_pressed = 0;
            if ((left_shift_is_pressed == 1)) {
                shift_is_pressed = 1;
            }
            if ((right_shift_is_pressed == 1)) {
                shift_is_pressed = 1;
            }
            if (((enter_is_pressed == 1) && (shift_is_pressed == 1))) {
                if ((shift_enter_was_pressed == 0)) {
                    if ((video_mode == 1)) {
                        video_mode = 0;
                        heidic_set_video_mode(0);
                    } else {
                        video_mode = 1;
                        heidic_set_video_mode(1);
                    }
                    shift_enter_was_pressed = 1;
                }
            } else {
                shift_enter_was_pressed = 0;
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
                player_pos.y = (player_pos.y + move_speed);
            }
            if ((heidic_is_key_pressed(window, 69) == 1)) {
                player_pos.y = (player_pos.y - move_speed);
            }
            if ((camera_mode == 1)) {
                Vec3 offset = heidic_vec3(0, 100, 0);
                Vec3 offset_pos = heidic_vec3_add(player_pos, offset);
                camera_pos = heidic_attach_camera_translation(offset_pos);
                camera_rot = heidic_attach_camera_rotation(player_rot);
            } else {
                int32_t left_ctrl_pressed = heidic_is_key_pressed(window, 341);
                int32_t right_ctrl_pressed = heidic_is_key_pressed(window, 345);
                int32_t ctrl_pressed = 0;
                if ((left_ctrl_pressed == 1)) {
                    ctrl_pressed = 1;
                }
                if ((right_ctrl_pressed == 1)) {
                    ctrl_pressed = 1;
                }
                int32_t right_mouse_pressed = heidic_is_mouse_button_pressed(window, 1);
                if (((ctrl_pressed == 1) && (right_mouse_pressed == 1))) {
                    float mouse_delta_y = heidic_get_mouse_delta_y(window);
                    if ((mouse_delta_y != 0)) {
                        if (((alt_pressed == 1) && (has_selection == 1))) {
                            float dolly_speed = 50;
                            dolly_orbit_distance = (dolly_orbit_distance - (mouse_delta_y * dolly_speed));
                            if ((dolly_orbit_distance < 10)) {
                                dolly_orbit_distance = 10;
                            }
                            if ((dolly_orbit_distance > 100000)) {
                                dolly_orbit_distance = 100000;
                            }
                        } else {
                            float zoom_speed_factor = (topdown_cam_height / 10000);
                            float zoom_speed = (50 * zoom_speed_factor);
                            topdown_cam_height = (topdown_cam_height - (mouse_delta_y * zoom_speed));
                            if ((topdown_cam_height < 10)) {
                                topdown_cam_height = 10;
                            }
                            if ((topdown_cam_height > 100000)) {
                                topdown_cam_height = 100000;
                            }
                        }
                    }
                }
                if ((heidic_is_key_pressed(window, 91) == 1)) {
                    float zoom_speed = 100;
                    topdown_cam_height = (topdown_cam_height - zoom_speed);
                    if ((topdown_cam_height < 100)) {
                        topdown_cam_height = 100;
                    }
                }
                if ((heidic_is_key_pressed(window, 93) == 1)) {
                    float zoom_speed = 100;
                    topdown_cam_height = (topdown_cam_height + zoom_speed);
                    if ((topdown_cam_height > 50000)) {
                        topdown_cam_height = 50000;
                    }
                }
                int32_t mouse_middle_pressed = heidic_is_mouse_button_pressed(window, 2);
                if ((mouse_middle_pressed == 1)) {
                    float mouse_delta_x = heidic_get_mouse_delta_x(window);
                    float mouse_delta_y = heidic_get_mouse_delta_y(window);
                    if (((mouse_delta_x != 0) || (mouse_delta_y != 0))) {
                        float pan_speed_factor = (topdown_cam_height / 10000);
                        float pan_speed = (4 * pan_speed_factor);
                        topdown_cam_pan_x = (topdown_cam_pan_x - (mouse_delta_x * pan_speed));
                        topdown_cam_pan_z = (topdown_cam_pan_z - (mouse_delta_y * pan_speed));
                    }
                    mouse_middle_was_pressed = 1;
                } else {
                    mouse_middle_was_pressed = 0;
                }
                if ((in_orbit_mode == 0)) {
                    topdown_cam_pos = heidic_vec3(topdown_cam_pan_x, topdown_cam_height, topdown_cam_pan_z);
                    camera_pos = topdown_cam_pos;
                    camera_rot = topdown_cam_rot;
                }
                in_orbit_mode = 0;
            }
            cube_x = player_pos.x;
            cube_y = player_pos.y;
            cube_z = player_pos.z;
            cube_rx = player_rot.x;
            cube_ry = player_rot.y;
            cube_rz = player_rot.z;
            heidic_begin_frame();
            heidic_imgui_setup_dockspace();
            if ((heidic_imgui_begin_main_menu_bar() == 1)) {
                if ((heidic_imgui_begin_menu("File") == 1)) {
                    if ((heidic_imgui_menu_item("Save Level As...") == 1)) {
                        int32_t save_result = heidic_show_save_dialog();
                        if ((save_result == 1)) {
                        }
                    }
                    if ((heidic_imgui_menu_item("Open Level...") == 1)) {
                        int32_t load_result = heidic_show_open_dialog();
                        if ((load_result == 1)) {
                        }
                    }
                    heidic_imgui_separator();
                    if ((heidic_imgui_menu_item("Exit") == 1)) {
                        heidic_set_window_should_close(window, 1);
                    }
                    heidic_imgui_end_menu();
                }
                if ((heidic_imgui_begin_menu("Object") == 1)) {
                    if ((heidic_imgui_menu_item("Add Cube") == 1)) {
                        float default_cube_size = 200;
                        int32_t new_cube_index = heidic_create_cube(0, 0, 0, default_cube_size, default_cube_size, default_cube_size);
                        selected_cube_index = (heidic_int_to_float(new_cube_index) + 2);
                        has_selection = 1;
                        selected_cube_x = 0;
                        selected_cube_y = 0;
                        selected_cube_z = 0;
                        selected_cube_sx = default_cube_size;
                        selected_cube_sy = default_cube_size;
                        selected_cube_sz = default_cube_size;
                    }
                    heidic_imgui_end_menu();
                }
                heidic_imgui_end_main_menu_bar();
            }
            if ((camera_mode == 0)) {
                heidic_update_camera_with_far(camera_pos.x, camera_pos.y, camera_pos.z, camera_rot.x, camera_rot.y, camera_rot.z, 50000);
            } else {
                heidic_update_camera(camera_pos.x, camera_pos.y, camera_pos.z, camera_rot.x, camera_rot.y, camera_rot.z);
            }
            heidic_draw_cube_grey(0, -500, 0, 0, 0, 0, 10000, 100, 10000);
            if ((show_grid == 1)) {
                heidic_draw_ground_plane(20000, 0.5, 0.5, 0.5);
            }
            if ((camera_mode == 1)) {
                player_cube_visible = INVISIBLE;
            } else {
                player_cube_visible = VISIBLE;
            }
            if ((player_cube_visible == VISIBLE)) {
                heidic_draw_cube(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, cube_sx, cube_sy, cube_sz);
            }
            int32_t cube_draw_index = 0;
            int32_t total_cubes = heidic_get_cube_total_count();
            while ((cube_draw_index < total_cubes)) {
                if ((heidic_get_cube_active(cube_draw_index) == 1)) {
                    float cube_x = heidic_get_cube_x(cube_draw_index);
                    float cube_y = heidic_get_cube_y(cube_draw_index);
                    float cube_z = heidic_get_cube_z(cube_draw_index);
                    float cube_sx = heidic_get_cube_sx(cube_draw_index);
                    float cube_sy = heidic_get_cube_sy(cube_draw_index);
                    float cube_sz = heidic_get_cube_sz(cube_draw_index);
                    float cube_r = heidic_get_cube_r(cube_draw_index);
                    float cube_g = heidic_get_cube_g(cube_draw_index);
                    float cube_b = heidic_get_cube_b(cube_draw_index);
                    heidic_draw_cube_colored(cube_x, cube_y, cube_z, 0, 0, 0, cube_sx, cube_sy, cube_sz, cube_r, cube_g, cube_b);
                }
                cube_draw_index = (cube_draw_index + 1);
            }
            float mouse_x = heidic_get_mouse_x(window);
            float mouse_y = heidic_get_mouse_y(window);
            Vec3 ray_origin = heidic_get_mouse_ray_origin(window);
            Vec3 ray_dir = heidic_get_mouse_ray_dir(window);
            heidic_draw_ray(window, 50000, 1, 1, 0);
            Vec3 debug_ray_origin = heidic_get_mouse_ray_origin(window);
            Vec3 debug_ray_dir = heidic_get_mouse_ray_dir(window);
            Vec3 debug_hit_pos = heidic_vec3(0, 0, 0);
            int32_t debug_found_hit = 0;
            float debug_closest_dist = 100000000000;
            float debug_hit_cube_x = 0;
            float debug_hit_cube_y = 0;
            float debug_hit_cube_z = 0;
            float debug_hit_cube_sx = 0;
            float debug_hit_cube_sy = 0;
            float debug_hit_cube_sz = 0;
            int32_t debug_hit_face = -1;
            int32_t debug_is_ground_plane = 0;
            float ground_cube_x = 0;
            float ground_cube_y = -500;
            float ground_cube_z = 0;
            float ground_cube_sx = 10000;
            float ground_cube_sy = 100;
            float ground_cube_sz = 10000;
            int32_t ground_cube_hit = heidic_raycast_cube_hit(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
            if ((ground_cube_hit == 1)) {
                Vec3 ground_cube_hit_point = heidic_raycast_cube_hit_point(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                float ground_cube_dist = ((((ground_cube_hit_point.x - debug_ray_origin.x) * (ground_cube_hit_point.x - debug_ray_origin.x)) + ((ground_cube_hit_point.y - debug_ray_origin.y) * (ground_cube_hit_point.y - debug_ray_origin.y))) + ((ground_cube_hit_point.z - debug_ray_origin.z) * (ground_cube_hit_point.z - debug_ray_origin.z)));
                if ((ground_cube_dist < debug_closest_dist)) {
                    debug_closest_dist = ground_cube_dist;
                    debug_hit_pos = ground_cube_hit_point;
                    debug_hit_cube_x = ground_cube_x;
                    debug_hit_cube_y = ground_cube_y;
                    debug_hit_cube_z = ground_cube_z;
                    debug_hit_cube_sx = ground_cube_sx;
                    debug_hit_cube_sy = ground_cube_sy;
                    debug_hit_cube_sz = ground_cube_sz;
                    debug_is_ground_plane = 1;
                    debug_found_hit = 1;
                }
            }
            int32_t debug_cube_index = 0;
            int32_t debug_total_cubes = heidic_get_cube_total_count();
            while ((debug_cube_index < debug_total_cubes)) {
                if ((heidic_get_cube_active(debug_cube_index) == 1)) {
                    float debug_cube_x = heidic_get_cube_x(debug_cube_index);
                    float debug_cube_y = heidic_get_cube_y(debug_cube_index);
                    float debug_cube_z = heidic_get_cube_z(debug_cube_index);
                    float debug_cube_sx = heidic_get_cube_sx(debug_cube_index);
                    float debug_cube_sy = heidic_get_cube_sy(debug_cube_index);
                    float debug_cube_sz = heidic_get_cube_sz(debug_cube_index);
                    int32_t debug_cube_hit = heidic_raycast_cube_hit(window, debug_cube_x, debug_cube_y, debug_cube_z, debug_cube_sx, debug_cube_sy, debug_cube_sz);
                    if ((debug_cube_hit == 1)) {
                        Vec3 debug_hit_point = heidic_raycast_cube_hit_point(window, debug_cube_x, debug_cube_y, debug_cube_z, debug_cube_sx, debug_cube_sy, debug_cube_sz);
                        float debug_dist = ((((debug_hit_point.x - debug_ray_origin.x) * (debug_hit_point.x - debug_ray_origin.x)) + ((debug_hit_point.y - debug_ray_origin.y) * (debug_hit_point.y - debug_ray_origin.y))) + ((debug_hit_point.z - debug_ray_origin.z) * (debug_hit_point.z - debug_ray_origin.z)));
                        if ((debug_dist < debug_closest_dist)) {
                            debug_closest_dist = debug_dist;
                            debug_hit_pos = debug_hit_point;
                            debug_hit_cube_x = debug_cube_x;
                            debug_hit_cube_y = debug_cube_y;
                            debug_hit_cube_z = debug_cube_z;
                            debug_hit_cube_sx = debug_cube_sx;
                            debug_hit_cube_sy = debug_cube_sy;
                            debug_hit_cube_sz = debug_cube_sz;
                            debug_is_ground_plane = 0;
                            debug_found_hit = 1;
                        }
                    }
                }
                debug_cube_index = (debug_cube_index + 1);
            }
            if ((debug_found_hit == 0)) {
                debug_hit_pos = heidic_vec3_add(debug_ray_origin, heidic_vec3_mul_scalar(debug_ray_dir, 500));
            }
            heidic_draw_cube_wireframe(debug_hit_pos.x, debug_hit_pos.y, debug_hit_pos.z, 0, 0, 0, 100, 100, 100, 1, 0, 0);
            if (((debug_found_hit == 1) && (debug_is_ground_plane == 0))) {
                float debug_cube_min_x = (debug_hit_cube_x - (debug_hit_cube_sx / 2));
                float debug_cube_max_x = (debug_hit_cube_x + (debug_hit_cube_sx / 2));
                float debug_cube_min_y = (debug_hit_cube_y - (debug_hit_cube_sy / 2));
                float debug_cube_max_y = (debug_hit_cube_y + (debug_hit_cube_sy / 2));
                float debug_cube_min_z = (debug_hit_cube_z - (debug_hit_cube_sz / 2));
                float debug_cube_max_z = (debug_hit_cube_z + (debug_hit_cube_sz / 2));
                float debug_dist_to_left = (debug_hit_pos.x - debug_cube_min_x);
                float debug_dist_to_right = (debug_cube_max_x - debug_hit_pos.x);
                float debug_dist_to_bottom = (debug_hit_pos.y - debug_cube_min_y);
                float debug_dist_to_top = (debug_cube_max_y - debug_hit_pos.y);
                float debug_dist_to_back = (debug_hit_pos.z - debug_cube_min_z);
                float debug_dist_to_front = (debug_cube_max_z - debug_hit_pos.z);
                float debug_min_dist = debug_dist_to_left;
                debug_hit_face = 0;
                if ((debug_dist_to_right < debug_min_dist)) {
                    debug_min_dist = debug_dist_to_right;
                    debug_hit_face = 1;
                }
                if ((debug_dist_to_bottom < debug_min_dist)) {
                    debug_min_dist = debug_dist_to_bottom;
                    debug_hit_face = 2;
                }
                if ((debug_dist_to_top < debug_min_dist)) {
                    debug_min_dist = debug_dist_to_top;
                    debug_hit_face = 3;
                }
                if ((debug_dist_to_back < debug_min_dist)) {
                    debug_min_dist = debug_dist_to_back;
                    debug_hit_face = 4;
                }
                if ((debug_dist_to_front < debug_min_dist)) {
                    debug_min_dist = debug_dist_to_front;
                    debug_hit_face = 5;
                }
                float highlight_offset = 1;
                if ((debug_hit_face == 0)) {
                    float y1 = debug_cube_min_y;
                    float y2 = debug_cube_max_y;
                    float z1 = debug_cube_min_z;
                    float z2 = debug_cube_max_z;
                    float x = (debug_cube_min_x - highlight_offset);
                    heidic_draw_line(x, y1, z1, x, y2, z1, 0, 1, 0);
                    heidic_draw_line(x, y2, z1, x, y2, z2, 0, 1, 0);
                    heidic_draw_line(x, y2, z2, x, y1, z2, 0, 1, 0);
                    heidic_draw_line(x, y1, z2, x, y1, z1, 0, 1, 0);
                } else {
                    if ((debug_hit_face == 1)) {
                        float y1 = debug_cube_min_y;
                        float y2 = debug_cube_max_y;
                        float z1 = debug_cube_min_z;
                        float z2 = debug_cube_max_z;
                        float x = (debug_cube_max_x + highlight_offset);
                        heidic_draw_line(x, y1, z1, x, y2, z1, 0, 1, 0);
                        heidic_draw_line(x, y2, z1, x, y2, z2, 0, 1, 0);
                        heidic_draw_line(x, y2, z2, x, y1, z2, 0, 1, 0);
                        heidic_draw_line(x, y1, z2, x, y1, z1, 0, 1, 0);
                    } else {
                        if ((debug_hit_face == 2)) {
                            float x1 = debug_cube_min_x;
                            float x2 = debug_cube_max_x;
                            float z1 = debug_cube_min_z;
                            float z2 = debug_cube_max_z;
                            float y = (debug_cube_min_y - highlight_offset);
                            heidic_draw_line(x1, y, z1, x2, y, z1, 0, 1, 0);
                            heidic_draw_line(x2, y, z1, x2, y, z2, 0, 1, 0);
                            heidic_draw_line(x2, y, z2, x1, y, z2, 0, 1, 0);
                            heidic_draw_line(x1, y, z2, x1, y, z1, 0, 1, 0);
                        } else {
                            if ((debug_hit_face == 3)) {
                                float x1 = debug_cube_min_x;
                                float x2 = debug_cube_max_x;
                                float z1 = debug_cube_min_z;
                                float z2 = debug_cube_max_z;
                                float y = (debug_cube_max_y + highlight_offset);
                                heidic_draw_line(x1, y, z1, x2, y, z1, 0, 1, 0);
                                heidic_draw_line(x2, y, z1, x2, y, z2, 0, 1, 0);
                                heidic_draw_line(x2, y, z2, x1, y, z2, 0, 1, 0);
                                heidic_draw_line(x1, y, z2, x1, y, z1, 0, 1, 0);
                            } else {
                                if ((debug_hit_face == 4)) {
                                    float x1 = debug_cube_min_x;
                                    float x2 = debug_cube_max_x;
                                    float y1 = debug_cube_min_y;
                                    float y2 = debug_cube_max_y;
                                    float z = (debug_cube_min_z - highlight_offset);
                                    heidic_draw_line(x1, y1, z, x2, y1, z, 0, 1, 0);
                                    heidic_draw_line(x2, y1, z, x2, y2, z, 0, 1, 0);
                                    heidic_draw_line(x2, y2, z, x1, y2, z, 0, 1, 0);
                                    heidic_draw_line(x1, y2, z, x1, y1, z, 0, 1, 0);
                                } else {
                                    float x1 = debug_cube_min_x;
                                    float x2 = debug_cube_max_x;
                                    float y1 = debug_cube_min_y;
                                    float y2 = debug_cube_max_y;
                                    float z = (debug_cube_max_z + highlight_offset);
                                    heidic_draw_line(x1, y1, z, x2, y1, z, 0, 1, 0);
                                    heidic_draw_line(x2, y1, z, x2, y2, z, 0, 1, 0);
                                    heidic_draw_line(x2, y2, z, x1, y2, z, 0, 1, 0);
                                    heidic_draw_line(x1, y2, z, x1, y1, z, 0, 1, 0);
                                }
                            }
                        }
                    }
                }
            }
            int32_t mouse_left_pressed = heidic_is_mouse_button_pressed(window, 0);
            int32_t mouse_right_pressed = heidic_is_mouse_button_pressed(window, 1);
            if ((mouse_left_pressed == 1)) {
                if ((mouse_mode_left_was_pressed == 0)) {
                    mouse_mode = 1;
                    heidic_set_cursor_mode(window, 0);
                    mouse_mode_left_was_pressed = 1;
                }
            } else {
                mouse_mode_left_was_pressed = 0;
            }
            if ((mouse_right_pressed == 1)) {
                if ((mouse_mode_right_was_pressed == 0)) {
                    mouse_mode = 0;
                    if ((camera_mode == 1)) {
                        heidic_set_cursor_mode(window, 2);
                    } else {
                        heidic_set_cursor_mode(window, 1);
                    }
                    mouse_mode_right_was_pressed = 1;
                }
            } else {
                mouse_mode_right_was_pressed = 0;
            }
            if ((((mouse_mode == 1) && (mouse_left_pressed == 1)) && (alt_pressed == 0))) {
                int32_t interacting = 0;
                if ((mouse_left_was_pressed == 1)) {
                    interacting = heidic_gizmo_is_interacting();
                }
                if ((interacting == 0)) {
                    if ((mouse_left_was_pressed == 0)) {
                        has_selection = 0;
                        selected_cube_index = -1;
                        float closest_dist = 100000000000;
                        int32_t cube_test_index = 0;
                        int32_t total_cubes_test = heidic_get_cube_total_count();
                        while ((cube_test_index < total_cubes_test)) {
                            if ((heidic_get_cube_active(cube_test_index) == 1)) {
                                float test_cube_x = heidic_get_cube_x(cube_test_index);
                                float test_cube_y = heidic_get_cube_y(cube_test_index);
                                float test_cube_z = heidic_get_cube_z(cube_test_index);
                                float test_cube_sx = heidic_get_cube_sx(cube_test_index);
                                float test_cube_sy = heidic_get_cube_sy(cube_test_index);
                                float test_cube_sz = heidic_get_cube_sz(cube_test_index);
                                int32_t created_hit = heidic_raycast_cube_hit(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                if ((created_hit == 1)) {
                                    Vec3 hit_point = heidic_raycast_cube_hit_point(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                    float dist = ((((hit_point.x - ray_origin.x) * (hit_point.x - ray_origin.x)) + ((hit_point.y - ray_origin.y) * (hit_point.y - ray_origin.y))) + ((hit_point.z - ray_origin.z) * (hit_point.z - ray_origin.z)));
                                    if ((dist < closest_dist)) {
                                        closest_dist = dist;
                                        has_selection = 1;
                                        float cube_index_f = heidic_int_to_float(cube_test_index);
                                        selected_cube_index = (cube_index_f + 2);
                                        selected_cube_x = test_cube_x;
                                        selected_cube_y = test_cube_y;
                                        selected_cube_z = test_cube_z;
                                        selected_cube_sx = test_cube_sx;
                                        selected_cube_sy = test_cube_sy;
                                        selected_cube_sz = test_cube_sz;
                                        Vec3 target_pos = heidic_vec3(test_cube_x, test_cube_y, test_cube_z);
                                        Vec3 current_cam_pos = camera_pos;
                                        float calculated_distance = heidic_vec3_distance(current_cam_pos, target_pos);
                                        if (((calculated_distance > 10) && (calculated_distance < 100000))) {
                                            dolly_orbit_distance = calculated_distance;
                                        } else {
                                            dolly_orbit_distance = 2000;
                                        }
                                        dolly_orbit_azimuth = 0;
                                        dolly_orbit_elevation = 45;
                                    }
                                }
                            }
                            cube_test_index = (cube_test_index + 1);
                        }
                        mouse_left_was_pressed = 1;
                    }
                }
            } else {
                mouse_left_was_pressed = 0;
            }
            if ((has_selection == 1)) {
                heidic_draw_cube_wireframe(selected_cube_x, selected_cube_y, selected_cube_z, 0, 0, 0, (selected_cube_sx * 1.01), (selected_cube_sy * 1.01), (selected_cube_sz * 1.01), 0, 0, 0);
                Vec3 new_pos = heidic_gizmo_translate(window, selected_cube_x, selected_cube_y, selected_cube_z);
                selected_cube_x = new_pos.x;
                selected_cube_y = new_pos.y;
                selected_cube_z = new_pos.z;
                if ((selected_cube_index >= 2)) {
                    float cube_storage_index = (selected_cube_index - 2);
                    heidic_set_cube_pos_f(cube_storage_index, selected_cube_x, selected_cube_y, selected_cube_z);
                }
            }
            if ((show_debug == 1)) {
                heidic_imgui_begin("Test Window");
                heidic_imgui_text("=== TEST WINDOW ===");
                heidic_imgui_text("This is a test window for docking!");
                heidic_imgui_text("Try docking this to the Debug Panel.");
                heidic_imgui_text_float("Selected Index", selected_cube_index);
                heidic_imgui_text_float("Selected X", selected_cube_x);
                heidic_imgui_text_float("Selected Y", selected_cube_y);
                heidic_imgui_text_float("Selected Z", selected_cube_z);
                heidic_imgui_text("=== CREATED CUBES ===");
                int32_t active_cube_count = heidic_get_cube_count();
                int32_t total_cube_count = heidic_get_cube_total_count();
                heidic_imgui_text_float("Active Cubes", heidic_int_to_float(active_cube_count));
                heidic_imgui_text_float("Total Cubes", heidic_int_to_float(total_cube_count));
                heidic_imgui_end();
            }
            float ground_check_distance = 200;
            is_grounded = heidic_raycast_ground_hit(player_pos.x, player_pos.y, player_pos.z, ground_check_distance);
            if ((is_grounded == 1)) {
                Vec3 ground_hit = heidic_raycast_ground_hit_point(player_pos.x, player_pos.y, player_pos.z, ground_check_distance);
                heidic_draw_line(player_pos.x, player_pos.y, player_pos.z, ground_hit.x, ground_hit.y, ground_hit.z, 0, 1, 0);
            } else {
                Vec3 ground_check_end = heidic_vec3(player_pos.x, (player_pos.y - ground_check_distance), player_pos.z);
                heidic_draw_line(player_pos.x, player_pos.y, player_pos.z, ground_check_end.x, ground_check_end.y, ground_check_end.z, 1, 0, 0);
            }
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
                heidic_imgui_text("=== Raycasting ===");
                heidic_imgui_text("Mouse Screen Coords:");
                heidic_imgui_text_float("  Mouse X (screen)", mouse_x);
                heidic_imgui_text_float("  Mouse Y (screen)", mouse_y);
                heidic_imgui_text("Ray Origin (world):");
                heidic_imgui_text_float("  Ray Origin X", ray_origin.x);
                heidic_imgui_text_float("  Ray Origin Y", ray_origin.y);
                heidic_imgui_text_float("  Ray Origin Z", ray_origin.z);
                heidic_imgui_text("Ray Direction (normalized):");
                heidic_imgui_text_float("  Ray Dir X", ray_dir.x);
                heidic_imgui_text_float("  Ray Dir Y", ray_dir.y);
                heidic_imgui_text_float("  Ray Dir Z", ray_dir.z);
                heidic_imgui_text("Camera to Origin Dist:");
                float dist_camera_to_origin = ((((ray_origin.x - camera_pos.x) * (ray_origin.x - camera_pos.x)) + ((ray_origin.y - camera_pos.y) * (ray_origin.y - camera_pos.y))) + ((ray_origin.z - camera_pos.z) * (ray_origin.z - camera_pos.z)));
                heidic_imgui_text_float("  Distance", dist_camera_to_origin);
                heidic_imgui_text("Selection:");
                if ((has_selection == 1)) {
                    heidic_imgui_text("  Selected Cube");
                    heidic_imgui_text_float("  Index", selected_cube_index);
                    heidic_imgui_text_float("  X", selected_cube_x);
                    heidic_imgui_text_float("  Y", selected_cube_y);
                    heidic_imgui_text_float("  Z", selected_cube_z);
                } else {
                    heidic_imgui_text("  No Selection (Click to select)");
                }
                heidic_imgui_text("Ground Detection:");
                if ((is_grounded == 1)) {
                    heidic_imgui_text("  GROUNDED");
                } else {
                    heidic_imgui_text("  IN AIR");
                }
                heidic_imgui_end();
            }
            heidic_end_frame();
        }
        heidic_imgui_save_layout("");
        heidic_cleanup_renderer();
        heidic_destroy_window(window);
        heidic_glfw_terminate();
        return 0;
}

int main(int argc, char* argv[]) {
    heidic_main();
    return 0;
}
