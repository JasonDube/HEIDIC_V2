use crate::ast::*;
use anyhow::{Result, Context};
use std::fs;
use std::path::Path;
use std::process::Command;

pub struct CodeGenerator;

impl CodeGenerator {
    pub fn new() -> Self {
        Self
    }
    
    pub fn generate(&mut self, program: &Program) -> Result<String> {
        let mut output = String::new();
        
        // Generate includes and standard library
        output.push_str("#include <iostream>\n");
        output.push_str("#include <vector>\n");
        output.push_str("#include <string>\n");
        output.push_str("#include <unordered_map>\n");
        output.push_str("#include <memory>\n");
        output.push_str("#include <cmath>\n");
        output.push_str("#include <cstdint>\n");
        output.push_str("\n");
        
        // Include EDEN standard library (Vulkan, GLFW, GLM math, ImGui)
        output.push_str("// EDEN ENGINE Standard Library\n");
        output.push_str("#include \"stdlib/vulkan.h\"\n");
        output.push_str("#include \"stdlib/glfw.h\"\n");
        output.push_str("#include \"stdlib/math.h\"\n");
        output.push_str("#include \"stdlib/eden_imgui.h\"\n");
        output.push_str("\n");
        
        // Generate type aliases first (so they can be used by structs/components)
        for item in &program.items {
            if let Item::TypeAlias(alias) = item {
                output.push_str(&self.generate_type_alias(alias));
            }
        }
        
        // Compile and embed shaders first (before other code generation)
        let mut shaders = Vec::new();
        for item in &program.items {
            if let Item::Shader(s) = item {
                shaders.push(s.clone());
            }
        }
        
        if !shaders.is_empty() {
            output.push_str("// Embedded Shaders (compiled to SPIR-V at compile-time)\n");
            for shader in &shaders {
                output.push_str(&self.generate_shader(shader)?);
            }
            output.push_str("\n");
        }
        
        // Generate query types (ECS query structures)
        let mut query_types = Vec::new();
        for item in &program.items {
            self.collect_query_types(item, &mut query_types);
        }
        
        // Remove duplicates (simple comparison)
        query_types.sort_by(|a, b| {
            let a_str = format!("{:?}", a);
            let b_str = format!("{:?}", b);
            a_str.cmp(&b_str)
        });
        query_types.dedup_by(|a, b| {
            if a.len() != b.len() {
                return false;
            }
            a.iter().zip(b.iter()).all(|(x, y)| {
                match (x, y) {
                    (Type::Component(a_name), Type::Component(b_name)) => a_name == b_name,
                    (Type::ComponentSOA(a_name), Type::ComponentSOA(b_name)) => a_name == b_name,
                    _ => false,
                }
            })
        });
        
        if !query_types.is_empty() {
            output.push_str("// ECS Query Types\n");
            for query_type in &query_types {
                output.push_str(&self.generate_query_type(query_type)?);
            }
            output.push_str("\n");
        }
        
        // Generate structs, components, and SOA types
        for item in &program.items {
            match item {
                Item::Struct(s) => {
                    output.push_str(&self.generate_struct(s, 0));
                }
                Item::Component(c) => {
                    output.push_str(&self.generate_component(c, 0));
                }
                Item::MeshSOA(m) => {
                    output.push_str(&self.generate_mesh_soa(m, 0));
                }
                Item::ComponentSOA(c) => {
                    output.push_str(&self.generate_component_soa(c, 0));
                }
                _ => {}
            }
        }
        
        // Generate extern function declarations (C linkage)
        let mut extern_libraries = std::collections::HashSet::new();
        for item in &program.items {
            if let Item::ExternFunction(ext) = item {
                output.push_str("extern \"C\" {\n");
                let return_type = self.type_to_cpp_extern(&ext.return_type);
                output.push_str(&format!("    {} {}(", return_type, ext.name));
                for (i, param) in ext.params.iter().enumerate() {
                    if i > 0 {
                        output.push_str(", ");
                    }
                    output.push_str(&format!("{} {}", 
                        self.type_to_cpp_extern(&param.ty), 
                        param.name));
                }
                output.push_str(");\n");
                output.push_str("}\n");
                
                if let Some(ref lib) = ext.library {
                    extern_libraries.insert(lib.clone());
                }
            }
        }
        if !extern_libraries.is_empty() {
            output.push_str("\n// Link libraries: ");
            for lib in &extern_libraries {
                output.push_str(lib);
                output.push_str(" ");
            }
            output.push_str("\n");
        }
        output.push_str("\n");
        
        // Generate forward declarations for all functions
        let mut functions = Vec::new();
        let mut has_main = false;
        for item in &program.items {
            match item {
                Item::Function(f) => {
                    if f.name == "main" {
                        has_main = true;
                    }
                    functions.push(f.clone());
                    // Generate forward declaration
                    let func_name = if f.name == "main" {
                        "heidic_main".to_string()
                    } else {
                        f.name.clone()
                    };
                    let return_type = if f.name == "main" && matches!(f.return_type, Type::Void) {
                        "int".to_string()
                    } else {
                        self.type_to_cpp(&f.return_type)
                    };
                    output.push_str(&format!("{} {}(", return_type, func_name));
                    for (i, param) in f.params.iter().enumerate() {
                        if i > 0 {
                            output.push_str(", ");
                        }
                        output.push_str(&format!("{} {}", 
                            self.type_to_cpp(&param.ty), 
                            param.name));
                    }
                    output.push_str(");\n");
                }
                Item::System(s) => {
                    for func in &s.functions {
                        functions.push(func.clone());
                        // Generate forward declaration
                        output.push_str(&format!("{} {}(", 
                            self.type_to_cpp(&func.return_type), 
                            func.name));
                        for (i, param) in func.params.iter().enumerate() {
                            if i > 0 {
                                output.push_str(", ");
                            }
                            output.push_str(&format!("{} {}", 
                                self.type_to_cpp(&param.ty), 
                                param.name));
                        }
                        output.push_str(");\n");
                    }
                }
                _ => {}
            }
        }
        output.push_str("\n");
        
        // Generate function implementations
        for f in &functions {
            output.push_str(&self.generate_function(f, 0));
        }
        
        // Add C++ main wrapper if HEIDIC main exists
        if has_main {
            output.push_str("int main(int argc, char* argv[]) {\n");
            output.push_str("    heidic_main();\n");
            output.push_str("    return 0;\n");
            output.push_str("}\n");
        }
        
        Ok(output)
    }
    
