#include "slang-emit-llvm.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-layout.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "../core/slang-char-util.h"
#include "../core/slang-func-ptr.h"
#include "../compiler-core/slang-artifact-associated-impl.h"
#include "../compiler-core/slang-artifact-desc-util.h"
#include <llvm/AsmParser/Parser.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <filesystem>

using namespace slang;

namespace Slang
{

class LLVMJITSharedLibrary2 : public ComBaseObject, public ISlangSharedLibrary
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE
    {
        if (auto ptr = getInterface(guid))
        {
            return ptr;
        }
        return getObject(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        auto fn = jit->lookup(name);
        return fn ? (void*)fn.get().getValue() : nullptr;
    }

    LLVMJITSharedLibrary2(std::unique_ptr<llvm::orc::LLJIT> jit)
        : jit(std::move(jit))
    {
    }

protected:
    ISlangUnknown* getInterface(const SlangUUID& uuid)
    {
        if (uuid == ISlangUnknown::getTypeGuid() || uuid == ISlangCastable::getTypeGuid() ||
            uuid == ISlangSharedLibrary::getTypeGuid())
        {
            return static_cast<ISlangSharedLibrary*>(this);
        }
        return nullptr;
    }

    void* getObject(const SlangUUID& uuid)
    {
        SLANG_UNUSED(uuid);
        return nullptr;
    }


    std::unique_ptr<llvm::orc::LLJIT> jit;
};

struct BinaryLLVMOutputStream: public llvm::raw_pwrite_stream
{
    List<uint8_t>& output;

    BinaryLLVMOutputStream(List<uint8_t>& output)
        : raw_pwrite_stream(true), output(output)
    {
        SetUnbuffered();
    }

    void write_impl(const char *Ptr, size_t Size) override
    {
        output.insertRange(output.getCount(), reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override
    {
        memcpy(
            output.getBuffer() + Offset,
            reinterpret_cast<const uint8_t*>(Ptr),
            Size);
    }

    uint64_t current_pos() const override
    {
        return output.getCount();
    }

    void reserveExtraSpace(uint64_t ExtraSize) override
    {
        output.reserve(tell() + ExtraSize);
    }
};

static bool maybeGetName(
    llvm::StringRef* linkageNameOut,
    llvm::StringRef* prettyNameOut,
    IRInst* irInst
){
    *linkageNameOut = "";
    *prettyNameOut = "";

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

    if (prettyName.getLength() == 0)
        prettyName = linkageName;
    else if (linkageName.getLength() == 0)
        linkageName = prettyName;

    if (prettyName.getLength() == 0 || linkageName.getLength() == 0)
        return false;

    *linkageNameOut = llvm::StringRef(linkageName.begin(), linkageName.getLength());
    *prettyNameOut = llvm::StringRef(prettyName.begin(), prettyName.getLength());
    return true;
}

static llvm::StringRef getStringLitAsLLVMString(IRInst* inst)
{
    auto source = as<IRStringLit>(inst)->getStringSlice();
    return llvm::StringRef(source.begin(), source.getLength());
}

static IRStructField* findStructField(IRStructType* irStruct, UInt irIndex)
{
    UInt i = 0;
    for (auto field : irStruct->getFields())
    {
        if (i == irIndex)
            return field;
        i++;
    }
    return nullptr;
}

static UInt getStructIndexByKey(IRStructType* irStruct, IRStructKey* irKey)
{
    UInt i = 0;
    for (auto field : irStruct->getFields())
    {
        if(field->getKey() == irKey)
            return i;
        i++;
    }
    SLANG_UNEXPECTED("Requested key that doesn't exist in struct!");
}

static bool isVolatile(IRInst* value)
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

static void findDebugLocation(
    llvm::DICompileUnit* compileUnit,
    const Dictionary<IRInst*, llvm::DIFile*>& sourceDebugInfo,
    IRInst* inst,
    llvm::DIFile*& file,
    llvm::DIScope*& scope,
    unsigned& line
){
    if (!inst || inst->getOp() == kIROp_ModuleInst)
    {
        scope = compileUnit;
        file = compileUnit->getFile();
        line = 0;
    }
    else if (auto debugLocation = inst->findDecoration<IRDebugLocationDecoration>())
    {
        file = sourceDebugInfo.getValue(debugLocation->getSource());
        // TODO: More detailed scope info.
        scope = file;
        line = getIntVal(debugLocation->getLine());
    }
    else findDebugLocation(compileUnit, sourceDebugInfo, inst->getParent(), file, scope, line);
}

static llvm::Instruction::CastOps getLLVMIntExtensionOp(IRType* type)
{
    switch(type->getOp())
    {
    case kIROp_BoolType:
    case kIROp_UInt16Type:
    case kIROp_UIntType:
    case kIROp_UInt64Type:
    case kIROp_UIntPtrType:
    case kIROp_PtrType:
        return llvm::Instruction::CastOps::ZExt;
    default:
        return llvm::Instruction::CastOps::SExt;
    }
}

static UInt parseNumber(const char*& cursor, const char* end)
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
}

// Converting types from Slang IR to LLVM is non-trivial. This is because LLVM
// doesn't provide good tools for enforcing specific layout rules - the layout
// is target-specific. And that is for a good reason: performance.
//
// The only way to get predictable layouts is to use packed structs and lower
// vectors as arrays, inserting padding manually as needed.
//
// Another complication is that creating values of aggregate types in LLVM is 
// not recommended:
// https://llvm.org/docs/Frontend/PerformanceTips.html#avoid-creating-values-of-aggregate-type
// Instead, they must be created with alloca and passed around as pointers in
// the IR. However, we still need to be able to create value instances of
// aggregate types to encode constant structs in the global scope!
//
// This class can translate Slang IR types to LLVM in three different contexts:
//
// * Value
// * Storage (with trailing padding)
// * Storage (without trailing padding)
//
// _Value_ types are not observable through memory, and they essentially map to
// "registers" (SSA values). Hence, they can always use the fastest
// representation.
//
// _Storage_ types are used when the data is observable through a pointer, be
// it via a stack allocation or an arbitrary pointer. There are two flavors,
// with and without trailing padding. The unpadded versions are only ever used
// inside structs, and are not directly accessed. They're interpreted as their
// padded variants.
//
// Storage types can be created for specific IRTypeLayoutRules, and a
// layout-converting memcpy is available as well. This is needed e.g. when
// copying a 'alloca' struct into a StructuredBuffer with a different layout
// than what is used for general pointers.
//
// This class implements caching for types and instructions.
class LLVMTypeTranslator
{
private:
    llvm::IRBuilderBase* builder;
    llvm::DIBuilder* debugBuilder;
    CompilerOptionSet* compilerOptions;
    llvm::DataLayout targetDataLayout;
    llvm::DICompileUnit* compileUnit;
    const Dictionary<IRInst*, llvm::DIFile*>* sourceDebugInfo;
    IRTypeLayoutRules* defaultPointerRules;

    struct StorageTypeInfo
    {
        // Including trailing padding, e.g.
        //
        // struct Type
        // {
        //     uint32_t a;
        //     uint16_t b;
        //     uint8_t PAD[2]; // <<< includes this
        // };
        llvm::Type* padded = nullptr;

        // Excluding trailing padding. May be equal to 'padded' if the type
        // doesn't need trailing padding.
        llvm::Type* unpadded = nullptr;

        // The amount of trailing padding is stored here separately, due to the
        // padded / unpadded type split.
        UInt trailingPadding = 0;

        // Populated for structs, maps a given Slang IR field index to the LLVM
        // struct type field index. This is needed to skip over padding.
        List<UInt> fieldIndexToLLVM;

        // Excludes trailing padding.
        List<UInt> sizePerLLVMField;

        // Filled in separately via getDebugType.
        llvm::DIType* debugType = nullptr;

        llvm::Type* get(bool withTrailingPadding)
        {
            return withTrailingPadding ? padded : unpadded;
        }

        StorageTypeInfo& operator=(llvm::Type* type)
        {
            padded = unpadded = type;
            return *this;
        }
    };

    Dictionary<IRType*, llvm::Type*> valueTypeMap;
    Dictionary<IRTypeLayoutRules*, Dictionary<IRType*, StorageTypeInfo>> storageTypeMap;
    Dictionary<IRType*, IRType*> legalizedResourceTypeMap;

    struct ConstantInfo
    {
        llvm::Constant* padded = nullptr;
        llvm::Constant* unpadded = nullptr;
        llvm::Constant* get(bool withTrailingPadding)
        {
            return withTrailingPadding ? padded : unpadded;
        }

        ConstantInfo& operator=(llvm::Constant* constant)
        {
            padded = unpadded = constant;
            return *this;
        }
    };
    Dictionary<IRInst*, ConstantInfo> constantMap;

public:
    LLVMTypeTranslator(
        llvm::IRBuilderBase& builder,
        llvm::DIBuilder& debugBuilder,
        CompilerOptionSet& compilerOptions,
        llvm::DataLayout targetDataLayout,
        llvm::DICompileUnit* compileUnit,
        const Dictionary<IRInst*, llvm::DIFile*>& sourceDebugInfo,
        IRTypeLayoutRules* rules
    ):  builder(&builder),
        debugBuilder(&debugBuilder),
        compilerOptions(&compilerOptions),
        targetDataLayout(targetDataLayout),
        compileUnit(compileUnit),
        sourceDebugInfo(&sourceDebugInfo),
        defaultPointerRules(rules)
    {
    }

    // Returns the type you must use for passing around SSA values.
    llvm::Type* getValueType(IRType* type)
    {
        if (valueTypeMap.containsKey(type))
            return valueTypeMap.getValue(type);

        llvm::Type* llvmType = nullptr;

        switch (type->getOp())
        {
        case kIROp_VoidType:
            llvmType = builder->getVoidTy();
            break;
        case kIROp_HalfType:
            // TODO: Should we use normal float types for these in SSA? Maybe
            // depending on target triple?
            llvmType = builder->getHalfTy();
            break;
        case kIROp_FloatType:
            llvmType = builder->getFloatTy();
            break;
        case kIROp_DoubleType:
            llvmType = builder->getDoubleTy();
            break;
        case kIROp_Int8Type:
        case kIROp_UInt8Type:
            llvmType = builder->getInt8Ty();
            break;
        case kIROp_BoolType:
            llvmType = builder->getInt1Ty();
            break;
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
            llvmType = builder->getInt16Ty();
            break;
        case kIROp_IntType:
        case kIROp_UIntType:
#if SLANG_PTR_IS_32
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
            llvmType = builder->getInt32Ty();
            break;
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
#if SLANG_PTR_IS_64
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
            llvmType = builder->getInt64Ty();
            break;

        case kIROp_RateQualifiedType:
            llvmType = getValueType(as<IRRateQualifiedType>(type)->getValueType());
            break;

        case kIROp_PtrType:
        case kIROp_NativePtrType:
        case kIROp_NativeStringType:
        case kIROp_RawPointerType:
        case kIROp_OutParamType:
        case kIROp_RefParamType:
        case kIROp_BorrowInOutParamType:
        case kIROp_BorrowInParamType:
        case kIROp_ArrayType:  // Arrays are passed as pointers in SSA values
        case kIROp_UnsizedArrayType:
        case kIROp_StructType: // Structs are passed as pointers in SSA values
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            // LLVM only has opaque pointers now, so everything that lowers as
            // a pointer is just that same opaque pointer.
            llvmType = builder->getPtrTy(0);
            break;
        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                llvm::Type* elemType = getValueType(vecType->getElementType());
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvmType = llvm::VectorType::get(elemType, llvm::ElementCount::getFixed(elemCount));
            }
            break;
        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(type);

                List<llvm::Type*> paramTypes;
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
                    paramTypes.add(builder->getPtrTy(0));
                    llvmReturnType = builder->getVoidTy();
                }

                llvmType = llvm::FunctionType::get(
                    llvmReturnType,
                    llvm::ArrayRef(paramTypes.begin(), paramTypes.end()),
                    false);
            }
            break;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
            llvmType = llvm::StructType::get(
                builder->getPtrTy(0),
                builder->getIntPtrTy(targetDataLayout)
            );
            break;
        default:
            SLANG_UNEXPECTED("Unsupported type for LLVM target!");
            break;
        }

