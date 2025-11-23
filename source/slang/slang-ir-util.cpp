#include "slang-ir-util.h"

#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"

namespace Slang
{

bool isPointerOfType(IRInst* type, IROp opCode)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() && ptrType->getValueType()->getOp() == opCode;
    }
    return false;
}

bool isUserPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrType>(type);
    if (!ptrType)
        return false;
    return ptrType->getAddressSpace() == AddressSpace::UserPointer;
}

IRType* getVectorElementType(IRType* type)
{
    if (auto vectorType = as<IRVectorType>(type))
        return vectorType->getElementType();
    if (auto coopVecType = as<IRCoopVectorType>(type))
        return coopVecType->getElementType();
    if (auto coopMatType = as<IRCoopMatrixType>(type))
        return coopMatType->getElementType();
    return type;
}

IRType* getVectorOrCoopMatrixElementType(IRType* type)
{
    auto vectorElementType = getVectorElementType(type);
    if (vectorElementType != type)
        return vectorElementType;
    if (auto coopMatrixType = as<IRCoopMatrixType>(type))
        return coopMatrixType->getElementType();
    return type;
}

IRType* getMatrixElementType(IRType* type)
{
    if (auto matrixType = as<IRMatrixType>(type))
        return matrixType->getElementType();
    return type;
}

Dictionary<IRInst*, IRInst*> buildInterfaceRequirementDict(IRInterfaceType* interfaceType)
{
    Dictionary<IRInst*, IRInst*> result;
    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (!entry)
            continue;
        result[entry->getRequirementKey()] = entry->getRequirementVal();
    }
    return result;
}

bool isPointerOfType(IRInst* type, IRInst* elementType)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        return ptrType->getValueType() &&
               isTypeEqual(ptrType->getValueType(), (IRType*)elementType);
    }
    return false;
}

bool isPtrToClassType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ClassType);
}

bool isPtrToArrayType(IRInst* type)
{
    return isPointerOfType(type, kIROp_ArrayType) || isPointerOfType(type, kIROp_UnsizedArrayType);
}


bool isComInterfaceType(IRType* type)
{
    if (!type)
        return false;
    if (type->findDecoration<IRComInterfaceDecoration>() || type->getOp() == kIROp_ComPtrType)
    {
        return true;
    }
    if (auto witnessTableType = as<IRWitnessTableTypeBase>(type))
    {
        return isComInterfaceType((IRType*)witnessTableType->getConformanceType());
    }
    if (auto ptrType = as<IRNativePtrType>(type))
    {
        auto valueType = ptrType->getValueType();
        return valueType->findDecoration<IRComInterfaceDecoration>() != nullptr;
    }

    return false;
}

IROp getTypeStyle(IROp op)
{
    switch (op)
    {
    case kIROp_VoidType:
    case kIROp_BoolType:
    case kIROp_EnumType:
        {
            return op;
        }
    case kIROp_Int8Type:
    case kIROp_Int16Type:
    case kIROp_IntType:
    case kIROp_UInt8Type:
    case kIROp_UInt16Type:
    case kIROp_UIntType:
    case kIROp_Int64Type:
    case kIROp_UInt64Type:
    case kIROp_IntPtrType:
    case kIROp_UIntPtrType:
        {
            // All int like
            return kIROp_IntType;
        }
    case kIROp_HalfType:
    case kIROp_FloatType:
    case kIROp_DoubleType:
        {
            // All float like
            return kIROp_FloatType;
        }
    default:
        return kIROp_Invalid;
    }
}

IROp getTypeStyle(BaseType op)
{
    switch (op)
    {
    case BaseType::Void:
        return kIROp_VoidType;
    case BaseType::Bool:
        return kIROp_BoolType;
    case BaseType::Char:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::UIntPtr:
        return kIROp_IntType;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
        return kIROp_FloatType;
    default:
        return kIROp_Invalid;
    }
}

IRInst* specializeWithGeneric(
    IRBuilder& builder,
    IRInst* genericToSpecialize,
    IRGeneric* userGeneric)
{
    List<IRInst*> genArgs;
    for (auto param : userGeneric->getFirstBlock()->getParams())
    {
        genArgs.add(param);
    }
    return builder.emitSpecializeInst(
        builder.getTypeKind(),
        genericToSpecialize,
        (UInt)genArgs.getCount(),
        genArgs.getBuffer());
}

IRInst* maybeSpecializeWithGeneric(
    IRBuilder& builder,
    IRInst* genericToSpecailize,
    IRInst* userGeneric)
{
    if (auto gen = as<IRGeneric>(userGeneric))
    {
        if (auto toSpecialize = as<IRGeneric>(genericToSpecailize))
        {
            return specializeWithGeneric(builder, toSpecialize, gen);
        }
    }
    return genericToSpecailize;
}

// Returns true if is not possible to produce side-effect from a value of `dataType`.
bool isValueType(IRInst* dataType)
{
    dataType = getResolvedInstForDecorations(unwrapAttributedType(dataType));
    if (as<IRBasicType>(dataType))
        return true;
    switch (dataType->getOp())
    {
    case kIROp_StructType:
    case kIROp_InterfaceType:
    case kIROp_ClassType:
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_TupleType:
    case kIROp_ResultType:
    case kIROp_OptionalType:
    case kIROp_DifferentialPairType:
    case kIROp_DifferentialPairUserCodeType:
    case kIROp_DynamicType:
    case kIROp_AnyValueType:
    case kIROp_ArrayType:
    case kIROp_FuncType:
    case kIROp_RaytracingAccelerationStructureType:
    case kIROp_GLSLAtomicUintType:
    case kIROp_EnumType:
        return true;
    default:
        // Read-only resource handles are considered as Value type.
        if (auto resType = as<IRResourceTypeBase>(dataType))
            return (resType->getAccess() == SLANG_RESOURCE_ACCESS_READ);
        else if (as<IRSamplerStateTypeBase>(dataType))
            return true;
        else if (as<IRHLSLByteAddressBufferType>(dataType))
            return true;
        else if (as<IRHLSLStructuredBufferType>(dataType))
            return true;
        return false;
    }
}

bool isScalarOrVectorType(IRInst* type)
{
    switch (type->getOp())
    {
    case kIROp_VectorType:
        return true;
    default:
        return as<IRBasicType>(type) != nullptr;
    }
}

bool isSimpleDataType(IRType* type)
{
    type = (IRType*)unwrapAttributedType(type);
    if (as<IRBasicType>(type))
        return true;
    switch (type->getOp())
    {
    case kIROp_StructType:
        {
            auto structType = as<IRStructType>(type);
            for (auto field : structType->getFields())
            {
                if (!isSimpleDataType(field->getFieldType()))
                    return false;
            }
            return true;
            break;
        }
    case kIROp_Param:
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_InterfaceType:
    case kIROp_AnyValueType:
    case kIROp_PtrType:
        return true;
    case kIROp_EnumType:
        {
            auto enumType = as<IREnumType>(type);
            auto tagType = enumType->getTagType();
            return isSimpleDataType(tagType);
        }
    case kIROp_ArrayType:
    case kIROp_UnsizedArrayType:
        return isSimpleDataType((IRType*)type->getOperand(0));
    default:
        return false;
    }
}

bool isSimpleHLSLDataType(IRInst* inst)
{
    // TODO: Add criteria
    // https://github.com/shader-slang/slang/issues/4792
    SLANG_UNUSED(inst);
    return true;
}

bool isWrapperType(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_ArrayType:
    case kIROp_TextureType:
    case kIROp_VectorType:
    case kIROp_MatrixType:
    case kIROp_PtrType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_HLSLStructuredBufferType:
    case kIROp_HLSLRWStructuredBufferType:
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
    case kIROp_HLSLAppendStructuredBufferType:
    case kIROp_HLSLConsumeStructuredBufferType:
    case kIROp_TupleType:
    case kIROp_OptionalType:
    case kIROp_TypePack:
        return true;
    default:
        return false;
    }
}

SourceLoc findFirstUseLoc(IRInst* inst)
{
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        if (use->getUser()->sourceLoc.isValid())
        {
            return use->getUser()->sourceLoc;
        }
    }
    return inst->sourceLoc;
}

IRInst* hoistValueFromGeneric(
    IRBuilder& inBuilder,
    IRInst* value,
    IRInst*& outSpecializedVal,
    bool replaceExistingValue)
{
    auto outerGeneric = as<IRGeneric>(findOuterGeneric(value));
    if (!outerGeneric)
        return value;
    IRBuilder builder = inBuilder;
    builder.setInsertBefore(outerGeneric);
    auto newGeneric = builder.emitGeneric();
    builder.setInsertInto(newGeneric);
    builder.emitBlock();
    IRInst* newResultVal = nullptr;

    // Clone insts in outerGeneric up until `value`.
    IRCloneEnv cloneEnv;
    for (auto inst : outerGeneric->getFirstBlock()->getChildren())
    {
        auto newInst = cloneInst(&cloneEnv, &builder, inst);
        if (inst == value)
        {
            builder.emitReturn(newInst);
            newResultVal = newInst;
            break;
        }
    }
    SLANG_RELEASE_ASSERT(newResultVal);
    if (newResultVal->getOp() == kIROp_Func)
    {
        IRBuilder subBuilder = builder;
        IRInst* subOutSpecialized = nullptr;
        auto genericFuncType = hoistValueFromGeneric(
            subBuilder,
            newResultVal->getFullType(),
            subOutSpecialized,
            false);
        newGeneric->setFullType((IRType*)genericFuncType);
    }
    else
    {
        newGeneric->setFullType(builder.getTypeKind());
    }
    if (replaceExistingValue)
    {
        builder.setInsertBefore(value);
        outSpecializedVal = specializeWithGeneric(builder, newGeneric, outerGeneric);
        value->replaceUsesWith(outSpecializedVal);
        value->removeAndDeallocate();
    }
    eliminateDeadCode(newGeneric);
    return newGeneric;
}

