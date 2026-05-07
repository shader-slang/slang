// slang-ir-legalize-cuda-surface-format.cpp
#include "slang-ir-legalize-cuda-surface-format.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

// Forward declaration from slang-ir-resolve-texture-format.cpp
// We reuse the same type-replacement logic.
static IRType* replaceImageElementType(IRInst* originalType, IRInst* newElementType)
{
    switch (originalType->getOp())
    {
    case kIROp_ArrayType:
    case kIROp_UnsizedArrayType:
    case kIROp_PtrType:
    case kIROp_OutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_BorrowInOutParamType:
        {
            auto newInnerType =
                replaceImageElementType(originalType->getOperand(0), newElementType);
            if (newInnerType != originalType->getOperand(0))
            {
                IRBuilder builder(originalType);
                builder.setInsertBefore(originalType);
                IRCloneEnv cloneEnv;
                cloneEnv.mapOldValToNew.add(originalType->getOperand(0), newInnerType);
                return (IRType*)cloneInst(&cloneEnv, &builder, originalType);
            }
            return (IRType*)originalType;
        }
    default:
        if (as<IRResourceTypeBase>(originalType))
            return (IRType*)newElementType;
        return (IRType*)originalType;
    }
}

struct CUDASurfaceFormatLegalizer
{
    IRModule* m_module;
    DiagnosticSink* m_sink;
    IRBuilder m_builder;

    // Determine the effective format for a texture resource instruction.
    // Checks IRFormatDecoration first (higher priority), then falls back to
    // the texture type's format operand.
    ImageFormat getEffectiveFormat(IRInst* textureInst)
    {
        if (auto decor = findImageFormatDecoration(textureInst))
            return decor->getFormat();

        // Fall back to the texture type's format operand.
        auto textureType = as<IRTextureTypeBase>(textureInst->getDataType());
        if (!textureType)
        {
            // Might be an array of textures.
            if (auto arrayType = as<IRArrayTypeBase>(textureInst->getDataType()))
                textureType = as<IRTextureTypeBase>(arrayType->getElementType());
        }
        if (textureType && textureType->hasFormat())
        {
            auto format = (ImageFormat)textureType->getFormat();
            if (format != ImageFormat::unknown)
                return format;
        }
        return ImageFormat::unknown;
    }

    // Check if a format actually needs conversion (i.e. the raw storage type
    // differs from the element access type).
    bool needsConversion(IRInst* textureInst, ImageFormat format)
    {
        auto textureType = as<IRTextureTypeBase>(textureInst->getDataType());
        if (!textureType)
        {
            if (auto arrayType = as<IRArrayTypeBase>(textureInst->getDataType()))
                textureType = as<IRTextureTypeBase>(arrayType->getElementType());
        }
        if (!textureType)
            return false;

        IRType* elementType = textureType->getElementType();
        return !isImageFormatCompatible(format, elementType);
    }

    // Check if the format is compatible with the access element type.
    // This mirrors _isImageFormatCompatible in slang-intrinsic-expand.cpp.
    static bool isImageFormatCompatible(ImageFormat imageFormat, IRType* dataType)
    {
        int numElems = 1;
        if (auto vecType = as<IRVectorType>(dataType))
        {
            numElems = int(getIntVal(vecType->getElementCount()));
            dataType = vecType->getElementType();
        }

        BaseType baseType = BaseType::Void;
        if (auto basicType = as<IRBasicType>(dataType))
            baseType = basicType->getBaseType();

        const auto& info = getImageFormatInfo(imageFormat);

        if (numElems != info.channelCount)
            return false;

        BaseType formatBaseType = BaseType::Void;
        switch (info.scalarType)
        {
        case SLANG_SCALAR_TYPE_INT32:
            formatBaseType = BaseType::Int;
            break;
        case SLANG_SCALAR_TYPE_UINT32:
            formatBaseType = BaseType::UInt;
            break;
        case SLANG_SCALAR_TYPE_INT16:
            formatBaseType = BaseType::Int16;
            break;
        case SLANG_SCALAR_TYPE_UINT16:
            formatBaseType = BaseType::UInt16;
            break;
        case SLANG_SCALAR_TYPE_INT8:
            formatBaseType = BaseType::Int8;
            break;
        case SLANG_SCALAR_TYPE_UINT8:
            formatBaseType = BaseType::UInt8;
            break;
        case SLANG_SCALAR_TYPE_FLOAT16:
            formatBaseType = BaseType::Half;
            break;
        case SLANG_SCALAR_TYPE_FLOAT32:
            formatBaseType = BaseType::Float;
            break;
        case SLANG_SCALAR_TYPE_FLOAT64:
            formatBaseType = BaseType::Double;
            break;
        default:
            formatBaseType = BaseType::Void;
            break;
        }

        return formatBaseType == baseType;
    }

    // Get the IR base type for the raw storage of a given image format.
    BaseType getStorageBaseType(ImageFormat format)
    {
        // SNORM formats are listed as UINT8/UINT16 in the format defs, but we
        // need signed types so that sitofp works correctly during decode.
        auto kind = getConversionKind(format);
        if (kind == FormatConversionKind::Snorm8)
            return BaseType::Int8;
        if (kind == FormatConversionKind::Snorm16)
            return BaseType::Int16;

        const auto& info = getImageFormatInfo(format);
        switch (info.scalarType)
        {
        case SLANG_SCALAR_TYPE_UINT8:
            return BaseType::UInt8;
        case SLANG_SCALAR_TYPE_INT8:
            return BaseType::Int8;
        case SLANG_SCALAR_TYPE_UINT16:
            return BaseType::UInt16;
        case SLANG_SCALAR_TYPE_INT16:
            return BaseType::Int16;
        case SLANG_SCALAR_TYPE_UINT32:
            return BaseType::UInt;
        case SLANG_SCALAR_TYPE_INT32:
            return BaseType::Int;
        case SLANG_SCALAR_TYPE_FLOAT16:
            return BaseType::UInt16; // stored as uint16, bitcast to/from half
        case SLANG_SCALAR_TYPE_FLOAT32:
            return BaseType::Float;
        case SLANG_SCALAR_TYPE_FLOAT64:
            return BaseType::Double;
        default:
            return BaseType::Void;
        }
    }