    fn generate_struct(&self, s: &StructDef, indent: usize) -> String {
        let mut output = format!("struct {} {{\n", s.name);
        for field in &s.fields {
            output.push_str(&format!("{}    {} {};\n", 
                self.indent(indent + 1), 
                self.type_to_cpp(&field.ty), 
                field.name));
        }
        output.push_str("};\n\n");
        output
    }
    
    fn generate_component(&self, c: &ComponentDef, indent: usize) -> String {
        let mut output = format!("struct {} {{\n", c.name);
        
        // Generate fields
        for field in &c.fields {
            output.push_str(&format!("{}    {} {};\n", 
                self.indent(indent + 1), 
                self.type_to_cpp(&field.ty), 
                field.name));
        }
        
        // Generate constructor with default values
        let has_defaults = c.fields.iter().any(|f| f.default_value.is_some());
        if has_defaults {
            output.push_str(&format!("{}\n", self.indent(indent + 1)));
            output.push_str(&format!("{}    {}({}) {{\n", 
                self.indent(indent + 1), 
                c.name,
                self.generate_constructor_params(&c.fields)));
            
            // Initialize fields
            for field in &c.fields {
                if let Some(ref default_expr) = field.default_value {
                    output.push_str(&format!("{}        {} = {};\n",
                        self.indent(indent + 1),
                        field.name,
                        self.generate_expression(default_expr)));
                }
            }
            output.push_str(&format!("{}    }}\n", self.indent(indent + 1)));
        }
        
        output.push_str("};\n\n");
        output
    }
    