        valueTypeMap[type] = llvmType;
        return llvmType;
    }

    // The storage type must be used for Alloca, Load, Store, GetElementPtr,
    // MemCpy and global variables.
    llvm::Type* getStorageType(IRType* type, IRTypeLayoutRules* rules, bool withTrailingPadding = true)
    {
        {
            Dictionary<IRType*, StorageTypeInfo>& types = storageTypeMap[rules];
            if (types.containsKey(type))
                return types.getValue(type).get(withTrailingPadding);
        }

        StorageTypeInfo llvmTypeInfo;
        switch (type->getOp())
        {
        case kIROp_BoolType:
            if (rules)
            {
                IRSizeAndAlignment sizeAlignment;
                Slang::getSizeAndAlignment(*compilerOptions, rules, type, &sizeAlignment);
                llvmTypeInfo = builder->getIntNTy(sizeAlignment.size * 8);
            }
            else llvmTypeInfo = builder->getInt8Ty();
            break;

        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);
                llvmTypeInfo = rules ? 
                    getExplicitLayoutStructTypeInfo(structType, rules) :
                    getNativeStructTypeInfo(structType);
            }
            break;

        case kIROp_UnsizedArrayType:
        case kIROp_ArrayType:
            {
                auto arrayType = static_cast<IRArrayTypeBase*>(type);
                llvmTypeInfo = getArrayTypeInfo(arrayType, rules);
            }
            break;

        case kIROp_VectorType:
            if (rules)
            {
                auto vecType = static_cast<IRVectorType*>(type);
                // Vector elements should be simple scalar types.
                llvm::Type* elemType = getValueType(vecType->getElementType());
                int elemCount = getIntVal(vecType->getElementCount());

                IRSizeAndAlignment elementAlignment;
                Slang::getSizeAndAlignment(*compilerOptions, rules, vecType->getElementType(), &elementAlignment);

                int alignedCount = getVectorAlignedCount(vecType, rules);

                llvmTypeInfo = llvm::ArrayType::get(elemType, elemCount);
                if (alignedCount != elemCount)
                    llvmTypeInfo.padded = llvm::ArrayType::get(elemType, alignedCount);

                llvmTypeInfo.trailingPadding = (alignedCount - elemCount) * elementAlignment.size;
            }
            else llvmTypeInfo = getValueType(type);
            break;

        case kIROp_RateQualifiedType:
            {
                auto rqt = as<IRRateQualifiedType>(type);
                auto valType = rqt->getValueType();
                llvmTypeInfo.padded = getStorageType(valType, rules, true);
                llvmTypeInfo.unpadded = getStorageType(valType, rules, false);
            }
            break;

        default:
            // No special storage considerations, just use the same type as SSA
            // values.
            llvmTypeInfo = getValueType(type);
            break;
        }

        {
            Dictionary<IRType*, StorageTypeInfo>& types = storageTypeMap[rules];
            types[type] = std::move(llvmTypeInfo);
        }
        return llvmTypeInfo.get(withTrailingPadding);
    }

    llvm::DIType* getDebugType(IRType* type, IRTypeLayoutRules* rules)
    {
        // First, ensure we have storage info for this type!
        getStorageType(type, rules);
        {
            auto& storageInfo = storageTypeMap.getValue(rules).getValue(type);
            if (storageInfo.debugType)
                return storageInfo.debugType;
        }

        llvm::DIType* llvmType = nullptr;
#if SLANG_PTR_IS_32
        const int ptrSize = 32;
#else
        const int ptrSize = 64;
#endif
        switch (type->getOp())
        {
        case kIROp_VoidType:
            llvmType = debugBuilder->createUnspecifiedType("void");
            break;
        case kIROp_HalfType:
            llvmType = debugBuilder->createBasicType("half", 16, llvm::dwarf::DW_ATE_float);
            break;
        case kIROp_FloatType:
            llvmType = debugBuilder->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
            break;
        case kIROp_DoubleType:
            llvmType = debugBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
            break;
        case kIROp_Int8Type:
            llvmType = debugBuilder->createBasicType("int8_t", 8, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UInt8Type:
            llvmType = debugBuilder->createBasicType("int8_t", 8, llvm::dwarf::DW_ATE_unsigned);
            break;
        case kIROp_BoolType:
            llvmType = debugBuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
            break;
        case kIROp_Int16Type:
            llvmType = debugBuilder->createBasicType("int16_t", 16, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UInt16Type:
            llvmType = debugBuilder->createBasicType("uint16_t", 16, llvm::dwarf::DW_ATE_unsigned);
            break;
        case kIROp_IntType:
            llvmType = debugBuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UIntType:
            llvmType = debugBuilder->createBasicType("uint", 32, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_IntPtrType:
            llvmType = debugBuilder->createBasicType("intptr", ptrSize, llvm::dwarf::DW_ATE_signed);
            break;

        case kIROp_UIntPtrType:
            llvmType = debugBuilder->createBasicType("uintptr", ptrSize, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_Int64Type:
            llvmType = debugBuilder->createBasicType("int64_t", 64, llvm::dwarf::DW_ATE_signed);
            break;

        case kIROp_UInt64Type:
            llvmType = debugBuilder->createBasicType("uint64_t", 64, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_PtrType:
            {
                auto ptr = as<IRPtrType>(type);
                llvmType = debugBuilder->createPointerType(getDebugType(ptr->getValueType(), rules), ptrSize);
            }
            break;

        case kIROp_NativePtrType:
            llvmType = debugBuilder->createBasicType("NativeRef", ptrSize, llvm::dwarf::DW_ATE_address);
            break;

        case kIROp_NativeStringType:
            {
                llvmType = debugBuilder->createPointerType(
                    debugBuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char),
                    ptrSize);
            }
            break;

        case kIROp_OutParamType:
        case kIROp_RefParamType:
        case kIROp_BorrowInOutParamType:
        case kIROp_BorrowInParamType:
            {
                auto ptr = as<IRPtrTypeBase>(type);
                llvmType = debugBuilder->createReferenceType(llvm::dwarf::DW_TAG_reference_type, getDebugType(ptr->getValueType(), rules), ptrSize);
            }
            break;

        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvm::DIType* elemType = getDebugType(vecType->getElementType(), rules);
                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(vecType, rules);

                if (sizeAndAlignment.size < sizeAndAlignment.alignment)
                    sizeAndAlignment.size = sizeAndAlignment.alignment;

                llvm::Metadata *subscript = debugBuilder->getOrCreateSubrange(0, elemCount);
                llvm::DINodeArray subscriptArray = debugBuilder->getOrCreateArray(subscript);
                llvmType = debugBuilder->createVectorType(
                    sizeAndAlignment.size*8,
                    sizeAndAlignment.alignment*8,
                    elemType,
                    subscriptArray
                );
            }
            break;

        case kIROp_UnsizedArrayType:
        case kIROp_ArrayType:
            {
                auto arrayType = static_cast<IRArrayTypeBase*>(type);
                llvm::DIType* elemType = getDebugType(arrayType->getElementType(), rules);
                auto irElemCount = arrayType->getElementCount();
                auto elemCount = irElemCount ? getIntVal(irElemCount) : 0;

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(arrayType, rules);

                sizeAndAlignment.size = align(sizeAndAlignment.size, sizeAndAlignment.alignment);

                llvm::Metadata *subscript = debugBuilder->getOrCreateSubrange(0, elemCount);
                llvm::DINodeArray subscriptArray = debugBuilder->getOrCreateArray(subscript);
                llvmType = debugBuilder->createArrayType(sizeAndAlignment.size*8, sizeAndAlignment.alignment*8, elemType, subscriptArray);
            }
            break;

        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(structType, rules);

                llvm::DIFile* file;
                llvm::DIScope* scope;
                unsigned line;
                findDebugLocation(compileUnit, *sourceDebugInfo, structType, file, scope, line);

                List<llvm::Metadata*> types;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;
                    llvm::DIType* debugType = getDebugType(fieldType, rules);

                    IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(field->getFieldType(), rules);
                    IRIntegerValue offset = getOffset(field, rules);

                    IRStructKey* key = field->getKey();
                    llvm::StringRef linkageName, prettyName;
                    maybeGetName(&linkageName, &prettyName, key);

                    types.add(debugBuilder->createMemberType(
                        scope, prettyName, file, line,
                        sizeAndAlignment.size * 8, sizeAndAlignment.alignment * 8,
                        offset * 8,
                        llvm::DINode::FlagZero,
                        debugType
                    ));
                }
                llvm::DINodeArray fieldTypes = debugBuilder->getOrCreateArray(
                    llvm::ArrayRef<llvm::Metadata*>(types.begin(), types.end())
                );

                llvm::StringRef linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, type);

                sizeAndAlignment.size = align(sizeAndAlignment.size, sizeAndAlignment.alignment);

                llvmType = debugBuilder->createStructType(
                    scope,
                    prettyName,
                    file,
                    line,
                    sizeAndAlignment.size*8,
                    sizeAndAlignment.alignment*8,
                    llvm::DINode::FlagZero,
                    nullptr,
                    fieldTypes
                );
            }
            break;

        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(type);

                List<llvm::Metadata*> elements;
                elements.add(getDebugType(funcType->getResultType(), rules));
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    elements.add(getDebugType(paramType, rules));
                }

                llvmType = debugBuilder->createSubroutineType(
                    debugBuilder->getOrCreateTypeArray(
                        llvm::ArrayRef<llvm::Metadata*>(elements.begin(), elements.end()))
                );
            }
            break;

        default:
            {
                llvm::StringRef linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, type);
                llvmType = debugBuilder->createUnspecifiedType(prettyName);
            }
            break;
        }

        {
            auto& storageInfo = storageTypeMap.getValue(rules).getValue(type);
            storageInfo.debugType = llvmType;
        }
        return llvmType;
    }

    // Use this instead of the regular Slang::getOffset(), it handles querying
    // LLVM's own layout as well if 'rules' is nullptr.
    IRIntegerValue getOffset(IRStructField* field, IRTypeLayoutRules* rules)
    {
        IRIntegerValue offset = 0;
        if (rules)
        {
            Slang::getOffset(
                *compilerOptions,
                rules,
                field,
                &offset);
        }
        else
        {
            auto structType = as<IRStructType>(field->getParent());
            UInt index = 0;
            for (auto ff : structType->getFields())
            {
                if(ff == field)
                    break;
                index++;
            }
            auto llvmType = llvm::cast<llvm::StructType>(getStorageType(structType, rules));
            const llvm::StructLayout* llvmStructLayout = targetDataLayout.getStructLayout(llvmType);
            offset = llvmStructLayout->getElementOffset(index);
        }
        return offset;
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
                    legalizedType = builder.getArrayType(legalizedElemType, arrayType->getElementCount());
            }
            break;
        default:
            break;
        }

        legalizedResourceTypeMap[type] = legalizedType;
        return legalizedType;
    }

    // Use this instead of the regular Slang::getSizeAndAlignment(), it handles querying
    // LLVM's own layout as well if 'rules' is nullptr.
    IRSizeAndAlignment getSizeAndAlignment(IRType* type, IRTypeLayoutRules* rules)
    {
        IRSizeAndAlignment elementSizeAlignment;

        if (rules)
        {
            Slang::getSizeAndAlignment(
                *compilerOptions,
                rules,
                legalizeResourceTypes(type),
                &elementSizeAlignment);
        }
        else
        {
            auto llvmType = getStorageType(type, rules);
            elementSizeAlignment.alignment = targetDataLayout.getABITypeAlign(llvmType).value();
            elementSizeAlignment.size = targetDataLayout.getTypeStoreSize(llvmType);
        }

        return elementSizeAlignment;
    }

    // Allocates stack memory for `getStorageType(type, rules)`. Returns a
    // pointer to the start of that memory.
    llvm::Value* emitAlloca(IRType* type, IRTypeLayoutRules* rules, size_t count = 1)
    {
        auto llvmType = getStorageType(type, rules);
        IRSizeAndAlignment sizeAlign = getSizeAndAlignment(type, rules);

        return builder->Insert(new llvm::AllocaInst(
            llvmType, 0, builder->getInt32(count), llvm::Align(sizeAlign.alignment)
        ));
    }

    // llvmVal must be using `getValueType(valType)` and is stored using the
    // specified `getStorageType(valType, rules)` into llvmPtr.
    //
    // Returns the store instruction.
    llvm::Value* emitStore(
        llvm::Value* llvmPtr,
        llvm::Value* llvmVal,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false
    ){
        IRSizeAndAlignment sizeAlignment = getSizeAndAlignment(valType, rules);
        switch(valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto storageType = getStorageType(valType, rules);
                auto expanded = builder->CreateZExt(llvmVal, storageType);
                return builder->CreateAlignedStore(expanded, llvmPtr, llvm::MaybeAlign(sizeAlignment.alignment), isVolatile);
            }
            break;
        case kIROp_ArrayType:
        case kIROp_StructType:
            // Arrays and struct values are always represented with an alloca
            // pointer.
            SLANG_ASSERT(llvmVal->getType()->isPointerTy());
            if (rules != defaultPointerRules)
            {
                return crossLayoutMemCpy(llvmPtr, llvmVal, valType, rules, defaultPointerRules, isVolatile);
            }
            else
            {
                // Pointer-to-pointer copy, so generate inline memcpy.
                return builder->CreateMemCpyInline(
                    llvmPtr,
                    llvm::MaybeAlign(sizeAlignment.alignment),
                    llvmVal,
                    llvm::MaybeAlign(sizeAlignment.alignment),
                    builder->getInt32(sizeAlignment.size),
                    isVolatile
                );
            }
        default:
            return builder->CreateAlignedStore(llvmVal, llvmPtr, llvm::MaybeAlign(sizeAlignment.alignment), isVolatile);
        }
    }

    // Returns the loaded data using `getValueType(valType)` from llvmPtr, which
    // is expected to be using `getStorageType(valType, rules)`.
    //
    // Returns the load instruction (= loaded value)
    llvm::Value* emitLoad(
        llvm::Value* llvmPtr,
        IRType* valType,
        IRTypeLayoutRules* rules,
        bool isVolatile = false
    ){
        IRSizeAndAlignment elementSizeAlignment = getSizeAndAlignment(valType, rules);

        switch(valType->getOp())
        {
        case kIROp_BoolType:
            {
                // Booleans are i1 in values, but something larger in memory.
                auto llvmType = getValueType(valType);
                auto storageType = getStorageType(valType, rules);
                auto storageBool = builder->CreateAlignedLoad(
                    storageType,
                    llvmPtr,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    isVolatile);

                return builder->CreateTrunc(storageBool, llvmType);
            }
            break;
        case kIROp_ArrayType:
        case kIROp_StructType:
            if (rules != defaultPointerRules)
            {
                llvm::Value* llvmVar = emitAlloca(valType, rules);
                crossLayoutMemCpy(llvmVar, llvmPtr, valType, defaultPointerRules, rules, isVolatile);
                return llvmVar;
            }
            else
            {
                llvm::Value* llvmVar = emitAlloca(valType, rules);

                // Pointer-to-pointer copy, so generate inline memcpy.
                builder->CreateMemCpyInline(
                    llvmVar,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    llvmPtr,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    builder->getInt32(elementSizeAlignment.size),
                    isVolatile
                );
                return llvmVar;
            }
        default:
            {
                auto llvmType = getValueType(valType);
                return builder->CreateAlignedLoad(
                    llvmType,
                    llvmPtr,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    isVolatile);
            }
        }
    }

    // Uses `getStorageType(valType, rules)` to compute an offset pointer with
    // `indexInst` based on `llvmPtr`. `elementType` is set to the Slang IR type
    // that corresponds to the selected element.
    llvm::Value* emitGetElementPtr(
        llvm::Value* llvmPtr,
        llvm::Value* indexInst,
        IRType* valType,
        IRTypeLayoutRules* rules,
        IRType*& elementType
    ){
        llvm::Type* llvmType = getStorageType(valType, rules);
        if(auto structType = as<IRStructType>(valType))
        {
            // 'index' must be constant for struct types.
            int64_t index = 0;
            if (auto intLit = llvm::cast<llvm::ConstantInt>(indexInst))
            {
                index = intLit->getValue().getLimitedValue();
            }
            else
            {
                SLANG_ASSERT_FAILURE("GetElement on structs only supports constant indices on LLVM");
            }

            auto& storageInfo = storageTypeMap.getValue(rules).getValue(structType);

            elementType = findStructField(structType, index)->getFieldType();
            UInt llvmIndex = storageInfo.fieldIndexToLLVM[index];

            llvm::Value* indices[2] = {
                builder->getInt32(0),
                builder->getInt32(llvmIndex),
            };
            return builder->CreateGEP(llvmType, llvmPtr, indices);
        }
        else if(auto arrayType = as<IRArrayTypeBase>(valType))
        {
            elementType = arrayType->getElementType();
        }
        else if(auto vectorType = as<IRVectorType>(valType))
        {
            elementType = vectorType->getElementType();
        }
        else
        {
            SLANG_ASSERT_FAILURE("Unhandled type for GetElementPtr!");
        }

        llvm::Value* indices[2] = {
            builder->getInt32(0),
            indexInst
        };
        return builder->CreateGEP(llvmType, llvmPtr, indices);
    }

    // Tries to emit the given constant, in a way which is compatible with
    // `getStorageType(valType, defaultPointerRules)`.
    llvm::Constant* maybeEmitConstant(IRInst* inst, bool storage = false, bool withTrailingPadding = true)
    {
        if (constantMap.containsKey(inst) && !storage)
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
                {
                    llvmConstant = llvm::ConstantInt::get(getValueType(type), litInst->value.intVal);
                }
                break;
            }

        case kIROp_FloatLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                {
                    llvmConstant = llvm::ConstantFP::get(getValueType(type), litInst->value.floatVal);
                }
            }
            break;

        case kIROp_StringLit:
            llvmConstant = builder->CreateGlobalString(getStringLitAsLLVMString(inst));
            break;

        case kIROp_PtrLit:
            {
                auto ptrLit = static_cast<IRPtrLit*>(inst);
                IRPtrType* type = as<IRPtrType>(inst->getDataType());
                auto llvmType = llvm::cast<llvm::PointerType>(getValueType(type));
                if (ptrLit->getValue() == nullptr)
                {
                    llvmConstant = llvm::ConstantPointerNull::get(llvmType);
                }
                else
                {
                    llvmConstant = llvm::ConstantExpr::getIntToPtr(
                        llvm::ConstantInt::get(
#if SLANG_PTR_IS_64
                            builder->getInt64Ty(),
#else
                            builder->getInt32Ty(),
#endif
                            ((uintptr_t)ptrLit->getValue())),
                        llvmType
                    );
                }
            }
            break;
        case kIROp_MakeVector:
            {
                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto partInst = inst->getOperand(aa);
                    auto constVal = maybeEmitConstant(partInst, storage);
                    if (!constVal)
                        return nullptr;
                    if (auto subvectorType = as<IRVectorType>(partInst->getDataType()))
                    {
                        auto elemCount = getIntVal(subvectorType->getElementCount());
                        for (IRIntegerValue j = 0; j < elemCount; ++j)
                        {
                            auto index = llvm::ConstantInt::get(builder->getInt32Ty(), j);
                            values.add(llvm::ConstantExpr::getExtractElement(constVal, index));
                        }
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

                if (defaultPointerRules && storage)
                {
                    // To remove padding and alignment requirements from the
                    // vector, lower it as an array.
                    auto elemType = getValueType(vectorType->getElementType());
                    llvmConstant = llvm::ConstantArray::get(
                        llvm::ArrayType::get(elemType, values.getCount()),
                        llvm::ArrayRef(values.begin(), values.end()));

                    int alignedCount = getVectorAlignedCount(vectorType, defaultPointerRules);
                    if (alignedCount != values.getCount())
                    {
                        // Fill padding with poison, nobody should use it in
                        // computations.
                        while (values.getCount() < alignedCount)
                            values.add(llvm::PoisonValue::get(elemType));

                        llvmConstant.padded = llvm::ConstantArray::get(
                            llvm::ArrayType::get(elemType, values.getCount()),
                            llvm::ArrayRef(values.begin(), values.end()));
                    }
                }
                else
                {
                    llvmConstant = llvm::ConstantVector::get(
                        llvm::ArrayRef(values.begin(), values.end()));
                }
            }
            break;
        case kIROp_MakeVectorFromScalar:
            {
                auto vectorType = cast<IRVectorType>(inst->getDataType());
                int elemCount = getIntVal(vectorType->getElementCount());
                llvm::Constant* value = maybeEmitConstant(inst->getOperand(0), storage);
                if (!value)
                    return nullptr;

                if (defaultPointerRules && storage)
                {
                    auto elemType = getValueType(vectorType->getElementType());
                    auto values = List<llvm::Constant*>::makeRepeated(value, elemCount);
                    llvmConstant = llvm::ConstantArray::get(
                        llvm::ArrayType::get(elemType, values.getCount()),
                        llvm::ArrayRef(values.begin(), values.end()));

                    int alignedCount = getVectorAlignedCount(vectorType, defaultPointerRules);
                    if (alignedCount != values.getCount())
                    {
                        while (values.getCount() < alignedCount)
                            values.add(llvm::PoisonValue::get(elemType));

                        llvmConstant.padded = llvm::ConstantArray::get(
                            llvm::ArrayType::get(elemType, values.getCount()),
                            llvm::ArrayRef(values.begin(), values.end()));
                    }
                }
                else
                {
                    llvmConstant = llvm::ConstantVector::getSplat(
                        llvm::ElementCount::getFixed(elemCount),
                        value);
                }
            }
            break;
            // It is possible to remove both MakeArray and MakeStruct here;
            // that only causes more complex code for initializing global
            // variables.
        case kIROp_MakeArray:
            {
                auto arrayType = cast<IRArrayTypeBase>(inst->getDataType());
                List<llvm::Constant*> values;

                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa), true, true);
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }

                auto llvmPaddedType = getStorageType(arrayType, defaultPointerRules, true);
                llvmConstant = llvm::ConstantArray::get(
                    llvm::cast<llvm::ArrayType>(llvmPaddedType),
                    llvm::ArrayRef(values.begin(), values.end()));

                if (needsUnpaddedArrayTypeWorkaround(arrayType, defaultPointerRules))
                {
                    // This should be a struct for the workaround.
                    auto llvmType = getStorageType(arrayType, defaultPointerRules, false);
                    SLANG_ASSERT(llvmType->isStructTy());
                    auto paddedElemType = getStorageType(arrayType->getElementType(), defaultPointerRules, true);

                    auto initPart = llvm::ConstantArray::get(
                        llvm::ArrayType::get(paddedElemType, values.getCount()-1),
                        llvm::ArrayRef(values.begin(), values.getCount()-1));

                    auto lastPart = maybeEmitConstant(inst->getOperand(inst->getOperandCount()-1), true, false);

                    llvm::Constant* parts[2] = {initPart, lastPart};

                    llvmConstant.unpadded = llvm::ConstantStruct::get(
                        llvm::cast<llvm::StructType>(llvmType), parts
                    );
                }
            }
            break;
        case kIROp_MakeStruct:
            if (defaultPointerRules)
            {
                // The struct is packed, so we need to insert padding manually.
                auto structType = cast<IRStructType>(inst->getDataType());
                auto llvmPaddedType = getStorageType(structType, defaultPointerRules, true);
                auto llvmUnpaddedType = getStorageType(structType, defaultPointerRules, false);

                auto& storageInfo = storageTypeMap.getValue(defaultPointerRules).getValue(structType);

                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa), true, false);
                    if (!constVal)
                        return nullptr;

                    UInt entryIndex = storageInfo.fieldIndexToLLVM[aa];

                    // Add padding entries until we get to the current field.
                    while (UInt(values.getCount()) < entryIndex)
                    {
                        UInt padSize = storageInfo.sizePerLLVMField[values.getCount()];
                        values.add(getZeroPaddingConstant(padSize));
                    }

                    values.add(constVal);
                }

                llvmConstant.unpadded = llvm::ConstantStruct::get(
                    llvm::cast<llvm::StructType>(llvmUnpaddedType),
                    llvm::ArrayRef(values.begin(), values.end()));

                if (storageInfo.trailingPadding)
                    values.add(getZeroPaddingConstant(storageInfo.trailingPadding));

                llvmConstant.padded = llvm::ConstantStruct::get(
                    llvm::cast<llvm::StructType>(llvmPaddedType),
                    llvm::ArrayRef(values.begin(), values.end()));
            }
            else
            {
                // The struct is not packed, we can just make the struct the
                // way LLVM wants.
                auto llvmType = getStorageType(inst->getDataType(), defaultPointerRules);
                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEmitConstant(inst->getOperand(aa));
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }
                llvmConstant = llvm::ConstantStruct::get(
                    llvm::cast<llvm::StructType>(llvmType),
                    llvm::ArrayRef(values.begin(), values.end()));
            }
            break;
        default:
            break;
        }

        if ((llvmConstant.padded || llvmConstant.unpadded) && !storage)
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

    // Returns the index of the given struct field in the corresponding LLVM
    // struct. There may be differences due to added padding fields.
    UInt mapFieldIndexToLLVM(IRStructType* irStruct, IRTypeLayoutRules* rules, UInt irIndex)
    {
        if (rules)
        {
            // `storageTypeMap` is filled out through getStorageType(), so we
            // have to call it first even though we don't care about the result
            // here.
            getStorageType(irStruct, rules);
            return storageTypeMap.getValue(rules).getValue(irStruct).fieldIndexToLLVM[irIndex];
        }
        else
        {
            return irIndex;
        }
    }

