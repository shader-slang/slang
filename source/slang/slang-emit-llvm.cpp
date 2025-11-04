#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "core/slang-char-util.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-util.h"
#include "slang-llvm/slang-llvm-builder.h"

#include <filesystem>

using namespace slang;

namespace Slang
{

// This function attempts to find names associated with a variable or function.
// `linkageName` is the name actually used as a symbol in object code and LLVM
// IR, while `prettyName` is shown to the user in a debugger.
static bool maybeGetName(
    TerminatedCharSlice* linkageNameOut,
    TerminatedCharSlice* prettyNameOut,
    IRInst* irInst)
{
    *linkageNameOut = TerminatedCharSlice();
    *prettyNameOut = TerminatedCharSlice();

    UnownedStringSlice linkageName;
    UnownedStringSlice prettyName;
    if (auto externCppDecoration = irInst->findDecoration<IRExternCppDecoration>())
    {
        linkageName = externCppDecoration->getName();
    }
    else if (auto linkageDecoration = irInst->findDecoration<IRLinkageDecoration>())
    {
        linkageName = linkageDecoration->getMangledName();
    }

    if (auto nameDecor = irInst->findDecoration<IRNameHintDecoration>())
    {
        prettyName = nameDecor->getName();
    }

    // If we don't have a pretty name or a linkage name, use the other one for
    // both.
    if (prettyName.getLength() == 0)
        prettyName = linkageName;
    else if (linkageName.getLength() == 0)
        linkageName = prettyName;

    if (prettyName.getLength() == 0 || linkageName.getLength() == 0)
        return false;

    *linkageNameOut = TerminatedCharSlice(linkageName.begin(), linkageName.getLength());
    *prettyNameOut = TerminatedCharSlice(prettyName.begin(), prettyName.getLength());
    return true;
}

static bool isPtrVolatile(IRInst* value)
{
    if (auto memoryQualifier = value->findDecoration<IRMemoryQualifierSetDecoration>())
    {
        if (memoryQualifier->getMemoryQualifierBit() & MemoryQualifierSetModifier::Flags::kVolatile)
        {
            return true;
        }
    }
    return false;
}

// Tries to determine a debug location for a given inst. E.g. if given a struct,
// it'll first try to check if it has explicit debug location information, and
// if not, it'll give the module's beginning at least.
static void findDebugLocation(
    const Dictionary<IRInst*, LLVMDebugNode*>& sourceDebugInfo,
    IRInst* inst,
    LLVMDebugNode*& file,
    unsigned& line)
{
    if (!inst || inst->getOp() == kIROp_ModuleInst)
    {
        file = nullptr;
        line = 0;
    }
    else if (auto debugLocation = inst->findDecoration<IRDebugLocationDecoration>())
    {
        file = sourceDebugInfo.getValue(debugLocation->getSource());
        line = getIntVal(debugLocation->getLine());
    }
    else
        findDebugLocation(sourceDebugInfo, inst->getParent(), file, line);
}

static TerminatedCharSlice getStringLitAsSlice(IRInst* inst)
{
    auto source = as<IRStringLit>(inst)->getStringSlice();
    return TerminatedCharSlice(source.begin(), source.getLength());
}


/*
static llvm::Instruction::CastOps getLLVMIntExtensionOp(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_BoolType:
    case kIROp_UInt16Type:
    case kIROp_UIntType:
    case kIROp_UInt64Type:
    case kIROp_UIntPtrType:
    case kIROp_PtrType:
        // Zero-extend unsigned types
        return llvm::Instruction::CastOps::ZExt;
    default:
        // Sign-extend the rest
        return llvm::Instruction::CastOps::SExt;
    }
}
*/

// This class helps with converting types from Slang IR to LLVM IR. It can
// create types for two different contexts:
//
// * getType(): creates types which appear in SSA values and are not observable
//   through memory.
//
// * getDebugType(): "pretty" types with correct offset annotations so that
//   debuggers can show the contents of variables.
//
// One design choice here is that all memory-related LLVM instructions (e.g.
// getelementptr and alloca) are emitted with byte arrays (with proper alignment
// annotations) instead of their actual types. There are two reasons for this:
//
// 1. LLVM is undergoing a transition to a more type-agnostic form:
//    https://www.npopov.com/2025/01/05/This-year-in-LLVM-2024.html#ptradd
//    In short, there is no longer any reason to specify the real type to
//    getelementptr; LLVM already canonicalizes it to use `i8` and byte offsets
//    anyway, and the type-based getelementptr itself may be removed in a
//    future version.
//
// 2. This allows us to entirely ignore LLVM's ideas of how structs are laid
//    out in memory. We can fully control the memory layout of our own structs
//    with no limitations from LLVM, which is great, because we already have
//    IRTypeLayoutRules.
//
// Furthermore, struct and array types are only passed around as pointers to
// memory in stack. That's because using aggregate types in values is
// "officially" discouraged:
// https://llvm.org/docs/Frontend/PerformanceTips.html#avoid-creating-values-of-aggregate-type
//
// With these design choices, only global struct/array constants need to be
// given an actual aggregate type in LLVM. So they build the type on-demand in
// `emitConstant`.
class LLVMTypeTranslator
{
private:
    ILLVMBuilder* builder;
    CompilerOptionSet* compilerOptions;
    const Dictionary<IRInst*, LLVMDebugNode*>* sourceDebugInfo;
    IRTypeLayoutRules* defaultPointerRules;

    Dictionary<IRType*, LLVMType*> valueTypeMap;
    Dictionary<IRTypeLayoutRules*, Dictionary<IRType*, LLVMDebugNode*>> debugTypeMap;
    Dictionary<IRType*, IRType*> legalizedResourceTypeMap;

    struct ConstantInfo
    {
        // Includes trailing padding
        LLVMInst* padded = nullptr;

        // Excludes trailing padding
        LLVMInst* unpadded = nullptr;

        LLVMInst* get(bool withTrailingPadding)
        {
            return withTrailingPadding ? padded : unpadded;
        }

        ConstantInfo& operator=(LLVMInst* constant)
        {
            padded = unpadded = constant;
            return *this;
        }
    };
    Dictionary<IRInst*, ConstantInfo> constantMap;

public:
    LLVMTypeTranslator(
        ILLVMBuilder* builder,
        CompilerOptionSet& compilerOptions,
        const Dictionary<IRInst*, LLVMDebugNode*>& sourceDebugInfo,
        IRTypeLayoutRules* rules)
        : builder(builder)
        , compilerOptions(&compilerOptions)
        , sourceDebugInfo(&sourceDebugInfo)
        , defaultPointerRules(rules)
    {
    }

    int getTypeBits(IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_BoolType:
            return 1;
        case kIROp_Int8Type:
        case kIROp_UInt8Type:
            return 8;
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
        case kIROp_HalfType:
            return 16;
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_FloatType:
            return 32;
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
        case kIROp_DoubleType:
            return 64;

        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
        case kIROp_PtrType:
            return builder->getPointerSizeInBits();

        default:
            SLANG_ASSERT_FAILURE("Unexpected type in getTypeBits!");
            return 0;
        }
    }

    // Returns the type you must use for passing around SSA values in LLVM IR.
    LLVMType* getType(IRType* type)
    {
        if (valueTypeMap.containsKey(type))
            return valueTypeMap.getValue(type);

        LLVMType* llvmType = nullptr;

        switch (type->getOp())
        {
        case kIROp_VoidType:
            llvmType = builder->getVoidType();
            break;

        case kIROp_HalfType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
            llvmType = builder->getFloatType(getTypeBits(type));

        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_UInt8Type:
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
            llvmType = builder->getIntType(getTypeBits(type));
            break;

        case kIROp_RateQualifiedType:
            llvmType = getType(as<IRRateQualifiedType>(type)->getValueType());
            break;

        case kIROp_PtrType:
        case kIROp_NativePtrType:
        case kIROp_StringType:
        case kIROp_NativeStringType:
        case kIROp_RawPointerType:
        case kIROp_OutParamType:
        case kIROp_RefParamType:
        case kIROp_BorrowInOutParamType:
        case kIROp_BorrowInParamType:
        case kIROp_ArrayType: // Arrays are passed as pointers in SSA values
        case kIROp_UnsizedArrayType:
        case kIROp_StructType: // Structs are passed as pointers in SSA values
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            // LLVM only has opaque pointers now, so everything that lowers as
            // a pointer is just that same opaque pointer.
            llvmType = builder->getPointerType();
            break;

        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                LLVMType* elemType = getType(vecType->getElementType());
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvmType = builder->getVectorType(elemCount, elemType);
            }
            break;
        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(type);

                List<LLVMType*> paramTypes;
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    paramTypes.add(getType(paramType));
                }

                auto resultType = funcType->getResultType();
                auto llvmReturnType = getType(resultType);

                // If attempting to return an aggregate, turn it into an extra
                // output parameter that is passed with a pointer.
                if (isAggregateType(resultType))
                {
                    paramTypes.add(builder->getPointerType());
                    llvmReturnType = builder->getVoidType();
                }

