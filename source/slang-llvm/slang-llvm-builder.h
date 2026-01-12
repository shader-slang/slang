#ifndef SLANG_LLVM_BUILDER_H
#define SLANG_LLVM_BUILDER_H

// The LLVM target of Slang is split in two halves:
// * slang/slang-emit-llvm.cpp (Emitter)
// * slang-llvm/slang-llvm-builder.cpp (Builder)
//
// The Emitter handles all matters related to Slang IR, calling into Builder
// once it knows what it wants to emit. The Builder handles generating LLVM IR
// without knowledge of Slang IR or any other Slang internals.
//
// This split is done for a couple of reasons. First, the Slang compiler where
// Emitter resides, cannot directly link to LLVM. Instead, LLVM is linked to
// a separate and optional shared library, `slang-llvm.dll`. This is because as
// a dependency, LLVM is big (hundreds of megabytes) and hairy (can require
// compiler flags that we don't want for the compiler code). Many (or most) of
// the users of the Slang compiler have no need for LLVM.
//
// Secondly, this allows `slang-compiler` and `slang-llvm` to be somewhat
// independent; one can update the LLVM version of the Builder in `slang-llvm`
// without needing to recompile `slang-compiler`. It is also possible to add
// support for new instructions to the Emitter without having to recompile
// `slang-llvm`.
//
// When deciding on which side to place new code, consider:
// * Access to Slang IR or other matters internal to the compiler belong in Emitter.
// * Access to LLVM IRBuilder and code generation belong to Builder.
// * Builder should not be tied to Slang language concepts, but it should
//   cover general GPU concepts (e.g. buffers, textures etc.).
//
// A general rule of thumb is: you should be able to use Builder to generate
// code for _any_ GPU-targeting language, not just what Slang currently is. Even
// though the LLVM target only considers CPUs for now, this may not always be
// the case in the future, so it's good to retain the option to also emit
// GPU-specific things correctly.

#include "slang.h"

#include <compiler-core/slang-artifact.h>
#include <core/slang-common.h>

#ifdef SLANG_LLVM_IMPL
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Value.h"
#endif

namespace Slang
{

#ifdef SLANG_LLVM_IMPL
using LLVMInst = llvm::Value;
using LLVMType = llvm::Type;
using LLVMDebugNode = llvm::DINode;
#else
struct LLVMInst;
struct LLVMType;
struct LLVMDebugNode;
#endif

struct LLVMBuilderOptions
{
    SlangCompileTarget target;
    // Leave empty for default target (host)
    CharSlice targetTriple;
    CharSlice cpu;
    CharSlice features;
    CharSlice debugCommandLineArgs;
    SlangOptimizationLevel optLevel;
    SlangDebugInfoLevel debugLevel;
    SlangFpDenormalMode fp32DenormalMode;
    SlangFpDenormalMode fp64DenormalMode;
    SlangFloatingPointMode fpMode;
    Slice<TerminatedCharSlice> llvmArguments;
};

enum LLVMAttribute : uint32_t
{
    SLANG_LLVM_ATTR_NONE = 0,
    SLANG_LLVM_ATTR_NOALIAS = 1 << 0,
    SLANG_LLVM_ATTR_NOCAPTURE = 1 << 1,
    SLANG_LLVM_ATTR_READONLY = 1 << 2,
    SLANG_LLVM_ATTR_WRITEONLY = 1 << 3,
};

enum LLVMFuncAttribute : uint32_t
{
    SLANG_LLVM_FUNC_ATTR_NONE = 0,
    SLANG_LLVM_FUNC_ATTR_ALWAYSINLINE = 1 << 0,
    SLANG_LLVM_FUNC_ATTR_NOINLINE = 1 << 1,
    SLANG_LLVM_FUNC_ATTR_EXTERNALLYVISIBLE = 1 << 2,
    SLANG_LLVM_FUNC_ATTR_READNONE = 1 << 3,
};

// Would be nice if we could match these to either Slang or LLVM ops, but we
// can't, as neither is stable. Updating either LLVM or the Slang compiler would
// therefore break this list.
enum class LLVMCompareOp : uint32_t
{
    // Compares
    Equal = 0,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
};

enum class LLVMUnaryOp : uint32_t
{
    // Unary
    Negate = 0,
    Not
};

enum class LLVMBinaryOp : uint32_t
{
    // Unary
    Add = 0,
    Sub,
    Mul,
    Div,
    Rem,
    And,
    Or,
    Xor,
    RightShift,
    LeftShift
};

class ILLVMBuilder : public ICastable
{
public:
    SLANG_COM_INTERFACE(
        0xc426a086,
        0xd334,
        0x43bd,
        {0xb7, 0x80, 0x4e, 0x1a, 0xfa, 0x97, 0x21, 0x88})

