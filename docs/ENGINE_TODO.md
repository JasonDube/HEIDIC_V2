# EDEN Engine TODO

This document tracks engine-level improvements and bug fixes.

## High Priority

### Fullscreen Mode Fix
- **Status**: Open
- **Priority**: High
- **Description**: Fix fullscreen mode toggle causing application to become unresponsive
- **Details**: When toggling from windowed to fullscreen, the Vulkan swapchain needs to be recreated to match the new resolution. Current implementation does not handle this.
- **Related**: See `docs/KNOWN_ISSUES.md` for full details
- **Files**: `vulkan/eden_vulkan_helpers.cpp`

## Medium Priority

(Add more items as needed)

## Low Priority

(Add more items as needed)

