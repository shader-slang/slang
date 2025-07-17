#pragma once

#include "core/slang-riff.h"

namespace Slang
{

struct IRModule;
class GlobalSession;
using Session = GlobalSession;
class SerialSourceLocReader;
class SerialSourceLocWriter;

void writeSerializedModuleIR(
    RIFF::BuildCursor& cursor,
    IRModule* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

[[nodiscard]] Result readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    RefPtr<IRModule>& outIRModule);

[[nodiscard]] Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String& compilerVersion,
    UInt& version,
    String& name);

} // namespace Slang
