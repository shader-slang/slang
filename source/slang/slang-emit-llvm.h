// slang-emit-llvm.h
#ifndef SLANG_EMIT_LLVM_H
#define SLANG_EMIT_LLVM_H

#include "../core/slang-basic.h"
#include "slang-ir-link.h"

namespace Slang
{

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    String& assemblyOut);

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    List<uint8_t>& objectOut);

// Generates an ISlangSharedLibrary or an error.
SlangResult emitLLVMJITFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact);


} // namespace Slang

#endif
