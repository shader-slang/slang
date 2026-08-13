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

// Insert IRDebugVar / IRDebugValue instrumentation for all functions in `module`.
//
// This function is safe to call more than once on the same module. A second call instruments
// only variables and parameters that were skipped by the first call (e.g. locals whose types
// were unresolved IRSpecialize during early lowering and became concrete after
// specializeModule). Already-instrumented params and locals are detected and skipped so no
// duplicate IRDebugVar records are emitted.
void insertDebugValueStore(DebugValueStoreContext& context, IRModule* module);

} // namespace Slang

#endif // SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
