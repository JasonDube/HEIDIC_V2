use crate::ast::*;
use anyhow::{Result, bail};
use std::collections::HashMap;

pub struct TypeChecker {
    symbols: HashMap<String, Type>,
    functions: HashMap<String, FunctionDef>,
    structs: HashMap<String, StructDef>,
    components: HashMap<String, ComponentDef>,
    mesh_soas: HashMap<String, MeshSOADef>,
    component_soas: HashMap<String, ComponentSOADef>,
    shaders: HashMap<String, ShaderDef>,
    type_aliases: HashMap<String, TypeAliasDef>,
}

impl TypeChecker {
    pub fn new() -> Self {
        Self {
            symbols: HashMap::new(),
            functions: HashMap::new(),
            structs: HashMap::new(),
            components: HashMap::new(),
            mesh_soas: HashMap::new(),
            component_soas: HashMap::new(),
            shaders: HashMap::new(),
            type_aliases: HashMap::new(),
        }
    }
    
    pub fn check(&mut self, program: &Program) -> Result<()> {
        // First pass: collect all definitions
        for item in &program.items {
            match item {
                Item::Struct(s) => {
                    self.structs.insert(s.name.clone(), s.clone());
                }
                Item::Component(c) => {
                    self.components.insert(c.name.clone(), c.clone());
                }
                Item::MeshSOA(m) => {
                    // Validate that all fields are arrays
                    for field in &m.fields {
                        if !matches!(field.ty, Type::Array(_)) {
                            bail!("SOA mesh field '{}' must be an array type", field.name);
                        }
                    }
                    self.mesh_soas.insert(m.name.clone(), m.clone());
                }
                Item::ComponentSOA(c) => {
                    // Validate that all fields are arrays
                    for field in &c.fields {
                        if !matches!(field.ty, Type::Array(_)) {
                            bail!("SOA component field '{}' must be an array type", field.name);
                        }
                    }
                    self.component_soas.insert(c.name.clone(), c.clone());
                }
                Item::Shader(s) => {
                    // Validate shader path exists (basic check - file existence checked at codegen time)
                    if s.path.is_empty() {
                        bail!("Shader '{}' must have a non-empty path", s.name);
                    }
                    self.shaders.insert(s.name.clone(), s.clone());
                }
                Item::Function(f) => {
                    self.functions.insert(f.name.clone(), f.clone());
                }
                Item::ExternFunction(ext) => {
                    // Create a function def from extern for type checking
                    let func_def = FunctionDef {
                        name: ext.name.clone(),
                        params: ext.params.clone(),
                        return_type: ext.return_type.clone(),
                        body: Vec::new(), // Extern functions have no body
                    };
                    self.functions.insert(ext.name.clone(), func_def);
                }
                Item::System(s) => {
                    for func in &s.functions {
                        self.functions.insert(func.name.clone(), func.clone());
                    }
                }
                Item::TypeAlias(alias) => {
                    self.type_aliases.insert(alias.name.clone(), alias.clone());
                }
            }
        }
        
        // Second pass: type check
        for item in &program.items {
            match item {
                Item::Function(f) => {
                    self.check_function(f)?;
                }
                Item::System(s) => {
                    for func in &s.functions {
                        self.check_function(func)?;
                    }
                }
                _ => {}
            }
        }
        
        Ok(())
    }
    
    fn check_function(&mut self, func: &FunctionDef) -> Result<()> {
        self.symbols.clear();
        
        // Add parameters to symbol table
        for param in &func.params {
            self.symbols.insert(param.name.clone(), param.ty.clone());
        }
        
        // Check function body
        for stmt in &func.body {
            self.check_statement(stmt)?;
        }
        
        Ok(())
    }
    
