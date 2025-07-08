#pragma once

#include "core/slang-riff.h"

namespace Slang
{

struct IRModule;
class Session;
class SerialSourceLocReader;
class SerialSourceLocWriter;

void writeSerializedModuleIR(
    RIFF::BuildCursor& cursor,
    IRModule* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

void readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    RefPtr<IRModule>& outIRModule);

} // namespace Slang