void moveInstChildren(IRInst* dest, IRInst* src)
{
    for (auto child = dest->getFirstDecorationOrChild(); child;)
    {
        auto next = child->getNextInst();
        child->removeAndDeallocate();
        child = next;
    }
    for (auto child = src->getFirstDecorationOrChild(); child;)
    {
        auto next = child->getNextInst();
        child->insertAtEnd(dest);
        child = next;
    }
}

String dumpIRToString(IRInst* root, IRDumpOptions options)
{
    StringBuilder sb;
    StringWriter writer(&sb, Slang::WriterFlag::AutoFlush);
    dumpIR(root, options, nullptr, &writer);
    return sb.toString();
}

void copyNameHintAndDebugDecorations(IRInst* dest, IRInst* src)
{
    IRDecoration* nameHintDecoration = nullptr;
    IRDecoration* linkageDecoration = nullptr;
    IRDecoration* debugLocationDecoration = nullptr;
    for (auto decor = src->getFirstDecoration(); decor; decor = decor->getNextDecoration())
    {
        switch (decor->getOp())
        {
        case kIROp_NameHintDecoration:
            nameHintDecoration = decor;
            break;
        case kIROp_ImportDecoration:
        case kIROp_ExportDecoration:
            linkageDecoration = decor;
            break;
        case kIROp_DebugLocationDecoration:
            debugLocationDecoration = decor;
            break;
        }
    }
    if (nameHintDecoration)
    {
        cloneDecoration(nameHintDecoration, dest);
    }
    if (linkageDecoration)
    {
        cloneDecoration(linkageDecoration, dest);
    }
    if (debugLocationDecoration)
    {
        cloneDecoration(debugLocationDecoration, dest);
    }
}