                llvmType = builder->getFunctionType(
                    llvmReturnType,
                    Slice(paramTypes.begin(), paramTypes.getCount())
                );
            }
            break;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
            llvmType = builder->getBufferType();
            break;
        // TODO: Textures, samplers, atomics, TLASes, etc.
        default:
            // Matrices and CoopVectors & CoopMatrices are already lowered into
            // something else before this LLVM emitter runs, and won't be
            // present.
            SLANG_UNEXPECTED("Unsupported type for LLVM target!");
            break;
        }

        valueTypeMap[type] = llvmType;
        return llvmType;
    }

    LLVMDebugNode* getDebugType(IRType* type, IRTypeLayoutRules* rules)
    {
        {
            auto& types = debugTypeMap[rules];
            if (types.containsKey(type))
                return types.getValue(type);
        }

        auto legalizedType = legalizeResourceTypes(type);

        LLVMDebugNode* llvmType = nullptr;
        switch (legalizedType->getOp())
        {
        case kIROp_VoidType:
            llvmType = builder->getDebugVoidType();
            break;
        case kIROp_HalfType:
            llvmType = builder->getDebugFloatType("half", 16);
            break;
        case kIROp_FloatType:
            llvmType = builder->getDebugFloatType("float", 32);
            break;
        case kIROp_DoubleType:
            llvmType = builder->getDebugFloatType("double", 64);
            break;
        case kIROp_BoolType:
            llvmType = builder->getDebugIntType("bool", false, 1);
            break;
        case kIROp_Int8Type:
            llvmType = builder->getDebugIntType("int8_t", true, 8);
            break;
        case kIROp_UInt8Type:
            llvmType = builder->getDebugIntType("uint8_t", false, 8);
            break;
        case kIROp_Int16Type:
            llvmType = builder->getDebugIntType("int16_t", true, 16);
            break;
        case kIROp_UInt16Type:
            llvmType = builder->getDebugIntType("uint16_t", false, 16);
            break;
        case kIROp_IntType:
            llvmType = builder->getDebugIntType("int", true, 32);
            break;
        case kIROp_UIntType:
            llvmType = builder->getDebugIntType("uint", false, 32);
            break;
        case kIROp_Int64Type:
            llvmType = builder->getDebugIntType("int64_t", true, 64);
            break;
        case kIROp_UInt64Type:
            llvmType = builder->getDebugIntType("uint64_t", false, 64);
            break;
        case kIROp_IntPtrType:
            llvmType = builder->getDebugIntType(
                "intptr_t", true, builder->getPointerSizeInBits());
            break;
        case kIROp_UIntPtrType:
            llvmType = builder->getDebugIntType(
                "uintptr_t", false, builder->getPointerSizeInBits());
            break;

        case kIROp_PtrType:
            {
                auto ptr = as<IRPtrType>(legalizedType);
                llvmType = builder->getDebugPointerType(
                    getDebugType(ptr->getValueType(), rules));
            }
            break;

        case kIROp_NativePtrType:
        case kIROp_RawPointerType:
            llvmType = builder->getDebugPointerType(nullptr);
            break;

        case kIROp_StringType:
        case kIROp_NativeStringType:
            llvmType = builder->getDebugStringType();
            break;

        case kIROp_OutParamType:
        case kIROp_RefParamType:
        case kIROp_BorrowInOutParamType:
        case kIROp_BorrowInParamType:
            {
                auto ptr = as<IRPtrTypeBase>(legalizedType);
                llvmType = builder->getDebugReferenceType(getDebugType(ptr->getValueType(), rules));
            }
            break;

        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(legalizedType);
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                LLVMDebugNode* elemType = getDebugType(vecType->getElementType(), rules);
                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(vecType, rules);
                llvmType = builder->getDebugVectorType(
                    sizeAndAlignment.size, sizeAndAlignment.alignment, elemCount, elemType);
            }
            break;

        case kIROp_UnsizedArrayType:
        case kIROp_ArrayType:
            {
                auto arrayType = static_cast<IRArrayTypeBase*>(legalizedType);
                LLVMDebugNode* elemType = getDebugType(arrayType->getElementType(), rules);
                auto irElemCount = arrayType->getElementCount();
                auto elemCount = irElemCount ? getIntVal(irElemCount) : 0;

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(arrayType, rules);
                sizeAndAlignment.size = align(sizeAndAlignment.size, sizeAndAlignment.alignment);

                llvmType = builder->getDebugArrayType(
                    sizeAndAlignment.size,
                    sizeAndAlignment.alignment,
                    elemCount,
                    elemType);
            }
            break;

        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(legalizedType);

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(structType, rules);

                LLVMDebugNode* file;
                unsigned line;
                findDebugLocation(*sourceDebugInfo, structType, file, line);

                List<LLVMDebugNode*> types;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;
                    LLVMDebugNode* debugType = getDebugType(fieldType, rules);

                    IRSizeAndAlignment sizeAndAlignment =
                        getSizeAndAlignment(field->getFieldType(), rules);
                    IRIntegerValue offset = getOffset(field, rules);

                    IRStructKey* key = field->getKey();
                    TerminatedCharSlice linkageName, prettyName;
                    maybeGetName(&linkageName, &prettyName, key);

                    types.add(builder->getDebugStructField(
                        debugType,
                        prettyName,
                        offset,
                        sizeAndAlignment.size,
                        sizeAndAlignment.alignment,
                        file,
                        line));
                }
                TerminatedCharSlice linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, legalizedType);
                sizeAndAlignment.size = align(sizeAndAlignment.size, sizeAndAlignment.alignment);

                llvmType = builder->getDebugStructType(
                    Slice(types.begin(), types.getCount()),
                    prettyName,
                    sizeAndAlignment.size,
                    sizeAndAlignment.alignment,
                    file, 
                    line
                );
            }
            break;

        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(legalizedType);

                List<LLVMDebugNode*> params;
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    params.add(getDebugType(paramType, rules));
                }

                llvmType = builder->getDebugFunctionType(
                    getDebugType(funcType->getResultType(), rules),
                    Slice(params.begin(), params.getCount())
                );
            }
            break;

        case kIROp_RateQualifiedType:
            llvmType = getDebugType(as<IRRateQualifiedType>(legalizedType)->getValueType(), rules);
            break;

        default:
            {
                TerminatedCharSlice linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, legalizedType);
                llvmType = builder->getDebugFallbackType(prettyName);
            }
            break;
        }

        {
            auto& types = debugTypeMap[rules];
            types[type] = llvmType;
        }
        return llvmType;
    }

    // Swaps resource buffers in Slang IR to their legalized struct
    // counterparts.
    IRType* legalizeResourceTypes(IRType* type)
    {
        if (legalizedResourceTypeMap.containsKey(type))
            return legalizedResourceTypeMap.getValue(type);

        IRBuilder builder(type->getModule());

        IRType* legalizedType = type;
        switch (type->getOp())
        {
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            legalizedType = builder.getRawPointerType();
            break;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
            {
                IRStructType* s = builder.createStructType();
                auto ptrKey = builder.createStructKey();
                auto sizeKey = builder.createStructKey();
                builder.createStructField(s, ptrKey, builder.getRawPointerType());
                builder.createStructField(s, sizeKey, builder.getType(kIROp_UIntPtrType));
                legalizedType = s;
            }
            break;
        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);
                bool illegal = false;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    auto legalizedFieldType = legalizeResourceTypes(fieldType);
                    if (legalizedFieldType != fieldType)
                        illegal = true;
                }

                if (illegal)
                {
                    IRStructType* s = builder.createStructType();
                    for (auto field : structType->getFields())
                    {
                        auto fieldType = field->getFieldType();
                        auto legalizedFieldType = legalizeResourceTypes(fieldType);
                        builder.createStructField(s, field->getKey(), legalizedFieldType);
                    }
                    legalizedType = s;
                }
            }
            break;
        case kIROp_ArrayType:
            {
                auto arrayType = as<IRArrayTypeBase>(type);
                auto elemType = arrayType->getElementType();
                auto legalizedElemType = legalizeResourceTypes(elemType);
                if (elemType != legalizedElemType)
                    legalizedType =
                        builder.getArrayType(legalizedElemType, arrayType->getElementCount());
            }
            break;
        default:
            break;
        }

        legalizedResourceTypeMap[type] = legalizedType;
        return legalizedType;
    }

    // Use this instead of the regular Slang::getOffset(), it handles
    // legalized resource types correctly.
    IRIntegerValue getOffset(IRStructField* field, IRTypeLayoutRules* rules)
    {
        IRIntegerValue offset = 0;
        auto structType = as<IRStructType>(field->getParent());
        auto legalStructType = as<IRStructType>(legalizeResourceTypes(structType));
        auto legalField = field;
        if (legalStructType != structType)
        {
            for (auto ff : legalStructType->getFields())
            {
                if (ff->getKey() == field->getKey())
                {
                    legalField = ff;
                    break;
                }
            }
        }
        Slang::getOffset(*compilerOptions, rules, legalField, &offset);
        return offset;
    }

    // Use this instead of the regular Slang::getSizeAndAlignment(), it handles
    // legalized resource types correctly.
    IRSizeAndAlignment getSizeAndAlignment(IRType* type, IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAlignment;

        Slang::getSizeAndAlignment(
            *compilerOptions,
            rules,
            legalizeResourceTypes(type),
            &sizeAlignment);

        return sizeAlignment;
    }

    // Allocates stack memory for given type. Returns a pointer to the start of
    // that memory.
    LLVMInst* emitAlloca(IRType* type, IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAlignment = getSizeAndAlignment(type, rules);
        return builder->emitAlloca(sizeAlignment.getStride(), sizeAlignment.alignment);
    }

    // llvmVal must be using `getType(valType)`. It will be stored in
    // llvmPtr. Returns the store instruction.
    LLVMInst* emitStore(
        LLVMInst* llvmPtr,
        LLVMInst* llvmVal,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false)
    {
        IRSizeAndAlignment sizeAlignment = getSizeAndAlignment(valType, rules);
        switch (valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto storageType = builder->getIntType(sizeAlignment.size * 8);
                auto expanded = builder->emitIntResize(llvmVal, storageType);
                return builder->emitStore(
                    expanded, llvmPtr, sizeAlignment.alignment, isVolatile);
            }
            break;
        case kIROp_ArrayType:
        case kIROp_StructType:
            // Arrays and struct values are always represented with an alloca
            // pointer.
            if (rules == defaultPointerRules)
            {
                // Equal memory layout, so we can just memcpy.
                return builder->emitCopy(
                    llvmPtr,
                    sizeAlignment.alignment,
                    llvmVal,
                    sizeAlignment.alignment,
                    sizeAlignment.size,
                    isVolatile);
            }
            else
            {
                return crossLayoutMemCpy(
                    llvmPtr,
                    llvmVal,
                    valType,
                    rules,
                    defaultPointerRules,
                    isVolatile);
            }
        default:
            return builder->emitStore(
                llvmVal, llvmPtr, sizeAlignment.alignment, isVolatile);
        }
    }

    // Returns the loaded data using `getType(valType)` from llvmPtr.
    // Returns the load instruction (= loaded value)
    LLVMInst* emitLoad(
        LLVMInst* llvmPtr,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false)
    {
        IRSizeAndAlignment sizeAlignment = getSizeAndAlignment(valType, rules);

        switch (valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto llvmType = getType(valType);
                auto storageType = builder->getIntType(sizeAlignment.size * 8);
                auto storageBool = builder->emitLoad(
                    storageType,
                    llvmPtr,
                    sizeAlignment.alignment,
                    isVolatile);
                return builder->emitIntResize(storageBool, llvmType);
            }
            break;
        case kIROp_ArrayType:
        case kIROp_StructType:
            if (rules == defaultPointerRules)
            {
                // Equal memory layout, so we can just memcpy.
                LLVMInst* llvmVar = emitAlloca(valType, defaultPointerRules);

                // Pointer-to-pointer copy, so generate inline memcpy.
                builder->emitCopy(
                    llvmVar,
                    sizeAlignment.alignment,
                    llvmPtr,
                    sizeAlignment.alignment,
                    sizeAlignment.size,
                    isVolatile);
                return llvmVar;
            }
            else
            {
                LLVMInst* llvmVar = emitAlloca(valType, defaultPointerRules);
                crossLayoutMemCpy(
                    llvmVar,
                    llvmPtr,
                    valType,
                    defaultPointerRules,
                    rules,
                    isVolatile);
                return llvmVar;
            }
        default:
            {
                auto llvmType = getType(valType);
                return builder->emitLoad(
                    llvmType,
                    llvmPtr,
                    sizeAlignment.alignment,
                    isVolatile);
            }
        }
    }

    // Computes an offset with `indexInst` based on `llvmPtr`. `indexInst` need
    // not be constant.
    LLVMInst* emitArrayGetElementPtr(
        LLVMInst* llvmPtr,
        LLVMInst* indexInst,
        IRType* elemType,
        IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(elemType, rules);
        return builder->emitGetElementPtr(
            llvmPtr,
            sizeAndAlignment.getStride(),
            indexInst);
    }

    // Computes a pointer to a struct field.
    LLVMInst* emitStructGetElementPtr(
        LLVMInst* llvmPtr,
        IRStructField* field,
        IRTypeLayoutRules* rules)
    {
        IRIntegerValue offset = getOffset(field, rules);
        return builder->emitGetElementPtr(llvmPtr, 1, builder->getConstantInt(builder->getIntType(32), offset));
    }

    // Emits zero padding as much as needed to make curSize == targetSize.
    UInt emitPaddingConstant(List<LLVMInst*>& fields, UInt& curSize, UInt targetSize)
    {
        SLANG_ASSERT(curSize <= targetSize);

        if (curSize < targetSize)
        {
            UInt bytes = targetSize - curSize;
            curSize = targetSize;

            auto zero = builder->getConstantInt(builder->getIntType(8), 0);
            auto zeros = List<LLVMInst*>::makeRepeated(zero, bytes);
            fields.add(builder->getConstantArray(Slice(zeros.begin(), zeros.getCount())));

            return bytes;
        }
        return 0;
    }

    // Tries to emit the given value as a constant, but may return nullptr if
    // that is not possible.
    LLVMInst* maybeEmitConstant(
        IRInst* inst,
        bool inAggregate = false,
        bool withTrailingPadding = true)
    {
        if (constantMap.containsKey(inst) && !inAggregate)
            return constantMap.getValue(inst).get(withTrailingPadding);

        ConstantInfo llvmConstant;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                    llvmConstant = builder->getConstantInt(getType(type), litInst->value.intVal);
                break;
            }

        case kIROp_FloatLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                    llvmConstant = builder->getConstantFloat(getType(type), litInst->value.floatVal);
            }
            break;

        case kIROp_StringLit:
            llvmConstant = builder->getConstantString(getStringLitAsSlice(inst));
            break;

        case kIROp_PtrLit:
            {
                auto ptrLit = static_cast<IRPtrLit*>(inst);
                llvmConstant = builder->getConstantPtr((uintptr_t)ptrLit->getValue());
            }
            break;
        case kIROp_MakeVector:
            {
                List<LLVMInst*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto partInst = inst->getOperand(aa);
                    auto constVal = maybeEmitConstant(partInst, inAggregate);
                    if (!constVal)
                        return nullptr;
                    if (auto subvectorType = as<IRVectorType>(partInst->getDataType()))
                    {
                        auto elemCount = getIntVal(subvectorType->getElementCount());
                        for (IRIntegerValue j = 0; j < elemCount; ++j)
                            values.add(builder->getConstantExtractElement(constVal, j));
                    }
                    else values.add(constVal);
                }

                auto vectorType = as<IRVectorType>(inst->getDataType());

                // So apparently, at least autodiff can generate MakeVectors
                // which actually just make a float. Interesting.
                if (!vectorType)
                {
                    llvmConstant = values[0];
                    break;
                }

                if (inAggregate)
                {
                    // To remove padding and alignment requirements from the
                    // vector, lower it as an array.
                    auto elemType = getType(vectorType->getElementType());
                    llvmConstant = builder->getConstantArray(Slice(values.begin(), values.getCount()));

                    int alignedCount = getVectorAlignedCount(vectorType, defaultPointerRules);
                    if (alignedCount != values.getCount())
                    {
                        // Fill padding with poison, nobody should use it in
                        // computations.
                        while (values.getCount() < alignedCount)
                            values.add(builder->getPoison(elemType));
                        llvmConstant.padded = builder->getConstantArray(Slice(values.begin(), values.getCount()));
                    }
                }
                else
                {
                    llvmConstant = builder->getConstantVector(Slice(values.begin(), values.getCount()));
                }
            }
            break;
        case kIROp_MakeVectorFromScalar:
            {
                auto vectorType = cast<IRVectorType>(inst->getDataType());
                int elemCount = getIntVal(vectorType->getElementCount());
                LLVMInst* value = maybeEmitConstant(inst->getOperand(0), inAggregate);
                if (!value)
                    return nullptr;

                if (inAggregate)
                {
                    auto elemType = getType(vectorType->getElementType());
                    auto values = List<LLVMInst*>::makeRepeated(value, elemCount);
                    llvmConstant = builder->getConstantArray(Slice(values.begin(), values.getCount()));

                    int alignedCount = getVectorAlignedCount(vectorType, defaultPointerRules);
                    if (alignedCount != values.getCount())
                    {
                        while (values.getCount() < alignedCount)
                            values.add(builder->getPoison(elemType));

                        llvmConstant.padded = builder->getConstantArray(Slice(values.begin(), values.getCount()));
                    }
                }
                else
                {
                    llvmConstant = builder->getConstantVector(value, elemCount);
                }
            }
            break;
        case kIROp_MakeArray:
            {
                auto arrayType = cast<IRArrayTypeBase>(inst->getDataType());
                List<LLVMInst*> values;

                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa), true, true);
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }

                llvmConstant = builder->getConstantArray(Slice(values.begin(), values.getCount()));

                if (needsUnpaddedArrayTypeWorkaround(arrayType, defaultPointerRules))
                {
                    auto lastPart = maybeEmitConstant(
                        inst->getOperand(inst->getOperandCount() - 1),
                        true,
                        false);

                    auto initPart = builder->getConstantArray(Slice(values.begin(), values.getCount()-1));

                    // This should be a struct for the workaround.
                    LLVMInst* fields[2] = {initPart, lastPart};
                    llvmConstant.unpadded = builder->getConstantStruct(Slice(fields, 2));
                }
            }
            break;
        case kIROp_MakeStruct:
            {
                auto structType = cast<IRStructType>(inst->getDataType());

                UInt llvmSize = 0;
                IRSizeAndAlignment sizeAndAlignment =
                    getSizeAndAlignment(structType, defaultPointerRules);

                List<LLVMInst*> values;
                auto field = structType->getFields().begin();
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa, ++field)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa), true, false);
                    if (!constVal)
                        return nullptr;

                    IRIntegerValue offset = getOffset(*field, defaultPointerRules);
                    // Insert padding until we're at the requested offset.
                    emitPaddingConstant(values, llvmSize, offset);

                    values.add(constVal);
                    llvmSize += builder->getStoreSizeOf(constVal);
                }

                llvmConstant.unpadded = builder->getConstantStruct(Slice<LLVMInst*>(values.begin(), values.getCount()));

                emitPaddingConstant(values, llvmSize, sizeAndAlignment.getStride());

                llvmConstant.padded = builder->getConstantStruct(Slice<LLVMInst*>(values.begin(), values.getCount()));
            }
            break;
        default:
            break;
        }

        if ((llvmConstant.padded || llvmConstant.unpadded) && !inAggregate)
            constantMap[inst] = llvmConstant;
        return llvmConstant.get(withTrailingPadding);
    }

    // Returns true for any type that translates into an aggregate type in LLVM.
    bool isAggregateType(IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_StructType:
            return true;
        default:
            return false;
        }
    }

private:
    // Copies data from a pointer to another, taking layout differences into
    // account.
    LLVMInst* crossLayoutMemCpy(
        LLVMInst* dstPtr,
        LLVMInst* srcPtr,
        IRType* type,
        IRTypeLayoutRules* dstLayout,
        IRTypeLayoutRules* srcLayout,
        bool isVolatile)
    {
        IRSizeAndAlignment dstSizeAlignment = getSizeAndAlignment(type, dstLayout);
        IRSizeAndAlignment srcSizeAlignment = getSizeAndAlignment(type, srcLayout);

        int minSize = std::min(dstSizeAlignment.size, srcSizeAlignment.size);

        switch (type->getOp())
        {
        case kIROp_ArrayType:
            {
                auto arrayType = as<IRArrayTypeBase>(type);
                auto elemType = arrayType->getElementType();
                auto irElemCount = arrayType->getElementCount();
                auto elemCount = getIntVal(irElemCount);

                LLVMInst* last = nullptr;
                for (int elem = 0; elem < elemCount; ++elem)
                {
                    auto dstElemPtr = emitArrayGetElementPtr(
                        dstPtr,
                        builder->getConstantInt(builder->getIntType(32), elem),
                        elemType,
                        dstLayout);
                    auto srcElemPtr = emitArrayGetElementPtr(
                        srcPtr,
                        builder->getConstantInt(builder->getIntType(32), elem),
                        elemType,
                        srcLayout);
                    last = crossLayoutMemCpy(
                        dstElemPtr,
                        srcElemPtr,
                        elemType,
                        dstLayout,
                        srcLayout,
                        isVolatile);
                }
                return last;
            }
            break;
        case kIROp_StructType:
            {
                auto structType = as<IRStructType>(type);

                LLVMInst* last = nullptr;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;

                    auto dstElemPtr = emitStructGetElementPtr(dstPtr, field, dstLayout);
                    auto srcElemPtr = emitStructGetElementPtr(srcPtr, field, srcLayout);

                    last = crossLayoutMemCpy(
                        dstElemPtr,
                        srcElemPtr,
                        fieldType,
                        dstLayout,
                        srcLayout,
                        isVolatile);
                }
                return last;
            }
            break;
        default:
            // Assume that there are no substantial layout differences (aside
            // from trailing padding, which is skipped via minSize), so copying
            // is safe.
            return builder->emitCopy(
                dstPtr,
                dstSizeAlignment.alignment,
                srcPtr,
                srcSizeAlignment.alignment,
                minSize,
                isVolatile);
        }
        return nullptr;
    }

    UInt getVectorAlignedCount(IRVectorType* vecType, IRTypeLayoutRules* rules)
    {
        int elemCount = getIntVal(vecType->getElementCount());
        IRSizeAndAlignment elementAlignment;
        Slang::getSizeAndAlignment(
            *compilerOptions,
            rules,
            vecType->getElementType(),
            &elementAlignment);
        IRSizeAndAlignment vectorAlignment =
            rules->getVectorSizeAndAlignment(elementAlignment, elemCount);

        return align(vectorAlignment.size, vectorAlignment.alignment) / elementAlignment.size;
    }

    // True for arrays that have trailing padding. Those arrays are split into
    // a struct:
    //
    // struct
    // {
    //     paddedElemType init[count-1];
    //     unpaddedElemType last;
    // };
    bool needsUnpaddedArrayTypeWorkaround(IRArrayTypeBase* arrayType, IRTypeLayoutRules* rules)
    {
        auto irElemCount = arrayType->getElementCount();
        auto elemCount = int(getIntVal(irElemCount));
        IRSizeAndAlignment sizeAndAlignment =
            getSizeAndAlignment(arrayType->getElementType(), rules);

        bool hasTrailingPadding =
            sizeAndAlignment.size != align(sizeAndAlignment.size, sizeAndAlignment.alignment);
        return elemCount != 0 && hasTrailingPadding;
    }
};