    fn generate_mesh_soa(&self, m: &MeshSOADef, indent: usize) -> String {
        // SOA structures: each field is a separate array
        // This generates the same structure as regular structs, but semantically
        // represents Structure-of-Arrays layout (optimized for CUDA/OptiX)
        let mut output = format!("// SOA (Structure-of-Arrays) mesh data - optimized for CUDA/OptiX\n");
        output.push_str(&format!("struct {} {{\n", m.name));
        
        for field in &m.fields {
            output.push_str(&format!("{}    {} {};\n", 
                self.indent(indent + 1), 
                self.type_to_cpp(&field.ty), 
                field.name));
        }
        
        output.push_str("};\n\n");
        output
    }
    
    fn generate_component_soa(&self, c: &ComponentSOADef, indent: usize) -> String {
        // SOA components: each field is a separate array
        // This is the preferred layout for ECS systems (better cache performance)
        let mut output = format!("// SOA (Structure-of-Arrays) component - optimized for ECS iteration\n");
        output.push_str(&format!("struct {} {{\n", c.name));
        
        for field in &c.fields {
            output.push_str(&format!("{}    {} {};\n", 
                self.indent(indent + 1), 
                self.type_to_cpp(&field.ty), 
                field.name));
        }
        
        output.push_str("};\n\n");
        output
    }
    
    fn generate_constructor_params(&self, fields: &[Field]) -> String {
        let mut params = Vec::new();
        for field in fields {
            if let Some(ref default_expr) = field.default_value {
                params.push(format!("{} {} = {}",
                    self.type_to_cpp(&field.ty),
                    field.name,
                    self.generate_expression(default_expr)));
            } else {
                params.push(format!("{} {}",
                    self.type_to_cpp(&field.ty),
                    field.name));
            }
        }
        params.join(", ")
    }
    
    fn generate_type_alias(&self, alias: &TypeAliasDef) -> String {
        let target_type = self.type_to_cpp(&alias.target_type);
        format!("using {} = {};\n", alias.name, target_type)
    }
    
    fn generate_function(&self, f: &FunctionDef, indent: usize) -> String {
        let mut output = String::new();
        
        // Rename HEIDIC main to avoid conflict with C++ main
        let func_name = if f.name == "main" {
            "heidic_main".to_string()
        } else {
            f.name.clone()
        };
        
        // If it's the main function with void return, change to int for C++
        let return_type = if f.name == "main" && matches!(f.return_type, Type::Void) {
            "int".to_string()
        } else {
            self.type_to_cpp(&f.return_type)
        };
        
        output.push_str(&format!("{} {}(", return_type, func_name));
        
        // Parameters
        for (i, param) in f.params.iter().enumerate() {
            if i > 0 {
                output.push_str(", ");
            }
            output.push_str(&format!("{} {}", 
                self.type_to_cpp(&param.ty), 
                param.name));
        }
        output.push_str(") {\n");
        
        for stmt in &f.body {
            output.push_str(&self.generate_statement(stmt, indent + 1));
        }
        
        // If it's main with void return type, add return 0
        if f.name == "main" && matches!(f.return_type, Type::Void) {
            output.push_str(&format!("{}    return 0;\n", self.indent(indent + 1)));
        }
        
        output.push_str("}\n\n");
        output
    }
    
