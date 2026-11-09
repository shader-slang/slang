#ifndef SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
#define SLANG_IR_INSERT_DEBUG_VALUE_STORE_H

#include "core/slang-array-view.h"

namespace Slang
{
struct IRModule;
struct IRInst;

/// Emits declarations and value bindings for source variables of every type in the module.
void insertDebugValueStore(IRModule* module);

/// Splits a pack's debug binding into existing element fragments when its parameters expand.
void expandDebugValuePackBindings(IRInst* packValue, ArrayView<IRInst*> elements);

} // namespace Slang

#endif // SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
