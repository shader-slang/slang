// slang-serialize-ast.h
#ifndef SLANG_SERIALIZE_AST_H
#define SLANG_SERIALIZE_AST_H

#include "core/slang-riff.h"
#include "slang-ast-all.h"
#include "slang-ast-builder.h"
#include "slang-ast-support-types.h"
#include "slang-serialize-source-loc.h"
#include "slang-serialize.h"

namespace Slang
{
class Linkage;

void writeSerializedModuleAST(
    RIFF::BuildCursor& cursor,
    ModuleDecl* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

/// Read the AST module serialized in `chunk`.
///
/// Pass `Fossil::Trust::Trusted` only for a blob that shipped inside the compiler
/// binary -- in practice just the embedded core module -- which skips validating
/// it. Any blob that came from outside the compiler, including every
/// `.slang-module` loaded from disk, must be left `Untrusted` so that the
/// validating walk proves it safe to navigate. The default is `Untrusted`, so
/// omitting the argument errs toward validating.
///
ModuleDecl* readSerializedModuleAST(
    Linkage* linkage,
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    ISlangBlob* blobHoldingSerializedData,
    RIFF::Chunk const* chunk,
    SerialSourceLocReader* sourceLocReader,
    SourceLoc requestingSourceLoc,
    Fossil::Trust trust = Fossil::Trust::Untrusted);

} // namespace Slang

#endif