    fn generate_statement(&self, stmt: &Statement, indent: usize) -> String {
        match stmt {
            Statement::Let { name, ty, value } => {
                let type_str = if let Some(ty) = ty {
                    self.type_to_cpp(ty)
                } else {
                    "auto".to_string()
                };
                format!("{}    {} {} = {};\n", 
                    self.indent(indent),
                    type_str,
                    name,
                    self.generate_expression(value))
            }
            Statement::Assign { target, value } => {
                format!("{}    {} = {};\n",
                    self.indent(indent),
                    self.generate_expression(target),
                    self.generate_expression(value))
            }
            Statement::If { condition, then_block, else_block } => {
                let mut output = format!("{}    if ({}) {{\n", 
                    self.indent(indent),
                    self.generate_expression(condition));
                for stmt in then_block {
                    output.push_str(&self.generate_statement(stmt, indent + 1));
                }
                if let Some(else_block) = else_block {
                    output.push_str(&format!("{}    }} else {{\n", self.indent(indent)));
                    for stmt in else_block {
                        output.push_str(&self.generate_statement(stmt, indent + 1));
                    }
                }
                output.push_str(&format!("{}    }}\n", self.indent(indent)));
                output
            }
            Statement::While { condition, body } => {
                let mut output = format!("{}    while ({}) {{\n", 
                    self.indent(indent),
                    self.generate_expression(condition));
                for stmt in body {
                    output.push_str(&self.generate_statement(stmt, indent + 1));
                }
                output.push_str(&format!("{}    }}\n", self.indent(indent)));
                output
            }
            Statement::Loop { body } => {
                let mut output = format!("{}    while (true) {{\n", self.indent(indent));
                for stmt in body {
                    output.push_str(&self.generate_statement(stmt, indent + 1));
                }
                output.push_str(&format!("{}    }}\n", self.indent(indent)));
                output
            }
            Statement::Return(expr) => {
                if let Some(expr) = expr {
                    format!("{}    return {};\n",
                        self.indent(indent),
                        self.generate_expression(expr))
                } else {
                    format!("{}    return 0;\n", self.indent(indent))
                }
            }
            Statement::Expression(expr) => {
                format!("{}    {};\n",
                    self.indent(indent),
                    self.generate_expression(expr))
            }
            Statement::Block(stmts) => {
                let mut output = format!("{}    {{\n", self.indent(indent));
                for stmt in stmts {
                    output.push_str(&self.generate_statement(stmt, indent + 1));
                }
                output.push_str(&format!("{}    }}\n", self.indent(indent)));
                output
            }
        }
    }
    
    fn generate_expression(&self, expr: &Expression) -> String {
        match expr {
            Expression::Literal(lit) => {
                match lit {
                    Literal::Int(n) => n.to_string(),
                    Literal::Float(n) => n.to_string(),
                    Literal::Bool(b) => b.to_string(),
                    Literal::String(s) => format!("\"{}\"", s),
                }
            }
            Expression::Variable(name) => name.clone(),
            Expression::BinaryOp { op, left, right } => {
                let op_str = match op {
                    BinaryOp::Add => "+",
                    BinaryOp::Sub => "-",
                    BinaryOp::Mul => "*",
                    BinaryOp::Div => "/",
                    BinaryOp::Mod => "%",
                    BinaryOp::Eq => "==",
                    BinaryOp::Ne => "!=",
                    BinaryOp::Lt => "<",
                    BinaryOp::Le => "<=",
                    BinaryOp::Gt => ">",
                    BinaryOp::Ge => ">=",
                    BinaryOp::And => "&&",
                    BinaryOp::Or => "||",
                };
                format!("({} {} {})", 
                    self.generate_expression(left),
                    op_str,
                    self.generate_expression(right))
            }
            Expression::UnaryOp { op, expr } => {
                let op_str = match op {
                    UnaryOp::Neg => "-",
                    UnaryOp::Not => "!",
                };
                format!("{}({})", op_str, self.generate_expression(expr))
            }
            Expression::Call { name, args } => {
                // Handle built-in print function
                if name == "print" {
                    let mut output = String::from("std::cout");
                    for arg in args {
                        output.push_str(" << ");
                        output.push_str(&self.generate_expression(arg));
                    }
                    output.push_str(" << std::endl");
                    return output;
                }
                
                // Handle ImGui function calls (convert to ImGui:: namespace)
                if name.starts_with("ImGui_") || name.starts_with("ImGui::") {
                    let imgui_name = if name.starts_with("ImGui_") {
                        format!("ImGui::{}", &name[6..])
                    } else {
                        name.clone()
                    };
                    let mut output = format!("{}(", imgui_name);
                    for (i, arg) in args.iter().enumerate() {
                        if i > 0 {
                            output.push_str(", ");
                        }
                        output.push_str(&self.generate_expression(arg));
                    }
                    output.push_str(")");
                    return output;
                }
                
                // Regular function call
                let mut output = format!("{}(", name);
                for (i, arg) in args.iter().enumerate() {
                    if i > 0 {
                        output.push_str(", ");
                    }
                    let arg_expr = self.generate_expression(arg);
                    output.push_str(&arg_expr);
                }
                output.push_str(")");
                output
            }
            Expression::MemberAccess { object, member } => {
                format!("{}.{}", 
                    self.generate_expression(object),
                    member)
            }
            Expression::Index { array, index } => {
                format!("{}[{}]", 
                    self.generate_expression(array),
                    self.generate_expression(index))
            }
            Expression::StructLiteral { name, fields } => {
                let mut output = format!("{} {{", name);
                for (i, (field_name, value)) in fields.iter().enumerate() {
                    if i > 0 {
                        output.push_str(", ");
                    }
                    output.push_str(&format!(".{} = {}", 
                        field_name,
                        self.generate_expression(value)));
                }
                output.push_str("}");
                output
            }
        }
    }
    