struct LLVMEmitter
{
    CodeGenContext* codeGenContext;

    // If true, we should emit LLVM debug info. If false, all debug-related
    // members are uninitialized and unused.
    bool debug = false;

    ComPtr<ILLVMBuilder> builder;
    std::unique_ptr<LLVMTypeTranslator> types;

    // Changes the behaviour when encountering a global instruction. If false,
    // they're emitted as global variables. Otherwise, we'll try to inline them.
    bool inlineGlobalInstructions = false;

    // These layout rules are used for all data types when they're stored in
    // memory, EXCEPT when contained within a buffer with an explicit type layout
    // parameter.
    IRTypeLayoutRules* defaultPointerRules = nullptr;

    // The LLVM value class is closest to Slang's IRInst, as it can represent
    // constants, instructions and functions, whereas llvm::Instruction only
    // handles instructions.
    Dictionary<IRInst*, LLVMInst*> mapInstToLLVM;

    // Global instructions that have been promoted into global variables. That
    // happens when they get referenced. Later on, constructors for these must
    // be generated.
    Dictionary<IRInst*, LLVMInst*> promotedGlobalInsts;

    // Map of DebugSource instructions to LLVM debug files
    Dictionary<IRInst*, LLVMDebugNode*> sourceDebugInfo;
    Dictionary<IRInst*, LLVMDebugNode*> funcToDebugLLVM;

    // These enums represent built-in functions whose implementations must be
    // externally provided.
    enum class ExternalFunc
    {
        Printf
        // TODO: Texture sampling, RT functions?
    };
    // Map of external builtins. These are only forward-declared in the emitted
    // LLVM IR.
    Dictionary<ExternalFunc, LLVMInst*> externalFuncs;

    struct VariableDebugInfo
    {
        LLVMDebugNode* debugVar;
        // attached is true when the first related DebugValue has been
        // processed.
        bool attached = false;
        // isStackVar is true when the variable has been alloca'd and declared
        // as debugValue. Doesn't necessarily need further DebugValue tracking
        // after that, since a real variable now actually exists in LLVM.
        bool isStackVar = false;
    };
    Dictionary<IRInst*, VariableDebugInfo> variableDebugInfoMap;

    // Used to skip some instructions whose value is derived from DebugVar.
    HashSet<IRInst*> debugInsts;

    LLVMDebugNode* currentLocalScope = nullptr;
    bool debugInlinedScope = false;
    UInt uniqueIDCounter = 0;

    // Used to add code in in front of return in a function. If it returns
    // nullptr, the LLVM return instruction is generated "normally". Otherwise,
    // it's expected that you generated it in this function.
    //
    // This uses std::function instead of Slang::Func, because Slang::Func
    // requires RTTI which sadly is not available when linking to LLVM.
    using FuncEpilogueCallback = std::function<LLVMInst*(IRReturn*)>;

    LLVMEmitter(CodeGenContext* codeGenContext, bool useJIT = false)
        : codeGenContext(codeGenContext)
    {
        auto targetTripleOption =
            getOptions().getStringOption(CompilerOptionName::LLVMTargetTriple).getUnownedSlice();
        auto cpuOption =
            getOptions().getStringOption(CompilerOptionName::LLVMCPU).getUnownedSlice();
        if (cpuOption.getLength() == 0)
            cpuOption = UnownedStringSlice("generic");
        auto featOption =
            getOptions().getStringOption(CompilerOptionName::LLVMFeatures).getUnownedSlice();

        StringBuilder sb;
        getOptions().writeCommandLineArgs(codeGenContext->getSession(), sb);
        auto params = sb.toString();

        LLVMBuilderOptions builderOpt;
        builderOpt.target = asExternal(codeGenContext->getTargetFormat());
        builderOpt.targetTriple = TerminatedCharSlice(targetTripleOption.begin(), targetTripleOption.getLength());
        builderOpt.cpu = TerminatedCharSlice(cpuOption.begin(), cpuOption.getLength());
        builderOpt.features = TerminatedCharSlice(featOption.begin(), featOption.getLength());
        builderOpt.debugCommandLineArgs = TerminatedCharSlice(params.begin(), params.getLength());

        builderOpt.debugLevel = (SlangDebugInfoLevel)getOptions().getDebugInfoLevel();
        builderOpt.optLevel = (SlangOptimizationLevel)getOptions().getOptimizationLevel();
        builderOpt.fp32DenormalMode = (SlangFpDenormalMode)getOptions().getDenormalModeFp32();
        builderOpt.fp64DenormalMode = (SlangFpDenormalMode)getOptions().getDenormalModeFp64();
        builderOpt.fpMode = (SlangFloatingPointMode)getOptions().getFloatingPointMode();

        // TODO: Create ILLVMBuilder

        debug = getOptions().getDebugInfoLevel() != DebugInfoLevel::None;

        if (getOptions().shouldUseCLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::C);
        else if (getOptions().shouldUseScalarLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::Scalar);
        else if (getOptions().shouldUseDXLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::D3DConstantBuffer);
        else
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::LLVM);

