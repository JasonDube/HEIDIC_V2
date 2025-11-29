#[derive(Debug, Clone)]
pub enum Type {
    I32,
    I64,
    F32,
    F64,
    Bool,
    String,
    Array(Box<Type>),
    Struct(String),
    Component(String),
    MeshSOA(String),
    ComponentSOA(String),
    Void,
    // Vulkan types
    VkInstance,
    VkDevice,
    VkResult,
    VkPhysicalDevice,
    VkQueue,
    VkCommandPool,
    VkCommandBuffer,
    VkSwapchainKHR,
    VkSurfaceKHR,
    VkRenderPass,
    VkPipeline,
    VkFramebuffer,
    VkBuffer,
    VkImage,
    VkImageView,
    VkSemaphore,
    VkFence,
    // GLFW types
    GLFWwindow,
    GLFWbool,
    // Math types (mapped to GLM)
    Vec2,
    Vec3,
    Vec4,
    Mat4,
}

#[derive(Debug, Clone)]
pub struct Program {
    pub items: Vec<Item>,
}

#[derive(Debug, Clone)]
pub enum Item {
    Struct(StructDef),
    Component(ComponentDef),
    MeshSOA(MeshSOADef),
    ComponentSOA(ComponentSOADef),
    System(SystemDef),
    Function(FunctionDef),
    ExternFunction(ExternFunctionDef),
    TypeAlias(TypeAliasDef),
}

#[derive(Debug, Clone)]
pub struct TypeAliasDef {
    pub name: String,
    pub target_type: Type,
}

#[derive(Debug, Clone)]
pub struct StructDef {
    pub name: String,
    pub fields: Vec<Field>,
}

#[derive(Debug, Clone)]
pub struct ComponentDef {
    pub name: String,
    pub fields: Vec<Field>,
}

#[derive(Debug, Clone)]
pub struct MeshSOADef {
    pub name: String,
    pub fields: Vec<Field>, // All fields must be array types
}

#[derive(Debug, Clone)]
pub struct ComponentSOADef {
    pub name: String,
    pub fields: Vec<Field>, // All fields must be array types
}

#[derive(Debug, Clone)]
pub struct SystemDef {
    pub name: String,
    pub functions: Vec<FunctionDef>,
}

#[derive(Debug, Clone)]
pub struct Field {
    pub name: String,
    pub ty: Type,
    pub default_value: Option<Expression>,
}

#[derive(Debug, Clone)]
pub struct FunctionDef {
    pub name: String,
    pub params: Vec<Param>,
    pub return_type: Type,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub struct ExternFunctionDef {
    pub name: String,
    pub params: Vec<Param>,
    pub return_type: Type,
    pub library: Option<String>, // Library name to link against
}

#[derive(Debug, Clone)]
pub struct Param {
    pub name: String,
    pub ty: Type,
}

#[derive(Debug, Clone)]
pub enum Statement {
    Let { name: String, ty: Option<Type>, value: Expression },
    Assign { target: Expression, value: Expression },
    If { condition: Expression, then_block: Vec<Statement>, else_block: Option<Vec<Statement>> },
    While { condition: Expression, body: Vec<Statement> },
    Loop { body: Vec<Statement> },
    Return(Option<Expression>),
    Expression(Expression),
    Block(Vec<Statement>),
}

#[derive(Debug, Clone)]
pub enum Expression {
    Literal(Literal),
    Variable(String),
    BinaryOp { op: BinaryOp, left: Box<Expression>, right: Box<Expression> },
    UnaryOp { op: UnaryOp, expr: Box<Expression> },
    Call { name: String, args: Vec<Expression> },
    MemberAccess { object: Box<Expression>, member: String },
    Index { array: Box<Expression>, index: Box<Expression> },
    StructLiteral { name: String, fields: Vec<(String, Expression)> },
}

#[derive(Debug, Clone)]
pub enum Literal {
    Int(i64),
    Float(f64),
    Bool(bool),
    String(String),
}

#[derive(Debug, Clone)]
pub enum BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    And,
    Or,
}

#[derive(Debug, Clone)]
pub enum UnaryOp {
    Neg,
    Not,
}