    fn type_to_cpp(&self, ty: &Type) -> String {
        self.type_to_cpp_internal(ty, false)
    }

    fn type_to_cpp_extern(&self, ty: &Type) -> String {
        self.type_to_cpp_internal(ty, true)
    }

    fn type_to_cpp_internal(&self, ty: &Type, is_extern: bool) -> String {
        match ty {
            Type::I32 => "int32_t".to_string(),
            Type::I64 => "int64_t".to_string(),
            Type::F32 => "float".to_string(),
            Type::F64 => "double".to_string(),
            Type::Bool => "bool".to_string(),
            Type::String => {
                if is_extern {
                    "const char*".to_string()
                } else {
                    "std::string".to_string()
                }
            },
            Type::Array(element_type) => {
                format!("std::vector<{}>", self.type_to_cpp(element_type))
            }
            Type::Struct(name) => name.clone(),
            Type::Component(name) => name.clone(),
            Type::MeshSOA(name) => name.clone(),
            Type::ComponentSOA(name) => name.clone(),
            Type::Void => "void".to_string(),
            // Vulkan types
            Type::VkInstance => "VkInstance".to_string(),
            Type::VkDevice => "VkDevice".to_string(),
            Type::VkResult => "VkResult".to_string(),
            Type::VkPhysicalDevice => "VkPhysicalDevice".to_string(),
            Type::VkQueue => "VkQueue".to_string(),
            Type::VkCommandPool => "VkCommandPool".to_string(),
            Type::VkCommandBuffer => "VkCommandBuffer".to_string(),
            Type::VkSwapchainKHR => "VkSwapchainKHR".to_string(),
            Type::VkSurfaceKHR => "VkSurfaceKHR".to_string(),
            Type::VkRenderPass => "VkRenderPass".to_string(),
            Type::VkPipeline => "VkPipeline".to_string(),
            Type::VkFramebuffer => "VkFramebuffer".to_string(),
            Type::VkBuffer => "VkBuffer".to_string(),
            Type::VkImage => "VkImage".to_string(),
            Type::VkImageView => "VkImageView".to_string(),
            Type::VkSemaphore => "VkSemaphore".to_string(),
            Type::VkFence => "VkFence".to_string(),
            // GLFW types - GLFWwindow is already a pointer type in GLFW
            Type::GLFWwindow => "GLFWwindow*".to_string(), // GLFWwindow is GLFWwindow* in C
            Type::GLFWbool => "int32_t".to_string(), // GLFWbool is int32_t
            // Math types (mapped to GLM via stdlib/math.h)
            Type::Vec2 => "Vec2".to_string(),
            Type::Vec3 => "Vec3".to_string(),
            Type::Vec4 => "Vec4".to_string(),
            Type::Mat4 => "Mat4".to_string(),
            Type::Shader(name) => name.clone(),
            Type::Query(component_types) => {
                // Generate query type name: Query_Component1_Component2_...
                let type_names: Vec<String> = component_types.iter()
                    .map(|t| {
                        match t {
                            Type::Component(name) | Type::ComponentSOA(name) => name.clone(),
                            Type::Struct(name) => name.clone(), // Struct name used as component name
                            _ => format!("Unknown"),
                        }
                    })
                    .collect();
                format!("Query_{}", type_names.join("_"))
            }
        }
    }
    