        types.reset(new LLVMTypeTranslator(
            builder,
            getOptions(),
            sourceDebugInfo,
            defaultPointerRules));
    }

    DiagnosticSink* getSink() { return codeGenContext->getSink(); }

    CompilerOptionSet& getOptions() { return codeGenContext->getTargetProgram()->getOptionSet(); }

    /*
    llvm::Function* getExternalBuiltin(ExternalFunc extFunc)
    {
        if (externalFuncs.containsKey(extFunc))
            return externalFuncs.getValue(extFunc);

        llvm::Function* func = nullptr;
        auto emitDecl = [&](const char* name,
                            llvm::Type* result,
                            llvm::ArrayRef<llvm::Type*> params,
                            bool isVarArgs)
        {
            llvm::FunctionType* funcType = llvm::FunctionType::get(result, params, isVarArgs);
            return llvm::Function::Create(
                funcType,
                llvm::GlobalValue::ExternalLinkage,
                name,
                *llvmModule);
        };

        switch (extFunc)
        {
        case ExternalFunc::Printf:
            func = emitDecl("printf", llvmBuilder->getInt32Ty(), {llvmBuilder->getPtrTy()}, true);
            llvm::AttrBuilder attrs(*llvmContext);
            attrs.addCapturesAttr(llvm::CaptureInfo::none());
            attrs.addAttribute(llvm::Attribute::NoAlias);
            func->getArg(0)->addAttrs(attrs);
            break;
        }

        externalFuncs[extFunc] = func;
        return func;
    }

    // Finds the value of an instruction that has already been emitted, OR
    // creates the value if it's a constant. Also, if inlineGlobalInstructions
    // is set, this will inline such instructions as needed.
    llvm::Value* findValue(IRInst* inst)
    {
        if (mapInstToLLVM.containsKey(inst))
            return mapInstToLLVM.getValue(inst);

        bool globalInstruction = inst->getParent()->getOp() == kIROp_ModuleInst;

        llvm::Value* llvmValue = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_PtrLit:
        case kIROp_MakeVector:
        case kIROp_MakeVectorFromScalar:
            // kIROp_MakeArray & kIROp_MakeStruct are intentionally omitted
            // here; they must not be emitted as values in any other context
            // than global vars. `emitGlobalVarDecl` and
            // `emitGlobalInstructionCtor` handle that part.
            llvmValue = types->maybeEmitConstant(inst);
            break;
        case kIROp_Specialize:
            {
                auto s = as<IRSpecialize>(inst);
                auto g = s->getBase();
                auto e = "Specialize instruction remains in IR for LLVM emit, is something "
                         "undefined?\n" +
                         dumpIRToString(g);
                SLANG_UNEXPECTED(e.getBuffer());
            }
        default:
            if (globalInstruction && inlineGlobalInstructions)
            {
                return emitLLVMInstruction(inst);
            }
            else if (globalInstruction && !inlineGlobalInstructions)
            {
                // This is a global instruction getting referenced. So, we generate
                // a global variable for it (if we don't have one yet) and report
                // this incident.
                llvm::GlobalVariable* llvmVar = nullptr;
                auto type = inst->getDataType();
                if (promotedGlobalInsts.containsKey(inst))
                {
                    llvmVar = promotedGlobalInsts.getValue(inst);
                }
                else
                {
                    IRSizeAndAlignment sizeAndAlignment =
                        types->getSizeAndAlignment(type, defaultPointerRules);
                    auto llvmType = llvm::ArrayType::get(
                        llvmBuilder->getInt8Ty(),
                        sizeAndAlignment.getStride());

                    if (auto constVal = types->maybeEmitConstant(inst))
                    {
                        llvmVar = new llvm::GlobalVariable(
                            constVal->getType(),
                            false,
                            llvm::GlobalValue::PrivateLinkage);
                        llvmVar->setInitializer(constVal);
                    }
                    else
                    {
                        llvmVar = new llvm::GlobalVariable(
                            llvmType,
                            false,
                            llvm::GlobalValue::PrivateLinkage);
                        llvmVar->setAlignment(llvm::Align(sizeAndAlignment.alignment));
                    }
                    llvmModule->insertGlobalVariable(llvmVar);
                    promotedGlobalInsts[inst] = llvmVar;
                }

                // Aggregates are passed around as pointers.
                if (types->isAggregateType(type))
                    return llvmVar;

                // Otherwise, we'll need a load instruction to get the value.
                auto llvmValueType = types->getType(type);
                return llvmBuilder->CreateLoad(llvmValueType, llvmVar, false);
            }
            else
            {
                SLANG_UNEXPECTED("Unsupported value type for LLVM target, or referring to an "
                                 "instruction that hasn't been emitted yet!");
            }
            break;
        }

        SLANG_ASSERT(llvmValue);

        if (llvmValue)
            mapInstToLLVM[inst] = llvmValue;
        return llvmValue;
    }

    // Coerces the given value to the given type. Because LLVM IR does not carry
    // signedness, the information on whether the original type of 'val' is
    // signed is passed separately. This only affects integer extension.
    llvm::Value* coerceNumeric(llvm::Value* val, llvm::Type* type, bool isSigned)
    {
        auto valType = val->getType();
        auto valWidth = valType->getPrimitiveSizeInBits();
        auto targetWidth = type->getPrimitiveSizeInBits();

        if (type->isVectorTy() && !valType->isVectorTy())
        {
            auto vecType = llvm::cast<llvm::VectorType>(type);
            // Splat scalar into vector
            return llvmBuilder->CreateVectorSplat(
                vecType->getElementCount(),
                coerceNumeric(val, vecType->getElementType(), isSigned));
        }
        else if (type->getScalarType()->isFloatingPointTy())
        {
            // Extend or truncate float
            if (valWidth < targetWidth)
            {
                return llvmBuilder->CreateFPExt(val, type);
            }
            else if (valWidth > targetWidth)
            {
                return llvmBuilder->CreateFPTrunc(val, type);
            }
        }
        else if (type->getScalarType()->isIntegerTy())
        {
            // Extend or truncate int
            if (valWidth != targetWidth)
                return isSigned ? llvmBuilder->CreateSExtOrTrunc(val, type)
                                : llvmBuilder->CreateZExtOrTrunc(val, type);
        }

        return val;
    }

    // Some operations in Slang IR may have mixed scalar and vector parameters,
    // whereas LLVM IR requires only scalars or only vectors. This function
    // helps you promote each type as required.
    void promoteParams(
        llvm::Value*& aValInOut,
        bool aIsSigned,
        llvm::Value*& bValInOut,
        bool bIsSigned)
    {
        llvm::Type* aType = aValInOut->getType();
        llvm::Type* bType = bValInOut->getType();
        if (aType == bType)
            return;

        llvm::Type* aElementType = aType->getScalarType();
        llvm::Type* bElementType = bType->getScalarType();
        int aElementCount = 1;
        int bElementCount = 1;
        if (aType->isVectorTy())
        {
            auto vecType = llvm::cast<llvm::VectorType>(aType);
            aElementCount = vecType->getElementCount().getFixedValue();
        }
        if (bType->isVectorTy())
        {
            auto vecType = llvm::cast<llvm::VectorType>(bType);
            bElementCount = vecType->getElementCount().getFixedValue();
        }

        int aElementWidth = aElementType->getScalarSizeInBits();
        int bElementWidth = bElementType->getScalarSizeInBits();
        bool aFloat = aElementType->isFloatingPointTy();
        bool bFloat = bElementType->isFloatingPointTy();
        int maxElementCount = aElementCount > bElementCount ? aElementCount : bElementCount;

        llvm::Type* promotedType = aType;
        bool promotedSign = aIsSigned || bIsSigned;

        if (!aFloat && bFloat)
            promotedType = aElementType;
        else if (aFloat && !bFloat)
            promotedType = bElementType;
        else
            promotedType = aElementWidth > bElementWidth ? aElementType : bElementType;

        if (maxElementCount != 1)
            promotedType =
                llvm::VectorType::get(promotedType, llvm::ElementCount::getFixed(maxElementCount));

        aValInOut = coerceNumeric(aValInOut, promotedType, promotedSign);
        bValInOut = coerceNumeric(bValInOut, promotedType, promotedSign);
    }

    llvm::Value* emitCompare(IRInst* inst)
    {
        SLANG_ASSERT(inst->getOperandCount() == 2);

        IRType* aElementType = getVectorOrCoopMatrixElementType(inst->getOperand(0)->getDataType());
        IRType* bElementType = getVectorOrCoopMatrixElementType(inst->getOperand(1)->getDataType());
        bool aSigned = isSignedType(aElementType);
        bool bSigned = isSignedType(bElementType);

        auto a = findValue(inst->getOperand(0));
        auto b = findValue(inst->getOperand(1));
        promoteParams(a, aSigned, b, bSigned);

        bool isFloat = a->getType()->getScalarType()->isFloatingPointTy();
        bool isSigned = aSigned || bSigned;

        llvm::CmpInst::Predicate pred;
        switch (inst->getOp())
        {
        case kIROp_Less:
            pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OLT
                   : isSigned ? llvm::CmpInst::Predicate::ICMP_SLT
                              : llvm::CmpInst::Predicate::ICMP_ULT;
            break;
        case kIROp_Leq:
            pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OLE
                   : isSigned ? llvm::CmpInst::Predicate::ICMP_SLE
                              : llvm::CmpInst::Predicate::ICMP_ULE;
            break;
        case kIROp_Eql:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OEQ : llvm::CmpInst::Predicate::ICMP_EQ;
            break;
        case kIROp_Neq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_ONE : llvm::CmpInst::Predicate::ICMP_NE;
            break;
        case kIROp_Greater:
            pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OGT
                   : isSigned ? llvm::CmpInst::Predicate::ICMP_SGT
                              : llvm::CmpInst::Predicate::ICMP_UGT;
            break;
        case kIROp_Geq:
            pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OGE
                   : isSigned ? llvm::CmpInst::Predicate::ICMP_SGE
                              : llvm::CmpInst::Predicate::ICMP_UGE;
            break;
        default:
            SLANG_UNEXPECTED("Unsupported compare op");
            break;
        }

        return llvmBuilder->CreateCmp(pred, a, b);
    }

    llvm::Value* emitArithmetic(IRInst* inst)
    {
        auto resultType = types->getType(inst->getDataType());
        bool isFloat = resultType->getScalarType()->isFloatingPointTy();
        bool isSigned = isSignedType(inst->getDataType());

        if (inst->getOperandCount() == 1)
        {
            auto llvmValue = findValue(inst->getOperand(0));
            switch (inst->getOp())
            {
            case kIROp_Neg:
                return isFloat ? llvmBuilder->CreateFNeg(llvmValue)
                               : llvmBuilder->CreateNeg(llvmValue);
            case kIROp_Not:
            case kIROp_BitNot:
                return llvmBuilder->CreateNot(llvmValue);
            default:
                SLANG_UNEXPECTED("Unsupported unary arithmetic op");
                break;
            }
        }
        else if (inst->getOperandCount() == 2)
        {
            llvm::Instruction::BinaryOps op;
            switch (inst->getOp())
            {
            case kIROp_Add:
                op = isFloat ? llvm::Instruction::FAdd : llvm::Instruction::Add;
                break;
            case kIROp_Sub:
                op = isFloat ? llvm::Instruction::FSub : llvm::Instruction::Sub;
                break;
            case kIROp_Mul:
                op = isFloat ? llvm::Instruction::FMul : llvm::Instruction::Mul;
                break;
            case kIROp_Div:
                op = isFloat    ? llvm::Instruction::FDiv
                     : isSigned ? llvm::Instruction::SDiv
                                : llvm::Instruction::UDiv;
                break;
            case kIROp_IRem:
                op = isSigned ? llvm::Instruction::SRem : llvm::Instruction::URem;
                break;
            case kIROp_FRem:
                op = llvm::Instruction::FRem;
                break;
            case kIROp_And:
            case kIROp_BitAnd:
                op = llvm::Instruction::And;
                break;
            case kIROp_Or:
            case kIROp_BitOr:
                op = llvm::Instruction::Or;
                break;
            case kIROp_BitXor:
                op = llvm::Instruction::Xor;
                break;
            case kIROp_Rsh:
                op = isSigned ? llvm::Instruction::AShr : llvm::Instruction::LShr;
                break;
            case kIROp_Lsh:
                op = llvm::Instruction::Shl;
                break;
            default:
                SLANG_UNEXPECTED("Unsupported binary arithmetic op");
                break;
            }

            // Some ops in Slang, e.g. Lsh, may have differing types for the
            // operands. This is not allowed by LLVM. Both sides must match
            // the result type. Hence, we coerce as needed.
            auto a = coerceNumeric(findValue(inst->getOperand(0)), resultType, isSigned);
            auto b = coerceNumeric(findValue(inst->getOperand(1)), resultType, isSigned);

            return llvmBuilder->CreateBinOp(op, a, b);
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected number of operands for arithmetic op");
        }

        return nullptr;
    }

    IRTypeLayoutRules* getBufferLayoutRules(IRType* bufferType)
    {
        return getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), bufferType);
    }

    // Tries to find which layout rules apply to the given pointer, based on
    // "provenance": we track the pointer to where we got it and check if the
    // source is a buffer with a specific layout.
    IRTypeLayoutRules* getPtrLayoutRules(IRInst* ptr)
    {
        // Check if the pointer is actually based on an buffer with an explicit
        // layout. If so, we need to take that layout into account.
        if (auto structuredBufferInst = as<IRRWStructuredBufferGetElementPtr>(ptr))
        {
            auto baseType = cast<IRHLSLStructuredBufferTypeBase>(
                structuredBufferInst->getBase()->getDataType());
            return getBufferLayoutRules(baseType);
        }
        else if (auto cbufType = as<IRConstantBufferType>(ptr->getDataType()))
        {
            return getBufferLayoutRules(cbufType);
        }
        else if (auto gep = as<IRGetElementPtr>(ptr))
        {
            // Transitive
            return getPtrLayoutRules(gep->getBase());
        }
        else if (auto off = as<IRGetOffsetPtr>(ptr))
        {
            // Transitive
            return getPtrLayoutRules(off->getBase());
        }
        else if (auto fieldAddr = as<IRFieldAddress>(ptr))
        {
            // Transitive
            return getPtrLayoutRules(fieldAddr->getBase());
        }
        return defaultPointerRules;
    }

    static llvm::Value* _defaultOnReturnHandler(IRReturn*)
    {
        SLANG_ASSERT_FAILURE("Unexpected terminator in global scope!");
        return nullptr;
    }

    // Caution! This is only for emitting things which are considered
    // instructions in LLVM! It won't work for IRBlocks, IRFuncs & such.
    llvm::Value* emitLLVMInstruction(
        IRInst* inst,
        FuncEpilogueCallback onReturn = _defaultOnReturnHandler)
    {
        llvm::Value* llvmInst = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_PtrLit:
            llvmInst = types->maybeEmitConstant(inst);
            break;

        case kIROp_Less:
        case kIROp_Leq:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Geq:
            llvmInst = emitCompare(inst);
            break;

        case kIROp_Specialize:
        case kIROp_MissingReturn:
        case kIROp_StaticAssert:
        case kIROp_Unmodified:
            return nullptr;

        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_IRem:
        case kIROp_FRem:
        case kIROp_Neg:
        case kIROp_Not:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitNot:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Rsh:
        case kIROp_Lsh:
            llvmInst = emitArithmetic(inst);
            break;

        case kIROp_Return:
            {
                IRReturn* retInst = static_cast<IRReturn*>(inst);
                llvmInst = onReturn(retInst);

                // If onReturnCallback didn't generate the return instruction
                // for us, we have to do it here.
                if (!llvmInst)
                {
                    auto retVal = retInst->getVal();
                    if (retVal->getOp() == kIROp_VoidLit)
                    {
                        llvmInst = llvmBuilder->CreateRetVoid();
                    }
                    else
                    {
                        llvmInst = llvmBuilder->CreateRet(findValue(retVal));
                    }
                }
            }
            break;

        case kIROp_Var:
            {
                auto var = static_cast<IRVar*>(inst);
                auto ptrType = var->getDataType();

                llvm::Value* llvmVar =
                    types->emitAlloca(ptrType->getValueType(), defaultPointerRules);

                llvm::StringRef linkageName, prettyName;
                if (maybeGetName(&linkageName, &prettyName, inst))
                {
                    llvmVar->setName(linkageName);

                    // TODO: This is a bit of a hack. DebugVar fails to get
                    // linked to the actual Var sometimes, which is why we
                    // automatically create a debug var for each Slang IR var if
                    // it has a name.:/

                    llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
                    if (debug && loc)
                    {
                        auto varType =
                            types->getDebugType(ptrType->getValueType(), defaultPointerRules);

                        auto debugVar = llvmDebugBuilder->createAutoVariable(
                            currentLocalScope,
                            prettyName,
                            loc->getFile(),
                            loc->getLine(),
                            varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None);
                        llvmDebugBuilder->insertDeclare(
                            llvmVar,
                            llvm::cast<llvm::DILocalVariable>(debugVar),
                            llvmDebugBuilder->createExpression(),
                            loc,
                            llvmBuilder->GetInsertBlock());
                    }
                }

                llvmInst = llvmVar;
            }
            break;

        case kIROp_Loop:
        case kIROp_UnconditionalBranch:
            {
                auto branch = as<IRUnconditionalBranch>(inst);
                auto llvmTarget = llvm::cast<llvm::BasicBlock>(findValue(branch->getTargetBlock()));
                llvmInst = llvmBuilder->CreateBr(llvmTarget);
            }
            break;

        case kIROp_IfElse:
            {
                auto ifelseInst = static_cast<IRIfElse*>(inst);
                auto trueBlock =
                    llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getTrueBlock()));
                auto falseBlock =
                    llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getFalseBlock()));
                auto cond = findValue(ifelseInst->getCondition());
                llvmInst = llvmBuilder->CreateCondBr(cond, trueBlock, falseBlock);
            }
            break;

        case kIROp_Switch:
            {
                auto switchInst = static_cast<IRSwitch*>(inst);
                auto defaultBlock =
                    llvm::cast<llvm::BasicBlock>(findValue(switchInst->getBreakLabel()));
                auto llvmCondition = findValue(switchInst->getCondition());

                auto defaultLabel = switchInst->getDefaultLabel();
                if (defaultLabel)
                    defaultBlock = llvm::cast<llvm::BasicBlock>(findValue(defaultLabel));

                auto llvmSwitch = llvmBuilder->CreateSwitch(
                    llvmCondition,
                    defaultBlock,
                    switchInst->getCaseCount());
                for (UInt c = 0; c < switchInst->getCaseCount(); c++)
                {
                    auto value = switchInst->getCaseValue(c);
                    auto intLit = as<IRIntLit>(value);
                    SLANG_ASSERT(intLit);

                    auto llvmCaseBlock =
                        llvm::cast<llvm::BasicBlock>(findValue(switchInst->getCaseLabel(c)));
                    auto llvmCaseValue =
                        llvm::cast<llvm::ConstantInt>(types->maybeEmitConstant(intLit));

                    llvmSwitch->addCase(llvmCaseValue, llvmCaseBlock);
                }
                llvmInst = llvmSwitch;
            }
            break;

        case kIROp_Select:
            {
                auto selectInst = static_cast<IRSelect*>(inst);

                llvmInst = llvmBuilder->CreateSelect(
                    findValue(selectInst->getCondition()),
                    findValue(selectInst->getTrueResult()),
                    findValue(selectInst->getFalseResult()));
            }
            break;

        case kIROp_Store:
            {
                auto storeInst = static_cast<IRStore*>(inst);
                auto ptr = storeInst->getPtr();
                auto val = storeInst->getVal();

                llvmInst = types->emitStore(
                    findValue(ptr),
                    findValue(val),
                    val->getDataType(),
                    getPtrLayoutRules(ptr),
                    isPtrVolatile(ptr));
            }
            break;

        case kIROp_Load:
            {
                auto loadInst = static_cast<IRLoad*>(inst);
                auto ptr = loadInst->getPtr();

                llvmInst = types->emitLoad(
                    findValue(ptr),
                    loadInst->getDataType(),
                    getPtrLayoutRules(ptr),
                    isPtrVolatile(ptr));
            }
            break;

        case kIROp_LoadFromUninitializedMemory:
        case kIROp_Poison:
            {
                auto type = inst->getDataType();
                if (types->isAggregateType(type))
                {
                    // Aggregates are always stack-allocated.
                    llvmInst = types->emitAlloca(type, defaultPointerRules);
                }
                else
                {
                    llvmInst = llvm::PoisonValue::get(types->getType(type));
                }
            }
            break;

        case kIROp_MakeUInt64:
            {
                auto lowbits = findValue(inst->getOperand(0));
                auto highbits = findValue(inst->getOperand(1));

                auto i64Type = llvmBuilder->getInt64Ty();

                lowbits = llvmBuilder->CreateZExt(lowbits, i64Type);
                highbits = llvmBuilder->CreateZExt(highbits, i64Type);
                highbits = llvmBuilder->CreateShl(highbits, llvmBuilder->getInt64(32));
                llvmInst = llvmBuilder->CreateOr(lowbits, highbits);
            }
            break;

        case kIROp_MakeArray:
            llvmInst = types->emitAlloca(inst->getDataType(), defaultPointerRules);
            for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
            {
                auto op = inst->getOperand(aa);
                llvm::Value* ptr = types->emitArrayGetElementPtr(
                    llvmInst,
                    llvmBuilder->getInt32(aa),
                    op->getDataType(),
                    defaultPointerRules);
                types->emitStore(ptr, findValue(op), op->getDataType(), defaultPointerRules);
            }
            break;

        case kIROp_MakeStruct:
            {
                IRStructType* type = as<IRStructType>(inst->getDataType());
                llvmInst = types->emitAlloca(type, defaultPointerRules);
                auto field = type->getFields().begin();
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa, ++field)
                {
                    llvm::Value* ptr =
                        types->emitStructGetElementPtr(llvmInst, *field, defaultPointerRules);
                    auto op = inst->getOperand(aa);
                    types->emitStore(ptr, findValue(op), op->getDataType(), defaultPointerRules);
                }
            }
            break;

        case kIROp_MakeArrayFromElement:
            {
                auto arrayType = cast<IRArrayType>(inst->getDataType());
                auto elementCount = getIntVal(arrayType->getElementCount());
                llvmInst = types->emitAlloca(inst->getDataType(), defaultPointerRules);
                auto element = inst->getOperand(0);
                auto llvmElement = findValue(element);
                for (IRIntegerValue i = 0; i < elementCount; ++i)
                {
                    llvm::Value* ptr = types->emitArrayGetElementPtr(
                        llvmInst,
                        llvmBuilder->getInt32(i),
                        element->getDataType(),
                        defaultPointerRules);
                    types->emitStore(ptr, llvmElement, element->getDataType(), defaultPointerRules);
                }
            }
            break;

        case kIROp_MakeVector:
            llvmInst = types->maybeEmitConstant(inst);
            if (!llvmInst)
            {
                auto llvmType = types->getType(inst->getDataType());

                // MakeVector of a scalar is a scalar.
                if (!as<IRVectorType>(inst->getDataType()))
                {
                    llvmInst = findValue(inst->getOperand(0));
                    break;
                }

                llvmInst = llvm::PoisonValue::get(llvmType);
                UInt elemIndex = 0;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto val = findValue(inst->getOperand(aa));
                    auto valType = val->getType();
                    if (valType->isVectorTy())
                    {
                        auto vecType = llvm::cast<llvm::FixedVectorType>(valType);
                        auto elemCount = vecType->getNumElements();
                        for (UInt j = 0; j < elemCount; ++j)
                        {
                            auto entry = llvmBuilder->CreateExtractElement(val, j);
                            llvmInst = llvmBuilder->CreateInsertElement(llvmInst, entry, elemIndex);
                            elemIndex++;
                        }
                    }
                    else
                    {
                        llvmInst = llvmBuilder->CreateInsertElement(llvmInst, val, elemIndex);
                        elemIndex++;
                    }
                }
            }
            break;

        case kIROp_MakeVectorFromScalar:
            llvmInst = types->maybeEmitConstant(inst);
            if (!llvmInst)
            {
                auto llvmType = llvm::cast<llvm::VectorType>(types->getType(inst->getDataType()));
                auto val = findValue(inst->getOperand(0));
                llvmInst = llvmBuilder->CreateVectorSplat(llvmType->getElementCount(), val);
            }
            break;

        case kIROp_Swizzle:
            {
                auto swizzleInst = static_cast<IRSwizzle*>(inst);
                auto baseInst = swizzleInst->getBase();
                if (swizzleInst->getElementCount() == 1)
                {
                    llvmInst = llvmBuilder->CreateExtractElement(
                        findValue(baseInst),
                        findValue(swizzleInst->getElementIndex(0)));
                }
                else
                {
                    List<int> mask;
                    mask.reserve(swizzleInst->getElementCount());
                    for (UInt i = 0; i < swizzleInst->getElementCount(); ++i)
                    {
                        int val = as<IRIntLit>(swizzleInst->getElementIndex(i))->getValue();
                        mask.add(val);
                    }
                    llvmInst = llvmBuilder->CreateShuffleVector(
                        findValue(baseInst),
                        llvm::ArrayRef<int>(mask.begin(), mask.end()));
                }
            }
            break;

        case kIROp_SwizzleSet:
            {
                auto swizzledInst = static_cast<IRSwizzleSet*>(inst);

                llvmInst = findValue(swizzledInst->getBase());
                auto llvmSrc = findValue(swizzledInst->getSource());

                for (UInt i = 0; i < swizzledInst->getElementCount(); ++i)
                {
                    IRInst* irElementIndex = swizzledInst->getElementIndex(i);
                    IRIntegerValue elementIndex = getIntVal(irElementIndex);
                    auto llvmSrcElement = llvmBuilder->CreateExtractElement(llvmSrc, i);
                    llvmInst =
                        llvmBuilder->CreateInsertElement(llvmInst, llvmSrcElement, elementIndex);
                }
            }
            break;

        case kIROp_SwizzledStore:
            {
                auto swizzledInst = static_cast<IRSwizzledStore*>(inst);

                auto dst = swizzledInst->getDest();
                auto src = swizzledInst->getSource();
                auto llvmDst = findValue(dst);
                auto llvmSrc = findValue(src);

                auto dstType = as<IRPtrTypeBase>(dst->getDataType())->getValueType();

                IRTypeLayoutRules* rules = getPtrLayoutRules(dst);

                IRType* elementType = as<IRVectorType>(dstType)->getElementType();

                for (UInt i = 0; i < swizzledInst->getElementCount(); ++i)
                {
                    IRInst* irElementIndex = swizzledInst->getElementIndex(i);
                    IRIntegerValue elementIndex = getIntVal(irElementIndex);

                    auto llvmDstElement = types->emitArrayGetElementPtr(
                        llvmDst,
                        llvmBuilder->getInt32(elementIndex),
                        elementType,
                        rules);
                    auto llvmSrcElement = llvmBuilder->CreateExtractElement(llvmSrc, i);
                    llvmInst = types->emitStore(llvmDstElement, llvmSrcElement, elementType, rules);
                }
            }
            break;

        case kIROp_IntCast:
            {
                auto llvmValue = findValue(inst->getOperand(0));

                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();
                auto fromType = getVectorOrCoopMatrixElementType(fromTypeV);
                auto toType = getVectorOrCoopMatrixElementType(toTypeV);

                auto llvmFromType = types->getType(fromTypeV);
                auto llvmToType = types->getType(toTypeV);
                auto fromWidth = llvmFromType->getScalarSizeInBits();
                auto toWidth = llvmToType->getScalarSizeInBits();

                if (fromWidth == toWidth)
                {
                    // LLVM integers are sign-ambiguous, so if the width is the
                    // same, there's nothing to do.
                    llvmInst = llvmValue;
                }
                else if (as<IRBoolType>(toType))
                {
                    llvm::Constant* zero = llvm::ConstantInt::get(types->getType(fromType), 0);
                    if (toTypeV != toType)
                    { // Vector
                        zero = llvm::ConstantVector::getSplat(
                            llvm::cast<llvm::VectorType>(llvmToType)->getElementCount(),
                            zero);
                    }
                    llvmInst =
                        llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_NE, llvmValue, zero);
                }
                else
                {
                    llvm::Instruction::CastOps cast = llvm::Instruction::CastOps::Trunc;
                    if (toWidth > fromWidth)
                        cast = getLLVMIntExtensionOp(fromType);
                    llvmInst = llvmBuilder->CreateCast(cast, llvmValue, types->getType(toTypeV));
                }
            }
            break;

        case kIROp_FloatCast:
            {
                auto llvmValue = findValue(inst->getOperand(0));

                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();

                auto llvmFromType = types->getType(fromTypeV);
                auto llvmToType = types->getType(toTypeV);

                auto fromSize = llvmFromType->getScalarSizeInBits();
                auto toSize = llvmToType->getScalarSizeInBits();

                if (fromSize == toSize)
                {
                    llvmInst = llvmValue;
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        fromSize < toSize ? llvm::Instruction::CastOps::FPExt
                                          : llvm::Instruction::CastOps::FPTrunc,
                        llvmValue,
                        llvmToType);
                }
            }
            break;

        case kIROp_CastIntToFloat:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    isSignedType(fromTypeV) ? llvm::Instruction::CastOps::SIToFP
                                            : llvm::Instruction::CastOps::UIToFP,
                    findValue(inst->getOperand(0)),
                    types->getType(toTypeV));
            }
            break;

        case kIROp_CastFloatToInt:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto fromType = getVectorOrCoopMatrixElementType(fromTypeV);
                auto toTypeV = inst->getDataType();
                auto toType = getVectorOrCoopMatrixElementType(toTypeV);
                auto llvmValue = findValue(inst->getOperand(0));

                if (as<IRBoolType>(toType))
                {
                    llvmInst = llvmBuilder->CreateCmp(
                        llvm::CmpInst::Predicate::FCMP_UNE,
                        llvmValue,
                        llvm::ConstantFP::getZero(types->getType(fromType)));
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        isSignedType(toTypeV) ? llvm::Instruction::CastOps::FPToSI
                                              : llvm::Instruction::CastOps::FPToUI,
                        llvmValue,
                        types->getType(toTypeV));
                }
            }
            break;

        case kIROp_CastPtrToInt:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    llvm::Instruction::CastOps::PtrToInt,
                    findValue(fromValue),
                    types->getType(toTypeV));
            }
            break;

        case kIROp_CastPtrToBool:
            {
                auto fromValue = inst->getOperand(0);
                llvmInst = llvmBuilder->CreateIsNotNull(findValue(fromValue));
            }
            break;

        case kIROp_CastIntToPtr:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    llvm::Instruction::CastOps::IntToPtr,
                    findValue(fromValue),
                    types->getType(toTypeV));
            }
            break;

        case kIROp_PtrCast:
            {
                // ptr-to-ptr casts are no-ops due to opaque pointers.
                llvmInst = findValue(inst->getOperand(0));
            }
            break;

        case kIROp_BitCast:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();

                auto llvmFromValue = findValue(fromValue);
                auto llvmFromType = llvmFromValue->getType();
                auto llvmToType = types->getType(toTypeV);

                auto op = llvm::Instruction::CastOps::BitCast;
                // It appears that sometimes casts between ints and ptrs occur
                // as bitcasts. Fix the operation in that case.
                if (llvmFromType->isPointerTy() && llvmToType->isIntegerTy())
                    op = llvm::Instruction::CastOps::PtrToInt;
                else if (llvmFromType->isIntegerTy() && llvmToType->isPointerTy())
                    op = llvm::Instruction::CastOps::IntToPtr;
                else if (llvmFromType->isPointerTy() && !llvmToType->isPointerTy())
                {
                    // Cast from pointer to ???, so first cast to int and then
                    // perform the bitcast.
                    llvmFromValue = llvmBuilder->CreateCast(
                        llvm::Instruction::CastOps::PtrToInt,
                        llvmFromValue,
                        llvmBuilder->getIntPtrTy(targetDataLayout));
                }
                else if (!llvmFromType->isPointerTy() && llvmToType->isPointerTy())
                {
                    // Cast from ??? to pointer, so first bitcast to equally
                    // sized int type and then do IntToPtr cast.
                    llvmFromValue = llvmBuilder->CreateCast(
                        llvm::Instruction::CastOps::BitCast,
                        llvmFromValue,
                        llvmBuilder->getIntPtrTy(targetDataLayout));
                    op = llvm::Instruction::CastOps::IntToPtr;
                }

                llvmInst = llvmBuilder->CreateCast(op, llvmFromValue, llvmToType);
            }
            break;

        case kIROp_FieldAddress:
            {
                auto fieldAddressInst = static_cast<IRFieldAddress*>(inst);
                auto base = fieldAddressInst->getBase();

                if (debugInsts.contains(base))
                {
                    debugInsts.add(inst);
                    // This is emitted to annotate member accesses of structs,
                    // but we don't need that because our structs are
                    // stack-allocated (in LLVM IR's mind) and already declared
                    // as variables.
                    return nullptr;
                }

                auto key = as<IRStructKey>(fieldAddressInst->getField());

                IRStructType* baseStructType = nullptr;
                if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
                {
                    baseStructType = as<IRStructType>(ptrLikeType->getElementType());
                }
                else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
                {
                    baseStructType = as<IRStructType>(ptrType->getValueType());
                }

                auto rules = getPtrLayoutRules(base);
                auto field = findStructField(baseStructType, key);
                auto llvmBase = findValue(base);

                llvmInst = types->emitStructGetElementPtr(llvmBase, field, rules);
            }
            break;

        case kIROp_FieldExtract:
            {
                auto fieldExtractInst = static_cast<IRFieldExtract*>(inst);
                auto base = fieldExtractInst->getBase();
                auto structType = as<IRStructType>(base->getDataType());

                if (debugInsts.contains(base))
                {
                    debugInsts.add(inst);
                    return nullptr;
                }

                auto rules = getPtrLayoutRules(base);

                auto llvmBase = findValue(base);
                auto key = as<IRStructKey>(fieldExtractInst->getField());
                auto field = findStructField(structType, key);

                llvm::Value* ptr = types->emitStructGetElementPtr(llvmBase, field, rules);

                llvmInst = types->emitLoad(ptr, field->getFieldType(), rules);
            }
            break;

        case kIROp_GetOffsetPtr:
        case kIROp_GetElementPtr:
            {
                auto baseInst = inst->getOperand(0);
                auto indexInst = inst->getOperand(1);

                if (debugInsts.contains(baseInst))
                {
                    debugInsts.add(inst);
                    return nullptr;
                }

                IRType* baseType = nullptr;
                if (auto ptrType = as<IRPtrTypeBase>(baseInst->getDataType()))
                {
                    baseType = ptrType->getValueType();
                }
                else if (auto ptrType = as<IRPointerLikeType>(baseInst->getDataType()))
                {
                    baseType = as<IRType>(ptrType->getOperand(0));
                }
                else
                    SLANG_ASSERT_FAILURE("Unknown pointer type for GetElementPtr!");

                // I _REALLY_ dislike that this helper function needs an
                // IRBuilder :/
                IRBuilder builder(inst->getModule());
                IRType* elemType = getElementType(builder, baseType);

                llvmInst = types->emitArrayGetElementPtr(
                    findValue(baseInst),
                    findValue(indexInst),
                    elemType,
                    getPtrLayoutRules(baseInst));
            }
            break;

        case kIROp_GetElement:
            {
                auto geInst = static_cast<IRGetElement*>(inst);
                auto baseInst = geInst->getBase();
                auto indexInst = geInst->getIndex();

                if (debugInsts.contains(baseInst))
                {
                    debugInsts.add(inst);
                    return nullptr;
                }

                auto llvmVal = findValue(baseInst);

                auto baseTy = baseInst->getDataType();
                if (as<IRVectorType>(baseTy))
                {
                    // For vectors, we can use extractelement
                    llvmInst = llvmBuilder->CreateExtractElement(llvmVal, findValue(indexInst));
                }
                else if (auto arrayType = as<IRArrayTypeBase>(baseTy))
                {
                    // emitGEP + emitLoad.
                    SLANG_ASSERT(llvmVal->getType()->isPointerTy());
                    auto rules = getPtrLayoutRules(baseInst);
                    auto elemType = arrayType->getElementType();
                    llvm::Value* ptr = types->emitArrayGetElementPtr(
                        llvmVal,
                        findValue(indexInst),
                        elemType,
                        rules);
                    llvmInst = types->emitLoad(ptr, elemType, rules);
                }
                else
                    SLANG_ASSERT_FAILURE("Unknown data type for GetElement!");
            }
            break;

        case kIROp_BitfieldExtract:
            {
                auto type = types->getType(inst->getDataType());
                auto val = coerceNumeric(findValue(inst->getOperand(0)), type, false);
                auto off = coerceNumeric(findValue(inst->getOperand(1)), type, false);
                auto bts = coerceNumeric(findValue(inst->getOperand(2)), type, false);

                IRType* elementType =
                    getVectorOrCoopMatrixElementType(inst->getOperand(0)->getDataType());
                IRBasicType* basicType = as<IRBasicType>(elementType);
                bool isSigned = isSignedType(basicType);

                auto shiftedVal =
                    llvmBuilder->CreateLShr(val, coerceNumeric(off, val->getType(), false));

                auto numBits = coerceNumeric(
                    llvmBuilder->getInt32(type->getScalarSizeInBits()),
                    val->getType(),
                    false);
                auto highBits = llvmBuilder->CreateSub(numBits, bts);
                shiftedVal = llvmBuilder->CreateShl(shiftedVal, highBits);
                if (isSigned)
                    llvmInst = llvmBuilder->CreateAShr(shiftedVal, highBits);
                else
                    llvmInst = llvmBuilder->CreateLShr(shiftedVal, highBits);
            }
            break;

        case kIROp_BitfieldInsert:
            {
                auto type = types->getType(inst->getDataType());
                auto val = coerceNumeric(findValue(inst->getOperand(0)), type, false);
                auto insert = coerceNumeric(findValue(inst->getOperand(1)), type, false);
                auto off = coerceNumeric(findValue(inst->getOperand(2)), type, false);
                auto bts = coerceNumeric(findValue(inst->getOperand(3)), type, false);

                auto one = coerceNumeric(llvmBuilder->getInt32(1), val->getType(), false);
                auto mask = llvmBuilder->CreateShl(one, bts);
                mask = llvmBuilder->CreateSub(mask, one);
                mask = llvmBuilder->CreateShl(mask, off);

                insert = llvmBuilder->CreateShl(insert, off);
                insert = llvmBuilder->CreateAnd(insert, mask);

                auto notMask = llvmBuilder->CreateNot(mask);

                val = llvmBuilder->CreateAnd(val, notMask);
                llvmInst = llvmBuilder->CreateOr(val, insert);
            }
            break;

        case kIROp_Call:
            {
                auto callInst = static_cast<IRCall*>(inst);
                auto funcValue = callInst->getCallee();

                if (!mapInstToLLVM.containsKey(funcValue))
                {
                    // Function is missing - likely reason for this is that it's
                    // a __target_intrinsic that hasn't been emitted yet.
                    if (auto targetIntrinsic = Slang::findBestTargetIntrinsicDecoration(
                            funcValue,
                            codeGenContext->getTargetCaps()))
                    {
                        // 'irFunc' may have a template type, so we need to deduce
                        // the actual type here :/
                        auto func = createTargetIntrinsicFunc(callInst);
                        auto llvmFunc = ensureFuncDecl(func);
                        emitTargetIntrinsicFunction(
                            func,
                            llvmFunc,
                            targetIntrinsic,
                            targetIntrinsic->getDefinition());
                        mapInstToLLVM[funcValue] = llvmFunc;
                    }
                }

                auto llvmFuncInst = findValue(funcValue);
                auto llvmFunc = llvm::cast<llvm::Function>(llvmFuncInst);

                List<llvm::Value*> args;

                for (IRInst* arg : callInst->getArgsList())
                {
                    args.add(findValue(arg));
                }

                llvm::Value* allocValue = nullptr;
                // If attempting to return an aggregate, turn it into an extra
                // output parameter that is passed with a pointer.
                if (types->isAggregateType(inst->getDataType()))
                {
                    allocValue = types->emitAlloca(inst->getDataType(), defaultPointerRules);
                    args.add(allocValue);
                }
                auto returnVal =
                    llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
                llvmInst = allocValue ? allocValue : returnVal;
            }
            break;

        case kIROp_Printf:
            {
                auto llvmFunc = getExternalBuiltin(ExternalFunc::Printf);

                List<llvm::Value*> args;
                args.add(findValue(inst->getOperand(0)));
                if (inst->getOperandCount() == 2)
                {
                    auto operand = inst->getOperand(1);
                    if (auto makeStruct = as<IRMakeStruct>(operand))
                    {
                        // Flatten the tuple resulting from the variadic pack.
                        for (UInt bb = 0; bb < makeStruct->getOperandCount(); ++bb)
                        {
                            auto op = makeStruct->getOperand(bb);
                            auto llvmValue = findValue(op);
                            auto valueType = llvmValue->getType();

                            if (valueType->isFloatingPointTy() &&
                                valueType->getScalarSizeInBits() < 64)
                            {
                                // Floats need to get up-casted to at least f64
                                llvmValue = llvmBuilder->CreateCast(
                                    llvm::Instruction::CastOps::FPExt,
                                    llvmValue,
                                    llvmBuilder->getDoubleTy());
                            }
                            else if (
                                valueType->isIntegerTy() && valueType->getScalarSizeInBits() < 32)
                            {
                                // Ints are upcasted to at least i32.
                                llvmValue = llvmBuilder->CreateCast(
                                    getLLVMIntExtensionOp(op->getDataType()),
                                    llvmValue,
                                    llvmBuilder->getInt32Ty());
                            }
                            args.add(llvmValue);
                        }
                    }
                }

                llvmInst =
                    llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
            }
            break;

        case kIROp_RWStructuredBufferGetElementPtr:
            {
                auto gepInst = static_cast<IRRWStructuredBufferGetElementPtr*>(inst);

                auto baseType =
                    cast<IRHLSLRWStructuredBufferType>(gepInst->getBase()->getDataType());
                auto llvmBase = findValue(gepInst->getBase());
                auto llvmIndex = findValue(gepInst->getIndex());

                auto llvmPtr = llvmBuilder->CreateExtractValue(llvmBase, 0);

                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);
                llvmInst = types->emitArrayGetElementPtr(
                    llvmPtr,
                    llvmIndex,
                    baseType->getElementType(),
                    rules);
            }
            break;

        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferLoad:
            {
                auto base = inst->getOperand(0);
                auto llvmBase = findValue(base);
                auto llvmIndex = findValue(inst->getOperand(1));

                auto baseType = cast<IRHLSLStructuredBufferTypeBase>(base->getDataType());

                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);

                auto llvmPtr = types->emitArrayGetElementPtr(
                    llvmBasePtr,
                    llvmIndex,
                    baseType->getElementType(),
                    rules);
                llvmInst = types->emitLoad(llvmPtr, inst->getDataType(), rules);
            }
            break;

        case kIROp_RWStructuredBufferStore:
            {
                auto base = inst->getOperand(0);
                auto llvmBase = findValue(base);
                auto llvmIndex = findValue(inst->getOperand(1));
                auto val = inst->getOperand(2);

                auto baseType = cast<IRHLSLStructuredBufferTypeBase>(base->getDataType());

                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);

                auto llvmPtr = types->emitArrayGetElementPtr(
                    llvmBasePtr,
                    llvmIndex,
                    baseType->getElementType(),
                    rules);
                llvmInst = types->emitStore(llvmPtr, findValue(val), val->getDataType(), rules);
            }
            break;

        case kIROp_ByteAddressBufferLoad:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                auto llvmPtr =
                    llvmBuilder->CreateGEP(llvmBuilder->getInt8Ty(), llvmBasePtr, llvmIndex);

                llvmInst = types->emitLoad(llvmPtr, inst->getDataType(), defaultPointerRules);
            }
            break;

        case kIROp_ByteAddressBufferStore:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                llvm::Value* indices[] = {llvmIndex};
                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                auto llvmPtr =
                    llvmBuilder->CreateGEP(llvmBuilder->getInt8Ty(), llvmBasePtr, indices);
                auto val = inst->getOperand(inst->getOperandCount() - 1);
                llvmInst = types->emitStore(
                    llvmPtr,
                    findValue(val),
                    val->getDataType(),
                    defaultPointerRules);
            }
            break;

        case kIROp_StructuredBufferGetDimensions:
            {
                auto getDimensionsInst = cast<IRStructuredBufferGetDimensions>(inst);
                auto buffer = getDimensionsInst->getBuffer();
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(buffer->getDataType());
                auto llvmBuffer = findValue(buffer);

                IRTypeLayoutRules* layout = getBufferLayoutRules(bufferType);

                auto llvmUintType = llvmBuilder->getInt32Ty();
                auto llvmBaseCount = llvmBuilder->CreateExtractValue(llvmBuffer, 1);
                llvmBaseCount = llvmBuilder->CreateZExtOrTrunc(llvmBaseCount, llvmUintType);

                auto returnType =
                    llvm::VectorType::get(llvmUintType, llvm::ElementCount::getFixed(2));
                llvmInst = llvm::PoisonValue::get(returnType);
                llvmInst = llvmBuilder->CreateInsertElement(llvmInst, llvmBaseCount, uint64_t(0));

                auto stride = types->getSizeAndAlignment(bufferType->getElementType(), layout).size;
                llvmInst = llvmBuilder->CreateInsertElement(
                    llvmInst,
                    llvmBuilder->getInt32(stride),
                    uint64_t(1));
            }
            break;

        case kIROp_GetEquivalentStructuredBuffer:
            {
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(inst->getDataType());
                auto llvmByteBuffer = findValue(inst->getOperand(0));
                auto llvmByteCount = llvmBuilder->CreateExtractValue(llvmByteBuffer, 1);

                IRTypeLayoutRules* layout = getBufferLayoutRules(bufferType);
                auto stride = types->getSizeAndAlignment(bufferType->getElementType(), layout).size;
                auto llvmElementCount = llvmBuilder->CreateUDiv(
                    llvmByteCount,
                    llvm::ConstantInt::get(llvmBuilder->getIntPtrTy(targetDataLayout), stride));
                llvmInst = llvmBuilder->CreateInsertValue(llvmByteBuffer, llvmElementCount, 1);
            }
            break;

        case kIROp_Unreachable:
            return llvmBuilder->CreateUnreachable();

        case kIROp_GlobalValueRef:
            llvmInst = findValue(inst->getOperand(0));
            break;

        case kIROp_GetStringHash:
            {
                auto getStringHashInst = cast<IRGetStringHash>(inst);
                auto stringLit = getStringHashInst->getStringLit();

                if (stringLit)
                {
                    auto slice = stringLit->getStringSlice();
                    auto hash = getStableHashCode32(slice.begin(), slice.getLength()).hash;
                    llvmInst = llvmBuilder->getInt32(hash);
                }
                else
                {
                    // TODO: Probably best implemented in an improved prelude,
                    // slang-rt or in core module. Ideally, if built-in hashing
                    // support in the core module becomes a thing, that can be
                    // used for this too.
                    getSink()->diagnose(
                        inst,
                        Diagnostics::unimplemented,
                        "unexpected string hash for non-literal string");
                }
            }
            break;

        case kIROp_DebugVar:
            debugInsts.add(inst);
            if (debug && currentLocalScope)
            {
                auto debugVarInst = static_cast<IRDebugVar*>(inst);

                auto ptrType = as<IRPtrType>(debugVarInst->getDataType());
                auto varType = types->getDebugType(ptrType->getValueType(), defaultPointerRules);

                auto file = sourceDebugInfo.getValue(debugVarInst->getSource());
                auto line = getIntVal(debugVarInst->getLine());
                IRInst* argIndex = debugVarInst->getArgIndex();

                llvm::StringRef linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, inst);

                if (argIndex && !debugInlinedScope)
                {
                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder->createParameterVariable(
                            currentLocalScope,
                            prettyName,
                            getIntVal(argIndex) + 1,
                            file,
                            line,
                            varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None),
                        false,
                        false};
                }
                else
                {
                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder->createAutoVariable(
                            currentLocalScope,
                            prettyName,
                            file,
                            line,
                            varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None),
                        false,
                        false};
                }
            }
            return nullptr;

        case kIROp_DebugValue:
            debugInsts.add(inst);
            if (debug && currentLocalScope)
            {
                auto debugValueInst = static_cast<IRDebugValue*>(inst);
                auto debugVar = debugValueInst->getDebugVar();
                if (!variableDebugInfoMap.containsKey(debugVar))
                    return nullptr;

                VariableDebugInfo& debugInfo = variableDebugInfoMap.getValue(debugVar);

                llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
                if (!loc)
                    loc = llvm::DILocation::get(
                        *llvmContext,
                        debugInfo.debugVar->getLine(),
                        0,
                        debugInfo.debugVar->getScope());

                llvm::Value* value = findValue(debugValueInst->getValue());
                llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(value);
                if (!debugInfo.attached && alloca)
                {
                    debugInfo.isStackVar = true;
                    llvmDebugBuilder->insertDeclare(
                        alloca,
                        llvm::cast<llvm::DILocalVariable>(debugInfo.debugVar),
                        llvmDebugBuilder->createExpression(),
                        loc,
                        llvmBuilder->GetInsertBlock());
                }
                else if (!debugInfo.isStackVar)
                {
                    llvmDebugBuilder->insertDbgValueIntrinsic(
                        findValue(debugValueInst->getValue()),
                        llvm::cast<llvm::DILocalVariable>(debugInfo.debugVar),
                        llvmDebugBuilder->createExpression(),
                        loc,
                        llvmBuilder->GetInsertBlock());
                }
                debugInfo.attached = true;
            }
            return nullptr;

        case kIROp_DebugLine:
            debugInsts.add(inst);
            if (debug && currentLocalScope)
            {
                auto debugLineInst = static_cast<IRDebugLine*>(inst);

                // auto file = sourceDebugInfo.getValue(debugLineInst->getSource());
                auto line = getIntVal(debugLineInst->getLineStart());
                auto col = getIntVal(debugLineInst->getColStart());

                debugLineInst->getLineEnd();
                debugLineInst->getColStart();
                debugLineInst->getColEnd();

                llvmBuilder->SetCurrentDebugLocation(
                    llvm::DILocation::get(*llvmContext, line, col, currentLocalScope));
            }
            return nullptr;

        case kIROp_DebugScope:
            debugInsts.add(inst);
            if (debug)
            {
                // TODO: Couldn't figure out how to track scope of inlined
                // functions properly in LLVM, but we can simply avoid cases
                // where it would matter.
                //
                // LLVM loves to say:
                //     !dbg attachment points at wrong subprogram for function
                // if your scope points to something other than the surrounding
                // function, seemingly making it useless for inlined functions.

                debugInlinedScope = true;
            }
            return nullptr;

        case kIROp_DebugNoScope:
            debugInsts.add(inst);
            if (debug)
            {
                debugInlinedScope = false;
            }
            return nullptr;

        case kIROp_DebugInlinedAt:
        case kIROp_DebugInlinedVariable:
            debugInsts.add(inst);
            // TODO: Unhandled debug insts
            return nullptr;

        default:
            SLANG_UNEXPECTED("Unsupported instruction for LLVM target!");
            break;
        }

        SLANG_ASSERT(llvmInst);

        return llvmInst;
    }

    llvm::GlobalValue::LinkageTypes getLinkageType(IRInst* inst)
    {
        for (auto decor : inst->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_EntryPointDecoration:
            case kIROp_ExternCDecoration:
            case kIROp_ExternCppDecoration:
            case kIROp_DownstreamModuleExportDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_DllExportDecoration:
                {
                    return llvm::GlobalValue::ExternalLinkage;
                }
            default:
                break;
            }
        }
        return llvm::GlobalValue::PrivateLinkage;
    }

    llvm::DISubprogram* ensureDebugFunc(IRGlobalValueWithCode* func, IRDebugFunction* debugFunc)
    {
        if (funcToDebugLLVM.containsKey(func))
            return funcToDebugLLVM[func];

        IRType* funcType = as<IRType>(func->getDataType());
        llvm::DIFile* file = nullptr;
        int line = 0;
        if (debugFunc)
        {
            auto debugFuncType = as<IRType>(debugFunc->getDebugType());

            // TODO: Debug function types are in a bit of a poor state. Let's
            // only use them if they at least have the right type.
            if (as<IRFuncType>(debugFuncType))
                funcType = debugFuncType;

            file = sourceDebugInfo.getValue(debugFunc->getFile());
            line = getIntVal(debugFunc->getLine());
        }
        else
        {
            file = compileUnit->getFile();
        }

        llvm::StringRef linkageName, prettyName;

        maybeGetName(&linkageName, &prettyName, func);

        llvm::DIType* llvmFuncType = types->getDebugType(funcType, defaultPointerRules);
        auto sp = llvmDebugBuilder->createFunction(
            file,
            prettyName,
            linkageName,
            file,
            line,
            llvm::cast<llvm::DISubroutineType>(llvmFuncType),
            line,
            llvm::DINode::FlagPrototyped,
            llvm::DISubprogram::SPFlagDefinition);
        funcToDebugLLVM[func] = sp;
        return sp;
    }

    llvm::Function* ensureFuncDecl(IRFunc* func)
    {
        if (mapInstToLLVM.containsKey(func))
            return llvm::cast<llvm::Function>(mapInstToLLVM.getValue(func));

        auto funcType = static_cast<IRFuncType*>(func->getDataType());

        llvm::FunctionType* llvmFuncType = llvm::cast<llvm::FunctionType>(types->getType(funcType));

        llvm::Function* llvmFunc = llvm::Function::Create(
            llvmFuncType,
            getLinkageType(func),
            "", // Name is conditionally set below.
            *llvmModule);

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, func))
            llvmFunc->setName(linkageName);

        // If the name is missing for whatever reason, just generate one that
        // shouldn't clash with anything else.
        if (llvmFunc->getName().size() == 0)
            llvmFunc->setName("__slang_anonymous_func_" + std::to_string(uniqueIDCounter++));

        UInt i = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
        {
            auto llvmArg = llvmFunc->getArg(i);

            // Aliasing out and reference parameters are UB in Slang, and
            // telling this to LLVM should help with optimization.
            if (as<IROutParamType>(funcType->getParamType(i)))
            {
                llvmArg->addAttr(llvm::Attribute::WriteOnly);
                llvmArg->addAttr(llvm::Attribute::NoAlias);
            }
            else if (as<IRBorrowInParamType>(funcType->getParamType(i)))
            {
                llvmArg->addAttr(llvm::Attribute::ReadOnly);
                llvmArg->addAttr(llvm::Attribute::NoAlias);
            }
            else if (as<IRBorrowInOutParamType>(funcType->getParamType(i)))
            {
                llvmArg->addAttr(llvm::Attribute::NoAlias);
            }

            llvm::StringRef linkageName, prettyName;
            if (maybeGetName(&linkageName, &prettyName, pp))
                llvmArg->setName(linkageName);

            mapInstToLLVM[pp] = llvmArg;
        }

        // Attach attributes based on decorations!
        if (func->findDecoration<IRReadNoneDecoration>())
            llvmFunc->setOnlyAccessesArgMemory();
        else if (func->findDecoration<IRForceInlineDecoration>())
            llvmFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
        else if (func->findDecoration<IRNoInlineDecoration>())
            llvmFunc->addFnAttr(llvm::Attribute::AttrKind::NoInline);

        mapInstToLLVM[func] = llvmFunc;
        return llvmFunc;
    }

    // Declares the global variable in LLVM IR and sets an initializer for it
    // if it is trivial.
    llvm::Value* emitGlobalVarDecl(IRGlobalVar* var)
    {
        IRPtrType* ptrType = var->getDataType();

        llvm::GlobalVariable* llvmVar = nullptr;
        auto firstBlock = var->getFirstBlock();

        if (firstBlock)
        {
            auto returnInst = as<IRReturn>(firstBlock->getTerminator());
            if (returnInst)
            {
                // If the initializer is constant, we can emit that to the
                // variable directly.
                IRInst* val = returnInst->getVal();
                if (auto constantValue = types->maybeEmitConstant(val))
                {
                    // Easy case, it's just a constant in LLVM.
                    llvmVar = new llvm::GlobalVariable(
                        constantValue->getType(),
                        false,
                        getLinkageType(var));
                    llvmVar->setInitializer(constantValue);
                }
            }
        }

        IRSizeAndAlignment sizeAndAlignment =
            types->getSizeAndAlignment(ptrType->getValueType(), defaultPointerRules);

        if (!llvmVar)
        {
            // No initializer, let's just skip the type, no-one cares anyway :)
            llvmVar = new llvm::GlobalVariable(
                llvm::ArrayType::get(llvmBuilder->getInt8Ty(), sizeAndAlignment.getStride()),
                false,
                getLinkageType(var));
        }
        llvmVar->setAlignment(llvm::Align(sizeAndAlignment.alignment));

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, var))
            llvmVar->setName(linkageName);

        llvmModule->insertGlobalVariable(llvmVar);
        mapInstToLLVM[var] = llvmVar;
        return llvmVar;
    }

    void emitGlobalVarCtor(IRGlobalVar* var)
    {
        llvm::GlobalVariable* llvmVar =
            llvm::cast<llvm::GlobalVariable>(mapInstToLLVM.getValue(var));

        // If there's already an initializer, there's nothing more to do here.
        // This happens when the initializer is a constant and is set in
        // emitGlobalVarDecl.
        if (llvmVar->hasInitializer())
            return;

        auto firstBlock = var->getFirstBlock();

        if (!firstBlock)
            return;

        // Poison and add a global ctor.
        llvmVar->setInitializer(llvm::PoisonValue::get(llvmVar->getValueType()));

        llvm::FunctionType* ctorType = llvm::FunctionType::get(llvmBuilder->getVoidTy(), {}, false);

        std::string ctorName = "__slang_init_";

        llvm::StringRef linkageName, prettyName;
        maybeGetName(&linkageName, &prettyName, var);
        ctorName += linkageName;

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        llvm::Function* llvmCtor = llvm::Function::Create(
            ctorType,
            llvm::GlobalValue::InternalLinkage,
            ctorName,
            *llvmModule);

        // The return instruction of a global value initializer is supposed to
        // set the global value, so we intercept that return and turn it into a
        // store.
        auto epilogue = [&](IRReturn* ret) -> llvm::Value*
        {
            auto val = ret->getVal();
            types->emitStore(llvmVar, findValue(val), val->getDataType(), defaultPointerRules);
            return llvmBuilder->CreateRetVoid();
        };
        emitGlobalValueWithCode(var, llvmCtor, epilogue);

        llvm::Constant* ctorData[3] = {
            // Leave some room for other global constructors to run first.
            llvmBuilder->getInt32(globalCtors.getCount() + 16),
            llvmCtor,
            llvmVar};

        llvm::Constant* ctorEntry = llvm::ConstantStruct::get(
            llvmCtorType,
            llvm::ArrayRef(ctorData, llvmCtorType->getNumElements()));

        globalCtors.add(ctorEntry);
    }

    void emitGlobalDebugInfo(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto debugSource = as<IRDebugSource>(inst))
            {
                auto filename = as<IRStringLit>(debugSource->getFileName())->getStringSlice();

                std::filesystem::path path(std::string(filename.begin(), filename.getLength()));
                sourceDebugInfo[inst] = llvmDebugBuilder->createFile(
                    path.filename().string().c_str(),
                    path.parent_path().string().c_str(),
                    std::nullopt,
                    getStringLitAsLLVMString(debugSource->getSource()));
            }
        }
    }

    void emitGlobalDeclarations(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
                ensureFuncDecl(func);
            else if (auto globalVar = as<IRGlobalVar>(inst))
                emitGlobalVarDecl(globalVar);
        }
    }

    // Using std::string as the LLVM API works with that. It's not to be
    // ingested by Slang.
    std::string expandIntrinsic(
        llvm::Function* llvmFunc,
        IRInst* intrinsicInst,
        IRFunc* parentFunc,
        UnownedStringSlice intrinsicText)
    {
        std::string out;
        llvm::raw_string_ostream expanded(out);

        auto resultType = parentFunc->getResultType();

        char const* cursor = intrinsicText.begin();
        char const* end = intrinsicText.end();

        // This is a bit of a hack. We add a block to the function so that
        // it gets printed as a definition instead of a declaration, and then
        // fill in the actual contents later in this function.
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);
        llvmFunc->print(expanded);
        bb->eraseFromParent();

        while (out.size() > 0 && out.back() != '{')
            out.pop_back();

        expanded << "\nentry:\n";

        auto parseNumber = [&]()
        {
            char d = *cursor;
            SLANG_RELEASE_ASSERT(CharUtil::isDigit(d));
            UInt n = 0;
            while (CharUtil::isDigit(d))
            {
                n = n * 10 + (d - '0');
                cursor++;
                if (cursor == end)
                    break;
                d = *cursor;
            }
            return n;
        };

        while (cursor < end)
        {
            // Indicates the start of a 'special' sequence
            if (*cursor == '$')
            {
                cursor++;
                SLANG_RELEASE_ASSERT(cursor < end);
                char d = *cursor++;
                switch (d)
                {
                case 'S':
                case 'T':
                    // Value type of parameter
                    {
                        IRType* type = nullptr;
                        if (*cursor == 'R')
                        {
                            // Get the return type of the call
                            cursor++;
                            type = resultType;
                        }
                        else
                        {
                            UInt argIndex = parseNumber();
                            SLANG_RELEASE_ASSERT(argIndex < parentFunc->getParamCount());
                            type = parentFunc->getParamType(argIndex);
                        }

                        if (d == 'S')
                        {
                            IRSizeAndAlignment sizeAndAlignment =
                                types->getSizeAndAlignment(type, defaultPointerRules);
                            expanded << sizeAndAlignment.getStride();
                        }
                        else
                        {
                            auto llvmType = types->getType(type);
                            llvmType->print(expanded);
                        }
                    }
                    break;
                case '_': // '_<number>' excludes the type name
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    // Function parameter.
                    {
                        if (d != '_')
                            --cursor;
                        UInt argIndex = parseNumber();
                        SLANG_RELEASE_ASSERT(argIndex < parentFunc->getParamCount());
                        IRParam* param = nullptr;

                        for (IRParam* p : parentFunc->getParams())
                        {
                            if (argIndex == 0)
                            {
                                param = p;
                                break;
                            }
                            argIndex--;
                        }

                        auto llvmParam = findValue(param);
                        llvmParam->printAsOperand(expanded, d != '_');
                    }
                    break;
                case '[':
                    // Type operand
                    {
                        bool sizeQuery = false;
                        if (*cursor == 'S')
                        {
                            // '[S<number>]' returns the size of the type.
                            sizeQuery = true;
                            cursor++;
                        }
                        UInt argIndex = parseNumber() + 1;

                        SLANG_RELEASE_ASSERT(argIndex < intrinsicInst->getOperandCount());

                        auto arg = intrinsicInst->getOperand(argIndex);

                        IRType* argType = as<IRType>(arg);

                        if (sizeQuery)
                        {
                            IRSizeAndAlignment sizeAndAlignment =
                                types->getSizeAndAlignment(argType, defaultPointerRules);
                            expanded << sizeAndAlignment.getStride();
                        }
                        else
                        {
                            auto llvmType = types->getType(argType);
                            llvmType->print(expanded);
                        }

                        SLANG_ASSERT(*cursor == ']');
                        cursor++;
                    }
                    break;
                default:
                    SLANG_UNEXPECTED("bad format in intrinsic definition");
                    break;
                }
            }
            else
            {
                expanded << *cursor;
                cursor++;
            }
        }

        // If the inline IR contains an assignment to %result, we assume that a
        // corresponding return instruction needs to be added automatically.
        bool hasReturnValue = as<IRVoidType>(resultType) == nullptr;
        bool resultFound = out.find("%result") != std::string::npos;

        if (!hasReturnValue || resultFound)
        {
            expanded << "\nret ";
            if (hasReturnValue)
            {
                auto llvmResultType = types->getType(resultType);
                llvmResultType->print(expanded);
                expanded << " %result";
            }
            else
                expanded << "void";
        }

        expanded << "\n}\n";

        return out;
    }

    // If the Slang IR function containing a target intrinsic is not fully
    // specialized, this function creates a new concrete function based on the
    // call.
    IRFunc* createTargetIntrinsicFunc(IRCall* callInst)
    {
        IRBuilder builder(callInst->getModule());

        auto resultType = callInst->getDataType();
        List<IRType*> paramTypes;

        for (IRInst* arg : callInst->getArgsList())
        {
            paramTypes.add(arg->getDataType());
        }

        IRFuncType* funcType =
            builder.getFuncType(paramTypes.getCount(), paramTypes.begin(), resultType);
        IRFunc* func = builder.createFunc();
        func->setFullType(funcType);

        builder.setInsertInto(func);
        IRBlock* headerBlock = builder.emitBlock();
        builder.setInsertInto(headerBlock);

        for (IRInst* arg : callInst->getArgsList())
        {
            builder.emitParam(arg->getDataType());
        }
        return func;
    }

    // This function inserts the given LLVM IR in the global scope.
    // Uses std::string due to that being what LLVM emits and ingests.
    void emitGlobalLLVMIR(std::string& llvmTextIR)
    {
        llvm::SMDiagnostic diag;
        // This creates a temporary module with only the contents of llvmTextIR.
        std::unique_ptr<llvm::Module> sourceModule =
            llvm::parseAssemblyString(llvmTextIR, diag, *llvmContext);

        if (!sourceModule)
        { // Failed to parse the intrinsic
            std::string msgStr;
            llvm::raw_string_ostream diagOut(msgStr);
            diag.print("", diagOut, false);

            msgStr = "\n" + llvmTextIR + "\n" + msgStr;
            UnownedStringSlice msgSlice(msgStr.data(), msgStr.size());

            codeGenContext->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::snippetParsingFailed,
                msgSlice);
            return;
        }

        sourceModule->setDataLayout(targetMachine->createDataLayout());
        sourceModule->setTargetTriple(targetMachine->getTargetTriple());

        // Finally, we merge the contents of the temporary module back to the
        // main llvmModule.
        llvmLinker->linkInModule(std::move(sourceModule), llvm::Linker::OverrideFromSrc);
    }

    void emitTargetIntrinsicFunction(
        IRFunc* func,
        llvm::Function*& llvmFunc,
        IRInst* intrinsicInst,
        UnownedStringSlice intrinsicDef)
    {
        llvmFunc->setLinkage(llvm::Function::LinkageTypes::ExternalLinkage);

        std::string funcName = llvmFunc->getName().str();
        std::string llvmTextIR = expandIntrinsic(llvmFunc, intrinsicInst, func, intrinsicDef);

        emitGlobalLLVMIR(llvmTextIR);

        llvmFunc = llvm::cast<llvm::Function>(llvmModule->getNamedValue(funcName));
        llvmFunc->setLinkage(llvm::Function::LinkageTypes::PrivateLinkage);
        // Intrinsic functions usually do nothing other than call a single
        // instruction; we can just inline them by default.
        llvmFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);

        mapInstToLLVM[func] = llvmFunc;
    }

    void emitGlobalValueWithCode(
        IRGlobalValueWithCode* code,
        llvm::Function* llvmFunc,
        FuncEpilogueCallback epilogueCallback)
    {
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        debugInlinedScope = false;

        auto func = as<IRFunc>(code);
        bool intrinsic = false;
        if (func)
        {
            UnownedStringSlice intrinsicDef;
            IRInst* intrinsicInst;
            intrinsic = Slang::findTargetIntrinsicDefinition(
                func,
                codeGenContext->getTargetReq()->getTargetCaps(),
                intrinsicDef,
                intrinsicInst);
            if (intrinsic)
            {
                emitTargetIntrinsicFunction(func, llvmFunc, intrinsicInst, intrinsicDef);
            }
        }

        llvm::DISubprogram* sp = nullptr;
        if (debug)
        {
            IRDebugFunction* debugFunc = nullptr;
            if (auto debugFuncDecoration = code->findDecoration<IRDebugFuncDecoration>())
                debugFunc = as<IRDebugFunction>(debugFuncDecoration->getDebugFunc());
            sp = ensureDebugFunc(code, debugFunc);
        }

        if (sp != nullptr)
        {
            llvmFunc->setSubprogram(sp);
            currentLocalScope = sp;
        }

        if (!intrinsic)
        {
            // Create all blocks first, so that branch instructions can refer
            // to blocks that haven't been filled in yet.
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);
                mapInstToLLVM[irBlock] = llvmBlock;
            }

            // Then, fill in the blocks. Lucky for us, there is pretty much a
            // 1:1 correspondence between Slang IR blocks and LLVM IR blocks, so
            // this is straightforward.
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock =
                    llvm::cast<llvm::BasicBlock>(mapInstToLLVM.getValue(irBlock));
                llvmBuilder->SetInsertPoint(llvmBlock);

                // If we are emitting debug data and are starting the first
                // block, set the debug location at the start of the function.
                if (sp && irBlock == code->getFirstBlock())
                {
                    llvmBuilder->SetCurrentDebugLocation(
                        llvm::DILocation::get(*llvmContext, sp->getLine(), 0, sp));
                }

                // Then, add the regular instructions.
                for (auto irInst : irBlock->getOrdinaryInsts())
                {
                    auto llvmInst = emitLLVMInstruction(irInst, epilogueCallback);
                    if (llvmInst)
                        mapInstToLLVM[irInst] = llvmInst;
                }
            }
        }

        currentLocalScope = nullptr;
    }

    // Creates a function that runs a whole workgroup of the entry point.
    // TODO: Ideally, this should vectorize over the workgroup.
    llvm::Function* emitComputeEntryPointGroupFunc(
        IRFunc* entryPoint,
        llvm::Function* llvmEntryPoint,
        IREntryPointDecoration* entryPointDecor)
    {
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

        llvm::Type* uintType = llvmBuilder->getInt32Ty();

        llvm::Type* argTypes[3] = {
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0)};
        llvm::FunctionType* dispatchFuncType =
            llvm::FunctionType::get(llvmBuilder->getVoidTy(), argTypes, false);

        auto groupName = String(entryPointDecor->getName()->getStringSlice());
        groupName.append("_Group");
        llvm::Function* dispatcher = llvm::Function::Create(
            dispatchFuncType,
            llvm::GlobalValue::ExternalLinkage,
            llvm::StringRef(groupName.begin(), groupName.getLength()),
            *llvmModule);

        for (auto& arg : dispatcher->args())
            arg.addAttr(llvm::Attribute::NoAlias);

        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", dispatcher);
        llvmBuilder->SetInsertPoint(entryBlock);

        // This is the data passed to the actual entry point.
        llvm::StructType* threadVaryingInputType = llvm::StructType::get(
            // groupID
            llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3)),
            // groupThreadID
            llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3)));

        // TODO: DeSPMD? That will also require scalarization of vector
        // operations, so it should be done after everything has been emitted.

        llvm::Type* groupIDType = llvm::ArrayType::get(uintType, 3);
        llvm::Value* groupID = dispatcher->getArg(0);
        llvm::Value* threadInput = llvmBuilder->CreateAlloca(threadVaryingInputType);

        llvm::Value* threadID[3];

        auto numThreadsDecor = entryPoint->findDecoration<IRNumThreadsDecoration>();
        llvm::Value* workGroupSize[3] = {
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getX()) : 1),
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getY()) : 1),
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getZ()) : 1)};

        for (int i = 0; i < 3; ++i)
        {
            llvm::Value* inIndices[2] = {llvmBuilder->getInt32(0), llvmBuilder->getInt32(i)};

            auto inGroupPtr = llvmBuilder->CreateGEP(groupIDType, groupID, inIndices);
            auto inGroupID = llvmBuilder->CreateLoad(
                uintType,
                inGroupPtr,
                llvm::Twine("ingroupID").concat(llvm::Twine(i)));

            llvm::Value* outIndices[3] = {
                llvmBuilder->getInt32(0),
                llvmBuilder->getInt32(0),
                llvmBuilder->getInt32(i)};
            auto outGroupPtr = llvmBuilder->CreateGEP(
                threadVaryingInputType,
                threadInput,
                outIndices,
                llvm::Twine("groupID").concat(llvm::Twine(i)));
            llvmBuilder->CreateStore(inGroupID, outGroupPtr);

            outIndices[1] = llvmBuilder->getInt32(1);
            threadID[i] = llvmBuilder->CreateGEP(
                threadVaryingInputType,
                threadInput,
                outIndices,
                llvm::Twine("threadID").concat(llvm::Twine(i)));
        }

        llvm::BasicBlock* threadEntryBlocks[3];
        llvm::BasicBlock* threadBodyBlocks[3];
        llvm::BasicBlock* threadEndBlocks[3];
        llvm::BasicBlock* endBlock;

        // Create all blocks first, so that they can jump around.
        for (int i = 0; i < 3; ++i)
        {
            threadEntryBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("threadEntry").concat(llvm::Twine(i)),
                dispatcher);
            threadBodyBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("threadBody").concat(llvm::Twine(i)),
                dispatcher);
        }

        for (int i = 2; i >= 0; --i)
        {
            threadEndBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("threadEnd").concat(llvm::Twine(i)),
                dispatcher);
        }
        endBlock = llvm::BasicBlock::Create(*llvmContext, llvm::Twine("end"), dispatcher);

        // Populate thread dispatch headers.
        for (int i = 0; i < 3; ++i)
        {
            llvmBuilder->CreateStore(llvmBuilder->getInt32(0), threadID[i]);
            llvmBuilder->CreateBr(threadEntryBlocks[i]);
            llvmBuilder->SetInsertPoint(threadEntryBlocks[i]);

            auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
            auto cond =
                llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, workGroupSize[i]);

            auto merge = i == 0 ? endBlock : threadEndBlocks[i - 1];
            llvmBuilder->CreateCondBr(cond, threadBodyBlocks[i], merge);
            llvmBuilder->SetInsertPoint(threadBodyBlocks[i]);
        }

        // Do the call to the actual entry point function.
        llvm::Value* args[3] = {threadInput, dispatcher->getArg(1), dispatcher->getArg(2)};
        llvmBuilder->CreateCall(llvmEntryPoint, args);
        llvmBuilder->CreateBr(threadEndBlocks[2]);

        // Finish thread dispatch blocks
        for (int i = 2; i >= 0; --i)
        {
            llvmBuilder->SetInsertPoint(threadEndBlocks[i]);
            auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
            auto inc = llvmBuilder->CreateAdd(id, llvmBuilder->getInt32(1));
            llvmBuilder->CreateStore(inc, threadID[i]);
            llvmBuilder->CreateBr(threadEntryBlocks[i]);
        }

        llvmBuilder->SetInsertPoint(endBlock);
        llvmBuilder->CreateRetVoid();
        return dispatcher;
    }

    // This generates a dispatching function for a given compute entry
    // point. It runs workgroups serially.
    void emitComputeEntryPointDispatcher(
        IRFunc* entryPoint,
        llvm::Function* llvmEntryPoint,
        IREntryPointDecoration* entryPointDecor)
    {
        llvm::Function* groupFunc =
            emitComputeEntryPointGroupFunc(entryPoint, llvmEntryPoint, entryPointDecor);

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

        llvm::Type* uintType = llvmBuilder->getInt32Ty();

        llvm::Type* argTypes[3] = {
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0)};
        llvm::FunctionType* dispatchFuncType =
            llvm::FunctionType::get(llvmBuilder->getVoidTy(), argTypes, false);

        auto entryPointName = entryPointDecor->getName()->getStringSlice();
        llvm::Function* dispatcher = llvm::Function::Create(
            dispatchFuncType,
            llvm::GlobalValue::ExternalLinkage,
            llvm::StringRef(entryPointName.begin(), entryPointName.getLength()),
            *llvmModule);

        for (auto& arg : dispatcher->args())
            arg.addAttr(llvm::Attribute::NoAlias);

        // This has to be ABI-compatible with the C++ target. So don't go
        // changing this without a good reason to.
        llvm::StructType* varyingInputType = llvm::StructType::get(
            // startGroupID
            llvm::ArrayType::get(uintType, 3),
            // endGroupID
            llvm::ArrayType::get(uintType, 3));

        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", dispatcher);
        llvmBuilder->SetInsertPoint(entryBlock);

        llvm::Value* varyingInput = dispatcher->getArg(0);
        llvm::Type* groupIDType = llvm::ArrayType::get(uintType, 3);
        llvm::Value* groupIDVar = llvmBuilder->CreateAlloca(groupIDType);

        llvm::Value* startGroupID[3];
        llvm::Value* groupID[3];
        llvm::Value* endGroupID[3];

        for (int i = 0; i < 3; ++i)
        {
            llvm::Value* varyingIndices[3] = {
                llvmBuilder->getInt32(0),
                llvmBuilder->getInt32(0), // startGroupID && groupID
                llvmBuilder->getInt32(i),
            };

            auto startGroupPtr =
                llvmBuilder->CreateGEP(varyingInputType, varyingInput, varyingIndices);
            startGroupID[i] = llvmBuilder->CreateLoad(
                uintType,
                startGroupPtr,
                llvm::Twine("startGroupID").concat(llvm::Twine(i)));

            llvm::Value* groupIndices[2] = {llvmBuilder->getInt32(0), llvmBuilder->getInt32(i)};
            groupID[i] = llvmBuilder->CreateGEP(
                groupIDType,
                groupIDVar,
                groupIndices,
                llvm::Twine("groupID").concat(llvm::Twine(i)));
            varyingIndices[1] = llvmBuilder->getInt32(1);
            auto endGroupPtr =
                llvmBuilder->CreateGEP(varyingInputType, varyingInput, varyingIndices);
            endGroupID[i] = llvmBuilder->CreateLoad(
                uintType,
                endGroupPtr,
                llvm::Twine("endGroupID").concat(llvm::Twine(i)));
        }
        // We need 3 nested loops... :(

        llvm::BasicBlock* wgEntryBlocks[3];
        llvm::BasicBlock* wgBodyBlocks[3];
        llvm::BasicBlock* wgEndBlocks[3];
        llvm::BasicBlock* endBlock;

        // Create all blocks first, so that they can jump around.
        for (int i = 0; i < 3; ++i)
        {
            wgEntryBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("dispatchEntry").concat(llvm::Twine(i)),
                dispatcher);
            wgBodyBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("dispatchBody").concat(llvm::Twine(i)),
                dispatcher);
        }

        for (int i = 2; i >= 0; --i)
        {
            wgEndBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("dispatchEnd").concat(llvm::Twine(i)),
                dispatcher);
        }
        endBlock = llvm::BasicBlock::Create(*llvmContext, llvm::Twine("end"), dispatcher);

        // Populate group dispatch headers.
        for (int i = 0; i < 3; ++i)
        {
            llvmBuilder->CreateStore(startGroupID[i], groupID[i]);
            llvmBuilder->CreateBr(wgEntryBlocks[i]);
            llvmBuilder->SetInsertPoint(wgEntryBlocks[i]);
            auto id = llvmBuilder->CreateLoad(uintType, groupID[i]);
            auto cond =
                llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, endGroupID[i]);

            auto merge = i == 0 ? endBlock : wgEndBlocks[i - 1];

            llvmBuilder->CreateCondBr(cond, wgBodyBlocks[i], merge);
            llvmBuilder->SetInsertPoint(wgBodyBlocks[i]);
        }

        // Do the call to the actual entry point function.
        llvm::Value* args[3] = {groupIDVar, dispatcher->getArg(1), dispatcher->getArg(2)};
        llvmBuilder->CreateCall(groupFunc, args);
        llvmBuilder->CreateBr(wgEndBlocks[2]);

        // Finish group dispatch blocks
        for (int i = 2; i >= 0; --i)
        {
            llvmBuilder->SetInsertPoint(wgEndBlocks[i]);
            auto id = llvmBuilder->CreateLoad(uintType, groupID[i]);
            auto inc = llvmBuilder->CreateAdd(id, llvmBuilder->getInt32(1));
            llvmBuilder->CreateStore(inc, groupID[i]);
            llvmBuilder->CreateBr(wgEntryBlocks[i]);
        }

        llvmBuilder->SetInsertPoint(endBlock);
        llvmBuilder->CreateRetVoid();
    }

    void emitFuncDefinition(IRFunc* func)
    {
        llvm::Function* llvmFunc = ensureFuncDecl(func);

        // Aggregate return types are turned into an extra parameter instead of
        // a return value, so `storeArg` and `epilogue` are used to route the
        // ostensible return value into that parameter.
        llvm::Value* storeArg = nullptr;
        if (types->isAggregateType(func->getResultType()))
            storeArg = llvmFunc->getArg(llvmFunc->arg_size() - 1);

        auto epilogue = [&](IRReturn* ret) -> llvm::Value*
        {
            if (storeArg)
            {
                auto val = ret->getVal();
                types->emitStore(storeArg, findValue(val), val->getDataType(), defaultPointerRules);
                return llvmBuilder->CreateRetVoid();
            }

            return nullptr;
        };

        emitGlobalValueWithCode(func, llvmFunc, epilogue);

        // We need to emit the dispatching functions for entry points so that
        // they can be called.
        auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
        if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Compute)
        {
            emitComputeEntryPointDispatcher(func, llvmFunc, entryPointDecor);
        }
    }

    void emitGlobalFunctions(IRModule* irModule)
    {
        // Use a separate worklist so that if new functions get added, they
        // won't get iterated over. That can happen in some circumstances with
        // __target_intrinsic.
        List<IRInst*> workList;
        for (auto inst : irModule->getGlobalInsts())
        {
            workList.add(inst);
        }

        for (auto inst : workList)
        {
            if (auto func = as<IRFunc>(inst))
            {
                if (isDefinition(func))
                    emitFuncDefinition(func);
            }
            else if (auto globalVar = as<IRGlobalVar>(inst))
            {
                // Globals may have initializer blocks with code, so they're
                // also handled in this pass.
                emitGlobalVarCtor(globalVar);
            }
        }
    }

    // Emits all remaining global instructions within a function that gets
    // called during the initialization of the program / library (llvm.global_ctors).
    void emitGlobalInstructionCtor()
    {
        if (promotedGlobalInsts.getCount() == 0)
            return;

        llvm::FunctionType* ctorType = llvm::FunctionType::get(llvmBuilder->getVoidTy(), {}, false);

        std::string ctorName = "__slang_global_insts";

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        llvm::Function* llvmCtor = llvm::Function::Create(
            ctorType,
            llvm::GlobalValue::InternalLinkage,
            ctorName,
            *llvmModule);

        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "", llvmCtor);
        llvmBuilder->SetInsertPoint(entryBlock);

        inlineGlobalInstructions = true;

        for (auto [inst, globalVar] : promotedGlobalInsts)
        {
            if (globalVar->hasInitializer())
                continue;

            // Emit the instructions needed to construct the value.
            auto llvmInst = emitLLVMInstruction(inst);
            globalVar->setInitializer(llvm::PoisonValue::get(globalVar->getValueType()));
            types->emitStore(globalVar, llvmInst, inst->getDataType(), defaultPointerRules);
        }

        inlineGlobalInstructions = false;
        llvmBuilder->CreateRetVoid();

        llvm::Constant* ctorData[3] = {
            llvmBuilder->getInt32(0),
            llvmCtor,
            llvm::ConstantPointerNull::get(llvm::PointerType::get(*llvmContext, 0))};

        llvm::Constant* ctorEntry = llvm::ConstantStruct::get(
            llvmCtorType,
            llvm::ArrayRef(ctorData, llvmCtorType->getNumElements()));

        globalCtors.add(ctorEntry);
    }

    void processModule(IRModule* irModule)
    {
        emitGlobalDebugInfo(irModule);
        emitGlobalDeclarations(irModule);
        emitGlobalFunctions(irModule);
        emitGlobalInstructionCtor();
    }
*/
};

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter(codeGenContext);
    //emitter.processModule(irModule);
    return emitter.builder->generateAssembly(outArtifact);
}

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter(codeGenContext);
    //emitter.processModule(irModule);
    return emitter.builder->generateObjectCode(outArtifact);
}

SlangResult emitLLVMJITFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter(codeGenContext, true);
    //emitter.processModule(irModule);
    return emitter.builder->generateJITLibrary(outArtifact);
}

} // namespace Slang