    fn check_statement(&mut self, stmt: &Statement) -> Result<()> {
        match stmt {
            Statement::Let { name, ty, value } => {
                let value_type = self.check_expression(value)?;
                if let Some(declared_type) = ty {
                    if !self.types_compatible(declared_type, &value_type) {
                        bail!("Type mismatch: expected {:?}, got {:?}", declared_type, value_type);
                    }
                    self.symbols.insert(name.clone(), declared_type.clone());
                } else {
                    self.symbols.insert(name.clone(), value_type);
                }
            }
            Statement::Assign { target, value } => {
                let target_type = self.check_expression(target)?;
                let value_type = self.check_expression(value)?;
                if !self.types_compatible(&target_type, &value_type) {
                    bail!("Type mismatch in assignment");
                }
            }
            Statement::If { condition, then_block, else_block } => {
                let cond_type = self.check_expression(condition)?;
                if !matches!(cond_type, Type::Bool) {
                    bail!("If condition must be bool");
                }
                for stmt in then_block {
                    self.check_statement(stmt)?;
                }
                if let Some(else_block) = else_block {
                    for stmt in else_block {
                        self.check_statement(stmt)?;
                    }
                }
            }
            Statement::While { condition, body } => {
                let cond_type = self.check_expression(condition)?;
                if !matches!(cond_type, Type::Bool) {
                    bail!("While condition must be bool");
                }
                for stmt in body {
                    self.check_statement(stmt)?;
                }
            }
            Statement::Loop { body } => {
                for stmt in body {
                    self.check_statement(stmt)?;
                }
            }
            Statement::Return(expr) => {
                if let Some(expr) = expr {
                    self.check_expression(expr)?;
                }
            }
            Statement::Expression(expr) => {
                self.check_expression(expr)?;
            }
            Statement::Block(stmts) => {
                for stmt in stmts {
                    self.check_statement(stmt)?;
                }
            }
        }
        Ok(())
    }
    