    //==========================================================================
    // Introspection
    //==========================================================================
    virtual SLANG_NO_THROW int SLANG_MCALL getPointerSizeInBits() = 0;
    virtual SLANG_NO_THROW int SLANG_MCALL getStoreSizeOf(LLVMInst* value) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL printType(String& outStr, LLVMType* type) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
    printValue(String& outStr, LLVMInst* value, bool withType = true) = 0;

    //==========================================================================
    // Types
    //==========================================================================
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL getVoidType() = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL getIntType(int bitSize) = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL getFloatType(int bitSize) = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL getPointerType() = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL
    getVectorType(int elementCount, LLVMType* elementType) = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL getBufferType() = 0;
    virtual SLANG_NO_THROW LLVMType* SLANG_MCALL
    getFunctionType(LLVMType* returnType, Slice<LLVMType*> paramTypes, bool variadic = false) = 0;

    //==========================================================================
    // Global symbols
    //==========================================================================
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    declareFunction(LLVMType* funcType, CharSlice name, uint32_t attributes) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    getFunctionArg(LLVMInst* funcDecl, int argIndex) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
    setArgInfo(LLVMInst* arg, CharSlice name, uint32_t attributes) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL declareGlobalVariable(
        LLVMInst* initializer,
        int64_t alignment,
        bool externallyVisible = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    declareGlobalVariable(int64_t size, int64_t alignment, bool externallyVisible = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL declareGlobalConstructor() = 0;

    //==========================================================================
    // Blocks
    //==========================================================================
    virtual SLANG_NO_THROW void SLANG_MCALL
    beginFunction(LLVMInst* func, LLVMDebugNode* debugFunc) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBlock(LLVMInst* func) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL insertIntoBlock(LLVMInst* block) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL endFunction(LLVMInst* func) = 0;

    //==========================================================================
    // Instructions
    //==========================================================================
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitAlloca(int64_t size, int64_t alignment) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitGetElementPtr(LLVMInst* ptr, int64_t stride, LLVMInst* index) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitStore(LLVMInst* value, LLVMInst* ptr, int64_t alignment, bool isVolatile = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitLoad(LLVMType* type, LLVMInst* ptr, int64_t alignment, bool isVolatile = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitCopy(
        LLVMInst* dstPtr,
        int64_t dstAlign,
        LLVMInst* srcPtr,
        int64_t srcAlign,
        int64_t bytes,
        bool isVolatile = false) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCall(LLVMInst* func, Slice<LLVMInst*> params) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitExtractElement(LLVMInst* vector, LLVMInst* index) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitInsertElement(LLVMInst* vector, LLVMInst* element, LLVMInst* index) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitVectorSplat(LLVMInst* element, int64_t count) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitVectorShuffle(LLVMInst* vector, Slice<int> mask) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitfieldExtract(
        LLVMInst* value,
        LLVMInst* offset,
        LLVMInst* bits,
        LLVMType* resultType,
        bool isSigned) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitfieldInsert(
        LLVMInst* value,
        LLVMInst* insert,
        LLVMInst* offset,
        LLVMInst* bits,
        LLVMType* resultType) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCompareOp(LLVMCompareOp op, LLVMInst* a, bool aIsSigned, LLVMInst* b, bool bIsSigned) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitUnaryOp(LLVMUnaryOp op, LLVMInst* val) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBinaryOp(
        LLVMBinaryOp op,
        LLVMInst* a,
        LLVMInst* b,
        LLVMType* resultType = nullptr,
        bool resultIsSigned = false) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitSelect(LLVMInst* cond, LLVMInst* trueValue, LLVMInst* falseValue) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitReturn(LLVMInst* returnValue = nullptr) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBranch(LLVMInst* targetBlock) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCondBranch(LLVMInst* cond, LLVMInst* trueBlock, LLVMInst* falseBlock) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitSwitch(
        LLVMInst* cond,
        Slice<LLVMInst*> values,
        Slice<LLVMInst*> blocks,
        LLVMInst* defaultBlock) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitUnreachable() = 0;

    // Coerces the given value to the given type. Because LLVM IR does not carry
    // signedness, the information on whether the types are signed is passed
    // separately.
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCast(LLVMInst* src, LLVMType* dstType, bool srcIsSigned, bool dstIsSigned) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitCast(LLVMInst* src, LLVMType* dstType) = 0;

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitPrintf(LLVMInst* format, Slice<LLVMInst*> args, Slice<bool> argIsSigned) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitGetBufferPtr(LLVMInst* buffer) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitGetBufferSize(LLVMInst* buffer) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitChangeBufferStride(LLVMInst* buffer, int64_t prevStride, int64_t newStride) = 0;

    //==========================================================================
    // Constant values
    //==========================================================================
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getPoison(LLVMType* type) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantInt(LLVMType* type, uint64_t value) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantPtr(uint64_t value) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantFloat(LLVMType* type, double value) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantArray(Slice<LLVMInst*> values) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantString(CharSlice literal) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantStruct(Slice<LLVMInst*> values) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantVector(Slice<LLVMInst*> values) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantVector(LLVMInst* value, int count) = 0;
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    getConstantExtractElement(LLVMInst* value, int index) = 0;

    //==========================================================================
    // Debug info
    //==========================================================================
    virtual SLANG_NO_THROW void SLANG_MCALL setName(LLVMInst* inst, CharSlice name) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugFallbackType(CharSlice name) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugVoidType() = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugIntType(const char* name, bool isSigned, int bitSize) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFloatType(const char* name, int bitSize) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugPointerType(LLVMDebugNode* pointee) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugReferenceType(LLVMDebugNode* pointee) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStringType() = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugVectorType(
        int64_t sizeBytes,
        int64_t alignBytes,
        int64_t elementCount,
        LLVMDebugNode* elementType) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugArrayType(
        int64_t sizeBytes,
        int64_t alignBytes,
        int64_t elementCount,
        LLVMDebugNode* elementType) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStructField(
        LLVMDebugNode* type,
        CharSlice name,
        int64_t offset,
        int64_t size,
        int64_t alignment,
        LLVMDebugNode* file,
        int line) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStructType(
        Slice<LLVMDebugNode*> fields,
        CharSlice name,
        int64_t size,
        int64_t alignment,
        LLVMDebugNode* file,
        int line) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFunctionType(LLVMDebugNode* returnType, Slice<LLVMDebugNode*> paramTypes) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugFunction(
        LLVMDebugNode* funcType,
        CharSlice name,
        CharSlice linkageName,
        LLVMDebugNode* file,
        int line) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setDebugLocation(int line, int column) = 0;
    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFile(CharSlice filename, CharSlice directory, CharSlice source) = 0;

    virtual SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL emitDebugVar(
        CharSlice name,
        LLVMDebugNode* type,
        LLVMDebugNode* file = nullptr,
        int line = -1,
        int argIndex = -1) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
    emitDebugValue(LLVMDebugNode* debugVar, LLVMInst* value) = 0;

    //==========================================================================
    // Code generation
    //==========================================================================

    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitInlineIRFunction(LLVMInst* func, CharSlice content) = 0;

    // Creates a function that runs a whole workgroup of the entry point.
    // TODO: Ideally, this should vectorize over the workgroup.
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL emitComputeEntryPointWorkGroup(
        LLVMInst* entryPointFunc,
        CharSlice name,
        int xSize,
        int ySize,
        int zSize,
        int subgroupSize) = 0;
    // This generates a dispatching function for a given compute entry
    // point. It runs workgroups serially.
    virtual SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitComputeEntryPointDispatcher(LLVMInst* workGroupFunc, CharSlice name) = 0;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL generateAssembly(IArtifact** outArtifact) = 0;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL generateObjectCode(IArtifact** outArtifact) = 0;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL generateJITLibrary(IArtifact** outArtifact) = 0;
};

} // namespace Slang

#endif