    fn indent(&self, level: usize) -> String {
        "    ".repeat(level)
    }
    
    fn generate_shader(&self, shader: &ShaderDef) -> Result<String> {
        let mut output = String::new();
        
        // Determine glslc shader stage flag
        let stage_flag = match shader.stage {
            ShaderStage::Vertex => "vertex",
            ShaderStage::Fragment => "fragment",
            ShaderStage::Compute => "compute",
            ShaderStage::Geometry => "geometry",
            ShaderStage::TessellationControl => "tessellationControl",
            ShaderStage::TessellationEvaluation => "tessellationEvaluation",
        };
        
        // Find shader source file
        let shader_path = Path::new(&shader.path);
        let mut found_path = None;
        
        if shader_path.exists() {
            found_path = Some(shader_path.to_path_buf());
        } else {
            // Try relative paths
            let possible_paths = vec![
                shader_path.to_path_buf(),
                Path::new("shaders").join(shader_path.file_name().unwrap_or_default()),
                Path::new("../shaders").join(shader_path.file_name().unwrap_or_default()),
            ];
            
            for path in &possible_paths {
                if path.exists() {
                    found_path = Some(path.clone());
                    break;
                }
            }
        }
        
        if let Some(path) = found_path {
                // Compile shader to SPIR-V
                let spv_path = path.with_extension("spv");
                
                // Run glslc to compile shader
                let output_cmd = Command::new("glslc")
                    .arg("-fshader-stage")
                    .arg(stage_flag)
                    .arg(&path)
                    .arg("-o")
                    .arg(&spv_path)
                    .output()
                    .with_context(|| format!("Failed to run glslc. Make sure glslc is in PATH. Shader: {}", shader.path))?;
                
                if !output_cmd.status.success() {
                    let stderr = String::from_utf8_lossy(&output_cmd.stderr);
                    anyhow::bail!("Shader compilation failed for {}:\n{}", shader.path, stderr);
                }
                
                // Read SPIR-V bytecode
                let spirv_bytes = fs::read(&spv_path)
                    .with_context(|| format!("Failed to read compiled SPIR-V file: {}", spv_path.display()))?;
                
                // Generate C++ code with embedded SPIR-V
                output.push_str(&format!("// Shader: {} ({} stage)\n", shader.name, stage_flag));
                output.push_str(&format!("static const uint8_t {}_spirv[] = {{\n", shader.name));
                
                // Write bytes in hex format
                for (i, byte) in spirv_bytes.iter().enumerate() {
                    if i % 16 == 0 {
                        output.push_str("    ");
                    }
                    output.push_str(&format!("0x{:02x},", byte));
                    if i % 16 == 15 || i == spirv_bytes.len() - 1 {
                        output.push_str("\n");
                    } else {
                        output.push_str(" ");
                    }
                }
                
                output.push_str("};\n");
                output.push_str(&format!("static const size_t {}_spirv_size = sizeof({}_spirv);\n\n", 
                    shader.name, shader.name));
                
                // Generate helper function to load shader
                output.push_str(&format!(
                    "// Helper function to load {} shader\n",
                    shader.name
                ));
                output.push_str(&format!(
                    "inline std::vector<uint32_t> load_{}_shader() {{\n",
                    shader.name
                ));
                output.push_str(&format!(
                    "    return std::vector<uint32_t>(reinterpret_cast<const uint32_t*>({}_spirv),\n",
                    shader.name
                ));
                output.push_str(&format!(
                    "        reinterpret_cast<const uint32_t*>({}_spirv) + ({}_spirv_size / sizeof(uint32_t)));\n",
                    shader.name, shader.name
                ));
                output.push_str("}\n\n");
        } else {
            anyhow::bail!("Shader file not found: {}", shader.path);
        }
        
        Ok(output)
    }
    