private:
    // Copies data from a pointer to another, taking layout differences into
    // account.
    llvm::Value* crossLayoutMemCpy(
        llvm::Value* dstPtr,
        llvm::Value* srcPtr,
        IRType* type,
        IRTypeLayoutRules* dstLayout,
        IRTypeLayoutRules* srcLayout,
        bool isVolatile
    ){
        IRSizeAndAlignment dstSizeAlignment = getSizeAndAlignment(type, dstLayout);
        IRSizeAndAlignment srcSizeAlignment = getSizeAndAlignment(type, srcLayout);

        int minSize = std::min(dstSizeAlignment.size, srcSizeAlignment.size);

        switch(type->getOp())
        {
        case kIROp_ArrayType:
            {
                auto arrayType = as<IRArrayTypeBase>(type);
                auto elemType = arrayType->getElementType();
                auto irElemCount = arrayType->getElementCount();
                auto elemCount = getIntVal(irElemCount);

                llvm::Value* last = nullptr;
                for (int elem = 0; elem < elemCount; ++elem)
                {
                    IRType* dummy;
                    auto dstElemPtr = emitGetElementPtr(dstPtr, builder->getInt32(elem), type, dstLayout, dummy);
                    auto srcElemPtr = emitGetElementPtr(srcPtr, builder->getInt32(elem), type, srcLayout, dummy);
                    last = crossLayoutMemCpy(
                        dstElemPtr,
                        srcElemPtr,
                        elemType,
                        dstLayout,
                        srcLayout,
                        isVolatile
                    );
                }
                return last;
            }
            break;
        case kIROp_StructType:
            {
                auto structType = as<IRStructType>(type);

                llvm::Value* last = nullptr;
                UInt fieldIndex = 0;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;

                    IRType* dummy;
                    UInt dstFieldIndex = mapFieldIndexToLLVM(structType, dstLayout, fieldIndex);
                    UInt srcFieldIndex = mapFieldIndexToLLVM(structType, srcLayout, fieldIndex);
                    auto dstElemPtr = emitGetElementPtr(dstPtr, builder->getInt32(dstFieldIndex), type, dstLayout, dummy);
                    auto srcElemPtr = emitGetElementPtr(srcPtr, builder->getInt32(srcFieldIndex), type, srcLayout, dummy);

                    last = crossLayoutMemCpy(
                        dstElemPtr,
                        srcElemPtr,
                        fieldType,
                        dstLayout,
                        srcLayout,
                        isVolatile
                    );
                    fieldIndex++;
                }
                return last;
            }
            break;
        default:
            // Assume that there are no substantial layout differences (aside
            // from trailing padding, which is skipped via minSize), so copying
            // is safe.
            return builder->CreateMemCpyInline(
                dstPtr,
                llvm::MaybeAlign(dstSizeAlignment.alignment),
                srcPtr,
                llvm::MaybeAlign(srcSizeAlignment.alignment),
                builder->getInt32(minSize),
                isVolatile
            );
        }
        return nullptr;
    }

    UInt getVectorAlignedCount(IRVectorType* vecType, IRTypeLayoutRules* rules)
    {
        int elemCount = getIntVal(vecType->getElementCount());
        IRSizeAndAlignment elementAlignment;
        Slang::getSizeAndAlignment(*compilerOptions, rules, vecType->getElementType(), &elementAlignment);
        IRSizeAndAlignment vectorAlignment = rules->getVectorSizeAndAlignment(
            elementAlignment,
            elemCount
        );

        if (!rules)
        {
            auto llvmType = getValueType(vecType);
            uint64_t alignment = targetDataLayout.getABITypeAlign(llvmType).value();
            uint64_t size = targetDataLayout.getTypeStoreSize(llvmType);
            return align(size, alignment) / elementAlignment.size;
        }

        return align(vectorAlignment.size, vectorAlignment.alignment) / elementAlignment.size;
    }

    llvm::Constant* getZeroPaddingConstant(UInt size)
    {
        auto byteType = builder->getInt8Ty();
        auto type = llvm::ArrayType::get(byteType, size);
        auto zero = llvm::ConstantInt::get(byteType, 0);
        auto zeros = List<llvm::Constant*>::makeRepeated(zero, size);
        return llvm::ConstantArray::get(type, llvm::ArrayRef(zeros.begin(), zeros.end()));
    }

    UInt emitPadding(List<llvm::Type*>& fields, UInt& curSize, UInt targetSize)
    {
        if(curSize < targetSize)
        {
            UInt bytes = targetSize - curSize;
            curSize = targetSize;
            fields.add(llvm::ArrayType::get(builder->getInt8Ty(), bytes));
            return bytes;
        }
        return 0;
    }

    // This layout is not native to LLVM, so we have to generate a packed
    // struct with manual padding.
    StorageTypeInfo getExplicitLayoutStructTypeInfo(IRStructType* structType, IRTypeLayoutRules* rules)
    {
        StorageTypeInfo llvmTypeInfo;

        List<llvm::Type*> fieldTypes;

        UInt llvmSize = 0;

        IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(structType, rules);

        for (auto field : structType->getFields())
        {
            auto fieldType = field->getFieldType();
            if (as<IRVoidType>(fieldType))
                continue;

            IRIntegerValue offset = 0;
            Slang::getOffset(*compilerOptions, rules, field, &offset);

            // Insert padding until we're at the requested offset.
            UInt padding = emitPadding(fieldTypes, llvmSize, offset);
            if (padding > 0)
                llvmTypeInfo.sizePerLLVMField.add(padding);

            // Record the current llvm field index for the mapping. 
            llvmTypeInfo.fieldIndexToLLVM.add(fieldTypes.getCount());

            // We don't want structs or arrays padded up to their alignment,
            // because scalar layout may pack fields in their predecessor's
            // trailing padding.
            auto llvmFieldType = getStorageType(fieldType, rules, false);
            fieldTypes.add(llvmFieldType);

            // Let's query the size from LLVM just to be safe, although I
            // think we could use getSizeAndAlignment() too.
            UInt fieldSize = targetDataLayout.getTypeStoreSize(llvmFieldType);
            llvmSize += fieldSize;
            llvmTypeInfo.sizePerLLVMField.add(fieldSize);
        }

        UInt paddedSize = align(sizeAndAlignment.size, sizeAndAlignment.alignment);
        llvmTypeInfo.trailingPadding = paddedSize - llvmSize;

        llvm::StructType* llvmUnpaddedStructType = llvm::StructType::get(
            builder->getContext(),
            llvm::ArrayRef<llvm::Type*>(fieldTypes.getBuffer(), fieldTypes.getCount()),
            true
        );
        llvm::StructType* llvmPaddedStructType = llvmUnpaddedStructType;

        // If trailing padding is desired, insert it.
        if (sizeAndAlignment.size != IRIntegerValue(paddedSize))
        {
            emitPadding(fieldTypes, llvmSize, paddedSize);
            llvmPaddedStructType = llvm::StructType::get(
                builder->getContext(),
                llvm::ArrayRef<llvm::Type*>(fieldTypes.getBuffer(), fieldTypes.getCount()),
                true
            );
        }

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, structType))
        {
            llvmPaddedStructType->setName(linkageName);
            llvmUnpaddedStructType->setName(linkageName);
        }

        llvmTypeInfo.padded = llvmPaddedStructType;
        llvmTypeInfo.unpadded = llvmUnpaddedStructType;

        return llvmTypeInfo;
    }

    // "Native" layout, so just dumps the fields in an unpacked LLVM aggregate.
    StorageTypeInfo getNativeStructTypeInfo(IRStructType* structType)
    {
        StorageTypeInfo llvmTypeInfo;
        List<llvm::Type*> fieldTypes;

        for (auto field : structType->getFields())
        {
            auto fieldType = field->getFieldType();
            if (as<IRVoidType>(fieldType))
                continue;
            llvmTypeInfo.fieldIndexToLLVM.add(fieldTypes.getCount());
            llvm::Type* llvmFieldType = getStorageType(fieldType, nullptr);
            UInt fieldSize = targetDataLayout.getTypeStoreSize(llvmFieldType);
            llvmTypeInfo.sizePerLLVMField.add(fieldSize);
            fieldTypes.add(llvmFieldType);
        }
        auto llvmStructType = llvm::StructType::get(
            builder->getContext(),
            llvm::ArrayRef<llvm::Type*>(fieldTypes.getBuffer(), fieldTypes.getCount()),
            false
        );

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, structType))
            llvmStructType->setName(linkageName);

        llvmTypeInfo = llvmStructType;
        return llvmTypeInfo;
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
        // Native layout never needs the workaround.
        if (!rules)
            return false;

        auto irElemCount = arrayType->getElementCount();
        auto elemCount = int(getIntVal(irElemCount));
        IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(arrayType->getElementType(), rules);

        bool hasTrailingPadding = sizeAndAlignment.size != align(sizeAndAlignment.size, sizeAndAlignment.alignment);
        return elemCount != 0 && hasTrailingPadding;
    }

    StorageTypeInfo getArrayTypeInfo(IRArrayTypeBase* arrayType, IRTypeLayoutRules* rules)
    {
        auto elemType = arrayType->getElementType();
        llvm::Type* llvmPaddedElemType = getStorageType(elemType, rules, true);
        llvm::Type* llvmUnpaddedElemType = getStorageType(elemType, rules, false);

        auto irElemCount = arrayType->getElementCount();

        StorageTypeInfo llvmTypeInfo;

        if (irElemCount == nullptr)
        {
            // UnsizedArrayType. Lowers as a zero-sized array for LLVM, luckily.
            llvmTypeInfo = llvm::ArrayType::get(llvmPaddedElemType, 0);
            return llvmTypeInfo;
        }

        auto elemCount = int(getIntVal(irElemCount));

        llvmTypeInfo = llvm::ArrayType::get(llvmPaddedElemType, elemCount);

        // The stride of an array must be based on the element type with
        // trailing padding included, but the last entry in that array may be
        // missing trailing padding in some layouts. To allow that in LLVM when
        // an unpadded array is needed, we pop the last element from the
        // array and emit an unpadded instance separately after the array.
        //
        // Unpadded types are never accessed directly, they're only ever
        // members in a struct that get interpreted as their padded versions
        // when accessed. Their size must be correct so that GEP works correctly
        // on that outer struct, but other than that, it doesn't matter what we
        // emit. 
        //
        // We can therefore pick any representation that is convenient to
        // fill in maybeEmitConstant, as that's the only context where these
        // are used as value types.
        if (needsUnpaddedArrayTypeWorkaround(arrayType, rules))
        {
            llvm::Type* fieldTypes[2] = {
                llvm::ArrayType::get(llvmPaddedElemType, elemCount-1),
                llvmUnpaddedElemType
            };
            llvmTypeInfo.unpadded = llvm::StructType::get(builder->getContext(), fieldTypes, true);
            llvmTypeInfo.trailingPadding = storageTypeMap.getValue(rules).getValue(elemType).trailingPadding;
        }
        return llvmTypeInfo;
    }
};

