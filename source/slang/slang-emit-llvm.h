// slang-emit-llvm.h
#ifndef SLANG_EMIT_LLVM_H
#define SLANG_EMIT_LLVM_H

#include "../core/slang-basic.h"
#include "slang-ir-link.h"

namespace Slang
{

// Generates a string blob for the given target triple
SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact);

// Generates an object code blob for the given target triple
SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact);

// Generates an ISlangSharedLibrary or an error.
SlangResult emitLLVMJITFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact);

} // namespace Slang

#endif
