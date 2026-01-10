#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "core/slang-char-util.h"
#include "core/slang-func-ptr.h"
#include "core/slang-io.h"
#include "core/slang-type-text-util.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-util.h"
#include "slang-llvm/slang-llvm-builder.h"

using namespace slang;

// This file converts a Slang IR module into LLVM IR, optionally optimizing it
// and generating object code. LLVM's IRBuilder is used through our own COM
// interface ILLVMBuilder; see `slang-llvm-builder.h` for more context on what
// that is and why.
//
// One overarching design choice is that all memory-related LLVM instructions
// (e.g. getelementptr and alloca) are emitted with byte arrays (with proper
// alignment annotations) instead of their actual types. There are two reasons
// for this:
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
// `maybeEmitConstant`.
//
// So, Slang types are translated to LLVM in three ways:
//
// * Value types: types as they appear in SSA values. Value types are not
//   observable through memory (LLVMTypeTranslator::getValueType())
//
// * Storage types: types for values observable through memory. The only context
//   where these exist are global constants.
//
// * Debug types: types for debug metadata. They allow debuggers to show
//   human-readable type names and contents of variables.
//   (LLVMTypeTranslator::getDebugType())
//
// Outside of global constants, aggregates like structs and arrays in Slang IR
// never actually become concrete types in LLVM IR.

namespace Slang
{

// This function attempts to find names associated with a variable or function.
// `linkageName` is the name actually used as a symbol in object code and LLVM
// IR, while `prettyName` is shown to the user in a debugger.
static bool maybeGetName(CharSlice* linkageNameOut, CharSlice* prettyNameOut, IRInst* irInst)
{
    *linkageNameOut = CharSlice();
    *prettyNameOut = CharSlice();

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

    *linkageNameOut = CharSlice(linkageName.begin(), linkageName.getLength());
    *prettyNameOut = CharSlice(prettyName.begin(), prettyName.getLength());
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

static bool isSigned(IRInst* value)
{
    IRType* elementType = getVectorOrCoopMatrixElementType(value->getDataType());
    return isSignedType(elementType);
}

// Tries to determine a debug location for a given inst. E.g. if given a struct,
// it'll first try to check if it has explicit debug location information, and
// if not, it'll give the module's beginning at least.
static void findDebugLocation(
    const Dictionary<IRInst*, LLVMDebugNode*>& instToDebugLLVM,
    IRInst* inst,
    LLVMDebugNode*& file,
    int& line)
{
    if (!inst || inst->getOp() == kIROp_ModuleInst)
    {
        file = nullptr;
        line = 0;
    }
    else if (auto debugLocation = inst->findDecoration<IRDebugLocationDecoration>())
    {
        file = instToDebugLLVM.getValue(debugLocation->getSource());
        line = int(getIntVal(debugLocation->getLine()));
    }
    else
        findDebugLocation(instToDebugLLVM, inst->getParent(), file, line);
}

static CharSlice getStringLitAsSlice(IRInst* inst)
{
    auto source = as<IRStringLit>(inst)->getStringSlice();
    return CharSlice(source.begin(), source.getLength());
}

// This class helps with converting types from Slang IR to LLVM IR. It can
// create types for two different contexts:
//
// * getValueType(): creates types which appear in SSA values and are not
//   observable through memory.
//
// * getDebugType(): "pretty" types with correct offset annotations so that
//   debuggers can show the contents of variables.
//
class LLVMTypeTranslator
{
private:
    ILLVMBuilder* builder;
    TargetRequest* targetReq;
    const Dictionary<IRInst*, LLVMDebugNode*>* instToDebugLLVM;

    Dictionary<IRType*, LLVMType*> valueTypeMap;
    Dictionary<IRTypeLayoutRules*, Dictionary<IRType*, LLVMDebugNode*>> debugTypeMap;
    Dictionary<IRType*, IRType*> legalizedResourceTypeMap;

public:
    LLVMTypeTranslator(
        ILLVMBuilder* builder,
        TargetRequest* targetReq,
        const Dictionary<IRInst*, LLVMDebugNode*>& instToDebugLLVM)
        : builder(builder), targetReq(targetReq), instToDebugLLVM(&instToDebugLLVM)
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
        }
    }

    LLVMType* getFuncType(IRFuncType* funcType)
    {
        List<LLVMType*> paramTypes;
        for (UInt i = 0; i < funcType->getParamCount(); ++i)
        {
            IRType* paramType = funcType->getParamType(i);
            paramTypes.add(getValueType(paramType));
        }

        auto resultType = funcType->getResultType();
        auto llvmReturnType = getValueType(resultType);

        // If attempting to return an aggregate, turn it into an extra
        // output parameter that is passed with a pointer.
        if (isAggregateType(resultType))
        {
            paramTypes.add(builder->getPointerType());
            llvmReturnType = builder->getVoidType();
        }

        return builder->getFunctionType(
            llvmReturnType,
            Slice(paramTypes.begin(), paramTypes.getCount()));
    }

    // Returns the type you must use for passing around SSA values in LLVM IR.
    LLVMType* getValueType(IRType* type)
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
            break;

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
            llvmType = getValueType(as<IRRateQualifiedType>(type)->getValueType());
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
        case kIROp_FuncType:
            // LLVM only has opaque pointers now, so everything that lowers as
            // a pointer is just that same opaque pointer.
            llvmType = builder->getPointerType();
            break;

        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                LLVMType* elemType = getValueType(vecType->getElementType());
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvmType = builder->getVectorType(elemCount, elemType);
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