struct LLVMEmitter
{
    std::unique_ptr<llvm::LLVMContext> llvmContext;
    std::unique_ptr<llvm::Module> llvmModule;
    std::unique_ptr<llvm::Linker> llvmLinker;
    std::unique_ptr<llvm::IRBuilderBase> llvmBuilder;
    std::unique_ptr<llvm::DIBuilder> llvmDebugBuilder;
    std::unique_ptr<LLVMTypeTranslator> types;
    llvm::DICompileUnit* compileUnit;
    llvm::TargetMachine* targetMachine;
    CodeGenContext* codeGenContext;

    bool debug = false;
    bool inlineGlobalInstructions = false;
    IRTypeLayoutRules* defaultPointerRules = nullptr;

    // The LLVM value class is closest to Slang's IRInst, as it can represent
    // constants, instructions and functions, whereas llvm::Instruction only
    // handles instructions.
    Dictionary<IRInst*, llvm::Value*> mapInstToLLVM;

    // Global instructions that have been promoted into global variables. That
    // happens when they get referenced. Later on, constructors for these must
    // be generated.
    Dictionary<IRInst*, llvm::GlobalVariable*> promotedGlobalInsts;

    List<llvm::Constant*> globalCtors;
    llvm::StructType* llvmCtorType;

    // Map of DebugSource instructions to LLVM debug files
    Dictionary<IRInst*, llvm::DIFile*> sourceDebugInfo;
    Dictionary<IRInst*, llvm::DISubprogram*> funcToDebugLLVM;

    struct VariableDebugInfo
    {
        llvm::DIVariable* debugVar;
        // attached = first related DebugValue has been processed
        bool attached = false;
        // isStackVar = has been alloca'd and declared as debugValue. Doesn't
        // necessarily need further DebugValue tracking after that, since a
        // real variable now actually exists in LLVM.
        bool isStackVar = false;
    };
    Dictionary<IRInst*, VariableDebugInfo> variableDebugInfoMap;

    // Used to skip some instructions whose value is derived from DebugVar.
    HashSet<IRInst*> debugInsts;

    llvm::DILocalScope* currentLocalScope = nullptr;
    bool debugInlinedScope = false;
    UInt uniqueIDCounter = 0;

    // Used to add code in the entry block of a function
    using FuncPrologueCallback = Func<void>;

    // Used to add code in in front of return in a function. If it returns
    // nullptr, the LLVM return instruction is generated "normally". Otherwise,
    // it's expected that you generated it in this function.
    using FuncEpilogueCallback = Func<llvm::Value*, IRReturn*>;

    LLVMEmitter(CodeGenContext* codeGenContext, bool useJIT = false)
        : codeGenContext(codeGenContext)
    {
        llvmContext.reset(new llvm::LLVMContext());
        llvmModule.reset(new llvm::Module("module", *llvmContext));
        llvmLinker.reset(new llvm::Linker(*llvmModule));
        llvmDebugBuilder.reset(new llvm::DIBuilder(*llvmModule));

        auto session = codeGenContext->getSession();

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        if (getOptions().getOptimizationLevel() == OptimizationLevel::None)
        {
            // The default IR builder has a built-in constant folder; for
            // absolutely zero optimization, we need to disable it like this.
            llvmBuilder.reset(new llvm::IRBuilder<llvm::NoFolder>(*llvmContext));
        }
        else
        {
            llvmBuilder.reset(new llvm::IRBuilder<>(*llvmContext));
        }

        // Functions that initialize global variables are "constructors" in
        // LLVM; they're called automatically at the start of the program and
        // need to be recorded with this type.
        llvmCtorType = llvm::StructType::get(
            llvmBuilder->getInt32Ty(),
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0)
        );

        // TODO: Should probably complain here if the target machine's pointer
        // size doesn't match SLANG_PTR_IS_32 & SLANG_PTR_IS_64. Although, I'd
        // rather just fix the whole pointer size mechanism in Slang.
        auto targetTripleOption = getOptions().getStringOption(CompilerOptionName::LLVMTargetTriple).getUnownedSlice();
        std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
        if (targetTripleOption.getLength() != 0 && !useJIT)
            targetTripleStr = std::string(targetTripleOption.begin(), targetTripleOption.getLength());

