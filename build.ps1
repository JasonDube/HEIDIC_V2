# EDEN ENGINE (Heidic V2) Build Script
# Usage: .\build.ps1 examples\top_down\top_down.hd

param(
    [Parameter(Mandatory=$true)]
    [string]$SourceFile
)

# Set up PATH
$env:PATH = "$env:USERPROFILE\.cargo\bin;$env:PATH"

# Get absolute path and directory
$sourcePath = Resolve-Path $SourceFile
$sourceDir = [System.IO.Path]::GetDirectoryName($sourcePath)
$baseName = [System.IO.Path]::GetFileNameWithoutExtension($SourceFile)
$cppFile = Join-Path $sourceDir "$baseName.cpp"
$exeFile = Join-Path $sourceDir "$baseName.exe"

# Get project root (where build.ps1 is located)
$projectRoot = $PSScriptRoot
if (-not $projectRoot) {
    $projectRoot = Get-Location
}

Write-Host "=== EDEN ENGINE Build ===" -ForegroundColor Cyan
Write-Host "Compiling HEIDIC: $SourceFile" -ForegroundColor Green

# Step 1: Compile HEIDIC to C++ using cargo
Push-Location $projectRoot
& cargo run -- compile $sourcePath
$heidicResult = $LASTEXITCODE
Pop-Location

if ($heidicResult -ne 0) {
    Write-Host "HEIDIC compilation failed!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $cppFile)) {
    Write-Host "Error: Generated C++ file not found: $cppFile" -ForegroundColor Red
    exit 1
}

Write-Host "Compiling C++: $cppFile" -ForegroundColor Green

# Change to source directory for compilation
Push-Location $sourceDir

# Set up Vulkan SDK paths
$vulkanSdk = $null
if ($env:VULKAN_SDK) {
    $vulkanSdk = $env:VULKAN_SDK
    Write-Host "Using VULKAN_SDK: $vulkanSdk" -ForegroundColor Cyan
} elseif (Test-Path "C:\VulkanSDK") {
    $vulkanSdk = (Get-ChildItem "C:\VulkanSDK" | Sort-Object Name -Descending | Select-Object -First 1).FullName
    $env:VULKAN_SDK = $vulkanSdk
    Write-Host "Found Vulkan SDK: $vulkanSdk" -ForegroundColor Cyan
}

# Set up GLFW paths
$glfwPath = $null
$glfwLibPath = $null
$glfwLibFound = $false
if (Test-Path "C:\glfw-3.4") {
    $glfwPath = "C:\glfw-3.4"
    $possibleLibPaths = @(
        "$glfwPath\build\src",
        "$glfwPath\lib-mingw-w64",
        "$glfwPath\lib-vc2022\x64",
        "$glfwPath\lib\x64"
    )
    foreach ($libPath in $possibleLibPaths) {
        if (Test-Path $libPath) {
            $libFile = Get-ChildItem $libPath -Filter "libglfw3.a" -ErrorAction SilentlyContinue | Select-Object -First 1
            if (-not $libFile) {
                $libFile = Get-ChildItem $libPath -Filter "glfw3.a" -ErrorAction SilentlyContinue | Select-Object -First 1
            }
            if ($libFile) {
                $glfwLibPath = $libFile.DirectoryName
                $glfwLibFound = $true
                Write-Host "Found GLFW library at: $($libFile.FullName)" -ForegroundColor Cyan
                break
            }
        }
    }
    if (-not $glfwLibFound) {
        Write-Host "Warning: GLFW library not found. GLFW needs to be built." -ForegroundColor Yellow
    }
}

# Check for GLM
$glmPath = Join-Path $projectRoot "third_party\glm"
if (-not (Test-Path $glmPath)) {
    $glmPath = $null
    if (Test-Path "C:\glm\glm\glm.hpp") {
        $glmPath = "C:\glm"
    }
}
if ($glmPath) {
    Write-Host "Found GLM at: $glmPath" -ForegroundColor Cyan
}

# Check for ImGui
$imguiPath = Join-Path $projectRoot "third_party\imgui"
$imguiEnabled = Test-Path (Join-Path $imguiPath "imgui.h")
if ($imguiEnabled) {
    Write-Host "Found ImGui at: $imguiPath" -ForegroundColor Cyan
}

# Build common compiler flags
$commonFlags = @("-std=c++17", "-O3", "-I$projectRoot", "-I$projectRoot\stdlib")
if ($glmPath) {
    $commonFlags += "-I$glmPath"
}
if ($vulkanSdk) {
    $commonFlags += "-I$vulkanSdk\Include"
    $commonFlags += "-L$vulkanSdk\Lib"
}
if ($glfwPath) {
    $commonFlags += "-I$glfwPath\include"
    if ($glfwLibFound) {
        $commonFlags += "-L$glfwLibPath"
    }
}
if ($imguiEnabled) {
    $commonFlags += "-I$imguiPath"
    $commonFlags += "-I$imguiPath\backends"
}

