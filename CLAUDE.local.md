# Slang Repository Structure

For the repoName: `shader-slang/slang` repository, the deepwiki mcp server structure includes:

## Main Topics:
 - 1 Overview
 - 2 Compiler Architecture
   - 2.1 Front-End: Parsing & AST
   - 2.2 Semantic Analysis
   - 2.3 Intermediate Representation
   - 2.4 IR Lowering & Optimization
   - 2.5 Code Generation
 - 3 Session Management & API
   - 3.1 Session & Linkage System
   - 3.2 ComponentType Hierarchy
 - 4 Language Features
   - 4.1 Automatic Differentiation
   - 4.2 Module System
   - 4.3 Interfaces & Generics
   - 4.4 Capabilities System
 - 5 Target Platforms
   - 5.1 HLSL & DirectX Targets
   - 5.2 GLSL & Vulkan Targets
   - 5.3 C++, CUDA & CPU Targets
   - 5.4 Other Targets & Extensions
 - 6 Rendering System
   - 6.1 GFX API Architecture
   - 6.2 Shader Objects & Resource Binding
   - 6.3 Device & Pipeline Management
 - 7 Testing & Build Infrastructure
   - 7.1 Test Framework
   - 7.2 Build System
   - 7.3 CI/CD Pipeline
 - 8 Development Tools
   - 8.1 Language Server
   - 8.2 Utilities & Core Libraries
### MCP Tools
#### 1. `mcp__deepwiki__ask_question`
- **Purpose**: Ask specific questions about a GitHub repository
- **Parameters**:
  - `repoName` (required): GitHub repository in format "shader-slang/slang"
  - `question` (required): The question to ask about the repository
- **Usage**: Returns AI-generated answers based on repository analysis

## Build Instructions
- Build using: `cmake --build --preset debug`
- Test using: `./build/Debug/bin/slangc -target path/to/file.slang`
    - Add `-dump-ir` for IR stages / optimization logs and understanding
