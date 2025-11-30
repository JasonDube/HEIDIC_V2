use crate::ast::*;
use crate::lexer::{Token, Lexer};
use anyhow::{Result, bail, Context};
use std::fs;
use std::path::{Path, PathBuf};
use std::collections::HashSet;

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
    current_file: Option<PathBuf>, // Track current file for relative includes
    included_files: HashSet<PathBuf>, // Track included files to prevent circular includes
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self {
            tokens,
            current: 0,
            current_file: None,
            included_files: HashSet::new(),
        }
    }
    
    pub fn new_with_file(tokens: Vec<Token>, file_path: PathBuf) -> Self {
        let mut included = HashSet::new();
        included.insert(file_path.clone());
        Self {
            tokens,
            current: 0,
            current_file: Some(file_path),
            included_files: included,
        }
    }
    
    pub fn parse(&mut self) -> Result<Program> {
        let mut items = Vec::new();
        
        while !self.is_at_end() {
            let item = self.parse_item()?;
            match item {
                Item::Include(path) => {
                    // Process include: read file, parse it, merge items
                    let included_items = self.process_include(&path)?;
                    items.extend(included_items);
                }
                _ => {
                    items.push(item);
                }
            }
        }
        
        Ok(Program { items })
    }
    
    fn process_include(&mut self, include_path: &str) -> Result<Vec<Item>> {
        // Resolve include path
        // If path starts with "stdlib/", resolve relative to project root
        // Otherwise, resolve relative to current file
        let resolved_path = if include_path.starts_with("stdlib/") {
            // For stdlib includes, resolve relative to project root
            if let Some(ref current_file) = self.current_file {
                // Extract path before mutable borrow
                let current_file_path = current_file.clone();
                
                // Try to find project root by looking for Cargo.toml
                let mut search_path = current_file_path.parent();
                let mut project_root: Option<PathBuf> = None;
                
                while let Some(parent) = search_path {
                    let cargo_toml = parent.join("Cargo.toml");
                    if cargo_toml.exists() {
                        project_root = Some(parent.to_path_buf());
                        break;
                    }
                    search_path = parent.parent();
                }
                
                // If we found project root, use it
                if let Some(root) = project_root {
                    return Ok(self.process_include_from_root(&root, include_path)?);
                }
                
                // Fallback: if file is in examples/, go up 2 levels
                let file_str = current_file_path.to_string_lossy();
                if file_str.contains("examples") {
                    if let Some(examples_dir) = current_file_path.parent() {
                        if let Some(project_root) = examples_dir.parent() {
                            let project_root_buf = project_root.to_path_buf();
                            return Ok(self.process_include_from_root(&project_root_buf, include_path)?);
                        }
                    }
                }
                
                // Last resort: try relative to current file
                PathBuf::from(include_path)
            } else {
                PathBuf::from(include_path)
            }
        } else {
            // Resolve relative to current file
            if let Some(ref current_file) = self.current_file {
                let current_dir = current_file.parent().unwrap_or(Path::new("."));
                current_dir.join(include_path)
            } else {
                PathBuf::from(include_path)
            }
        };
        
        // Continue with existing logic
        self.process_include_path(resolved_path)
    }
    
    fn process_include_from_root(&mut self, project_root: &Path, include_path: &str) -> Result<Vec<Item>> {
        let resolved_path = project_root.join(include_path);
        self.process_include_path(resolved_path)
    }
    
    fn process_include_path(&mut self, resolved_path: PathBuf) -> Result<Vec<Item>> {
        
        // Check for circular includes
        if self.included_files.contains(&resolved_path) {
            bail!("Circular include detected: {}", resolved_path.display());
        }
        
        // Read and parse included file
        let source = fs::read_to_string(&resolved_path)
            .with_context(|| format!("Failed to read included file: {}", resolved_path.display()))?;
        
        let mut lexer = Lexer::new(&source);
        let tokens = lexer.tokenize()?;
        
        // Create new parser for included file
        let mut included_parser = Parser {
            tokens,
            current: 0,
            current_file: Some(resolved_path.clone()),
            included_files: self.included_files.clone(),
        };
        included_parser.included_files.insert(resolved_path);
        
        let included_program = included_parser.parse()?;
        Ok(included_program.items)
    }
    
    fn parse_item(&mut self) -> Result<Item> {
        match self.peek() {
            Token::Struct => {
                self.advance();
                Ok(Item::Struct(self.parse_struct()?))
            }
            Token::Component => {
                self.advance();
                Ok(Item::Component(self.parse_component()?))
            }
            Token::MeshSOA => {
                self.advance();
                Ok(Item::MeshSOA(self.parse_mesh_soa()?))
            }
            Token::ComponentSOA => {
                self.advance();
                Ok(Item::ComponentSOA(self.parse_component_soa()?))
            }
            Token::Shader => {
                self.advance();
                Ok(Item::Shader(self.parse_shader()?))
            }
            Token::System => {
                self.advance();
                Ok(Item::System(self.parse_system()?))
            }
            Token::Extern => {
                self.advance();
                Ok(Item::ExternFunction(self.parse_extern_function()?))
            }
            Token::Type => {
                self.advance();
                Ok(Item::TypeAlias(self.parse_type_alias()?))
            }
            Token::Include => {
                self.advance();
                Ok(Item::Include(self.parse_include()?))
            }
            Token::Fn => {
                self.advance();
                Ok(Item::Function(self.parse_function()?))
            }
            _ => bail!("Unexpected token at item level: {:?}", self.peek()),
        }
    }
    
    fn parse_include(&mut self) -> Result<String> {
        // Parse: include "path/to/file.hd"
        if let Token::StringLit(path) = self.peek() {
            let path = path.clone();
            self.advance();
            self.expect(&Token::Semicolon)?;
            Ok(path)
        } else {
            bail!("Expected string literal after include");
        }
    }
    
    fn parse_type_alias(&mut self) -> Result<TypeAliasDef> {
        let name = self.expect_ident()?;
        self.expect(&Token::Eq)?;
        let target_type = self.parse_type()?;
        self.expect(&Token::Semicolon)?;
        Ok(TypeAliasDef { name, target_type })
    }
    
    fn parse_struct(&mut self) -> Result<StructDef> {
        let name = self.expect_ident()?;
        self.expect(&Token::LBrace)?;
        
        let mut fields = Vec::new();
        while !self.check(&Token::RBrace) {
            fields.push(self.parse_field()?);
            if !self.check(&Token::RBrace) {
                self.expect(&Token::Comma)?;
            }
        }
        self.expect(&Token::RBrace)?;
        
        Ok(StructDef { name, fields })
    }
    
    fn parse_component(&mut self) -> Result<ComponentDef> {
        let name = self.expect_ident()?;
        self.expect(&Token::LBrace)?;
        
        let mut fields = Vec::new();
        while !self.check(&Token::RBrace) {
            fields.push(self.parse_field()?);
            if !self.check(&Token::RBrace) {
                self.expect(&Token::Comma)?;
            }
        }
        self.expect(&Token::RBrace)?;
        
        Ok(ComponentDef { name, fields })
    }
    
    fn parse_mesh_soa(&mut self) -> Result<MeshSOADef> {
        let name = self.expect_ident()?;
        self.expect(&Token::LBrace)?;
        
        let mut fields = Vec::new();
        while !self.check(&Token::RBrace) {
            fields.push(self.parse_field()?);
            if !self.check(&Token::RBrace) {
                self.expect(&Token::Comma)?;
            }
        }
        self.expect(&Token::RBrace)?;
        
        Ok(MeshSOADef { name, fields })
    }
    
    fn parse_component_soa(&mut self) -> Result<ComponentSOADef> {
        let name = self.expect_ident()?;
        self.expect(&Token::LBrace)?;
        
        let mut fields = Vec::new();
        while !self.check(&Token::RBrace) {
            fields.push(self.parse_field()?);
            if !self.check(&Token::RBrace) {
                self.expect(&Token::Comma)?;
            }
        }
        self.expect(&Token::RBrace)?;
        
        Ok(ComponentSOADef { name, fields })
    }
    
    fn parse_shader(&mut self) -> Result<ShaderDef> {
        // Parse shader stage
        let stage = match self.peek() {
            Token::Vertex => {
                self.advance();
                ShaderStage::Vertex
            }
            Token::Fragment => {
                self.advance();
                ShaderStage::Fragment
            }
            Token::Compute => {
                self.advance();
                ShaderStage::Compute
            }
            Token::Geometry => {
                self.advance();
                ShaderStage::Geometry
            }
            Token::TessellationControl => {
                self.advance();
                ShaderStage::TessellationControl
            }
            Token::TessellationEvaluation => {
                self.advance();
                ShaderStage::TessellationEvaluation
            }
            _ => bail!("Expected shader stage (vertex, fragment, compute, etc.), got {:?}", self.peek()),
        };
        
        // Parse shader name
        let name = self.expect_ident()?;
        
        // Parse shader path (string literal)
        let path = if let Token::StringLit(ref path_str) = *self.peek() {
            let path = path_str.clone();
            self.advance();
            path
        } else {
            bail!("Expected string literal for shader path, got {:?}", self.peek());
        };
        
        // Parse optional block (for future metadata)
        if self.check(&Token::LBrace) {
            self.advance();
            // Skip block content for now (just consume it)
            while !self.check(&Token::RBrace) {
                self.advance();
            }
            self.expect(&Token::RBrace)?;
        }
        
        Ok(ShaderDef { name, stage, path })
    }
    
    fn parse_system(&mut self) -> Result<SystemDef> {
        let name = self.expect_ident()?;
        self.expect(&Token::LBrace)?;
        
        let mut functions = Vec::new();
        while !self.check(&Token::RBrace) {
            if self.check(&Token::Fn) {
                self.advance();
                functions.push(self.parse_function()?);
            } else {
                bail!("Expected function in system");
            }
        }
        self.expect(&Token::RBrace)?;
        
        Ok(SystemDef { name, functions, attribute: None })
    }
    
    fn parse_system_attribute(&mut self) -> Result<SystemAttribute> {
        self.expect(&Token::LParen)?;
        
        // Parse system name (first argument)
        let name = self.expect_ident()?;
        let mut after = Vec::new();
        let mut before = Vec::new();
        
        // Parse optional after/before clauses
        while self.check(&Token::Comma) {
            self.advance(); // consume comma
            
            // Check for "after" or "before"
            if let Token::Ident(ref keyword) = *self.peek() {
                if keyword == "after" {
                    self.advance();
                    self.expect(&Token::Eq)?;
                    let system_name = self.expect_ident()?;
                    after.push(system_name);
                } else if keyword == "before" {
                    self.advance();
                    self.expect(&Token::Eq)?;
                    let system_name = self.expect_ident()?;
                    before.push(system_name);
                } else {
                    bail!("Expected 'after' or 'before' in @system attribute, got {}", keyword);
                }
            } else {
                break; // No more attributes
            }
        }
        
        self.expect(&Token::RParen)?;
        
        Ok(SystemAttribute { name, after, before })
    }
    
    fn parse_extern_function(&mut self) -> Result<ExternFunctionDef> {
        self.expect(&Token::Fn)?;
        let name = self.expect_ident()?;
        self.expect(&Token::LParen)?;
        
        let mut params = Vec::new();
        if !self.check(&Token::RParen) {
            loop {
                let param_name = self.expect_ident()?;
                self.expect(&Token::Colon)?;
                let param_type = self.parse_type()?;
                
                // Parse optional default value
                let default_value = if self.check(&Token::Eq) {
                    self.advance();
                    Some(self.parse_expression()?)
                } else {
                    None
                };
                
                params.push(Param {
                    name: param_name,
                    ty: param_type,
                    default_value,
                });
                
                if !self.check(&Token::Comma) {
                    break;
                }
                self.advance();
            }
        }
        self.expect(&Token::RParen)?;
        
        let return_type = if self.check(&Token::Colon) {
            self.advance();
            self.parse_type()?
        } else {
            Type::Void
        };
        
        // Optional library name: extern fn name() from "library"
        let library = if let Token::Ident(ref s) = *self.peek() {
            if s == "from" {
                self.advance(); // "from"
                let lib_token = self.peek().clone();
                if let Token::StringLit(lib_name) = lib_token {
                    self.advance();
                    Some(lib_name)
                } else {
                    None
                }
            } else {
                None
            }
        } else {
            None
        };
        
        self.expect(&Token::Semicolon)?;
        
        Ok(ExternFunctionDef {
            name,
            params,
            return_type,
            library,
        })
    }
    
    fn parse_function(&mut self) -> Result<FunctionDef> {
        // Parse optional @system attribute
        let attribute = if self.check(&Token::At) {
            self.advance();
            if let Token::Ident(ref attr_name) = *self.peek() {
                if attr_name == "system" {
                    self.advance();
                    Some(self.parse_system_attribute()?)
                } else {
                    bail!("Unknown attribute: @{}", attr_name);
                }
            } else {
                bail!("Expected attribute name after @");
            }
        } else {
            None
        };
        
        let name = self.expect_ident()?;
        self.expect(&Token::LParen)?;
        
        let mut params = Vec::new();
        if !self.check(&Token::RParen) {
            loop {
                let param_name = self.expect_ident()?;
                self.expect(&Token::Colon)?;
                let param_type = self.parse_type()?;
                // Parse optional default value
                let default_value = if self.check(&Token::Eq) {
                    self.advance();
                    Some(self.parse_expression()?)
                } else {
                    None
                };
                
                params.push(Param {
                    name: param_name,
                    ty: param_type,
                    default_value,
                });
                
                if !self.check(&Token::Comma) {
                    break;
                }
                self.advance();
            }
        }
        self.expect(&Token::RParen)?;
        
        let return_type = if self.check(&Token::Colon) {
            self.advance();
            self.parse_type()?
        } else {
            Type::Void
        };
        
        let body = self.parse_block()?;
        
        Ok(FunctionDef {
            name,
            params,
            return_type,
            body,
            attribute,
        })
    }
    
    fn parse_field(&mut self) -> Result<Field> {
        let name = self.expect_ident()?;
        self.expect(&Token::Colon)?;
        let ty = self.parse_type()?;
        let default_value = if self.check(&Token::Eq) {
            self.advance();
            Some(self.parse_expression()?)
        } else {
            None
        };
        Ok(Field { name, ty, default_value })
    }
    
    fn parse_type(&mut self) -> Result<Type> {
        match self.peek() {
            Token::I32 => {
                self.advance();
                Ok(Type::I32)
            }
            Token::I64 => {
                self.advance();
                Ok(Type::I64)
            }
            Token::F32 => {
                self.advance();
                Ok(Type::F32)
            }
            Token::F64 => {
                self.advance();
                Ok(Type::F64)
            }
            Token::Bool => {
                self.advance();
                Ok(Type::Bool)
            }
            Token::String => {
                self.advance();
                Ok(Type::String)
            }
            Token::Void => {
                self.advance();
                Ok(Type::Void)
            }
            Token::VkInstance => {
                self.advance();
                Ok(Type::VkInstance)
            }
            Token::VkDevice => {
                self.advance();
                Ok(Type::VkDevice)
            }
            Token::VkResult => {
                self.advance();
                Ok(Type::VkResult)
            }
            Token::VkPhysicalDevice => {
                self.advance();
                Ok(Type::VkPhysicalDevice)
            }
            Token::VkQueue => {
                self.advance();
                Ok(Type::VkQueue)
            }
            Token::VkCommandPool => {
                self.advance();
                Ok(Type::VkCommandPool)
            }
            Token::VkCommandBuffer => {
                self.advance();
                Ok(Type::VkCommandBuffer)
            }
            Token::VkSwapchainKHR => {
                self.advance();
                Ok(Type::VkSwapchainKHR)
            }
            Token::VkSurfaceKHR => {
                self.advance();
                Ok(Type::VkSurfaceKHR)
            }
            Token::VkRenderPass => {
                self.advance();
                Ok(Type::VkRenderPass)
            }
            Token::VkPipeline => {
                self.advance();
                Ok(Type::VkPipeline)
            }
            Token::VkFramebuffer => {
                self.advance();
                Ok(Type::VkFramebuffer)
            }
            Token::VkBuffer => {
                self.advance();
                Ok(Type::VkBuffer)
            }
            Token::VkImage => {
                self.advance();
                Ok(Type::VkImage)
            }
            Token::VkImageView => {
                self.advance();
                Ok(Type::VkImageView)
            }
            Token::VkSemaphore => {
                self.advance();
                Ok(Type::VkSemaphore)
            }
            Token::VkFence => {
                self.advance();
                Ok(Type::VkFence)
            }
            Token::GLFWwindow => {
                self.advance();
                Ok(Type::GLFWwindow)
            }
            Token::GLFWbool => {
                self.advance();
                Ok(Type::GLFWbool)
            }
            Token::Vec2 => {
                self.advance();
                Ok(Type::Vec2)
            }
            Token::Vec3 => {
                self.advance();
                Ok(Type::Vec3)
            }
            Token::Vec4 => {
                self.advance();
                Ok(Type::Vec4)
            }
            Token::Mat4 => {
                self.advance();
                Ok(Type::Mat4)
            }
            Token::Camera => {
                self.advance();
                Ok(Type::Camera)
            }
            Token::FrameArena => {
                self.advance();
                Ok(Type::FrameArena)
            }
            Token::Ident(ref name) => {
                let name_clone = name.clone();
                self.advance();
                Ok(Type::Struct(name_clone))
            }
            Token::LBracket => {
                self.advance();
                let element_type = self.parse_type()?;
                self.expect(&Token::RBracket)?;
                Ok(Type::Array(Box::new(element_type)))
            }
            Token::Query => {
                self.advance();
                self.expect(&Token::Lt)?;
                let mut component_types = Vec::new();
                
                // Parse first component type
                component_types.push(self.parse_type()?);
                
                // Parse additional component types
                while self.check(&Token::Comma) {
                    self.advance();
                    component_types.push(self.parse_type()?);
                }
                
                self.expect(&Token::Gt)?;
                Ok(Type::Query(component_types))
            }
            _ => bail!("Unexpected token in type: {:?}", self.peek()),
        }
    }
    
    fn parse_block(&mut self) -> Result<Vec<Statement>> {
        self.expect(&Token::LBrace)?;
        let mut statements = Vec::new();
        
        while !self.check(&Token::RBrace) {
            statements.push(self.parse_statement()?);
        }
        self.expect(&Token::RBrace)?;
        
        Ok(statements)
    }
    
    fn parse_statement(&mut self) -> Result<Statement> {
        match self.peek() {
            Token::Let => {
                self.advance();
                let name = self.expect_ident()?;
                let ty = if self.check(&Token::Colon) {
                    self.advance();
                    Some(self.parse_type()?)
                } else {
                    None
                };
                self.expect(&Token::Eq)?;
                let value = self.parse_expression()?;
                self.expect(&Token::Semicolon)?;
                Ok(Statement::Let { name, ty, value })
            }
            Token::If => {
                self.advance();
                // Optional parentheses around condition
                let condition = if self.check(&Token::LParen) {
                    self.advance();
                    let expr = self.parse_expression()?;
                    self.expect(&Token::RParen)?;
                    expr
                } else {
                    self.parse_expression()?
                };
                let then_block = self.parse_block()?;
                let else_block = if self.check(&Token::Else) {
                    self.advance();
                    Some(self.parse_block()?)
                } else {
                    None
                };
                Ok(Statement::If {
                    condition,
                    then_block,
                    else_block,
                })
            }
            Token::While => {
                self.advance();
                // Optional parentheses around condition
                let condition = if self.check(&Token::LParen) {
                    self.advance();
                    let expr = self.parse_expression()?;
                    self.expect(&Token::RParen)?;
                    expr
                } else {
                    self.parse_expression()?
                };
                let body = self.parse_block()?;
                Ok(Statement::While { condition, body })
            }
            Token::Loop => {
                self.advance();
                let body = self.parse_block()?;
                Ok(Statement::Loop { body })
            }
            Token::Return => {
                self.advance();
                let expr = if !self.check(&Token::Semicolon) {
                    Some(self.parse_expression()?)
                } else {
                    None
                };
                self.expect(&Token::Semicolon)?;
                Ok(Statement::Return(expr))
            }
            _ => {
                let expr = self.parse_expression()?;
                if self.check(&Token::Eq) {
                    self.advance();
                    let value = self.parse_expression()?;
                    self.expect(&Token::Semicolon)?;
                    Ok(Statement::Assign {
                        target: expr,
                        value,
                    })
                } else {
                    self.expect(&Token::Semicolon)?;
                    Ok(Statement::Expression(expr))
                }
            }
        }
    }
    
    fn parse_expression(&mut self) -> Result<Expression> {
        self.parse_assignment()
    }
    
    fn parse_assignment(&mut self) -> Result<Expression> {
        let expr = self.parse_or()?;
        Ok(expr)
    }
    
    fn parse_or(&mut self) -> Result<Expression> {
        let mut expr = self.parse_pipe()?;
        
        while self.check(&Token::OrOr) {
            self.advance();
            let right = self.parse_pipe()?;
            expr = Expression::BinaryOp {
                op: BinaryOp::Or,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_pipe(&mut self) -> Result<Expression> {
        let mut expr = self.parse_and()?;
        
        while self.check(&Token::PipeOp) {
            self.advance();
            let right = self.parse_and()?;
            // Pipe operator: a |> f() becomes f(a)
            // We need to transform this into a function call
            // For now, we'll handle it as a special binary op that codegen will transform
            expr = Expression::BinaryOp {
                op: BinaryOp::Pipe,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_and(&mut self) -> Result<Expression> {
        let mut expr = self.parse_equality()?;
        
        while self.check(&Token::AndAnd) {
            self.advance();
            let right = self.parse_equality()?;
            expr = Expression::BinaryOp {
                op: BinaryOp::And,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_equality(&mut self) -> Result<Expression> {
        let mut expr = self.parse_comparison()?;
        
        while self.check(&Token::EqEq) || self.check(&Token::Ne) {
            let op = if self.check(&Token::EqEq) {
                self.advance();
                BinaryOp::Eq
            } else {
                self.advance();
                BinaryOp::Ne
            };
            let right = self.parse_comparison()?;
            expr = Expression::BinaryOp {
                op,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_comparison(&mut self) -> Result<Expression> {
        let mut expr = self.parse_bitwise_or()?;
        
        while matches!(self.peek(), Token::Lt | Token::Le | Token::Gt | Token::Ge) {
            let op = match self.peek() {
                Token::Lt => {
                    self.advance();
                    BinaryOp::Lt
                }
                Token::Le => {
                    self.advance();
                    BinaryOp::Le
                }
                Token::Gt => {
                    self.advance();
                    BinaryOp::Gt
                }
                Token::Ge => {
                    self.advance();
                    BinaryOp::Ge
                }
                _ => unreachable!(),
            };
            let right = self.parse_bitwise_or()?;
            expr = Expression::BinaryOp {
                op,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_bitwise_or(&mut self) -> Result<Expression> {
        let mut expr = self.parse_term()?;
        
        // Parse | operator (but not |> which is PipeOp)
        while self.check(&Token::Pipe) {
            // Check if next token would make this |> instead of |
            if self.current < self.tokens.len() - 1 {
                // Peek ahead - if next is >, this is actually |> 
                match &self.tokens[self.current + 1] {
                    Token::Gt => {
                        // This is |> not |, so don't parse as bitwise OR
                        break;
                    }
                    _ => {
                        // This is a real | operator
                    }
                }
            }
            self.advance();
            let right = self.parse_term()?;
            expr = Expression::BinaryOp {
                op: BinaryOp::BitwiseOr,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_term(&mut self) -> Result<Expression> {
        let mut expr = self.parse_factor()?;
        
        while self.check(&Token::Plus) || self.check(&Token::Minus) {
            let op = if self.check(&Token::Plus) {
                self.advance();
                BinaryOp::Add
            } else {
                self.advance();
                BinaryOp::Sub
            };
            let right = self.parse_factor()?;
            expr = Expression::BinaryOp {
                op,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_factor(&mut self) -> Result<Expression> {
        let mut expr = self.parse_unary()?;
        
        while self.check(&Token::Star) || self.check(&Token::Slash) || self.check(&Token::Percent) {
            let op = match self.peek() {
                Token::Star => {
                    self.advance();
                    BinaryOp::Mul
                }
                Token::Slash => {
                    self.advance();
                    BinaryOp::Div
                }
                Token::Percent => {
                    self.advance();
                    BinaryOp::Mod
                }
                _ => unreachable!(),
            };
            let right = self.parse_unary()?;
            expr = Expression::BinaryOp {
                op,
                left: Box::new(expr),
                right: Box::new(right),
            };
        }
        
        Ok(expr)
    }
    
    fn parse_unary(&mut self) -> Result<Expression> {
        if self.check(&Token::Bang) {
            self.advance();
            let expr = self.parse_unary()?;
            return Ok(Expression::UnaryOp {
                op: UnaryOp::Not,
                expr: Box::new(expr),
            });
        }
        
        if self.check(&Token::Minus) {
            self.advance();
            let expr = self.parse_unary()?;
            return Ok(Expression::UnaryOp {
                op: UnaryOp::Neg,
                expr: Box::new(expr),
            });
        }
        
        self.parse_call()
    }
    
    fn parse_call(&mut self) -> Result<Expression> {
        let mut expr = self.parse_primary()?;
        
        loop {
            if self.check(&Token::LParen) {
                self.advance();
                
                // Check if this is a named argument call (name = value)
                let first_token = self.peek().clone();
                let is_named = if let Token::Ident(_) = first_token {
                    // Look ahead to see if next token is =
                    let saved_current = self.current;
                    if let Ok(_) = self.expect_ident() {
                        if self.check(&Token::Eq) {
                            self.current = saved_current; // Reset
                            true
                        } else {
                            self.current = saved_current; // Reset
                            false
                        }
                    } else {
                        false
                    }
                } else {
                    false
                };
                
                if is_named {
                    // Parse named arguments: f(a = 1, b = 2)
                    let mut named_args = Vec::new();
                    if !self.check(&Token::RParen) {
                        loop {
                            let arg_name = self.expect_ident()?;
                            self.expect(&Token::Eq)?;
                            let arg_value = self.parse_expression()?;
                            named_args.push((arg_name, arg_value));
                            
                            if !self.check(&Token::Comma) {
                                break;
                            }
                            self.advance();
                        }
                    }
                    self.expect(&Token::RParen)?;
                    
                    if let Expression::Variable(name) = expr {
                        expr = Expression::NamedCall { name, named_args };
                    } else {
                        bail!("Expected function name for named arguments");
                    }
                } else {
                    // Regular positional arguments
                    let mut args = Vec::new();
                    if !self.check(&Token::RParen) {
                        loop {
                            args.push(self.parse_expression()?);
                            if !self.check(&Token::Comma) {
                                break;
                            }
                            self.advance();
                        }
                    }
                    self.expect(&Token::RParen)?;
                    
                    if let Expression::Variable(name) = expr {
                        expr = Expression::Call { name, args };
                    } else {
                        bail!("Expected function name");
                    }
                }
            } else if self.check(&Token::Dot) {
                self.advance();
                let member = self.expect_ident()?;
                
                // Check if this is a method call with type arguments: frame.alloc_array<Vec3>(count)
                if self.check(&Token::Lt) {
                    // Parse type arguments
                    self.advance(); // consume '<'
                    let mut type_args = Vec::new();
                    type_args.push(self.parse_type()?);
                    while self.check(&Token::Comma) {
                        self.advance();
                        type_args.push(self.parse_type()?);
                    }
                    self.expect(&Token::Gt)?; // consume '>'
                    
                    // Now expect function call
                    if self.check(&Token::LParen) {
                        self.advance();
                        let mut args = Vec::new();
                        if !self.check(&Token::RParen) {
                            loop {
                                args.push(self.parse_expression()?);
                                if !self.check(&Token::Comma) {
                                    break;
                                }
                                self.advance();
                            }
                        }
                        self.expect(&Token::RParen)?;
                        
                        expr = Expression::MethodCall {
                            object: Box::new(expr),
                            method: member,
                            type_args: Some(type_args),
                            args,
                        };
                    } else {
                        bail!("Expected '(' after method name with type arguments");
                    }
                } else if self.check(&Token::LParen) {
                    // Regular method call without type arguments
                    self.advance();
                    let mut args = Vec::new();
                    if !self.check(&Token::RParen) {
                        loop {
                            args.push(self.parse_expression()?);
                            if !self.check(&Token::Comma) {
                                break;
                            }
                            self.advance();
                        }
                    }
                    self.expect(&Token::RParen)?;
                    
                    expr = Expression::MethodCall {
                        object: Box::new(expr),
                        method: member,
                        type_args: None,
                        args,
                    };
                } else {
                    // Regular member access
                    expr = Expression::MemberAccess {
                        object: Box::new(expr),
                        member,
                    };
                }
            } else if self.check(&Token::LBracket) {
                self.advance();
                let index = self.parse_expression()?;
                self.expect(&Token::RBracket)?;
                expr = Expression::Index {
                    array: Box::new(expr),
                    index: Box::new(index),
                };
            } else {
                break;
            }
        }
        
        Ok(expr)
    }
    
    fn parse_primary(&mut self) -> Result<Expression> {
        let token = self.peek().clone();
        match token {
            Token::Int(n) => {
                self.advance();
                // Check for unit suffix: 64.MiB
                if self.check(&Token::Dot) {
                    self.advance();
                    if let Token::Ident(ref unit) = *self.peek() {
                        let unit_upper = unit.to_uppercase();
                        if unit_upper == "MIB" || unit_upper == "KIB" || unit_upper == "GIB" {
                            self.advance();
                            Ok(Expression::UnitSuffix {
                                value: Box::new(Expression::Literal(Literal::Int(n))),
                                unit: unit_upper.clone(),
                            })
                        } else {
                            // Not a unit, treat as member access
                            Ok(Expression::Literal(Literal::Int(n)))
                        }
                    } else {
                        Ok(Expression::Literal(Literal::Int(n)))
                    }
                } else {
                    Ok(Expression::Literal(Literal::Int(n)))
                }
            }
            Token::Float(n) => {
                self.advance();
                // Check for unit suffix: 64.5.MiB
                if self.check(&Token::Dot) {
                    self.advance();
                    if let Token::Ident(ref unit) = *self.peek() {
                        let unit_upper = unit.to_uppercase();
                        if unit_upper == "MIB" || unit_upper == "KIB" || unit_upper == "GIB" {
                            self.advance();
                            Ok(Expression::UnitSuffix {
                                value: Box::new(Expression::Literal(Literal::Float(n))),
                                unit: unit_upper.clone(),
                            })
                        } else {
                            Ok(Expression::Literal(Literal::Float(n)))
                        }
                    } else {
                        Ok(Expression::Literal(Literal::Float(n)))
                    }
                } else {
                    Ok(Expression::Literal(Literal::Float(n)))
                }
            }
            Token::True => {
                self.advance();
                Ok(Expression::Literal(Literal::Bool(true)))
            }
            Token::False => {
                self.advance();
                Ok(Expression::Literal(Literal::Bool(false)))
            }
            Token::StringLit(s) => {
                self.advance();
                Ok(Expression::Literal(Literal::String(s)))
            }
            Token::Ident(name) => {
                self.advance();
                Ok(Expression::Variable(name))
            }
            Token::LParen => {
                self.advance();
                let expr = self.parse_expression()?;
                self.expect(&Token::RParen)?;
                Ok(expr)
            }
            _ => bail!("Unexpected token in expression: {:?}", self.peek()),
        }
    }
    
    fn expect_ident(&mut self) -> Result<String> {
        let token = self.peek().clone();
        match token {
            Token::Ident(name) => {
                self.advance();
                Ok(name)
            }
            _ => bail!("Expected identifier, got {:?}", self.peek()),
        }
    }
    
    fn expect(&mut self, token: &Token) -> Result<()> {
        if self.check(token) {
            self.advance();
            Ok(())
        } else {
            bail!("Expected {:?}, got {:?}", token, self.peek())
        }
    }
    
    fn check(&self, token: &Token) -> bool {
        !self.is_at_end() && std::mem::discriminant(self.peek()) == std::mem::discriminant(token)
    }
    
    fn peek(&self) -> &Token {
        &self.tokens[self.current]
    }
    
    fn advance(&mut self) {
        if !self.is_at_end() {
            self.current += 1;
        }
    }
    
    fn is_at_end(&self) -> bool {
        self.current >= self.tokens.len()
    }
}

