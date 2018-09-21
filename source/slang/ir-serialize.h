// ir-serialize.h
#ifndef SLANG_IR_SERIALIZE_H_INCLUDED
#define SLANG_IR_SERIALIZE_H_INCLUDED

#include "../core/basic.h"
#include "../core/stream.h"

#include "ir.h"

namespace Slang {

Result serializeModule(IRModule* module, Stream* stream);

} // namespace Slang

#endif