        std::string error;
        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTripleStr, error);
        if (!target)
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::unrecognizedTargetTriple, targetTripleStr.c_str(), error.c_str());
            return;
        }

        llvm::TargetOptions opt;

        opt.setFPDenormalMode(getLLVMDenormalMode(getOptions().getDenormalModeFp64()));
        opt.setFP32DenormalMode(getLLVMDenormalMode(getOptions().getDenormalModeFp32()));

        opt.NoTrappingFPMath = 1;
        switch(getOptions().getFloatingPointMode())
        {
        default:
        case FloatingPointMode::Default:
            break;
        case FloatingPointMode::Fast:
            opt.UnsafeFPMath = 1;
            opt.NoSignedZerosFPMath = 1;
            opt.ApproxFuncFPMath = 1;
            break;
        case FloatingPointMode::Precise:
            opt.UnsafeFPMath = 0;
            opt.NoInfsFPMath = 0;
            opt.NoNaNsFPMath = 0;
            opt.NoSignedZerosFPMath = 0;
            opt.ApproxFuncFPMath = 0;
            break;
        }

        llvm::CodeGenOptLevel optLevel = llvm::CodeGenOptLevel::None;
        switch(getOptions().getOptimizationLevel())
        {
        case OptimizationLevel::None:
            optLevel = llvm::CodeGenOptLevel::None;
            break;
        default:
        case OptimizationLevel::Default:
            optLevel = llvm::CodeGenOptLevel::Less;
            break;
        case OptimizationLevel::High:
            optLevel = llvm::CodeGenOptLevel::Default;
            break;
        case OptimizationLevel::Maximal:
            optLevel = llvm::CodeGenOptLevel::Aggressive;
            break;
        }

        llvm::Triple targetTriple(targetTripleStr);
        targetMachine = target->createTargetMachine(
            targetTriple, "generic", "", opt, llvm::Reloc::PIC_, std::nullopt,
            optLevel
        );
        llvmModule->setDataLayout(targetMachine->createDataLayout());
        llvmModule->setTargetTriple(targetTriple);

        debug = getOptions().getDebugInfoLevel() != DebugInfoLevel::None;

        if (debug)
        {
            llvmModule->addModuleFlag(
                llvm::Module::Warning,
                "Debug Info Version",
                 llvm::DEBUG_METADATA_VERSION
            );

            StringBuilder sb;
            getOptions().writeCommandLineArgs(codeGenContext->getSession(), sb);

            auto params = sb.toString();

            // TODO: Separate debug info - that'll need to use the SplitName
            // parameter!
            compileUnit = llvmDebugBuilder->createCompileUnit(
                // TODO: We should probably apply for a language ID in DWARF? Not
                // sure how that process goes. Anyway, let's just use C++ as the
                // ID until this target is properly usable. C doesn't work
                // because debuggers won't use the un-mangled name in that lang.
                llvm::dwarf::DW_LANG_C_plus_plus,

                // The "compile unit" model doesn't work super well for Slang -
                // we're supposed to only have one for the debug info per output
                // file, as if we were building each .slang-module into a separate
                // `.o`. Which we can't really do without losing large swathes of
                // language features.
                //
                // Anyway, for now, we'll give no name to the "compile unit" and
                // just operate with individual files separately. This doesn't
                // prevent the debugger from seeing our source in any way.
                llvmDebugBuilder->createFile("<slang-llvm>", "."),

                "Slang compiler",

                getOptions().getOptimizationLevel() != OptimizationLevel::None,

                llvm::StringRef(params.begin(), params.getLength()),

                0
            );
        }

        if (getOptions().shouldUseCLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::C);
        else if (getOptions().shouldUseScalarLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::Scalar);
        else if (getOptions().shouldUseDXLayout())
            defaultPointerRules = IRTypeLayoutRules::get(IRTypeLayoutRuleName::D3DConstantBuffer);

        types.reset(new LLVMTypeTranslator(
            *llvmBuilder,
            *llvmDebugBuilder,
            getOptions(),
            targetMachine->createDataLayout(),
            compileUnit,
            sourceDebugInfo,
            defaultPointerRules));

        String prelude = session->getPreludeForLanguage(SourceLanguage::LLVM);
        llvm::ModuleSummaryIndex llvmSummaryIndex(true);
        llvm::SMDiagnostic diag;
        llvm::MemoryBufferRef buf(
            llvm::StringRef(prelude.begin(), prelude.getLength()),
            llvm::StringRef("prelude")
        );
        if(llvm::parseAssemblyInto(buf, llvmModule.get(), &llvmSummaryIndex, diag))
        {
            //auto msg = diag.getMessage();
            //printf("%s\n", msg.str().c_str());
            SLANG_UNEXPECTED("Failed to parse LLVM prelude!");
        }
    }

    ~LLVMEmitter()
    {
        delete targetMachine;
    }

    CompilerOptionSet& getOptions()
    {
        return codeGenContext->getTargetProgram()->getOptionSet();
    }

    llvm::DenormalMode getLLVMDenormalMode(FloatingPointDenormalMode mode)
    {
        switch(mode)
        {
        default:
        case FloatingPointDenormalMode::Any:
            return llvm::DenormalMode::getDefault();
        case FloatingPointDenormalMode::Preserve:
            return llvm::DenormalMode::getPreserveSign();
        case FloatingPointDenormalMode::FlushToZero:
            return llvm::DenormalMode::getPositiveZero();
        }
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
            // than global vars.
            llvmValue = types->maybeEmitConstant(inst);
            break;
        case kIROp_Specialize:
            {
                auto s = as<IRSpecialize>(inst);
                auto g = s->getBase();
                auto e = "Specialize instruction remains in IR for LLVM emit, is something undefined?\n" + dumpIRToString(g);
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
                auto llvmType = types->getStorageType(inst->getDataType(), defaultPointerRules);
                if (promotedGlobalInsts.containsKey(inst))
                {
                    llvmVar = promotedGlobalInsts.getValue(inst);
                }
                else
                {
                    llvmVar = new llvm::GlobalVariable(llvmType, false, llvm::GlobalValue::PrivateLinkage);
                    llvmModule->insertGlobalVariable(llvmVar);
                    llvmVar->setInitializer(llvm::PoisonValue::get(llvmType));
                    promotedGlobalInsts[inst] = llvmVar;
                }

                // Aggregates are passed around as pointers.
                if (llvmType->isAggregateType())
                    return llvmVar;
                // Otherwise, we'll need a load instruction to get the value.
                return llvmBuilder->CreateLoad(llvmType, llvmVar, false);
            }
            else
            {
                SLANG_UNEXPECTED(
                    "Unsupported value type for LLVM target, or referring to an "
                    "instruction that hasn't been emitted yet!");
            }
            break;
        }

        SLANG_ASSERT(llvmValue);

        if(llvmValue)
            mapInstToLLVM[inst] = llvmValue;
        return llvmValue;
    }

    void getUnderlyingTypeInfo(IRInst* inst, bool& isFloat, bool& isSigned)
    {
        IRType* elementType = getVectorOrCoopMatrixElementType(inst->getOperand(0)->getDataType());
        IRBasicType* basicType = as<IRBasicType>(elementType);
        isFloat = isFloatingType(basicType);
        isSigned = isSignedType(basicType);
    }

    llvm::Value* _coerceNumeric(llvm::Value* val, llvm::Type* type, bool isSigned)
    {
        auto valType = val->getType();
        auto valWidth = valType->getPrimitiveSizeInBits();
        auto targetWidth = type->getPrimitiveSizeInBits();

        if (type->isVectorTy() && !valType->isVectorTy())
        {
            auto vecType = llvm::cast<llvm::VectorType>(type);
            // Splat into vector
            return llvmBuilder->CreateVectorSplat(
                vecType->getElementCount(),
                _coerceNumeric(val, vecType->getElementType(), isSigned));
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
                return isSigned ? llvmBuilder->CreateSExtOrTrunc(val, type) : llvmBuilder->CreateZExtOrTrunc(val, type);
        }

        return val;
    }

    // Some operations in Slang IR may have mixed scalar and vector parameters,
    // whereas LLVM IR requires only scalars or only vectors. This function
    // helps you figure out which one it is.
    void _promoteParams(
        llvm::Value*& aValInOut, bool aIsSigned,
        llvm::Value*& bValInOut, bool bIsSigned
    ){
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
            promotedType = llvm::VectorType::get(promotedType, llvm::ElementCount::getFixed(maxElementCount));

        aValInOut = _coerceNumeric(aValInOut, promotedType, promotedSign);
        bValInOut = _coerceNumeric(bValInOut, promotedType, promotedSign);
    }

    llvm::Value* _emitCompare(IRInst* inst)
    {
        SLANG_ASSERT(inst->getOperandCount() == 2);

        IRType* aElementType = getVectorOrCoopMatrixElementType(inst->getOperand(0)->getDataType());
        IRType* bElementType = getVectorOrCoopMatrixElementType(inst->getOperand(1)->getDataType());
        bool aSigned = isSignedType(aElementType);
        bool bSigned = isSignedType(bElementType);

        auto a = findValue(inst->getOperand(0));
        auto b = findValue(inst->getOperand(1));
        _promoteParams(a, aSigned, b, bSigned);

        bool isFloat = a->getType()->getScalarType()->isFloatingPointTy();
        bool isSigned = aSigned || bSigned;

        llvm::CmpInst::Predicate pred;
        switch (inst->getOp())
        {
        case kIROp_Less:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OLT :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SLT : llvm::CmpInst::Predicate::ICMP_ULT;
            break;
        case kIROp_Leq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OLE :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SLE : llvm::CmpInst::Predicate::ICMP_ULE;
            break;
        case kIROp_Eql:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OEQ : llvm::CmpInst::Predicate::ICMP_EQ;
            break;
        case kIROp_Neq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_ONE : llvm::CmpInst::Predicate::ICMP_NE;
            break;
        case kIROp_Greater:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OGT :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SGT : llvm::CmpInst::Predicate::ICMP_UGT;
            break;
        case kIROp_Geq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OGE :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SGE : llvm::CmpInst::Predicate::ICMP_UGE;
            break;
        default:
            SLANG_UNEXPECTED("Unsupported compare op");
            break;
        }

        return llvmBuilder->CreateCmp(pred, a, b);
    }

    llvm::Value* _emitArithmetic(IRInst* inst)
    {
        auto resultType = types->getValueType(inst->getDataType());
        bool isFloat = resultType->getScalarType()->isFloatingPointTy();
        bool isSigned = isSignedType(inst->getDataType());

        if (inst->getOperandCount() == 1)
        {
            auto llvmValue = findValue(inst->getOperand(0));
            switch (inst->getOp())
            {
            case kIROp_Neg:
                return isFloat ? llvmBuilder->CreateFNeg(llvmValue) : llvmBuilder->CreateNeg(llvmValue);
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
                op = isFloat ? llvm::Instruction::FDiv : isSigned ? llvm::Instruction::SDiv : llvm::Instruction::UDiv;
                break;
            case kIROp_IRem:
                op = isSigned ? llvm::Instruction::SRem : llvm::Instruction::URem;
                break;
            case kIROp_FRem:
                op = llvm::Instruction::FRem;
                break;
            case kIROp_And:
                op = llvm::Instruction::And;
                break;
            case kIROp_Or:
                op = llvm::Instruction::Or;
                break;
            case kIROp_BitAnd:
                op = llvm::Instruction::And;
                break;
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
            auto a = _coerceNumeric(findValue(inst->getOperand(0)), resultType, isSigned);
            auto b = _coerceNumeric(findValue(inst->getOperand(1)), resultType, isSigned);

            return llvmBuilder->CreateBinOp(op, a, b);
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected number of operands for arithmetic op");
        }

        return nullptr;
    }

    void declareAllocaDebugVar(llvm::StringRef name, llvm::Value* llvmVar, IRType* type)
    {
        if (!debug)
            return;

        auto varType = types->getDebugType(type, defaultPointerRules);

        llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
        if (!loc)
            return;

        auto debugVar = llvmDebugBuilder->createAutoVariable(
            currentLocalScope, name, loc->getFile(), loc->getLine(),
            varType, getOptions().getOptimizationLevel() == OptimizationLevel::None
        );
        llvmDebugBuilder->insertDeclare(
            llvmVar,
            llvm::cast<llvm::DILocalVariable>(debugVar),
            llvmDebugBuilder->createExpression(),
            loc,
            llvmBuilder->GetInsertBlock()
        );
    }

    static llvm::Value* _defaultOnReturnHandler(IRReturn*)
    {
        SLANG_ASSERT_FAILURE("Unexpected terminator in global scope!");
        return nullptr;
    }

    IRTypeLayoutRules* getPtrLayoutRules(IRInst* ptr)
    {
        // Check if this store is actually into an RWStructuredBuffer.
        // If so, we need to take its layout into account.
        if (auto structuredBufferInst = as<IRRWStructuredBufferGetElementPtr>(ptr))
        {
            auto baseType = cast<IRHLSLStructuredBufferTypeBase>(
                structuredBufferInst->getBase()->getDataType());
            return getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), baseType);
        }
        else if (auto cbufType = as<IRConstantBufferType>(ptr->getDataType()))
        {
            return getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), cbufType);
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

    // Caution! This is only for emitting things which are considered
    // instructions in LLVM! It won't work for IRBlocks, IRFuncs & such.
    llvm::Value* emitLLVMInstruction(
        IRInst* inst,
        FuncEpilogueCallback onReturn = _defaultOnReturnHandler
    ){
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
            llvmInst = _emitCompare(inst);
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
            llvmInst = _emitArithmetic(inst);
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

                llvm::Value* llvmVar = types->emitAlloca(ptrType->getValueType(), defaultPointerRules);

                llvm::StringRef linkageName, prettyName;
                if (maybeGetName(&linkageName, &prettyName, inst))
                {
                    llvmVar->setName(linkageName);
                    // TODO: This may be a bit of a hack. DebugVar fails to get
                    // linked to the actual Var sometimes, which is why we do
                    // this :/
                    declareAllocaDebugVar(prettyName, llvmVar, ptrType->getValueType());
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
                auto trueBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getTrueBlock()));
                auto falseBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getFalseBlock()));
                auto cond = findValue(ifelseInst->getCondition());
                llvmInst = llvmBuilder->CreateCondBr(cond, trueBlock, falseBlock);
            }
            break;

        case kIROp_Switch:
            {
                auto switchInst = static_cast<IRSwitch*>(inst);
                auto defaultBlock = llvm::cast<llvm::BasicBlock>(findValue(switchInst->getBreakLabel()));
                auto llvmCondition = findValue(switchInst->getCondition());

                auto defaultLabel = switchInst->getDefaultLabel();
                if (defaultLabel)
                    defaultBlock = llvm::cast<llvm::BasicBlock>(findValue(defaultLabel));

                auto llvmSwitch = llvmBuilder->CreateSwitch(llvmCondition, defaultBlock, switchInst->getCaseCount());
                for (UInt c = 0; c < switchInst->getCaseCount(); c++)
                {
                    auto value = switchInst->getCaseValue(c);
                    auto intLit = as<IRIntLit>(value);
                    SLANG_ASSERT(intLit);

                    auto llvmCaseBlock = llvm::cast<llvm::BasicBlock>(findValue(switchInst->getCaseLabel(c)));
                    auto llvmCaseValue = llvm::cast<llvm::ConstantInt>(types->maybeEmitConstant(intLit));

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
                    findValue(selectInst->getFalseResult())
                );
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
                    isVolatile(ptr)
                );
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
                    isVolatile(ptr)
                );
            }
            break;

        case kIROp_Undefined:
            {
                auto type = inst->getDataType();
                if (types->isAggregateType(type))
                {
                    // Aggregates are always stack-allocated.
                    llvmInst = types->emitAlloca(type, defaultPointerRules);
                }
                else
                {
                    llvmInst = llvm::PoisonValue::get(types->getValueType(type));
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
        case kIROp_MakeStruct:
            {
                llvmInst = types->emitAlloca(inst->getDataType(), defaultPointerRules);
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    IRType* elementType = nullptr;
                    llvm::Value* ptr = types->emitGetElementPtr(
                        llvmInst,
                        llvmBuilder->getInt32(aa),
                        inst->getDataType(),
                        defaultPointerRules,
                        elementType);
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
                    IRType* elementType = nullptr;
                    llvm::Value* ptr = types->emitGetElementPtr(
                        llvmInst,
                        llvmBuilder->getInt32(i),
                        inst->getDataType(),
                        defaultPointerRules,
                        elementType);
                    types->emitStore(ptr, llvmElement, element->getDataType(), defaultPointerRules);
                }
            }
            break;

        case kIROp_MakeVector:
            llvmInst = types->maybeEmitConstant(inst);
            if (!llvmInst)
            {
                auto llvmType = types->getValueType(inst->getDataType());

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
                auto llvmType = llvm::cast<llvm::VectorType>(types->getValueType(inst->getDataType()));
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
                    llvmInst = llvmBuilder->CreateExtractElement(findValue(baseInst), findValue(swizzleInst->getElementIndex(0)));
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
                    llvmInst = llvmBuilder->CreateShuffleVector(findValue(baseInst), llvm::ArrayRef<int>(mask.begin(), mask.end()));
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

                for (UInt i = 0; i < swizzledInst->getElementCount(); ++i)
                {
                    IRInst* irElementIndex = swizzledInst->getElementIndex(i);
                    IRIntegerValue elementIndex = getIntVal(irElementIndex);

                    IRType* elementType;
                    auto llvmDstElement = types->emitGetElementPtr(
                        llvmDst, llvmBuilder->getInt32(elementIndex), dstType, rules, elementType);
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

                auto llvmFromType = types->getValueType(fromTypeV);
                auto llvmToType = types->getValueType(toTypeV);
                auto fromWidth = llvmFromType->getScalarSizeInBits();
                auto toWidth = llvmToType->getScalarSizeInBits();

                if (fromWidth == toWidth)
                {
                    // LLVM integers are sign-ambiguous, so if the width is the
                    // same, there's nothing to do.
                    llvmInst = llvmValue;
                }
                else if(as<IRBoolType>(toType))
                {
                    llvm::Constant* zero = llvm::ConstantInt::get(
                        types->getValueType(fromType), 0);
                    if (toTypeV != toType)
                    { // Vector
                        zero = llvm::ConstantVector::getSplat(
                            llvm::cast<llvm::VectorType>(llvmToType)->getElementCount(),
                            zero);
                    }
                    llvmInst = llvmBuilder->CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_NE,
                        llvmValue,
                        zero
                    );
                }
                else
                {
                    llvm::Instruction::CastOps cast = llvm::Instruction::CastOps::Trunc;
                    if (toWidth > fromWidth)
                        cast = getLLVMIntExtensionOp(fromType);
                    llvmInst = llvmBuilder->CreateCast(cast, llvmValue, types->getValueType(toTypeV));
                }
            }
            break;

        case kIROp_FloatCast:
            {
                auto llvmValue = findValue(inst->getOperand(0));

                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();

                auto llvmFromType = types->getValueType(fromTypeV);
                auto llvmToType = types->getValueType(toTypeV);

                auto fromSize = llvmFromType->getScalarSizeInBits();
                auto toSize = llvmToType->getScalarSizeInBits();

                if (fromSize == toSize)
                {
                    llvmInst = llvmValue;
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        fromSize < toSize ? llvm::Instruction::CastOps::FPExt : llvm::Instruction::CastOps::FPTrunc,
                        llvmValue, llvmToType
                    );
                }
            }
            break;

        case kIROp_CastIntToFloat:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    isSignedType(fromTypeV) ?
                        llvm::Instruction::CastOps::SIToFP : 
                        llvm::Instruction::CastOps::UIToFP,
                    findValue(inst->getOperand(0)),
                    types->getValueType(toTypeV)
                );
            }
            break;

        case kIROp_CastFloatToInt:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto fromType = getVectorOrCoopMatrixElementType(fromTypeV);
                auto toTypeV = inst->getDataType();
                auto toType = getVectorOrCoopMatrixElementType(toTypeV);
                auto llvmValue = findValue(inst->getOperand(0));

                if(as<IRBoolType>(toType))
                {
                    llvmInst = llvmBuilder->CreateCmp(
                        llvm::CmpInst::Predicate::FCMP_UNE,
                        llvmValue,
                        llvm::ConstantFP::getZero(types->getValueType(fromType))
                    );
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        isSignedType(toTypeV) ?
                            llvm::Instruction::CastOps::FPToSI :
                            llvm::Instruction::CastOps::FPToUI,
                        llvmValue,
                        types->getValueType(toTypeV)
                    );
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
                    types->getValueType(toTypeV)
                );
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
                    types->getValueType(toTypeV)
                );
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
                auto llvmToType = types->getValueType(toTypeV);

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
#if SLANG_PTR_IS_64
                        llvmBuilder->getInt64Ty()
