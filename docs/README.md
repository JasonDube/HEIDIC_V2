# HEIDIC v2 (EDEN ENGINE)

A custom programming language and game engine built from scratch, featuring a compiler that targets modern C++17 and a high-performance rendering backend using Vulkan.

## Overview

HEIDIC v2 is a statically-typed, compiled language designed for game development. It integrates the **EDEN ENGINE**, a lightweight runtime providing Vulkan rendering, GLFW windowing, GLM mathematics, and ImGui debugging tools.

## Features

### Language
- **Primitives**: `i32`, `i64`, `f32`, `f64`, `bool`, `string`, `void`.
- **Composite Types**: Structs, Components (ECS), Arrays.
- **Math**: Native `Vec2`, `Vec3`, `Vec4`, `Mat4` (mapped to GLM).
- **Control Flow**: `if/else`, `while`, `loop`.
- **Interop**: `extern` function support for C/C++ bindings.

### Engine (EDEN)
- **Vulkan Backend**: High-performance 3D rendering pipeline.
- **Windowing**: GLFW integration.
- **UI**: Immediate Mode GUI (Dear ImGui) integration.
- **Coordinate System**: Right-Handed, Y-Up (1 unit = 1 cm).

## Examples

### Top Down Demo
A 3D example featuring a programmable cube and a debug camera with an ImGui control panel.

**Run**:
```bash
.\run_top_down.bat
```

## Building the Compiler

```bash
cd heidic_v2
cargo build --release
```

## Project Structure

```
heidic_v2/
├── src/                # Compiler Source (Rust)
├── stdlib/             # Standard Library Headers (C++)
├── vulkan/             # EDEN Engine Backend (C++)
├── examples/           # Example Projects (.hd)
├── third_party/        # Dependencies (ImGui, GLM)
└── run_top_down.bat    # Build Script
```

## Conventions
See `CONVENTIONS.md` for detailed information on coordinate systems, units, and DCC tool compatibility.

## Development Log
See `DEVLOG.md` for the history of development challenges and solutions.