void getTypeNameHint(StringBuilder& sb, IRInst* type)
{
    if (!type)
        return;

    switch (type->getOp())
    {
    case kIROp_BoolType:
        sb << "bool";
        break;
    case kIROp_FloatType:
        sb << "float";
        break;
    case kIROp_HalfType:
        sb << "half";
        break;
    case kIROp_DoubleType:
        sb << "double";
        break;
    case kIROp_IntType:
        sb << "int";
        break;
    case kIROp_Int8Type:
        sb << "int8_t";
        break;
    case kIROp_Int16Type:
        sb << "int16_t";
        break;
    case kIROp_Int64Type:
        sb << "int64_t";
        break;
    case kIROp_IntPtrType:
        sb << "intptr_t";
        break;
    case kIROp_UIntType:
        sb << "uint";
        break;
    case kIROp_UInt8Type:
        sb << "uint8_t";
        break;
    case kIROp_UInt16Type:
        sb << "uint16_t";
        break;
    case kIROp_UInt64Type:
        sb << "uint64_t";
        break;
    case kIROp_UIntPtrType:
        sb << "uintptr_t";
        break;
    case kIROp_CharType:
        sb << "char";
        break;
    case kIROp_StringType:
        sb << "string";
        break;
    case kIROp_ArrayType:
        sb << "array<";
        getTypeNameHint(sb, type->getOperand(0));
        sb << ",";
        getTypeNameHint(sb, as<IRArrayType>(type)->getElementCount());
        sb << ">";
        break;
    case kIROp_UnsizedArrayType:
        sb << "runtime_array<";
        getTypeNameHint(sb, type->getOperand(0));
        sb << ">";
        break;
    case kIROp_SubpassInputType:
        {
            auto textureType = as<IRSubpassInputType>(type);
            sb << "SubpassInput";
            if (textureType->isMultisample())
                sb << "MS";
            break;
        }
    case kIROp_TextureType:
    case kIROp_GLSLImageType:
        {
            auto textureType = as<IRResourceTypeBase>(type);
            switch (textureType->getAccess())
            {
            case SLANG_RESOURCE_ACCESS_APPEND:
                sb << "Append";
                break;
            case SLANG_RESOURCE_ACCESS_CONSUME:
                sb << "Consume";
                break;
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                sb << "RasterizerOrdered";
                break;
            case SLANG_RESOURCE_ACCESS_WRITE:
                sb << "RW";
                break;
            case SLANG_RESOURCE_ACCESS_FEEDBACK:
                sb << "Feedback";
                break;
            case SLANG_RESOURCE_ACCESS_READ:
                break;
            }
            if (textureType->isCombined())
            {
                switch (textureType->GetBaseShape())
                {
                case SLANG_TEXTURE_1D:
                    sb << "Sampler1D";
                    break;
                case SLANG_TEXTURE_2D:
                    sb << "Sampler2D";
                    break;
                case SLANG_TEXTURE_3D:
                    sb << "Sampler3D";
                    break;
                case SLANG_TEXTURE_CUBE:
                    sb << "SamplerCube";
                    break;
                case SLANG_TEXTURE_BUFFER:
                    sb << "SamplerBuffer";
                    break;
                }
            }
            else
            {
                switch (textureType->GetBaseShape())
                {
                case SLANG_TEXTURE_1D:
                    sb << "Texture1D";
                    break;
                case SLANG_TEXTURE_2D:
                    sb << "Texture2D";
                    break;
                case SLANG_TEXTURE_3D:
                    sb << "Texture3D";
                    break;
                case SLANG_TEXTURE_CUBE:
                    sb << "TextureCube";
                    break;
                case SLANG_TEXTURE_BUFFER:
                    sb << "Buffer";
                    break;
                }
            }
            if (textureType->isMultisample())
            {
                sb << "MS";
            }
            if (textureType->isArray())
            {
                sb << "Array";
            }
            if (textureType->isShadow())
            {
                sb << "Shadow";
            }
        }
        break;
    case kIROp_ParameterBlockType:
        sb << "ParameterBlock<";
        getTypeNameHint(sb, as<IRParameterBlockType>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_ConstantBufferType:
        sb << "cbuffer<";
        getTypeNameHint(sb, as<IRConstantBufferType>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_TextureBufferType:
        sb << "tbuffer<";
        getTypeNameHint(sb, as<IRTextureBufferType>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_GLSLShaderStorageBufferType:
        sb << "StorageBuffer<";
        getTypeNameHint(sb, as<IRGLSLShaderStorageBufferType>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_HLSLByteAddressBufferType:
        sb << "ByteAddressBuffer";
        break;
    case kIROp_HLSLRWByteAddressBufferType:
        sb << "RWByteAddressBuffer";
        break;
    case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
        sb << "RasterizerOrderedByteAddressBuffer";
        break;
    case kIROp_GLSLAtomicUintType:
        sb << "AtomicCounter";
        break;
    case kIROp_RaytracingAccelerationStructureType:
        sb << "RaytracingAccelerationStructure";
        break;
    case kIROp_HitObjectType:
        sb << "HitObject";
        break;
    case kIROp_HLSLStructuredBufferType:
        sb << "StructuredBuffer<";
        getTypeNameHint(sb, as<IRHLSLStructuredBufferTypeBase>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_HLSLRWStructuredBufferType:
        sb << "RWStructuredBuffer<";
        getTypeNameHint(sb, as<IRHLSLStructuredBufferTypeBase>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_HLSLAppendStructuredBufferType:
        sb << "AppendStructuredBuffer<";
        getTypeNameHint(sb, as<IRHLSLStructuredBufferTypeBase>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_HLSLConsumeStructuredBufferType:
        sb << "ConsumeStructuredBuffer<";
        getTypeNameHint(sb, as<IRHLSLStructuredBufferTypeBase>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        sb << "RasterizerOrderedStructuredBuffer<";
        getTypeNameHint(sb, as<IRHLSLStructuredBufferTypeBase>(type)->getElementType());
        sb << ">";
        break;
    case kIROp_SamplerStateType:
        sb << "SamplerState";
        break;
    case kIROp_SamplerComparisonStateType:
        sb << "SamplerComparisonState";
        break;
    case kIROp_TextureFootprintType:
        sb << "TextureFootprint";
        break;
    case kIROp_Specialize:
        {
            auto specialize = as<IRSpecialize>(type);
            getTypeNameHint(sb, specialize->getBase());
            sb << "<";
            bool isFirst = true;
            for (UInt i = 0; i < specialize->getArgCount(); i++)
            {
                auto arg = specialize->getArg(i);
                if (!arg->getDataType())
                    continue;
                if (arg->getDataType()->getOp() == kIROp_WitnessTableType)
                    continue;
                if (!isFirst)
                    sb << ",";
                getTypeNameHint(sb, arg);
                isFirst = false;
            }
            sb << ">";
        }
        break;
    case kIROp_AttributedType:
        getTypeNameHint(sb, as<IRAttributedType>(type)->getBaseType());
        break;
    case kIROp_RateQualifiedType:
        getTypeNameHint(sb, as<IRRateQualifiedType>(type)->getValueType());
        break;
    case kIROp_VectorType:
        sb << "vector<";
        getTypeNameHint(sb, type->getOperand(0));
        sb << ",";
        getTypeNameHint(sb, as<IRVectorType>(type)->getElementCount());
        sb << ">";
        break;
    case kIROp_MatrixType:
        sb << "matrix<";
        getTypeNameHint(sb, type->getOperand(0));
        sb << ",";
        getTypeNameHint(sb, as<IRMatrixType>(type)->getRowCount());
        sb << ",";
        getTypeNameHint(sb, as<IRMatrixType>(type)->getColumnCount());
        sb << ">";
        break;
    case kIROp_IntLit:
        sb << as<IRIntLit>(type)->getValue();
        break;
    case kIROp_BoolLit:
        sb << (as<IRBoolLit>(type)->getValue() ? "true" : "false");
        break;
    case kIROp_FloatLit:
        sb << as<IRFloatLit>(type)->getValue();
        break;
    case kIROp_StringLit:
        {
            auto stringLit = as<IRStringLit>(type);
            sb << "\"";
            sb << stringLit->getStringSlice();
            sb << "\"";
        }
        break;
    case kIROp_VoidLit:
        sb << "void";
        break;
    case kIROp_PtrLit:
        {
            auto ptrLit = as<IRPtrLit>(type);
            sb << "ptr_";
            sb << (UInt64)ptrLit->getValue();
        }
        break;
    default:
        if (auto decor = type->findDecoration<IRNameHintDecoration>())
            sb << decor->getName();
        break;
    }
}

IRInst* getRootAddr(IRInst* addr)
{
    for (;;)
    {
        switch (addr->getOp())
        {
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            addr = addr->getOperand(0);
            continue;
        default:
            break;
        }
        break;
    }
    return addr;
}

IRInst* getRootAddr(IRInst* addr, List<IRInst*>& outAccessChain, List<IRInst*>* outTypes)
{
    for (;;)
    {
        switch (addr->getOp())
        {
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            outAccessChain.add(addr->getOperand(1));
            if (outTypes)
                outTypes->add(addr->getFullType());
            addr = addr->getOperand(0);
            continue;
        default:
            break;
        }
        break;
    }
    outAccessChain.reverse();
    if (outTypes)
        outTypes->reverse();
    return addr;
}


IRInst* getRootBufferOrAddr(IRInst* addr)
{
    auto rootAddr = getRootAddr(addr);
    if (as<IRRWStructuredBufferGetElementPtr>(rootAddr))
    {
        auto bufferHandle = rootAddr->getOperand(0);
        // Check if the bufferHandle itself is a load from a global parameter.
        if (auto load = as<IRLoad>(bufferHandle))
        {
            auto newRoot = getRootAddr(load->getPtr());
            if (newRoot->getOp() == kIROp_GlobalParam)
                return newRoot;
        }
    }
    return rootAddr;
}

// The aliasing class of an address. This is used to determine
// if two addresses may alias.
enum class AddressAliasingClass
{
    Unknown,
    UserPointer,      // A user pointer into global memory
    Var,              // A thread-local or groupshared var.
    ConstantBuffer,   // A constant buffer or parameter block.
    BoundBuffer,      // A bound buffer.
    BoundTexture,     // A bound texture resource.
    DescriptorHandle, // A bindless buffer or resource.
};

AddressAliasingClass getAliasingClass(IRInst* addr)
{
    if (auto globalParam = as<IRGlobalParam>(addr))
    {
        auto type = unwrapArray(globalParam->getDataType());
        if (!type)
            return AddressAliasingClass::Unknown;
        switch (type->getOp())
        {
        case kIROp_TextureType:
            return AddressAliasingClass::BoundTexture;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLAppendStructuredBufferType:
        case kIROp_HLSLConsumeStructuredBufferType:
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
        case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
        case kIROp_GLSLShaderStorageBufferType:
            return AddressAliasingClass::BoundBuffer;
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            return AddressAliasingClass::ConstantBuffer;
        case kIROp_PtrType:
            if (isUserPointerType(type))
                return AddressAliasingClass::UserPointer;
            return AddressAliasingClass::Unknown;
        case kIROp_DynamicResourceType:
            return AddressAliasingClass::DescriptorHandle;
        default:
            return AddressAliasingClass::Unknown;
        }
    }
    else if (as<IRVar>(addr))
        return AddressAliasingClass::Var;
    else if (as<IRGlobalVar>(addr))
        return AddressAliasingClass::Var;
    else if (as<IRRWStructuredBufferGetElementPtr>(addr))
        return AddressAliasingClass::DescriptorHandle;
    else if (as<IRCastDescriptorHandleToResource>(addr))
        return AddressAliasingClass::DescriptorHandle;

    auto type = addr->getDataType();
    if (isUserPointerType(type))
        return AddressAliasingClass::UserPointer;
    return AddressAliasingClass::Unknown;
}

bool canAddrClassesAlias(AddressAliasingClass c1, AddressAliasingClass c2)
{
    if (c1 == AddressAliasingClass::Unknown || c2 == AddressAliasingClass::Unknown)
        return true;

    switch (c1)
    {
    case AddressAliasingClass::Unknown:
        return true;
    case AddressAliasingClass::UserPointer:
    case AddressAliasingClass::Var:
        // A users pointer or var can only alias with another
        // object that is either a user pointer or var.
        //
        // Generally, a var should never alias with anything else that isn't a var,
        // if we never allow the user to take address of a local var.
        // We don't allow taking addresses of a local var on most GPU targets, but
        // we currently do expose an internal intrinsic to do so when targeting CPU.
        // We should consider disallowing this across the board, or enable more aggresive
        // criteria when targeting GPU backends.
        // For now we stay conservative and just report true even when addr1 is var and
        // addr2 is not rooted from a var.
        //
        return c2 == AddressAliasingClass::UserPointer || c2 == AddressAliasingClass::Var;
    case AddressAliasingClass::BoundBuffer:
    case AddressAliasingClass::BoundTexture:
        // A bound resource can only alias with another
        // object that is a bound resource or descriptor handle
        return c2 == c1 || c2 == AddressAliasingClass::DescriptorHandle;

    case AddressAliasingClass::DescriptorHandle:
        // Can alias with any other resource.
        switch (c2)
        {
        case AddressAliasingClass::BoundBuffer:
        case AddressAliasingClass::BoundTexture:
        case AddressAliasingClass::DescriptorHandle:
            return true;
        default:
            return false;
        }
    case AddressAliasingClass::ConstantBuffer:
        // Constant buffer cannot alias with anything.
        return false;
    }
    // For any other unknown case, assume they may alias.
    return true;
}

// Has `var` being used in a way that may allow it to alias with a user pointer?
bool canVarAliasWithUserPointer(TargetRequest* target, IRInst* var)
{
    if (target && !isCPUTarget(target))
    {
        // We don't allow taking the address of a variable on anything other
        // than the CPU target. Therefore a var can never alias with a user
        // pointer on these targets.
        return false;
    }

    SLANG_UNUSED(var);
    return true;
}

// A simple and conservative address aliasing check.
bool canAddressesPotentiallyAlias(
    TargetRequest* target,
    IRGlobalValueWithCode* func,
    IRInst* addr1,
    IRInst* addr2)
{
    if (addr1 == addr2)
        return true;

    auto root1 = getRootBufferOrAddr(addr1);
    auto root2 = getRootBufferOrAddr(addr2);

    auto addr1Class = getAliasingClass(root1);
    auto addr2Class = getAliasingClass(root2);

    if (!canAddrClassesAlias(addr1Class, addr2Class))
        return false;

    if (addr1Class == addr2Class)
    {
        // For these classes of addresses, the identity of the root
        // determines whether or not the addresse can alias.
        // Note that we assume two different bound resources can never
        // alias, and two different variables can never alias.
        switch (addr1Class)
        {
        case AddressAliasingClass::Var:
        case AddressAliasingClass::BoundBuffer:
        case AddressAliasingClass::BoundTexture:
        case AddressAliasingClass::ConstantBuffer:
            if (root1 != root2)
                return false;
            break;
        }
    }

    // A param and a var can never alias.
    if (root1->getOp() == kIROp_Param && root1->getParent() == func->getFirstBlock() &&
            root2->getOp() == kIROp_Var ||
        root1->getOp() == kIROp_Var && root2->getOp() == kIROp_Param &&
            root2->getParent() == func->getFirstBlock())
        return false;

    // If one addr is user pointer and one addr is a var,
    // they can never alias, if the user code never took the address of
    // the var.
    if (addr1Class == AddressAliasingClass::Var && addr2Class == AddressAliasingClass::UserPointer)
    {
        return canVarAliasWithUserPointer(target, root1);
    }
    if (addr2Class == AddressAliasingClass::Var && addr1Class == AddressAliasingClass::UserPointer)
    {
        return canVarAliasWithUserPointer(target, root2);
    }

    // If two addrs are rooted from the same object but found to statically differ in access chain,
    // then they cannot alias.
    if (root1 == root2)
    {
        List<IRInst*> accessChain1;
        List<IRInst*> accessChain2;

        // Since getRootBufferOrAddr has a different behavior around
        // RWStructuredBufferGetElementPtr compared to getRootAddr,
        // we need to call getRootAddr here again to get a simpler access chain
        // that we can handle here, so that we don't need to handle the nuance
        // of whether or not to trace past any RWStructuredBufferGetElementPtr.
        //
        root1 = getRootAddr(addr1, accessChain1, nullptr);
        root2 = getRootAddr(addr2, accessChain2, nullptr);
        if (root1 != root2)
            return true;
        for (Index i = 0; i < Math::Min(accessChain1.getCount(), accessChain2.getCount()); i++)
        {
            auto node1 = accessChain1[i];
            auto node2 = accessChain2[i];
            if (as<IRStructKey>(node1) && as<IRStructKey>(node2))
            {
                // Two different field keys means the two addresses cannot alias.
                // TODO: If we are going to support union types, we need to exclude that
                // here.
                if (node1 != node2)
                    return false;
                // If the keys are the same, continue looking further down the access chain.
                continue;
            }
            // Two different constant indices means the two addresses cannot alias.
            auto index1 = as<IRIntLit>(node1);
            auto index2 = as<IRIntLit>(node2);
            if (index1 && index2 && index1->getValue() != index2->getValue())
                return false;
            // In all other cases, such as when either one of the indices is
            // a untime value, we treat the two indices as potentially being the same.
            return true;
        }
    }
    return true;
}

bool canAddressesPotentiallyAlias(IRGlobalValueWithCode* func, IRInst* addr1, IRInst* addr2)
{
    return canAddressesPotentiallyAlias(nullptr, func, addr1, addr2);
}

bool isPtrLikeOrHandleType(IRInst* type)
{
    if (!type)
        return false;
    if (as<IRPointerLikeType>(type))
        return true;
    if (as<IRPseudoPtrType>(type))
        return true;
    if (as<IRHLSLStructuredBufferTypeBase>(type))
        return true;
    switch (type->getOp())
    {
    case kIROp_ComPtrType:
    case kIROp_RawPointerType:
    case kIROp_RTTIPointerType:
    case kIROp_OutParamType:
    case kIROp_BorrowInOutParamType:
    case kIROp_PtrType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_GLSLShaderStorageBufferType:
        return true;
    }
    return false;
}

bool canInstHaveSideEffectAtAddress(IRGlobalValueWithCode* func, IRInst* inst, IRInst* addr)
{
    switch (inst->getOp())
    {
    case kIROp_Store:
        // If the target of the store inst may overlap addr, return true.
        if (canAddressesPotentiallyAlias(func, as<IRStore>(inst)->getPtr(), addr))
            return true;
        break;
    case kIROp_SwizzledStore:
        // If the target of the swizzled store inst may overlap addr, return true.
        if (canAddressesPotentiallyAlias(func, as<IRSwizzledStore>(inst)->getDest(), addr))
            return true;
        break;
    case kIROp_Call:
        {
            auto call = as<IRCall>(inst);

            // If addr is a global variable, calling a function may change its value.
            // So we need to return true here to be conservative.
            if (!isChildInstOf(getRootAddr(addr), func))
            {
                auto callee = call->getCallee();
                if (callee && !doesCalleeHaveSideEffect(callee))
                {
                    // An exception is if the callee is side-effect free and is not reading from
                    // memory.
                }
                else
                {
                    return true;
                }
            }

            // If any pointer typed argument of the call inst may overlap addr, return true.
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                SLANG_RELEASE_ASSERT(call->getArg(i)->getDataType());
                if (isPtrLikeOrHandleType(call->getArg(i)->getDataType()))
                {
                    if (canAddressesPotentiallyAlias(func, call->getArg(i), addr))
                        return true;
                }
                else if (!isValueType(call->getArg(i)->getDataType()))
                {
                    // This is some unknown handle type, we assume it can have any side effects.
                    return true;
                }
            }
        }
        break;
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
        {
            auto branch = as<IRUnconditionalBranch>(inst);
            // If any pointer typed argument of the branch inst may overlap addr, return true.
            for (UInt i = 0; i < branch->getArgCount(); i++)
            {
                SLANG_RELEASE_ASSERT(branch->getArg(i)->getDataType());
                if (isPtrLikeOrHandleType(branch->getArg(i)->getDataType()))
                {
                    if (canAddressesPotentiallyAlias(func, branch->getArg(i), addr))
                        return true;
                }
                else if (!isValueType(branch->getArg(i)->getDataType()))
                {
                    // This is some unknown handle type, we assume it can have any side effects.
                    return true;
                }
            }
        }
        break;
    case kIROp_CastPtrToInt:
    case kIROp_Reinterpret:
    case kIROp_BitCast:
        {
            // If we are trying to cast an address to something else, return true.
            if (isPtrLikeOrHandleType(inst->getOperand(0)->getDataType()) &&
                canAddressesPotentiallyAlias(func, inst->getOperand(0), addr))
                return true;
            else if (!isValueType(inst->getOperand(0)->getDataType()))
            {
                // This is some unknown handle type, we assume it can have any side effects.
                return true;
            }
        }
        break;
    default:
        // Default behavior is that any insts that have side effect may affect `addr`.
        if (inst->mightHaveSideEffects())
            return true;
        break;
    }
    return false;
}

IRInst* getUnitPoisonVal(IRBuilder builder, IRModule* module)
{
    IRInst* undefInst = nullptr;

    for (auto inst : module->getModuleInst()->getChildren())
    {
        if (inst->getOp() == kIROp_Poison && inst->getDataType() &&
            inst->getDataType()->getOp() == kIROp_VoidType)
        {
            undefInst = inst;
            break;
        }
    }
    if (!undefInst)
    {
        auto voidType = builder.getVoidType();
        builder.setInsertAfter(voidType);
        undefInst = builder.emitPoison(voidType);
    }
    return undefInst;
}

IROp getSwapSideComparisonOp(IROp op)
{
    switch (op)
    {
    case kIROp_Eql:
        return kIROp_Eql;
    case kIROp_Neq:
        return kIROp_Neq;
    case kIROp_Leq:
        return kIROp_Geq;
    case kIROp_Geq:
        return kIROp_Leq;
    case kIROp_Less:
        return kIROp_Greater;
    case kIROp_Greater:
        return kIROp_Less;
    default:
        return kIROp_Nop;
    }
}

IRInst* emitLoopBlocks(
    IRBuilder* builder,
    IRInst* initVal,
    IRInst* finalVal,
    IRBlock*& loopBodyBlock,
    IRBlock*& loopBreakBlock)
{
    IRBuilder loopBuilder = *builder;
    auto loopHeadBlock = loopBuilder.emitBlock();
    loopBodyBlock = loopBuilder.emitBlock();
    auto ifBreakBlock = loopBuilder.emitBlock();
    loopBreakBlock = loopBuilder.emitBlock();
    auto loopContinueBlock = loopBuilder.emitBlock();
    builder->emitLoop(loopHeadBlock, loopBreakBlock, loopContinueBlock, 1, &initVal);
    loopBuilder.setInsertInto(loopHeadBlock);
    auto loopParam = loopBuilder.emitParam(initVal->getFullType());
    auto cmpResult = loopBuilder.emitLess(loopParam, finalVal);
    loopBuilder.emitIfElse(cmpResult, loopBodyBlock, ifBreakBlock, ifBreakBlock);
    loopBuilder.setInsertInto(loopBodyBlock);
    loopBuilder.emitBranch(loopContinueBlock);
    loopBuilder.setInsertInto(loopContinueBlock);
    auto newParam = loopBuilder.emitAdd(
        loopParam->getFullType(),
        loopParam,
        loopBuilder.getIntValue(loopBuilder.getIntType(), 1));
    loopBuilder.emitBranch(loopHeadBlock, 1, &newParam);
    loopBuilder.setInsertInto(ifBreakBlock);
    loopBuilder.emitBranch(loopBreakBlock);
    return loopParam;
}

void sortBlocksInFunc(IRGlobalValueWithCode* func)
{
    auto order = getReversePostorder(func);
    for (auto block : order)
        block->insertAtEnd(func);
}

void removeLinkageDecorations(IRInst* inst)
{
    if (!inst)
        return;

    List<IRInst*> toRemove;
    for (auto decoration : inst->getDecorations())
    {
        switch (decoration->getOp())
        {
        case kIROp_ImportDecoration:
        case kIROp_ExportDecoration:
        case kIROp_ExternCppDecoration:
        case kIROp_PublicDecoration:
        case kIROp_KeepAliveDecoration:
        case kIROp_DllImportDecoration:
        case kIROp_CudaDeviceExportDecoration:
        case kIROp_DllExportDecoration:
        case kIROp_HLSLExportDecoration:
            toRemove.add(decoration);
            break;
        default:
            break;
        }
    }
    for (auto decoration : toRemove)
        decoration->removeAndDeallocate();
}

void setInsertBeforeOrdinaryInst(IRBuilder* builder, IRInst* inst)
{
    if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
    {
        SLANG_RELEASE_ASSERT(as<IRBlock>(inst->getParent()));
        auto lastParam = as<IRBlock>(inst->getParent())->getLastParam();
        builder->setInsertAfter(lastParam);
    }
    else
    {
        builder->setInsertBefore(inst);
    }
}

void setInsertAfterOrdinaryInst(IRBuilder* builder, IRInst* inst)
{
    if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
    {
        SLANG_RELEASE_ASSERT(as<IRBlock>(inst->getParent()));
        auto lastParam = as<IRBlock>(inst->getParent())->getLastParam();
        builder->setInsertAfter(lastParam);
    }
    else
    {
        builder->setInsertAfter(inst);
    }
}

IRInst* tryFindBasePtr(IRInst* inst, IRInst* parentFunc)
{
    // Keep going up the tree until we find a variable.
    switch (inst->getOp())
    {
    case kIROp_Var:
        return getParentFunc(inst) == parentFunc ? inst : nullptr;
    case kIROp_Param:
        return getParentFunc(inst) == parentFunc ? inst : nullptr;
    case kIROp_GetElementPtr:
        return tryFindBasePtr(as<IRGetElementPtr>(inst)->getBase(), parentFunc);
    case kIROp_FieldAddress:
        return tryFindBasePtr(as<IRFieldAddress>(inst)->getBase(), parentFunc);
    default:
        return nullptr;
    }
}

bool areCallArgumentsSideEffectFree(IRCall* call, SideEffectAnalysisOptions options)
{
    // If the function has no side effect and is not writing to any outputs,
    // we can safely treat the call as a normal inst.

    IRFunc* parentFunc = nullptr;

    IRParam* param = nullptr;
    if (auto calleeFunc = getResolvedInstForDecorations(call->getCallee()))
    {
        if (auto block = calleeFunc->getFirstBlock())
        {
            param = block->getFirstParam();
        }
    }

    for (UInt i = 0; i < call->getArgCount();
         i++, (param = param ? param->getNextParam() : nullptr))
    {
        auto arg = call->getArg(i);
        if (isValueType(arg->getDataType()))
            continue;

        // If the argument type is not a known value type,
        // assume it is a pointer or handle through which side effect can take place.
        if (!parentFunc)
        {
            parentFunc = getParentFunc(call);
            if (!parentFunc)
                return false;
        }

        auto module = parentFunc->getModule();
        if (!module)
            return false;

        if (arg->getOp() == kIROp_Var && getParentFunc(arg) == parentFunc)
        {
            IRDominatorTree* dom = nullptr;
            if (isBitSet(options, SideEffectAnalysisOptions::UseDominanceTree))
                dom = module->findOrCreateDominatorTree(parentFunc);

            // If the pointer argument is a local variable (thus can't alias with other
            // addresses) and it is never read from in the function, we can safely treat the
            // call as having no side-effect. This is a conservative test, but is sufficient to
            // detect the most common case where a temporary variable is used as the inout
            // argument and the result stored in the temp variable isn't being used elsewhere in
            // the parent func.
            //
            // A more aggresive test can check all other address uses reachable from the call
            // site and see if any of them are aliasing with the argument.
            for (auto use = arg->firstUse; use; use = use->nextUse)
            {
                if (as<IRDecoration>(use->getUser()))
                    continue;
                switch (use->getUser()->getOp())
                {
                case kIROp_Store:
                case kIROp_SwizzledStore:
                    // We are fine with stores into the variable, since store operations
                    // are not dependent on whatever we do in the call here.
                    continue;
                default:
                    // Skip the call itself if the var is used as an argument to an out
                    // parameter since we are checking if the call has side effect. We can't
                    // treat the call as side effect free if var is used as an inout parameter,
                    // because if the call is inside a loop there will be a visible side effect
                    // after the call.
                    if (use->getUser() == call)
                    {
                        auto funcType = as<IRFuncType>(call->getCallee()->getDataType());
                        if (!funcType)
                            return false;
                        if (funcType->getParamCount() > i &&
                            as<IROutParamType>(funcType->getParamType(i)))
                            continue;

                        // We are an argument to an inout parameter.
                        // We can only treat the call as side effect free if the call is not
                        // inside a loop.
                        //
                        // If we don't have the loop information here, we will conservatively
                        // return false.
                        //
                        if (!dom)
                            return false;

                        // If we have dominator tree available, use it to check if the call is
                        // inside a loop.
                        auto callBlock = as<IRBlock>(call->getParent());
                        if (!callBlock)
                            return false;
                        auto varBlock = as<IRBlock>(arg->getParent());
                        if (!varBlock)
                            return false;
                        auto idom = callBlock;
                        while (idom != varBlock)
                        {
                            idom = dom->getImmediateDominator(idom);
                            if (!idom)
                                return false; // If we are here, var does not dominate the call,
                                              // which should never happen.
                            if (auto loop = as<IRLoop>(idom->getTerminator()))
                            {
                                if (!dom->dominates(loop->getBreakBlock(), callBlock))
                                    return false; // The var is used in a loop, must return
                                                  // false.
                            }
                        }
                        // If we reach here, the var is used as an inout parameter for the call,
                        // but the call is not nested in a loop at an higher nesting level than
                        // where the var is defined, so we can treat the use as DCE-able.
                        continue;
                    }
                    // We have some other unknown use of the variable address, they can
                    // be loads, or calls using addresses derived from the variable,
                    // we will treat the call as having side effect to be safe.
                    return false;
                }
            }
        }
        else
        {
            if (param && param->findDecoration<IRIgnoreSideEffectsDecoration>())
                continue;

            return false;
        }
    }
    return true;
}

bool isPureFunctionalCall(IRCall* call, SideEffectAnalysisOptions options)
{
    auto callee = getResolvedInstForDecorations(call->getCallee());
    if (callee->findDecoration<IRReadNoneDecoration>())
    {
        return areCallArgumentsSideEffectFree(call, options);
    }
    return false;
}

bool isSideEffectFreeFunctionalCall(IRCall* call, SideEffectAnalysisOptions options)
{
    if (!doesCalleeHaveSideEffect(call->getCallee()))
    {
        return areCallArgumentsSideEffectFree(call, options);
    }
    return false;
}

// Enumerate any associated functions of 'func'
// that might be used by a pass (e.g. auto-diff)
//
template<typename TFunc>
void forEachAssociatedFunction(IRInst* func, TFunc callback)
{
    // Resolve the function to get all its decorations
    auto resolvedFunc = getResolvedInstForDecorations(func);
    if (!resolvedFunc)
        return;

    // We'll scan for appropriate decorations and return
    // the function references.
    //
    // TODO: In the future, as we get more function transformation
    // passes, we might want to create a parent class for such
    // decorations that associate functions with each other.
    //
    for (auto decor : resolvedFunc->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_UserDefinedBackwardDerivativeDecoration:
            if (as<IRUserDefinedBackwardDerivativeDecoration>(decor))
            {
                auto associatedCallee = as<IRUserDefinedBackwardDerivativeDecoration>(decor)
                                            ->getBackwardDerivativeFunc();
                callback(associatedCallee);
            }
            break;

        case kIROp_ForwardDerivativeDecoration:
            if (as<IRForwardDerivativeDecoration>(decor))
            {
                auto associatedCallee =
                    as<IRForwardDerivativeDecoration>(decor)->getForwardDerivativeFunc();
                callback(associatedCallee);
            }
            break;

        case kIROp_PrimalSubstituteDecoration:
            if (as<IRPrimalSubstituteDecoration>(decor))
            {
                auto associatedCallee =
                    as<IRPrimalSubstituteDecoration>(decor)->getPrimalSubstituteFunc();
                callback(associatedCallee);
            }
            break;

        default:
            break;
        }
    }
}

bool doesCalleeHaveSideEffect(IRInst* callee)
{
    bool sideEffect = true;

    for (auto decor : getResolvedInstForDecorations(callee)->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_NoSideEffectDecoration:
        case kIROp_ReadNoneDecoration:
        case kIROp_IgnoreSideEffectsDecoration:
            sideEffect = false;
            break;
        default:
            break;
        }
    }

    // If the callee has no side effect, check if any of its associated functions have side
    // effect. If so, we want to keep the callee around.
    //
    // Typically, once the relevant pass has completed, the association is removed,
    // and at that point we can remove the function.
    //
    if (!sideEffect)
    {
        forEachAssociatedFunction(
            callee,
            [&](IRInst* associatedCallee)
            {
                sideEffect |= doesCalleeHaveSideEffect(associatedCallee);
                return;
            });
    }

    return sideEffect;
}

IRInst* findInterfaceRequirement(IRInterfaceType* type, IRInst* key)
{
    for (UInt i = 0; i < type->getOperandCount(); i++)
    {
        if (auto req = as<IRInterfaceRequirementEntry>(type->getOperand(i)))
        {
            if (req->getRequirementKey() == key)
                return req->getRequirementVal();
        }
    }
    return nullptr;
}

IRInst* findWitnessTableEntry(IRWitnessTable* table, IRInst* key)
{
    for (auto entry : table->getEntries())
    {
        if (entry->getRequirementKey() == key)
            return entry->getSatisfyingVal();
    }
    return nullptr;
}

IRInst* getVulkanPayloadLocation(IRInst* payloadGlobalVar)
{
    IRInst* location = nullptr;
    for (auto decor : payloadGlobalVar->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_VulkanRayPayloadDecoration:
        case kIROp_VulkanRayPayloadInDecoration:
        case kIROp_VulkanCallablePayloadDecoration:
        case kIROp_VulkanCallablePayloadInDecoration:
        case kIROp_VulkanHitObjectAttributesDecoration:
            return decor->getOperand(0);
        default:
            continue;
        }
    }
    return location;
}

IRInst* getInstInBlock(IRInst* inst)
{
    SLANG_RELEASE_ASSERT(inst);

    if (const auto block = as<IRBlock>(inst->getParent()))
        return inst;

    return getInstInBlock(inst->getParent());
}

ShortList<IRInst*> getPhiArgs(IRInst* phiParam)
{
    ShortList<IRInst*> result;
    auto block = cast<IRBlock>(phiParam->getParent());
    UInt paramIndex = 0;
    for (auto p = block->getFirstParam(); p; p = p->getNextParam())
    {
        if (p == phiParam)
            break;
        paramIndex++;
    }
    for (auto predBlock : block->getPredecessors())
    {
        auto termInst = as<IRUnconditionalBranch>(predBlock->getTerminator());
        SLANG_ASSERT(paramIndex < termInst->getArgCount());
        result.add(termInst->getArg(paramIndex));
    }
    return result;
}

void removePhiArgs(IRInst* phiParam)
{
    auto block = cast<IRBlock>(phiParam->getParent());
    UInt paramIndex = 0;
    for (auto p = block->getFirstParam(); p; p = p->getNextParam())
    {
        if (p == phiParam)
            break;
        paramIndex++;
    }
    for (auto predBlock : block->getPredecessors())
    {
        auto termInst = as<IRUnconditionalBranch>(predBlock->getTerminator());
        SLANG_ASSERT(paramIndex < termInst->getArgCount());
        termInst->removeArgument(paramIndex);
    }
}

int getParamIndexInBlock(IRParam* paramInst)
{
    auto block = as<IRBlock>(paramInst->getParent());
    if (!block)
        return -1;
    int paramIndex = 0;
    for (auto param : block->getParams())
    {
        if (param == paramInst)
            return paramIndex;
        paramIndex++;
    }
    return -1;
}

bool isGlobalOrUnknownMutableAddress(IRGlobalValueWithCode* parentFunc, IRInst* inst)
{
    auto root = getRootAddr(inst);

    auto type = unwrapAttributedType(inst->getDataType());
    if (!isPtrLikeOrHandleType(type))
        return false;

    if (root)
    {
        if (as<IRGLSLShaderStorageBufferType>(root->getDataType()))
        {
            // A storage buffer is mutable, so we need to treat it as a mutable address.
            return true;
        }
        // If this is a global readonly resource, it is not a mutable address.
        if (as<IRParameterGroupType>(root->getDataType()))
        {
            return false;
        }
        if (as<IRHLSLStructuredBufferType>(root->getDataType()))
        {
            return false;
        }
    }

    switch (root->getOp())
    {
    case kIROp_GlobalVar:
    case kIROp_GlobalParam:
    case kIROp_GlobalConstant:
    case kIROp_Var:
    case kIROp_Param:
        break;
    case kIROp_Call:
        return true;
    default:
        return true;
    }

    auto addrInstParent = getParentFunc(root);
    return (addrInstParent != parentFunc);
}

bool isZero(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IntLit:
        return as<IRIntLit>(inst)->getValue() == 0;
    case kIROp_FloatLit:
        return as<IRFloatLit>(inst)->getValue() == 0.0;
    case kIROp_BoolLit:
        return as<IRBoolLit>(inst)->getValue() == false;
    case kIROp_MakeCoopVector:
    case kIROp_MakeVector:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
        {
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                if (!isZero(inst->getOperand(i)))
                {
                    return false;
                }
            }
            return true;
        }
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
        return isZero(inst->getOperand(0));
    default:
        return false;
    }
}

bool isOne(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_IntLit:
        return as<IRIntLit>(inst)->getValue() == 1;
    case kIROp_FloatLit:
        return as<IRFloatLit>(inst)->getValue() == 1.0;
    case kIROp_BoolLit:
        return as<IRBoolLit>(inst)->getValue();
    case kIROp_MakeCoopVector:
    case kIROp_MakeVector:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
        {
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                if (!isOne(inst->getOperand(i)))
                {
                    return false;
                }
            }
            return true;
        }
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
        return isOne(inst->getOperand(0));
    default:
        return false;
    }
}