#else
                        llvmBuilder->getInt32Ty()
#endif
                    );
                }
                else if (!llvmFromType->isPointerTy() && llvmToType->isPointerTy())
                {
                    // Cast from ??? to pointer, so first bitcast to equally
                    // sized int type and then do IntToPtr cast.
                    llvmFromValue = llvmBuilder->CreateCast(llvm::Instruction::CastOps::BitCast, llvmFromValue,
#if SLANG_PTR_IS_64
                        llvmBuilder->getInt64Ty()
#else
                        llvmBuilder->getInt32Ty()
#endif
                    );
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

                UInt index = getStructIndexByKey(baseStructType, key);

                index = types->mapFieldIndexToLLVM(baseStructType, rules, index);

                auto llvmStructType = types->getStorageType(baseStructType, rules);
                auto llvmBase = findValue(base);
                llvm::Value* idx[2] = {
                    llvmBuilder->getInt32(0),
                    llvmBuilder->getInt32(index)
                };

                llvmInst = llvmBuilder->CreateGEP(llvmStructType, llvmBase, idx);
            }
            break;

        case kIROp_FieldExtract:
            {
                auto fieldExtractInst = static_cast<IRFieldExtract*>(inst);
                auto structType = as<IRStructType>(fieldExtractInst->getBase()->getDataType());
                auto base = fieldExtractInst->getBase();

                if (debugInsts.contains(base))
                {
                    debugInsts.add(inst);
                    return nullptr;
                }

                auto rules = getPtrLayoutRules(base);

                auto llvmBase = findValue(base);
                unsigned idx = types->mapFieldIndexToLLVM(
                    structType,
                    rules,
                    getStructIndexByKey(structType, as<IRStructKey>(fieldExtractInst->getField())));

                IRType* elementType = nullptr;
                llvm::Value* ptr = types->emitGetElementPtr(
                    llvmBase, llvmBuilder->getInt32(idx), structType,
                    rules, elementType);

                llvmInst = types->emitLoad(ptr, elementType, rules);
            }
            break;

        case kIROp_GetOffsetPtr:
            {
                auto gopInst = static_cast<IRGetOffsetPtr*>(inst);
                auto baseInst = gopInst->getBase();
                auto offsetInst = gopInst->getOffset();

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

                llvmInst = llvmBuilder->CreateGEP(
                    types->getStorageType(baseType, getPtrLayoutRules(baseInst)),
                    findValue(baseInst), findValue(offsetInst));
            }
            break;

        case kIROp_GetElementPtr:
            {
                auto gepInst = static_cast<IRGetElementPtr*>(inst);
                auto baseInst = gepInst->getBase();
                auto indexInst = gepInst->getIndex();

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

                IRType* elementType = nullptr;
                llvmInst = types->emitGetElementPtr(
                    findValue(baseInst),
                    findValue(indexInst),
                    baseType,
                    getPtrLayoutRules(baseInst),
                    elementType
                );
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
                else if(as<IRArrayTypeBase>(baseTy) || as<IRStructType>(baseTy))
                {
                    // emitGEP + emitLoad.
                    SLANG_ASSERT(llvmVal->getType()->isPointerTy());
                    IRType* elementType = nullptr;
                    auto rules = getPtrLayoutRules(baseInst);
                    llvm::Value* ptr = types->emitGetElementPtr(
                        llvmVal, findValue(indexInst), baseTy,
                        rules, elementType);
                    llvmInst = types->emitLoad(ptr, elementType, rules);
                }
            }
            break;

        case kIROp_BitfieldExtract:
            {
                auto type = types->getValueType(inst->getDataType());
                auto val = _coerceNumeric(findValue(inst->getOperand(0)), type, false);
                auto off = _coerceNumeric(findValue(inst->getOperand(1)), type, false);
                auto bts = _coerceNumeric(findValue(inst->getOperand(2)), type, false);

                bool isFloat, isSigned;
                getUnderlyingTypeInfo(inst, isFloat, isSigned);

                auto shiftedVal = llvmBuilder->CreateLShr(
                    val, _coerceNumeric(off, val->getType(), false)
                );

                auto numBits = _coerceNumeric(llvmBuilder->getInt32(type->getScalarSizeInBits()), val->getType(), false);
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
                auto type = types->getValueType(inst->getDataType());
                auto val = _coerceNumeric(findValue(inst->getOperand(0)), type, false);
                auto insert = _coerceNumeric(findValue(inst->getOperand(1)), type, false);
                auto off = _coerceNumeric(findValue(inst->getOperand(2)), type, false);
                auto bts = _coerceNumeric(findValue(inst->getOperand(3)), type, false);

                auto one = _coerceNumeric(llvmBuilder->getInt32(1), val->getType(), false);
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

                for(IRInst* arg : callInst->getArgsList())
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
                auto returnVal = llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
                llvmInst = allocValue ? allocValue : returnVal;
            }
            break;

        case kIROp_Printf:
            {
                // This function comes from the prelude.
                auto llvmFunc = cast<llvm::Function>(llvmModule->getNamedValue("printf"));
                SLANG_ASSERT(llvmFunc);

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

                            if (valueType->isFloatingPointTy() && valueType->getScalarSizeInBits() < 64)
                            {
                                // Floats need to get up-casted to at least f64
                                llvmValue = llvmBuilder->CreateCast(
                                    llvm::Instruction::CastOps::FPExt,
                                    llvmValue,
                                    llvmBuilder->getDoubleTy()
                                );
                            }
                            else if (valueType->isIntegerTy() && valueType->getScalarSizeInBits() < 32)
                            {
                                // Ints are upcasted to at least i32.
                                llvmValue = llvmBuilder->CreateCast(
                                    getLLVMIntExtensionOp(op->getDataType()),
                                    llvmValue,
                                    llvmBuilder->getInt32Ty()
                                );
                            }
                            args.add(llvmValue);
                        }
                    }
                }

                llvmInst = llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
            }
            break;

        case kIROp_RWStructuredBufferGetElementPtr:
            {
                auto gepInst = static_cast<IRRWStructuredBufferGetElementPtr*>(inst);

                auto baseType = cast<IRHLSLRWStructuredBufferType>(gepInst->getBase()->getDataType());
                auto llvmBase = findValue(gepInst->getBase());
                auto llvmIndex = findValue(gepInst->getIndex());

                auto llvmPtr = llvmBuilder->CreateExtractValue(llvmBase, 0);

                llvm::Value* indices[] = {llvmIndex};
                IRTypeLayoutRules* layout = getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), baseType);
                llvmInst = llvmBuilder->CreateGEP(
                    types->getStorageType(baseType->getElementType(), layout),
                    llvmPtr, indices);
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
                IRTypeLayoutRules* layout = getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), baseType);

                llvm::Value* indices[] = {llvmIndex};
                auto llvmPtr = llvmBuilder->CreateGEP(
                    types->getStorageType(baseType->getElementType(), layout),
                    llvmBasePtr, indices);
                llvmInst = types->emitLoad(
                    llvmPtr,
                    inst->getDataType(),
                    layout);
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
                IRTypeLayoutRules* layout = getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), baseType);

                llvm::Value* indices[] = {llvmIndex};
                auto llvmPtr = llvmBuilder->CreateGEP(
                    types->getStorageType(baseType->getElementType(), layout),
                    llvmBasePtr, indices);
                llvmInst = types->emitStore(
                    llvmPtr,
                    findValue(val),
                    val->getDataType(),
                    layout);
            }
            break;

        case kIROp_ByteAddressBufferLoad:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                llvm::Value* indices[] = {llvmIndex};
                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                auto llvmPtr = llvmBuilder->CreateGEP(
                    llvmBuilder->getInt8Ty(),
                    llvmBasePtr,
                    indices);
                llvmInst = types->emitLoad(
                    llvmPtr,
                    inst->getDataType(),
                    defaultPointerRules);
            }
            break;

        case kIROp_ByteAddressBufferStore:
            {
                auto llvmBase = findValue(inst->getOperand(0));
                auto llvmIndex = findValue(inst->getOperand(1));

                llvm::Value* indices[] = {llvmIndex};
                auto llvmBasePtr = llvmBuilder->CreateExtractValue(llvmBase, 0);
                auto llvmPtr = llvmBuilder->CreateGEP(
                    llvmBuilder->getInt8Ty(),
                    llvmBasePtr,
                    indices);
                auto val = inst->getOperand(inst->getOperandCount() - 1);
                llvmInst = types->emitStore(
                    llvmPtr,
                    findValue(val),
                    val->getDataType(),
                    defaultPointerRules);
            }
            break;

        case kIROp_Unreachable:
            return llvmBuilder->CreateUnreachable();

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
                            currentLocalScope, prettyName, getIntVal(argIndex)+1, file,
                            line, varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None
                        ),
                        false,
                        false
                    };
                }
                else
                {
                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder->createAutoVariable(
                            currentLocalScope, prettyName, file, line, varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None
                        ),
                        false,
                        false
                    };
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
                    loc = llvm::DILocation::get(*llvmContext, debugInfo.debugVar->getLine(), 0, debugInfo.debugVar->getScope());

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
                        llvmBuilder->GetInsertBlock()
                    );
                }
                else if(!debugInfo.isStackVar)
                {
                    llvmDebugBuilder->insertDbgValueIntrinsic(
                        findValue(debugValueInst->getValue()),
                        llvm::cast<llvm::DILocalVariable>(debugInfo.debugVar),
                        llvmDebugBuilder->createExpression(),
                        loc,
                        llvmBuilder->GetInsertBlock()
                    );
                }
                debugInfo.attached = true;
            }
            return nullptr;

        case kIROp_DebugLine:
            debugInsts.add(inst);
            if (debug && currentLocalScope)
            {
                auto debugLineInst = static_cast<IRDebugLine*>(inst);

                //auto file = sourceDebugInfo.getValue(debugLineInst->getSource());
                auto line = getIntVal(debugLineInst->getLineStart());
                auto col = getIntVal(debugLineInst->getColStart());

                debugLineInst->getLineEnd();
                debugLineInst->getColStart();
                debugLineInst->getColEnd();

                llvmBuilder->SetCurrentDebugLocation(
                    llvm::DILocation::get(*llvmContext, line, col, currentLocalScope)
                );
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
        {
            return funcToDebugLLVM[func];
        }

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
            llvm::DISubprogram::SPFlagDefinition
        );
        funcToDebugLLVM[func] = sp;
        return sp;
    }

    llvm::Function* ensureFuncDecl(IRFunc* func)
    {
        if (mapInstToLLVM.containsKey(func))
            return llvm::cast<llvm::Function>(mapInstToLLVM.getValue(func));

        auto funcType = static_cast<IRFuncType*>(func->getDataType());

        llvm::FunctionType* llvmFuncType = llvm::cast<llvm::FunctionType>(types->getValueType(funcType));

        llvm::Function* llvmFunc = llvm::Function::Create(
            llvmFuncType,
            getLinkageType(func),
            "", // Name is conditionally set below.
            *llvmModule
        );

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, func))
            llvmFunc->setName(linkageName);

        if (llvmFunc->getName().size() == 0)
            llvmFunc->setName("__slang_anonymous_func_" + std::to_string(uniqueIDCounter++));

        UInt i = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
        {
            auto llvmArg = llvmFunc->getArg(i);

            if (as<IROutParamType>(funcType->getParamType(i)))
                llvmArg->addAttr(llvm::Attribute::WriteOnly);
            else if (as<IRBorrowInParamType>(funcType->getParamType(i)))
                llvmArg->addAttr(llvm::Attribute::ReadOnly);

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

    llvm::Value* ensureGlobalVarDecl(IRGlobalVar* var)
    {
        if (mapInstToLLVM.containsKey(var))
            return mapInstToLLVM.getValue(var);

        IRPtrType* ptrType = var->getDataType();
        auto varType = types->getStorageType(ptrType->getValueType(), defaultPointerRules);
        // The global vars are never emitted as constant, constants are always
        // inlined into where they're needed.
        llvm::GlobalVariable* llvmVar = new llvm::GlobalVariable(varType, false, getLinkageType(var));

        llvm::StringRef linkageName, prettyName;
        if (maybeGetName(&linkageName, &prettyName, var))
            llvmVar->setName(linkageName);

        llvmModule->insertGlobalVariable(llvmVar);
        mapInstToLLVM[var] = llvmVar;
        return llvmVar;
    }

    void emitGlobalVarCtor(IRGlobalVar* var)
    {
        llvm::GlobalVariable* llvmVar = llvm::cast<llvm::GlobalVariable>(mapInstToLLVM.getValue(var));

        auto firstBlock = var->getFirstBlock();

        if(!firstBlock)
            return;

        if (auto returnInst = as<IRReturn>(firstBlock->getTerminator()))
        {
            // If the initializer is constant, we can emit that to the
            // variable directly.
            IRInst* val = returnInst->getVal();
            if (auto constantValue = types->maybeEmitConstant(val))
            {
                // Easy case, it's just a constant in LLVM.
                llvmVar->setInitializer(constantValue);
                return;
            }
        }

        // Poison and add a global ctor.
        llvmVar->setInitializer(llvm::PoisonValue::get(llvmVar->getValueType()));

        llvm::FunctionType* ctorType = llvm::FunctionType::get(
            llvmBuilder->getVoidTy(), {}, false);

        std::string ctorName = "__slang_init_";

        llvm::StringRef linkageName, prettyName;
        maybeGetName(&linkageName, &prettyName, var);
        ctorName += linkageName;

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        llvm::Function* llvmCtor = llvm::Function::Create(
            ctorType, llvm::GlobalValue::InternalLinkage, ctorName, *llvmModule
        );

        auto prologue = [](){};
        auto epilogue = [&](IRReturn* ret) -> llvm::Value* {
            auto val = ret->getVal();
            types->emitStore(llvmVar, findValue(val), val->getDataType(), defaultPointerRules);
            return llvmBuilder->CreateRetVoid();
        };
        emitGlobalValueWithCode(var, llvmCtor, prologue, epilogue);

        llvm::Constant* ctorData[3] = {
            // Leave some room for other global constructors to run first.
            llvmBuilder->getInt32(globalCtors.getCount()+16),
            llvmCtor,
            llvmVar
        };

        llvm::Constant *ctorEntry = llvm::ConstantStruct::get(llvmCtorType,
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

                std::filesystem::path path(
                    std::string(filename.begin(), filename.getLength())
                );
                sourceDebugInfo[inst] = llvmDebugBuilder->createFile(
                    path.filename().c_str(),
                    path.parent_path().c_str(),
                    std::nullopt,
                    getStringLitAsLLVMString(debugSource->getSource())
                );
            }
        }
    }

    void emitGlobalDeclarations(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
                ensureFuncDecl(func);
            else if(auto globalVar = as<IRGlobalVar>(inst))
                ensureGlobalVarDecl(globalVar);
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
        llvmFunc->print(expanded);

        while (out.size() > 0 && out.back() != '{')
            out.pop_back();

        expanded << "\nentry:\n";

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
                case 'T':
                    // Type of parameter
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
                            UInt argIndex = parseNumber(cursor, end);
                            SLANG_RELEASE_ASSERT(argIndex < parentFunc->getParamCount());
                            type = parentFunc->getParamType(argIndex);
                        }

                        auto llvmType = types->getValueType(type);
                        llvmType->print(expanded);
                    }
                    break;
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
                        --cursor;
                        UInt argIndex = parseNumber(cursor, end);
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
                        llvmParam->print(expanded);
                    }
                    break;
                case '[':
                    {
                        UInt argIndex = parseNumber(cursor, end)+1;

                        SLANG_RELEASE_ASSERT(argIndex < intrinsicInst->getOperandCount());

                        auto arg = intrinsicInst->getOperand(argIndex);

                        auto llvmType = types->getValueType(as<IRType>(arg));
                        llvmType->print(expanded);

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

        bool hasReturnValue = as<IRVoidType>(resultType) == nullptr;
        bool resultFound =
            out.find("%result=") != std::string::npos ||
            out.find("%result =") != std::string::npos;

        if (!hasReturnValue || resultFound)
        {
            expanded << "\nret ";
            if (hasReturnValue)
            {
                auto llvmResultType = types->getValueType(resultType);
                llvmResultType->print(expanded);
                expanded << " %result";
            }
            else expanded << "void";
        }

        expanded << "\n}\n";

        return out;
    }

    IRFunc* createTargetIntrinsicFunc(IRCall* callInst)
    {
        IRBuilder builder(callInst->getModule());

        auto resultType = callInst->getDataType();
        List<IRType*> paramTypes;

        for(IRInst* arg : callInst->getArgsList())
        {
            paramTypes.add(arg->getDataType());
        }

        IRFuncType* funcType = builder.getFuncType(paramTypes.getCount(), paramTypes.begin(), resultType);
        IRFunc* func = builder.createFunc();
        func->setFullType(funcType);

        builder.setInsertInto(func);
        IRBlock* headerBlock = builder.emitBlock();
        builder.setInsertInto(headerBlock);

        for(IRInst* arg : callInst->getArgsList())
        {
            builder.emitParam(arg->getDataType());
        }
        return func;
    }

    void emitTargetIntrinsicFunction(
        IRFunc* func,
        llvm::Function*& llvmFunc,
        IRInst* intrinsicInst,
        UnownedStringSlice intrinsicDef
    ){
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);

        llvmFunc->setLinkage(llvm::Function::LinkageTypes::ExternalLinkage);

        std::string funcName = llvmFunc->getName().str();
        std::string llvmTextIR = expandIntrinsic(llvmFunc, intrinsicInst, func, intrinsicDef);

        bb->eraseFromParent();

        llvm::SMDiagnostic diag;
        std::unique_ptr<llvm::Module> sourceModule = llvm::parseAssemblyString(
            llvmTextIR, diag, *llvmContext);

        if (!sourceModule)
        {
            auto msg = diag.getMessage();
            printf("inline ir:\n%s\nerror: %s\n", llvmTextIR.c_str(), msg.str().c_str());
            SLANG_UNEXPECTED("Failed to parse LLVM inline IR!");
        }

        sourceModule->setDataLayout(targetMachine->createDataLayout());
        sourceModule->setTargetTriple(targetMachine->getTargetTriple());

        llvmLinker->linkInModule(std::move(sourceModule), llvm::Linker::OverrideFromSrc);

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
        FuncPrologueCallback prologueCallback,
        FuncEpilogueCallback epilogueCallback
    ){
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        debugInlinedScope = false;

        auto func = as<IRFunc>(code);
        bool intrinsic = false;
        if (func)
        {
            UnownedStringSlice intrinsicDef;
            IRInst* intrinsicInst;
            intrinsic = Slang::findTargetIntrinsicDefinition(func, codeGenContext->getTargetReq()->getTargetCaps(), intrinsicDef, intrinsicInst);
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
            // Create all blocks first
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);
                mapInstToLLVM[irBlock] = llvmBlock;
            }

            // Then, fill in the blocks. Lucky for us, there appears to basically be
            // a 1:1 correspondence between Slang IR blocks and LLVM IR blocks, so
            // this is straightforward.
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::cast<llvm::BasicBlock>(mapInstToLLVM.getValue(irBlock));
                llvmBuilder->SetInsertPoint(llvmBlock);

                if (irBlock == code->getFirstBlock())
                {
                    if (sp)
                    {
                        llvmBuilder->SetCurrentDebugLocation(
                            llvm::DILocation::get(*llvmContext, sp->getLine(), 0, sp)
                        );
                    }
                    prologueCallback();
                }

                // Then, add the regular instructions.
                for (auto irInst: irBlock->getOrdinaryInsts())
                {
                    auto llvmInst = emitLLVMInstruction(irInst, epilogueCallback);
                    if(llvmInst)
                        mapInstToLLVM[irInst] = llvmInst;
                }
            }
        }

        currentLocalScope = nullptr;
    }

    void emitComputeEntryPointDispatcher(
        IRFunc* entryPoint,
        llvm::Function* llvmEntryPoint,
        IREntryPointDecoration* entryPointDecor)
    {
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

        llvm::Type* uintType = llvmBuilder->getInt32Ty();

        llvm::Type* argTypes[3] = {
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0)
        };

        llvm::FunctionType* dispatcherType = llvm::FunctionType::get(
            llvmBuilder->getVoidTy(), argTypes, false);

        auto entryPointName = entryPointDecor->getName()->getStringSlice();
        llvm::Function* dispatcher = llvm::Function::Create(
            dispatcherType, llvm::GlobalValue::ExternalLinkage,
            llvm::StringRef(entryPointName.begin(), entryPointName.getLength()), *llvmModule
        );

        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", dispatcher);
        llvmBuilder->SetInsertPoint(entryBlock);

        // This has to be ABI-compatible with the C++ target. So don't go
        // changing this without a good reason to.
        llvm::StructType* varyingInputType = llvm::StructType::get(
            // startGroupID
            llvm::ArrayType::get(uintType, 3),
            // endGroupID
            llvm::ArrayType::get(uintType, 3)
        );
        // This is the data passed to the actual entry point.
        llvm::StructType* threadVaryingInputType = llvm::StructType::get(
            // groupID
            llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3)),
            // groupThreadID
            llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3))
        );

        llvm::Value* varyingInput = dispatcher->getArg(0);
        llvm::Value* threadInput = llvmBuilder->CreateAlloca(threadVaryingInputType);

        llvm::Value* startGroupID[3];
        llvm::Value* endGroupID[3];
        llvm::Value* groupID[3];
        llvm::Value* threadID[3];

        auto numThreadsDecor = entryPoint->findDecoration<IRNumThreadsDecoration>();
        llvm::Value* workGroupSize[3] = {
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getX()) : 1),
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getY()) : 1),
            llvmBuilder->getInt32(numThreadsDecor ? getIntVal(numThreadsDecor->getZ()) : 1)
        };

        for(int i = 0; i < 3; ++i)
        {
            llvm::Value* gepIndices[3] = {
                llvmBuilder->getInt32(0),
                llvmBuilder->getInt32(0), // startGroupID && groupID
                llvmBuilder->getInt32(i),
            };

            auto startGroupPtr = llvmBuilder->CreateGEP(varyingInputType, varyingInput, gepIndices);
            startGroupID[i] = llvmBuilder->CreateLoad(uintType, startGroupPtr,
                llvm::Twine("startGroupID").concat(llvm::Twine(i))
            );
            groupID[i] = llvmBuilder->CreateGEP(threadVaryingInputType, threadInput, gepIndices,
                llvm::Twine("groupID").concat(llvm::Twine(i))
            );
            // Pick endGroupID & groupThreadID
            gepIndices[1] = llvmBuilder->getInt32(1);
            auto endGroupPtr = llvmBuilder->CreateGEP(varyingInputType, varyingInput, gepIndices);
            endGroupID[i] = llvmBuilder->CreateLoad(uintType, endGroupPtr,
                llvm::Twine("endGroupID").concat(llvm::Twine(i))
            );
            threadID[i] = llvmBuilder->CreateGEP(threadVaryingInputType, threadInput, gepIndices,
                llvm::Twine("threadID").concat(llvm::Twine(i))
            );
        }
        // We need 6 nested loops... :(

        llvm::BasicBlock* wgEntryBlocks[3];
        llvm::BasicBlock* wgBodyBlocks[3];
        llvm::BasicBlock* wgEndBlocks[3];
        llvm::BasicBlock* threadEntryBlocks[3];
        llvm::BasicBlock* threadBodyBlocks[3];
        llvm::BasicBlock* threadEndBlocks[3];
        llvm::BasicBlock* endBlock;

        // Create all blocks first, so that they can jump around.
        for(int i = 0; i < 3; ++i)
        {
            wgEntryBlocks[i] = llvm::BasicBlock::Create(*llvmContext,
                llvm::Twine("groupEntry").concat(llvm::Twine(i)),
                dispatcher);
            wgBodyBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("groupBody").concat(llvm::Twine(i)),
                dispatcher);
        }
        for(int i = 0; i < 3; ++i)
        {
            threadEntryBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("threadEntry").concat(llvm::Twine(i)),
                dispatcher
            );
            threadBodyBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("threadBody").concat(llvm::Twine(i)),
                dispatcher);
        }

        for(int i = 2; i >= 0; --i)
        {
            threadEndBlocks[i] = llvm::BasicBlock::Create(*llvmContext,
                llvm::Twine("threadEnd").concat(llvm::Twine(i)),
                dispatcher);
        }
        for(int i = 2; i >= 0; --i)
        {
            wgEndBlocks[i] = llvm::BasicBlock::Create(
                *llvmContext,
                llvm::Twine("groupEnd").concat(llvm::Twine(i)),
                dispatcher);
        }
        endBlock =  llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("end"),
            dispatcher);

        // Populate group dispatch headers.
        for(int i = 0; i < 3; ++i)
        {
            llvmBuilder->CreateStore(startGroupID[i], groupID[i]);
            llvmBuilder->CreateBr(wgEntryBlocks[i]);
            llvmBuilder->SetInsertPoint(wgEntryBlocks[i]);
            auto id = llvmBuilder->CreateLoad(uintType, groupID[i]);
            auto cond = llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, endGroupID[i]);

            auto merge = i == 0 ? endBlock : wgEndBlocks[i-1];

            llvmBuilder->CreateCondBr(cond, wgBodyBlocks[i], merge);
            llvmBuilder->SetInsertPoint(wgBodyBlocks[i]);
        }

        // Populate thread dispatch headers.
        for(int i = 0; i < 3; ++i)
        {
            llvmBuilder->CreateStore(llvmBuilder->getInt32(0), threadID[i]);
            llvmBuilder->CreateBr(threadEntryBlocks[i]);
            llvmBuilder->SetInsertPoint(threadEntryBlocks[i]);

            auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
            auto cond = llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, workGroupSize[i]);

            auto merge = i == 0 ? wgEndBlocks[2] : threadEndBlocks[i-1];
            llvmBuilder->CreateCondBr(cond, threadBodyBlocks[i], merge);
            llvmBuilder->SetInsertPoint(threadBodyBlocks[i]);
        }

        // Do the call to the actual entry point function.
        llvm::Value* args[3] = {
            threadInput,
            dispatcher->getArg(1),
            dispatcher->getArg(2)
        };
        llvmBuilder->CreateCall(llvmEntryPoint, args);
        llvmBuilder->CreateBr(threadEndBlocks[2]);

        // Finish thread dispatch blocks
        for(int i = 2; i >= 0; --i)
        {
            llvmBuilder->SetInsertPoint(threadEndBlocks[i]);
            auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
            auto inc = llvmBuilder->CreateAdd(id, llvmBuilder->getInt32(1));
            llvmBuilder->CreateStore(inc, threadID[i]);
            llvmBuilder->CreateBr(threadEntryBlocks[i]);
        }

        // Finish group dispatch blocks
        for(int i = 2; i >= 0; --i)
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

        llvm::Value* storeArg = nullptr;
        if (types->isAggregateType(func->getResultType()))
            storeArg = llvmFunc->getArg(llvmFunc->arg_size()-1);

        // The prologue and epilogue are needed to deal with out and inout - the
        // correct semantics for them involve copy-in and copy-out, which we
        // implement in the prologue (emitted before first instruction in entry
        // block) and epilogue (emitted before return).
        //
        // This difference can be seen in this case:
        //
        // void evilFunction(out int a, out int b)
        // {
        //     a = 1;
        //     b = 2;
        //     // Must print "1, 2" even when a and b are given the same
        //     // argument.
        //     printf("%d, %d\n", a, b);
        // }
        //
        // void entryPoint()
        // {
        //     int num;
        //     evilFunction(num, num);
        // }
        //
        // However, this can be optimized. If the function only takes one
        // parameter that is 'out' or 'inout' and zero pointers, it cannot
        // alias, and therefore it being backed by a pointer is unobservable.

        int pointerCount = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam())
        {
            if (as<IRPtrTypeBase>(pp->getDataType()))
                pointerCount++;
        }

        auto prologue = [&](){
            if(pointerCount <= 1)
                return;
            UInt i = 0;
            for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
            {
                llvm::Value* llvmArg = llvmFunc->getArg(i);
                auto argType = pp->getDataType();

                llvm::StringRef linkageName, prettyName;
                maybeGetName(&linkageName, &prettyName, pp);

                if (auto outType = as<IROutParamType>(argType))
                {
                    // Replace with uninitialized alloca, parameter is only for
                    // copy-out!
                    llvmArg = types->emitAlloca(outType->getValueType(), defaultPointerRules);
                    declareAllocaDebugVar(prettyName, llvmArg, outType->getValueType());
                }
                else if (auto inOutType = as<IRBorrowInOutParamType>(argType))
                {
                    // Replace with initialized alloca.
                    auto newArg = types->emitAlloca(inOutType->getValueType(), defaultPointerRules);
                    IRSizeAndAlignment elementSizeAlignment = types->getSizeAndAlignment(inOutType->getValueType(), defaultPointerRules);
                    llvmBuilder->CreateMemCpyInline(
                        newArg,
                        llvm::MaybeAlign(elementSizeAlignment.alignment),
                        llvmArg,
                        llvm::MaybeAlign(elementSizeAlignment.alignment),
                        llvmBuilder->getInt32(elementSizeAlignment.size)
                    );
                    llvmArg = newArg;
                    declareAllocaDebugVar(prettyName, llvmArg, inOutType->getValueType());
                }
                mapInstToLLVM[pp] = llvmArg;
            }
        };

        auto epilogue = [&](IRReturn* ret) -> llvm::Value* {
            if (pointerCount > 1)
            {
                UInt i = 0;
                for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
                {
                    llvm::Value* llvmArg = llvmFunc->getArg(i);
                    auto argType = pp->getDataType();

                    if (as<IROutParamType>(argType) || as<IRBorrowInOutParamType>(argType))
                    {
                        auto ptrType = as<IRPtrTypeBase>(argType);
                        // Copy-out!
                        IRSizeAndAlignment elementSizeAlignment = types->getSizeAndAlignment(ptrType->getValueType(), defaultPointerRules);
                        llvmBuilder->CreateMemCpyInline(
                            llvmArg,
                            llvm::MaybeAlign(elementSizeAlignment.alignment),
                            mapInstToLLVM.getValue(pp),
                            llvm::MaybeAlign(elementSizeAlignment.alignment),
                            llvmBuilder->getInt32(elementSizeAlignment.size)
                        );
                    }
                }
            }

            if (storeArg)
            {
                auto val = ret->getVal();
                types->emitStore(storeArg, findValue(val), val->getDataType(), defaultPointerRules);
                return llvmBuilder->CreateRetVoid();
            }

            return nullptr;
        };

        emitGlobalValueWithCode(func, llvmFunc, prologue, epilogue);

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
        // won't get iterated over.
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
            else if(auto globalVar = as<IRGlobalVar>(inst))
            {
                emitGlobalVarCtor(globalVar);
            }
        }
    }

    void emitGlobalInstructionCtor()
    {
        if (promotedGlobalInsts.getCount() == 0)
            return;

        llvm::FunctionType* ctorType = llvm::FunctionType::get(
            llvmBuilder->getVoidTy(), {}, false);

        std::string ctorName = "__slang_global_insts";

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        llvm::Function* llvmCtor = llvm::Function::Create(
            ctorType, llvm::GlobalValue::InternalLinkage, ctorName, *llvmModule
        );

        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "", llvmCtor);
        llvmBuilder->SetInsertPoint(entryBlock);

        inlineGlobalInstructions = true;

        for (auto [inst, globalVar]: promotedGlobalInsts)
        {
            // See if we can lower it directly as a constant
            auto constVal = types->maybeEmitConstant(inst);
            if (constVal)
            {
                globalVar->setInitializer(constVal);
            }
            else
            {
                // Otherwise, emit the instructions needed to construct the
                // value.
                auto llvmInst = emitLLVMInstruction(inst);
                types->emitStore(globalVar, llvmInst, inst->getDataType(), defaultPointerRules);
            }
        }

        inlineGlobalInstructions = false;
        llvmBuilder->CreateRetVoid();

        llvm::Constant* ctorData[3] = {
            llvmBuilder->getInt32(0),
            llvmCtor,
            llvm::ConstantPointerNull::get(llvm::PointerType::get(*llvmContext, 0))
        };

        llvm::Constant *ctorEntry = llvm::ConstantStruct::get(llvmCtorType,
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

    // Optimizes the LLVM IR and destroys mapInstToLLVM.
    void optimize()
    {
        mapInstToLLVM.clear();

        llvm::LoopAnalysisManager loopAnalysisManager;
        llvm::FunctionAnalysisManager functionAnalysisManager;
        llvm::CGSCCAnalysisManager CGSCCAnalysisManager;
        llvm::ModuleAnalysisManager moduleAnalysisManager;

        llvm::PassBuilder passBuilder(targetMachine);
        passBuilder.registerModuleAnalyses(moduleAnalysisManager);
        passBuilder.registerCGSCCAnalyses(CGSCCAnalysisManager);
        passBuilder.registerFunctionAnalyses(functionAnalysisManager);
        passBuilder.registerLoopAnalyses(loopAnalysisManager);
        passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, CGSCCAnalysisManager, moduleAnalysisManager);

        llvm::OptimizationLevel llvmLevel = llvm::OptimizationLevel::O0;

        switch(getOptions().getOptimizationLevel())
        {
        case OptimizationLevel::None:
            llvmLevel = llvm::OptimizationLevel::O0;
            break;
        default:
        case OptimizationLevel::Default:
            llvmLevel = llvm::OptimizationLevel::O1;
            break;
        case OptimizationLevel::High:
            llvmLevel = llvm::OptimizationLevel::O2;
            break;
        case OptimizationLevel::Maximal:
            llvmLevel = llvm::OptimizationLevel::O3;
            break;
        }

        // Run the actual optimizations.
        llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvmLevel);
        modulePassManager.run(*llvmModule, moduleAnalysisManager);
    }

    void finalize()
    {
        // Dump the global constructors array
        if (globalCtors.getCount() != 0)
        {
            auto ctorArrayType = llvm::ArrayType::get(llvmCtorType, globalCtors.getCount());
            auto ctorArray = llvm::ConstantArray::get(ctorArrayType, llvm::ArrayRef(globalCtors.begin(), globalCtors.end()));
            new llvm::GlobalVariable(
                *llvmModule, ctorArrayType, false, llvm::GlobalValue::AppendingLinkage,
                ctorArray, "llvm.global_ctors"
            );
        }
        
        //std::string out;
        //llvm::raw_string_ostream rso(out);
        //llvmModule->print(rso, nullptr);
        //printf("%s\n", out.c_str());

        llvm::verifyModule(*llvmModule, &llvm::errs());

        // O0 is separately handled inside `optimize()`; we need to call it in
        // any case to make sure that `ForceInline` functions get inlined.
        optimize();

        if (debug)
        {
            llvmDebugBuilder->finalize();
        }
    }

    void dumpAssembly(String& assemblyOut)
    {
        std::string out;
        llvm::raw_string_ostream rso(out);
        llvmModule->print(rso, nullptr);
        assemblyOut = out.c_str();
    }

    void generateObjectCode(List<uint8_t>& objectOut)
    {
        BinaryLLVMOutputStream output(objectOut);

        llvm::legacy::PassManager pass;
        auto fileType = llvm::CodeGenFileType::ObjectFile;
        if (targetMachine->addPassesToEmitFile(pass, output, nullptr, fileType))
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::llvmCodegenFailed);
            return;
        }

        pass.run(*llvmModule);
    }

    SlangResult generateJITLibrary(IArtifact** outArtifact)
    {
        std::unique_ptr<llvm::orc::LLJIT> jit;
        ComPtr<IArtifactDiagnostics> diagnostics(new ArtifactDiagnostics());
        {
            llvm::orc::LLJITBuilder jitBuilder;
            llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> expectJit = jitBuilder.create();

            if (!expectJit)
            {
                auto err = expectJit.takeError();

                std::string jitErrorString;
                llvm::raw_string_ostream jitErrorStream(jitErrorString);

                jitErrorStream << err;

                ArtifactDiagnostic diagnostic;

                StringBuilder buf;
                buf << "Unable to create JIT engine: " << jitErrorString.c_str();

                diagnostic.severity = ArtifactDiagnostic::Severity::Error;
                diagnostic.stage = ArtifactDiagnostic::Stage::Link;
                diagnostic.text = TerminatedCharSlice(buf.getBuffer(), buf.getLength());

                // Add the error
                diagnostics->add(diagnostic);
                diagnostics->setResult(SLANG_FAIL);

                auto artifact = ArtifactUtil::createArtifact(
                    ArtifactDesc::make(ArtifactKind::None, ArtifactPayload::None));
                ArtifactUtil::addAssociated(artifact, diagnostics);

                *outArtifact = artifact.detach();
                return SLANG_OK;
            }
            jit = std::move(*expectJit);
        }
        llvm::orc::ThreadSafeModule threadSafeModule(
            std::move(llvmModule), std::move(llvmContext));

        if (auto err = jit->addIRModule(std::move(threadSafeModule)))
        {
            return SLANG_FAIL;
        }

        // Create the shared library
        ComPtr<ISlangSharedLibrary> sharedLibrary(new LLVMJITSharedLibrary2(std::move(jit)));

        const auto targetDesc = ArtifactDescUtil::makeDescForCompileTarget(
            SlangCompileTarget(getOptions().getTarget()));

        auto artifact = ArtifactUtil::createArtifact(targetDesc);
        ArtifactUtil::addAssociated(artifact, diagnostics);

        artifact->addRepresentation(sharedLibrary);

        *outArtifact = artifact.detach();

        return SLANG_OK;
    }
};

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    String& assemblyOut)
{
    LLVMEmitter emitter(codeGenContext);
    emitter.processModule(irModule);
    emitter.finalize();
    emitter.dumpAssembly(assemblyOut);
    return SLANG_OK;
}

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    List<uint8_t>& objectOut)
{
    LLVMEmitter emitter(codeGenContext);
    emitter.processModule(irModule);
    emitter.finalize();
    emitter.generateObjectCode(objectOut);
    return SLANG_OK;
}

SlangResult emitLLVMJITFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    IArtifact** outArtifact)
{
    LLVMEmitter emitter(codeGenContext, true);
    emitter.processModule(irModule);
    emitter.finalize();
    return emitter.generateJITLibrary(outArtifact);
}

} // namespace Slang
