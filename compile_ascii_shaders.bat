@echo off
rem Compile shaders for ascii_import_test

setlocal
cd /d "%~dp0"

if not defined VULKAN_SDK (
    echo VULKAN_SDK is not set. Please set it to your Vulkan SDK root, e.g. C:\VulkanSDK\1.4.328.1
    goto :eof
)

set GLSLC="%VULKAN_SDK%\Bin\glslc.exe"

%GLSLC% -fshader-stage=vertex   ..\shaders\vert_cube.glsl -o examples\ascii_import_test\vert_cube.spv
if errorlevel 1 goto :eof

%GLSLC% -fshader-stage=fragment ..\shaders\frag_cube.glsl -o examples\ascii_import_test\frag_cube.spv
if errorlevel 1 goto :eof

echo Shaders compiled for ascii_import_test.

endlocal