    // Build the IR type for the raw storage of a given image format.
    // e.g. rgba8 -> uchar4, r16f -> uint16_t, rg16f -> ushort2
    IRType* getStorageType(ImageFormat format)
    {
        const auto& info = getImageFormatInfo(format);
        BaseType baseType = getStorageBaseType(format);
        if (baseType == BaseType::Void)
            return nullptr;

        IRType* scalarType = m_builder.getBasicType(baseType);
        if (info.channelCount == 1)
            return scalarType;
        return m_builder.getVectorType(
            scalarType,
            m_builder.getIntValue(m_builder.getIntType(), info.channelCount));
    }

    // Create a new texture type with a different element type.
    IRType* createRawTextureType(IRType* originalType, IRType* newElementType)
    {
        // Handle array-of-texture types.
        if (auto arrayType = as<IRArrayType>(originalType))
        {
            auto innerTexType = as<IRTextureTypeBase>(arrayType->getElementType());
            if (innerTexType)
            {
                auto newTexType = createRawTextureTypeFromTexture(innerTexType, newElementType);
                return m_builder.getArrayType(newTexType, arrayType->getElementCount());
            }
        }

        if (auto texType = as<IRTextureTypeBase>(originalType))
            return createRawTextureTypeFromTexture(texType, newElementType);

        return originalType;
    }

    IRType* createRawTextureTypeFromTexture(IRTextureTypeBase* texType, IRType* newElementType)
    {
        // Create a format operand for "unknown" (no format decoration needed after pass).
        auto formatArg =
            m_builder.getIntValue(m_builder.getUIntType(), IRIntegerValue(ImageFormat::unknown));

        return m_builder.getTextureType(
            newElementType,
            texType->getShapeInst(),
            texType->getIsArrayInst(),
            texType->getIsMultisampleInst(),
            texType->getSampleCountInst(),
            texType->getAccessInst(),
            texType->getIsShadowInst(),
            texType->getIsCombinedInst(),
            formatArg);
    }

