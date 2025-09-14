#include "slang-emit-llvm.h"

using namespace slang;

namespace Slang
{

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    const List<IRFunc*>& irEntryPoints,
    String& assemblyOut)
{
    printf("TODO: LLVM IR ASSEMBLY\n");
    assemblyOut = "placeholder";
    return SLANG_OK;
}

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    const List<IRFunc*>& irEntryPoints,
    List<uint8_t>& objectOut)
{
    printf("TODO: LLVM OBJECT CODE\n");
    return SLANG_OK;
}

} // namespace Slang
