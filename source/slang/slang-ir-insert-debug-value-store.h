#ifndef SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
#define SLANG_IR_INSERT_DEBUG_VALUE_STORE_H

#include "core/slang-basic.h"

namespace Slang
{
struct IRModule;
struct IRType;
struct IRFunc;
struct IRInst;
struct IRCloneEnv;

struct DebugValueStoreContext
{
    Dictionary<IRType*, bool> m_mapTypeToDebugability;
    bool isDebuggableType(IRType* type);
    bool isDebugVarTypeSupported(IRType* type);
    void insertDebugValueStore(IRFunc* func);
    bool isTypeKind(IRInst* inst);
};
void insertDebugValueStore(DebugValueStoreContext& context, IRModule* module);

// Repairs the `DebugVarStorage` decorations of `func` after `func` was built by cloning another
// function with `env`. A cloner that copies parameters before the function body copies their
// decorations before the debug variables they name, so a copied decoration can still name the
// original function's debug variable. We point it at that debug variable's clone in `func`, or
// remove it if `func` has no clone of it.
void rebindDebugVarStorageDecorations(IRFunc* func, IRCloneEnv* env);

} // namespace Slang

#endif // SLANG_IR_INSERT_DEBUG_VALUE_STORE_H
