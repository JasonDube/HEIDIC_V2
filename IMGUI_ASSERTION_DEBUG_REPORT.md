# ImGui `g.WithinFrameScope` Assertion Debug Report

## Problem Summary

**Error**: `Assertion failed: g.WithinFrameScope, file C:/Users/nitro/Desktop/PROJECTS/heidic_v2/third_party/imgui/imgui.cpp, line 7683`

**When it occurs**: Right after pressing 'C' to combine connected cubes, immediately after "Combination complete" is printed.

**Context**: This is a HEIDIC language project (custom transpiler to C++) using ImGui for UI, Vulkan for rendering, and GLFW for window management.

## Architecture

- **HEIDIC** → transpiles to C++ → compiled with g++
- **ImGui** (docking branch) for UI
- **Vulkan** for rendering
- **GLFW** for window/input

## Code Flow

### 1. User presses 'C' key
```heidic
// In gateway_editor_v1.hd
if combine_c_is_pressed == 1 {
    if combine_c_was_pressed == 0 {
        print("C key pressed - Combining connected cubes\n");
        heidic_combine_connected_cubes();
        print("Combination complete\n");
    }
}
```

### 2. Combination function (C++)
```cpp
extern "C" void heidic_combine_connected_cubes() {
    // Resets all combination IDs
    // Clears: g_editingCombinationId, g_pendingEditingCombinationId, 
    //         g_pendingClickCombinationId, g_shouldSetFocusOnInput
    // Uses union-find algorithm to group connected cubes
    // Assigns new combination IDs
}
```

### 3. Frame rendering (after combination)
The assertion occurs during the ImGui rendering phase, likely in the Outliner window which displays combinations.

## Outliner Code Structure

```heidic
// In gateway_editor_v1.hd
heidic_imgui_begin("Outliner");
// ... show cube counts ...

// Loop through combinations
while combination_id < combination_count {
    // Check if editing
    if editing_id == combination_id {
        // Show InputText for editing
        if heidic_imgui_input_text_combination_name() == 1 {
            // Save name
        }
        // Check for Escape/click outside
        if heidic_imgui_should_stop_editing() == 1 {
            heidic_stop_editing_combination_name();
        }
    } else {
        // Show clickable Selectable
        if heidic_imgui_selectable_str(combo_name) == 1 {
            heidic_start_editing_combination_name(combination_id);
        }
    }
    // ... show expanded cubes ...
}
heidic_imgui_end();
```

## Deferred Editing Logic

To prevent frame scope issues, we use deferred editing:

1. **Click detected** → `g_pendingClickCombinationId` is set
2. **Next frame start** (`heidic_begin_frame`):
   - `g_pendingClickCombinationId` → `g_pendingEditingCombinationId`
   - `g_pendingEditingCombinationId` → `g_editingCombinationId`
   - `g_shouldSetFocusOnInput = true`
3. **During rendering** → `InputText` is shown, focus is set once

## Suspected Problem Areas

### 1. **State Inconsistency After Combination**
When `heidic_combine_connected_cubes()` runs:
- Clears `g_editingCombinationId = -1`
- Clears `g_pendingEditingCombinationId = -1`
- Clears `g_pendingClickCombinationId = -1`
- Clears `g_shouldSetFocusOnInput = false`

But if the Outliner is rendering in the same frame, it might:
- Still have `editing_id` from previous frame
- Try to call `heidic_imgui_input_text_combination_name()` with invalid state
- Call `SetKeyboardFocusHere()` when not in valid frame scope

### 2. **SetKeyboardFocusHere() Called at Wrong Time**
```cpp
if (g_shouldSetFocusOnInput) {
    ImGui::SetKeyboardFocusHere(0);  // ← Might trigger assertion
    g_shouldSetFocusOnInput = false;
}
```

This is called during `InputText` rendering, but might be called:
- After frame scope has ended
- When window is not active
- When ImGui is in an invalid state

### 3. **ImGui Functions Called Outside Frame Scope**
The assertion specifically says `g.WithinFrameScope` is false, meaning:
- An ImGui function was called after `ImGui::EndFrame()` or `ImGui::Render()`
- Or before `ImGui::NewFrame()`
- Or in a callback/event handler outside the frame

### 4. **Possible Race Condition**
The combination function runs during input handling (same frame), then:
- Outliner tries to render
- But combination just cleared editing state
- Outliner might try to access invalid combination data
- Or call ImGui functions with stale state

## Debug Logging Added

I've added extensive debug logging to track:
1. When `heidic_combine_connected_cubes()` starts/ends
2. When `heidic_begin_frame()` processes pending editing
3. When `heidic_imgui_input_text_combination_name()` is called
4. When `SetKeyboardFocusHere()` is called
5. When `heidic_imgui_should_stop_editing()` is called
6. All ImGui function calls that might trigger the assertion

## Questions for Investigation

1. **Is the assertion happening in `SetKeyboardFocusHere()`?**
   - Check debug logs to see if it's called right before assertion

2. **Is it happening in `InputText()`?**
   - Check if `InputText` is being called with invalid state

3. **Is it happening in `IsKeyPressed()` or `IsMouseClicked()`?**
   - These are called in `heidic_imgui_should_stop_editing()`

4. **Is the Outliner window being rendered after combination?**
   - Check if `heidic_imgui_begin("Outliner")` is called after combination

5. **Is there a timing issue with frame boundaries?**
   - Combination runs during input handling
   - Outliner renders later in same frame
   - But state was cleared, causing inconsistency

## Potential Solutions to Try

### Solution 1: Don't Clear Editing State During Combination
Instead of clearing `g_editingCombinationId` immediately, mark it as "invalid" and let the Outliner handle it gracefully.

### Solution 2: Defer Combination to Next Frame
Move `heidic_combine_connected_cubes()` to run at the start of the next frame (similar to deferred editing).

### Solution 3: Add Frame Scope Check Before All ImGui Calls
```cpp
// Check if we're in a valid frame scope before calling ImGui functions
// But we can't access WithinFrameScope without imgui_internal.h
// So we need another way to verify...
```

### Solution 4: Remove SetKeyboardFocusHere Entirely
Try removing the focus setting and see if the assertion goes away. This would confirm if `SetKeyboardFocusHere()` is the culprit.

### Solution 5: Ensure Combination Only Runs Between Frames
Add a flag to ensure combination only runs when it's safe (e.g., at the start of a frame, not during rendering).

## Next Steps

1. **Run with debug logging** to see exactly which function triggers the assertion
2. **Check the call stack** when assertion fires (if possible)
3. **Try Solution 4** (remove SetKeyboardFocusHere) to isolate the problem
4. **Try Solution 2** (defer combination) to see if timing is the issue
5. **Check if Outliner is trying to render with invalid combination IDs** after combination clears them

## Files Modified

- `vulkan/eden_vulkan_helpers.cpp` - Added debug logging, deferred editing logic
- `examples/gateway_editor_v1/gateway_editor_v1.hd` - Outliner rendering code
- `vulkan/eden_vulkan_helpers.h` - Function declarations
- `stdlib/eden.hd` - HEIDIC function declarations

## ImGui Version

Using ImGui **docking branch** (not master), which includes docking features.

## Build System

- **HEIDIC compiler**: Rust-based transpiler
- **C++ compiler**: g++ with C++17
- **Platform**: Windows 10

