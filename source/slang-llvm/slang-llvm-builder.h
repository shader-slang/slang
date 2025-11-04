#ifndef SLANG_LLVM_BUILDER_H
#define SLANG_LLVM_BUILDER_H

#include "core/slang-common.h"
#include "compiler-core/slang-artifact.h"
#include "slang.h"

namespace Slang
{

struct LLVMInst;
struct LLVMType;
struct LLVMDebugNode;

struct LLVMBuilderOptions
{
    TerminatedCharSlice targetTriple;
    TerminatedCharSlice cpu;
    TerminatedCharSlice features;
    TerminatedCharSlice debugCommandLineArgs;
    SlangOptimizationLevel optLevel;
    SlangDebugInfoLevel debugLevel;
    SlangFpDenormalMode fp32DenormalMode;
    SlangFpDenormalMode fp64DenormalMode;
    bool useJIT;
    SlangFloatingPointMode fpMode;
    SlangCompileTarget target;
};

enum LLVMAttribute : uint32_t
{
    SLANG_LLVM_ATTR_NONE      = 0,
    SLANG_LLVM_ATTR_NOALIAS   = 1<<0,
    SLANG_LLVM_ATTR_NOCAPTURE = 1<<1,
    SLANG_LLVM_ATTR_READONLY  = 1<<2,
    SLANG_LLVM_ATTR_WRITEONLY = 1<<3,
};

enum LLVMFuncAttribute : uint32_t
{
    SLANG_LLVM_FUNC_ATTR_NONE              = 0,
    SLANG_LLVM_FUNC_ATTR_ALWAYSINLINE      = 1<<0,
    SLANG_LLVM_FUNC_ATTR_NOINLINE          = 1<<1,
    SLANG_LLVM_FUNC_ATTR_EXTERNALLYVISIBLE = 1<<2,
};

class ILLVMBuilder : public ISlangUnknown
{
public:
    SLANG_COM_INTERFACE(0xc426a086, 0xd334, 0x43bd, {0xb7, 0x80, 0x4e, 0x1a, 0xfa, 0x97, 0x21, 0x88})

    //==========================================================================
    // Native type layout info
    //==========================================================================
    virtual int getPointerSizeInBits() = 0;
    virtual int getStoreSizeOf(LLVMInst* value) = 0;

    //==========================================================================
    // Types
    //==========================================================================
    virtual LLVMType* getVoidType() = 0;
    virtual LLVMType* getIntType(int bitSize) = 0;
    virtual LLVMType* getFloatType(int bitSize) = 0;
    virtual LLVMType* getPointerType() = 0;
    virtual LLVMType* getVectorType(int elementCount, LLVMType* elementType) = 0;
    virtual LLVMType* getBufferType() = 0;
    virtual LLVMType* getFunctionType(LLVMType* returnType, Slice<LLVMType*> paramTypes, bool variadic = false) = 0;

    //==========================================================================
    // Global symbols
    //==========================================================================
    virtual LLVMInst* declareFunction(
        LLVMType* funcType,
        TerminatedCharSlice name,
        uint32_t attributes) = 0;
    virtual LLVMInst* getFunctionArg(LLVMInst* funcDecl, int argIndex) = 0;
    virtual void setAttribute(LLVMInst* arg, uint32_t attribute) = 0;
    virtual LLVMInst* declareGlobalVariable(
        LLVMInst* initializer,
        bool externallyVisible = false) = 0;
    virtual LLVMInst* declareGlobalVariable(
        int size,
        int alignment,
        bool externallyVisible = false) = 0;

