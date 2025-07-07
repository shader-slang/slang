// slang-serialize-ir.h
#ifndef SLANG_SERIALIZE_IR_H_INCLUDED
#define SLANG_SERIALIZE_IR_H_INCLUDED

#include "../core/slang-riff.h"
#include "slang-ir.h"
#include "slang-serialize-ir-types.h"
#include "slang-serialize-source-loc.h"

// For TranslationUnitRequest
// and FrontEndCompileRequest::ExtraEntryPointInfo
#include "slang-compiler.h"

namespace Slang
{

struct IRModule;

void writeSerializedModuleIR(
    RIFF::BuildCursor& cursor,
    IRModule* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

[[nodiscard]]
SlangResult readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    // ISlangBlob* blobHoldingSerializedData,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    RefPtr<IRModule>& outIRModule);

} // namespace Slang

#endif