    fn check_expression(&self, expr: &Expression) -> Result<Type> {
        match expr {
            Expression::Literal(lit) => {
                Ok(match lit {
                    Literal::Int(_) => Type::I32,
                    Literal::Float(_) => Type::F32,
                    Literal::Bool(_) => Type::Bool,
                    Literal::String(_) => Type::String,
                })
            }
            Expression::Variable(name) => {
                self.symbols.get(name)
                    .cloned()
                    .ok_or_else(|| anyhow::anyhow!("Undefined variable: {}", name))
            }
            Expression::BinaryOp { op, left, right } => {
                let left_type = self.check_expression(left)?;
                let right_type = self.check_expression(right)?;
                
                match op {
                    BinaryOp::Add | BinaryOp::Sub | BinaryOp::Mul | BinaryOp::Div | BinaryOp::Mod => {
                        if matches!(left_type, Type::I32 | Type::I64 | Type::F32 | Type::F64) &&
                           matches!(right_type, Type::I32 | Type::I64 | Type::F32 | Type::F64) {
                            Ok(left_type) // Simplified: return left type
                        } else {
                            bail!("Arithmetic operations require numeric types");
                        }
                    }
                    BinaryOp::Eq | BinaryOp::Ne | BinaryOp::Lt | BinaryOp::Le | BinaryOp::Gt | BinaryOp::Ge => {
                        Ok(Type::Bool)
                    }
                    BinaryOp::And | BinaryOp::Or => {
                        if matches!(left_type, Type::Bool) && matches!(right_type, Type::Bool) {
                            Ok(Type::Bool)
                        } else {
                            bail!("Logical operations require bool types");
                        }
                    }
                }
            }
            Expression::UnaryOp { op, expr } => {
                let expr_type = self.check_expression(expr)?;
                match op {
                    UnaryOp::Neg => {
                        if matches!(expr_type, Type::I32 | Type::I64 | Type::F32 | Type::F64) {
                            Ok(expr_type)
                        } else {
                            bail!("Negation requires numeric type");
                        }
                    }
                    UnaryOp::Not => {
                        if matches!(expr_type, Type::Bool) {
                            Ok(Type::Bool)
                        } else {
                            bail!("Not requires bool type");
                        }
                    }
                }
            }
            Expression::Call { name, args } => {
                // Handle built-in print function
                if name == "print" {
                    // Print can take any number of arguments of any type
                    for arg in args {
                        self.check_expression(arg)?;
                    }
                    return Ok(Type::Void);
                }
                
                // Handle GLFW built-in functions
                let glfw_result = match name.as_str() {
                    "glfwInit" => {
                        if args.len() != 0 {
                            bail!("glfwInit() takes no arguments");
                        }
                        Ok(Type::I32)
                    }
                    "glfwCreateWindow" => {
                        if args.len() != 5 {
                            bail!("glfwCreateWindow() takes 5 arguments: width, height, title, monitor, share");
                        }
                        self.check_expression(&args[0])?; // width
                        self.check_expression(&args[1])?; // height
                        self.check_expression(&args[2])?; // title (string)
                        self.check_expression(&args[3])?; // monitor
                        self.check_expression(&args[4])?; // share
                        Ok(Type::GLFWwindow)
                    }
                    "glfwWindowShouldClose" => {
                        if args.len() != 1 {
                            bail!("glfwWindowShouldClose() takes 1 argument");
                        }
                        self.check_expression(&args[0])?;
                        Ok(Type::I32)
                    }
                    "glfwPollEvents" => {
                        if args.len() != 0 {
                            bail!("glfwPollEvents() takes no arguments");
                        }
                        Ok(Type::Void)
                    }
                    "glfwGetKey" => {
                        if args.len() != 2 {
                            bail!("glfwGetKey() takes 2 arguments");
                        }
                        self.check_expression(&args[0])?;
                        self.check_expression(&args[1])?;
                        Ok(Type::I32)
                    }
                    "glfwSetWindowShouldClose" => {
                        if args.len() != 2 {
                            bail!("glfwSetWindowShouldClose() takes 2 arguments");
                        }
                        self.check_expression(&args[0])?;
                        self.check_expression(&args[1])?;
                        Ok(Type::Void)
                    }
                    "glfwDestroyWindow" => {
                        if args.len() != 1 {
                            bail!("glfwDestroyWindow() takes 1 argument");
                        }
                        self.check_expression(&args[0])?;
                        Ok(Type::Void)
                    }
                    "glfwTerminate" => {
                        if args.len() != 0 {
                            bail!("glfwTerminate() takes no arguments");
                        }
                        Ok(Type::Void)
                    }
                    "glfwWindowHint" => {
                        if args.len() != 2 {
                            bail!("glfwWindowHint() takes 2 arguments: hint, value");
                        }
                        self.check_expression(&args[0])?;
                        self.check_expression(&args[1])?;
                        Ok(Type::Void)
                    }
                    _ => Err(anyhow::anyhow!("Not a built-in GLFW function")),
                };
                
                if let Ok(return_type) = glfw_result {
                    return Ok(return_type);
                }
                
                // Handle ImGui built-in functions (basic ones for now)
                let imgui_result = match name.as_str() {
                    "ImGui_Begin" | "ImGui::Begin" => {
                        if args.len() < 1 {
                            bail!("ImGui::Begin() takes at least 1 argument");
                        }
                        self.check_expression(&args[0])?; // title (string)
                        Ok(Type::Bool)
                    }
                    "ImGui_End" | "ImGui::End" => {
                        if args.len() != 0 {
                            bail!("ImGui::End() takes no arguments");
                        }
                        Ok(Type::Void)
                    }
                    "ImGui_Text" | "ImGui::Text" => {
                        if args.len() < 1 {
                            bail!("ImGui::Text() takes at least 1 argument");
                        }
                        for arg in args {
                            self.check_expression(arg)?;
                        }
                        Ok(Type::Void)
                    }
                    "ImGui_Button" | "ImGui::Button" => {
                        if args.len() < 1 {
                            bail!("ImGui::Button() takes at least 1 argument");
                        }
                        self.check_expression(&args[0])?; // label (string)
                        Ok(Type::Bool)
                    }
                    "ImGui_NewFrame" | "ImGui::NewFrame" => {
                        if args.len() != 0 {
                            bail!("ImGui::NewFrame() takes no arguments");
                        }
                        Ok(Type::Void)
                    }
                    "ImGui_Render" | "ImGui::Render" => {
                        if args.len() != 0 {
                            bail!("ImGui::Render() takes no arguments");
                        }
                        Ok(Type::Void)
                    }
                    _ => Err(anyhow::anyhow!("Not a built-in ImGui function")),
                };
                
                if let Ok(return_type) = imgui_result {
                    return Ok(return_type);
                }
                
                let func = self.functions.get(name)
                    .ok_or_else(|| anyhow::anyhow!("Undefined function: {}", name))?;
                
                if args.len() != func.params.len() {
                    bail!("Argument count mismatch for function {}", name);
                }
                
                for (arg, param) in args.iter().zip(func.params.iter()) {
                    let arg_type = self.check_expression(arg)?;
                    if !self.types_compatible(&param.ty, &arg_type) {
                        bail!("Argument type mismatch in function call {}", name);
                    }
                }
                
                Ok(func.return_type.clone())
            }
            Expression::MemberAccess { object, member: _ } => {
                // Simplified: just check object is valid
                self.check_expression(object)?;
                Ok(Type::F32) // Placeholder
            }
            Expression::Index { array, index: _ } => {
                match self.check_expression(array)? {
                    Type::Array(element_type) => Ok(*element_type),
                    _ => bail!("Index operation requires array type"),
                }
            }
            Expression::StructLiteral { name: _, fields: _ } => {
                // Placeholder
                Ok(Type::Void)
            }
        }
    }
    
