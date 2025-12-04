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
#include "stdlib/eden_imgui.h"
#include "vulkan/eden_vulkan_helpers.h"

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
    int32_t heidic_ctrl_down(GLFWwindow* window);
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
    void heidic_flush_colored_cubes();
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
    int32_t heidic_load_hdm_mesh(const char* filepath);
}
extern "C" {
    int32_t heidic_get_mesh_count();
}
extern "C" {
    int32_t heidic_get_mesh_id(int32_t index);
}
extern "C" {
    void heidic_draw_mesh(int32_t mesh_id, float x, float y, float z, float rx, float ry, float rz);
}
extern "C" {
    void heidic_draw_mesh_scaled(int32_t mesh_id, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
}
extern "C" {
    void heidic_draw_mesh_scaled_with_center(int32_t mesh_id, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float center_x, float center_y, float center_z);
}
extern "C" {
    int32_t heidic_get_mesh_instance_count();
}
extern "C" {
    int32_t heidic_get_mesh_instance_total_count();
}
extern "C" {
    int32_t heidic_get_mesh_instance_mesh_id(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_x(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_y(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_z(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_center_x(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_center_y(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_center_z(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_min_x(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_min_y(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_min_z(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_max_x(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_max_y(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_bbox_max_z(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_sx(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_sy(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_sz(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_rx(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_ry(int32_t instance_id);
}
extern "C" {
    float heidic_get_mesh_instance_rz(int32_t instance_id);
}
extern "C" {
    void heidic_set_mesh_instance_pos(int32_t instance_id, float x, float y, float z);
}
extern "C" {
    void heidic_set_mesh_instance_scale(int32_t instance_id, float sx, float sy, float sz);
}
extern "C" {
    void heidic_set_mesh_instance_rotation(int32_t instance_id, float rx, float ry, float rz);
}
extern "C" {
    const char* heidic_get_mesh_instance_name(int32_t instance_id);
}
extern "C" {
    int32_t heidic_get_mesh_instance_active(int32_t instance_id);
}
extern "C" {
    void heidic_clear_mesh_selection();
}
extern "C" {
    void heidic_add_mesh_to_selection(int32_t instance_id);
}
extern "C" {
    void heidic_remove_mesh_from_selection(int32_t instance_id);
}
extern "C" {
    void heidic_toggle_mesh_selection(int32_t instance_id);
}
extern "C" {
    int32_t heidic_is_mesh_selected(int32_t instance_id);
}
extern "C" {
    int32_t heidic_get_mesh_selection_count();
}
extern "C" {
    const char* heidic_format_mesh_name(int32_t instance_id);
}
extern "C" {
    void heidic_set_component_mode(int32_t mode);
}
extern "C" {
    int32_t heidic_get_component_mode();
}
extern "C" {
    int32_t heidic_raycast_mesh_edges(GLFWwindow* window, int32_t instance_id);
}
extern "C" {
    void heidic_draw_edge(int32_t instance_id, int32_t v0_idx, int32_t v1_idx, float r, float g, float b);
}
extern "C" {
    void heidic_draw_mesh_edges(int32_t instance_id, float hover_r, float hover_g, float hover_b, float selected_r, float selected_g, float selected_b);
}
extern "C" {
    int32_t heidic_get_hovered_edge_instance_id();
}
extern "C" {
    int32_t heidic_get_hovered_edge_v0();
}
extern "C" {
    int32_t heidic_get_hovered_edge_v1();
}
extern "C" {
    void heidic_select_edge(int32_t instance_id, int32_t v0, int32_t v1);
}
extern "C" {
    void heidic_deselect_edge(int32_t instance_id, int32_t v0, int32_t v1);
}
extern "C" {
    int32_t heidic_is_edge_selected(int32_t instance_id, int32_t v0, int32_t v1);
}
extern "C" {
    void heidic_clear_edge_selection();
}
extern "C" {
    void heidic_imgui_init(GLFWwindow* window);
}
extern "C" {
    int32_t heidic_imgui_begin(const char* name);
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
    void heidic_imgui_text_str_wrapper(const char* text);
}
extern "C" {
    void heidic_imgui_text_colored(const char* text, float r, float g, float b, float a);
}
extern "C" {
    void heidic_imgui_text_bold(const char* text);
}
extern "C" {
    void heidic_imgui_text_float(const char* label, float value);
}
extern "C" {
    const char* heidic_format_cube_name(int32_t index);
}
extern "C" {
    const char* heidic_format_cube_name_with_index(int32_t index);
}
extern "C" {
    float heidic_imgui_drag_float(const char* label, float v, float speed);
}
extern "C" {
    float heidic_imgui_slider_float(const char* label, float v, float v_min, float v_max);
}
extern "C" {
    float heidic_imgui_input_float(const char* label, float v, float step, float step_fast);
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
    int32_t heidic_imgui_collapsing_header(const char* label);
}
extern "C" {
    int32_t heidic_imgui_button_str_wrapper(const char* label);
}
extern "C" {
    void heidic_imgui_same_line();
}
extern "C" {
    void heidic_imgui_push_id(int32_t id);
}
extern "C" {
    void heidic_imgui_pop_id();
}
extern "C" {
    const char* heidic_string_to_char_ptr(const char* str);
}
extern "C" {
    int32_t heidic_imgui_selectable_str(const char* label);
}
extern "C" {
    int32_t heidic_imgui_selectable_colored(const char* label, float r, float g, float b, float a);
}
extern "C" {
    int32_t heidic_imgui_image_button(const char* str_id, int64_t texture_id, float size_x, float size_y, float tint_r, float tint_g, float tint_b, float tint_a);
}
extern "C" {
    int32_t heidic_imgui_is_item_clicked();
}
extern "C" {
    int32_t heidic_imgui_is_key_enter_pressed();
}
extern "C" {
    int32_t heidic_imgui_is_key_escape_pressed();
}
extern "C" {
    int32_t heidic_imgui_input_text(const char* label, const char* buffer, int32_t buffer_size);
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
    int32_t heidic_show_open_mesh_dialog();
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
    int32_t heidic_imgui_wants_mouse();
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
    int32_t heidic_raycast_mesh_bbox_hit(GLFWwindow* window, int32_t instance_id);
}
extern "C" {
    Vec3 heidic_raycast_mesh_bbox_hit_point(GLFWwindow* window, int32_t instance_id);
}
extern "C" {
    void heidic_draw_cube_wireframe(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
}
extern "C" {
    void heidic_draw_wedge_wireframe(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
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
    float heidic_gizmo_scale(GLFWwindow* window, float center_x, float center_y, float center_z, float bbox_min_x, float bbox_min_y, float bbox_min_z, float bbox_max_x, float bbox_max_y, float bbox_max_z, float current_scale);
}
extern "C" {
    int32_t heidic_gizmo_scale_is_interacting();
}
extern "C" {
    Vec3 heidic_gizmo_rotate(GLFWwindow* window, float x, float y, float z, float current_rx, float current_ry, float current_rz);
}
extern "C" {
    int32_t heidic_gizmo_rotate_is_interacting();
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
    const char* heidic_get_cube_texture_name(int32_t index);
}
extern "C" {
    int32_t heidic_load_texture_for_rendering(const char* texture_name);
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
    float heidic_get_cube_rx(int32_t index);
}
extern "C" {
    float heidic_get_cube_ry(int32_t index);
}
extern "C" {
    float heidic_get_cube_rz(int32_t index);
}
extern "C" {
    void heidic_set_cube_rotation(int32_t index, float rx, float ry, float rz);
}
extern "C" {
    void heidic_set_cube_scale(int32_t index, float sx, float sy, float sz);
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
extern "C" {
    void heidic_draw_wedge_colored(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b);
}
extern "C" {
    int32_t heidic_create_wedge_with_color(float x, float y, float z, float sx, float sy, float sz, float r, float g, float b);
}
extern "C" {
    int32_t heidic_get_wedge_total_count();
}
extern "C" {
    int32_t heidic_get_wedge_active(int32_t index);
}
extern "C" {
    float heidic_get_wedge_x(int32_t index);
}
extern "C" {
    float heidic_get_wedge_y(int32_t index);
}
extern "C" {
    float heidic_get_wedge_z(int32_t index);
}
extern "C" {
    float heidic_get_wedge_sx(int32_t index);
}
extern "C" {
    float heidic_get_wedge_sy(int32_t index);
}
extern "C" {
    float heidic_get_wedge_sz(int32_t index);
}
extern "C" {
    float heidic_get_wedge_rx(int32_t index);
}
extern "C" {
    float heidic_get_wedge_ry(int32_t index);
}
extern "C" {
    float heidic_get_wedge_rz(int32_t index);
}
extern "C" {
    void heidic_set_wedge_pos(int32_t index, float x, float y, float z);
}
extern "C" {
    void heidic_set_wedge_scale(int32_t index, float sx, float sy, float sz);
}
extern "C" {
    void heidic_set_wedge_rotation(int32_t index, float rx, float ry, float rz);
}
extern "C" {
    const char* heidic_get_wedge_texture_name(int32_t index);
}
extern "C" {
    const char* heidic_format_wedge_name(int32_t index);
}
extern "C" {
    void heidic_combine_connected_cubes();
}
extern "C" {
    void heidic_combine_connected_cubes_from_selection(int32_t selected_cube_storage_index);
}
extern "C" {
    int32_t heidic_get_cube_combination_id(int32_t cube_index);
}
extern "C" {
    int32_t heidic_get_combination_cube_count(int32_t combination_id);
}
extern "C" {
    int32_t heidic_get_combination_first_cube(int32_t combination_id);
}
extern "C" {
    int32_t heidic_get_combination_next_cube(int32_t cube_index);
}
extern "C" {
    int32_t heidic_get_combination_count();
}
extern "C" {
    const char* heidic_format_combination_name(int32_t combination_id);
}
extern "C" {
    const char* heidic_get_combination_name_buffer(int32_t combination_id);
}
extern "C" {
    void heidic_set_combination_name_wrapper_str(int32_t combination_id, const char* name);
}
extern "C" {
    void heidic_start_editing_combination_name(int32_t combination_id);
}
extern "C" {
    void heidic_stop_editing_combination_name();
}
extern "C" {
    int32_t heidic_get_editing_combination_id();
}
extern "C" {
    const char* heidic_get_combination_name_edit_buffer();
}
extern "C" {
    int32_t heidic_imgui_input_text_combination_simple(int32_t combination_id);
}
extern "C" {
    void heidic_clear_selection();
}
extern "C" {
    void heidic_add_to_selection(int32_t cube_storage_index);
}
extern "C" {
    void heidic_remove_from_selection(int32_t cube_storage_index);
}
extern "C" {
    void heidic_toggle_selection(int32_t cube_storage_index);
}
extern "C" {
    int32_t heidic_is_cube_selected(int32_t cube_storage_index);
}
extern "C" {
    int32_t heidic_get_selection_count();
}
extern "C" {
    void heidic_combine_selected_cubes();
}
extern "C" {
    int32_t heidic_combine_selected_cubes_to_mesh(const char* filepath);
}
extern "C" {
    int32_t heidic_smooth_selected_cubes();
}
extern "C" {
    void heidic_load_texture_list();
}
extern "C" {
    int32_t heidic_get_texture_count();
}
extern "C" {
    const char* heidic_get_texture_name(int32_t index);
}
extern "C" {
    const char* heidic_get_selected_texture();
}
extern "C" {
    void heidic_set_selected_texture(const char* texture_name);
}
extern "C" {
    int64_t heidic_get_texture_preview_id(const char* texture_name);
}
extern "C" {
    int32_t heidic_imgui_input_text_combination_name();
}
extern "C" {
    int32_t heidic_imgui_should_stop_editing();
}
extern "C" {
    void heidic_toggle_combination_expanded(int32_t combination_id);
}
extern "C" {
    int32_t heidic_is_combination_expanded(int32_t combination_id);
}

int heidic_main();

int heidic_main() {
        int32_t  VISIBLE = 1;
        int32_t  INVISIBLE = 0;
        std::cout << "Initializing GLFW...\n" << std::endl;
        if ((heidic_glfw_init() == 0)) {
            return 0;
        }
        heidic_glfw_vulkan_hints();
        GLFWwindow*  window = heidic_create_window(1280, 720, "EDEN ENGINE - Gateway Editor v1");
        if ((heidic_init_renderer(window) == 0)) {
            heidic_glfw_terminate();
            return 0;
        }
        std::string  default_ini_path_load = "";
        heidic_imgui_load_layout(default_ini_path_load.c_str());
        Vec3  player_pos = heidic_vec3(3000, 100, 3000);
        Vec3  player_rot = heidic_vec3(0, 0, 0);
        Vec3  camera_pos = heidic_vec3(0, 100, 0);
        Vec3  camera_rot = heidic_vec3(0, 0, 0);
        float  cube_x = 0;
        float  cube_y = 0;
        float  cube_z = 0;
        float  cube_rx = 0;
        float  cube_ry = 0;
        float  cube_rz = 0;
        float  cube_sx = 100;
        float  cube_sy = 100;
        float  cube_sz = 100;
        float  move_speed = 15;
        float  rot_speed = 2;
        float  mouse_sensitivity = 0.1;
        float  pitch_max = 90;
        float  pitch_min = -90;
        int32_t  show_debug = 1;
        int32_t  f1_was_pressed = 0;
        int32_t  camera_mode = 1;
        int32_t  tab_was_pressed = 0;
        int32_t  player_cube_visible = VISIBLE;
        int32_t  mouse_mode = 1;
        int32_t  mouse_mode_left_was_pressed = 0;
        int32_t  mouse_mode_right_was_pressed = 0;
        heidic_set_cursor_mode(window, 0);
        int32_t  show_grid = 1;
        int32_t  g_was_pressed = 0;
        int32_t  video_mode = 1;
        int32_t  shift_enter_was_pressed = 0;
        float  selected_cube_x = 0;
        float  selected_cube_y = 0;
        float  selected_cube_z = 0;
        float  selected_cube_sx = 0;
        float  selected_cube_sy = 0;
        float  selected_cube_sz = 0;
        int32_t  has_selection = 0;
        Vec3  stored_preview_pos = heidic_vec3(0, 0, 0);
        int32_t  stored_preview_valid = 0;
        float  selected_cube_index = -1;
        int32_t  selected_mesh_instance_id = -1;
        float  selected_mesh_x = 0;
        float  selected_mesh_y = 0;
        float  selected_mesh_z = 0;
        int32_t  has_mesh_selection = 0;
        int32_t  selected_wedge_index = -1;
        float  selected_wedge_x = 0;
        float  selected_wedge_y = 0;
        float  selected_wedge_z = 0;
        float  selected_wedge_sx = 0;
        float  selected_wedge_sy = 0;
        float  selected_wedge_sz = 0;
        int32_t  has_wedge_selection = 0;
        int32_t  is_grounded = 0;
        int32_t  mouse_left_was_pressed = 0;
        int32_t  mouse_middle_was_pressed = 0;
        int32_t  delete_was_pressed = 0;
        int32_t  key_0_was_pressed = 0;
        int32_t  build_mode = 0;
        int32_t  key_1_was_pressed = 0;
        int32_t  key_2_was_pressed = 0;
        int32_t  wedge_preview_rotation = 0;
        int32_t  last_wedge_preview_rotation = -1;
        int32_t  combine_c_was_pressed = 0;
        int32_t  smooth_s_was_pressed = 0;
        float  topdown_cam_height = 10000;
        float  topdown_cam_pan_x = 0;
        float  topdown_cam_pan_z = 0;
        Vec3  topdown_cam_pos = heidic_vec3(0, topdown_cam_height, 0);
        Vec3  topdown_cam_rot = heidic_vec3(-90, 0, 0);
        float  dolly_orbit_azimuth = 0;
        float  dolly_orbit_elevation = 45;
        float  dolly_orbit_distance = 2000;
        int32_t  in_orbit_mode = 0;
        int32_t  alt_was_pressed = 0;
        int32_t  space_was_pressed = 0;
        float  created_cube_size = 200;
        std::cout << "Starting loop...\n" << std::endl;
        while ((heidic_window_should_close(window) == 0)) {
            heidic_poll_events();
            int32_t  is_editing_combination = heidic_get_editing_combination_id();
            int32_t  block_input = 0;
            if ((is_editing_combination >= 0)) {
                block_input = 1;
            }
            float  mouse_x = heidic_get_mouse_x(window);
            float  mouse_y = heidic_get_mouse_y(window);
            Vec3  ray_origin = heidic_get_mouse_ray_origin(window);
            Vec3  ray_dir = heidic_get_mouse_ray_dir(window);
            if ((heidic_is_key_pressed(window, 256) == 1)) {
                if ((is_editing_combination >= 0)) {
                    heidic_stop_editing_combination_name();
                } else {
                    heidic_set_window_should_close(window, 1);
                }
            }
            int32_t  left_alt_pressed = heidic_is_key_pressed(window, 342);
            int32_t  right_alt_pressed = heidic_is_key_pressed(window, 346);
            int32_t  alt_pressed = 0;
            if ((left_alt_pressed == 1)) {
                alt_pressed = 1;
            }
            if ((right_alt_pressed == 1)) {
                alt_pressed = 1;
            }
            if ((block_input == 0)) {
                int32_t  f1_is_pressed = heidic_is_key_pressed(window, 290);
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
                int32_t  tab_is_pressed = heidic_is_key_pressed(window, 258);
                if ((tab_is_pressed == 1)) {
                    if ((tab_was_pressed == 0)) {
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
                        tab_was_pressed = 1;
                    }
                } else {
                    tab_was_pressed = 0;
                }
                if (((alt_pressed == 1) && (mouse_mode == 0))) {
                    if ((has_selection == 1)) {
                        if ((alt_was_pressed == 0)) {
                            Vec3  target_pos = heidic_vec3(selected_cube_x, selected_cube_y, selected_cube_z);
                            Vec3  current_cam_pos = camera_pos;
                            Vec3  to_camera = heidic_vec3_sub(current_cam_pos, target_pos);
                            float  dist = heidic_vec3_distance(current_cam_pos, target_pos);
                            if ((dist > 0.001)) {
                                float  dir_x = (to_camera.x / dist);
                                float  dir_y = (to_camera.y / dist);
                                float  dir_z = (to_camera.z / dist);
                                float  azimuth_rad = heidic_atan2(dir_x, dir_z);
                                dolly_orbit_azimuth = heidic_convert_radians_to_degrees(azimuth_rad);
                                float  elevation_rad = heidic_asin(dir_y);
                                dolly_orbit_elevation = heidic_convert_radians_to_degrees(elevation_rad);
                                if (((dist > 10) && (dist < 100000))) {
                                    dolly_orbit_distance = dist;
                                }
                            }
                        }
                        float  dolly_delta_x = heidic_get_mouse_delta_x(window);
                        float  dolly_delta_y = heidic_get_mouse_delta_y(window);
                        float  orbit_sensitivity = 0.3;
                        dolly_orbit_azimuth = (dolly_orbit_azimuth - (dolly_delta_x * orbit_sensitivity));
                        dolly_orbit_elevation = (dolly_orbit_elevation + (dolly_delta_y * orbit_sensitivity));
                        if ((dolly_orbit_elevation > 89)) {
                            dolly_orbit_elevation = 89;
                        }
                        if ((dolly_orbit_elevation < -89)) {
                            dolly_orbit_elevation = -89;
                        }
                        Vec3  target_pos = heidic_vec3(selected_cube_x, selected_cube_y, selected_cube_z);
                        float  azimuth_rad = heidic_convert_degrees_to_radians(dolly_orbit_azimuth);
                        float  elevation_rad = heidic_convert_degrees_to_radians(dolly_orbit_elevation);
                        float  cos_elev = heidic_cos(elevation_rad);
                        float  sin_elev = heidic_sin(elevation_rad);
                        float  cos_azim = heidic_cos(azimuth_rad);
                        float  sin_azim = heidic_sin(azimuth_rad);
                        float  offset_x = ((dolly_orbit_distance * cos_elev) * sin_azim);
                        float  offset_y = (dolly_orbit_distance * sin_elev);
                        float  offset_z = ((dolly_orbit_distance * cos_elev) * cos_azim);
                        Vec3  offset = heidic_vec3(offset_x, offset_y, offset_z);
                        Vec3  new_cam_pos = heidic_vec3_add(target_pos, offset);
                        in_orbit_mode = 1;
                        if ((camera_mode == 0)) {
                            topdown_cam_pos = new_cam_pos;
                            camera_pos = new_cam_pos;
                            Vec3  to_target = heidic_vec3_sub(target_pos, new_cam_pos);
                            float  dist_to_target = heidic_vec3_distance(new_cam_pos, target_pos);
                            if ((dist_to_target > 0.001)) {
                                float  dir_x = (to_target.x / dist_to_target);
                                float  dir_y = (to_target.y / dist_to_target);
                                float  dir_z = (to_target.z / dist_to_target);
                                float  yaw_rad = heidic_atan2(dir_x, dir_z);
                                float  yaw_deg = heidic_convert_radians_to_degrees(yaw_rad);
                                float  pitch_rad = heidic_asin(dir_y);
                                float  pitch_deg = heidic_convert_radians_to_degrees(pitch_rad);
                                topdown_cam_rot.x = pitch_deg;
                                topdown_cam_rot.y = yaw_deg;
                                topdown_cam_rot.z = 0;
                            } else {
                                topdown_cam_rot.x = -dolly_orbit_elevation;
                                topdown_cam_rot.y = (dolly_orbit_azimuth + 180);
                                topdown_cam_rot.z = 0;
                            }
                            camera_rot = topdown_cam_rot;
                        } else {
                            player_pos = new_cam_pos;
                            Vec3  to_target = heidic_vec3_sub(target_pos, new_cam_pos);
                            float  dist_to_target = heidic_vec3_distance(new_cam_pos, target_pos);
                            if ((dist_to_target > 0.001)) {
                                float  dir_x = (to_target.x / dist_to_target);
                                float  dir_y = (to_target.y / dist_to_target);
                                float  dir_z = (to_target.z / dist_to_target);
                                float  yaw_rad = heidic_atan2(dir_x, dir_z);
                                float  yaw_deg = heidic_convert_radians_to_degrees(yaw_rad);
                                float  pitch_rad = heidic_asin(dir_y);
                                float  pitch_deg = heidic_convert_radians_to_degrees(pitch_rad);
                                player_rot.y = yaw_deg;
                                player_rot.x = pitch_deg;
                            } else {
                                player_rot.y = (dolly_orbit_azimuth + 180);
                                player_rot.x = -dolly_orbit_elevation;
                            }
                        }
                    } else {
                        float  dolly_delta_x = heidic_get_mouse_delta_x(window);
                        float  dolly_delta_y = heidic_get_mouse_delta_y(window);
                        float  dolly_speed = 20;
                        if ((camera_mode == 0)) {
                            float  yaw_rad = heidic_convert_degrees_to_radians(topdown_cam_rot.y);
                            float  forward_x = heidic_sin(yaw_rad);
                            float  forward_z = heidic_cos(yaw_rad);
                            float  right_x = heidic_cos(yaw_rad);
                            float  right_z = -heidic_sin(yaw_rad);
                            float  move_x = (((right_x * dolly_delta_x) * dolly_speed) + ((forward_x * dolly_delta_y) * dolly_speed));
                            float  move_z = (((right_z * dolly_delta_x) * dolly_speed) + ((forward_z * dolly_delta_y) * dolly_speed));
                            topdown_cam_pan_x = (topdown_cam_pan_x + move_x);
                            topdown_cam_pan_z = (topdown_cam_pan_z + move_z);
                        } else {
                            float  yaw_rad = heidic_convert_degrees_to_radians(player_rot.y);
                            float  forward_x = heidic_sin(yaw_rad);
                            float  forward_z = heidic_cos(yaw_rad);
                            float  right_x = heidic_cos(yaw_rad);
                            float  right_z = -heidic_sin(yaw_rad);
                            float  move_x = (((right_x * dolly_delta_x) * dolly_speed) + ((forward_x * dolly_delta_y) * dolly_speed));
                            float  move_z = (((right_z * dolly_delta_x) * dolly_speed) + ((forward_z * dolly_delta_y) * dolly_speed));
                            player_pos.x = (player_pos.x + move_x);
                            player_pos.z = (player_pos.z + move_z);
                        }
                    }
                    alt_was_pressed = 1;
                } else {
                    alt_was_pressed = 0;
                }
                if ((((camera_mode == 1) && (mouse_mode == 0)) && (alt_pressed == 0))) {
                    float  mouse_delta_x = heidic_get_mouse_delta_x(window);
                    float  mouse_delta_y = heidic_get_mouse_delta_y(window);
                    player_rot.y = (player_rot.y - (mouse_delta_x * mouse_sensitivity));
                    player_rot.x = (player_rot.x - (mouse_delta_y * mouse_sensitivity));
                    if ((player_rot.x > pitch_max)) {
                        player_rot.x = pitch_max;
                    }
                    float  pitch_check = (pitch_min - player_rot.x);
                    if ((pitch_check > 0)) {
                        player_rot.x = pitch_min;
                    }
                }
                int32_t  g_is_pressed = heidic_is_key_pressed(window, 71);
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
                int32_t  key_1_pressed = heidic_is_key_pressed(window, 49);
                if ((key_1_pressed == 1)) {
                    if ((key_1_was_pressed == 0)) {
                        build_mode = 0;
                        std::cout << "Build mode: Cube\n" << std::endl;
                    }
                    key_1_was_pressed = 1;
                } else {
                    key_1_was_pressed = 0;
                }
                int32_t  key_2_pressed = heidic_is_key_pressed(window, 50);
                if ((key_2_pressed == 1)) {
                    if ((key_2_was_pressed == 0)) {
                        build_mode = 1;
                        std::cout << "Build mode: Wedge\n" << std::endl;
                    }
                    key_2_was_pressed = 1;
                } else {
                    key_2_was_pressed = 0;
                }
                if ((build_mode == 1)) {
                    float  mouse_scroll = heidic_get_mouse_scroll_y(window);
                    float  scroll_threshold = 0.1;
                    if ((heidic_imgui_wants_mouse() == 0)) {
                        if ((mouse_scroll > scroll_threshold)) {
                            wedge_preview_rotation = (wedge_preview_rotation - 1);
                            if ((wedge_preview_rotation < 0)) {
                                wedge_preview_rotation = 11;
                            }
                        } else {
                            if ((mouse_scroll < -scroll_threshold)) {
                                wedge_preview_rotation = (wedge_preview_rotation + 1);
                                if ((wedge_preview_rotation > 11)) {
                                    wedge_preview_rotation = 0;
                                }
                            }
                        }
                    }
                }
                int32_t  space_is_pressed = heidic_is_key_pressed(window, 32);
                if ((space_is_pressed == 0)) {
                    space_is_pressed = heidic_is_key_pressed(window, 57);
                }
                if ((space_is_pressed == 1)) {
                    if ((space_was_pressed == 0)) {
                        std::cout << "SPACEBAR PRESSED! Creating cube at ray hit point\n" << std::endl;
                        float  default_cube_size = 100;
                        Vec3  create_pos = heidic_vec3(0, 0, 0);
                        if ((stored_preview_valid == 1)) {
                            create_pos = stored_preview_pos;
                            std::cout << "Using stored preview position for cube placement\n" << std::endl;
                        } else {
                            std::cout << "WARNING: No valid preview position, calculating placement\n" << std::endl;
                            Vec3  create_ray_origin = heidic_get_mouse_ray_origin(window);
                            Vec3  create_ray_dir = heidic_get_mouse_ray_dir(window);
                            int32_t  found_hit = 0;
                            float  closest_dist = 100000000000;
                            float  hit_cube_x = 0;
                            float  hit_cube_y = 0;
                            float  hit_cube_z = 0;
                            float  hit_cube_sx = 0;
                            float  hit_cube_sy = 0;
                            float  hit_cube_sz = 0;
                            Vec3  hit_point = heidic_vec3(0, 0, 0);
                            int32_t  is_ground_plane = 0;
                            int32_t  is_wedge = 0;
                            float  ground_cube_x = 0;
                            float  ground_cube_y = -500;
                            float  ground_cube_z = 0;
                            float  ground_cube_sx = 10000;
                            float  ground_cube_sy = 100;
                            float  ground_cube_sz = 10000;
                            int32_t  ground_cube_hit = heidic_raycast_cube_hit(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                            if ((ground_cube_hit == 1)) {
                                Vec3  ground_cube_hit_point = heidic_raycast_cube_hit_point(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                                float  ground_cube_dist = ((((ground_cube_hit_point.x - create_ray_origin.x) * (ground_cube_hit_point.x - create_ray_origin.x)) + ((ground_cube_hit_point.y - create_ray_origin.y) * (ground_cube_hit_point.y - create_ray_origin.y))) + ((ground_cube_hit_point.z - create_ray_origin.z) * (ground_cube_hit_point.z - create_ray_origin.z)));
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
                            int32_t  cube_test_index = 0;
                            int32_t  total_cubes_test = heidic_get_cube_total_count();
                            while ((cube_test_index < total_cubes_test)) {
                                if ((heidic_get_cube_active(cube_test_index) == 1)) {
                                    float  test_cube_x = heidic_get_cube_x(cube_test_index);
                                    float  test_cube_y = heidic_get_cube_y(cube_test_index);
                                    float  test_cube_z = heidic_get_cube_z(cube_test_index);
                                    float  test_cube_sx = heidic_get_cube_sx(cube_test_index);
                                    float  test_cube_sy = heidic_get_cube_sy(cube_test_index);
                                    float  test_cube_sz = heidic_get_cube_sz(cube_test_index);
                                    int32_t  cube_hit = heidic_raycast_cube_hit(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                    if ((cube_hit == 1)) {
                                        Vec3  test_hit_point = heidic_raycast_cube_hit_point(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                        float  dist = ((((test_hit_point.x - create_ray_origin.x) * (test_hit_point.x - create_ray_origin.x)) + ((test_hit_point.y - create_ray_origin.y) * (test_hit_point.y - create_ray_origin.y))) + ((test_hit_point.z - create_ray_origin.z) * (test_hit_point.z - create_ray_origin.z)));
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
                                    float  hit_top_y = (hit_cube_y + (hit_cube_sy / 2));
                                    create_pos.y = (hit_top_y + (default_cube_size / 2));
                                    create_pos.x = hit_point.x;
                                    create_pos.z = hit_point.z;
                                    std::cout << "Hit ground plane! Stacking on top\n" << std::endl;
                                } else {
                                    float  cube_min_x = (hit_cube_x - (hit_cube_sx / 2));
                                    float  cube_max_x = (hit_cube_x + (hit_cube_sx / 2));
                                    float  cube_min_y = (hit_cube_y - (hit_cube_sy / 2));
                                    float  cube_max_y = (hit_cube_y + (hit_cube_sy / 2));
                                    float  cube_min_z = (hit_cube_z - (hit_cube_sz / 2));
                                    float  cube_max_z = (hit_cube_z + (hit_cube_sz / 2));
                                    float  dist_to_left = (hit_point.x - cube_min_x);
                                    float  dist_to_right = (cube_max_x - hit_point.x);
                                    float  dist_to_bottom = (hit_point.y - cube_min_y);
                                    float  dist_to_top = (cube_max_y - hit_point.y);
                                    float  dist_to_back = (hit_point.z - cube_min_z);
                                    float  dist_to_front = (cube_max_z - hit_point.z);
                                    float  min_dist = dist_to_left;
                                    int32_t  hit_face = 0;
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
                        }
                        float  block_r = 1;
                        float  block_g = 1;
                        float  block_b = 1;
                        int32_t  block_index = -1;
                        if ((build_mode == 0)) {
                            block_index = heidic_create_cube_with_color(create_pos.x, create_pos.y, create_pos.z, default_cube_size, default_cube_size, default_cube_size, block_r, block_g, block_b);
                            std::cout << "Created cube at index: " << std::endl;
                        } else {
                            float  wedge_rot_x = 0;
                            float  wedge_rot_y = 0;
                            float  wedge_rot_z = 0;
                            if ((wedge_preview_rotation == 0)) {
                                wedge_rot_x = 270;
                                wedge_rot_y = 180;
                                wedge_rot_z = 90;
                            } else {
                                if ((wedge_preview_rotation == 1)) {
                                    wedge_rot_x = 180;
                                    wedge_rot_y = 90;
                                    wedge_rot_z = 270;
                                } else {
                                    if ((wedge_preview_rotation == 2)) {
                                        wedge_rot_x = 90;
                                        wedge_rot_y = 270;
                                        wedge_rot_z = 180;
                                    } else {
                                        if ((wedge_preview_rotation == 3)) {
                                            wedge_rot_x = 180;
                                            wedge_rot_y = 0;
                                            wedge_rot_z = 270;
                                        } else {
                                            if ((wedge_preview_rotation == 4)) {
                                                wedge_rot_x = 180;
                                                wedge_rot_y = 180;
                                                wedge_rot_z = 90;
                                            } else {
                                                if ((wedge_preview_rotation == 5)) {
                                                    wedge_rot_x = 180;
                                                    wedge_rot_y = 90;
                                                    wedge_rot_z = 90;
                                                } else {
                                                    if ((wedge_preview_rotation == 6)) {
                                                        wedge_rot_x = 180;
                                                        wedge_rot_y = 270;
                                                        wedge_rot_z = 90;
                                                    } else {
                                                        if ((wedge_preview_rotation == 7)) {
                                                            wedge_rot_x = 180;
                                                            wedge_rot_y = 0;
                                                            wedge_rot_z = 90;
                                                        } else {
                                                            if ((wedge_preview_rotation == 8)) {
                                                                wedge_rot_x = 180;
                                                                wedge_rot_y = 90;
                                                                wedge_rot_z = 180;
                                                            } else {
                                                                if ((wedge_preview_rotation == 9)) {
                                                                    wedge_rot_x = 90;
                                                                    wedge_rot_y = 270;
                                                                    wedge_rot_z = 270;
                                                                } else {
                                                                    if ((wedge_preview_rotation == 10)) {
                                                                        wedge_rot_x = 90;
                                                                        wedge_rot_y = 270;
                                                                        wedge_rot_z = 90;
                                                                    } else {
                                                                        if ((wedge_preview_rotation == 11)) {
                                                                            wedge_rot_x = 180;
                                                                            wedge_rot_y = 0;
                                                                            wedge_rot_z = 180;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            block_index = heidic_create_wedge_with_color(create_pos.x, create_pos.y, create_pos.z, default_cube_size, default_cube_size, default_cube_size, block_r, block_g, block_b);
                            heidic_set_wedge_rotation(block_index, wedge_rot_x, wedge_rot_y, wedge_rot_z);
                            std::cout << "Created wedge at index: " << std::endl;
                        }
                        if ((block_index >= 0)) {
                            std::cout << block_index << std::endl;
                            std::cout << "\n" << std::endl;
                            if ((build_mode == 0)) {
                                has_selection = 1;
                                selected_cube_index = (heidic_int_to_float(block_index) + 2);
                                selected_cube_x = create_pos.x;
                                selected_cube_y = create_pos.y;
                                selected_cube_z = create_pos.z;
                                selected_cube_sx = default_cube_size;
                                selected_cube_sy = default_cube_size;
                                selected_cube_sz = default_cube_size;
                                has_wedge_selection = 0;
                                selected_wedge_index = -1;
                                has_mesh_selection = 0;
                                selected_mesh_instance_id = -1;
                            } else {
                                has_wedge_selection = 1;
                                selected_wedge_index = block_index;
                                selected_wedge_x = create_pos.x;
                                selected_wedge_y = create_pos.y;
                                selected_wedge_z = create_pos.z;
                                selected_wedge_sx = default_cube_size;
                                selected_wedge_sy = default_cube_size;
                                selected_wedge_sz = default_cube_size;
                                has_selection = 0;
                                selected_cube_index = -1;
                                has_mesh_selection = 0;
                                selected_mesh_instance_id = -1;
                            }
                        } else {
                            if ((build_mode == 0)) {
                                std::cout << "Failed to create cube\n" << std::endl;
                            } else {
                                std::cout << "Failed to create wedge\n" << std::endl;
                            }
                        }
                        space_was_pressed = 1;
                    }
                } else {
                    space_was_pressed = 0;
                }
                int32_t  key_0_pressed = heidic_is_key_pressed(window, 48);
                if ((key_0_pressed == 1)) {
                    if ((key_0_was_pressed == 0)) {
                        std::cout << "KEY '0' PRESSED! Creating wedge at ray hit point\n" << std::endl;
                        float  default_wedge_size = 100;
                        Vec3  create_ray_origin = heidic_get_mouse_ray_origin(window);
                        Vec3  create_ray_dir = heidic_get_mouse_ray_dir(window);
                        Vec3  create_pos = heidic_vec3(0, 0, 0);
                        int32_t  found_hit = 0;
                        float  ground_cube_x = 0;
                        float  ground_cube_y = -500;
                        float  ground_cube_z = 0;
                        float  ground_cube_sx = 10000;
                        float  ground_cube_sy = 100;
                        float  ground_cube_sz = 10000;
                        int32_t  ground_cube_hit = heidic_raycast_cube_hit(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                        if ((ground_cube_hit == 1)) {
                            Vec3  ground_cube_hit_point = heidic_raycast_cube_hit_point(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                            create_pos.y = ((ground_cube_y + (ground_cube_sy / 2)) + (default_wedge_size / 2));
                            create_pos.x = ground_cube_hit_point.x;
                            create_pos.z = ground_cube_hit_point.z;
                            found_hit = 1;
                        } else {
                            create_pos = heidic_vec3_add(create_ray_origin, heidic_vec3_mul_scalar(create_ray_dir, 500));
                        }
                        float  wedge_r = 1;
                        float  wedge_g = 1;
                        float  wedge_b = 1;
                        int32_t  wedge_index = heidic_create_wedge_with_color(create_pos.x, create_pos.y, create_pos.z, default_wedge_size, default_wedge_size, default_wedge_size, wedge_r, wedge_g, wedge_b);
                        if ((wedge_index >= 0)) {
                            std::cout << "Created wedge at index: " << std::endl;
                            std::cout << wedge_index << std::endl;
                            std::cout << "\n" << std::endl;
                        }
                        key_0_was_pressed = 1;
                    }
                } else {
                    key_0_was_pressed = 0;
                }
                int32_t  m_is_pressed = heidic_is_key_pressed(window, 77);
                int32_t  ctrl_is_down = heidic_ctrl_down(window);
                if (((m_is_pressed == 1) && (ctrl_is_down == 1))) {
                    int32_t  selected_count = heidic_get_selection_count();
                    if ((selected_count > 0)) {
                        std::string  mesh_filepath = "meshes/combined_mesh.hdm";
                        int32_t  result = heidic_combine_selected_cubes_to_mesh(mesh_filepath.c_str());
                        if ((result == 1)) {
                            std::cout << "Successfully combined selected cubes into mesh: " << std::endl;
                            std::cout << mesh_filepath << std::endl;
                            std::cout << "\n" << std::endl;
                        } else {
                            std::cout << "Failed to combine cubes into mesh\n" << std::endl;
                        }
                    }
                }
                int32_t  s_is_pressed = heidic_is_key_pressed(window, 83);
                int32_t  s_shift_is_pressed = 0;
                int32_t  s_left_shift = heidic_is_key_pressed(window, 340);
                int32_t  s_right_shift = heidic_is_key_pressed(window, 344);
                if ((s_left_shift == 1)) {
                    s_shift_is_pressed = 1;
                }
                if ((s_right_shift == 1)) {
                    s_shift_is_pressed = 1;
                }
                if (((s_is_pressed == 1) && (s_shift_is_pressed == 1))) {
                    if ((smooth_s_was_pressed == 0)) {
                        int32_t  selected_count = heidic_get_selection_count();
                        if ((selected_count > 0)) {
                            int32_t  result = heidic_smooth_selected_cubes();
                            if ((result == 1)) {
                                std::cout << "Successfully smoothed selected cubes\n" << std::endl;
                            } else {
                                std::cout << "Failed to smooth cubes\n" << std::endl;
                            }
                        } else {
                            std::cout << "No cubes selected for smoothing\n" << std::endl;
                        }
                        smooth_s_was_pressed = 1;
                    }
                } else {
                    smooth_s_was_pressed = 0;
                }
                int32_t  c_is_pressed = heidic_is_key_pressed(window, 67);
                if ((c_is_pressed == 1)) {
                    if ((combine_c_was_pressed == 0)) {
                        int32_t  selection_count = heidic_get_selection_count();
                        if ((selection_count > 0)) {
                            std::cout << "C pressed - Combining selected cubes\n" << std::endl;
                            heidic_combine_selected_cubes();
                            std::cout << "Combination complete\n" << std::endl;
                        } else {
                        }
                    }
                    combine_c_was_pressed = 1;
                } else {
                    combine_c_was_pressed = 0;
                }
                int32_t  delete_is_pressed = heidic_is_key_pressed(window, 261);
                if ((delete_is_pressed == 1)) {
                    if ((delete_was_pressed == 0)) {
                        if (((has_selection == 1) && (selected_cube_index >= 2))) {
                            int32_t  cube_vector_index = heidic_float_to_int((selected_cube_index - 2));
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
                int32_t  enter_is_pressed = heidic_is_key_pressed(window, 257);
                int32_t  left_shift_is_pressed = heidic_is_key_pressed(window, 340);
                int32_t  right_shift_is_pressed = heidic_is_key_pressed(window, 344);
                int32_t  shift_is_pressed = 0;
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
                float  rot_y_rad = heidic_convert_degrees_to_radians(player_rot.y);
                float  forward_x = -heidic_sin(rot_y_rad);
                float  forward_z = -heidic_cos(rot_y_rad);
                float  right_x = heidic_cos(rot_y_rad);
                float  right_z = -heidic_sin(rot_y_rad);
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
                    Vec3  offset = heidic_vec3(0, 100, 0);
                    Vec3  offset_pos = heidic_vec3_add(player_pos, offset);
                    camera_pos = heidic_attach_camera_translation(offset_pos);
                    camera_rot = heidic_attach_camera_rotation(player_rot);
                } else {
                    int32_t  left_ctrl_pressed = heidic_is_key_pressed(window, 341);
                    int32_t  right_ctrl_pressed = heidic_is_key_pressed(window, 345);
                    int32_t  ctrl_pressed = 0;
                    if ((left_ctrl_pressed == 1)) {
                        ctrl_pressed = 1;
                    }
                    if ((right_ctrl_pressed == 1)) {
                        ctrl_pressed = 1;
                    }
                    int32_t  right_mouse_pressed = heidic_is_mouse_button_pressed(window, 1);
                    if (((ctrl_pressed == 1) && (right_mouse_pressed == 1))) {
                        float  mouse_delta_y = heidic_get_mouse_delta_y(window);
                        if ((mouse_delta_y != 0)) {
                            if (((alt_pressed == 1) && (has_selection == 1))) {
                                float  dolly_speed = 50;
                                dolly_orbit_distance = (dolly_orbit_distance - (mouse_delta_y * dolly_speed));
                                if ((dolly_orbit_distance < 10)) {
                                    dolly_orbit_distance = 10;
                                }
                                if ((dolly_orbit_distance > 100000)) {
                                    dolly_orbit_distance = 100000;
                                }
                            } else {
                                float  zoom_speed_factor = (topdown_cam_height / 10000);
                                float  zoom_speed = (50 * zoom_speed_factor);
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
                        float  zoom_speed = 100;
                        topdown_cam_height = (topdown_cam_height - zoom_speed);
                        if ((topdown_cam_height < 100)) {
                            topdown_cam_height = 100;
                        }
                    }
                    if ((heidic_is_key_pressed(window, 93) == 1)) {
                        float  zoom_speed = 100;
                        topdown_cam_height = (topdown_cam_height + zoom_speed);
                        if ((topdown_cam_height > 50000)) {
                            topdown_cam_height = 50000;
                        }
                    }
                    int32_t  mouse_middle_pressed = heidic_is_mouse_button_pressed(window, 2);
                    if ((mouse_middle_pressed == 1)) {
                        float  mouse_delta_x = heidic_get_mouse_delta_x(window);
                        float  mouse_delta_y = heidic_get_mouse_delta_y(window);
                        if (((mouse_delta_x != 0) || (mouse_delta_y != 0))) {
                            float  pan_speed_factor = (topdown_cam_height / 10000);
                            float  pan_speed = (4 * pan_speed_factor);
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
                            int32_t  save_result = heidic_show_save_dialog();
                            if ((save_result == 1)) {
                            }
                        }
                        if ((heidic_imgui_menu_item("Open Level...") == 1)) {
                            int32_t  load_result = heidic_show_open_dialog();
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
                            float  default_cube_size = 200;
                            int32_t  new_cube_index = heidic_create_cube(0, 0, 0, default_cube_size, default_cube_size, default_cube_size);
                            selected_cube_index = (heidic_int_to_float(new_cube_index) + 2);
                            has_selection = 1;
                            selected_cube_x = 0;
                            selected_cube_y = 0;
                            selected_cube_z = 0;
                            selected_cube_sx = default_cube_size;
                            selected_cube_sy = default_cube_size;
                            selected_cube_sz = default_cube_size;
                        }
                        if ((heidic_imgui_menu_item("Load Mesh") == 1)) {
                            int32_t  mesh_id = heidic_show_open_mesh_dialog();
                            if ((mesh_id >= 0)) {
                                std::cout << "Successfully loaded mesh with ID: " << std::endl;
                                std::cout << mesh_id << std::endl;
                                std::cout << "\n" << std::endl;
                            } else {
                                std::cout << "Failed to load mesh or user cancelled\n" << std::endl;
                            }
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
                heidic_load_texture_for_rendering("default.bmp");
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
                int32_t  cube_draw_index = 0;
                int32_t  total_cubes = heidic_get_cube_total_count();
                std::string  current_texture = "";
                int32_t  first_cube = 1;
                while ((cube_draw_index < total_cubes)) {
                    if ((heidic_get_cube_active(cube_draw_index) == 1)) {
                        std::string  cube_texture_name = heidic_get_cube_texture_name(cube_draw_index);
                        if ((cube_texture_name == "")) {
                            cube_texture_name = "default.bmp";
                        }
                        if (((first_cube == 0) && (cube_texture_name != current_texture))) {
                            heidic_flush_colored_cubes();
                            heidic_load_texture_for_rendering(cube_texture_name.c_str());
                            current_texture = cube_texture_name;
                        } else {
                            if ((first_cube == 1)) {
                                heidic_load_texture_for_rendering(cube_texture_name.c_str());
                                current_texture = cube_texture_name;
                                first_cube = 0;
                            }
                        }
                        float  cube_x = heidic_get_cube_x(cube_draw_index);
                        float  cube_y = heidic_get_cube_y(cube_draw_index);
                        float  cube_z = heidic_get_cube_z(cube_draw_index);
                        float  cube_sx = heidic_get_cube_sx(cube_draw_index);
                        float  cube_sy = heidic_get_cube_sy(cube_draw_index);
                        float  cube_sz = heidic_get_cube_sz(cube_draw_index);
                        float  cube_rx = heidic_get_cube_rx(cube_draw_index);
                        float  cube_ry = heidic_get_cube_ry(cube_draw_index);
                        float  cube_rz = heidic_get_cube_rz(cube_draw_index);
                        heidic_draw_cube_colored(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, cube_sx, cube_sy, cube_sz, 1, 1, 1);
                        if ((heidic_is_cube_selected(cube_draw_index) == 1)) {
                            heidic_draw_cube_wireframe(cube_x, cube_y, cube_z, cube_rx, cube_ry, cube_rz, (cube_sx * 1.01), (cube_sy * 1.01), (cube_sz * 1.01), 1, 1, 1);
                        }
                    }
                    cube_draw_index = (cube_draw_index + 1);
                }
                heidic_flush_colored_cubes();
                int32_t  wedge_total_count = heidic_get_wedge_total_count();
                int32_t  wedge_draw_index = 0;
                int32_t  first_wedge = 1;
                std::string  current_wedge_texture = "";
                while ((wedge_draw_index < wedge_total_count)) {
                    if ((heidic_get_wedge_active(wedge_draw_index) == 1)) {
                        std::string  wedge_texture_name = heidic_get_wedge_texture_name(wedge_draw_index);
                        if ((wedge_texture_name == "")) {
                            wedge_texture_name = "default.bmp";
                        }
                        if (((first_wedge == 0) && (wedge_texture_name != current_wedge_texture))) {
                            heidic_flush_colored_cubes();
                            heidic_load_texture_for_rendering(wedge_texture_name.c_str());
                            current_wedge_texture = wedge_texture_name;
                        } else {
                            if ((first_wedge == 1)) {
                                heidic_load_texture_for_rendering(wedge_texture_name.c_str());
                                current_wedge_texture = wedge_texture_name;
                                first_wedge = 0;
                            }
                        }
                        float  wedge_x = heidic_get_wedge_x(wedge_draw_index);
                        float  wedge_y = heidic_get_wedge_y(wedge_draw_index);
                        float  wedge_z = heidic_get_wedge_z(wedge_draw_index);
                        float  wedge_sx = heidic_get_wedge_sx(wedge_draw_index);
                        float  wedge_sy = heidic_get_wedge_sy(wedge_draw_index);
                        float  wedge_sz = heidic_get_wedge_sz(wedge_draw_index);
                        float  wedge_rx = heidic_get_wedge_rx(wedge_draw_index);
                        float  wedge_ry = heidic_get_wedge_ry(wedge_draw_index);
                        float  wedge_rz = heidic_get_wedge_rz(wedge_draw_index);
                        heidic_draw_wedge_colored(wedge_x, wedge_y, wedge_z, wedge_rx, wedge_ry, wedge_rz, wedge_sx, wedge_sy, wedge_sz, 1, 1, 1);
                    }
                    wedge_draw_index = (wedge_draw_index + 1);
                }
                heidic_flush_colored_cubes();
                int32_t  mesh_instance_count = heidic_get_mesh_instance_total_count();
                if ((mesh_instance_count > 0)) {
                    heidic_load_texture_for_rendering("default.bmp");
                    int32_t  mesh_instance_index = 0;
                    while ((mesh_instance_index < mesh_instance_count)) {
                        if ((heidic_get_mesh_instance_active(mesh_instance_index) == 1)) {
                            int32_t  instance_mesh_id = heidic_get_mesh_instance_mesh_id(mesh_instance_index);
                            float  instance_x = heidic_get_mesh_instance_x(mesh_instance_index);
                            float  instance_y = heidic_get_mesh_instance_y(mesh_instance_index);
                            float  instance_z = heidic_get_mesh_instance_z(mesh_instance_index);
                            float  mesh_center_x = heidic_get_mesh_instance_center_x(mesh_instance_index);
                            float  mesh_center_y = heidic_get_mesh_instance_center_y(mesh_instance_index);
                            float  mesh_center_z = heidic_get_mesh_instance_center_z(mesh_instance_index);
                            float  mesh_sx = heidic_get_mesh_instance_sx(mesh_instance_index);
                            float  mesh_sy = heidic_get_mesh_instance_sy(mesh_instance_index);
                            float  mesh_sz = heidic_get_mesh_instance_sz(mesh_instance_index);
                            float  mesh_rx = heidic_get_mesh_instance_rx(mesh_instance_index);
                            float  mesh_ry = heidic_get_mesh_instance_ry(mesh_instance_index);
                            float  mesh_rz = heidic_get_mesh_instance_rz(mesh_instance_index);
                            float  world_x = (instance_x + mesh_center_x);
                            float  world_y = (instance_y + mesh_center_y);
                            float  world_z = (instance_z + mesh_center_z);
                            heidic_draw_mesh_scaled_with_center(instance_mesh_id, instance_x, instance_y, instance_z, mesh_rx, mesh_ry, mesh_rz, mesh_sx, mesh_sy, mesh_sz, mesh_center_x, mesh_center_y, mesh_center_z);
                            if ((heidic_is_mesh_selected(mesh_instance_index) == 1)) {
                                float  bbox_min_x = heidic_get_mesh_instance_bbox_min_x(mesh_instance_index);
                                float  bbox_min_y = heidic_get_mesh_instance_bbox_min_y(mesh_instance_index);
                                float  bbox_min_z = heidic_get_mesh_instance_bbox_min_z(mesh_instance_index);
                                float  bbox_max_x = heidic_get_mesh_instance_bbox_max_x(mesh_instance_index);
                                float  bbox_max_y = heidic_get_mesh_instance_bbox_max_y(mesh_instance_index);
                                float  bbox_max_z = heidic_get_mesh_instance_bbox_max_z(mesh_instance_index);
                                float  scaled_min_x = (bbox_min_x * mesh_sx);
                                float  scaled_min_y = (bbox_min_y * mesh_sy);
                                float  scaled_min_z = (bbox_min_z * mesh_sz);
                                float  scaled_max_x = (bbox_max_x * mesh_sx);
                                float  scaled_max_y = (bbox_max_y * mesh_sy);
                                float  scaled_max_z = (bbox_max_z * mesh_sz);
                                float  bbox_center_x = (instance_x + mesh_center_x);
                                float  bbox_center_y = (instance_y + mesh_center_y);
                                float  bbox_center_z = (instance_z + mesh_center_z);
                                float  bbox_size_x = (scaled_max_x - scaled_min_x);
                                float  bbox_size_y = (scaled_max_y - scaled_min_y);
                                float  bbox_size_z = (scaled_max_z - scaled_min_z);
                                heidic_draw_cube_wireframe(bbox_center_x, bbox_center_y, bbox_center_z, mesh_rx, mesh_ry, mesh_rz, bbox_size_x, bbox_size_y, bbox_size_z, 1, 1, 0);
                            }
                        }
                        mesh_instance_index = (mesh_instance_index + 1);
                    }
                    std::cout << "[HEIDIC DEBUG] After mesh instance loop\n" << std::endl;
                } else {
                }
                heidic_draw_ray(window, 50000, 1, 1, 0);
                if ((1 == 1)) {
                    Vec3  debug_ray_origin = heidic_get_mouse_ray_origin(window);
                    Vec3  debug_ray_dir = heidic_get_mouse_ray_dir(window);
                    Vec3  debug_hit_pos = heidic_vec3(0, 0, 0);
                    int32_t  debug_found_hit = 0;
                    float  debug_closest_dist = 100000000000;
                    float  debug_hit_cube_x = 0;
                    float  debug_hit_cube_y = 0;
                    float  debug_hit_cube_z = 0;
                    float  debug_hit_cube_sx = 0;
                    float  debug_hit_cube_sy = 0;
                    float  debug_hit_cube_sz = 0;
                    int32_t  debug_hit_face = -1;
                    int32_t  debug_is_ground_plane = 0;
                    int32_t  debug_is_wedge = 0;
                    float  ground_cube_x = 0;
                    float  ground_cube_y = -500;
                    float  ground_cube_z = 0;
                    float  ground_cube_sx = 10000;
                    float  ground_cube_sy = 100;
                    float  ground_cube_sz = 10000;
                    int32_t  ground_cube_hit = heidic_raycast_cube_hit(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                    if ((ground_cube_hit == 1)) {
                        Vec3  ground_cube_hit_point = heidic_raycast_cube_hit_point(window, ground_cube_x, ground_cube_y, ground_cube_z, ground_cube_sx, ground_cube_sy, ground_cube_sz);
                        float  ground_cube_dist = ((((ground_cube_hit_point.x - debug_ray_origin.x) * (ground_cube_hit_point.x - debug_ray_origin.x)) + ((ground_cube_hit_point.y - debug_ray_origin.y) * (ground_cube_hit_point.y - debug_ray_origin.y))) + ((ground_cube_hit_point.z - debug_ray_origin.z) * (ground_cube_hit_point.z - debug_ray_origin.z)));
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
                    int32_t  debug_cube_index = 0;
                    int32_t  debug_total_cubes = heidic_get_cube_total_count();
                    while ((debug_cube_index < debug_total_cubes)) {
                        if ((heidic_get_cube_active(debug_cube_index) == 1)) {
                            float  debug_cube_x = heidic_get_cube_x(debug_cube_index);
                            float  debug_cube_y = heidic_get_cube_y(debug_cube_index);
                            float  debug_cube_z = heidic_get_cube_z(debug_cube_index);
                            float  debug_cube_sx = heidic_get_cube_sx(debug_cube_index);
                            float  debug_cube_sy = heidic_get_cube_sy(debug_cube_index);
                            float  debug_cube_sz = heidic_get_cube_sz(debug_cube_index);
                            int32_t  debug_cube_hit = heidic_raycast_cube_hit(window, debug_cube_x, debug_cube_y, debug_cube_z, debug_cube_sx, debug_cube_sy, debug_cube_sz);
                            if ((debug_cube_hit == 1)) {
                                Vec3  debug_hit_point = heidic_raycast_cube_hit_point(window, debug_cube_x, debug_cube_y, debug_cube_z, debug_cube_sx, debug_cube_sy, debug_cube_sz);
                                float  debug_dist = ((((debug_hit_point.x - debug_ray_origin.x) * (debug_hit_point.x - debug_ray_origin.x)) + ((debug_hit_point.y - debug_ray_origin.y) * (debug_hit_point.y - debug_ray_origin.y))) + ((debug_hit_point.z - debug_ray_origin.z) * (debug_hit_point.z - debug_ray_origin.z)));
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
                    int32_t  debug_wedge_index = 0;
                    int32_t  debug_total_wedges = heidic_get_wedge_total_count();
                    while ((debug_wedge_index < debug_total_wedges)) {
                        if ((debug_wedge_index == 0)) {
                            std::cout << "[HEIDIC DEBUG] First iteration of wedge raycast loop\n" << std::endl;
                        }
                        if ((heidic_get_wedge_active(debug_wedge_index) == 1)) {
                            if ((debug_wedge_index == 0)) {
                                std::cout << "[HEIDIC DEBUG] First wedge is active, getting position\n" << std::endl;
                            }
                            float  debug_wedge_x = heidic_get_wedge_x(debug_wedge_index);
                            float  debug_wedge_y = heidic_get_wedge_y(debug_wedge_index);
                            float  debug_wedge_z = heidic_get_wedge_z(debug_wedge_index);
                            float  debug_wedge_sx = heidic_get_wedge_sx(debug_wedge_index);
                            float  debug_wedge_sy = heidic_get_wedge_sy(debug_wedge_index);
                            float  debug_wedge_sz = heidic_get_wedge_sz(debug_wedge_index);
                            if ((debug_wedge_index == 0)) {
                                std::cout << "[HEIDIC DEBUG] About to raycast wedge\n" << std::endl;
                            }
                            int32_t  debug_wedge_hit = heidic_raycast_cube_hit(window, debug_wedge_x, debug_wedge_y, debug_wedge_z, debug_wedge_sx, debug_wedge_sy, debug_wedge_sz);
                            if ((debug_wedge_index == 0)) {
                                std::cout << "[HEIDIC DEBUG] After wedge raycast call\n" << std::endl;
                            }
                            if ((debug_wedge_hit == 1)) {
                                Vec3  debug_wedge_hit_point = heidic_raycast_cube_hit_point(window, debug_wedge_x, debug_wedge_y, debug_wedge_z, debug_wedge_sx, debug_wedge_sy, debug_wedge_sz);
                                float  debug_wedge_dist = ((((debug_wedge_hit_point.x - debug_ray_origin.x) * (debug_wedge_hit_point.x - debug_ray_origin.x)) + ((debug_wedge_hit_point.y - debug_ray_origin.y) * (debug_wedge_hit_point.y - debug_ray_origin.y))) + ((debug_wedge_hit_point.z - debug_ray_origin.z) * (debug_wedge_hit_point.z - debug_ray_origin.z)));
                                if ((debug_wedge_dist < debug_closest_dist)) {
                                    debug_closest_dist = debug_wedge_dist;
                                    debug_hit_pos = debug_wedge_hit_point;
                                    debug_hit_cube_x = debug_wedge_x;
                                    debug_hit_cube_y = debug_wedge_y;
                                    debug_hit_cube_z = debug_wedge_z;
                                    debug_hit_cube_sx = debug_wedge_sx;
                                    debug_hit_cube_sy = debug_wedge_sy;
                                    debug_hit_cube_sz = debug_wedge_sz;
                                    debug_is_ground_plane = 0;
                                    debug_is_wedge = 1;
                                    debug_found_hit = 1;
                                }
                            }
                        }
                        debug_wedge_index = (debug_wedge_index + 1);
                    }
                    int32_t  debug_mesh_instance_index = 0;
                    int32_t  debug_total_mesh_instances = heidic_get_mesh_instance_total_count();
                    while ((debug_mesh_instance_index < debug_total_mesh_instances)) {
                        if ((heidic_get_mesh_instance_active(debug_mesh_instance_index) == 1)) {
                            int32_t  debug_mesh_hit = heidic_raycast_mesh_bbox_hit(window, debug_mesh_instance_index);
                            if ((debug_mesh_hit == 1)) {
                                Vec3  debug_mesh_hit_point = heidic_raycast_mesh_bbox_hit_point(window, debug_mesh_instance_index);
                                float  debug_mesh_dist = ((((debug_mesh_hit_point.x - debug_ray_origin.x) * (debug_mesh_hit_point.x - debug_ray_origin.x)) + ((debug_mesh_hit_point.y - debug_ray_origin.y) * (debug_mesh_hit_point.y - debug_ray_origin.y))) + ((debug_mesh_hit_point.z - debug_ray_origin.z) * (debug_mesh_hit_point.z - debug_ray_origin.z)));
                                if ((debug_mesh_dist < debug_closest_dist)) {
                                    debug_closest_dist = debug_mesh_dist;
                                    debug_hit_pos = debug_mesh_hit_point;
                                    debug_hit_cube_x = debug_mesh_hit_point.x;
                                    debug_hit_cube_y = debug_mesh_hit_point.y;
                                    debug_hit_cube_z = debug_mesh_hit_point.z;
                                    debug_hit_cube_sx = 100;
                                    debug_hit_cube_sy = 100;
                                    debug_hit_cube_sz = 100;
                                    debug_is_ground_plane = 0;
                                    debug_found_hit = 1;
                                }
                            }
                        }
                        debug_mesh_instance_index = (debug_mesh_instance_index + 1);
                    }
                    Vec3  debug_placement_pos = debug_hit_pos;
                    float  debug_default_cube_size = 100;
                    if ((debug_found_hit == 0)) {
                        debug_hit_pos = heidic_vec3_add(debug_ray_origin, heidic_vec3_mul_scalar(debug_ray_dir, 500));
                        debug_placement_pos = debug_hit_pos;
                        stored_preview_pos = debug_placement_pos;
                        stored_preview_valid = 1;
                    }
                    if (((debug_found_hit == 1) && (debug_is_ground_plane == 0))) {
                        float  debug_cube_min_x = (debug_hit_cube_x - (debug_hit_cube_sx / 2));
                        float  debug_cube_max_x = (debug_hit_cube_x + (debug_hit_cube_sx / 2));
                        float  debug_cube_min_y = (debug_hit_cube_y - (debug_hit_cube_sy / 2));
                        float  debug_cube_max_y = (debug_hit_cube_y + (debug_hit_cube_sy / 2));
                        float  debug_cube_min_z = (debug_hit_cube_z - (debug_hit_cube_sz / 2));
                        float  debug_cube_max_z = (debug_hit_cube_z + (debug_hit_cube_sz / 2));
                        float  debug_dist_to_left = (debug_hit_pos.x - debug_cube_min_x);
                        float  debug_dist_to_right = (debug_cube_max_x - debug_hit_pos.x);
                        float  debug_dist_to_bottom = (debug_hit_pos.y - debug_cube_min_y);
                        float  debug_dist_to_top = (debug_cube_max_y - debug_hit_pos.y);
                        float  debug_dist_to_back = (debug_hit_pos.z - debug_cube_min_z);
                        float  debug_dist_to_front = (debug_cube_max_z - debug_hit_pos.z);
                        float  debug_min_dist = debug_dist_to_left;
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
                        if ((debug_hit_face == 0)) {
                            debug_placement_pos.x = (debug_cube_min_x - (debug_default_cube_size / 2));
                            debug_placement_pos.y = debug_hit_cube_y;
                            debug_placement_pos.z = debug_hit_cube_z;
                        } else {
                            if ((debug_hit_face == 1)) {
                                debug_placement_pos.x = (debug_cube_max_x + (debug_default_cube_size / 2));
                                debug_placement_pos.y = debug_hit_cube_y;
                                debug_placement_pos.z = debug_hit_cube_z;
                            } else {
                                if ((debug_hit_face == 2)) {
                                    debug_placement_pos.x = debug_hit_cube_x;
                                    debug_placement_pos.y = (debug_cube_min_y - (debug_default_cube_size / 2));
                                    debug_placement_pos.z = debug_hit_cube_z;
                                } else {
                                    if ((debug_hit_face == 3)) {
                                        debug_placement_pos.x = debug_hit_cube_x;
                                        debug_placement_pos.y = (debug_cube_max_y + (debug_default_cube_size / 2));
                                        debug_placement_pos.z = debug_hit_cube_z;
                                    } else {
                                        if ((debug_hit_face == 4)) {
                                            debug_placement_pos.x = debug_hit_cube_x;
                                            debug_placement_pos.y = debug_hit_cube_y;
                                            debug_placement_pos.z = (debug_cube_min_z - (debug_default_cube_size / 2));
                                        } else {
                                            debug_placement_pos.x = debug_hit_cube_x;
                                            debug_placement_pos.y = debug_hit_cube_y;
                                            debug_placement_pos.z = (debug_cube_max_z + (debug_default_cube_size / 2));
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (((debug_found_hit == 1) && (debug_is_ground_plane == 1))) {
                            float  hit_top_y = (debug_hit_cube_y + (debug_hit_cube_sy / 2));
                            debug_placement_pos.y = (hit_top_y + (debug_default_cube_size / 2));
                            debug_placement_pos.x = debug_hit_pos.x;
                            debug_placement_pos.z = debug_hit_pos.z;
                        }
                    }
                    stored_preview_pos = debug_placement_pos;
                    stored_preview_valid = 1;
                    if ((build_mode == 0)) {
                        heidic_draw_cube_wireframe(debug_placement_pos.x, debug_placement_pos.y, debug_placement_pos.z, 0, 0, 0, 100, 100, 100, 1, 0, 0);
                    } else {
                        float  wedge_rot_x = 0;
                        float  wedge_rot_y = 0;
                        float  wedge_rot_z = 0;
                        if ((wedge_preview_rotation == 0)) {
                            wedge_rot_x = 270;
                            wedge_rot_y = 180;
                            wedge_rot_z = 90;
                        } else {
                            if ((wedge_preview_rotation == 1)) {
                                wedge_rot_x = 180;
                                wedge_rot_y = 90;
                                wedge_rot_z = 270;
                            } else {
                                if ((wedge_preview_rotation == 2)) {
                                    wedge_rot_x = 90;
                                    wedge_rot_y = 270;
                                    wedge_rot_z = 180;
                                } else {
                                    if ((wedge_preview_rotation == 3)) {
                                        wedge_rot_x = 180;
                                        wedge_rot_y = 0;
                                        wedge_rot_z = 270;
                                    } else {
                                        if ((wedge_preview_rotation == 4)) {
                                            wedge_rot_x = 180;
                                            wedge_rot_y = 180;
                                            wedge_rot_z = 90;
                                        } else {
                                            if ((wedge_preview_rotation == 5)) {
                                                wedge_rot_x = 180;
                                                wedge_rot_y = 90;
                                                wedge_rot_z = 90;
                                            } else {
                                                if ((wedge_preview_rotation == 6)) {
                                                    wedge_rot_x = 180;
                                                    wedge_rot_y = 270;
                                                    wedge_rot_z = 90;
                                                } else {
                                                    if ((wedge_preview_rotation == 7)) {
                                                        wedge_rot_x = 180;
                                                        wedge_rot_y = 0;
                                                        wedge_rot_z = 90;
                                                    } else {
                                                        if ((wedge_preview_rotation == 8)) {
                                                            wedge_rot_x = 180;
                                                            wedge_rot_y = 90;
                                                            wedge_rot_z = 180;
                                                        } else {
                                                            if ((wedge_preview_rotation == 9)) {
                                                                wedge_rot_x = 90;
                                                                wedge_rot_y = 270;
                                                                wedge_rot_z = 270;
                                                            } else {
                                                                if ((wedge_preview_rotation == 10)) {
                                                                    wedge_rot_x = 90;
                                                                    wedge_rot_y = 270;
                                                                    wedge_rot_z = 90;
                                                                } else {
                                                                    if ((wedge_preview_rotation == 11)) {
                                                                        wedge_rot_x = 180;
                                                                        wedge_rot_y = 0;
                                                                        wedge_rot_z = 180;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        last_wedge_preview_rotation = wedge_preview_rotation;
                        heidic_draw_wedge_wireframe(debug_placement_pos.x, debug_placement_pos.y, debug_placement_pos.z, wedge_rot_x, wedge_rot_y, wedge_rot_z, 100, 100, 100, 1, 0, 0);
                    }
                }
                int32_t  mouse_left_pressed = heidic_is_mouse_button_pressed(window, 0);
                int32_t  mouse_right_pressed = heidic_is_mouse_button_pressed(window, 1);
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
                    } else {
                    }
                } else {
                    mouse_mode_right_was_pressed = 0;
                }
                int32_t  gizmo_clicked = 0;
                if ((((((mouse_mode == 1) && (mouse_left_pressed == 1)) && (alt_pressed == 0)) && (has_mesh_selection == 1)) && (selected_mesh_instance_id >= 0))) {
                    float  mesh_center_x = heidic_get_mesh_instance_center_x(selected_mesh_instance_id);
                    float  mesh_center_y = heidic_get_mesh_instance_center_y(selected_mesh_instance_id);
                    float  mesh_center_z = heidic_get_mesh_instance_center_z(selected_mesh_instance_id);
                    float  gizmo_x = (selected_mesh_x + mesh_center_x);
                    float  gizmo_y = (selected_mesh_y + mesh_center_y);
                    float  gizmo_z = (selected_mesh_z + mesh_center_z);
                    float  current_scale = heidic_get_mesh_instance_sx(selected_mesh_instance_id);
                    Vec3  temp_pos = heidic_gizmo_translate(window, gizmo_x, gizmo_y, gizmo_z);
                    int32_t  translate_interacting = heidic_gizmo_is_interacting();
                    float  bbox_min_x = heidic_get_mesh_instance_bbox_min_x(selected_mesh_instance_id);
                    float  bbox_min_y = heidic_get_mesh_instance_bbox_min_y(selected_mesh_instance_id);
                    float  bbox_min_z = heidic_get_mesh_instance_bbox_min_z(selected_mesh_instance_id);
                    float  bbox_max_x = heidic_get_mesh_instance_bbox_max_x(selected_mesh_instance_id);
                    float  bbox_max_y = heidic_get_mesh_instance_bbox_max_y(selected_mesh_instance_id);
                    float  bbox_max_z = heidic_get_mesh_instance_bbox_max_z(selected_mesh_instance_id);
                    float  temp_scale = heidic_gizmo_scale(window, gizmo_x, gizmo_y, gizmo_z, bbox_min_x, bbox_min_y, bbox_min_z, bbox_max_x, bbox_max_y, bbox_max_z, current_scale);
                    int32_t  scale_interacting = heidic_gizmo_scale_is_interacting();
                    if (((translate_interacting == 1) || (scale_interacting == 1))) {
                        gizmo_clicked = 1;
                    }
                } else {
                }
                if ((((mouse_mode == 1) && (mouse_left_pressed == 1)) && (alt_pressed == 0))) {
                    int32_t  ctrl_left_pressed = heidic_is_key_pressed(window, 341);
                    int32_t  ctrl_right_pressed = heidic_is_key_pressed(window, 345);
                    int32_t  ctrl_pressed = 0;
                    if (((ctrl_left_pressed == 1) || (ctrl_right_pressed == 1))) {
                        ctrl_pressed = 1;
                    }
                    int32_t  interacting = 0;
                    int32_t  scale_interacting = 0;
                    if ((mouse_left_was_pressed == 1)) {
                        interacting = heidic_gizmo_is_interacting();
                        scale_interacting = heidic_gizmo_scale_is_interacting();
                    }
                    if (((gizmo_clicked == 0) && ((interacting == 0) && (scale_interacting == 0)))) {
                        if ((mouse_left_was_pressed == 0)) {
                            float  closest_dist = 100000000000;
                            int32_t  hit_cube_index = -1;
                            int32_t  cube_test_index = 0;
                            int32_t  total_cubes_test = heidic_get_cube_total_count();
                            while ((cube_test_index < total_cubes_test)) {
                                if ((heidic_get_cube_active(cube_test_index) == 1)) {
                                    float  test_cube_x = heidic_get_cube_x(cube_test_index);
                                    float  test_cube_y = heidic_get_cube_y(cube_test_index);
                                    float  test_cube_z = heidic_get_cube_z(cube_test_index);
                                    float  test_cube_sx = heidic_get_cube_sx(cube_test_index);
                                    float  test_cube_sy = heidic_get_cube_sy(cube_test_index);
                                    float  test_cube_sz = heidic_get_cube_sz(cube_test_index);
                                    int32_t  created_hit = heidic_raycast_cube_hit(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                    if ((created_hit == 1)) {
                                        Vec3  hit_point = heidic_raycast_cube_hit_point(window, test_cube_x, test_cube_y, test_cube_z, test_cube_sx, test_cube_sy, test_cube_sz);
                                        float  dist = ((((hit_point.x - ray_origin.x) * (hit_point.x - ray_origin.x)) + ((hit_point.y - ray_origin.y) * (hit_point.y - ray_origin.y))) + ((hit_point.z - ray_origin.z) * (hit_point.z - ray_origin.z)));
                                        if ((dist < closest_dist)) {
                                            closest_dist = dist;
                                            hit_cube_index = cube_test_index;
                                        }
                                    }
                                }
                                cube_test_index = (cube_test_index + 1);
                            }
                            int32_t  wedge_test_index = 0;
                            int32_t  total_wedges_test = heidic_get_wedge_total_count();
                            int32_t  hit_wedge_index = -1;
                            while ((wedge_test_index < total_wedges_test)) {
                                if ((heidic_get_wedge_active(wedge_test_index) == 1)) {
                                    float  test_wedge_x = heidic_get_wedge_x(wedge_test_index);
                                    float  test_wedge_y = heidic_get_wedge_y(wedge_test_index);
                                    float  test_wedge_z = heidic_get_wedge_z(wedge_test_index);
                                    float  test_wedge_sx = heidic_get_wedge_sx(wedge_test_index);
                                    float  test_wedge_sy = heidic_get_wedge_sy(wedge_test_index);
                                    float  test_wedge_sz = heidic_get_wedge_sz(wedge_test_index);
                                    int32_t  wedge_hit = heidic_raycast_cube_hit(window, test_wedge_x, test_wedge_y, test_wedge_z, test_wedge_sx, test_wedge_sy, test_wedge_sz);
                                    if ((wedge_hit == 1)) {
                                        Vec3  hit_point = heidic_raycast_cube_hit_point(window, test_wedge_x, test_wedge_y, test_wedge_z, test_wedge_sx, test_wedge_sy, test_wedge_sz);
                                        float  dist = ((((hit_point.x - ray_origin.x) * (hit_point.x - ray_origin.x)) + ((hit_point.y - ray_origin.y) * (hit_point.y - ray_origin.y))) + ((hit_point.z - ray_origin.z) * (hit_point.z - ray_origin.z)));
                                        if ((dist < closest_dist)) {
                                            closest_dist = dist;
                                            hit_wedge_index = wedge_test_index;
                                            hit_cube_index = -1;
                                        }
                                    }
                                }
                                wedge_test_index = (wedge_test_index + 1);
                            }
                            if ((hit_wedge_index >= 0)) {
                                heidic_clear_selection();
                                heidic_clear_mesh_selection();
                                selected_wedge_index = hit_wedge_index;
                                selected_wedge_x = heidic_get_wedge_x(hit_wedge_index);
                                selected_wedge_y = heidic_get_wedge_y(hit_wedge_index);
                                selected_wedge_z = heidic_get_wedge_z(hit_wedge_index);
                                selected_wedge_sx = heidic_get_wedge_sx(hit_wedge_index);
                                selected_wedge_sy = heidic_get_wedge_sy(hit_wedge_index);
                                selected_wedge_sz = heidic_get_wedge_sz(hit_wedge_index);
                                has_wedge_selection = 1;
                                has_selection = 0;
                                has_mesh_selection = 0;
                                selected_cube_index = -1;
                                selected_mesh_instance_id = -1;
                            } else {
                                if ((hit_cube_index >= 0)) {
                                    if ((ctrl_pressed == 1)) {
                                        heidic_toggle_selection(hit_cube_index);
                                    } else {
                                        heidic_clear_selection();
                                        heidic_add_to_selection(hit_cube_index);
                                    }
                                    float  cube_index_f = heidic_int_to_float(hit_cube_index);
                                    selected_cube_index = (cube_index_f + 2);
                                    selected_cube_x = heidic_get_cube_x(hit_cube_index);
                                    selected_cube_y = heidic_get_cube_y(hit_cube_index);
                                    selected_cube_z = heidic_get_cube_z(hit_cube_index);
                                    selected_cube_sx = heidic_get_cube_sx(hit_cube_index);
                                    selected_cube_sy = heidic_get_cube_sy(hit_cube_index);
                                    selected_cube_sz = heidic_get_cube_sz(hit_cube_index);
                                    has_selection = 1;
                                    Vec3  target_pos = heidic_vec3(selected_cube_x, selected_cube_y, selected_cube_z);
                                    Vec3  current_cam_pos = camera_pos;
                                    float  calculated_distance = heidic_vec3_distance(current_cam_pos, target_pos);
                                    if (((calculated_distance > 10) && (calculated_distance < 100000))) {
                                        dolly_orbit_distance = calculated_distance;
                                    } else {
                                        dolly_orbit_distance = 2000;
                                    }
                                    dolly_orbit_azimuth = 0;
                                    dolly_orbit_elevation = 45;
                                }
                            }
                            mouse_left_was_pressed = 1;
                        }
                    } else {
                        int32_t  imgui_wants_mouse = heidic_imgui_wants_mouse();
                        if ((((ctrl_pressed == 0) && (gizmo_clicked == 0)) && (imgui_wants_mouse == 0))) {
                            heidic_clear_selection();
                            has_selection = 0;
                            selected_cube_index = -1;
                            heidic_clear_mesh_selection();
                            has_mesh_selection = 0;
                            selected_mesh_instance_id = -1;
                            has_wedge_selection = 0;
                            selected_wedge_index = -1;
                        }
                    }
                } else {
                    mouse_left_was_pressed = 0;
                }
            }
            if ((has_selection == 1)) {
                heidic_draw_cube_wireframe(selected_cube_x, selected_cube_y, selected_cube_z, 0, 0, 0, (selected_cube_sx * 1.01), (selected_cube_sy * 1.01), (selected_cube_sz * 1.01), 0, 0, 0);
                Vec3  new_pos = heidic_gizmo_translate(window, selected_cube_x, selected_cube_y, selected_cube_z);
                selected_cube_x = new_pos.x;
                selected_cube_y = new_pos.y;
                selected_cube_z = new_pos.z;
                if ((selected_cube_index >= 2)) {
                    float  cube_storage_index = (selected_cube_index - 2);
                    heidic_set_cube_pos_f(cube_storage_index, selected_cube_x, selected_cube_y, selected_cube_z);
                }
            }
            if (((has_mesh_selection == 1) && (selected_mesh_instance_id >= 0))) {
                selected_mesh_x = heidic_get_mesh_instance_x(selected_mesh_instance_id);
                selected_mesh_y = heidic_get_mesh_instance_y(selected_mesh_instance_id);
                selected_mesh_z = heidic_get_mesh_instance_z(selected_mesh_instance_id);
                float  mesh_center_x = heidic_get_mesh_instance_center_x(selected_mesh_instance_id);
                float  mesh_center_y = heidic_get_mesh_instance_center_y(selected_mesh_instance_id);
                float  mesh_center_z = heidic_get_mesh_instance_center_z(selected_mesh_instance_id);
                float  current_scale = heidic_get_mesh_instance_sx(selected_mesh_instance_id);
                float  gizmo_x = (selected_mesh_x + mesh_center_x);
                float  gizmo_y = (selected_mesh_y + mesh_center_y);
                float  gizmo_z = (selected_mesh_z + mesh_center_z);
                int32_t  scale_interacting = heidic_gizmo_scale_is_interacting();
                int32_t  translate_interacting = heidic_gizmo_is_interacting();
                float  bbox_min_x = heidic_get_mesh_instance_bbox_min_x(selected_mesh_instance_id);
                float  bbox_min_y = heidic_get_mesh_instance_bbox_min_y(selected_mesh_instance_id);
                float  bbox_min_z = heidic_get_mesh_instance_bbox_min_z(selected_mesh_instance_id);
                float  bbox_max_x = heidic_get_mesh_instance_bbox_max_x(selected_mesh_instance_id);
                float  bbox_max_y = heidic_get_mesh_instance_bbox_max_y(selected_mesh_instance_id);
                float  bbox_max_z = heidic_get_mesh_instance_bbox_max_z(selected_mesh_instance_id);
                float  new_scale = heidic_gizmo_scale(window, gizmo_x, gizmo_y, gizmo_z, bbox_min_x, bbox_min_y, bbox_min_z, bbox_max_x, bbox_max_y, bbox_max_z, current_scale);
                if ((new_scale != current_scale)) {
                    heidic_set_mesh_instance_scale(selected_mesh_instance_id, new_scale, new_scale, new_scale);
                }
                if ((scale_interacting == 0)) {
                    Vec3  new_gizmo_pos = heidic_gizmo_translate(window, gizmo_x, gizmo_y, gizmo_z);
                    if ((translate_interacting == 1)) {
                        float  new_instance_x = (new_gizmo_pos.x - mesh_center_x);
                        float  new_instance_y = (new_gizmo_pos.y - mesh_center_y);
                        float  new_instance_z = (new_gizmo_pos.z - mesh_center_z);
                        selected_mesh_x = new_instance_x;
                        selected_mesh_y = new_instance_y;
                        selected_mesh_z = new_instance_z;
                        heidic_set_mesh_instance_pos(selected_mesh_instance_id, selected_mesh_x, selected_mesh_y, selected_mesh_z);
                    }
                }
            }
            if ((show_debug == 1)) {
                if ((heidic_imgui_begin("Codex Of Forms") == 1)) {
                    int32_t  combination_count = heidic_get_combination_count();
                    int32_t  total_cube_count = heidic_get_cube_total_count();
                    int32_t  global_cube_display_index = 1;
                    int32_t  combo_id = 0;
                    while ((combo_id < combination_count)) {
                        heidic_imgui_push_id(combo_id);
                        int32_t  editing_id = heidic_get_editing_combination_id();
                        int32_t  is_editing_this = 0;
                        if ((editing_id == combo_id)) {
                            is_editing_this = 1;
                        }
                        std::string  combo_name = heidic_format_combination_name(combo_id);
                        int32_t  is_expanded = heidic_is_combination_expanded(combo_id);
                        std::string  expand_button_text = "+";
                        if ((is_expanded == 1)) {
                            expand_button_text = "-";
                        }
                        if ((heidic_imgui_button(expand_button_text.c_str()) == 1)) {
                            heidic_toggle_combination_expanded(combo_id);
                        }
                        heidic_imgui_same_line();
                        int32_t  enter_pressed = heidic_imgui_input_text_combination_simple(combo_id);
                        if ((is_expanded == 1)) {
                            int32_t  cube_index = 0;
                            while ((cube_index < total_cube_count)) {
                                if ((heidic_get_cube_active(cube_index) == 1)) {
                                    if ((heidic_get_cube_combination_id(cube_index) == combo_id)) {
                                        heidic_imgui_push_id(cube_index);
                                        std::string  cube_name = heidic_format_cube_name_with_index(global_cube_display_index);
                                        if ((heidic_is_cube_selected(cube_index) == 1)) {
                                            heidic_imgui_text_bold(cube_name.c_str());
                                        } else {
                                            if ((heidic_imgui_selectable_colored(cube_name.c_str(), 1, 0, 0, 1) == 1)) {
                                                heidic_clear_selection();
                                                heidic_add_to_selection(cube_index);
                                                float  cube_index_f = heidic_int_to_float(cube_index);
                                                selected_cube_index = (cube_index_f + 2);
                                                selected_cube_x = heidic_get_cube_x(cube_index);
                                                selected_cube_y = heidic_get_cube_y(cube_index);
                                                selected_cube_z = heidic_get_cube_z(cube_index);
                                                selected_cube_sx = heidic_get_cube_sx(cube_index);
                                                selected_cube_sy = heidic_get_cube_sy(cube_index);
                                                selected_cube_sz = heidic_get_cube_sz(cube_index);
                                                has_selection = 1;
                                            }
                                        }
                                        heidic_imgui_pop_id();
                                        global_cube_display_index = (global_cube_display_index + 1);
                                    }
                                }
                                cube_index = (cube_index + 1);
                            }
                        } else {
                            int32_t  cube_index = 0;
                            while ((cube_index < total_cube_count)) {
                                if ((heidic_get_cube_active(cube_index) == 1)) {
                                    if ((heidic_get_cube_combination_id(cube_index) == combo_id)) {
                                        global_cube_display_index = (global_cube_display_index + 1);
                                    }
                                }
                                cube_index = (cube_index + 1);
                            }
                        }
                        heidic_imgui_pop_id();
                        combo_id = (combo_id + 1);
                    }
                    int32_t  cube_index = 0;
                    while ((cube_index < total_cube_count)) {
                        if ((heidic_get_cube_active(cube_index) == 1)) {
                            if ((heidic_get_cube_combination_id(cube_index) == -1)) {
                                heidic_imgui_push_id(cube_index);
                                std::string  cube_name = heidic_format_cube_name_with_index(global_cube_display_index);
                                if ((heidic_is_cube_selected(cube_index) == 1)) {
                                    heidic_imgui_text_bold(cube_name.c_str());
                                } else {
                                    if ((heidic_imgui_selectable_str(cube_name.c_str()) == 1)) {
                                        heidic_clear_selection();
                                        heidic_add_to_selection(cube_index);
                                        float  cube_index_f = heidic_int_to_float(cube_index);
                                        selected_cube_index = (cube_index_f + 2);
                                        selected_cube_x = heidic_get_cube_x(cube_index);
                                        selected_cube_y = heidic_get_cube_y(cube_index);
                                        selected_cube_z = heidic_get_cube_z(cube_index);
                                        selected_cube_sx = heidic_get_cube_sx(cube_index);
                                        selected_cube_sy = heidic_get_cube_sy(cube_index);
                                        selected_cube_sz = heidic_get_cube_sz(cube_index);
                                        has_selection = 1;
                                    }
                                }
                                heidic_imgui_pop_id();
                                global_cube_display_index = (global_cube_display_index + 1);
                            }
                        }
                        cube_index = (cube_index + 1);
                    }
                    int32_t  mesh_instance_count = heidic_get_mesh_instance_total_count();
                    if ((mesh_instance_count > 0)) {
                        heidic_imgui_separator();
                        heidic_imgui_text("Meshes:");
                        int32_t  mesh_instance_index = 0;
                        while ((mesh_instance_index < mesh_instance_count)) {
                            if ((heidic_get_mesh_instance_active(mesh_instance_index) == 1)) {
                                heidic_imgui_push_id((mesh_instance_index + 10000));
                                std::string  mesh_name = heidic_format_mesh_name(mesh_instance_index);
                                int32_t  is_mesh_selected = heidic_is_mesh_selected(mesh_instance_index);
                                if ((heidic_imgui_selectable_str(mesh_name.c_str()) == 1)) {
                                    if ((is_mesh_selected == 1)) {
                                        heidic_clear_mesh_selection();
                                        has_mesh_selection = 0;
                                        selected_mesh_instance_id = -1;
                                    } else {
                                        heidic_clear_selection();
                                        heidic_clear_mesh_selection();
                                        heidic_add_mesh_to_selection(mesh_instance_index);
                                        selected_mesh_instance_id = mesh_instance_index;
                                        selected_mesh_x = heidic_get_mesh_instance_x(mesh_instance_index);
                                        selected_mesh_y = heidic_get_mesh_instance_y(mesh_instance_index);
                                        selected_mesh_z = heidic_get_mesh_instance_z(mesh_instance_index);
                                        has_mesh_selection = 1;
                                        has_selection = 0;
                                        selected_cube_index = -1;
                                    }
                                }
                                heidic_imgui_pop_id();
                            }
                            mesh_instance_index = (mesh_instance_index + 1);
                        }
                    }
                    int32_t  wedge_total_count = heidic_get_wedge_total_count();
                    if ((wedge_total_count > 0)) {
                        heidic_imgui_separator();
                        heidic_imgui_text("Wedges:");
                        int32_t  wedge_index = 0;
                        while ((wedge_index < wedge_total_count)) {
                            if ((heidic_get_wedge_active(wedge_index) == 1)) {
                                heidic_imgui_push_id((wedge_index + 20000));
                                std::string  wedge_name = heidic_format_wedge_name(wedge_index);
                                int32_t  is_wedge_selected = 0;
                                if (((has_wedge_selection == 1) && (selected_wedge_index == wedge_index))) {
                                    is_wedge_selected = 1;
                                }
                                if ((is_wedge_selected == 1)) {
                                    heidic_imgui_text_bold(wedge_name.c_str());
                                } else {
                                    if ((heidic_imgui_selectable_str(wedge_name.c_str()) == 1)) {
                                        heidic_clear_selection();
                                        heidic_clear_mesh_selection();
                                        selected_wedge_index = wedge_index;
                                        selected_wedge_x = heidic_get_wedge_x(wedge_index);
                                        selected_wedge_y = heidic_get_wedge_y(wedge_index);
                                        selected_wedge_z = heidic_get_wedge_z(wedge_index);
                                        selected_wedge_sx = heidic_get_wedge_sx(wedge_index);
                                        selected_wedge_sy = heidic_get_wedge_sy(wedge_index);
                                        selected_wedge_sz = heidic_get_wedge_sz(wedge_index);
                                        has_wedge_selection = 1;
                                        has_selection = 0;
                                        has_mesh_selection = 0;
                                        selected_cube_index = -1;
                                        selected_mesh_instance_id = -1;
                                    }
                                }
                                heidic_imgui_pop_id();
                            }
                            wedge_index = (wedge_index + 1);
                        }
                    }
                    heidic_imgui_end();
                }
                if ((build_mode == 1)) {
                    if ((heidic_imgui_begin("Wedge Rotation Position") == 1)) {
                        float  current_rot_x = 0;
                        float  current_rot_y = 0;
                        float  current_rot_z = 0;
                        if ((wedge_preview_rotation == 0)) {
                            current_rot_x = 270;
                            current_rot_y = 180;
                            current_rot_z = 90;
                        } else {
                            if ((wedge_preview_rotation == 1)) {
                                current_rot_x = 180;
                                current_rot_y = 90;
                                current_rot_z = 270;
                            } else {
                                if ((wedge_preview_rotation == 2)) {
                                    current_rot_x = 90;
                                    current_rot_y = 270;
                                    current_rot_z = 180;
                                } else {
                                    if ((wedge_preview_rotation == 3)) {
                                        current_rot_x = 180;
                                        current_rot_y = 0;
                                        current_rot_z = 270;
                                    } else {
                                        if ((wedge_preview_rotation == 4)) {
                                            current_rot_x = 180;
                                            current_rot_y = 180;
                                            current_rot_z = 90;
                                        } else {
                                            if ((wedge_preview_rotation == 5)) {
                                                current_rot_x = 180;
                                                current_rot_y = 90;
                                                current_rot_z = 90;
                                            } else {
                                                if ((wedge_preview_rotation == 6)) {
                                                    current_rot_x = 180;
                                                    current_rot_y = 270;
                                                    current_rot_z = 90;
                                                } else {
                                                    if ((wedge_preview_rotation == 7)) {
                                                        current_rot_x = 180;
                                                        current_rot_y = 0;
                                                        current_rot_z = 90;
                                                    } else {
                                                        if ((wedge_preview_rotation == 8)) {
                                                            current_rot_x = 180;
                                                            current_rot_y = 90;
                                                            current_rot_z = 180;
                                                        } else {
                                                            if ((wedge_preview_rotation == 9)) {
                                                                current_rot_x = 90;
                                                                current_rot_y = 270;
                                                                current_rot_z = 270;
                                                            } else {
                                                                if ((wedge_preview_rotation == 10)) {
                                                                    current_rot_x = 90;
                                                                    current_rot_y = 270;
                                                                    current_rot_z = 90;
                                                                } else {
                                                                    if ((wedge_preview_rotation == 11)) {
                                                                        current_rot_x = 180;
                                                                        current_rot_y = 0;
                                                                        current_rot_z = 180;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        heidic_imgui_text("Current Position:");
                        heidic_imgui_text("Index:");
                        heidic_imgui_same_line();
                        heidic_imgui_text_float("##PosIndex", heidic_int_to_float(wedge_preview_rotation));
                        heidic_imgui_text("(0-11)");
                        heidic_imgui_separator();
                        heidic_imgui_text("Rotation Coordinates:");
                        heidic_imgui_text("X:");
                        heidic_imgui_same_line();
                        heidic_imgui_text_float("##RotX", current_rot_x);
                        heidic_imgui_text("Y:");
                        heidic_imgui_same_line();
                        heidic_imgui_text_float("##RotY", current_rot_y);
                        heidic_imgui_text("Z:");
                        heidic_imgui_same_line();
                        heidic_imgui_text_float("##RotZ", current_rot_z);
                        heidic_imgui_separator();
                        heidic_imgui_text("Use mouse wheel to cycle");
                        heidic_imgui_text("through 12 positions");
                        heidic_imgui_end();
                    }
                }
                if (((has_wedge_selection == 1) && (selected_wedge_index >= 0))) {
                    if ((heidic_imgui_begin("Transform Control") == 1)) {
                        float  current_x = heidic_get_wedge_x(selected_wedge_index);
                        float  current_y = heidic_get_wedge_y(selected_wedge_index);
                        float  current_z = heidic_get_wedge_z(selected_wedge_index);
                        float  current_sx = heidic_get_wedge_sx(selected_wedge_index);
                        float  current_sy = heidic_get_wedge_sy(selected_wedge_index);
                        float  current_sz = heidic_get_wedge_sz(selected_wedge_index);
                        float  current_rx = heidic_get_wedge_rx(selected_wedge_index);
                        float  current_ry = heidic_get_wedge_ry(selected_wedge_index);
                        float  current_rz = heidic_get_wedge_rz(selected_wedge_index);
                        heidic_imgui_text("Translation:");
                        heidic_imgui_text("X:");
                        heidic_imgui_same_line();
                        float  new_x = heidic_imgui_input_float("##WedgeTransX", current_x, 1, 10);
                        heidic_imgui_text("Y:");
                        heidic_imgui_same_line();
                        float  new_y = heidic_imgui_input_float("##WedgeTransY", current_y, 1, 10);
                        heidic_imgui_text("Z:");
                        heidic_imgui_same_line();
                        float  new_z = heidic_imgui_input_float("##WedgeTransZ", current_z, 1, 10);
                        if ((((new_x != current_x) || (new_y != current_y)) || (new_z != current_z))) {
                            heidic_set_wedge_pos(selected_wedge_index, new_x, new_y, new_z);
                            selected_wedge_x = new_x;
                            selected_wedge_y = new_y;
                            selected_wedge_z = new_z;
                        }
                        heidic_imgui_separator();
                        heidic_imgui_text("Scale:");
                        heidic_imgui_text("X:");
                        heidic_imgui_same_line();
                        float  new_sx = heidic_imgui_input_float("##WedgeScaleX", current_sx, 0.1, 1);
                        heidic_imgui_text("Y:");
                        heidic_imgui_same_line();
                        float  new_sy = heidic_imgui_input_float("##WedgeScaleY", current_sy, 0.1, 1);
                        heidic_imgui_text("Z:");
                        heidic_imgui_same_line();
                        float  new_sz = heidic_imgui_input_float("##WedgeScaleZ", current_sz, 0.1, 1);
                        if ((new_sx < 0.01)) {
                            new_sx = 0.01;
                        }
                        if ((new_sy < 0.01)) {
                            new_sy = 0.01;
                        }
                        if ((new_sz < 0.01)) {
                            new_sz = 0.01;
                        }
                        if ((((new_sx != current_sx) || (new_sy != current_sy)) || (new_sz != current_sz))) {
                            heidic_set_wedge_scale(selected_wedge_index, new_sx, new_sy, new_sz);
                        }
                        heidic_imgui_separator();
                        heidic_imgui_text("Rotation:");
                        float  new_rx_slider = heidic_imgui_slider_float("Rotation X", current_rx, 0, 360);
                        float  new_rx_input = heidic_imgui_input_float("##WedgeRX", current_rx, 1, 10);
                        float  new_rx = new_rx_slider;
                        if ((new_rx_input != current_rx)) {
                            new_rx = new_rx_input;
                        }
                        if ((new_rx != current_rx)) {
                            if ((new_rx < 0)) {
                                new_rx = 0;
                            }
                            if ((new_rx > 360)) {
                                new_rx = 360;
                            }
                            heidic_set_wedge_rotation(selected_wedge_index, new_rx, current_ry, current_rz);
                        }
                        float  new_ry_slider = heidic_imgui_slider_float("Rotation Y", current_ry, 0, 360);
                        float  new_ry_input = heidic_imgui_input_float("##WedgeRY", current_ry, 1, 10);
                        float  new_ry = new_ry_slider;
                        if ((new_ry_input != current_ry)) {
                            new_ry = new_ry_input;
                        }
                        if ((new_ry != current_ry)) {
                            if ((new_ry < 0)) {
                                new_ry = 0;
                            }
                            if ((new_ry > 360)) {
                                new_ry = 360;
                            }
                            heidic_set_wedge_rotation(selected_wedge_index, current_rx, new_ry, current_rz);
                        }
                        float  new_rz_slider = heidic_imgui_slider_float("Rotation Z", current_rz, 0, 360);
                        float  new_rz_input = heidic_imgui_input_float("##WedgeRZ", current_rz, 1, 10);
                        float  new_rz = new_rz_slider;
                        if ((new_rz_input != current_rz)) {
                            new_rz = new_rz_input;
                        }
                        if ((new_rz != current_rz)) {
                            if ((new_rz < 0)) {
                                new_rz = 0;
                            }
                            if ((new_rz > 360)) {
                                new_rz = 360;
                            }
                            heidic_set_wedge_rotation(selected_wedge_index, current_rx, current_ry, new_rz);
                        }
                        heidic_imgui_end();
                    }
                }
                if (((has_mesh_selection == 1) && (selected_mesh_instance_id >= 0))) {
                    std::cout << "[HEIDIC DEBUG] About to begin Transform Control window (mesh)\n" << std::endl;
                    if ((heidic_imgui_begin("Transform Control") == 1)) {
                        std::cout << "[HEIDIC DEBUG] Inside Transform Control window (mesh)\n" << std::endl;
                        float  current_x = heidic_get_mesh_instance_x(selected_mesh_instance_id);
                        float  current_y = heidic_get_mesh_instance_y(selected_mesh_instance_id);
                        float  current_z = heidic_get_mesh_instance_z(selected_mesh_instance_id);
                        float  current_sx = heidic_get_mesh_instance_sx(selected_mesh_instance_id);
                        float  current_sy = heidic_get_mesh_instance_sy(selected_mesh_instance_id);
                        float  current_sz = heidic_get_mesh_instance_sz(selected_mesh_instance_id);
                        float  current_rx = heidic_get_mesh_instance_rx(selected_mesh_instance_id);
                        float  current_ry = heidic_get_mesh_instance_ry(selected_mesh_instance_id);
                        float  current_rz = heidic_get_mesh_instance_rz(selected_mesh_instance_id);
                        heidic_imgui_text("Translation:");
                        heidic_imgui_text("X:");
                        heidic_imgui_same_line();
                        float  new_x = heidic_imgui_input_float("##TransX", current_x, 1, 10);
                        heidic_imgui_text("Y:");
                        heidic_imgui_same_line();
                        float  new_y = heidic_imgui_input_float("##TransY", current_y, 1, 10);
                        heidic_imgui_text("Z:");
                        heidic_imgui_same_line();
                        float  new_z = heidic_imgui_input_float("##TransZ", current_z, 1, 10);
                        if ((((new_x != current_x) || (new_y != current_y)) || (new_z != current_z))) {
                            heidic_set_mesh_instance_pos(selected_mesh_instance_id, new_x, new_y, new_z);
                            selected_mesh_x = new_x;
                            selected_mesh_y = new_y;
                            selected_mesh_z = new_z;
                        }
                        heidic_imgui_separator();
                        heidic_imgui_text("Scale:");
                        heidic_imgui_text("X:");
                        heidic_imgui_same_line();
                        float  new_sx = heidic_imgui_input_float("##ScaleX", current_sx, 0.1, 1);
                        heidic_imgui_text("Y:");
                        heidic_imgui_same_line();
                        float  new_sy = heidic_imgui_input_float("##ScaleY", current_sy, 0.1, 1);
                        heidic_imgui_text("Z:");
                        heidic_imgui_same_line();
                        float  new_sz = heidic_imgui_input_float("##ScaleZ", current_sz, 0.1, 1);
                        if ((new_sx < 0.01)) {
                            new_sx = 0.01;
                        }
                        if ((new_sy < 0.01)) {
                            new_sy = 0.01;
                        }
                        if ((new_sz < 0.01)) {
                            new_sz = 0.01;
                        }
                        if ((((new_sx != current_sx) || (new_sy != current_sy)) || (new_sz != current_sz))) {
                            heidic_set_mesh_instance_scale(selected_mesh_instance_id, new_sx, new_sy, new_sz);
                        }
                        heidic_imgui_separator();
                        heidic_imgui_text("Rotation:");
                        float  new_rx_slider = heidic_imgui_slider_float("Rotation X", current_rx, 0, 360);
                        float  new_rx_input = heidic_imgui_input_float("##RX", current_rx, 1, 10);
                        float  new_rx = new_rx_slider;
                        if ((new_rx_input != current_rx)) {
                            new_rx = new_rx_input;
                        }
                        if ((new_rx != current_rx)) {
                            if ((new_rx < 0)) {
                                new_rx = 0;
                            }
                            if ((new_rx > 360)) {
                                new_rx = 360;
                            }
                            heidic_set_mesh_instance_rotation(selected_mesh_instance_id, new_rx, current_ry, current_rz);
                        }
                        float  new_ry_slider = heidic_imgui_slider_float("Rotation Y", current_ry, 0, 360);
                        float  new_ry_input = heidic_imgui_input_float("##RY", current_ry, 1, 10);
                        float  new_ry = new_ry_slider;
                        if ((new_ry_input != current_ry)) {
                            new_ry = new_ry_input;
                        }
                        if ((new_ry != current_ry)) {
                            if ((new_ry < 0)) {
                                new_ry = 0;
                            }
                            if ((new_ry > 360)) {
                                new_ry = 360;
                            }
                            heidic_set_mesh_instance_rotation(selected_mesh_instance_id, current_rx, new_ry, current_rz);
                        }
                        float  new_rz_slider = heidic_imgui_slider_float("Rotation Z", current_rz, 0, 360);
                        float  new_rz_input = heidic_imgui_input_float("##RZ", current_rz, 1, 10);
                        float  new_rz = new_rz_slider;
                        if ((new_rz_input != current_rz)) {
                            new_rz = new_rz_input;
                        }
                        if ((new_rz != current_rz)) {
                            if ((new_rz < 0)) {
                                new_rz = 0;
                            }
                            if ((new_rz > 360)) {
                                new_rz = 360;
                            }
                            heidic_set_mesh_instance_rotation(selected_mesh_instance_id, current_rx, current_ry, new_rz);
                        }
                        heidic_imgui_end();
                    }
                }
                if (((has_selection == 1) && (selected_cube_index >= 2))) {
                    int32_t  cube_storage_index = heidic_float_to_int((selected_cube_index - 2));
                    if ((heidic_get_cube_active(cube_storage_index) == 1)) {
                        std::cout << "[HEIDIC DEBUG] About to begin Transform Control window (mesh)\n" << std::endl;
                        if ((heidic_imgui_begin("Transform Control") == 1)) {
                            std::cout << "[HEIDIC DEBUG] Inside Transform Control window (mesh)\n" << std::endl;
                            float  current_x = heidic_get_cube_x(cube_storage_index);
                            float  current_y = heidic_get_cube_y(cube_storage_index);
                            float  current_z = heidic_get_cube_z(cube_storage_index);
                            float  current_sx = heidic_get_cube_sx(cube_storage_index);
                            float  current_sy = heidic_get_cube_sy(cube_storage_index);
                            float  current_sz = heidic_get_cube_sz(cube_storage_index);
                            float  current_rx = heidic_get_cube_rx(cube_storage_index);
                            float  current_ry = heidic_get_cube_ry(cube_storage_index);
                            float  current_rz = heidic_get_cube_rz(cube_storage_index);
                            heidic_imgui_text("Translation:");
                            heidic_imgui_text("X:");
                            heidic_imgui_same_line();
                            float  new_x = heidic_imgui_input_float("##CubeTransX", current_x, 1, 10);
                            heidic_imgui_text("Y:");
                            heidic_imgui_same_line();
                            float  new_y = heidic_imgui_input_float("##CubeTransY", current_y, 1, 10);
                            heidic_imgui_text("Z:");
                            heidic_imgui_same_line();
                            float  new_z = heidic_imgui_input_float("##CubeTransZ", current_z, 1, 10);
                            if ((((new_x != current_x) || (new_y != current_y)) || (new_z != current_z))) {
                                heidic_set_cube_pos(cube_storage_index, new_x, new_y, new_z);
                            }
                            heidic_imgui_separator();
                            heidic_imgui_text("Scale:");
                            heidic_imgui_text("X:");
                            heidic_imgui_same_line();
                            float  new_sx = heidic_imgui_input_float("##CubeScaleX", current_sx, 0.1, 1);
                            heidic_imgui_text("Y:");
                            heidic_imgui_same_line();
                            float  new_sy = heidic_imgui_input_float("##CubeScaleY", current_sy, 0.1, 1);
                            heidic_imgui_text("Z:");
                            heidic_imgui_same_line();
                            float  new_sz = heidic_imgui_input_float("##CubeScaleZ", current_sz, 0.1, 1);
                            if ((new_sx < 0.01)) {
                                new_sx = 0.01;
                            }
                            if ((new_sy < 0.01)) {
                                new_sy = 0.01;
                            }
                            if ((new_sz < 0.01)) {
                                new_sz = 0.01;
                            }
                            if ((((new_sx != current_sx) || (new_sy != current_sy)) || (new_sz != current_sz))) {
                                heidic_set_cube_scale(cube_storage_index, new_sx, new_sy, new_sz);
                            }
                            heidic_imgui_separator();
                            heidic_imgui_text("Rotation:");
                            float  new_rx_slider = heidic_imgui_slider_float("Rotation X", current_rx, 0, 360);
                            float  new_rx_input = heidic_imgui_input_float("##CubeRX", current_rx, 1, 10);
                            float  new_rx = new_rx_slider;
                            if ((new_rx_input != current_rx)) {
                                new_rx = new_rx_input;
                            }
                            if ((new_rx != current_rx)) {
                                if ((new_rx < 0)) {
                                    new_rx = 0;
                                }
                                if ((new_rx > 360)) {
                                    new_rx = 360;
                                }
                                heidic_set_cube_rotation(cube_storage_index, new_rx, current_ry, current_rz);
                            }
                            float  new_ry_slider = heidic_imgui_slider_float("Rotation Y", current_ry, 0, 360);
                            float  new_ry_input = heidic_imgui_input_float("##CubeRY", current_ry, 1, 10);
                            float  new_ry = new_ry_slider;
                            if ((new_ry_input != current_ry)) {
                                new_ry = new_ry_input;
                            }
                            if ((new_ry != current_ry)) {
                                if ((new_ry < 0)) {
                                    new_ry = 0;
                                }
                                if ((new_ry > 360)) {
                                    new_ry = 360;
                                }
                                heidic_set_cube_rotation(cube_storage_index, current_rx, new_ry, current_rz);
                            }
                            float  new_rz_slider = heidic_imgui_slider_float("Rotation Z", current_rz, 0, 360);
                            float  new_rz_input = heidic_imgui_input_float("##CubeRZ", current_rz, 1, 10);
                            float  new_rz = new_rz_slider;
                            if ((new_rz_input != current_rz)) {
                                new_rz = new_rz_input;
                            }
                            if ((new_rz != current_rz)) {
                                if ((new_rz < 0)) {
                                    new_rz = 0;
                                }
                                if ((new_rz > 360)) {
                                    new_rz = 360;
                                }
                                heidic_set_cube_rotation(cube_storage_index, current_rx, current_ry, new_rz);
                            }
                            heidic_imgui_end();
                        }
                    }
                }
                if ((heidic_imgui_begin("Texture Swatches") == 1)) {
                    heidic_load_texture_list();
                    int32_t  texture_count = heidic_get_texture_count();
                    int32_t  texture_index = 0;
                    int32_t  items_per_row = 6;
                    int32_t  current_col = 0;
                    float  swatch_size = 64;
                    while ((texture_index < texture_count)) {
                        std::string  texture_name = heidic_get_texture_name(texture_index);
                        int64_t  texture_id = heidic_get_texture_preview_id(texture_name.c_str());
                        heidic_imgui_push_id(texture_index);
                        std::string  selected_texture = heidic_get_selected_texture();
                        int32_t  is_selected = 0;
                        if ((selected_texture == texture_name)) {
                            is_selected = 1;
                        }
                        if ((texture_id != 0)) {
                            float  tint_r = 1;
                            float  tint_g = 1;
                            float  tint_b = 1;
                            float  tint_a = 1;
                            if ((is_selected == 1)) {
                                tint_r = 1;
                                tint_g = 1;
                                tint_b = 0.5;
                            }
                            std::string  button_id = texture_name;
                            if ((heidic_imgui_image_button(button_id.c_str(), texture_id, swatch_size, swatch_size, tint_r, tint_g, tint_b, tint_a) == 1)) {
                                heidic_set_selected_texture(texture_name.c_str());
                            }
                        } else {
                            heidic_imgui_text_str_wrapper(texture_name.c_str());
                        }
                        heidic_imgui_pop_id();
                        current_col = (current_col + 1);
                        if ((current_col < items_per_row)) {
                            heidic_imgui_same_line();
                        } else {
                            current_col = 0;
                        }
                        texture_index = (texture_index + 1);
                    }
                    heidic_imgui_end();
                }
            }
            float  ground_check_distance = 200;
            is_grounded = heidic_raycast_ground_hit(player_pos.x, player_pos.y, player_pos.z, ground_check_distance);
            if ((is_grounded == 1)) {
                Vec3  ground_hit = heidic_raycast_ground_hit_point(player_pos.x, player_pos.y, player_pos.z, ground_check_distance);
                heidic_draw_line(player_pos.x, player_pos.y, player_pos.z, ground_hit.x, ground_hit.y, ground_hit.z, 0, 1, 0);
            } else {
                Vec3  ground_check_end = heidic_vec3(player_pos.x, (player_pos.y - ground_check_distance), player_pos.z);
                heidic_draw_line(player_pos.x, player_pos.y, player_pos.z, ground_check_end.x, ground_check_end.y, ground_check_end.z, 1, 0, 0);
            }
            heidic_end_frame();
            heidic_imgui_save_layout(default_ini_path_load.c_str());
        }
        return 0;
}

int main(int argc, char* argv[]) {
    heidic_main();
    return 0;
}