    //==========================================================================
    // Instruction emitting
    //==========================================================================
    virtual LLVMInst* emitAlloca(int size, int alignment) = 0;
    virtual LLVMInst* emitGetElementPtr(LLVMInst* ptr, int stride, LLVMInst* index) = 0;
    virtual LLVMInst* emitStore(LLVMInst* value, LLVMInst* ptr, int alignment, bool isVolatile = false) = 0;
    virtual LLVMInst* emitLoad(LLVMType* type, LLVMInst* ptr, int alignment, bool isVolatile = false) = 0;
    virtual LLVMInst* emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned = false) = 0;
    virtual LLVMInst* emitCopy(LLVMInst* dstPtr, int dstAlign, LLVMInst* srcPtr, int srcAlign, int bytes, bool isVolatile = false) = 0;

    // Coerces the given value to the given type. Because LLVM IR does not carry
    // signedness, the information on whether the original type of 'val' is
    // signed is passed separately. This only affects integer extension.
    virtual LLVMInst* coerceNumeric(LLVMInst* src, LLVMType* dstType, bool valueIsSigned) = 0;

    // Some operations in Slang IR may have mixed scalar and vector parameters,
    // whereas LLVM IR requires only scalars or only vectors. This function
    // helps you promote each type as required.
    virtual void operationPromote(LLVMInst** aValInOut, bool aIsSigned, LLVMInst** bValInOut, bool bIsSigned) = 0;

    //==========================================================================
    // Constant values
    //==========================================================================
    virtual LLVMInst* getPoison(LLVMType* type) = 0;
    virtual LLVMInst* getConstantInt(LLVMType* type, uint64_t value) = 0;
    virtual LLVMInst* getConstantPtr(uint64_t value) = 0;
    virtual LLVMInst* getConstantFloat(LLVMType* type, double value) = 0;
    virtual LLVMInst* getConstantArray(Slice<LLVMInst*> values) = 0;
    virtual LLVMInst* getConstantString(TerminatedCharSlice literal) = 0;
    virtual LLVMInst* getConstantStruct(Slice<LLVMInst*> values) = 0;
    virtual LLVMInst* getConstantVector(Slice<LLVMInst*> values) = 0;
    virtual LLVMInst* getConstantVector(LLVMInst* value, int count) = 0;
    virtual LLVMInst* getConstantExtractElement(LLVMInst* value, int index) = 0;

    //==========================================================================
    // Debug info
    //==========================================================================
    virtual LLVMDebugNode* getDebugFallbackType(TerminatedCharSlice name) = 0;
    virtual LLVMDebugNode* getDebugVoidType() = 0;
    virtual LLVMDebugNode* getDebugIntType(const char* name, bool isSigned, int bitSize) = 0;
    virtual LLVMDebugNode* getDebugFloatType(const char* name, int bitSize) = 0;
    virtual LLVMDebugNode* getDebugPointerType(LLVMDebugNode* pointee) = 0;
    virtual LLVMDebugNode* getDebugReferenceType(LLVMDebugNode* pointee) = 0;
    virtual LLVMDebugNode* getDebugStringType() = 0;
    virtual LLVMDebugNode* getDebugVectorType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType) = 0;
    virtual LLVMDebugNode* getDebugArrayType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType) = 0;
    virtual LLVMDebugNode* getDebugStructField(
        LLVMDebugNode* type,
        TerminatedCharSlice name,
        int offset,
        int size,
        int alignment,
        LLVMDebugNode* file,
        int line
    ) = 0;
    virtual LLVMDebugNode* getDebugStructType(
        Slice<LLVMDebugNode*> fields,
        TerminatedCharSlice name,
        int size,
        int alignment,
        LLVMDebugNode* file,
        int line
    ) = 0;
    virtual LLVMDebugNode* getDebugFunctionType(LLVMDebugNode* returnType, Slice<LLVMDebugNode*> paramTypes) = 0;

    //==========================================================================
    // Code generation
    //==========================================================================
    virtual SlangResult generateAssembly(IArtifact** outArtifact) = 0;
    virtual SlangResult generateObjectCode(IArtifact** outArtifact) = 0;
    virtual SlangResult generateJITLibrary(IArtifact** outArtifact) = 0;
};

}

#endif
