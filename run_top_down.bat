@echo off
echo Compiling Heidic Code...
cargo run -- compile examples/top_down/top_down.hd
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo Compiling C++ Code...
g++ -std=c++17 -O3 ^
    -I. -Istdlib ^
    -I"%VULKAN_SDK%\Include" ^
    -Ithird_party\glm ^
    -Ithird_party\imgui ^
    -Ithird_party\imgui\backends ^
    -L"%VULKAN_SDK%\Lib" ^
    -I"C:\glfw-3.4\include" ^
    -L"C:\glfw-3.4\build\src" ^
    examples\top_down\top_down.cpp ^
    vulkan\eden_vulkan_helpers.cpp ^
    third_party\imgui\imgui.cpp ^
    third_party\imgui\imgui_draw.cpp ^
    third_party\imgui\imgui_tables.cpp ^
    third_party\imgui\imgui_widgets.cpp ^
    third_party\imgui\backends\imgui_impl_vulkan.cpp ^
    third_party\imgui\backends\imgui_impl_glfw.cpp ^
    -lvulkan-1 -lglfw3 -lgdi32 -luser32 -lshell32 ^
    -o examples\top_down\top_down.exe

if %ERRORLEVEL% NEQ 0 (
    echo C++ Compilation Failed!
    exit /b %ERRORLEVEL%
)

echo Running...
cd examples\top_down
top_down.exe
pause