    LLVMDebugNode* getDebugFuncType(IRFuncType* funcType, IRTypeLayoutRules* rules)
    {
        List<LLVMDebugNode*> params;
        for (UInt i = 0; i < funcType->getParamCount(); ++i)
        {
            IRType* paramType = funcType->getParamType(i);
            params.add(getDebugType(paramType, rules));
        }

        return builder->getDebugFunctionType(
            getDebugType(funcType->getResultType(), rules),
            Slice(params.begin(), params.getCount()));
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
            llvmType = builder->getDebugIntType("intptr_t", true, builder->getPointerSizeInBits());
            break;
        case kIROp_UIntPtrType:
            llvmType =
                builder->getDebugIntType("uintptr_t", false, builder->getPointerSizeInBits());
            break;

        case kIROp_PtrType:
            {
                auto ptr = as<IRPtrType>(legalizedType);
                llvmType = builder->getDebugPointerType(getDebugType(ptr->getValueType(), rules));
            }
            break;

        case kIROp_NativePtrType:
        case kIROp_RawPointerType:
        case kIROp_FuncType:
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
                auto elemCount = getIntVal(vecType->getElementCount());
                LLVMDebugNode* elemType = getDebugType(vecType->getElementType(), rules);
                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(vecType, rules);
                llvmType = builder->getDebugVectorType(
                    sizeAndAlignment.size,
                    sizeAndAlignment.alignment,
                    elemCount,
                    elemType);
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
                int line;
                findDebugLocation(*instToDebugLLVM, structType, file, line);

                List<LLVMDebugNode*> types;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;
                    LLVMDebugNode* debugType = getDebugType(fieldType, rules);

                    IRSizeAndAlignment fieldSizeAndAlignment =
                        getSizeAndAlignment(field->getFieldType(), rules);
                    IRIntegerValue offset = getOffset(field, rules);

                    IRStructKey* key = field->getKey();
                    CharSlice linkageName, prettyName;
                    maybeGetName(&linkageName, &prettyName, key);

                    types.add(builder->getDebugStructField(
                        debugType,
                        prettyName,
                        offset,
                        fieldSizeAndAlignment.size,
                        fieldSizeAndAlignment.alignment,
                        file,
                        line));
                }
                CharSlice linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, legalizedType);
                sizeAndAlignment.size = align(sizeAndAlignment.size, sizeAndAlignment.alignment);

                llvmType = builder->getDebugStructType(
                    Slice(types.begin(), types.getCount()),
                    prettyName,
                    sizeAndAlignment.size,
                    sizeAndAlignment.alignment,
                    file,
                    line);
            }
            break;

        case kIROp_RateQualifiedType:
            llvmType = getDebugType(as<IRRateQualifiedType>(legalizedType)->getValueType(), rules);
            break;

        default:
            {
                CharSlice linkageName, prettyName;
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

        IRBuilder irBuilder(type->getModule());

        IRType* legalizedType = type;
        switch (type->getOp())
        {
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            legalizedType = irBuilder.getRawPointerType();
            break;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
            {
                IRStructType* s = irBuilder.createStructType();
                auto ptrKey = irBuilder.createStructKey();
                auto sizeKey = irBuilder.createStructKey();
                irBuilder.createStructField(s, ptrKey, irBuilder.getRawPointerType());
                irBuilder.createStructField(s, sizeKey, irBuilder.getType(kIROp_UIntPtrType));
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
                    IRStructType* s = irBuilder.createStructType();
                    for (auto field : structType->getFields())
                    {
                        auto fieldType = field->getFieldType();
                        auto legalizedFieldType = legalizeResourceTypes(fieldType);
                        irBuilder.createStructField(s, field->getKey(), legalizedFieldType);
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
                        irBuilder.getArrayType(legalizedElemType, arrayType->getElementCount());
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
        Slang::getOffset(targetReq, rules, legalField, &offset);
        return offset;
    }

    // Use this instead of the regular Slang::getSizeAndAlignment(), it handles
    // legalized resource types correctly.
    IRSizeAndAlignment getSizeAndAlignment(IRType* type, IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAlignment;

        Slang::getSizeAndAlignment(targetReq, rules, legalizeResourceTypes(type), &sizeAlignment);

        return sizeAlignment;
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

    UInt getVectorAlignedCount(IRVectorType* vecType, IRTypeLayoutRules* rules)
    {
        auto elemCount = getIntVal(vecType->getElementCount());
        IRSizeAndAlignment elementAlignment;
        Slang::getSizeAndAlignment(targetReq, rules, vecType->getElementType(), &elementAlignment);
        IRSizeAndAlignment vectorAlignment =
            rules->getVectorSizeAndAlignment(elementAlignment, elemCount);

        return align(vectorAlignment.size, vectorAlignment.alignment) / elementAlignment.size;
    }

    // Checks if an array type would be incorrectly padded if translated to LLVM
    // naively.
    //
    // The problematic situation occurs when there is a Slang array type T[...]
    // where T is a type that gets laid out so that its size is less than its
    // stride in the given layout rules.
    //
    // LLVM is heavily influenced by C/C++, which enforce that the size of a
    // type is always rounded up to its alignment, so that the size and stride
    // are always identical, and an LLVM array type bakes in this behavior.
    //
    // The transformation we use to work around this is to translate problematic
    // Slang array types into:
    //
    // struct
    // {
    //     paddedElemType init[count-1];
    //     unpaddedElemType last;
    // };
    //
    // This change to the LLVM representation of the type doesn't create
    // problems for logic that indexes into an array, because we use untyped
    // pointers and explicit offsets rather than relying on type-based
    // getElementPtr operations. The only place where we need to consider this
    // layout detail (other than lowering the type itself) is when emitting
    // constants in `maybeEmitConstants`, as that's the only place where
    // aggregate types (like arrays) actually need to be present in the emitted
    // IR.
    bool needsArrayTypePaddingWorkaround(IRArrayTypeBase* arrayType, IRTypeLayoutRules* rules)
    {
        auto irElemCount = arrayType->getElementCount();
        auto elemCount = int(getIntVal(irElemCount));
        IRSizeAndAlignment sizeAndAlignment =
            getSizeAndAlignment(arrayType->getElementType(), rules);

        bool elementIsPaddedToAlignment =
            sizeAndAlignment.size == align(sizeAndAlignment.size, sizeAndAlignment.alignment);
        return elemCount != 0 && !elementIsPaddedToAlignment;
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
    Dictionary<IRInst*, LLVMInst*> deferredGlobalInsts;

    // Map of debug instructions to LLVM debug nodes.
    Dictionary<IRInst*, LLVMDebugNode*> instToDebugLLVM;

    // Used to skip some instructions whose value is derived from DebugVar.
    HashSet<IRInst*> debugInsts;

    bool debugInlinedScope = false;
    UInt uniqueIDCounter = 0;

    // Cached, because it's used so often.
    LLVMType* int32Type = nullptr;
    LLVMType* int64Type = nullptr;

    // List of global variables whose initializers could not be emitted as
    // constants. They need to be initialized during runtime.
    List<IRGlobalVar*> deferredGlobalVars;

    // Stack allocations are done in the function header block. They must be
    // done in the first block, because LLVM is bad at hoisting allocas from
    // loops, leading into stack overflows if a long-running loop has local
    // variables.
    LLVMInst* stackHeaderBlock = nullptr;

    // This is stored so that we can get back to inserting into the current
    // block after inserting allocas into the header block.
    LLVMInst* currentBlock = nullptr;

    struct ConstantInfo
    {
        // Includes trailing padding
        LLVMInst* padded = nullptr;

        // Excludes trailing padding
        LLVMInst* unpadded = nullptr;

        LLVMInst* get(bool withTrailingPadding) { return withTrailingPadding ? padded : unpadded; }

        ConstantInfo& operator=(LLVMInst* constant)
        {
            padded = unpadded = constant;
            return *this;
        }
    };
    Dictionary<IRInst*, ConstantInfo> constantMap;

    // Used to add code in in front of return in a function. If it returns
    // nullptr, the LLVM return instruction is generated "normally". Otherwise,
    // it's expected that you generated it in this function.
    using FuncEpilogueCallback = Slang::Func<LLVMInst*, IRReturn*>;

    SlangResult init(CodeGenContext* context, bool useJIT = false)
    {
        codeGenContext = context;

        ISlangSharedLibrary* library = codeGenContext->getSession()->getOrLoadSlangLLVM();
        if (!library)
        {
            codeGenContext->getSink()->diagnose(
                SourceLoc(),
                Diagnostics::unableToGenerateCodeForTarget,
                TypeTextUtil::getCompileTargetName(
                    SlangCompileTarget(codeGenContext->getTargetFormat())));
            return SLANG_FAIL;
        }

        using BuilderFuncV2 = SlangResult (*)(
            const SlangUUID& intfGuid,
            Slang::ILLVMBuilder** out,
            Slang::LLVMBuilderOptions options,
            Slang::IArtifact** outErrorArtifact);

        auto builderFunc = (BuilderFuncV2)library->findFuncByName("createLLVMBuilder_V2");
        if (!builderFunc)
            return SLANG_FAIL;

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
        builderOpt.targetTriple = CharSlice();
        // For JIT, we must always use the host target triple
        if (!useJIT)
            builderOpt.targetTriple =
                CharSlice(targetTripleOption.begin(), targetTripleOption.getLength());
        builderOpt.cpu = CharSlice(cpuOption.begin(), cpuOption.getLength());
        builderOpt.features = CharSlice(featOption.begin(), featOption.getLength());
        builderOpt.debugCommandLineArgs = CharSlice(params.begin(), params.getLength());
        builderOpt.debugLevel = (SlangDebugInfoLevel)getOptions().getDebugInfoLevel();
        builderOpt.optLevel = (SlangOptimizationLevel)getOptions().getOptimizationLevel();
        builderOpt.fp32DenormalMode = (SlangFpDenormalMode)getOptions().getDenormalModeFp32();
        builderOpt.fp64DenormalMode = (SlangFpDenormalMode)getOptions().getDenormalModeFp64();
        builderOpt.fpMode = (SlangFloatingPointMode)getOptions().getFloatingPointMode();

        List<TerminatedCharSlice> llvmArguments;
        List<String> downstreamArgs = getOptions().getDownstreamArgs("llvm");
        for (const auto& arg : downstreamArgs)
            llvmArguments.add(TerminatedCharSlice(arg.getBuffer()));
        builderOpt.llvmArguments = Slice(llvmArguments.begin(), llvmArguments.getCount());

        ComPtr<IArtifact> errorArtifact;
        SLANG_RETURN_ON_FAIL(builderFunc(
            ILLVMBuilder::getTypeGuid(),
            builder.writeRef(),
            builderOpt,
            errorArtifact.writeRef()));

        if (errorArtifact)
        {
            auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(errorArtifact);
            for (Int i = 0; i < diagnostics->getCount(); ++i)
            {
                auto diag = diagnostics->getAt(i);
                getSink()->diagnoseRaw(Severity(diag->severity), diag->text);
            }
            return SLANG_FAIL;
        }

        debug = getOptions().getDebugInfoLevel() != DebugInfoLevel::None;

        if (getOptions().shouldUseCLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::C);
        else if (getOptions().shouldUseScalarLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::Scalar);
        else if (getOptions().shouldUseDXLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::D3DConstantBuffer);
        else
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::LLVM);

        types.reset(
            new LLVMTypeTranslator(builder, codeGenContext->getTargetReq(), instToDebugLLVM));

        int32Type = builder->getIntType(32);
        int64Type = builder->getIntType(64);
        return SLANG_OK;
    }

    DiagnosticSink* getSink() { return codeGenContext->getSink(); }

    CompilerOptionSet& getOptions() { return codeGenContext->getTargetProgram()->getOptionSet(); }

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
                    llvmConstant =
                        builder->getConstantInt(types->getValueType(type), litInst->value.intVal);
                break;
            }

        case kIROp_FloatLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                    llvmConstant = builder->getConstantFloat(
                        types->getValueType(type),
                        litInst->value.floatVal);
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
                            values.add(builder->getConstantExtractElement(constVal, int(j)));
                    }
                    else
                        values.add(constVal);
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
                    auto elemType = types->getValueType(vectorType->getElementType());
                    llvmConstant =
                        builder->getConstantArray(Slice(values.begin(), values.getCount()));

                    Int alignedCount =
                        Int(types->getVectorAlignedCount(vectorType, defaultPointerRules));
                    if (alignedCount != values.getCount())
                    {
                        // Fill padding with poison, nobody should use it in
                        // computations.
                        while (values.getCount() < alignedCount)
                            values.add(builder->getPoison(elemType));
                        llvmConstant.padded =
                            builder->getConstantArray(Slice(values.begin(), values.getCount()));
                    }
                }
                else
                {
                    llvmConstant =
                        builder->getConstantVector(Slice(values.begin(), values.getCount()));
                }
            }
            break;
        case kIROp_MakeVectorFromScalar:
            {
                auto vectorType = cast<IRVectorType>(inst->getDataType());
                int elemCount = int(getIntVal(vectorType->getElementCount()));
                LLVMInst* value = maybeEmitConstant(inst->getOperand(0), inAggregate);
                if (!value)
                    return nullptr;

                if (inAggregate)
                {
                    auto elemType = types->getValueType(vectorType->getElementType());
                    auto values = List<LLVMInst*>::makeRepeated(value, elemCount);
                    llvmConstant =
                        builder->getConstantArray(Slice(values.begin(), values.getCount()));

                    Int alignedCount =
                        Int(types->getVectorAlignedCount(vectorType, defaultPointerRules));
                    if (alignedCount != values.getCount())
                    {
                        while (values.getCount() < alignedCount)
                            values.add(builder->getPoison(elemType));

                        llvmConstant.padded =
                            builder->getConstantArray(Slice(values.begin(), values.getCount()));
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

                if (types->needsArrayTypePaddingWorkaround(arrayType, defaultPointerRules))
                {
                    auto lastPart = maybeEmitConstant(
                        inst->getOperand(inst->getOperandCount() - 1),
                        true,
                        false);

                    auto initPart =
                        builder->getConstantArray(Slice(values.begin(), values.getCount() - 1));

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
                    types->getSizeAndAlignment(structType, defaultPointerRules);

                List<LLVMInst*> values;
                auto field = structType->getFields().begin();
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa, ++field)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa), true, false);
                    if (!constVal)
                        return nullptr;

                    IRIntegerValue offset = types->getOffset(*field, defaultPointerRules);
                    // Insert padding until we're at the requested offset.
                    emitPaddingConstant(values, llvmSize, offset);

                    values.add(constVal);
                    llvmSize += builder->getStoreSizeOf(constVal);
                }

                llvmConstant.unpadded =
                    builder->getConstantStruct(Slice<LLVMInst*>(values.begin(), values.getCount()));

                emitPaddingConstant(values, llvmSize, sizeAndAlignment.getStride());

                llvmConstant.padded =
                    builder->getConstantStruct(Slice<LLVMInst*>(values.begin(), values.getCount()));
            }
            break;
        default:
            break;
        }

        if ((llvmConstant.padded || llvmConstant.unpadded) && !inAggregate)
            constantMap[inst] = llvmConstant;
        return llvmConstant.get(withTrailingPadding);
    }


    // Finds the value of an instruction that has already been emitted, OR
    // creates the value if it's a constant. Also, if inlineGlobalInstructions
    // is set, this will inline such instructions as needed.
    LLVMInst* findValue(IRInst* inst)
    {
        if (mapInstToLLVM.containsKey(inst))
            return mapInstToLLVM.getValue(inst);

        bool globalInstruction = inst->getParent()->getOp() == kIROp_ModuleInst;

        auto op = inst->getOp();

        // Check first if the instruction is a constant expression. If so, we
        // need not consider any other means of finding its value.
        LLVMInst* constVal = nullptr;
        if (op != kIROp_MakeArray && op != kIROp_MakeStruct)
        {
            // kIROp_MakeArray & kIROp_MakeStruct are intentionally omitted
            // here; they must not be emitted as values in any other context
            // than global vars. `emitGlobalVarDecl` and
            // `emitGlobalInstructionCtor` handle that part.
            constVal = maybeEmitConstant(inst);
        }

        LLVMInst* llvmValue = nullptr;
        if (constVal)
        {
            llvmValue = constVal;
        }
        else if (op == kIROp_VoidLit)
        {
            return nullptr;
        }
        else if (op == kIROp_Specialize)
        {
            auto s = as<IRSpecialize>(inst);
            auto g = s->getBase();
            auto e = "Specialize instruction remains in IR for LLVM emit, is something "
                     "undefined?\n" +
                     dumpIRToString(g);
            SLANG_UNEXPECTED(e.getBuffer());
        }
        else if (globalInstruction)
        {
            // We also inline everything that is not an aggregate, as they're
            // generally unproblematic to inline.
            if (inlineGlobalInstructions || !types->isAggregateType(inst->getDataType()))
                return emitLLVMInstruction(inst);

            // This is a non-constant global instruction getting referenced. So,
            // we generate a global variable for it (if we don't have one yet)
            // and report this incident.
            auto type = inst->getDataType();
            IRSizeAndAlignment sizeAndAlignment =
                types->getSizeAndAlignment(type, defaultPointerRules);
            if (deferredGlobalInsts.containsKey(inst))
            {
                llvmValue = deferredGlobalInsts.getValue(inst);
            }
            else
            {
                llvmValue = builder->declareGlobalVariable(
                    sizeAndAlignment.size,
                    sizeAndAlignment.alignment,
                    false);
                deferredGlobalInsts[inst] = llvmValue;
            }

            // Global variables are pointers to the actual data. If the type
            // is not an aggregate, we need to load to get the value.
            // Aggregates are passed around as pointers so they don't need
            // to be loaded.
            if (!types->isAggregateType(type))
                llvmValue = builder->emitLoad(
                    types->getValueType(type),
                    llvmValue,
                    sizeAndAlignment.alignment);

            // Don't cache result if the emitted instruction was
            // deferred, we'll need to be able to generate the inlined
            // version later!
            return llvmValue;
        }
        else
        {
            SLANG_UNEXPECTED("Unsupported value type for LLVM target, or referring to an "
                             "instruction that hasn't been emitted yet!");
        }

        SLANG_ASSERT(llvmValue);

        if (llvmValue)
            mapInstToLLVM[inst] = llvmValue;
        return llvmValue;
    }

    // Allocates stack memory for given type. Returns a pointer to the start of
    // that memory.
    LLVMInst* emitStackVariable(IRType* type, IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAlignment = types->getSizeAndAlignment(type, rules);

        // All allocas should occur in the first block:
        // https://llvm.org/docs/Frontend/PerformanceTips.html#use-of-allocas
        builder->insertIntoBlock(stackHeaderBlock);

        LLVMInst* alloca = builder->emitAlloca(sizeAlignment.getStride(), sizeAlignment.alignment);

        builder->insertIntoBlock(currentBlock);
        return alloca;
    }

    // Computes an offset with `indexInst` based on `llvmPtr`. `indexInst` need
    // not be constant.
    LLVMInst* emitArrayGetElementPtr(
        LLVMInst* llvmPtr,
        LLVMInst* indexInst,
        IRType* elemType,
        IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment sizeAndAlignment = types->getSizeAndAlignment(elemType, rules);
        return builder->emitGetElementPtr(llvmPtr, sizeAndAlignment.getStride(), indexInst);
    }

    // Computes a pointer to a struct field.
    LLVMInst* emitStructGetElementPtr(
        LLVMInst* llvmPtr,
        IRStructField* field,
        IRTypeLayoutRules* rules)
    {
        IRIntegerValue offset = types->getOffset(field, rules);
        return builder->emitGetElementPtr(
            llvmPtr,
            1,
            builder->getConstantInt(builder->getIntType(32), offset));
    }

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
        IRSizeAndAlignment dstSizeAlignment = types->getSizeAndAlignment(type, dstLayout);
        IRSizeAndAlignment srcSizeAlignment = types->getSizeAndAlignment(type, srcLayout);

        auto minSize = std::min(dstSizeAlignment.size, srcSizeAlignment.size);

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
    }

    // llvmVal must be using `getValueType(valType)`. It will be stored in
    // llvmPtr. Returns the store instruction.
    LLVMInst* emitStore(
        LLVMInst* llvmPtr,
        LLVMInst* llvmVal,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false)
    {
        IRSizeAndAlignment sizeAlignment = types->getSizeAndAlignment(valType, rules);
        switch (valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto storageType = builder->getIntType(int(sizeAlignment.size * 8));
                auto expanded = builder->emitIntResize(llvmVal, storageType);
                return builder->emitStore(expanded, llvmPtr, sizeAlignment.alignment, isVolatile);
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
            return builder->emitStore(llvmVal, llvmPtr, sizeAlignment.alignment, isVolatile);
        }
    }

    // Returns the loaded data using `getValueType(valType)` from llvmPtr.
    // Returns the load instruction (= loaded value)
    LLVMInst* emitLoad(
        LLVMInst* llvmPtr,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false)
    {
        IRSizeAndAlignment sizeAlignment = types->getSizeAndAlignment(valType, rules);

        switch (valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto llvmType = types->getValueType(valType);
                auto storageType = builder->getIntType(int(sizeAlignment.size * 8));
                auto storageBool =
                    builder->emitLoad(storageType, llvmPtr, sizeAlignment.alignment, isVolatile);
                return builder->emitIntResize(storageBool, llvmType);
            }
            break;
        case kIROp_ArrayType:
        case kIROp_StructType:
            if (rules == defaultPointerRules)
            {
                // Equal memory layout, so we can just memcpy.
                LLVMInst* llvmVar = emitStackVariable(valType, defaultPointerRules);

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
                LLVMInst* llvmVar = emitStackVariable(valType, defaultPointerRules);
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
                auto llvmType = types->getValueType(valType);
                return builder->emitLoad(llvmType, llvmPtr, sizeAlignment.alignment, isVolatile);
            }
        }
    }

    LLVMInst* emitCompare(IRInst* inst)
    {
        SLANG_ASSERT(inst->getOperandCount() == 2);

        bool aSigned = isSigned(inst->getOperand(0));
        bool bSigned = isSigned(inst->getOperand(1));

        auto a = findValue(inst->getOperand(0));
        auto b = findValue(inst->getOperand(1));

        LLVMCompareOp op;
        switch (inst->getOp())
        {
        case kIROp_Less:
            op = LLVMCompareOp::Less;
            break;
        case kIROp_Leq:
            op = LLVMCompareOp::LessEqual;
            break;
        case kIROp_Eql:
            op = LLVMCompareOp::Equal;
            break;
        case kIROp_Neq:
            op = LLVMCompareOp::NotEqual;
            break;
        case kIROp_Greater:
            op = LLVMCompareOp::Greater;
            break;
        case kIROp_Geq:
            op = LLVMCompareOp::GreaterEqual;
            break;
        default:
            SLANG_UNEXPECTED("Unsupported compare op");
            break;
        }

        return builder->emitCompareOp(op, a, aSigned, b, bSigned);
    }

    LLVMInst* emitArithmetic(IRInst* inst)
    {
        auto resultType = types->getValueType(inst->getDataType());

        if (inst->getOperandCount() == 1)
        {
            auto llvmValue = findValue(inst->getOperand(0));
            LLVMUnaryOp op;
            switch (inst->getOp())
            {
            case kIROp_Neg:
                op = LLVMUnaryOp::Negate;
                break;
            case kIROp_Not:
            case kIROp_BitNot:
                op = LLVMUnaryOp::Not;
                break;
            default:
                SLANG_UNEXPECTED("Unsupported unary arithmetic op");
                break;
            }
            return builder->emitUnaryOp(op, llvmValue);
        }
        else if (inst->getOperandCount() == 2)
        {
            LLVMBinaryOp op;
            switch (inst->getOp())
            {
            case kIROp_Add:
                op = LLVMBinaryOp::Add;
                break;
            case kIROp_Sub:
                op = LLVMBinaryOp::Sub;
                break;
            case kIROp_Mul:
                op = LLVMBinaryOp::Mul;
                break;
            case kIROp_Div:
                op = LLVMBinaryOp::Div;
                break;
            case kIROp_IRem:
            case kIROp_FRem:
                op = LLVMBinaryOp::Rem;
                break;
            case kIROp_And:
            case kIROp_BitAnd:
                op = LLVMBinaryOp::And;
                break;
            case kIROp_Or:
            case kIROp_BitOr:
                op = LLVMBinaryOp::Or;
                break;
            case kIROp_BitXor:
                op = LLVMBinaryOp::Xor;
                break;
            case kIROp_Rsh:
                op = LLVMBinaryOp::RightShift;
                break;
            case kIROp_Lsh:
                op = LLVMBinaryOp::LeftShift;
                break;
            default:
                SLANG_UNEXPECTED("Unsupported binary arithmetic op");
                break;
            }

            return builder->emitBinaryOp(
                op,
                findValue(inst->getOperand(0)),
                findValue(inst->getOperand(1)),
                resultType,
                isSigned(inst));
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected number of operands for arithmetic op");
        }
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

    static LLVMInst* _defaultOnReturnHandler(IRReturn*)
    {
        SLANG_ASSERT_FAILURE("Unexpected terminator in global scope!");
    }

    // Caution! This is only for emitting things which are considered
    // instructions in LLVM! It won't work for IRBlocks, IRFuncs & such.
    LLVMInst* emitLLVMInstruction(
        IRInst* inst,
        FuncEpilogueCallback onReturn = _defaultOnReturnHandler)
    {
        LLVMInst* llvmInst = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_PtrLit:
            llvmInst = maybeEmitConstant(inst);
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
                    llvmInst = builder->emitReturn(findValue(retInst->getVal()));
            }
            break;

        case kIROp_Var:
            {
                auto var = static_cast<IRVar*>(inst);
                auto ptrType = var->getDataType();

                LLVMInst* llvmVar = emitStackVariable(ptrType->getValueType(), defaultPointerRules);

                CharSlice linkageName, prettyName;
                if (maybeGetName(&linkageName, &prettyName, inst))
                {
                    builder->setName(llvmVar, linkageName);

                    // TODO: This is a bit of a hack. DebugVar fails to get
                    // linked to the actual Var sometimes, which is why we
                    // automatically create a debug var for each Slang IR var if
                    // it has a name.:/
                    if (debug)
                    {
                        auto varType =
                            types->getDebugType(ptrType->getValueType(), defaultPointerRules);
                        auto debugVar = builder->emitDebugVar(prettyName, varType);
                        builder->emitDebugValue(debugVar, llvmVar);
                    }
                }

                llvmInst = llvmVar;
            }
            break;

        case kIROp_Loop:
        case kIROp_UnconditionalBranch:
            {
                auto branch = as<IRUnconditionalBranch>(inst);
                llvmInst = builder->emitBranch(findValue(branch->getTargetBlock()));
            }
            break;

        case kIROp_IfElse:
            {
                auto ifelseInst = static_cast<IRIfElse*>(inst);
                auto cond = findValue(ifelseInst->getCondition());
                auto trueBlock = findValue(ifelseInst->getTrueBlock());
                auto falseBlock = findValue(ifelseInst->getFalseBlock());
                llvmInst = builder->emitCondBranch(cond, trueBlock, falseBlock);
            }
            break;

        case kIROp_Switch:
            {
                auto switchInst = static_cast<IRSwitch*>(inst);
                auto defaultBlock = findValue(switchInst->getBreakLabel());
                auto llvmCondition = findValue(switchInst->getCondition());

                auto defaultLabel = switchInst->getDefaultLabel();
                if (defaultLabel)
                    defaultBlock = findValue(defaultLabel);

                List<LLVMInst*> values;
                List<LLVMInst*> blocks;
                for (UInt c = 0; c < switchInst->getCaseCount(); c++)
                {
                    auto value = switchInst->getCaseValue(c);
                    auto intLit = as<IRIntLit>(value);
                    SLANG_ASSERT(intLit);

                    values.add(maybeEmitConstant(intLit));
                    blocks.add(findValue(switchInst->getCaseLabel(c)));
                }
                llvmInst = builder->emitSwitch(
                    llvmCondition,
                    Slice(values.begin(), values.getCount()),
                    Slice(blocks.begin(), blocks.getCount()),
                    defaultBlock);
            }
            break;

        case kIROp_Select:
            {
                auto selectInst = static_cast<IRSelect*>(inst);
                llvmInst = builder->emitSelect(
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

                llvmInst = emitStore(
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

                llvmInst = emitLoad(
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
                    // Aggregates are always stack-allocated; we need to give a
                    // valid pointer even if the value is undefined.
                    llvmInst = emitStackVariable(type, defaultPointerRules);
                }
                else
                {
                    llvmInst = builder->getPoison(types->getValueType(type));
                }
            }
            break;

        case kIROp_MakeUInt64:
            {
                auto lowbits = findValue(inst->getOperand(0));
                auto highbits = findValue(inst->getOperand(1));

                lowbits = builder->emitCast(lowbits, int64Type, false, false);
                highbits = builder->emitCast(highbits, int64Type, false, false);

                highbits = builder->emitBinaryOp(
                    LLVMBinaryOp::LeftShift,
                    highbits,
                    builder->getConstantInt(int64Type, 32));
                llvmInst = builder->emitBinaryOp(LLVMBinaryOp::Or, lowbits, highbits);
            }
            break;

        case kIROp_MakeArray:
            llvmInst = emitStackVariable(inst->getDataType(), defaultPointerRules);
            for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
            {
                auto op = inst->getOperand(aa);
                LLVMInst* ptr = emitArrayGetElementPtr(
                    llvmInst,
                    builder->getConstantInt(int32Type, aa),
                    op->getDataType(),
                    defaultPointerRules);
                emitStore(ptr, findValue(op), op->getDataType(), defaultPointerRules);
            }
            break;

        case kIROp_MakeStruct:
            {
                IRStructType* type = as<IRStructType>(inst->getDataType());
                llvmInst = emitStackVariable(type, defaultPointerRules);
                auto field = type->getFields().begin();
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa, ++field)
                {
                    LLVMInst* ptr = emitStructGetElementPtr(llvmInst, *field, defaultPointerRules);
                    auto op = inst->getOperand(aa);
                    emitStore(ptr, findValue(op), op->getDataType(), defaultPointerRules);
                }
            }
            break;

        case kIROp_MakeArrayFromElement:
            {
                auto arrayType = cast<IRArrayType>(inst->getDataType());
                auto elementCount = getIntVal(arrayType->getElementCount());
                llvmInst = emitStackVariable(inst->getDataType(), defaultPointerRules);
                auto element = inst->getOperand(0);
                auto llvmElement = findValue(element);
                for (IRIntegerValue i = 0; i < elementCount; ++i)
                {
                    LLVMInst* ptr = emitArrayGetElementPtr(
                        llvmInst,
                        builder->getConstantInt(int32Type, i),
                        element->getDataType(),
                        defaultPointerRules);
                    emitStore(ptr, llvmElement, element->getDataType(), defaultPointerRules);
                }
            }
            break;

        case kIROp_MakeVector:
            llvmInst = maybeEmitConstant(inst);
            if (!llvmInst)
            {
                auto llvmType = types->getValueType(inst->getDataType());

                // MakeVector of a scalar is a scalar.
                if (!as<IRVectorType>(inst->getDataType()))
                {
                    llvmInst = findValue(inst->getOperand(0));
                    break;
                }

                llvmInst = builder->getPoison(llvmType);
                UInt elemIndex = 0;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto op = inst->getOperand(aa);
                    auto val = findValue(op);

                    if (auto vector = as<IRVectorType>(op->getDataType()))
                    {
                        auto elemCount = getIntVal(vector->getElementCount());
                        for (IRIntegerValue j = 0; j < elemCount; ++j)
                        {
                            auto entry = builder->emitExtractElement(
                                val,
                                builder->getConstantInt(int32Type, j));
                            llvmInst = builder->emitInsertElement(
                                llvmInst,
                                entry,
                                builder->getConstantInt(int32Type, elemIndex));
                            elemIndex++;
                        }
                    }
                    else
                    {
                        llvmInst = builder->emitInsertElement(
                            llvmInst,
                            val,
                            builder->getConstantInt(int32Type, elemIndex));
                        elemIndex++;
                    }
                }
            }
            break;

        case kIROp_MakeVectorFromScalar:
            llvmInst = maybeEmitConstant(inst);
            if (!llvmInst)
            {
                auto val = findValue(inst->getOperand(0));

                auto vector = cast<IRVectorType>(inst->getDataType());
                auto elemCount = getIntVal(vector->getElementCount());
                llvmInst = builder->emitVectorSplat(val, elemCount);
            }
            break;

        case kIROp_Swizzle:
            {
                auto swizzleInst = static_cast<IRSwizzle*>(inst);
                auto baseInst = swizzleInst->getBase();
                if (swizzleInst->getElementCount() == 1)
                {
                    llvmInst = builder->emitExtractElement(
                        findValue(baseInst),
                        findValue(swizzleInst->getElementIndex(0)));
                }
                else
                {
                    List<int> mask;
                    mask.reserve(swizzleInst->getElementCount());
                    for (UInt i = 0; i < swizzleInst->getElementCount(); ++i)
                    {
                        int val = int(as<IRIntLit>(swizzleInst->getElementIndex(i))->getValue());
                        mask.add(val);
                    }
                    llvmInst = builder->emitVectorShuffle(
                        findValue(baseInst),
                        Slice(mask.begin(), mask.getCount()));
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
                    auto index = maybeEmitConstant(irElementIndex);
                    auto llvmSrcElement =
                        builder->emitExtractElement(llvmSrc, builder->getConstantInt(int32Type, i));
                    llvmInst = builder->emitInsertElement(llvmInst, llvmSrcElement, index);
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

                    auto llvmDstElement = emitArrayGetElementPtr(
                        llvmDst,
                        maybeEmitConstant(irElementIndex),
                        elementType,
                        rules);
                    auto llvmSrcElement =
                        builder->emitExtractElement(llvmSrc, builder->getConstantInt(int32Type, i));
                    llvmInst = emitStore(llvmDstElement, llvmSrcElement, elementType, rules);
                }
            }
            break;

        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
        case kIROp_CastPtrToInt:
        case kIROp_CastPtrToBool:
        case kIROp_CastIntToPtr:
            llvmInst = builder->emitCast(
                findValue(inst->getOperand(0)),
                types->getValueType(inst->getDataType()),
                isSigned(inst->getOperand(0)),
                isSignedType(inst->getDataType()));
            break;

        case kIROp_PtrCast:
            // ptr-to-ptr casts are always no-ops due to opaque pointers.
            llvmInst = findValue(inst->getOperand(0));
            break;

        case kIROp_BitCast:
            llvmInst = builder->emitBitCast(
                findValue(inst->getOperand(0)),
                types->getValueType(inst->getDataType()));
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

                llvmInst = emitStructGetElementPtr(llvmBase, field, rules);
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

                LLVMInst* ptr = emitStructGetElementPtr(llvmBase, field, rules);

                llvmInst = emitLoad(ptr, field->getFieldType(), rules);
            }
            break;

        case kIROp_GetOffsetPtr:
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
                else
                    SLANG_ASSERT_FAILURE("Unknown pointer type for GetOffsetPtr!");

                llvmInst = emitArrayGetElementPtr(
                    findValue(baseInst),
                    findValue(indexInst),
                    baseType,
                    getPtrLayoutRules(baseInst));
            }
            break;

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
                else if (auto ptrLikeType = as<IRPointerLikeType>(baseInst->getDataType()))
                {
                    baseType = as<IRType>(ptrLikeType->getOperand(0));
                }
                else
                    SLANG_ASSERT_FAILURE("Unknown pointer type for GetElementPtr!");

                // I _REALLY_ dislike that this helper function needs an
                // IRBuilder :/
                IRBuilder irBuilder(inst->getModule());
                IRType* elemType = getElementType(irBuilder, baseType);

                llvmInst = emitArrayGetElementPtr(
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
                    llvmInst = builder->emitExtractElement(llvmVal, findValue(indexInst));
                }
                else if (auto arrayType = as<IRArrayTypeBase>(baseTy))
                {
                    // emitGEP + emitLoad.
                    auto rules = getPtrLayoutRules(baseInst);
                    auto elemType = arrayType->getElementType();
                    LLVMInst* ptr =
                        emitArrayGetElementPtr(llvmVal, findValue(indexInst), elemType, rules);
                    llvmInst = emitLoad(ptr, elemType, rules);
                }
                else
                    SLANG_ASSERT_FAILURE("Unknown data type for GetElement!");
            }
            break;

        case kIROp_BitfieldExtract:
            llvmInst = builder->emitBitfieldExtract(
                findValue(inst->getOperand(0)),
                findValue(inst->getOperand(1)),
                findValue(inst->getOperand(2)),
                types->getValueType(inst->getDataType()),
                isSigned(inst->getOperand(0)));
            break;

        case kIROp_BitfieldInsert:
            llvmInst = builder->emitBitfieldInsert(
                findValue(inst->getOperand(0)),
                findValue(inst->getOperand(1)),
                findValue(inst->getOperand(2)),
                findValue(inst->getOperand(3)),
                types->getValueType(inst->getDataType()));
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

                List<LLVMInst*> args;

                for (IRInst* arg : callInst->getArgsList())
                    args.add(findValue(arg));

                LLVMInst* allocValue = nullptr;
                // If attempting to return an aggregate, turn it into an extra
                // output parameter that is passed with a pointer.
                if (types->isAggregateType(inst->getDataType()))
                {
                    allocValue = emitStackVariable(inst->getDataType(), defaultPointerRules);
                    args.add(allocValue);
                }
                auto returnVal =
                    builder->emitCall(findValue(funcValue), Slice(args.begin(), args.getCount()));
                llvmInst = allocValue ? allocValue : returnVal;
            }
            break;

        case kIROp_Printf:
            {
                List<LLVMInst*> args;
                List<bool> argIsSigned;
                if (inst->getOperandCount() == 2)
                {
                    auto operand = inst->getOperand(1);
                    if (auto makeStruct = as<IRMakeStruct>(operand))
                    {
                        // Flatten the tuple resulting from the variadic pack.
                        for (UInt bb = 0; bb < makeStruct->getOperandCount(); ++bb)
                        {
                            auto op = makeStruct->getOperand(bb);
                            op->getDataType();
                            auto llvmValue = findValue(op);

                            args.add(llvmValue);
                            argIsSigned.add(isSigned(op));
                        }
                    }
                }

                llvmInst = builder->emitPrintf(
                    findValue(inst->getOperand(0)),
                    Slice(args.begin(), args.getCount()),
                    Slice(argIsSigned.begin(), argIsSigned.getCount()));
            }
            break;

        case kIROp_RWStructuredBufferGetElementPtr:
            {
                auto gepInst = static_cast<IRRWStructuredBufferGetElementPtr*>(inst);

                auto baseType =
                    cast<IRHLSLRWStructuredBufferType>(gepInst->getBase()->getDataType());
                auto llvmBase = findValue(gepInst->getBase());
                auto llvmIndex = findValue(gepInst->getIndex());

                auto llvmPtr = builder->emitGetBufferPtr(llvmBase);

                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);
                llvmInst =
                    emitArrayGetElementPtr(llvmPtr, llvmIndex, baseType->getElementType(), rules);
            }
            break;

        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferLoad:
            {
                auto base = inst->getOperand(0);
                auto llvmBase = findValue(base);
                auto llvmIndex = findValue(inst->getOperand(1));

                auto baseType = cast<IRHLSLStructuredBufferTypeBase>(base->getDataType());

                auto llvmBasePtr = builder->emitGetBufferPtr(llvmBase);
                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);

                auto llvmPtr = emitArrayGetElementPtr(
                    llvmBasePtr,
                    llvmIndex,
                    baseType->getElementType(),
                    rules);
                llvmInst = emitLoad(llvmPtr, inst->getDataType(), rules);
            }
            break;

        case kIROp_RWStructuredBufferStore:
            {
                auto base = inst->getOperand(0);
                auto llvmBase = findValue(base);
                auto llvmIndex = findValue(inst->getOperand(1));
                auto val = inst->getOperand(2);

                auto baseType = cast<IRHLSLStructuredBufferTypeBase>(base->getDataType());

                auto llvmBasePtr = builder->emitGetBufferPtr(llvmBase);
                IRTypeLayoutRules* rules = getBufferLayoutRules(baseType);

                auto llvmPtr = emitArrayGetElementPtr(
                    llvmBasePtr,
                    llvmIndex,
                    baseType->getElementType(),
                    rules);
                llvmInst = emitStore(llvmPtr, findValue(val), val->getDataType(), rules);
            }
            break;

        case kIROp_ByteAddressBufferLoad:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                auto llvmBasePtr = builder->emitGetBufferPtr(llvmBase);
                auto llvmPtr = builder->emitGetElementPtr(llvmBasePtr, 1, llvmIndex);

                llvmInst = emitLoad(llvmPtr, inst->getDataType(), defaultPointerRules);
            }
            break;

        case kIROp_ByteAddressBufferStore:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                auto llvmBasePtr = builder->emitGetBufferPtr(llvmBase);
                auto llvmPtr = builder->emitGetElementPtr(llvmBasePtr, 1, llvmIndex);
                auto val = inst->getOperand(inst->getOperandCount() - 1);
                llvmInst =
                    emitStore(llvmPtr, findValue(val), val->getDataType(), defaultPointerRules);
            }
            break;

        case kIROp_StructuredBufferGetDimensions:
            {
                auto getDimensionsInst = cast<IRStructuredBufferGetDimensions>(inst);
                auto buffer = getDimensionsInst->getBuffer();
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(buffer->getDataType());
                auto llvmBuffer = findValue(buffer);

                IRTypeLayoutRules* layout = getBufferLayoutRules(bufferType);

                auto llvmBaseCount = builder->emitGetBufferSize(llvmBuffer);
                llvmBaseCount = builder->emitCast(llvmBaseCount, int32Type, false, false);

                auto returnType = builder->getVectorType(2, int32Type);
                llvmInst = builder->getPoison(returnType);
                llvmInst = builder->emitInsertElement(
                    llvmInst,
                    llvmBaseCount,
                    builder->getConstantInt(int32Type, 0));

                auto stride = types->getSizeAndAlignment(bufferType->getElementType(), layout).size;
                llvmInst = builder->emitInsertElement(
                    llvmInst,
                    builder->getConstantInt(int32Type, stride),
                    builder->getConstantInt(int32Type, 1));
            }
            break;

        case kIROp_GetEquivalentStructuredBuffer:
            {
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(inst->getDataType());
                auto llvmByteBuffer = findValue(inst->getOperand(0));
                IRTypeLayoutRules* layout = getBufferLayoutRules(bufferType);
                auto stride = types->getSizeAndAlignment(bufferType->getElementType(), layout).size;
                llvmInst = builder->emitChangeBufferStride(llvmByteBuffer, 1, stride);
            }
            break;

        case kIROp_MissingReturn:
        case kIROp_Unreachable:
            return builder->emitUnreachable();

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
                    llvmInst = builder->getConstantInt(int32Type, hash);
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

        case kIROp_GetNativeStr:
        case kIROp_MakeString:
            // For now, String == NativeString on the LLVM target.
            llvmInst = findValue(inst->getOperand(0));
            break;

        case kIROp_DebugVar:
            debugInsts.add(inst);
            if (debug)
            {
                auto debugVarInst = static_cast<IRDebugVar*>(inst);

                auto ptrType = as<IRPtrType>(debugVarInst->getDataType());
                auto varType = types->getDebugType(ptrType->getValueType(), defaultPointerRules);

                auto file = instToDebugLLVM.getValue(debugVarInst->getSource());
                int line = int(getIntVal(debugVarInst->getLine()));
                IRInst* argIndex = debugVarInst->getArgIndex();

                CharSlice linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, inst);

                int arg = argIndex && !debugInlinedScope ? int(getIntVal(argIndex)) : -1;
                instToDebugLLVM[inst] = builder->emitDebugVar(prettyName, varType, file, line, arg);
            }
            return nullptr;

        case kIROp_DebugValue:
            debugInsts.add(inst);
            if (debug)
            {
                auto debugValueInst = static_cast<IRDebugValue*>(inst);
                auto debugVar = debugValueInst->getDebugVar();
                if (!instToDebugLLVM.containsKey(debugVar))
                    return nullptr;
                auto value = findValue(debugValueInst->getValue());
                builder->emitDebugValue(instToDebugLLVM.getValue(debugVar), value);
            }
            return nullptr;

        case kIROp_DebugLine:
            debugInsts.add(inst);
            if (debug)
            {
                auto debugLineInst = static_cast<IRDebugLine*>(inst);

                // auto file = instToDebugLLVM.getValue(debugLineInst->getSource());
                auto line = int(getIntVal(debugLineInst->getLineStart()));
                auto col = int(getIntVal(debugLineInst->getColStart()));

                builder->setDebugLocation(line, col);
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
            return nullptr;

        default:
            SLANG_UNEXPECTED("Unsupported instruction for LLVM target!");
            break;
        }

        SLANG_ASSERT(llvmInst);

        return llvmInst;
    }

    bool isDeclExternallyVisible(IRInst* inst)
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
                    return true;
                }
            default:
                break;
            }
        }
        return false;
    }

    LLVMDebugNode* ensureDebugFunc(IRGlobalValueWithCode* func, IRDebugFunction* debugFunc)
    {
        if (instToDebugLLVM.containsKey(func))
            return instToDebugLLVM[func];

        IRFuncType* funcType = as<IRFuncType>(func->getDataType());
        LLVMDebugNode* file = nullptr;
        int line = 0;
        if (debugFunc)
        {
            auto debugType = as<IRType>(debugFunc->getDebugType());

            // TODO: Debug function types are in a bit of a poor state. Let's
            // only use them if they at least have the right type.
            if (auto debugFuncType = as<IRFuncType>(debugType))
                funcType = debugFuncType;

            file = instToDebugLLVM.getValue(debugFunc->getFile());
            line = int(getIntVal(debugFunc->getLine()));
        }

        CharSlice linkageName, prettyName;
        maybeGetName(&linkageName, &prettyName, func);

        LLVMDebugNode* llvmFuncType = types->getDebugFuncType(funcType, defaultPointerRules);
        LLVMDebugNode* sp =
            builder->getDebugFunction(llvmFuncType, prettyName, linkageName, file, line);
        instToDebugLLVM[func] = sp;
        return sp;
    }

    LLVMInst* ensureFuncDecl(IRFunc* func)
    {
        if (mapInstToLLVM.containsKey(func))
            return mapInstToLLVM.getValue(func);

        auto funcType = static_cast<IRFuncType*>(func->getDataType());

        LLVMType* llvmFuncType = types->getFuncType(funcType);

        String tmp;
        CharSlice linkageName, prettyName;
        if (!maybeGetName(&linkageName, &prettyName, func))
        {
            // If the name is missing for whatever reason, just generate one that
            // shouldn't clash with anything else.
            tmp = "__slang_anonymous_func_";
            tmp.append(uniqueIDCounter++);
            linkageName = CharSlice(tmp.begin(), tmp.getLength());
        }

        uint32_t funcAttributes = 0;
        if (isDeclExternallyVisible(func))
            funcAttributes |= SLANG_LLVM_FUNC_ATTR_EXTERNALLYVISIBLE;

        // Attach attributes based on decorations!
        if (func->findDecoration<IRReadNoneDecoration>())
            funcAttributes |= SLANG_LLVM_FUNC_ATTR_READNONE;
        if (func->findDecoration<IRForceInlineDecoration>())
            funcAttributes |= SLANG_LLVM_FUNC_ATTR_ALWAYSINLINE;
        if (func->findDecoration<IRNoInlineDecoration>())
            funcAttributes |= SLANG_LLVM_FUNC_ATTR_NOINLINE;

        LLVMInst* llvmFunc = builder->declareFunction(llvmFuncType, linkageName, funcAttributes);

        UInt i = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
        {
            auto llvmArg = builder->getFunctionArg(llvmFunc, int(i));

            // Aliasing out and reference parameters are UB in Slang, and
            // telling this to LLVM should help with optimization.
            uint32_t attributes = 0;
            if (as<IROutParamType>(funcType->getParamType(i)))
            {
                attributes = SLANG_LLVM_ATTR_WRITEONLY | SLANG_LLVM_ATTR_NOALIAS;
            }
            else if (as<IRBorrowInParamType>(funcType->getParamType(i)))
            {
                attributes = SLANG_LLVM_ATTR_READONLY | SLANG_LLVM_ATTR_NOALIAS;
            }
            else if (as<IRBorrowInOutParamType>(funcType->getParamType(i)))
            {
                attributes = SLANG_LLVM_ATTR_NOALIAS;
            }

            CharSlice argLinkageName, argPrettyName;
            maybeGetName(&argLinkageName, &argPrettyName, pp);

            builder->setArgInfo(llvmArg, argLinkageName, attributes);
            mapInstToLLVM[pp] = llvmArg;
        }

        mapInstToLLVM[func] = llvmFunc;
        return llvmFunc;
    }

    // Declares the global variable in LLVM IR and sets an initializer for it
    // if it is trivial.
    LLVMInst* emitGlobalVarDecl(IRGlobalVar* var)
    {
        IRPtrType* ptrType = var->getDataType();

        IRSizeAndAlignment sizeAndAlignment =
            types->getSizeAndAlignment(ptrType->getValueType(), defaultPointerRules);

        auto firstBlock = var->getFirstBlock();
        LLVMInst* llvmVar = nullptr;

        if (firstBlock)
        {
            auto returnInst = as<IRReturn>(firstBlock->getTerminator());
            if (returnInst)
            {
                // If the initializer is constant, we can emit that to the
                // variable directly.
                IRInst* val = returnInst->getVal();
                if (auto constantValue = maybeEmitConstant(val))
                {
                    // Easy case, it's just a constant in LLVM.
                    llvmVar = builder->declareGlobalVariable(
                        constantValue,
                        sizeAndAlignment.alignment,
                        isDeclExternallyVisible(var));
                }
            }
        }

        if (!llvmVar)
        {
            // No initializer, so emit it untyped.
            llvmVar = builder->declareGlobalVariable(
                sizeAndAlignment.getStride(),
                sizeAndAlignment.alignment,
                isDeclExternallyVisible(var));
            if (firstBlock)
                deferredGlobalVars.add(var);
        }

        CharSlice linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, var))
            builder->setName(llvmVar, linkageName);

        mapInstToLLVM[var] = llvmVar;
        return llvmVar;
    }

    void emitGlobalDebugInfo(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto debugSource = as<IRDebugSource>(inst))
            {
                auto filename = as<IRStringLit>(debugSource->getFileName())->getStringSlice();

                String path = Path::getParentDirectory(filename);
                String file = Path::getFileName(filename);

                instToDebugLLVM[inst] = builder->getDebugFile(
                    CharSlice(file),
                    CharSlice(path),
                    getStringLitAsSlice(debugSource->getSource()));
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

    String expandIntrinsic(
        IRInst* intrinsicInst,
        IRFunc* parentFunc,
        UnownedStringSlice intrinsicText)
    {
        String expanded;

        auto resultType = parentFunc->getResultType();

        char const* cursor = intrinsicText.begin();
        char const* end = intrinsicText.end();

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
                            expanded.append(sizeAndAlignment.getStride());
                        }
                        else
                        {
                            auto llvmType = types->getValueType(type);
                            builder->printType(expanded, llvmType);
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
                        builder->printValue(expanded, llvmParam, d != '_');
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
                            expanded.append(sizeAndAlignment.getStride());
                        }
                        else
                        {
                            auto llvmType = types->getValueType(argType);
                            builder->printType(expanded, llvmType);
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
                expanded.appendChar(*cursor);
                cursor++;
            }
        }

        // If the inline IR contains an assignment to %result, we assume that a
        // corresponding return instruction needs to be added automatically.
        bool hasReturnValue = as<IRVoidType>(resultType) == nullptr;
        bool resultFound = expanded.indexOf("%result") >= 0;

        if (!hasReturnValue || resultFound)
        {
            expanded.append("\nret ");
            if (hasReturnValue)
            {
                auto llvmResultType = types->getValueType(resultType);
                builder->printType(expanded, llvmResultType);
                expanded.append(" %result");
            }
            else
                expanded.append("void");
        }

        return expanded;
    }

    // If the Slang IR function containing a target intrinsic is not fully
    // specialized, this function creates a new concrete function based on the
    // call.
    IRFunc* createTargetIntrinsicFunc(IRCall* callInst)
    {
        IRBuilder irBuilder(callInst->getModule());

        auto resultType = callInst->getDataType();
        List<IRType*> paramTypes;

        for (IRInst* arg : callInst->getArgsList())
        {
            paramTypes.add(arg->getDataType());
        }

        IRFuncType* funcType =
            irBuilder.getFuncType(paramTypes.getCount(), paramTypes.begin(), resultType);
        IRFunc* func = irBuilder.createFunc();
        func->setFullType(funcType);

        irBuilder.setInsertInto(func);
        IRBlock* headerBlock = irBuilder.emitBlock();
        irBuilder.setInsertInto(headerBlock);

        for (IRInst* arg : callInst->getArgsList())
        {
            irBuilder.emitParam(arg->getDataType());
        }
        return func;
    }

    // This function inserts the given LLVM IR in the global scope.
    void emitTargetIntrinsicFunction(
        IRFunc* func,
        LLVMInst*& llvmFunc,
        IRInst* intrinsicInst,
        UnownedStringSlice intrinsicDef)
    {
        String llvmTextIR = expandIntrinsic(intrinsicInst, func, intrinsicDef);

        llvmFunc = builder->emitInlineIRFunction(
            llvmFunc,
            CharSlice(llvmTextIR.begin(), llvmTextIR.getLength()));
        mapInstToLLVM[func] = llvmFunc;
    }

    void emitGlobalValueWithCode(
        IRGlobalValueWithCode* code,
        LLVMInst* llvmFunc,
        FuncEpilogueCallback epilogueCallback)
    {
        sortBlocksInFunc(code);

        // Create all blocks first, so that branch instructions can refer
        // to blocks that haven't been filled in yet.
        for (auto irBlock : code->getBlocks())
        {
            LLVMInst* llvmBlock = builder->emitBlock(llvmFunc);
            mapInstToLLVM[irBlock] = llvmBlock;
        }

        // Then, fill in the blocks. Lucky for us, there is pretty much a
        // 1:1 correspondence between Slang IR blocks and LLVM IR blocks, so
        // this is straightforward.
        for (auto irBlock : code->getBlocks())
        {
            currentBlock = mapInstToLLVM.getValue(irBlock);
            builder->insertIntoBlock(currentBlock);

            // Then, add the regular instructions.
            for (auto irInst : irBlock->getOrdinaryInsts())
            {
                auto llvmInst = emitLLVMInstruction(irInst, epilogueCallback);
                if (llvmInst)
                    mapInstToLLVM[irInst] = llvmInst;
            }
        }
        currentBlock = nullptr;
    }

    void emitFuncDefinition(IRFunc* func)
    {
        LLVMInst* llvmFunc = ensureFuncDecl(func);
        debugInlinedScope = false;

        UnownedStringSlice intrinsicDef;
        IRInst* intrinsicInst;
        bool intrinsic = Slang::findTargetIntrinsicDefinition(
            func,
            codeGenContext->getTargetReq()->getTargetCaps(),
            intrinsicDef,
            intrinsicInst);
        if (intrinsic)
            emitTargetIntrinsicFunction(func, llvmFunc, intrinsicInst, intrinsicDef);

        LLVMDebugNode* llvmDebugFunc = nullptr;
        if (debug)
        {
            IRDebugFunction* debugFunc = nullptr;
            if (auto debugFuncDecoration = func->findDecoration<IRDebugFuncDecoration>())
                debugFunc = as<IRDebugFunction>(debugFuncDecoration->getDebugFunc());
            llvmDebugFunc = ensureDebugFunc(func, debugFunc);
        }

        builder->beginFunction(llvmFunc, llvmDebugFunc);

        if (!intrinsic)
        {
            // Aggregate return types are turned into an extra parameter instead of
            // a return value, so `storeArg` and `epilogue` are used to route the
            // ostensible return value into that parameter.
            LLVMInst* storeArg = nullptr;
            if (types->isAggregateType(func->getResultType()))
                storeArg = builder->getFunctionArg(llvmFunc, int(func->getParamCount()));

            auto epilogue = [&](IRReturn* ret) -> LLVMInst*
            {
                if (storeArg)
                {
                    auto val = ret->getVal();
                    emitStore(storeArg, findValue(val), val->getDataType(), defaultPointerRules);
                    return builder->emitReturn();
                }

                return nullptr;
            };

            stackHeaderBlock = builder->emitBlock(llvmFunc);

            emitGlobalValueWithCode(func, llvmFunc, epilogue);

            // Finally, we need no more stack variables and can make the
            // alloca header jump into the actual entry block.
            builder->insertIntoBlock(stackHeaderBlock);
            builder->emitBranch(mapInstToLLVM.getValue(func->getFirstBlock()));
            stackHeaderBlock = nullptr;
        }

        builder->endFunction(llvmFunc);

        // We need to emit the dispatching functions for entry points so that
        // they can be called.
        auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
        if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Compute)
        {
            auto groupName = String(entryPointDecor->getName()->getStringSlice());
            auto numThreadsDecor = func->findDecoration<IRNumThreadsDecoration>();

            LLVMInst* groupFunc = builder->emitComputeEntryPointWorkGroup(
                llvmFunc,
                getStringLitAsSlice(entryPointDecor->getName()),
                numThreadsDecor ? int(getIntVal(numThreadsDecor->getX())) : 1,
                numThreadsDecor ? int(getIntVal(numThreadsDecor->getY())) : 1,
                numThreadsDecor ? int(getIntVal(numThreadsDecor->getZ())) : 1,
                32);

            auto entryPointName = entryPointDecor->getName();
            builder->emitComputeEntryPointDispatcher(
                groupFunc,
                getStringLitAsSlice(entryPointName));
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
        }
    }

    // Emits all global initializers within a function that gets called during
    // the initialization of the program / library (llvm.global_ctors).
    void emitGlobalInstructionCtor()
    {
        if (deferredGlobalInsts.getCount() == 0 && deferredGlobalVars.getCount() == 0)
            return;

        LLVMInst* globalConstructor = builder->declareGlobalConstructor();

        stackHeaderBlock = builder->emitBlock(globalConstructor);

        // First emit promoted global instructions.
        LLVMInst* initBlock = builder->emitBlock(globalConstructor);
        builder->insertIntoBlock(initBlock);
        currentBlock = initBlock;
        inlineGlobalInstructions = true;

        for (auto [inst, globalVar] : deferredGlobalInsts)
        {
            // Emit the instructions needed to construct the value, and store
            // the results into the variable as appropriate.
            auto llvmInst = emitLLVMInstruction(inst);
            emitStore(globalVar, llvmInst, inst->getDataType(), defaultPointerRules);
        }

        LLVMInst* prevBlock = initBlock;

        // Then, emit the global variables with non-trivial initializers.
        inlineGlobalInstructions = false;

        for (auto globalVar : deferredGlobalVars)
        {
            LLVMInst* llvmVar = mapInstToLLVM.getValue(globalVar);

            LLVMInst* afterBlock = builder->emitBlock(globalConstructor);

            auto epilogue = [&](IRReturn* ret) -> LLVMInst*
            {
                auto val = ret->getVal();
                emitStore(llvmVar, findValue(val), val->getDataType(), defaultPointerRules);
                return builder->emitBranch(afterBlock);
            };
            emitGlobalValueWithCode(globalVar, globalConstructor, epilogue);
            // Generate jump from prevBlock to firstBlock
            LLVMInst* llvmFirstBlock = mapInstToLLVM.getValue(globalVar->getFirstBlock());
            builder->insertIntoBlock(prevBlock);
            builder->emitBranch(llvmFirstBlock);

            prevBlock = afterBlock;
        }

        builder->insertIntoBlock(prevBlock);
        builder->emitReturn();

        builder->insertIntoBlock(stackHeaderBlock);
        builder->emitBranch(initBlock);
        stackHeaderBlock = nullptr;
    }

    void processModule(IRModule* irModule)
    {
        emitGlobalDebugInfo(irModule);
        emitGlobalDeclarations(irModule);
        emitGlobalFunctions(irModule);
        emitGlobalInstructionCtor();
    }
};

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter;
    SLANG_RETURN_ON_FAIL(emitter.init(codeGenContext));
    emitter.processModule(irModule);
    return emitter.builder->generateAssembly(outArtifact);
}

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter;
    SLANG_RETURN_ON_FAIL(emitter.init(codeGenContext));
    emitter.processModule(irModule);
    return emitter.builder->generateObjectCode(outArtifact);
}

SlangResult emitLLVMJITFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter;
    SLANG_RETURN_ON_FAIL(emitter.init(codeGenContext, true));
    emitter.processModule(irModule);
    return emitter.builder->generateJITLibrary(outArtifact);
}

} // namespace Slang
