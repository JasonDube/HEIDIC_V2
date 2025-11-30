use std::fs;
use std::path::{Path, PathBuf};
use anyhow::{Context, Result};

mod lexer;
mod parser;
mod ast;
mod type_checker;
mod codegen;

use lexer::Lexer;
use parser::Parser;
use type_checker::TypeChecker;
use codegen::CodeGenerator;

fn main() -> Result<()> {
    let args: Vec<String> = std::env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Usage: heidic_v2 <command> [args...]");
        eprintln!("Commands:");
        eprintln!("  compile <file>  - Compile a HEIDIC v2 source file");
        eprintln!("  run <file>      - Compile and run a HEIDIC v2 source file");
        return Ok(());
    }
    
    let command = &args[1];
    
    match command.as_str() {
        "compile" => {
            if args.len() < 3 {
                anyhow::bail!("Usage: heidic_v2 compile <file>");
            }
            let file_path = &args[2];
            compile_file(file_path)?;
        }
        "run" => {
            if args.len() < 3 {
                anyhow::bail!("Usage: heidic_v2 run <file>");
            }
            let file_path = &args[2];
            compile_and_run(file_path)?;
        }
        _ => {
            anyhow::bail!("Unknown command: {}. Use 'compile' or 'run'", command);
        }
    }
    
    Ok(())
}

fn compile_file(file_path: &str) -> Result<()> {
    let source = fs::read_to_string(file_path)
        .with_context(|| format!("Failed to read file: {}", file_path))?;
    
    // Lexical analysis
    let mut lexer = Lexer::new(&source);
    let tokens = lexer.tokenize()?;
    
    // Parsing (with file path for include resolution)
    let file_path_buf = PathBuf::from(file_path);
    let mut parser = Parser::new_with_file(tokens, file_path_buf);
    let ast = parser.parse()?;
    
    // Type checking
    let mut type_checker = TypeChecker::new();
    type_checker.check(&ast)?;
    
    // Code generation
    let mut codegen = CodeGenerator::new();
    let cpp_code = codegen.generate(&ast)?;
    
    // Write output in the same directory as the source file
    let source_path = Path::new(file_path);
    let source_dir = source_path.parent().unwrap_or(Path::new("."));
    let output_path = source_dir.join(
        source_path
            .file_stem()
            .and_then(|s| s.to_str())
            .map(|s| format!("{}.cpp", s))
            .unwrap_or_else(|| "output.cpp".to_string())
    );
    
    fs::write(&output_path, cpp_code)
        .with_context(|| format!("Failed to write output file: {}", output_path.display()))?;
    
    println!("Compiled {} to {}", file_path, output_path.display());
    let exe_name = source_path.file_stem().unwrap().to_str().unwrap();
    println!("Compile with: g++ -std=c++17 -O3 {} -o {}", 
             output_path.display(), exe_name);
    
    Ok(())
}

fn compile_and_run(file_path: &str) -> Result<()> {
    compile_file(file_path)?;
    
    let exe_name = Path::new(file_path)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("output");
    
    // Note: In a real implementation, we'd compile and run automatically
    println!("To run: ./{}", exe_name);
    
    Ok(())
}

