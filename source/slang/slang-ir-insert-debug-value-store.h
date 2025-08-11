#ifndef SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
#define SLANG_IR_INSERT_DEBUG_VALUE_STORE_H

#include "core/slang-basic.h"

namespace Slang
{
struct IRModule;
struct IRType;
struct IRFunc;
struct IRInst;

struct DebugValueStoreContext
{
    Dictionary<IRType*, bool> m_mapTypeToDebugability;
    bool isDebuggableType(IRType* type);
    void insertDebugValueStore(IRFunc* func);
    bool isTypeKind(IRInst* inst);
};
void insertDebugValueStore(DebugValueStoreContext& context, IRModule* module);

} // namespace Slang

#endif // SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
