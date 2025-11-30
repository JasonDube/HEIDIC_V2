# Known Issues

This document tracks known bugs and issues in the EDEN Engine and HEIDIC compiler.

## Fullscreen Mode Issue

**Status**: Open  
**Severity**: Medium  
**Date Reported**: December 2025

### Description
When starting the application in windowed mode, everything works correctly. However, when toggling to fullscreen mode using `heidic_set_video_mode(0)`, the application becomes unresponsive. The screen may display correctly (or show a white screen), but all input handling and functionality ceases.

### Steps to Reproduce
1. Start application in windowed mode (default)
2. Press Shift+Enter to toggle to fullscreen mode
3. Application becomes unresponsive

### Possible Causes
- Vulkan swapchain not being recreated when window resolution/mode changes
- GLFW window mode change not properly handled by Vulkan renderer
- Non-standard monitor resolution compatibility issues
- Missing swapchain recreation logic in `heidic_set_video_mode()`

### Technical Details
- The `heidic_set_video_mode()` function uses `glfwSetWindowMonitor()` to change window mode
- Vulkan requires swapchain recreation when window size/mode changes
- Current implementation does not handle swapchain recreation

### Workaround
- Start application in windowed mode and avoid toggling to fullscreen

### Related Files
- `vulkan/eden_vulkan_helpers.cpp` - `heidic_set_video_mode()` implementation
- `examples/gateway_editor_v1/gateway_editor_v1.hd` - Example demonstrating the issue