    fn types_compatible(&self, expected: &Type, actual: &Type) -> bool {
        match (expected, actual) {
            (Type::I32, Type::I32) => true,
            (Type::I64, Type::I64) => true,
            (Type::F32, Type::F32) => true,
            (Type::F64, Type::F64) => true,
            (Type::Bool, Type::Bool) => true,
            (Type::String, Type::String) => true,
            (Type::Void, Type::Void) => true,
            (Type::Array(a), Type::Array(b)) => self.types_compatible(a, b),
            (Type::Struct(a), Type::Struct(b)) => a == b,
            (Type::Component(a), Type::Component(b)) => a == b,
            (Type::MeshSOA(a), Type::MeshSOA(b)) => a == b,
            (Type::ComponentSOA(a), Type::ComponentSOA(b)) => a == b,
            (Type::Shader(a), Type::Shader(b)) => a == b,
            // Vulkan types
            (Type::VkInstance, Type::VkInstance) => true,
            (Type::VkDevice, Type::VkDevice) => true,
            (Type::VkResult, Type::VkResult) => true,
            (Type::VkPhysicalDevice, Type::VkPhysicalDevice) => true,
            (Type::VkQueue, Type::VkQueue) => true,
            (Type::VkCommandPool, Type::VkCommandPool) => true,
            (Type::VkCommandBuffer, Type::VkCommandBuffer) => true,
            (Type::VkSwapchainKHR, Type::VkSwapchainKHR) => true,
            (Type::VkSurfaceKHR, Type::VkSurfaceKHR) => true,
            (Type::VkRenderPass, Type::VkRenderPass) => true,
            (Type::VkPipeline, Type::VkPipeline) => true,
            (Type::VkFramebuffer, Type::VkFramebuffer) => true,
            (Type::VkBuffer, Type::VkBuffer) => true,
            (Type::VkImage, Type::VkImage) => true,
            (Type::VkImageView, Type::VkImageView) => true,
            (Type::VkSemaphore, Type::VkSemaphore) => true,
            (Type::VkFence, Type::VkFence) => true,
            // GLFW types
            (Type::GLFWwindow, Type::GLFWwindow) => true,
            (Type::GLFWbool, Type::GLFWbool) => true,
            (Type::GLFWbool, Type::I32) => true, // GLFWbool is int32_t
            (Type::I32, Type::GLFWbool) => true,
            // Math types
            (Type::Vec2, Type::Vec2) => true,
            (Type::Vec3, Type::Vec3) => true,
            (Type::Vec4, Type::Vec4) => true,
            (Type::Mat4, Type::Mat4) => true,
            _ => false,
        }
    }
}

