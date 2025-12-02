# Quick Test Solutions for ImGui Assertion

## Test 1: Remove SetKeyboardFocusHere (Isolate the Problem)

**Hypothesis**: `SetKeyboardFocusHere()` is causing the assertion.

**Action**: Comment out the `SetKeyboardFocusHere()` call in `heidic_imgui_input_text_combination_name()`:

```cpp
// In vulkan/eden_vulkan_helpers.cpp, around line 3554
if (g_shouldSetFocusOnInput) {
    // ImGui::SetKeyboardFocusHere(0);  // ← COMMENT THIS OUT
    g_shouldSetFocusOnInput = false;
}
```

**Expected Result**: 
- If assertion goes away → `SetKeyboardFocusHere()` is the problem
- If assertion persists → Problem is elsewhere

---

## Test 2: Defer Combination to Next Frame

**Hypothesis**: Combination runs during input handling, causing state inconsistency when Outliner renders.

**Action**: Add a flag to defer combination:

```cpp
// In vulkan/eden_vulkan_helpers.cpp
static bool g_pendingCombination = false;

extern "C" void heidic_combine_connected_cubes() {
    g_pendingCombination = true;  // Defer to next frame
}

// In heidic_begin_frame(), at the start:
if (g_pendingCombination) {
    g_pendingCombination = false;
    // Actually do the combination here
    // ... (move the combination logic here)
}
```

**Expected Result**: 
- If assertion goes away → Timing issue confirmed
- If assertion persists → Not a timing issue

---

## Test 3: Skip InputText When Editing ID is Invalid

**Hypothesis**: Outliner is trying to render InputText with invalid `editing_id` after combination clears it.

**Action**: Add extra safety check in HEIDIC code:

```heidic
// In gateway_editor_v1.hd, around line 1550
let editing_id: i32 = heidic_get_editing_combination_id();
if editing_id == combination_id {
    // Double-check that combination_id is still valid
    let combo_count: i32 = heidic_get_combination_count();
    if combination_id >= 0 && combination_id < combo_count {
        // Show input text field for editing
        if heidic_imgui_input_text_combination_name() == 1 {
            // ...
        }
    } else {
        // Invalid combination ID, stop editing
        heidic_stop_editing_combination_name();
    }
}
```

**Expected Result**: 
- If assertion goes away → Invalid ID access was the problem
- If assertion persists → Problem is elsewhere

---

## Test 4: Don't Call should_stop_editing When Not Editing

**Hypothesis**: `heidic_imgui_should_stop_editing()` is being called even when not editing, causing assertion.

**Action**: Add guard in HEIDIC code:

```heidic
// In gateway_editor_v1.hd
if editing_id == combination_id {
    // ... input text code ...
    
    // Only check stop conditions if we're actually editing
    if editing_id >= 0 {
        if heidic_imgui_should_stop_editing() == 1 {
            heidic_stop_editing_combination_name();
        }
    }
}
```

**Expected Result**: 
- If assertion goes away → `should_stop_editing()` was being called incorrectly
- If assertion persists → Problem is elsewhere

---

## Test 5: Add Safety Check in should_stop_editing

**Hypothesis**: `IsKeyPressed()` or `IsMouseClicked()` are being called outside frame scope.

**Action**: Add early return if not editing:

```cpp
// In vulkan/eden_vulkan_helpers.cpp
extern "C" int heidic_imgui_should_stop_editing() {
    // Early return if not editing - don't call ImGui functions unnecessarily
    if (g_editingCombinationId < 0) {
        return 0;
    }
    
    // Rest of the function...
}
```

**Expected Result**: 
- If assertion goes away → ImGui functions were being called when not needed
- If assertion persists → Problem is elsewhere

---

## Recommended Testing Order

1. **Test 1** (Remove SetKeyboardFocusHere) - Quickest, most likely culprit
2. **Test 5** (Safety check in should_stop_editing) - Easy, low risk
3. **Test 3** (Skip InputText when invalid) - Medium effort
4. **Test 2** (Defer combination) - More complex, but addresses root cause
5. **Test 4** (Don't call should_stop_editing) - If others don't work

---

## How to Apply Tests

1. Make one change at a time
2. Build and test
3. Check if assertion is gone
4. If gone → That was the problem! Document the fix.
5. If still there → Revert and try next test

---

## Debug Output

With the debug logging I added, you should see output like:

```
[DEBUG] heidic_combine_connected_cubes: START
[DEBUG] heidic_combine_connected_cubes: Cleared all editing state
C key pressed - Combining connected cubes
Combination complete
[DEBUG] heidic_imgui_input_text_combination_name: editing_id=-1, shouldSetFocus=0
[DEBUG] heidic_imgui_should_stop_editing: checking...
[DEBUG] About to call IsKeyPressed(ImGuiKey_Escape)
```

**Look for**: The last `[DEBUG]` message before the assertion. That tells you which function triggered it.