# Object files directory
$objDir = Join-Path $projectRoot "vulkan\obj"
if (-not (Test-Path $objDir)) {
    New-Item -ItemType Directory -Path $objDir -Force | Out-Null
}

# Compile eden_vulkan_helpers.cpp to object file (incremental)
$helpersCppPath = Join-Path $projectRoot "vulkan\eden_vulkan_helpers.cpp"
$helpersObjPath = Join-Path $objDir "eden_vulkan_helpers.o"
$vulkanObjFiles = @()

if (Test-Path $helpersCppPath) {
    $needsCompile = $true
    if (Test-Path $helpersObjPath) {
        $helpersCppTime = (Get-Item $helpersCppPath).LastWriteTime
        $helpersObjTime = (Get-Item $helpersObjPath).LastWriteTime
        if ($helpersCppTime -lt $helpersObjTime) {
            $needsCompile = $false
        }
    }
    
    if ($needsCompile) {
        Write-Host "Compiling eden_vulkan_helpers.cpp" -ForegroundColor Cyan
        $helpersFlags = $commonFlags + @("-c", $helpersCppPath, "-o", $helpersObjPath)
        & "g++" $helpersFlags
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Failed to compile eden_vulkan_helpers!" -ForegroundColor Red
            Pop-Location
            exit 1
        }
    } else {
        Write-Host "Skipping eden_vulkan_helpers.cpp (up to date)" -ForegroundColor Gray
    }
    $vulkanObjFiles += $helpersObjPath
} else {
    Write-Host "Warning: eden_vulkan_helpers.cpp not found!" -ForegroundColor Yellow
}

# Compile ImGui source files to object files if enabled
if ($imguiEnabled) {
    $imguiFiles = @(
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "backends\imgui_impl_glfw.cpp",
        "backends\imgui_impl_vulkan.cpp"
    )
    foreach ($imguiFile in $imguiFiles) {
        $imguiCppPath = Join-Path $imguiPath $imguiFile
        if (Test-Path $imguiCppPath) {
            $imguiObjName = [System.IO.Path]::GetFileNameWithoutExtension($imguiFile) -replace "\\", "_"
            $imguiObjPath = Join-Path $objDir "$imguiObjName.o"
            
            $needsCompile = $true
            if (Test-Path $imguiObjPath) {
                $imguiCppTime = (Get-Item $imguiCppPath).LastWriteTime
                $imguiObjTime = (Get-Item $imguiObjPath).LastWriteTime
                if ($imguiCppTime -lt $imguiObjTime) {
                    $needsCompile = $false
                }
            }
            
            if ($needsCompile) {
                Write-Host "Compiling ImGui: $imguiFile" -ForegroundColor Cyan
                $imguiFlags = $commonFlags + @("-c", $imguiCppPath, "-o", $imguiObjPath)
                & "g++" $imguiFlags
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "Warning: Failed to compile ImGui file: $imguiFile" -ForegroundColor Yellow
                } else {
                    $vulkanObjFiles += $imguiObjPath
                }
            } else {
                Write-Host "Skipping $imguiFile (up to date)" -ForegroundColor Gray
                $vulkanObjFiles += $imguiObjPath
            }
        }
    }
}

# Compile main source file
Write-Host "Compiling main source: $baseName.cpp" -ForegroundColor Cyan
$mainObjPath = Join-Path $objDir "$baseName.o"
$mainCompileFlags = $commonFlags + @("-c", "$baseName.cpp", "-o", $mainObjPath)
& "g++" $mainCompileFlags
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to compile main source!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Link all object files together
Write-Host "Linking executable..." -ForegroundColor Cyan
$linkArgs = $commonFlags + @($mainObjPath) + $vulkanObjFiles + @("-o", $exeFile)
if ($vulkanSdk) {
    $linkArgs += "-lvulkan-1"
}
if ($glfwPath -and $glfwLibFound) {
    $linkArgs += "-lglfw3"
    $linkArgs += "-lgdi32"
    $linkArgs += "-luser32"
    $linkArgs += "-lshell32"
}

& "g++" $linkArgs
$linkResult = $LASTEXITCODE
Pop-Location

if ($linkResult -ne 0) {
    Write-Host "Linking failed!" -ForegroundColor Red
    exit 1
}

# Show relative path from current location
$relativeExePath = Resolve-Path $exeFile -Relative
Write-Host "=== Build successful! ===" -ForegroundColor Green
Write-Host "Run with: .\$relativeExePath" -ForegroundColor Green