    // Check if a function body contains a GenericAsm with a surface intrinsic string.
    // Returns the asm definition string if found, empty otherwise.
    UnownedStringSlice findSurfaceAsmInBody(IRInst* func)
    {
        for (auto block : as<IRFunc>(func)->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (inst->getOp() == kIROp_GenericAsm)
                {
                    auto asmInst = as<IRGenericAsm>(inst);
                    auto asmStr = asmInst->getAsm();
                    if (asmStr.indexOf(toSlice("surf")) != Index(-1))
                        return asmStr;
                }
            }
        }
        return UnownedStringSlice();
    }

    // Check if a call is a CUDA surface read by inspecting either the callee's
    // target intrinsic decoration or the body of a [ForceInline] function.
    bool isSurfaceReadCall(IRCall* call, IRInst*& outTexture)
    {
        auto callee = getResolvedInstForDecorations(call->getCallee());

        // Check target intrinsic decoration (post-inlining case).
        if (auto intrinsicDecor = findAnyTargetIntrinsicDecoration(callee))
        {
            auto defStr = intrinsicDecor->getDefinition();
            if (defStr.indexOf(toSlice("surf")) != Index(-1) &&
                defStr.indexOf(toSlice("read$C")) != Index(-1))
            {
                outTexture = call->getArg(0);
                return true;
            }
        }

        // Check function body (pre-inlining case).
        if (auto func = as<IRFunc>(callee))
        {
            auto asmStr = findSurfaceAsmInBody(func);
            if (asmStr.getLength() > 0 && asmStr.indexOf(toSlice("read$C")) != Index(-1))
            {
                outTexture = call->getArg(0);
                return true;
            }
        }

        return false;
    }

    // Check if a call is a CUDA surface write by inspecting either the callee's
    // target intrinsic decoration or the body of a [ForceInline] function.
    bool isSurfaceWriteCall(IRCall* call, IRInst*& outTexture, IRInst*& outValue)
    {
        auto callee = getResolvedInstForDecorations(call->getCallee());

        // Check target intrinsic decoration (post-inlining case).
        if (auto intrinsicDecor = findAnyTargetIntrinsicDecoration(callee))
        {
            auto defStr = intrinsicDecor->getDefinition();
            if (defStr.indexOf(toSlice("surf")) != Index(-1) &&
                defStr.indexOf(toSlice("write$C")) != Index(-1))
            {
                outTexture = call->getArg(0);
                outValue = call->getArg(2);
                return true;
            }
        }

        // Check function body (pre-inlining case).
        if (auto func = as<IRFunc>(callee))
        {
            auto asmStr = findSurfaceAsmInBody(func);
            if (asmStr.getLength() > 0 && asmStr.indexOf(toSlice("write$C")) != Index(-1))
            {
                outTexture = call->getArg(0);
                outValue = call->getArg(2);
                return true;
            }
        }

        return false;
    }

    // Propagate a type change through users (Load, GetElement, Store, etc.)
    // following the pattern from resolveTextureFormatForParameter.
    void updateTextureType(IRInst* textureInst, IRType* newTextureType)
    {
        List<IRUse*> workList;
        HashSet<IRUse*> workListSet;

        auto newInstType =
            (IRType*)replaceImageElementType(textureInst->getFullType(), newTextureType);
        textureInst->setFullType(newInstType);

        for (auto use = textureInst->firstUse; use; use = use->nextUse)
        {
            if (workListSet.add(use))
                workList.add(use);
        }

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto use = workList[i];
            auto user = use->getUser();
            switch (user->getOp())
            {
            case kIROp_GetElementPtr:
            case kIROp_GetElement:
            case kIROp_Load:
            case kIROp_Var:
                {
                    auto newUserType =
                        (IRType*)replaceImageElementType(user->getFullType(), newTextureType);
                    if (newUserType != user->getFullType())
                    {
                        user->setFullType(newUserType);
                        for (auto u = user->firstUse; u; u = u->nextUse)
                        {
                            if (workListSet.add(u))
                                workList.add(u);
                        }
                    }
                    break;
                }
            case kIROp_Store:
                {
                    auto store = as<IRStore>(user);
                    if (use == store->getValUse())
                    {
                        auto ptr = store->getPtr();
                        auto newPtrType =
                            (IRType*)replaceImageElementType(ptr->getFullType(), newTextureType);
                        if (newPtrType != ptr->getFullType())
                        {
                            ptr->setFullType(newPtrType);
                            for (auto u = ptr->firstUse; u; u = u->nextUse)
                            {
                                if (workListSet.add(u))
                                    workList.add(u);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    // Classification of format conversion semantics.
    enum class FormatConversionKind
    {
        None,       // No conversion (32-bit float/int/uint, or format matches access type)
        Float16,    // half precision float stored as uint16
        Unorm8,     // [0,1] float stored as uint8
        Unorm16,    // [0,1] float stored as uint16
        Snorm8,     // [-1,1] float stored as int8
        Snorm16,    // [-1,1] float stored as int16
        Int8,       // signed 8-bit → sign-extend to int32
        Int16,      // signed 16-bit → sign-extend to int32
        Uint8,      // unsigned 8-bit → zero-extend to uint32
        Uint16,     // unsigned 16-bit → zero-extend to uint32
        Bgra8Unorm, // UNORM8 with B↔R channel swap
        Packed,     // Unsupported packed formats
    };

    FormatConversionKind getConversionKind(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::rgba8:
        case ImageFormat::rg8:
        case ImageFormat::r8:
            return FormatConversionKind::Unorm8;
        case ImageFormat::rgba16:
        case ImageFormat::rg16:
        case ImageFormat::r16:
            return FormatConversionKind::Unorm16;
        case ImageFormat::rgba8_snorm:
        case ImageFormat::rg8_snorm:
        case ImageFormat::r8_snorm:
            return FormatConversionKind::Snorm8;
        case ImageFormat::rgba16_snorm:
        case ImageFormat::rg16_snorm:
        case ImageFormat::r16_snorm:
            return FormatConversionKind::Snorm16;
        case ImageFormat::rgba16f:
        case ImageFormat::rg16f:
        case ImageFormat::r16f:
            return FormatConversionKind::Float16;
        case ImageFormat::rgba8i:
        case ImageFormat::rg8i:
        case ImageFormat::r8i:
            return FormatConversionKind::Int8;
        case ImageFormat::rgba16i:
        case ImageFormat::rg16i:
        case ImageFormat::r16i:
            return FormatConversionKind::Int16;
        case ImageFormat::rgba8ui:
        case ImageFormat::rg8ui:
        case ImageFormat::r8ui:
            return FormatConversionKind::Uint8;
        case ImageFormat::rgba16ui:
        case ImageFormat::rg16ui:
        case ImageFormat::r16ui:
            return FormatConversionKind::Uint16;
        case ImageFormat::bgra8:
            return FormatConversionKind::Bgra8Unorm;
        case ImageFormat::rgb10_a2:
        case ImageFormat::rgb10_a2ui:
        case ImageFormat::r11f_g11f_b10f:
            return FormatConversionKind::Packed;
        default:
            return FormatConversionKind::None;
        }
    }

    // Emit decode conversion: raw storage value -> access type value.
    IRInst* emitDecode(ImageFormat format, IRInst* rawValue, IRType* accessType)
    {
        const auto& info = getImageFormatInfo(format);
        auto kind = getConversionKind(format);

        switch (kind)
        {
        case FormatConversionKind::Float16:
            return emitDecodeFloat16(rawValue, accessType, info.channelCount);
        case FormatConversionKind::Unorm8:
            return emitDecodeUnorm(rawValue, accessType, info.channelCount, 255.0);
        case FormatConversionKind::Unorm16:
            return emitDecodeUnorm(rawValue, accessType, info.channelCount, 65535.0);
        case FormatConversionKind::Snorm8:
            return emitDecodeSnorm(rawValue, accessType, info.channelCount, 127.0);
        case FormatConversionKind::Snorm16:
            return emitDecodeSnorm(rawValue, accessType, info.channelCount, 32767.0);
        case FormatConversionKind::Int8:
        case FormatConversionKind::Int16:
            return emitDecodeSignedInt(rawValue, accessType, info.channelCount);
        case FormatConversionKind::Uint8:
        case FormatConversionKind::Uint16:
            return emitDecodeUnsignedInt(rawValue, accessType, info.channelCount);
        case FormatConversionKind::Bgra8Unorm:
            return emitDecodeBgra8(rawValue, accessType);
        default:
            return rawValue;
        }
    }

    // Emit encode conversion: access type value -> raw storage value.
    IRInst* emitEncode(ImageFormat format, IRInst* accessValue, IRType* storageType)
    {
        const auto& info = getImageFormatInfo(format);
        auto kind = getConversionKind(format);

        switch (kind)
        {
        case FormatConversionKind::Float16:
            return emitEncodeFloat16(accessValue, storageType, info.channelCount);
        case FormatConversionKind::Unorm8:
            return emitEncodeUnorm(accessValue, storageType, info.channelCount, 255.0);
        case FormatConversionKind::Unorm16:
            return emitEncodeUnorm(accessValue, storageType, info.channelCount, 65535.0);
        case FormatConversionKind::Snorm8:
            return emitEncodeSnorm(accessValue, storageType, info.channelCount, 127.0);
        case FormatConversionKind::Snorm16:
            return emitEncodeSnorm(accessValue, storageType, info.channelCount, 32767.0);
        case FormatConversionKind::Int8:
        case FormatConversionKind::Int16:
            return emitEncodeSignedInt(accessValue, storageType, info.channelCount);
        case FormatConversionKind::Uint8:
        case FormatConversionKind::Uint16:
            return emitEncodeUnsignedInt(accessValue, storageType, info.channelCount);
        case FormatConversionKind::Bgra8Unorm:
            return emitEncodeBgra8(accessValue, storageType);
        default:
            return accessValue;
        }
    }

    // FLOAT16 decode: bitcast<half>(uint16_raw), then convert half -> float.
    // For multi-channel: extract each channel, convert, re-assemble.
    IRInst* emitDecodeFloat16(IRInst* rawValue, IRType* accessType, int channelCount)
    {
        auto halfType = m_builder.getBasicType(BaseType::Half);
        auto floatType = m_builder.getBasicType(BaseType::Float);

        if (channelCount == 1)
        {
            // Scalar: bitcast uint16 -> half, then half -> float
            auto halfVal = m_builder.emitBitCast(halfType, rawValue);
            return m_builder.emitCast(floatType, halfVal);
        }

        // Vector: extract each channel, convert, reassemble
        List<IRInst*> channels;
        for (int i = 0; i < channelCount; i++)
        {
            auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
            auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
            auto halfVal = m_builder.emitBitCast(halfType, rawChannel);
            auto floatVal = m_builder.emitCast(floatType, halfVal);
            channels.add(floatVal);
        }

        return m_builder.emitMakeVector(accessType, (UInt)channelCount, channels.getBuffer());
    }

    // FLOAT16 encode: float -> half, then bitcast<uint16>(half).
    IRInst* emitEncodeFloat16(IRInst* accessValue, IRType* storageType, int channelCount)
    {
        auto halfType = m_builder.getBasicType(BaseType::Half);
        auto uint16Type = m_builder.getBasicType(BaseType::UInt16);

        if (channelCount == 1)
        {
            auto halfVal = m_builder.emitCast(halfType, accessValue);
            return m_builder.emitBitCast(uint16Type, halfVal);
        }

        List<IRInst*> channels;
        for (int i = 0; i < channelCount; i++)
        {
            auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
            auto floatChannel = m_builder.emitElementExtract(accessValue, idx);
            auto halfVal = m_builder.emitCast(halfType, floatChannel);
            auto uint16Val = m_builder.emitBitCast(uint16Type, halfVal);
            channels.add(uint16Val);
        }

        return m_builder.emitMakeVector(storageType, (UInt)channelCount, channels.getBuffer());
    }

    // Helper: get the number of channels in an access type (vector width, or 1 for scalar).
    int getAccessChannelCount(IRType* accessType)
    {
        if (auto vecType = as<IRVectorType>(accessType))
            return int(getIntVal(vecType->getElementCount()));
        return 1;
    }

    // Helper: get the scalar element type of an access type.
    IRType* getAccessScalarType(IRType* accessType)
    {
        if (auto vecType = as<IRVectorType>(accessType))
            return vecType->getElementType();
        return accessType;
    }

    // Helper: build a vector or scalar result from decoded channels, handling channel
    // count mismatch. If format has fewer channels than access type, pad with (0,0,0,1).
    IRInst* buildAccessResult(
        List<IRInst*>& decodedChannels,
        int formatChannelCount,
        IRType* accessType)
    {
        int accessChannels = getAccessChannelCount(accessType);
        IRType* scalarType = getAccessScalarType(accessType);

        if (accessChannels == 1)
        {
            // Scalar access - just return the first decoded channel.
            return decodedChannels[0];
        }

        // Build full vector, padding if needed.
        List<IRInst*> finalChannels;
        for (int i = 0; i < accessChannels; i++)
        {
            if (i < formatChannelCount)
            {
                finalChannels.add(decodedChannels[i]);
            }
            else
            {
                // Pad: (0, 0, 0, 1) pattern - alpha channel (index 3) defaults to 1.
                IRFloatingPointValue padVal = (i == 3) ? 1.0 : 0.0;
                finalChannels.add(m_builder.getFloatValue(scalarType, padVal));
            }
        }

        return m_builder.emitMakeVector(
            accessType,
            (UInt)accessChannels,
            finalChannels.getBuffer());
    }

    // Helper: extract the first N channels from the access value for encoding.
    // If access type has more channels than format, truncate. If fewer, read what's available.
    void extractChannelsForEncode(
        IRInst* accessValue,
        IRType* accessType,
        int formatChannelCount,
        List<IRInst*>& outChannels)
    {
        int accessChannels = getAccessChannelCount(accessType);
        int channelsToExtract = (formatChannelCount < accessChannels) ? formatChannelCount
                                                                      : accessChannels;

        if (accessChannels == 1)
        {
            outChannels.add(accessValue);
            return;
        }

        for (int i = 0; i < channelsToExtract; i++)
        {
            auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
            outChannels.add(m_builder.emitElementExtract(accessValue, idx));
        }
    }

    // UNORM decode: uitofp(raw) * (1.0 / maxVal)
    IRInst* emitDecodeUnorm(IRInst* rawValue, IRType* accessType, int formatChannels, double maxVal)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scale = m_builder.getFloatValue(floatType, 1.0 / maxVal);

        List<IRInst*> decodedChannels;
        if (formatChannels == 1)
        {
            auto floatVal = m_builder.emitCast(floatType, rawValue);
            decodedChannels.add(m_builder.emitMul(floatType, floatVal, scale));
        }
        else
        {
            for (int i = 0; i < formatChannels; i++)
            {
                auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
                auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
                auto floatVal = m_builder.emitCast(floatType, rawChannel);
                decodedChannels.add(m_builder.emitMul(floatType, floatVal, scale));
            }
        }

        return buildAccessResult(decodedChannels, formatChannels, accessType);
    }

    // UNORM encode: fptoui(saturate(val) * maxVal + 0.5)
    IRInst* emitEncodeUnorm(
        IRInst* accessValue,
        IRType* storageType,
        int formatChannels,
        double maxVal)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scaleConst = m_builder.getFloatValue(floatType, maxVal);
        auto halfConst = m_builder.getFloatValue(floatType, 0.5);
        auto zeroConst = m_builder.getFloatValue(floatType, 0.0);
        auto oneConst = m_builder.getFloatValue(floatType, 1.0);

        IRType* storageScalarType;
        if (auto vecType = as<IRVectorType>(storageType))
            storageScalarType = vecType->getElementType();
        else
            storageScalarType = storageType;

        List<IRInst*> encodedChannels;
        List<IRInst*> inputChannels;
        extractChannelsForEncode(
            accessValue,
            accessValue->getDataType(),
            formatChannels,
            inputChannels);

        for (int i = 0; i < formatChannels; i++)
        {
            IRInst* ch =
                (i < (int)inputChannels.getCount()) ? inputChannels[i] : zeroConst;
            // saturate: max(0, min(1, val))
            auto cmpLt0 = m_builder.emitLess(ch, zeroConst);
            IRInst* clampLowArgs[] = {cmpLt0, zeroConst, ch};
            auto clampLow = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampLowArgs);
            auto cmpGt1 = m_builder.emitLess(oneConst, clampLow);
            IRInst* clampHighArgs[] = {cmpGt1, oneConst, clampLow};
            auto saturated = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampHighArgs);
            // val * maxVal + 0.5
            auto scaled = m_builder.emitMul(floatType, saturated, scaleConst);
            auto biased = m_builder.emitAdd(floatType, scaled, halfConst);
            // Convert to unsigned integer storage type
            auto encoded = m_builder.emitCast(storageScalarType, biased);
            encodedChannels.add(encoded);
        }

        if (formatChannels == 1)
            return encodedChannels[0];
        return m_builder.emitMakeVector(
            storageType,
            (UInt)formatChannels,
            encodedChannels.getBuffer());
    }

    // SNORM decode: max(sitofp(raw) * (1.0 / maxVal), -1.0)
    IRInst* emitDecodeSnorm(
        IRInst* rawValue,
        IRType* accessType,
        int formatChannels,
        double maxVal)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scale = m_builder.getFloatValue(floatType, 1.0 / maxVal);
        auto negOne = m_builder.getFloatValue(floatType, -1.0);

        List<IRInst*> decodedChannels;
        if (formatChannels == 1)
        {
            // sitofp (signed int to float)
            auto floatVal = m_builder.emitCast(floatType, rawValue);
            auto scaled = m_builder.emitMul(floatType, floatVal, scale);
            // max(scaled, -1.0)
            auto cmpLt = m_builder.emitLess(scaled, negOne);
            IRInst* selectArgs[] = {cmpLt, negOne, scaled};
            decodedChannels.add(
                m_builder.emitIntrinsicInst(floatType, kIROp_Select, 3, selectArgs));
        }
        else
        {
            for (int i = 0; i < formatChannels; i++)
            {
                auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
                auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
                auto floatVal = m_builder.emitCast(floatType, rawChannel);
                auto scaled = m_builder.emitMul(floatType, floatVal, scale);
                auto cmpLt = m_builder.emitLess(scaled, negOne);
                IRInst* selectArgs[] = {cmpLt, negOne, scaled};
                decodedChannels.add(
                    m_builder.emitIntrinsicInst(floatType, kIROp_Select, 3, selectArgs));
            }
        }

        return buildAccessResult(decodedChannels, formatChannels, accessType);
    }

    // SNORM encode: fptosi(clamp(val, -1, 1) * maxVal + copysign(0.5, val))
    // We use round-to-nearest: fptosi(clamp(val,-1,1) * maxVal + 0.5) for positive,
    // fptosi(clamp(val,-1,1) * maxVal - 0.5) for negative. Simplified: add sign-aware 0.5.
    // Actually for correctness: fptosi(round(clamp(val,-1,1) * maxVal))
    // which is equivalent to fptosi(clamp(val,-1,1) * maxVal + (val >= 0 ? 0.5 : -0.5))
    IRInst* emitEncodeSnorm(
        IRInst* accessValue,
        IRType* storageType,
        int formatChannels,
        double maxVal)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scaleConst = m_builder.getFloatValue(floatType, maxVal);
        auto negOne = m_builder.getFloatValue(floatType, -1.0);
        auto posOne = m_builder.getFloatValue(floatType, 1.0);
        auto halfConst = m_builder.getFloatValue(floatType, 0.5);
        auto negHalfConst = m_builder.getFloatValue(floatType, -0.5);
        auto zeroConst = m_builder.getFloatValue(floatType, 0.0);

        IRType* storageScalarType;
        if (auto vecType = as<IRVectorType>(storageType))
            storageScalarType = vecType->getElementType();
        else
            storageScalarType = storageType;

        List<IRInst*> encodedChannels;
        List<IRInst*> inputChannels;
        extractChannelsForEncode(
            accessValue,
            accessValue->getDataType(),
            formatChannels,
            inputChannels);

        for (int i = 0; i < formatChannels; i++)
        {
            IRInst* ch =
                (i < (int)inputChannels.getCount()) ? inputChannels[i] : zeroConst;
            // clamp(ch, -1, 1)
            auto cmpLtNeg1 = m_builder.emitLess(ch, negOne);
            IRInst* clampLowArgs[] = {cmpLtNeg1, negOne, ch};
            auto clampLow = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampLowArgs);
            auto cmpGt1 = m_builder.emitLess(posOne, clampLow);
            IRInst* clampHighArgs[] = {cmpGt1, posOne, clampLow};
            auto clamped = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampHighArgs);
            // scale
            auto scaled = m_builder.emitMul(floatType, clamped, scaleConst);
            // Round to nearest: add +0.5 if >= 0, -0.5 if < 0
            auto isNeg = m_builder.emitLess(scaled, zeroConst);
            IRInst* biasArgs[] = {isNeg, negHalfConst, halfConst};
            auto bias =
                m_builder.emitIntrinsicInst(floatType, kIROp_Select, 3, biasArgs);
            auto biased = m_builder.emitAdd(floatType, scaled, bias);
            // Convert to signed integer storage type (truncates toward zero)
            auto encoded = m_builder.emitCast(storageScalarType, biased);
            encodedChannels.add(encoded);
        }

        if (formatChannels == 1)
            return encodedChannels[0];
        return m_builder.emitMakeVector(
            storageType,
            (UInt)formatChannels,
            encodedChannels.getBuffer());
    }

    // Signed integer decode: sign-extend from int8/int16 to int32
    IRInst* emitDecodeSignedInt(IRInst* rawValue, IRType* accessType, int formatChannels)
    {
        auto intType = m_builder.getBasicType(BaseType::Int);
        int accessChannels = getAccessChannelCount(accessType);

        List<IRInst*> decodedChannels;
        if (formatChannels == 1)
        {
            // Cast from int8/int16 to int32 (sign extension)
            decodedChannels.add(m_builder.emitCast(intType, rawValue));
        }
        else
        {
            for (int i = 0; i < formatChannels; i++)
            {
                auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
                auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
                decodedChannels.add(m_builder.emitCast(intType, rawChannel));
            }
        }

        if (accessChannels == 1)
            return decodedChannels[0];

        // Pad remaining channels with 0 (integer default: 0,0,0,1 for alpha)
        List<IRInst*> finalChannels;
        for (int i = 0; i < accessChannels; i++)
        {
            if (i < formatChannels)
                finalChannels.add(decodedChannels[i]);
            else
                finalChannels.add(
                    m_builder.getIntValue(intType, (i == 3) ? 1 : 0));
        }

        return m_builder.emitMakeVector(
            accessType,
            (UInt)accessChannels,
            finalChannels.getBuffer());
    }

    // Unsigned integer decode: zero-extend from uint8/uint16 to uint32
    IRInst* emitDecodeUnsignedInt(IRInst* rawValue, IRType* accessType, int formatChannels)
    {
        auto uintType = m_builder.getBasicType(BaseType::UInt);
        int accessChannels = getAccessChannelCount(accessType);

        List<IRInst*> decodedChannels;
        if (formatChannels == 1)
        {
            decodedChannels.add(m_builder.emitCast(uintType, rawValue));
        }
        else
        {
            for (int i = 0; i < formatChannels; i++)
            {
                auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
                auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
                decodedChannels.add(m_builder.emitCast(uintType, rawChannel));
            }
        }

        if (accessChannels == 1)
            return decodedChannels[0];

        List<IRInst*> finalChannels;
        for (int i = 0; i < accessChannels; i++)
        {
            if (i < formatChannels)
                finalChannels.add(decodedChannels[i]);
            else
                finalChannels.add(
                    m_builder.getIntValue(uintType, (i == 3) ? 1 : 0));
        }

        return m_builder.emitMakeVector(
            accessType,
            (UInt)accessChannels,
            finalChannels.getBuffer());
    }

    // Signed integer encode: truncate from int32 to int8/int16
    IRInst* emitEncodeSignedInt(IRInst* accessValue, IRType* storageType, int formatChannels)
    {
        IRType* storageScalarType;
        if (auto vecType = as<IRVectorType>(storageType))
            storageScalarType = vecType->getElementType();
        else
            storageScalarType = storageType;

        List<IRInst*> encodedChannels;
        List<IRInst*> inputChannels;
        extractChannelsForEncode(
            accessValue,
            accessValue->getDataType(),
            formatChannels,
            inputChannels);

        auto zeroVal = m_builder.getIntValue(storageScalarType, 0);
        for (int i = 0; i < formatChannels; i++)
        {
            IRInst* ch = (i < (int)inputChannels.getCount()) ? inputChannels[i] : zeroVal;
            encodedChannels.add(m_builder.emitCast(storageScalarType, ch));
        }

        if (formatChannels == 1)
            return encodedChannels[0];
        return m_builder.emitMakeVector(
            storageType,
            (UInt)formatChannels,
            encodedChannels.getBuffer());
    }

    // Unsigned integer encode: truncate from uint32 to uint8/uint16
    IRInst* emitEncodeUnsignedInt(IRInst* accessValue, IRType* storageType, int formatChannels)
    {
        IRType* storageScalarType;
        if (auto vecType = as<IRVectorType>(storageType))
            storageScalarType = vecType->getElementType();
        else
            storageScalarType = storageType;

        List<IRInst*> encodedChannels;
        List<IRInst*> inputChannels;
        extractChannelsForEncode(
            accessValue,
            accessValue->getDataType(),
            formatChannels,
            inputChannels);

        auto zeroVal = m_builder.getIntValue(storageScalarType, 0);
        for (int i = 0; i < formatChannels; i++)
        {
            IRInst* ch = (i < (int)inputChannels.getCount()) ? inputChannels[i] : zeroVal;
            encodedChannels.add(m_builder.emitCast(storageScalarType, ch));
        }

        if (formatChannels == 1)
            return encodedChannels[0];
        return m_builder.emitMakeVector(
            storageType,
            (UInt)formatChannels,
            encodedChannels.getBuffer());
    }

    // BGRA8 decode: UNORM8 decode with B↔R channel swap (indices 0↔2).
    IRInst* emitDecodeBgra8(IRInst* rawValue, IRType* accessType)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scale = m_builder.getFloatValue(floatType, 1.0 / 255.0);
        int formatChannels = 4;

        // Decode all 4 channels as UNORM8
        List<IRInst*> decodedChannels;
        for (int i = 0; i < formatChannels; i++)
        {
            auto idx = m_builder.getIntValue(m_builder.getIntType(), i);
            auto rawChannel = m_builder.emitElementExtract(rawValue, idx);
            auto floatVal = m_builder.emitCast(floatType, rawChannel);
            decodedChannels.add(m_builder.emitMul(floatType, floatVal, scale));
        }

        // Swizzle: BGRA → RGBA (swap indices 0 and 2)
        auto tmp = decodedChannels[0];
        decodedChannels[0] = decodedChannels[2];
        decodedChannels[2] = tmp;

        return buildAccessResult(decodedChannels, formatChannels, accessType);
    }

    // BGRA8 encode: UNORM8 encode with B↔R channel swap (indices 0↔2).
    IRInst* emitEncodeBgra8(IRInst* accessValue, IRType* storageType)
    {
        auto floatType = m_builder.getBasicType(BaseType::Float);
        auto scaleConst = m_builder.getFloatValue(floatType, 255.0);
        auto halfConst = m_builder.getFloatValue(floatType, 0.5);
        auto zeroConst = m_builder.getFloatValue(floatType, 0.0);
        auto oneConst = m_builder.getFloatValue(floatType, 1.0);
        int formatChannels = 4;

        IRType* storageScalarType;
        if (auto vecType = as<IRVectorType>(storageType))
            storageScalarType = vecType->getElementType();
        else
            storageScalarType = storageType;

        // Extract input channels (RGBA order)
        List<IRInst*> inputChannels;
        extractChannelsForEncode(
            accessValue,
            accessValue->getDataType(),
            formatChannels,
            inputChannels);

        // Pad with defaults if needed
        while ((int)inputChannels.getCount() < formatChannels)
        {
            int idx = (int)inputChannels.getCount();
            inputChannels.add(m_builder.getFloatValue(floatType, (idx == 3) ? 1.0 : 0.0));
        }

        // Swap R↔B (index 0 and 2) for BGRA layout
        auto tmp = inputChannels[0];
        inputChannels[0] = inputChannels[2];
        inputChannels[2] = tmp;

        // Encode each channel as UNORM8
        List<IRInst*> encodedChannels;
        for (int i = 0; i < formatChannels; i++)
        {
            auto ch = inputChannels[i];
            // saturate
            auto cmpLt0 = m_builder.emitLess(ch, zeroConst);
            IRInst* clampLowArgs[] = {cmpLt0, zeroConst, ch};
            auto clampLow = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampLowArgs);
            auto cmpGt1 = m_builder.emitLess(oneConst, clampLow);
            IRInst* clampHighArgs[] = {cmpGt1, oneConst, clampLow};
            auto saturated = m_builder.emitIntrinsicInst(
                floatType,
                kIROp_Select,
                3,
                clampHighArgs);
            auto scaled = m_builder.emitMul(floatType, saturated, scaleConst);
            auto biased = m_builder.emitAdd(floatType, scaled, halfConst);
            encodedChannels.add(m_builder.emitCast(storageScalarType, biased));
        }

        return m_builder.emitMakeVector(
            storageType,
            (UInt)formatChannels,
            encodedChannels.getBuffer());
    }
    void rewriteSurfaceReadCall(IRCall* originalCall, ImageFormat format, IRType* storageType)
    {
        // The original call returns the access type (e.g. float4).
        // We need to change its return type to the storage type (e.g. ushort4),
        // then insert decode instructions after it.
        IRType* originalReturnType = originalCall->getFullType();

        // Collect all existing uses of the original call before modifying anything.
        List<IRUse*> originalUses;
        for (auto use = originalCall->firstUse; use; use = use->nextUse)
            originalUses.add(use);

        // Change the call's return type to the raw storage type.
        originalCall->setFullType(storageType);

        // Insert conversion after the call.
        m_builder.setInsertAfter(originalCall);
        IRInst* convertedResult = emitDecode(format, originalCall, originalReturnType);

        if (convertedResult != originalCall)
        {
            // Replace only the pre-existing uses with the converted result.
            // The decode chain instructions already reference originalCall correctly.
            for (auto use : originalUses)
                use->set(convertedResult);
        }
    }

    // Rewrite a surface write call to encode + use raw storage type.
    void rewriteSurfaceWriteCall(IRCall* originalCall, ImageFormat format, IRType* storageType)
    {
        // Write intrinsic asm: "surf2Dwrite$C<$T0>($2, $0, ($1).x * $E, ...)"
        // $0 = this (texture), $1 = coord, $2 = newValue
        // So the value to write is getArg(2).
        IRInst* originalValue = originalCall->getArg(2);

        // Insert encode before the call.
        m_builder.setInsertBefore(originalCall);
        IRInst* encodedValue = emitEncode(format, originalValue, storageType);

        // Replace the value operand in the call.
        if (encodedValue != originalValue)
        {
            // Arg(2) is operand index 3 (operand 0 is callee, 1-N are args).
            originalCall->setOperand(3, encodedValue);
        }
    }

    // Collect surface calls by traversing a 2-level use chain:
    // startInst -> (load/user) -> (call)
    void collectSurfaceCalls(
        IRInst* startInst,
        List<IRCall*>& readsToRewrite,
        List<IRCall*>& writesToRewrite,
        int depth = 0)
    {
        if (depth > 4)
            return;
        for (auto use = startInst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto call = as<IRCall>(user))
            {
                IRInst* textureArg;
                IRInst* valueArg;
                if (isSurfaceReadCall(call, textureArg))
                    readsToRewrite.add(call);
                else if (isSurfaceWriteCall(call, textureArg, valueArg))
                    writesToRewrite.add(call);
            }
            else
            {
                // Recurse through loads, gets, etc.
                collectSurfaceCalls(user, readsToRewrite, writesToRewrite, depth + 1);
            }
        }
    }

    // Emit a diagnostic for unsupported packed texture formats.
    void diagnoseUnsupportedFormat(ImageFormat format, IRInst* inst)
    {
        const auto& info = getImageFormatInfo(format);
        m_sink->diagnose(
            Diagnostics::UnsupportedCudaTextureFormat{.format = info.name, .location = inst->sourceLoc});
    }

    // Process a single direct global texture param: find all its surface calls,
    // rewrite types and calls, then remove the format decoration.
    void processTexture(IRInst* textureInst, ImageFormat format)
    {
        if (getConversionKind(format) == FormatConversionKind::Packed)
        {
            diagnoseUnsupportedFormat(format, textureInst);
            return;
        }

        IRType* storageType = getStorageType(format);
        if (!storageType)
            return;

        List<IRCall*> readsToRewrite;
        List<IRCall*> writesToRewrite;
        collectSurfaceCalls(textureInst, readsToRewrite, writesToRewrite);

        if (readsToRewrite.getCount() == 0 && writesToRewrite.getCount() == 0)
            return;

        IRType* newTextureType = createRawTextureType(textureInst->getDataType(), storageType);
        updateTextureType(textureInst, newTextureType);

        for (auto call : readsToRewrite)
            rewriteSurfaceReadCall(call, format, storageType);
        for (auto call : writesToRewrite)
            rewriteSurfaceWriteCall(call, format, storageType);

        if (auto decoration = textureInst->findDecoration<IRFormatDecoration>())
            decoration->removeFromParent();
    }

    // Process textures that are struct fields. The format decoration lives on the
    // struct field key. Access pattern: fieldKey -> get_field_addr -> load -> call.
    void processStructFieldTexture(IRInst* fieldKey, ImageFormat format)
    {
        if (getConversionKind(format) == FormatConversionKind::Packed)
        {
            diagnoseUnsupportedFormat(format, fieldKey);
            return;
        }

        IRType* storageType = getStorageType(format);
        if (!storageType)
            return;

        List<IRCall*> readsToRewrite;
        List<IRCall*> writesToRewrite;
        List<IRInst*> fieldAddresses;

        // Traverse: fieldKey -> field_addr(s) -> load(s) -> call(s)
        for (auto keyUse = fieldKey->firstUse; keyUse; keyUse = keyUse->nextUse)
        {
            auto fieldAddr = as<IRFieldAddress>(keyUse->getUser());
            if (!fieldAddr)
                continue;
            fieldAddresses.add(fieldAddr);
            collectSurfaceCalls(fieldAddr, readsToRewrite, writesToRewrite);
        }

        if (readsToRewrite.getCount() == 0 && writesToRewrite.getCount() == 0)
            return;

        // Find the original texture type from a field address (unwrap Ptr).
        IRTextureTypeBase* origTexType = nullptr;
        for (auto fa : fieldAddresses)
        {
            if (auto ptrType = as<IRPtrTypeBase>(fa->getFullType()))
            {
                origTexType = as<IRTextureTypeBase>(ptrType->getValueType());
                if (origTexType)
                    break;
            }
        }
        if (!origTexType)
            return;

        // Check if conversion is actually needed.
        if (isImageFormatCompatible(format, origTexType->getElementType()))
            return;

        // Create new texture type with raw storage element type.
        IRType* newTextureType = createRawTextureTypeFromTexture(origTexType, storageType);

        // Rewrite types for each field address (propagates to loads and their users).
        for (auto fieldAddr : fieldAddresses)
            updateTextureType(fieldAddr, newTextureType);

        for (auto call : readsToRewrite)
            rewriteSurfaceReadCall(call, format, storageType);
        for (auto call : writesToRewrite)
            rewriteSurfaceWriteCall(call, format, storageType);

        if (auto decoration = fieldKey->findDecoration<IRFormatDecoration>())
            decoration->removeFromParent();
    }

    void processModule()
    {
        m_builder = IRBuilder(m_module);

        // Collect textures needing conversion: both direct globals and struct fields.
        List<KeyValuePair<IRInst*, ImageFormat>> directTextures;
        List<KeyValuePair<IRInst*, ImageFormat>> structFieldTextures;

        for (auto globalInst : m_module->getGlobalInsts())
        {
            // Case 1: Direct global texture param.
            IRTextureTypeBase* texType = nullptr;
            if (auto t = as<IRTextureTypeBase>(globalInst->getDataType()))
                texType = t;
            else if (auto arrayType = as<IRArrayTypeBase>(globalInst->getDataType()))
                texType = as<IRTextureTypeBase>(arrayType->getElementType());

            if (texType)
            {
                ImageFormat format = getEffectiveFormat(globalInst);
                if (format != ImageFormat::unknown && needsConversion(globalInst, format))
                    directTextures.add(KeyValuePair<IRInst*, ImageFormat>(globalInst, format));
                continue;
            }

            // Case 2: Struct field key with format decoration.
            // These arise when textures are grouped in a GlobalParams struct.
            if (globalInst->getOp() == kIROp_StructKey)
            {
                auto formatDecor = globalInst->findDecoration<IRFormatDecoration>();
                if (!formatDecor)
                    continue;
                ImageFormat format = formatDecor->getFormat();
                if (format != ImageFormat::unknown)
                    structFieldTextures.add(
                        KeyValuePair<IRInst*, ImageFormat>(globalInst, format));
            }
        }

        // Case 3: Entry point function parameters.
        // For CUDA compute, entry point params are NOT promoted to globals,
        // so we must look inside entry point functions.
        for (auto globalInst : m_module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;
            if (!func->findDecoration<IREntryPointDecoration>())
                continue;

            for (auto param = func->getFirstParam(); param; param = param->getNextParam())
            {
                IRTextureTypeBase* texType = nullptr;
                if (auto t = as<IRTextureTypeBase>(param->getDataType()))
                    texType = t;
                else if (auto arrayType = as<IRArrayTypeBase>(param->getDataType()))
                    texType = as<IRTextureTypeBase>(arrayType->getElementType());

                if (texType)
                {
                    ImageFormat format = getEffectiveFormat(param);
                    if (format != ImageFormat::unknown && needsConversion(param, format))
                        directTextures.add(
                            KeyValuePair<IRInst*, ImageFormat>(param, format));
                }
            }
        }

        for (auto& pair : directTextures)
            processTexture(pair.key, pair.value);
        for (auto& pair : structFieldTextures)
            processStructFieldTexture(pair.key, pair.value);
    }
};

void legalizeCUDASurfaceFormat(IRModule* module, DiagnosticSink* sink)
{
    CUDASurfaceFormatLegalizer legalizer;
    legalizer.m_module = module;
    legalizer.m_sink = sink;
    legalizer.processModule();
}

} // namespace Slang
