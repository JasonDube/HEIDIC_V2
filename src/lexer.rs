use logos::Logos;
use anyhow::{Result, bail};

#[derive(Logos, Debug, PartialEq, Clone)]
#[logos(skip r"[ \t\n\r]+")]
#[logos(skip r"//[^\n]*")]
pub enum Token {
    // Keywords
    #[token("fn")]
    Fn,
    #[token("let")]
    Let,
    #[token("if")]
    If,
    #[token("else")]
    Else,
    #[token("while")]
    While,
    #[token("loop")]
    Loop,
    #[token("return")]
    Return,
    #[token("struct")]
    Struct,
    #[token("component")]
    Component,
    #[token("system")]
    System,
    #[token("for")]
    For,
    #[token("in")]
    In,
    #[token("query")]
    Query,
    #[token("extern")]
    Extern,
    #[token("type")]
    Type,
    #[token("mesh_soa")]
    MeshSOA,
    #[token("component_soa")]
    ComponentSOA,
    #[token("shader")]
    Shader,
    #[token("vertex")]
    Vertex,
    #[token("fragment")]
    Fragment,
    #[token("compute")]
    Compute,
    #[token("geometry")]
    Geometry,
    #[token("tessellation_control")]
    TessellationControl,
    #[token("tessellation_evaluation")]
    TessellationEvaluation,
    
    // Types
    #[token("i32")]
    I32,
    #[token("i64")]
    I64,
    #[token("f32")]
    F32,
    #[token("f64")]
    F64,
    #[token("bool")]
    Bool,
    #[token("string")]
    String,
    #[token("void")]
    Void,
    
    // Vulkan types
    #[token("VkInstance")]
    VkInstance,
    #[token("VkDevice")]
    VkDevice,
    #[token("VkResult")]
    VkResult,
    #[token("VkPhysicalDevice")]
    VkPhysicalDevice,
    #[token("VkQueue")]
    VkQueue,
    #[token("VkCommandPool")]
    VkCommandPool,
    #[token("VkCommandBuffer")]
    VkCommandBuffer,
    #[token("VkSwapchainKHR")]
    VkSwapchainKHR,
    #[token("VkSurfaceKHR")]
    VkSurfaceKHR,
    #[token("VkRenderPass")]
    VkRenderPass,
    #[token("VkPipeline")]
    VkPipeline,
    #[token("VkFramebuffer")]
    VkFramebuffer,
    #[token("VkBuffer")]
    VkBuffer,
    #[token("VkImage")]
    VkImage,
    #[token("VkImageView")]
    VkImageView,
    #[token("VkSemaphore")]
    VkSemaphore,
    #[token("VkFence")]
    VkFence,
    
    // GLFW types
    #[token("GLFWwindow")]
    GLFWwindow,
    #[token("GLFWbool")]
    GLFWbool,
    
    // Math types (mapped to GLM)
    #[token("Vec2")]
    Vec2,
    #[token("Vec3")]
    Vec3,
    #[token("Vec4")]
    Vec4,
    #[token("Mat4")]
    Mat4,
    
    // Literals
    #[regex(r"-?\d+", |lex| lex.slice().parse().ok())]
    Int(i64),
    #[regex(r"-?\d+\.\d+", |lex| lex.slice().parse().ok())]
    Float(f64),
    #[token("true")]
    True,
    #[token("false")]
    False,
    #[regex(r#""[^"]*""#, |lex| lex.slice()[1..lex.slice().len()-1].to_string())]
    StringLit(String),
    
    // Identifiers
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*", |lex| lex.slice().to_string())]
    Ident(String),
    
    // Operators
    #[token("+")]
    Plus,
    #[token("-")]
    Minus,
    #[token("*")]
    Star,
    #[token("/")]
    Slash,
    #[token("%")]
    Percent,
    #[token("==")]
    EqEq,
    #[token("!=")]
    Ne,
    #[token("<")]
    Lt,
    #[token("<=")]
    Le,
    #[token(">")]
    Gt,
    #[token(">=")]
    Ge,
    #[token("&&")]
    AndAnd,
    #[token("||")]
    OrOr,
    #[token("!")]
    Bang,
    #[token("=")]
    Eq,
    
    // Delimiters
    #[token("(")]
    LParen,
    #[token(")")]
    RParen,
    #[token("{")]
    LBrace,
    #[token("}")]
    RBrace,
    #[token("[")]
    LBracket,
    #[token("]")]
    RBracket,
    #[token(",")]
    Comma,
    #[token(":")]
    Colon,
    #[token(";")]
    Semicolon,
    #[token(".")]
    Dot,
}

pub struct Lexer {
    source: String,
}

impl Lexer {
    pub fn new(source: &str) -> Self {
        Self {
            source: source.to_string(),
        }
    }
    
    pub fn tokenize(&mut self) -> Result<Vec<Token>> {
        let mut lexer = Token::lexer(&self.source);
        let mut tokens = Vec::new();
        
        while let Some(token_result) = lexer.next() {
            match token_result {
                Ok(token) => tokens.push(token),
                Err(_) => {
                    bail!("Lexical error at position {}", lexer.span().start);
                }
            }
        }
        
        Ok(tokens)
    }
}

