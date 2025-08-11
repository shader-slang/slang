// slang-compiler-fwd.h
#pragma once

//
// This file provides forward declarations that are
// commonly used by the files that get included by
// as part of the umbrella header `slang-compiler.h`.
//

namespace Slang
{
class ASTBuilder;
class EndToEndCompileRequest;
class FrontEndCompileRequest;
struct IRModule;
class Linkage;
class Module;
struct ModuleChunk;
class ProgramLayout;
class Session;
class SharedASTBuilder;
struct SharedSemanticsContext;
class TargetProgram;
class TargetRequest;
class TranslationUnitRequest;
struct TypeCheckingCache;
class TypeLayout;

using LoadedModule = Module;

} // namespace Slang
