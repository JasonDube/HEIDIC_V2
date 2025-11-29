# HEIDIC_v2 Language Tweak To-Do List

This document tracks language feature improvements and enhancements for HEIDIC_v2.

## In Progress 🚧

*(No features currently in progress)*

## Pending 📋

- [ ] **System Dependency Declaration** - `@system(render, after = Physics, before = RenderSubmit)`
  - Status: Not started
  - Priority: High
  - Effort: ~2-3 days

- [ ] **Query Syntax Enhancement** - `query<Transform, Velocity>` for ECS
  - Status: Not started
  - Priority: High
  - Effort: ~1-2 days

- [x] **Compile-Time Shader Embedding** - `shader vertex "shaders/triangle.vert"`
  - Status: ✅ Completed
  - Implementation: Added `shader` keyword, shader stage keywords (vertex, fragment, compute, etc.), `ShaderDef` to AST, parser support, type checking, and codegen that compiles shaders with glslc and embeds SPIR-V bytecode as const arrays
  - Syntax: `shader vertex "path/to/shader.glsl" { }`
  - Generated as: Embedded SPIR-V bytecode arrays with helper functions `load_<name>_shader()` to load them
  - Requirements: glslc must be in PATH

- [ ] **Frame-Scoped Memory (FrameArena)** - `frame.alloc_array<Vec3>(count)`
  - Status: Not started
  - Priority: High
  - Effort: ~3-5 days

- [ ] **Component Auto-Registration** - Automatic component registry
  - Status: Not started
  - Priority: Medium
  - Effort: ~2-3 days

- [x] **SOA (Structure of Arrays) for Mesh/Vertex/Index Data** - All mesh, vertex, and index data is SOA by default
  - Status: ✅ Completed
  - Implementation: Added `mesh_soa` keyword, `MeshSOADef` to AST, parser support, type checking (validates all fields are arrays), and codegen that generates SOA C++ structures
  - Syntax: `mesh_soa Mesh { positions: [Vec3], uvs: [Vec2], colors: [Vec3], indices: [i32] }`
  - Generated as: `struct Mesh { std::vector<Vec3> positions; std::vector<Vec2> uvs; ... }`

- [x] **SOA (Structure of Arrays) Mode for Components** - `component_soa Velocity { x: [f32], y: [f32], z: [f32] }`
  - Status: ✅ Completed
  - Implementation: Added `component_soa` keyword, `ComponentSOADef` to AST, parser support, type checking, and codegen
  - Benefits: SOA layout is cache-friendly for ECS iteration, better for vectorization, and aligns with CUDA/OptiX preferences
  - Syntax: `component_soa Velocity { x: [f32], y: [f32], z: [f32] }`

## Completed ✅

- [x] **SOA (Structure of Arrays) Support** - `mesh_soa` and `component_soa` keywords
  - Status: ✅ Completed
  - Implementation: Added SOA support for both mesh data (CUDA/OptiX optimized) and ECS components (cache-friendly iteration)
  - Syntax: `mesh_soa Mesh { positions: [Vec3], uvs: [Vec2] }` and `component_soa Velocity { x: [f32], y: [f32] }`
  - Generated as C++ structs with separate `std::vector` fields for each attribute (SOA layout)

- [x] **Vulkan Type Aliases** - `type ImageView = VkImageView;` syntax
  - Status: ✅ Completed
  - Implementation: Added `TypeAlias` to AST, parser, type checker, and codegen
  - Generated as C++ `using` statements

- [x] **Default Values in Components** - `scale: Vec3 = Vec3(1,1,1)` syntax
  - Status: ✅ Completed
  - Implementation: Extended `Field` to include optional default value, parser handles `= expression`, codegen generates constructors with default parameters

## Notes

- Features are prioritized by effort vs. value
- Quick wins (Vulkan aliases, default values) are being tackled first
- Complex features (SOA, hot-reload) are deferred until core ECS is solid