    fn collect_query_types(&self, item: &Item, query_types: &mut Vec<Vec<Type>>) {
        match item {
            Item::Function(f) => {
                // Check parameters
                for param in &f.params {
                    if let Type::Query(component_types) = &param.ty {
                        query_types.push(component_types.clone());
                    }
                }
                // Check return type
                if let Type::Query(component_types) = &f.return_type {
                    query_types.push(component_types.clone());
                }
            }
            _ => {}
        }
    }
    
    fn generate_query_type(&self, component_types: &Vec<Type>) -> Result<String> {
        let mut output = String::new();
        
        // Generate query struct name
        let type_names: Vec<String> = component_types.iter()
            .map(|t| {
                match t {
                    Type::Component(name) | Type::ComponentSOA(name) => name.clone(),
                    Type::Struct(name) => name.clone(), // Struct name used as component name
                    _ => "Unknown".to_string(),
                }
            })
            .collect();
        let query_name = format!("Query_{}", type_names.join("_"));
        
        // Generate query struct
        output.push_str(&format!("// ECS Query for components: {}\n", type_names.join(", ")));
        output.push_str(&format!("struct {} {{\n", query_name));
        
        // For each component type, add a reference to its storage
        for (i, comp_type) in component_types.iter().enumerate() {
            let comp_name = match comp_type {
                Type::Component(name) | Type::ComponentSOA(name) => name.clone(),
                Type::Struct(name) => name.clone(), // Struct name used as component name
                _ => format!("Component{}", i),
            };
            
            // Generate a simple iterator/storage reference
            output.push_str(&format!("    std::vector<{}>* {}_components;\n", comp_name, comp_name.to_lowercase()));
        }
        
        output.push_str(&format!("    size_t count;  // Number of entities with all components\n"));
        output.push_str("};\n\n");
        
        // Generate helper function to iterate over query
        output.push_str(&format!("// Helper to iterate over {} query\n", query_name));
        output.push_str(&format!("template<typename Func>\n"));
        output.push_str(&format!("void for_each_{}({}& query, Func func) {{\n", 
            query_name.to_lowercase(), query_name));
        output.push_str("    for (size_t i = 0; i < query.count; ++i) {\n");
        
        // Generate entity access code
        output.push_str("        // Access components for entity i\n");
        for (i, comp_type) in component_types.iter().enumerate() {
            let comp_name = match comp_type {
                Type::Component(name) | Type::ComponentSOA(name) => name.clone(),
                Type::Struct(name) => name.clone(), // Struct name used as component name
                _ => format!("Component{}", i),
            };
            let var_name = comp_name.to_lowercase();
            output.push_str(&format!("        {}& {} = (*query.{}_components)[i];\n", 
                comp_name, var_name, var_name));
        }
        
        output.push_str("        // Call function with entity components\n");
        output.push_str("        func(");
        for (i, comp_type) in component_types.iter().enumerate() {
            if i > 0 {
                output.push_str(", ");
            }
            let comp_name = match comp_type {
                Type::Component(ref name) | Type::ComponentSOA(ref name) => name.to_lowercase(),
                Type::Struct(ref name) => name.to_lowercase(), // Struct name used as component name
                _ => format!("component{}", i),
            };
            output.push_str(&comp_name);
        }
        output.push_str(");\n");
        output.push_str("    }\n");
        output.push_str("}\n\n");
        
        Ok(output)
    }
}