IRPtrTypeBase* asRelevantPtrType(IRInst* inst)
{
    if (auto ptrType = as<IRPtrTypeBase>(inst))
    {
        if (ptrType->getAddressSpace() != AddressSpace::UserPointer)
            return ptrType;
    }
    return nullptr;
}

IRPtrTypeBase* isMutablePointerType(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_BorrowInParamType:
        return nullptr;
    default:
        return asRelevantPtrType(inst);
    }
}

void initializeScratchData(IRInst* inst)
{
    List<IRInst*> workList;
    workList.add(inst);
    while (workList.getCount() != 0)
    {
        auto item = workList.getLast();
        workList.removeLast();
        item->scratchData = 0;
        for (auto child = item->getLastDecorationOrChild(); child; child = child->getPrevInst())
            workList.add(child);
    }
}

void resetScratchDataBit(IRInst* inst, int bitIndex)
{
    List<IRInst*> workList;
    workList.add(inst);
    while (workList.getCount() != 0)
    {
        auto item = workList.getLast();
        workList.removeLast();
        item->scratchData &= ~(1ULL << bitIndex);
        for (auto child = item->getLastDecorationOrChild(); child; child = child->getPrevInst())
            workList.add(child);
    }
}

///
/// IRBlock related common helper methods
///
void moveParams(IRBlock* dest, IRBlock* src)
{
    for (auto param = src->getFirstChild(); param;)
    {
        auto nextInst = param->getNextInst();
        if (as<IRDecoration>(param) || as<IRParam, IRDynamicCastBehavior::NoUnwrap>(param))
        {
            param->insertAtEnd(dest);
        }
        else
        {
            break;
        }
        param = nextInst;
    }
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRLoop* loop,
    bool* outHasMultiLevelBreaks)
{
    return collectBlocksInRegion(
        dom,
        loop->getBreakBlock(),
        loop->getTargetBlock(),
        true,
        outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(IRDominatorTree* dom, IRLoop* loop)
{
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(
        dom,
        loop->getBreakBlock(),
        loop->getTargetBlock(),
        true,
        &hasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRSwitch* switchInst,
    bool* outHasMultiLevelBreaks)
{
    return collectBlocksInRegion(
        dom,
        switchInst->getBreakLabel(),
        as<IRBlock>(switchInst->getParent()),
        false,
        outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(IRDominatorTree* dom, IRSwitch* switchInst)
{
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(
        dom,
        switchInst->getBreakLabel(),
        as<IRBlock>(switchInst->getParent()),
        false,
        &hasMultiLevelBreaks);
}

HashSet<IRBlock*> getParentBreakBlockSet(IRDominatorTree* dom, IRBlock* block)
{
    HashSet<IRBlock*> parentBreakBlocksSet;
    for (IRBlock* currBlock = dom->getImmediateDominator(block); currBlock;
         currBlock = dom->getImmediateDominator(currBlock))
    {
        if (auto loopInst = as<IRLoop>(currBlock->getTerminator()))
        {
            if (!dom->dominates(loopInst->getBreakBlock(), block))
                parentBreakBlocksSet.add(loopInst->getBreakBlock());
        }
        else if (auto switchInst = as<IRSwitch>(currBlock->getTerminator()))
        {
            if (!dom->dominates(switchInst->getBreakLabel(), block))
                parentBreakBlocksSet.add(switchInst->getBreakLabel());
        }
    }

    return parentBreakBlocksSet;
}

List<IRBlock*> collectBlocksInRegion(
    IRDominatorTree* dom,
    IRBlock* breakBlock,
    IRBlock* firstBlock,
    bool includeFirstBlock,
    bool* outHasMultiLevelBreaks)
{
    List<IRBlock*> regionBlocks;
    HashSet<IRBlock*> regionBlocksSet;
    auto addBlock = [&](IRBlock* block)
    {
        if (regionBlocksSet.add(block))
            regionBlocks.add(block);
    };

    // Use dominator tree heirarchy to find break blocks of
    // all parent regions. We'll need to this to detect breaks
    // to outer regions (particularly when our region has no reachable
    // break block of its own)
    //
    HashSet<IRBlock*> parentBreakBlocksSet = getParentBreakBlockSet(dom, firstBlock);

    *outHasMultiLevelBreaks = false;

    addBlock(firstBlock);
    for (Index i = 0; i < regionBlocks.getCount(); i++)
    {
        auto block = regionBlocks[i];
        for (auto succ : block->getSuccessors())
        {
            if (parentBreakBlocksSet.contains(succ) && succ != breakBlock)
            {
                *outHasMultiLevelBreaks = true;
                continue;
            }

            if (succ == breakBlock)
                continue;
            if (!dom->dominates(firstBlock, succ))
                continue;
            if (!as<IRUnreachableBase>(breakBlock->getTerminator()))
            {
                if (dom->dominates(breakBlock, succ))
                    continue;
            }

            addBlock(succ);
        }
    }

    if (!includeFirstBlock)
    {
        regionBlocksSet.remove(firstBlock);
        regionBlocks.remove(firstBlock);
    }

    return regionBlocks;
}

List<IRBlock*> collectBlocksInRegion(
    IRGlobalValueWithCode* func,
    IRLoop* loopInst,
    bool* outHasMultiLevelBreaks)
{
    auto dom = computeDominatorTree(func);
    return collectBlocksInRegion(dom, loopInst, outHasMultiLevelBreaks);
}

List<IRBlock*> collectBlocksInRegion(IRGlobalValueWithCode* func, IRLoop* loopInst)
{
    auto dom = computeDominatorTree(func);
    bool hasMultiLevelBreaks = false;
    return collectBlocksInRegion(dom, loopInst, &hasMultiLevelBreaks);
}

IRBlock* getBlock(IRInst* inst)
{
    if (!inst)
        return nullptr;

    while (inst)
    {
        if (auto block = as<IRBlock>(inst))
            return block;
        inst = inst->getParent();
    }
    return nullptr;
}

///
/// End of IRBlock utility methods
///
IRVarLayout* findVarLayout(IRInst* value)
{
    if (auto layoutDecoration = value->findDecoration<IRLayoutDecoration>())
        return as<IRVarLayout>(layoutDecoration->getLayout());
    return nullptr;
}

UnownedStringSlice getBuiltinFuncName(IRInst* callee)
{
    auto decor = getResolvedInstForDecorations(callee)->findDecoration<IRKnownBuiltinDecoration>();
    if (!decor)
        return UnownedStringSlice();

    // For backward compatibility, convert enum back to string
    switch (decor->getName())
    {
    case KnownBuiltinDeclName::GeometryStreamAppend:
        return UnownedStringSlice::fromLiteral("GeometryStreamAppend");
    case KnownBuiltinDeclName::GeometryStreamRestart:
        return UnownedStringSlice::fromLiteral("GeometryStreamRestart");
    case KnownBuiltinDeclName::GetAttributeAtVertex:
        return UnownedStringSlice::fromLiteral("GetAttributeAtVertex");
    case KnownBuiltinDeclName::DispatchMesh:
        return UnownedStringSlice::fromLiteral("DispatchMesh");
    case KnownBuiltinDeclName::saturated_cooperation:
        return UnownedStringSlice::fromLiteral("saturated_cooperation");
    case KnownBuiltinDeclName::saturated_cooperation_using:
        return UnownedStringSlice::fromLiteral("saturated_cooperation_using");
    case KnownBuiltinDeclName::IDifferentiable:
        return UnownedStringSlice::fromLiteral("IDifferentiable");
    case KnownBuiltinDeclName::IDifferentiablePtr:
        return UnownedStringSlice::fromLiteral("IDifferentiablePtr");
    case KnownBuiltinDeclName::NullDifferential:
        return UnownedStringSlice::fromLiteral("NullDifferential");
    default:
        return UnownedStringSlice();
    }
}

KnownBuiltinDeclName getBuiltinFuncEnum(IRInst* callee)
{
    auto decor = getResolvedInstForDecorations(callee)->findDecoration<IRKnownBuiltinDecoration>();
    if (!decor)
        return KnownBuiltinDeclName::COUNT; // Use COUNT as invalid value
    return decor->getName();
}

void hoistInstOutOfASMBlocks(IRBlock* block)
{
    for (auto inst : block->getChildren())
    {
        if (auto asmBlock = as<IRSPIRVAsm>(inst))
        {
            IRInst* next = nullptr;
            for (auto i = asmBlock->getFirstChild(); i; i = next)
            {
                next = i->getNextInst();
                if (!as<IRSPIRVAsmInst>(i) && !as<IRSPIRVAsmOperand>(i))
                    i->insertBefore(asmBlock);
            }
        }
    }
}

IRType* getSPIRVSampledElementType(IRInst* sampledType)
{
    auto sampledElementType = getVectorElementType((IRType*)sampledType);

    IRBuilder builder(sampledType);
    switch (sampledElementType->getOp())
    {
    case kIROp_HalfType:
        sampledElementType = builder.getBasicType(BaseType::Float);
        break;
    case kIROp_UInt16Type:
    case kIROp_UInt8Type:
    case kIROp_CharType:
        sampledElementType = builder.getBasicType(BaseType::UInt);
        break;
    case kIROp_Int8Type:
    case kIROp_Int16Type:
        sampledElementType = builder.getBasicType(BaseType::Int);
        break;
    default:
        break;
    }
    return sampledElementType;
}

IRType* replaceVectorElementType(IRType* originalVectorType, IRType* t)
{
    if (auto orignalVectorType = as<IRVectorType>(originalVectorType))
    {
        IRBuilder builder(originalVectorType);
        return builder.getVectorType(t, orignalVectorType->getElementCount());
    }
    return t;
}

IRParam* getParamAt(IRBlock* block, UIndex ii)
{
    UIndex index = 0;
    for (auto param : block->getParams())
    {
        if (ii == index)
            return param;

        index++;
    }
    SLANG_UNEXPECTED("ii >= paramCount");
}

UnownedStringSlice getBasicTypeNameHint(IRType* basicType)
{
    switch (basicType->getOp())
    {
    case kIROp_IntType:
        return UnownedStringSlice::fromLiteral("int");
    case kIROp_Int8Type:
        return UnownedStringSlice::fromLiteral("int8_t");
    case kIROp_Int16Type:
        return UnownedStringSlice::fromLiteral("int16_t");
    case kIROp_Int64Type:
        return UnownedStringSlice::fromLiteral("int64_t");
    case kIROp_IntPtrType:
        return UnownedStringSlice::fromLiteral("intptr_t");
    case kIROp_UIntType:
        return UnownedStringSlice::fromLiteral("uint");
    case kIROp_UInt8Type:
        return UnownedStringSlice::fromLiteral("uint8_t");
    case kIROp_UInt16Type:
        return UnownedStringSlice::fromLiteral("uint16_t");
    case kIROp_UInt64Type:
        return UnownedStringSlice::fromLiteral("uint64_t");
    case kIROp_UIntPtrType:
        return UnownedStringSlice::fromLiteral("uintptr_t");
    case kIROp_FloatType:
        return UnownedStringSlice::fromLiteral("float");
    case kIROp_HalfType:
        return UnownedStringSlice::fromLiteral("half");
    case kIROp_DoubleType:
        return UnownedStringSlice::fromLiteral("double");
    case kIROp_BoolType:
        return UnownedStringSlice::fromLiteral("bool");
    case kIROp_VoidType:
        return UnownedStringSlice::fromLiteral("void");
    case kIROp_CharType:
        return UnownedStringSlice::fromLiteral("char");
    default:
        return UnownedStringSlice();
    }
}

struct GenericChildrenMigrationContextImpl
{
    IRCloneEnv cloneEnv;
    IRGeneric* srcGeneric;
    IRGeneric* dstGeneric;
    DeduplicateContext deduplicateContext;

    void init(IRGeneric* genericSrc, IRGeneric* genericDst, IRInst* insertBefore)
    {
        srcGeneric = genericSrc;
        dstGeneric = genericDst;

        if (!genericSrc)
            return;
        auto srcParam = genericSrc->getFirstBlock()->getFirstParam();
        auto dstParam = genericDst->getFirstBlock()->getFirstParam();
        while (srcParam && dstParam)
        {
            cloneEnv.mapOldValToNew[srcParam] = dstParam;
            srcParam = srcParam->getNextParam();
            dstParam = dstParam->getNextParam();
        }
        cloneEnv.mapOldValToNew[genericSrc] = genericDst;
        cloneEnv.mapOldValToNew[genericSrc->getFirstBlock()] = genericDst->getFirstBlock();

        if (insertBefore)
        {
            for (auto inst = genericDst->getFirstBlock()->getFirstOrdinaryInst();
                 inst && inst != insertBefore;
                 inst = inst->getNextInst())
            {
                IRInstKey key = {inst};
                deduplicateContext.deduplicateMap.addIfNotExists(key, inst);
            }
        }
    }

    IRInst* deduplicate(IRInst* value)
    {
        return deduplicateContext.deduplicate(
            value,
            [this](IRInst* inst)
            {
                if (inst->getParent() != dstGeneric->getFirstBlock())
                    return false;
                switch (inst->getOp())
                {
                case kIROp_Param:
                case kIROp_StructType:
                case kIROp_StructKey:
                case kIROp_InterfaceType:
                case kIROp_ClassType:
                case kIROp_Func:
                case kIROp_Generic:
                case kIROp_Expand:
                    return false;
                default:
                    break;
                }
                if (as<IRConstant>(inst))
                    return false;
                if (getIROpInfo(inst->getOp()).isHoistable())
                    return false;
                return true;
            });
    }

    IRInst* cloneInst(IRBuilder* builder, IRInst* src)
    {
        if (!srcGeneric)
            return src;
        if (findOuterGeneric(src) == srcGeneric)
        {
            auto cloned = Slang::cloneInst(&cloneEnv, builder, src);
            auto deduplicated = deduplicate(cloned);
            if (deduplicated != cloned)
                cloneEnv.mapOldValToNew[src] = deduplicated;
            return deduplicated;
        }
        return src;
    }
};

GenericChildrenMigrationContext::GenericChildrenMigrationContext()
{
    impl = new GenericChildrenMigrationContextImpl();
}

GenericChildrenMigrationContext::~GenericChildrenMigrationContext()
{
    delete impl;
}

IRCloneEnv* GenericChildrenMigrationContext::getCloneEnv()
{
    return &impl->cloneEnv;
}

void GenericChildrenMigrationContext::init(
    IRGeneric* genericSrc,
    IRGeneric* genericDst,
    IRInst* insertBefore)
{
    impl->init(genericSrc, genericDst, insertBefore);
}

IRInst* GenericChildrenMigrationContext::deduplicate(IRInst* value)
{
    return impl->deduplicate(value);
}

IRInst* GenericChildrenMigrationContext::cloneInst(IRBuilder* builder, IRInst* src)
{
    return impl->cloneInst(builder, src);
}

IRType* dropNormAttributes(IRType* const t)
{
    if (const auto a = as<IRAttributedType>(t))
    {
        switch (a->getAttr()->getOp())
        {
        case kIROp_UNormAttr:
        case kIROp_SNormAttr:
            return dropNormAttributes(a->getBaseType());
        }
    }
    return t;
}

void verifyComputeDerivativeGroupModifiers(
    DiagnosticSink* sink,
    SourceLoc errorLoc,
    bool quadAttr,
    bool linearAttr,
    IRNumThreadsDecoration* numThreadsDecor)
{
    if (!numThreadsDecor)
        return;

    if (quadAttr && linearAttr)
    {
        sink->diagnose(errorLoc, Diagnostics::onlyOneOfDerivativeGroupLinearOrQuadCanBeSet);
    }

    IRIntegerValue x = 1;
    IRIntegerValue y = 1;
    IRIntegerValue z = 1;
    if (numThreadsDecor->getX())
        x = numThreadsDecor->getX()->getValue();
    if (numThreadsDecor->getY())
        y = numThreadsDecor->getY()->getValue();
    if (numThreadsDecor->getZ())
        z = numThreadsDecor->getZ()->getValue();

    if (quadAttr)
    {
        if (x % 2 != 0 || y % 2 != 0)
            sink->diagnose(errorLoc, Diagnostics::derivativeGroupQuadMustBeMultiple2ForXYThreads);
    }
    else if (linearAttr)
    {
        if ((x * y * z) % 4 != 0)
            sink->diagnose(
                errorLoc,
                Diagnostics::derivativeGroupLinearMustBeMultiple4ForTotalThreadCount);
    }
}

int getIRVectorElementSize(IRType* type)
{
    if (type->getOp() != kIROp_VectorType)
        return 1;
    return (int)(as<IRIntLit>(as<IRVectorType>(type)->getElementCount())->value.intVal);
}
IRType* getIRVectorBaseType(IRType* type)
{
    if (type->getOp() != kIROp_VectorType)
        return type;
    return as<IRVectorType>(type)->getElementType();
}

IRType* getElementType(IRBuilder& builder, IRType* valueType)
{
    valueType = (IRType*)unwrapAttributedType(valueType);
    if (auto arrayType = as<IRArrayTypeBase>(valueType))
    {
        return arrayType->getElementType();
    }
    else if (auto vectorType = as<IRVectorType>(valueType))
    {
        return vectorType->getElementType();
    }
    else if (auto basicType = as<IRBasicType>(valueType))
    {
        return basicType;
    }
    else if (auto coopVecType = as<IRCoopVectorType>(valueType))
    {
        return coopVecType->getElementType();
    }
    else if (auto matrixType = as<IRMatrixType>(valueType))
    {
        return builder.getVectorType(matrixType->getElementType(), matrixType->getColumnCount());
    }
    else if (auto coopMatType = as<IRCoopMatrixType>(valueType))
    {
        return coopMatType->getElementType();
    }
    else if (auto hlslInputPatchType = as<IRHLSLInputPatchType>(valueType))
    {
        return hlslInputPatchType->getElementType();
    }
    return nullptr;
}

IRType* getFieldType(IRType* valueType, IRStructKey* key)
{
    valueType = (IRType*)unwrapAttributedType(valueType);
    if (auto structType = as<IRStructType>(valueType))
    {
        for (auto field : structType->getFields())
        {
            if (field->getKey() == key)
            {
                return field->getFieldType();
            }
        }
    }
    return nullptr;
}

Int getSpecializationConstantId(IRGlobalParam* param)
{
    auto layout = findVarLayout(param);
    if (!layout)
        return 0;

    auto offset = layout->findOffsetAttr(LayoutResourceKind::SpecializationConstant);
    if (!offset)
        return 0;

    return offset->getOffset();
}

IRBlock* getLoopHeaderForConditionBlock(IRBlock* block)
{
    // Go through uses and check if any of them are a loop condition block.
    for (auto use = block->firstUse; use; use = use->nextUse)
    {
        if (auto loop = as<IRLoop>(use->getUser()))
        {
            if (loop->getTargetBlock() == block)
                return cast<IRBlock>(loop->getParent());
        }
    }
    return nullptr;
}

void legalizeDefUse(IRGlobalValueWithCode* func)
{
    auto dom = computeDominatorTree(func);

    // Make a map of loop condition blocks to their loop header.
    // We need this because we'll be treating loop condition blocks as
    // special cases (they are the special blocks since they "dominate" themselves,
    // in the dominator tree sense)
    //
    Dictionary<IRBlock*, IRBlock*> loopHeaderBlockMap;
    for (auto block : func->getBlocks())
    {
        if (auto header = getLoopHeaderForConditionBlock(block))
            loopHeaderBlockMap.add(block, header);
    }

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getModifiableChildren())
        {
            // Inspect all uses of `inst` and find the common dominator of all use sites.
            IRBlock* commonDominator = block;
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                auto userBlock = as<IRBlock>(use->getUser()->getParent());
                if (!userBlock)
                    continue;
                while (commonDominator && !dom->dominates(commonDominator, userBlock))
                {
                    commonDominator = dom->getImmediateDominator(commonDominator);
                }
            }
            SLANG_ASSERT(commonDominator);

            // If commonDominator is 'block' and if the inst is not a Var in
            // a loop condition block, we can skip the legalization.
            //
            if (commonDominator == block &&
                !(as<IRVar>(inst) && loopHeaderBlockMap.containsKey(block)))
                continue;

            // Normally, if the common dominator is not `block`, we can simply move the
            // definition to the common dominator. An exception is when the common dominator is
            // the target block of a loop. Another exception is when a var in the loop condition
            // block is accessed both inside and outside the loop. It is technically visible,
            // but effects on the 'var' are not visible outside the loop, so we'll need to hoist
            // it out of the loop.
            //
            // Note that after normalization, loops are in the form of:
            // ```
            // loop { if (condition) block; else break; }
            // ```
            // If we find ourselves needing to make the inst available right before
            // the `if`, it means we are seeing uses of the inst outside the loop.
            // In this case, we should insert a var/move the inst before the loop
            // instead of before the `if`. This situation can occur in the IR if
            // the original code is lowered from a `do-while` loop.
            //
            bool shouldInitializeVar = false;
            if (loopHeaderBlockMap.containsKey(commonDominator))
            {
                bool shouldMoveToHeader = false;

                // Check that the break-block dominates any of the uses are past the break
                // block
                for (auto _use = inst->firstUse; _use; _use = _use->nextUse)
                {
                    if (dom->dominates(
                            as<IRLoop>(loopHeaderBlockMap[commonDominator]->getTerminator())
                                ->getBreakBlock(),
                            _use->getUser()->getParent()))
                    {
                        shouldMoveToHeader = true;
                        break;
                    }
                }
                if (shouldMoveToHeader)
                {
                    commonDominator = loopHeaderBlockMap[commonDominator];
                    shouldInitializeVar = true;
                }
            }

            // Now we can legalize uses based on the type of `inst`.
            if (auto var = as<IRVar>(inst))
            {
                // If inst is an var, this is easy, we just move it to the
                // common dominator.
                if (var->getParent() != commonDominator)
                    var->insertBefore(commonDominator->getTerminator());

                if (shouldInitializeVar)
                {
                    IRBuilder builder(func);
                    builder.setInsertAfter(var);
                    builder.emitStore(
                        var,
                        builder.emitDefaultConstruct(
                            as<IRPtrTypeBase>(var->getDataType())->getValueType()));
                }
            }
            else
            {
                // For all other insts, we need to create a local var for it,
                // and replace all uses with a load from the local var.
                IRBuilder builder(func);
                builder.setInsertBefore(commonDominator->getTerminator());
                IRVar* tempVar = builder.emitVar(inst->getFullType());
                auto defaultVal = builder.emitDefaultConstruct(inst->getFullType());
                builder.emitStore(tempVar, defaultVal);

                builder.setInsertAfter(inst);
                builder.emitStore(tempVar, inst);

                traverseUses(
                    inst,
                    [&](IRUse* use)
                    {
                        auto userBlock = as<IRBlock>(use->getUser()->getParent());
                        if (!userBlock)
                            return;
                        // Only fix the use of the current definition of `inst` does not
                        // dominate it.
                        if (!dom->dominates(block, userBlock))
                        {
                            // Replace the use with a load of tempVar.
                            builder.setInsertBefore(use->getUser());
                            auto load = builder.emitLoad(tempVar);
                            builder.replaceOperand(use, load);
                        }
                    });
            }
        }
    }
}

UnownedStringSlice getMangledName(IRInst* inst)
{
    for (auto decor : inst->getDecorations())
    {
        if (auto linkageDecor = as<IRLinkageDecoration>(decor))
            return linkageDecor->getMangledName();
    }
    return UnownedStringSlice();
}

bool isFirstBlock(IRInst* inst)
{
    auto block = as<IRBlock>(inst);
    if (!block)
        return false;
    if (!block->getParent())
        return false;
    return block->getParent()->getFirstBlock() == block;
}

bool isSpecConstRateType(IRType* type)
{
    if (auto rateQualifiedType = as<IRRateQualifiedType>(type))
    {
        if (as<IRSpecConstRate>(rateQualifiedType->getRate()))
        {
            return true;
        }
    }
    return false;
}

IRType* maybeAddRateType(IRBuilder* builder, IRType* rateQulifiedType, IRType* oldType)
{
    if (as<IRRateQualifiedType>(oldType))
    {
        return oldType;
    }

    if (isSpecConstRateType(rateQulifiedType))
    {
        return builder->getRateQualifiedType(builder->getSpecConstRate(), oldType);
    }
    return oldType;
}

bool canOperationBeSpecConst(IROp op, IRType* resultType, IRInst* const* fixedArgs, IRUse* operands)
{
    // Returns true for ops that can be declared as an operation under `OpSpecConstantOp`.
    //
    // Integer arithmetic and comparison operations can be `OpSpecConstantOp` with the `Shader`
    // capability, while floating-point arithmetic and comparison operations require the
    // `Kernel` capability. We only support `Shader` capability for now, return false when
    // floating-point arithmetic/comparison is encountered.
    switch (op)
    {
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_Neg:
        return !isFloatingType(resultType);

    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Leq:
    case kIROp_Geq:
    case kIROp_Less:
    case kIROp_Greater:
        {
            IRInst* operand1;
            IRInst* operand2;
            if (fixedArgs)
            {
                operand1 = fixedArgs[0];
                operand2 = fixedArgs[1];
            }
            else
            {
                operand1 = operands[0].get();
                operand2 = operands[1].get();
            }
            return !isFloatingType(operand1->getDataType()) &&
                   !isFloatingType(operand2->getDataType());
        }

    case kIROp_Not:
    case kIROp_IRem:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_BitNot:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_Select:
        return true;

    default:
        return false;
    }
}

bool isSpecConstOpHoistable(IROp op, IRType* type, IRInst* const* fixedArgs)
{
    auto rateType = as<IRRateQualifiedType>(type);
    return rateType && as<IRSpecConstRate>(rateType->getRate()) &&
           canOperationBeSpecConst(op, rateType->getValueType(), fixedArgs, nullptr);
}


bool isInstHoistable(IROp op, IRType* type, IRInst* const* fixedArgs)
{
    return (getIROpInfo(op).flags & kIROpFlag_Hoistable) ||
           isSpecConstOpHoistable(op, type, fixedArgs);
}

IRType* getUnsignedTypeFromSignedType(IRBuilder* builder, IRType* type)
{
    SLANG_RELEASE_ASSERT(isSignedType(type));

    auto elementType = getVectorOrCoopMatrixElementType(type);

    IROp op = type->getOp();
    switch (op)
    {
    case kIROp_MatrixType:
        {
            auto unsignedTypeOp = getOppositeSignIntTypeOp(elementType->getOp());
            auto matType = as<IRMatrixType>(type);
            SLANG_RELEASE_ASSERT(matType);
            return builder->getMatrixType(
                builder->getType(unsignedTypeOp),
                matType->getRowCount(),
                matType->getColumnCount(),
                matType->getLayout());
        }
    case kIROp_VectorType:
        {
            auto unsignedTypeOp = getOppositeSignIntTypeOp(elementType->getOp());
            auto vecType = as<IRVectorType>(type);
            SLANG_RELEASE_ASSERT(vecType);
            return builder->getVectorType(
                builder->getType(unsignedTypeOp),
                vecType->getElementCount());
        }
    case kIROp_IntType:
    case kIROp_Int16Type:
    case kIROp_Int64Type:
    case kIROp_Int8Type:
        return builder->getType(getOppositeSignIntTypeOp(elementType->getOp()));
    default:
        return type;
    }
}

bool isSignedType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_FloatType:
    case kIROp_DoubleType:
        return true;
    case kIROp_IntType:
    case kIROp_Int16Type:
    case kIROp_Int64Type:
    case kIROp_Int8Type:
        return true;
    case kIROp_VectorType:
        return isSignedType(as<IRVectorType>(type)->getElementType());
    case kIROp_MatrixType:
        return isSignedType(as<IRMatrixType>(type)->getElementType());
    default:
        return false;
    }
}

