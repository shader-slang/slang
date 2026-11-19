// slang-serialize-ast.h
#ifndef SLANG_SERIALIZE_AST_H
#define SLANG_SERIALIZE_AST_H

#include "core/slang-riff.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"
#include "slang/ast/slang-ast-all.h"
#include "slang/ast/slang-ast-builder.h"
#include "slang/ast/slang-ast-support-types.h"

namespace Slang
{
class Linkage;

void writeSerializedModuleAST(
    RIFF::BuildCursor& cursor,
    ModuleDecl* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

ModuleDecl* readSerializedModuleAST(
    Linkage* linkage,
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    ISlangBlob* blobHoldingSerializedData,
    RIFF::Chunk const* chunk,
    SerialSourceLocReader* sourceLocReader,
    SourceLoc requestingSourceLoc);

} // namespace Slang

#endif