bool isIROpaqueType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_TextureType:
    case kIROp_GLSLImageType:
    case kIROp_SamplerStateType:
    case kIROp_SamplerComparisonStateType:
    case kIROp_SubpassInputType:
    case kIROp_RaytracingAccelerationStructureType:
    case kIROp_RayQueryType:
    case kIROp_HitObjectType:
        return true;
    default:
        return false;
    }
}

bool isPointerToImmutableLocation(IRInst* loc)
{
    switch (loc->getOp())
    {
    case kIROp_GetStructuredBufferPtr:
    case kIROp_RWStructuredBufferGetElementPtr:
    case kIROp_ImageSubscript:
        return isPointerToImmutableLocation(loc->getOperand(0));
    default:
        break;
    }

    auto type = loc->getDataType();
    if (!type)
        return false;

    switch (type->getOp())
    {
    case kIROp_HLSLStructuredBufferType:
    case kIROp_HLSLByteAddressBufferType:
    case kIROp_ConstantBufferType:
    case kIROp_ParameterBlockType:
        return true;
    default:
        break;
    }

    if (auto textureType = as<IRTextureType>(type))
        return textureType->getAccess() == SLANG_RESOURCE_ACCESS_READ;

    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        switch (ptrType->getAddressSpace())
        {
        case AddressSpace::BuiltinInput:
        case AddressSpace::Input:
        case AddressSpace::MetalObjectData:
        case AddressSpace::Uniform:
        case AddressSpace::UniformConstant:
            return true;
        }
        if (ptrType->getAccessQualifier() == AccessQualifier::Immutable)
            return true;
    }
    return false;
}

bool isGenericParameter(IRInst* inst)
{
    // The generic parameter must be in the first block
    bool isParam = inst->getOp() == kIROp_Param;
    bool isGeneric = false;
    if (auto irBlock = as<IRBlock>(inst->parent))
    {
        isGeneric = as<IRGeneric>(irBlock->getParent()) != nullptr;
    }
    return isParam && isGeneric;
}

bool canRelaxInstOrderRule(IRInst* inst, IRInst* useOfInst)
{
    bool isSameBlock = (inst->getParent() == useOfInst->getParent());
    return isSameBlock && isGenericParameter(useOfInst) && (useOfInst->getDataType() == inst);
}

} // namespace Slang
