// slang-c-like-source-emitter.cpp
#include "slang-c-like-source-emitter.h"

#include "../core/slang-writer.h"
#include "slang-ir-bind-existentials.h"
#include "slang-ir-dce.h"
#include "slang-ir-entry-point-uniforms.h"
#include "slang-ir-glsl-legalize.h"

#include "slang-ir-link.h"
#include "slang-ir-restructure-scoping.h"
#include "slang-ir-specialize.h"
#include "slang-ir-specialize-resources.h"
#include "slang-ir-ssa.h"
#include "slang-ir-union.h"
#include "slang-ir-validate.h"
#include "slang-legalize-types.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-name.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"
#include "slang-visitor.h"

#include "slang-source-stream.h"
#include "slang-emit-context.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

// represents a declarator for use in emitting types
struct CLikeSourceEmitter::EDeclarator
{
    enum class Flavor
    {
        name,
        Array,
        UnsizedArray,
    };
    Flavor flavor;
    EDeclarator* next = nullptr;

    // Used for `Flavor::name`
    Name*       name;
    SourceLoc   loc;

    // Used for `Flavor::Array`
    IRInst* elementCount;
};

struct CLikeSourceEmitter::IRDeclaratorInfo
{
    enum class Flavor
    {
        Simple,
        Ptr,
        Array,
    };

    Flavor flavor;
    IRDeclaratorInfo* next;
    union
    {
        String const* name;
        IRInst* elementCount;
    };
};

// A chain of variables to use for emitting semantic/layout info
struct CLikeSourceEmitter::EmitVarChain
{
    VarLayout*      varLayout;
    EmitVarChain*   next;

    EmitVarChain()
        : varLayout(0)
        , next(0)
    {}

    EmitVarChain(VarLayout* varLayout)
        : varLayout(varLayout)
        , next(0)
    {}

    EmitVarChain(VarLayout* varLayout, EmitVarChain* next)
        : varLayout(varLayout)
        , next(next)
    {}
};

struct CLikeSourceEmitter::ComputeEmitActionsContext
{
    IRInst*             moduleInst;
    HashSet<IRInst*>    openInsts;
    Dictionary<IRInst*, EmitAction::Level> mapInstToLevel;
    List<EmitAction>*   actions;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!! CLikeSourceEmitter !!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */CLikeSourceEmitter::SourceStyle CLikeSourceEmitter::getSourceStyle(CodeGenTarget target)
{
    switch (target)
    {
        default:
        case CodeGenTarget::Unknown:
        case CodeGenTarget::None:
        {
            return SourceStyle::Unknown;
        }
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
        {
            return SourceStyle::GLSL;
        }
        case CodeGenTarget::HLSL:
        {
            return SourceStyle::HLSL;
        }
        case CodeGenTarget::SPIRV:
        case CodeGenTarget::SPIRVAssembly:
        case CodeGenTarget::DXBytecode:
        case CodeGenTarget::DXBytecodeAssembly:
        case CodeGenTarget::DXIL:
        case CodeGenTarget::DXILAssembly:
        {
            return SourceStyle::Unknown;
        }
        case CodeGenTarget::CSource:
        {
            return SourceStyle::C;
        }
        case CodeGenTarget::CPPSource:
        {
            return SourceStyle::CPP;
        }
    }
}

CLikeSourceEmitter::CLikeSourceEmitter(EmitContext* context)
    : m_context(context),
    m_stream(context->stream),
    m_sourceStyle(getSourceStyle(context->target))
{
    SLANG_ASSERT(m_sourceStyle != SourceStyle::Unknown);
}

//
// Types
//

void CLikeSourceEmitter::emitDeclarator(EDeclarator* declarator)
{
    if (!declarator) return;

    m_stream->emit(" ");

    switch (declarator->flavor)
    {
    case EDeclarator::Flavor::name:
        m_stream->emitName(declarator->name, declarator->loc);
        break;

    case EDeclarator::Flavor::Array:
        emitDeclarator(declarator->next);
        m_stream->emit("[");
        if(auto elementCount = declarator->elementCount)
        {
            emitVal(elementCount, getInfo(EmitOp::General));
        }
        m_stream->emit("]");
        break;

    case EDeclarator::Flavor::UnsizedArray:
        emitDeclarator(declarator->next);
        m_stream->emit("[]");
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unknown declarator flavor");
        break;
    }
}

void CLikeSourceEmitter::emitGLSLTypePrefix(IRType* type, bool promoteHalfToFloat)
{
    switch (type->op)
    {
    case kIROp_FloatType:
        // no prefix
        break;

    case kIROp_Int8Type:    m_stream->emit("i8");     break;
    case kIROp_Int16Type:   m_stream->emit("i16");    break;
    case kIROp_IntType:     m_stream->emit("i");      break;
    case kIROp_Int64Type:   m_stream->emit("i64");    break;

    case kIROp_UInt8Type:   m_stream->emit("u8");     break;
    case kIROp_UInt16Type:  m_stream->emit("u16");    break;
    case kIROp_UIntType:    m_stream->emit("u");      break;
    case kIROp_UInt64Type:  m_stream->emit("u64");    break;

    case kIROp_BoolType:    m_stream->emit("b");		break;

    case kIROp_HalfType:
    {
        _requireHalf();
        if (promoteHalfToFloat)
        {
            // no prefix
        }
        else
        {
            m_stream->emit("f16");
        }
        break;
    }
    case kIROp_DoubleType:  m_stream->emit("d");		break;

    case kIROp_VectorType:
        emitGLSLTypePrefix(cast<IRVectorType>(type)->getElementType(), promoteHalfToFloat);
        break;

    case kIROp_MatrixType:
        emitGLSLTypePrefix(cast<IRMatrixType>(type)->getElementType(), promoteHalfToFloat);
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled GLSL type prefix");
        break;
    }
}

void CLikeSourceEmitter::emitHLSLTextureType(IRTextureTypeBase* texType)
{
    switch(texType->getAccess())
    {
    case SLANG_RESOURCE_ACCESS_READ:
        break;

    case SLANG_RESOURCE_ACCESS_READ_WRITE:
        m_stream->emit("RW");
        break;

    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        m_stream->emit("RasterizerOrdered");
        break;

    case SLANG_RESOURCE_ACCESS_APPEND:
        m_stream->emit("Append");
        break;

    case SLANG_RESOURCE_ACCESS_CONSUME:
        m_stream->emit("Consume");
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
        break;
    }

    switch (texType->GetBaseShape())
    {
    case TextureFlavor::Shape::Shape1D:		m_stream->emit("Texture1D");		break;
    case TextureFlavor::Shape::Shape2D:		m_stream->emit("Texture2D");		break;
    case TextureFlavor::Shape::Shape3D:		m_stream->emit("Texture3D");		break;
    case TextureFlavor::Shape::ShapeCube:	m_stream->emit("TextureCube");	break;
    case TextureFlavor::Shape::ShapeBuffer:  m_stream->emit("Buffer");         break;
    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
        break;
    }

    if (texType->isMultisample())
    {
        m_stream->emit("MS");
    }
    if (texType->isArray())
    {
        m_stream->emit("Array");
    }
    m_stream->emit("<");
    emitType(texType->getElementType());
    m_stream->emit(" >");
}

void CLikeSourceEmitter::emitGLSLTextureOrTextureSamplerType(IRTextureTypeBase*  type, char const* baseName)
{
    if (type->getElementType()->op == kIROp_HalfType)
    {
        // Texture access is always as float types if half is specified

    }
    else
    {
        emitGLSLTypePrefix(type->getElementType(), true);
    }

    m_stream->emit(baseName);
    switch (type->GetBaseShape())
    {
    case TextureFlavor::Shape::Shape1D:		m_stream->emit("1D");		break;
    case TextureFlavor::Shape::Shape2D:		m_stream->emit("2D");		break;
    case TextureFlavor::Shape::Shape3D:		m_stream->emit("3D");		break;
    case TextureFlavor::Shape::ShapeCube:	m_stream->emit("Cube");	break;
    case TextureFlavor::Shape::ShapeBuffer:	m_stream->emit("Buffer");	break;
    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
        break;
    }

    if (type->isMultisample())
    {
        m_stream->emit("MS");
    }
    if (type->isArray())
    {
        m_stream->emit("Array");
    }
}

void CLikeSourceEmitter::emitGLSLTextureType(
    IRTextureType* texType)
{
    switch(texType->getAccess())
    {
    case SLANG_RESOURCE_ACCESS_READ_WRITE:
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        emitGLSLTextureOrTextureSamplerType(texType, "image");
        break;

    default:
        emitGLSLTextureOrTextureSamplerType(texType, "texture");
        break;
    }
}

void CLikeSourceEmitter::emitGLSLTextureSamplerType(IRTextureSamplerType* type)
{
    emitGLSLTextureOrTextureSamplerType(type, "sampler");
}

void CLikeSourceEmitter::emitGLSLImageType(IRGLSLImageType* type)
{
    emitGLSLTextureOrTextureSamplerType(type, "image");
}

void CLikeSourceEmitter::emitTextureType(IRTextureType* texType)
{
    switch(m_context->target)
    {
    case CodeGenTarget::HLSL:
        emitHLSLTextureType(texType);
        break;

    case CodeGenTarget::GLSL:
        emitGLSLTextureType(texType);
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
        break;
    }
}

void CLikeSourceEmitter::emitTextureSamplerType(IRTextureSamplerType* type)
{
    switch(m_context->target)
    {
    case CodeGenTarget::GLSL:
        emitGLSLTextureSamplerType(type);
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see combined texture-sampler types");
        break;
    }
}

void CLikeSourceEmitter::emitImageType(IRGLSLImageType* type)
{
    switch(m_context->target)
    {
    case CodeGenTarget::HLSL:
        emitHLSLTextureType(type);
        break;

    case CodeGenTarget::GLSL:
        emitGLSLImageType(type);
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see GLSL image types");
        break;
    }
}

static IROp _getCType(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        {
            return op;
        }
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        {
            // Promote all these to Int
            return kIROp_IntType;
        }
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
        {
            // Promote all these to Int16, we can just vary the call to make these work
            return kIROp_Int64Type;
        }
        case kIROp_DoubleType:
        {
            return kIROp_DoubleType;
        }
        case kIROp_HalfType:
        case kIROp_FloatType:
        {
            // Promote both to float
            return kIROp_FloatType;
        }
        default:
        {
            SLANG_ASSERT(!"Unhandled type");
            return kIROp_undefined;
        }
    }
}

static UnownedStringSlice _getCTypeVecPostFix(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("B");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
    }
}

#if 0
static UnownedStringSlice _getCTypeName(IROp op)
{
    switch (op)
    {
    case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("Bool");
    case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I32");
    case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F32");
    case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
    case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
    default:                    return UnownedStringSlice::fromLiteral("?");
    }
}
#endif

void CLikeSourceEmitter::_emitCVecType(IROp op, Int size)
{
    m_stream->emit("Vec");
    const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
    m_stream->emit(postFix);
    if (postFix.size() > 1)
    {
        m_stream->emit("_");
    }
    m_stream->emit(size);
}

void CLikeSourceEmitter::_emitCMatType(IROp op, IRIntegerValue rowCount, IRIntegerValue colCount)
{
    m_stream->emit("Mat");
    const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
    m_stream->emit(postFix);
    if (postFix.size() > 1)
    {
        m_stream->emit("_");
    }
    m_stream->emit(rowCount);
    m_stream->emit(colCount);
}

void CLikeSourceEmitter::_emitCFunc(BuiltInCOp cop, IRType* type)
{
    _emitSimpleType(type);
    m_stream->emit("_");

    switch (cop)
    {
        case BuiltInCOp::Init:  m_stream->emit("init");
        case BuiltInCOp::Splat: m_stream->emit("splat"); break;
    }
}

void CLikeSourceEmitter::emitVectorTypeName(IRType* elementType, IRIntegerValue elementCount)
{
    switch(m_context->target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::GLSL_Vulkan:
    case CodeGenTarget::GLSL_Vulkan_OneDesc:
        {
            if (elementCount > 1)
            {
                emitGLSLTypePrefix(elementType);
                m_stream->emit("vec");
                m_stream->emit(elementCount);
            }
            else
            {
                _emitSimpleType(elementType);
            }
        }
        break;

    case CodeGenTarget::HLSL:
        // TODO(tfoley): should really emit these with sugar
        m_stream->emit("vector<");
        emitType(elementType);
        m_stream->emit(",");
        m_stream->emit(elementCount);
        m_stream->emit(">");
        break;

    case CodeGenTarget::CSource:
    case CodeGenTarget::CPPSource:
        _emitCVecType(elementType->op, Int(elementCount));
        break;

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
        break;
    }
}

void CLikeSourceEmitter::_emitVectorType(IRVectorType* vecType)
{
    IRInst* elementCountInst = vecType->getElementCount();
    if (elementCountInst->op != kIROp_IntLit)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "Expecting an integral size for vector size");
        return;
    }

    const IRConstant* irConst = (const IRConstant*)elementCountInst;
    const IRIntegerValue elementCount = irConst->value.intVal;
    if (elementCount <= 0)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "Vector size must be greater than 0");
        return;
    }

    auto* elementType = vecType->getElementType();

    emitVectorTypeName(elementType, elementCount);
}

void CLikeSourceEmitter::_emitMatrixType(IRMatrixType* matType)
{
    switch(m_context->target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::GLSL_Vulkan:
    case CodeGenTarget::GLSL_Vulkan_OneDesc:
        {
            emitGLSLTypePrefix(matType->getElementType());
            m_stream->emit("mat");
            emitVal(matType->getRowCount(), getInfo(EmitOp::General));
            // TODO(tfoley): only emit the next bit
            // for non-square matrix
            m_stream->emit("x");
            emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
        }
        break;

    case CodeGenTarget::HLSL:
        // TODO(tfoley): should really emit these with sugar
        m_stream->emit("matrix<");
        emitType(matType->getElementType());
        m_stream->emit(",");
        emitVal(matType->getRowCount(), getInfo(EmitOp::General));
        m_stream->emit(",");
        emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
        m_stream->emit("> ");
        break;

    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CSource:
    {
        const auto rowCount = static_cast<const IRConstant*>(matType->getRowCount())->value.intVal;
        const auto colCount = static_cast<const IRConstant*>(matType->getColumnCount())->value.intVal;

        _emitCMatType(matType->getElementType()->op, rowCount, colCount);
        break;
    }

    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
        break;
    }
}

void CLikeSourceEmitter::emitSamplerStateType(IRSamplerStateTypeBase* samplerStateType)
{
    switch(m_context->target)
    {
    case CodeGenTarget::HLSL:
    default:
        switch (samplerStateType->op)
        {
        case kIROp_SamplerStateType:			m_stream->emit("SamplerState");			break;
        case kIROp_SamplerComparisonStateType:	m_stream->emit("SamplerComparisonState");	break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
            break;
        }
        break;

    case CodeGenTarget::GLSL:
        switch (samplerStateType->op)
        {
        case kIROp_SamplerStateType:			m_stream->emit("sampler");		break;
        case kIROp_SamplerComparisonStateType:	m_stream->emit("samplerShadow");	break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
            break;
        }
        break;
        break;
    }
}

void CLikeSourceEmitter::emitStructuredBufferType(IRHLSLStructuredBufferTypeBase* type)
{
    switch(m_context->target)
    {
    case CodeGenTarget::HLSL:
    default:
        {
            switch (type->op)
            {
            case kIROp_HLSLStructuredBufferType:                    m_stream->emit("StructuredBuffer");                   break;
            case kIROp_HLSLRWStructuredBufferType:                  m_stream->emit("RWStructuredBuffer");                 break;
            case kIROp_HLSLRasterizerOrderedStructuredBufferType:   m_stream->emit("RasterizerOrderedStructuredBuffer");  break;
            case kIROp_HLSLAppendStructuredBufferType:              m_stream->emit("AppendStructuredBuffer");             break;
            case kIROp_HLSLConsumeStructuredBufferType:             m_stream->emit("ConsumeStructuredBuffer");            break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled structured buffer type");
                break;
            }

            m_stream->emit("<");
            emitType(type->getElementType());
            m_stream->emit(" >");
        }
        break;

    case CodeGenTarget::GLSL:
        // TODO: We desugar global variables with structured-buffer type into GLSL
        // `buffer` declarations, but we don't currently handle structured-buffer types
        // in other contexts (e.g., as function parameters). The simplest thing to do
        // would be to emit a `StructuredBuffer<Foo>` as `Foo[]` and `RWStructuredBuffer<Foo>`
        // as `in out Foo[]`, but that is starting to get into the realm of transformations
        // that should really be handled during legalization, rather than during emission.
        //
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "structured buffer type used unexpectedly");
        break;
    }
}

void CLikeSourceEmitter::emitUntypedBufferType(IRUntypedBufferResourceType* type)
{
    switch(m_context->target)
    {
    case CodeGenTarget::HLSL:
    default:
        {
            switch (type->op)
            {
            case kIROp_HLSLByteAddressBufferType:                   m_stream->emit("ByteAddressBuffer");                  break;
            case kIROp_HLSLRWByteAddressBufferType:                 m_stream->emit("RWByteAddressBuffer");                break;
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  m_stream->emit("RasterizerOrderedByteAddressBuffer"); break;
            case kIROp_RaytracingAccelerationStructureType:         m_stream->emit("RaytracingAccelerationStructure");    break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                break;
            }
        }
        break;

    case CodeGenTarget::GLSL:
        {
            switch (type->op)
            {
            case kIROp_RaytracingAccelerationStructureType:
                requireGLSLExtension("GL_NV_ray_tracing");
                m_stream->emit("accelerationStructureNV");
                break;

            // TODO: These "translations" are obviously wrong for GLSL.
            case kIROp_HLSLByteAddressBufferType:                   m_stream->emit("ByteAddressBuffer");                  break;
            case kIROp_HLSLRWByteAddressBufferType:                 m_stream->emit("RWByteAddressBuffer");                break;
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  m_stream->emit("RasterizerOrderedByteAddressBuffer"); break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                break;
            }
        }
        break;
    }
}

void CLikeSourceEmitter::_requireHalf()
{
    if (getSourceStyle() == SourceStyle::GLSL)
    {
        m_context->extensionUsageTracker.requireGLSLHalfExtension();
    }
}

void CLikeSourceEmitter::_emitSimpleType(IRType* type)
{
    switch (type->op)
    {
    default:
        break;

    case kIROp_VoidType:    m_stream->emit("void");       return;
    case kIROp_BoolType:    m_stream->emit("bool");       return;

    case kIROp_Int8Type:    m_stream->emit("int8_t");     return;
    case kIROp_Int16Type:   m_stream->emit("int16_t");    return;
    case kIROp_IntType:     m_stream->emit("int");        return;
    case kIROp_Int64Type:   m_stream->emit("int64_t");    return;

    case kIROp_UInt8Type:   m_stream->emit("uint8_t");    return;
    case kIROp_UInt16Type:  m_stream->emit("uint16_t");   return;
    case kIROp_UIntType:    m_stream->emit("uint");       return;
    case kIROp_UInt64Type:  m_stream->emit("uint64_t");   return;

    case kIROp_HalfType:
    {
        _requireHalf();
        if (getSourceStyle() == SourceStyle::GLSL)
        {
            m_stream->emit("float16_t");
        }
        else
        {
            m_stream->emit("half");
        }
        return;
    }
    case kIROp_FloatType:   m_stream->emit("float");      return;
    case kIROp_DoubleType:  m_stream->emit("double");     return;

    case kIROp_VectorType:
        _emitVectorType((IRVectorType*)type);
        return;

    case kIROp_MatrixType:
        _emitMatrixType((IRMatrixType*)type);
        return;

    case kIROp_SamplerStateType:
    case kIROp_SamplerComparisonStateType:
        emitSamplerStateType(cast<IRSamplerStateTypeBase>(type));
        return;

    case kIROp_StructType:
        m_stream->emit(getIRName(type));
        return;
    }

    // TODO: Ideally the following should be data-driven,
    // based on meta-data attached to the definitions of
    // each of these IR opcodes.

    if (auto texType = as<IRTextureType>(type))
    {
        emitTextureType(texType);
        return;
    }
    else if (auto textureSamplerType = as<IRTextureSamplerType>(type))
    {
        emitTextureSamplerType(textureSamplerType);
        return;
    }
    else if (auto imageType = as<IRGLSLImageType>(type))
    {
        emitImageType(imageType);
        return;
    }
    else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
    {
        emitStructuredBufferType(structuredBufferType);
        return;
    }
    else if(auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        emitUntypedBufferType(untypedBufferType);
        return;
    }

    // HACK: As a fallback for HLSL targets, assume that the name of the
    // instruction being used is the same as the name of the HLSL type.
    if(m_context->target == CodeGenTarget::HLSL)
    {
        auto opInfo = getIROpInfo(type->op);
        m_stream->emit(opInfo.name);
        UInt operandCount = type->getOperandCount();
        if(operandCount)
        {
            m_stream->emit("<");
            for(UInt ii = 0; ii < operandCount; ++ii)
            {
                if(ii != 0) m_stream->emit(", ");
                emitVal(type->getOperand(ii), getInfo(EmitOp::General));
            }
            m_stream->emit(" >");
        }

        return;
    }

    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type");
}

void CLikeSourceEmitter::_emitArrayType(IRArrayType* arrayType, EDeclarator* declarator)
{
    EDeclarator arrayDeclarator;
    arrayDeclarator.flavor = EDeclarator::Flavor::Array;
    arrayDeclarator.next = declarator;
    arrayDeclarator.elementCount = arrayType->getElementCount();

    _emitType(arrayType->getElementType(), &arrayDeclarator);
}

void CLikeSourceEmitter::_emitUnsizedArrayType(IRUnsizedArrayType* arrayType, EDeclarator* declarator)
{
    EDeclarator arrayDeclarator;
    arrayDeclarator.flavor = EDeclarator::Flavor::UnsizedArray;
    arrayDeclarator.next = declarator;

    _emitType(arrayType->getElementType(), &arrayDeclarator);
}

void CLikeSourceEmitter::_emitType(IRType* type, EDeclarator* declarator)
{
    switch (type->op)
    {
    default:
        _emitSimpleType(type);
        emitDeclarator(declarator);
        break;

    case kIROp_RateQualifiedType:
        {
            auto rateQualifiedType = cast<IRRateQualifiedType>(type);
            _emitType(rateQualifiedType->getValueType(), declarator);
        }
        break;

    case kIROp_ArrayType:
        _emitArrayType(cast<IRArrayType>(type), declarator);
        break;

    case kIROp_UnsizedArrayType:
        _emitUnsizedArrayType(cast<IRUnsizedArrayType>(type), declarator);
        break;
    }

}

void CLikeSourceEmitter::emitType(
    IRType*             type,
    SourceLoc const&    typeLoc,
    Name*               name,
    SourceLoc const&    nameLoc)
{
    m_stream->advanceToSourceLocation(typeLoc);

    EDeclarator nameDeclarator;
    nameDeclarator.flavor = EDeclarator::Flavor::name;
    nameDeclarator.name = name;
    nameDeclarator.loc = nameLoc;
    _emitType(type, &nameDeclarator);
}

void CLikeSourceEmitter::emitType(IRType* type, Name* name)
{
    emitType(type, SourceLoc(), name, SourceLoc());
}

void CLikeSourceEmitter::emitType(IRType* type, const String& name)
{
    // HACK: the rest of the code wants a `Name`,
    // so we'll create one for a bit...
    Name tempName;
    tempName.text = name;

    emitType(type, SourceLoc(), &tempName, SourceLoc());
}


void CLikeSourceEmitter::emitType(IRType* type)
{
    _emitType(type, nullptr);
}

//
// Expressions
//

bool CLikeSourceEmitter::maybeEmitParens(EmitOpInfo& outerPrec, EmitOpInfo prec)
{
    bool needParens = (prec.leftPrecedence <= outerPrec.leftPrecedence)
        || (prec.rightPrecedence <= outerPrec.rightPrecedence);

    if (needParens)
    {
        m_stream->emit("(");

        outerPrec = getInfo(EmitOp::None);
    }
    return needParens;
}

void CLikeSourceEmitter::maybeCloseParens(bool needClose)
{
    if(needClose) m_stream->emit(")");
}

bool CLikeSourceEmitter::isTargetIntrinsicModifierApplicable(const String& targetName)
{
    switch(m_context->target)
    {
    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
        return false;

    case CodeGenTarget::CSource: return targetName == "c";
    case CodeGenTarget::CPPSource: return targetName == "cpp";
    case CodeGenTarget::GLSL: return targetName == "glsl";
    case CodeGenTarget::HLSL: return targetName == "hlsl";
    }
}

void CLikeSourceEmitter::emitType(IRType* type, Name* name, SourceLoc const& nameLoc)
{
    emitType(
        type,
        SourceLoc(),
        name,
        nameLoc);
}

void CLikeSourceEmitter::emitType(IRType* type, NameLoc const& nameAndLoc)
{
    emitType(type, nameAndLoc.name, nameAndLoc.loc);
}

bool CLikeSourceEmitter::isTargetIntrinsicModifierApplicable(
    IRTargetIntrinsicDecoration*    decoration)
{
    auto targetName = String(decoration->getTargetName());

    // If no target name was specified, then the modifier implicitly
    // applies to all targets.
    if(targetName.getLength() == 0)
        return true;

    return isTargetIntrinsicModifierApplicable(targetName);
}

void CLikeSourceEmitter::emitStringLiteral(
    String const&   value)
{
    m_stream->emit("\"");
    for (auto c : value)
    {
        // TODO: This needs a more complete implementation,
        // especially if we want to support Unicode.

        char buffer[] = { c, 0 };
        switch (c)
        {
        default:
            m_stream->emit(buffer);
            break;

        case '\"': m_stream->emit("\\\"");
        case '\'': m_stream->emit("\\\'");
        case '\\': m_stream->emit("\\\\");
        case '\n': m_stream->emit("\\n");
        case '\r': m_stream->emit("\\r");
        case '\t': m_stream->emit("\\t");
        }
    }
    m_stream->emit("\"");
}

void CLikeSourceEmitter::requireGLSLExtension(String const& name)
{
    m_context->extensionUsageTracker.requireGLSLExtension(name);
}

void CLikeSourceEmitter::requireGLSLVersion(ProfileVersion version)
{
    if (m_context->target != CodeGenTarget::GLSL)
        return;

    m_context->extensionUsageTracker.requireGLSLVersion(version);
}

void CLikeSourceEmitter::requireGLSLVersion(int version)
{
    switch (version)
    {
#define CASE(NUMBER) \
    case NUMBER: requireGLSLVersion(ProfileVersion::GLSL_##NUMBER); break

    CASE(110);
    CASE(120);
    CASE(130);
    CASE(140);
    CASE(150);
    CASE(330);
    CASE(400);
    CASE(410);
    CASE(420);
    CASE(430);
    CASE(440);
    CASE(450);

#undef CASE
    }
}

void CLikeSourceEmitter::setSampleRateFlag()
{
    m_context->entryPointLayout->flags |= EntryPointLayout::Flag::usesAnySampleRateInput;
}

void CLikeSourceEmitter::doSampleRateInputCheck(Name* name)
{
    auto text = getText(name);
    if (text == "gl_SampleID")
    {
        setSampleRateFlag();
    }
}

void CLikeSourceEmitter::emitVal(IRInst* val, EmitOpInfo const& outerPrec)
{
    if(auto type = as<IRType>(val))
    {
        emitType(type);
    }
    else
    {
        emitIRInstExpr(val, IREmitMode::Default, outerPrec);
    }
}

UInt CLikeSourceEmitter::getBindingOffset(EmitVarChain* chain, LayoutResourceKind kind)
{
    UInt offset = 0;
    for(auto cc = chain; cc; cc = cc->next)
    {
        if(auto resInfo = cc->varLayout->FindResourceInfo(kind))
        {
            offset += resInfo->index;
        }
    }
    return offset;
}

UInt CLikeSourceEmitter::getBindingSpace(EmitVarChain* chain, LayoutResourceKind kind)
{
    UInt space = 0;
    for(auto cc = chain; cc; cc = cc->next)
    {
        auto varLayout = cc->varLayout;
        if(auto resInfo = varLayout->FindResourceInfo(kind))
        {
            space += resInfo->space;
        }
        if(auto resInfo = varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
        {
            space += resInfo->index;
        }
    }
    return space;
}

void CLikeSourceEmitter::emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling)
{
    if(!chain)
        return;
    if(!chain->varLayout->FindResourceInfo(kind))
        return;

    UInt index = getBindingOffset(chain, kind);
    UInt space = getBindingSpace(chain, kind);

    switch(kind)
    {
    case LayoutResourceKind::Uniform:
        {
            UInt offset = index;

            // The HLSL `c` register space is logically grouped in 16-byte registers,
            // while we try to traffic in byte offsets. That means we need to pick
            // a register number, based on the starting offset in 16-byte register
            // units, and then a "component" within that register, based on 4-byte
            // offsets from there. We cannot support more fine-grained offsets than that.

            m_stream->emit(" : ");
            m_stream->emit(uniformSemanticSpelling);
            m_stream->emit("(c");

            // Size of a logical `c` register in bytes
            auto registerSize = 16;

            // Size of each component of a logical `c` register, in bytes
            auto componentSize = 4;

            size_t startRegister = offset / registerSize;
            m_stream->emit(int(startRegister));

            size_t byteOffsetInRegister = offset % registerSize;

            // If this field doesn't start on an even register boundary,
            // then we need to emit additional information to pick the
            // right component to start from
            if (byteOffsetInRegister != 0)
            {
                // The value had better occupy a whole number of components.
                SLANG_RELEASE_ASSERT(byteOffsetInRegister % componentSize == 0);

                size_t startComponent = byteOffsetInRegister / componentSize;

                static const char* kComponentNames[] = {"x", "y", "z", "w"};
                m_stream->emit(".");
                m_stream->emit(kComponentNames[startComponent]);
            }
            m_stream->emit(")");
        }
        break;

    case LayoutResourceKind::RegisterSpace:
    case LayoutResourceKind::GenericResource:
    case LayoutResourceKind::ExistentialTypeParam:
    case LayoutResourceKind::ExistentialObjectParam:
        // ignore
        break;
    default:
        {
            m_stream->emit(" : register(");
            switch( kind )
            {
            case LayoutResourceKind::ConstantBuffer:
                m_stream->emit("b");
                break;
            case LayoutResourceKind::ShaderResource:
                m_stream->emit("t");
                break;
            case LayoutResourceKind::UnorderedAccess:
                m_stream->emit("u");
                break;
            case LayoutResourceKind::SamplerState:
                m_stream->emit("s");
                break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled HLSL register type");
                break;
            }
            m_stream->emit(index);
            if(space)
            {
                m_stream->emit(", space");
                m_stream->emit(space);
            }
            m_stream->emit(")");
        }
    }
}

void CLikeSourceEmitter::emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling)
{
    if (!chain) return;

    auto layout = chain->varLayout;

    switch( m_context->target )
    {
    default:
        return;

    case CodeGenTarget::HLSL:
        break;
    }

    for( auto rr : layout->resourceInfos )
    {
        emitHLSLRegisterSemantic(rr.kind, chain, uniformSemanticSpelling);
    }
}

void CLikeSourceEmitter::emitHLSLRegisterSemantics(VarLayout* varLayout, char const* uniformSemanticSpelling)
{
    if(!varLayout)
        return;

    EmitVarChain chain(varLayout);
    emitHLSLRegisterSemantics(&chain, uniformSemanticSpelling);
}

void CLikeSourceEmitter::emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain)
{
    if(!chain)
        return;

    auto layout = chain->varLayout;
    for( auto rr : layout->resourceInfos )
    {
        emitHLSLRegisterSemantic(rr.kind, chain, "packoffset");
    }
}


void CLikeSourceEmitter::emitHLSLParameterGroupFieldLayoutSemantics(RefPtr<VarLayout> fieldLayout, EmitVarChain* inChain)
{
    EmitVarChain chain(fieldLayout, inChain);
    emitHLSLParameterGroupFieldLayoutSemantics(&chain);
}

bool CLikeSourceEmitter::emitGLSLLayoutQualifier(LayoutResourceKind kind, EmitVarChain* chain)
{
    if(!chain)
        return false;
    if(!chain->varLayout->FindResourceInfo(kind))
        return false;

    UInt index = getBindingOffset(chain, kind);
    UInt space = getBindingSpace(chain, kind);
    switch(kind)
    {
    case LayoutResourceKind::Uniform:
        {
            // Explicit offsets require a GLSL extension (which
            // is not universally supported, it seems) or a new
            // enough GLSL version (which we don't want to
            // universally require), so for right now we
            // won't actually output explicit offsets for uniform
            // shader parameters.
            //
            // TODO: We should fix this so that we skip any
            // extra work for parameters that are laid out as
            // expected by the default rules, but do *something*
            // for parameters that need non-default layout.
            //
            // Using the `GL_ARB_enhanced_layouts` feature is one
            // option, but we should also be able to do some
            // things by introducing padding into the declaration
            // (padding insertion would probably be best done at
            // the IR level).
            bool useExplicitOffsets = false;
            if (useExplicitOffsets)
            {
                requireGLSLExtension("GL_ARB_enhanced_layouts");

                m_stream->emit("layout(offset = ");
                m_stream->emit(index);
                m_stream->emit(")\n");
            }
        }
        break;

    case LayoutResourceKind::VertexInput:
    case LayoutResourceKind::FragmentOutput:
        m_stream->emit("layout(location = ");
        m_stream->emit(index);
        m_stream->emit(")\n");
        break;

    case LayoutResourceKind::SpecializationConstant:
        m_stream->emit("layout(constant_id = ");
        m_stream->emit(index);
        m_stream->emit(")\n");
        break;

    case LayoutResourceKind::ConstantBuffer:
    case LayoutResourceKind::ShaderResource:
    case LayoutResourceKind::UnorderedAccess:
    case LayoutResourceKind::SamplerState:
    case LayoutResourceKind::DescriptorTableSlot:
        m_stream->emit("layout(binding = ");
        m_stream->emit(index);
        if(space)
        {
            m_stream->emit(", set = ");
            m_stream->emit(space);
        }
        m_stream->emit(")\n");
        break;

    case LayoutResourceKind::PushConstantBuffer:
        m_stream->emit("layout(push_constant)\n");
        break;
    case LayoutResourceKind::ShaderRecord:
        m_stream->emit("layout(shaderRecordNV)\n");
        break;

    }
    return true;
}

void CLikeSourceEmitter::emitGLSLLayoutQualifiers(RefPtr<VarLayout> layout, EmitVarChain* inChain, LayoutResourceKind filter)
{
    if(!layout) return;

    switch( getSourceStyle())
    {
    default:
        return;

    case SourceStyle::GLSL:
        break;
    }

    EmitVarChain chain(layout, inChain);

    for( auto info : layout->resourceInfos )
    {
        // Skip info that doesn't match our filter
        if (filter != LayoutResourceKind::None
            && filter != info.kind)
        {
            continue;
        }

        emitGLSLLayoutQualifier(info.kind, &chain);
    }
}

void CLikeSourceEmitter::emitGLSLVersionDirective()
{
    auto effectiveProfile = m_context->effectiveProfile;
    if(effectiveProfile.getFamily() == ProfileFamily::GLSL)
    {
        requireGLSLVersion(effectiveProfile.GetVersion());
    }

    // HACK: We aren't picking GLSL versions carefully right now,
    // and so we might end up only requiring the initial 1.10 version,
    // even though even basic functionality needs a higher version.
    //
    // For now, we'll work around this by just setting the minimum required
    // version to a high one:
    //
    // TODO: Either correctly compute a minimum required version, or require
    // the user to specify a version as part of the target.
    m_context->extensionUsageTracker.requireGLSLVersion(ProfileVersion::GLSL_450);

    auto requiredProfileVersion = m_context->extensionUsageTracker.getRequiredGLSLProfileVersion();
    switch (requiredProfileVersion)
    {
#define CASE(TAG, VALUE)    \
    case ProfileVersion::TAG: m_stream->emit("#version " #VALUE "\n"); return

    CASE(GLSL_110, 110);
    CASE(GLSL_120, 120);
    CASE(GLSL_130, 130);
    CASE(GLSL_140, 140);
    CASE(GLSL_150, 150);
    CASE(GLSL_330, 330);
    CASE(GLSL_400, 400);
    CASE(GLSL_410, 410);
    CASE(GLSL_420, 420);
    CASE(GLSL_430, 430);
    CASE(GLSL_440, 440);
    CASE(GLSL_450, 450);
    CASE(GLSL_460, 460);
#undef CASE

    default:
        break;
    }

    // No information is available for us to guess a profile,
    // so it seems like we need to pick one out of thin air.
    //
    // Ideally we should infer a minimum required version based
    // on the constructs we have seen used in the user's code
    //
    // For now we just fall back to a reasonably recent version.

    m_stream->emit("#version 420\n");
}

void CLikeSourceEmitter::emitGLSLPreprocessorDirectives()
{
    switch(getSourceStyle())
    {
    // Don't emit this stuff unless we are targetting GLSL
    default:
        return;

    case SourceStyle::GLSL:
        break;
    }

    emitGLSLVersionDirective();
}

void CLikeSourceEmitter::emitLayoutDirectives(TargetRequest* targetReq)
{
    // We are going to emit the target-language-specific directives
    // needed to get the default matrix layout to match what was requested
    // for the given target.
    //
    // Note: we do not rely on the defaults for the target language,
    // because a user could take the HLSL/GLSL generated by Slang and pass
    // it to another compiler with non-default options specified on
    // the command line, leading to all kinds of trouble.
    //
    // TODO: We need an approach to "global" layout directives that will work
    // in the presence of multiple modules. If modules A and B were each
    // compiled with different assumptions about how layout is performed,
    // then types/variables defined in those modules should be emitted in
    // a way that is consistent with that layout...

    auto matrixLayoutMode = targetReq->getDefaultMatrixLayoutMode();

    switch(m_context->target)
    {
    default:
        return;

    case CodeGenTarget::GLSL:
        // Reminder: the meaning of row/column major layout
        // in our semantics is the *opposite* of what GLSL
        // calls them, because what they call "columns"
        // are what we call "rows."
        //
        switch(matrixLayoutMode)
        {
        case kMatrixLayoutMode_RowMajor:
        default:
            m_stream->emit("layout(column_major) uniform;\n");
            m_stream->emit("layout(column_major) buffer;\n");
            break;

        case kMatrixLayoutMode_ColumnMajor:
            m_stream->emit("layout(row_major) uniform;\n");
            m_stream->emit("layout(row_major) buffer;\n");
            break;
        }
        break;

    case CodeGenTarget::HLSL:
        switch(matrixLayoutMode)
        {
        case kMatrixLayoutMode_RowMajor:
        default:
            m_stream->emit("#pragma pack_matrix(row_major)\n");
            break;

        case kMatrixLayoutMode_ColumnMajor:
            m_stream->emit("#pragma pack_matrix(column_major)\n");
            break;
        }
        break;
    }
}

UInt CLikeSourceEmitter::allocateUniqueID()
{
    return m_context->uniqueIDCounter++;
}

// IR-level emit logic

UInt CLikeSourceEmitter::getID(IRInst* value)
{
    auto& mapIRValueToID = m_context->mapIRValueToID;

    UInt id = 0;
    if (mapIRValueToID.TryGetValue(value, id))
        return id;

    id = allocateUniqueID();
    mapIRValueToID.Add(value, id);
    return id;
}

/// "Scrub" a name so that it complies with restrictions of the target language.
String CLikeSourceEmitter::scrubName(const String& name)
{
    // We will use a plain `U` as a dummy character to insert
    // whenever we need to insert things to make a string into
    // valid name.
    //
    char const* dummyChar = "U";

    // Special case a name that is the empty string, just in case.
    if(name.getLength() == 0)
        return dummyChar;

    // Otherwise, we are going to walk over the name byte by byte
    // and write some legal characters to the output as we go.
    StringBuilder sb;

    if(getSourceStyle() == SourceStyle::GLSL)
    {
        // GLSL reserverse all names that start with `gl_`,
        // so if we are in danger of collision, then make
        // our name start with a dummy character instead.
        if(name.startsWith("gl_"))
        {
            sb.append(dummyChar);
        }
    }

    // We will also detect user-defined names that
    // might overlap with our convention for mangled names,
    // to avoid an possible collision.
    if(name.startsWith("_S"))
    {
        sb.Append(dummyChar);
    }

    // TODO: This is where we might want to consult
    // a dictionary of reserved words for the chosen target
    //
    //  if(isReservedWord(name)) { sb.Append(dummyChar); }
    //

    // We need to track the previous byte in
    // order to detect consecutive underscores for GLSL.
    int prevChar = -1;

    for(auto c : name)
    {
        // We will treat a dot character just like an underscore
        // for the purposes of producing a scrubbed name, so
        // that we translate `SomeType.someMethod` into
        // `SomeType_someMethod`.
        //
        // By handling this case at the top of this loop, we
        // ensure that a `.`-turned-`_` is handled just like
        // a `_` in the original name, and will be properly
        // scrubbed for GLSL output.
        //
        if(c == '.')
        {
            c = '_';
        }

        if(((c >= 'a') && (c <= 'z'))
            || ((c >= 'A') && (c <= 'Z')))
        {
            // Ordinary ASCII alphabetic characters are assumed
            // to always be okay.
        }
        else if((c >= '0') && (c <= '9'))
        {
            // We don't want to allow a digit as the first
            // byte in a name, since the result wouldn't
            // be a valid identifier in many target languages.
            if(prevChar == -1)
            {
                sb.append(dummyChar);
            }
        }
        else if(c == '_')
        {
            // We will collapse any consecutive sequence of `_`
            // characters into a single one (this means that
            // some names that were unique in the original
            // code might not resolve to unique names after
            // scrubbing, but that was true in general).

            if(prevChar == '_')
            {
                // Skip this underscore, so we don't output
                // more than one in a row.
                continue;
            }
        }
        else
        {
            // If we run into a character that wouldn't normally
            // be allowed in an identifier, we need to translate
            // it into something that *is* valid.
            //
            // Our solution for now will be very clumsy: we will
            // emit `x` and then the hexadecimal version of
            // the byte we were given.
            sb.append("x");
            sb.append(uint32_t((unsigned char) c), 16);

            // We don't want to apply the default handling below,
            // so skip to the top of the loop now.
            prevChar = c;
            continue;
        }

        sb.append(c);
        prevChar = c;
    }

    return sb.ProduceString();
}

String CLikeSourceEmitter::generateIRName(IRInst* inst)
{
    // If the instruction names something
    // that should be emitted as a target intrinsic,
    // then use that name instead.
    if(auto intrinsicDecoration = findTargetIntrinsicDecoration(inst))
    {
        return String(intrinsicDecoration->getDefinition());
    }

    // If we have a name hint on the instruction, then we will try to use that
    // to provide the actual name in the output code.
    //
    // We need to be careful that the name follows the rules of the target language,
    // so there is a "scrubbing" step that needs to be applied here.
    //
    // We also need to make sure that the name won't collide with other declarations
    // that might have the same name hint applied, so we will still unique
    // them by appending the numeric ID of the instruction.
    //
    // TODO: Find cases where we can drop the suffix safely.
    //
    // TODO: When we start having to handle symbols with external linkage for
    // things like DXIL libraries, we will need to *not* use the friendly
    // names for stuff that should be link-able.
    //
    if(auto nameHintDecoration = inst->findDecoration<IRNameHintDecoration>())
    {
        // The name we output will basically be:
        //
        //      <nameHint>_<uniqueID>
        //
        // Except that we will "scrub" the name hint first,
        // and we will omit the underscore if the (scrubbed)
        // name hint already ends with one.
        //

        String nameHint = nameHintDecoration->getName();
        nameHint = scrubName(nameHint);

        StringBuilder sb;
        sb.append(nameHint);

        // Avoid introducing a double underscore
        if(!nameHint.endsWith("_"))
        {
            sb.append("_");
        }

        String key = sb.ProduceString();
        UInt count = 0;
        m_context->uniqueNameCounters.TryGetValue(key, count);

        m_context->uniqueNameCounters[key] = count+1;

        sb.append(Int32(count));
        return sb.ProduceString();
    }

    // If the instruction has a mangled name, then emit using that.
    if(auto linkageDecoration = inst->findDecoration<IRLinkageDecoration>())
    {
        return linkageDecoration->getMangledName();
    }

    // Otherwise fall back to a construct temporary name
    // for the instruction.
    StringBuilder sb;
    sb << "_S";
    sb << Int32(getID(inst));

    return sb.ProduceString();
}

String CLikeSourceEmitter::getIRName(IRInst* inst)
{
    String name;
    if(!m_context->mapInstToName.TryGetValue(inst, name))
    {
        name = generateIRName(inst);
        m_context->mapInstToName.Add(inst, name);
    }
    return name;
}
void CLikeSourceEmitter::emitDeclarator(IRDeclaratorInfo* declarator)
{
    if(!declarator)
        return;

    switch( declarator->flavor )
    {
    case IRDeclaratorInfo::Flavor::Simple:
        m_stream->emit(" ");
        m_stream->emit(*declarator->name);
        break;

    case IRDeclaratorInfo::Flavor::Ptr:
        m_stream->emit("*");
        emitDeclarator(declarator->next);
        break;

    case IRDeclaratorInfo::Flavor::Array:
        emitDeclarator(declarator->next);
        m_stream->emit("[");
        emitIROperand(declarator->elementCount, IREmitMode::Default, getInfo(EmitOp::General));
        m_stream->emit("]");
        break;
    }
}

void CLikeSourceEmitter::emitIRSimpleValue(IRInst* inst)
{
    switch(inst->op)
    {
    case kIROp_IntLit:
        m_stream->emit(((IRConstant*) inst)->value.intVal);
        break;

    case kIROp_FloatLit:
        m_stream->emit(((IRConstant*) inst)->value.floatVal);
        break;

    case kIROp_BoolLit:
        {
            bool val = ((IRConstant*)inst)->value.intVal != 0;
            m_stream->emit(val ? "true" : "false");
        }
        break;

    default:
        SLANG_UNIMPLEMENTED_X("val case for emit");
        break;
    }

}

CodeGenTarget CLikeSourceEmitter::getTarget()
{
    return m_context->target;
}

bool CLikeSourceEmitter::shouldFoldIRInstIntoUseSites(IRInst* inst, IREmitMode mode)
{
    // Certain opcodes should never/always be folded in
    switch( inst->op )
    {
    default:
        break;

    // Never fold these in, because they represent declarations
    //
    case kIROp_Var:
    case kIROp_GlobalVar:
    case kIROp_GlobalConstant:
    case kIROp_GlobalParam:
    case kIROp_Param:
    case kIROp_Func:
        return false;

    // Always fold these in, because they are trivial
    //
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_BoolLit:
        return true;

    // Always fold these in, because their results
    // cannot be represented in the type system of
    // our current targets.
    //
    // TODO: when we add C/C++ as an optional target,
    // we could consider lowering insts that result
    // in pointers directly.
    //
    case kIROp_FieldAddress:
    case kIROp_getElementPtr:
    case kIROp_Specialize:
        return true;
    }

    // Always fold when we are inside a global constant initializer
    if (mode == IREmitMode::GlobalConstant)
        return true;

    switch( inst->op )
    {
    default:
        break;

    // HACK: don't fold these in because we currently lower
    // them to initializer lists, which aren't allowed in
    // general expression contexts.
    //
    // Note: we are doing this check *after* the check for `GlobalConstant`
    // mode, because otherwise we'd fail to emit initializer lists in
    // the main place where we want/need them.
    //
    case kIROp_makeStruct:
    case kIROp_makeArray:
        return false;

    }

    // Instructions with specific result *types* will usually
    // want to be folded in, because they aren't allowed as types
    // for temporary variables.
    auto type = inst->getDataType();

    // Unwrap any layers of array-ness from the type, so that
    // we can look at the underlying data type, in case we
    // should *never* expose a value of that type
    while (auto arrayType = as<IRArrayTypeBase>(type))
    {
        type = arrayType->getElementType();
    }

    // Don't allow temporaries of pointer types to be created.
    if(as<IRPtrTypeBase>(type))
    {
        return true;
    }

    // First we check for uniform parameter groups,
    // because a `cbuffer` or GLSL `uniform` block
    // does not have a first-class type that we can
    // pass around.
    //
    // TODO: We need to ensure that type legalization
    // cleans up cases where we use a parameter group
    // or parameter block type as a function parameter...
    //
    if(as<IRUniformParameterGroupType>(type))
    {
        // TODO: we need to be careful here, because
        // HLSL shader model 6 allows these as explicit
        // types.
        return true;
    }
    //
    // The stream-output and patch types need to be handled
    // too, because they are not really first class (especially
    // not in GLSL, but they also seem to confuse the HLSL
    // compiler when they get used as temporaries).
    //
    else if (as<IRHLSLStreamOutputType>(type))
    {
        return true;
    }
    else if (as<IRHLSLPatchType>(type))
    {
        return true;
    }


    // GLSL doesn't allow texture/resource types to
    // be used as first-class values, so we need
    // to fold them into their use sites in all cases
    if (getSourceStyle() == SourceStyle::GLSL)
    {
        if(as<IRResourceTypeBase>(type))
        {
            return true;
        }
        else if(as<IRHLSLStructuredBufferTypeBase>(type))
        {
            return true;
        }
        else if(as<IRUntypedBufferResourceType>(type))
        {
            return true;
        }
        else if(as<IRSamplerStateTypeBase>(type))
        {
            return true;
        }
    }

    // If the instruction is at global scope, then it might represent
    // a constant (e.g., the value of an enum case).
    //
    if(as<IRModuleInst>(inst->getParent()))
    {
        if(!inst->mightHaveSideEffects())
            return true;
    }

    // Having dealt with all of the cases where we *must* fold things
    // above, we can now deal with the more general cases where we
    // *should not* fold things.

    // Don't fold something with no users:
    if(!inst->hasUses())
        return false;

    // Don't fold something that has multiple users:
    if(inst->hasMoreThanOneUse())
        return false;

    // Don't fold something that might have side effects:
    if(inst->mightHaveSideEffects())
        return false;

    // Don't fold instructions that are marked `[precise]`.
    // This could in principle be extended to any other
    // decorations that affect the semantics of an instruction
    // in ways that require a temporary to be introduced.
    //
    if(inst->findDecoration<IRPreciseDecoration>())
        return false;

    // Okay, at this point we know our instruction must have a single use.
    auto use = inst->firstUse;
    SLANG_ASSERT(use);
    SLANG_ASSERT(!use->nextUse);

    auto user = use->getUser();

    // We'd like to figure out if it is safe to fold our instruction into `user`

    // First, let's make sure they are in the same block/parent:
    if(inst->getParent() != user->getParent())
        return false;

    // Now let's look at all the instructions between this instruction
    // and the user. If any of them might have side effects, then lets
    // bail out now.
    for(auto ii = inst->getNextInst(); ii != user; ii = ii->getNextInst())
    {
        if(!ii)
        {
            // We somehow reached the end of the block without finding
            // the user, which doesn't make sense if uses dominate
            // defs. Let's just play it safe and bail out.
            return false;
        }

        if(ii->mightHaveSideEffects())
            return false;
    }

    // Okay, if we reach this point then the user comes later in
    // the same block, and there are no instructions with side
    // effects in between, so it seems safe to fold things in.
    return true;
}

void CLikeSourceEmitter::emitIROperand(IRInst* inst, IREmitMode mode, EmitOpInfo const&  outerPrec)
{
    if( shouldFoldIRInstIntoUseSites(inst, mode) )
    {
        emitIRInstExpr(inst, mode, outerPrec);
        return;
    }

    switch(inst->op)
    {
    case 0: // nothing yet
    default:
        m_stream->emit(getIRName(inst));
        break;
    }
}

void CLikeSourceEmitter::emitIRArgs(IRInst* inst, IREmitMode mode)
{
    UInt argCount = inst->getOperandCount();
    IRUse* args = inst->getOperands();

    m_stream->emit("(");
    for(UInt aa = 0; aa < argCount; ++aa)
    {
        if(aa != 0) m_stream->emit(", ");
        emitIROperand(args[aa].get(), mode, getInfo(EmitOp::General));
    }
    m_stream->emit(")");
}

void CLikeSourceEmitter::emitIRType(IRType* type, String const& name)
{
    emitType(type, name);
}

void CLikeSourceEmitter::emitIRType(IRType* type, Name* name)
{
    emitType(type, name);
}

void CLikeSourceEmitter::emitIRType(IRType* type)
{
    emitType(type);
}

void CLikeSourceEmitter::emitIRRateQualifiers(IRRate* rate)
{
    if(!rate) return;

    if(as<IRConstExprRate>(rate))
    {
        switch( getSourceStyle() )
        {
        case SourceStyle::GLSL:
            m_stream->emit("const ");
            break;

        default:
            break;
        }
    }

    if (as<IRGroupSharedRate>(rate))
    {
        switch(getSourceStyle())
        {
        case SourceStyle::HLSL:
            m_stream->emit("groupshared ");
            break;

        case SourceStyle::GLSL:
            m_stream->emit("shared ");
            break;

        default:
            break;
        }
    }
}

void CLikeSourceEmitter::emitIRRateQualifiers(IRInst* value)
{
    emitIRRateQualifiers(value->getRate());
}

void CLikeSourceEmitter::emitIRInstResultDecl(IRInst* inst)
{
    auto type = inst->getDataType();
    if(!type)
        return;

    if (as<IRVoidType>(type))
        return;

    emitIRTempModifiers(inst);

    emitIRRateQualifiers(inst);

    emitIRType(type, getIRName(inst));
    m_stream->emit(" = ");
}

IRTargetIntrinsicDecoration* CLikeSourceEmitter::findTargetIntrinsicDecoration(IRInst* inst)
{
    for(auto dd : inst->getDecorations())
    {
        if (dd->op != kIROp_TargetIntrinsicDecoration)
            continue;

        auto targetIntrinsic = (IRTargetIntrinsicDecoration*)dd;
        if (isTargetIntrinsicModifierApplicable(targetIntrinsic))
            return targetIntrinsic;
    }

    return nullptr;
}

/* static */bool CLikeSourceEmitter::isOrdinaryName(String const& name)
{
    char const* cursor = name.begin();
    char const* end = name.end();

    while(cursor != end)
    {
        int c = *cursor++;
        if( (c >= 'a') && (c <= 'z') ) continue;
        if( (c >= 'A') && (c <= 'Z') ) continue;
        if( c == '_' ) continue;

        return false;
    }
    return true;
}

void CLikeSourceEmitter::emitTargetIntrinsicCallExpr(
    IRCall*                         inst,
    IRFunc*                         /* func */,
    IRTargetIntrinsicDecoration*    targetIntrinsic,
    IREmitMode                      mode,
    EmitOpInfo const&                  inOuterPrec)
{
    auto outerPrec = inOuterPrec;

    IRUse* args = inst->getOperands();
    Index argCount = inst->getOperandCount();

    // First operand was the function to be called
    args++;
    argCount--;

    auto name = String(targetIntrinsic->getDefinition());

    if(isOrdinaryName(name))
    {
        // Simple case: it is just an ordinary name, so we call it like a builtin.
        auto prec = getInfo(EmitOp::Postfix);
        bool needClose = maybeEmitParens(outerPrec, prec);

        m_stream->emit(name);
        m_stream->emit("(");
        for (Index aa = 0; aa < argCount; ++aa)
        {
            if (aa != 0) m_stream->emit(", ");
            emitIROperand(args[aa].get(), mode, getInfo(EmitOp::General));
        }
        m_stream->emit(")");

        maybeCloseParens(needClose);
        return;
    }
    else
    {
        int openParenCount = 0;

        const auto returnType = inst->getDataType();

        // If it returns void -> then we don't need parenthesis 
        if (as<IRVoidType>(returnType) == nullptr)
        {
            m_stream->emit("(");
            openParenCount++;
        }

        // General case: we are going to emit some more complex text.

        char const* cursor = name.begin();
        char const* end = name.end();
        while(cursor != end)
        {
            char c = *cursor++;
            if( c != '$' )
            {
                // Not an escape sequence
                m_stream->emitRawTextSpan(&c, &c+1);
                continue;
            }

            SLANG_RELEASE_ASSERT(cursor != end);

            char d = *cursor++;

            switch (d)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                {
                    // Simple case: emit one of the direct arguments to the call
                    Index argIndex = d - '0';
                    SLANG_RELEASE_ASSERT((0 <= argIndex) && (argIndex < argCount));
                    m_stream->emit("(");
                    emitIROperand(args[argIndex].get(), mode, getInfo(EmitOp::General));
                    m_stream->emit(")");
                }
                break;

            case 'p':
                {
                    // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                    // then this form will pair up the t and s arguments as needed for a GLSL
                    // texturing operation.
                    SLANG_RELEASE_ASSERT(argCount >= 2);

                    auto textureArg = args[0].get();
                    auto samplerArg = args[1].get();

                    if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                    {
                        emitGLSLTextureOrTextureSamplerType(baseTextureType, "sampler");

                        if (auto samplerType = as<IRSamplerStateTypeBase>(samplerArg->getDataType()))
                        {
                            if (as<IRSamplerComparisonStateType>(samplerType))
                            {
                                m_stream->emit("Shadow");
                            }
                        }

                        m_stream->emit("(");
                        emitIROperand(textureArg, mode, getInfo(EmitOp::General));
                        m_stream->emit(",");
                        emitIROperand(samplerArg, mode, getInfo(EmitOp::General));
                        m_stream->emit(")");
                    }
                    else
                    {
                        SLANG_UNEXPECTED("bad format in intrinsic definition");
                    }
                }
                break;

            case 'c':
                {
                    // When doing texture access in glsl the result may need to be cast.
                    // In particular if the underlying texture is 'half' based, glsl only accesses (read/write)
                    // as float. So we need to cast to a half type on output.
                    // When storing into a texture it is still the case the value written must be half - but
                    // we don't need to do any casting there as half is coerced to float without a problem.
                    SLANG_RELEASE_ASSERT(argCount >= 1);
                        
                    auto textureArg = args[0].get();
                    if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                    {
                        auto elementType = baseTextureType->getElementType();
                        IRBasicType* underlyingType = nullptr;
                        if (auto basicType = as<IRBasicType>(elementType))
                        {
                            underlyingType = basicType;
                        }
                        else if (auto vectorType = as<IRVectorType>(elementType))
                        {
                            underlyingType = as<IRBasicType>(vectorType->getElementType());
                        }

                        // We only need to output a cast if the underlying type is half.
                        if (underlyingType && underlyingType->op == kIROp_HalfType)
                        {
                            _emitSimpleType(elementType);
                            m_stream->emit("(");
                            openParenCount++;
                        }
                    }    
                }
                break;

            case 'z':
                {
                    // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                    // where `t` is a `Texture*<T>`, then this is the step where we try to
                    // properly swizzle the output of the equivalent GLSL call into the right
                    // shape.
                    SLANG_RELEASE_ASSERT(argCount >= 1);

                    auto textureArg = args[0].get();
                    if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                    {
                        auto elementType = baseTextureType->getElementType();
                        if (auto basicType = as<IRBasicType>(elementType))
                        {
                            // A scalar result is expected
                            m_stream->emit(".x");
                        }
                        else if (auto vectorType = as<IRVectorType>(elementType))
                        {
                            // A vector result is expected
                            auto elementCount = GetIntVal(vectorType->getElementCount());

                            if (elementCount < 4)
                            {
                                char const* swiz[] = { "", ".x", ".xy", ".xyz", "" };
                                m_stream->emit(swiz[elementCount]);
                            }
                        }
                        else
                        {
                            // What other cases are possible?
                        }
                    }
                    else
                    {
                        SLANG_UNEXPECTED("bad format in intrinsic definition");
                    }
                }
                break;

            case 'N':
                {
                    // Extract the element count from a vector argument so that
                    // we can use it in the constructed expression.

                    SLANG_RELEASE_ASSERT(*cursor >= '0' && *cursor <= '9');
                    Index argIndex = (*cursor++) - '0';
                    SLANG_RELEASE_ASSERT(argCount > argIndex);

                    auto vectorArg = args[argIndex].get();
                    if (auto vectorType = as<IRVectorType>(vectorArg->getDataType()))
                    {
                        auto elementCount = GetIntVal(vectorType->getElementCount());
                        m_stream->emit(elementCount);
                    }
                    else
                    {
                        SLANG_UNEXPECTED("bad format in intrinsic definition");
                    }
                }
                break;

            case 'V':
                {
                    // Take an argument of some scalar/vector type and pad
                    // it out to a 4-vector with the same element type
                    // (this is the inverse of `$z`).
                    //
                    SLANG_RELEASE_ASSERT(*cursor >= '0' && *cursor <= '9');
                    Index argIndex = (*cursor++) - '0';
                    SLANG_RELEASE_ASSERT(argCount > argIndex);

                    auto arg = args[argIndex].get();
                    IRIntegerValue elementCount = 1;
                    IRType* elementType = arg->getDataType();
                    if (auto vectorType = as<IRVectorType>(elementType))
                    {
                        elementCount = GetIntVal(vectorType->getElementCount());
                        elementType = vectorType->getElementType();
                    }

                    if(elementCount == 4)
                    {
                        // In the simple case, the operand is already a 4-vector,
                        // so we can just emit it as-is.
                        emitIROperand(arg, mode, getInfo(EmitOp::General));
                    }
                    else
                    {
                        // Otherwise, we need to construct a 4-vector from the
                        // value we have, padding it out with zero elements as
                        // needed.
                        //
                        emitVectorTypeName(elementType, 4);
                        m_stream->emit("(");
                        emitIROperand(arg, mode, getInfo(EmitOp::General));
                        for(IRIntegerValue ii = elementCount; ii < 4; ++ii)
                        {
                            m_stream->emit(", ");
                            if(getSourceStyle() == SourceStyle::GLSL)
                            {
                                _emitSimpleType(elementType);
                                m_stream->emit("(0)");
                            }
                            else
                            {
                                m_stream->emit("0");
                            }
                        }
                        m_stream->emit(")");
                    }
                }
                break;

            case 'a':
                {
                    // We have an operation that needs to lower to either
                    // `atomic*` or `imageAtomic*` for GLSL, depending on
                    // whether its first operand is a subscript into an
                    // array. This `$a` is the first `a` in `atomic`,
                    // so we will replace it accordingly.
                    //
                    // TODO: This distinction should be made earlier,
                    // with the front-end picking the right overload
                    // based on the "address space" of the argument.

                    Index argIndex = 0;
                    SLANG_RELEASE_ASSERT(argCount > argIndex);

                    auto arg = args[argIndex].get();
                    if(arg->op == kIROp_ImageSubscript)
                    {
                        m_stream->emit("imageA");
                    }
                    else
                    {
                        m_stream->emit("a");
                    }
                }
                break;

            case 'A':
                {
                    // We have an operand that represents the destination
                    // of an atomic operation in GLSL, and it should
                    // be lowered based on whether it is an ordinary l-value,
                    // or an image subscript. In the image subscript case
                    // this operand will turn into multiple arguments
                    // to the `imageAtomic*` function.
                    //

                    Index argIndex = 0;
                    SLANG_RELEASE_ASSERT(argCount > argIndex);

                    auto arg = args[argIndex].get();
                    if(arg->op == kIROp_ImageSubscript)
                    {
                        if(getSourceStyle() == SourceStyle::GLSL)
                        {
                            // TODO: we don't handle the multisample
                            // case correctly here, where the last
                            // component of the image coordinate needs
                            // to be broken out into its own argument.
                            //
                            m_stream->emit("(");
                            emitIROperand(arg->getOperand(0), mode, getInfo(EmitOp::General));
                            m_stream->emit("), ");

                            // The coordinate argument will have been computed
                            // as a `vector<uint, N>` because that is how the
                            // HLSL image subscript operations are defined.
                            // In contrast, the GLSL `imageAtomic*` operations
                            // expect `vector<int, N>` coordinates, so we
                            // hill hackily insert the conversion here as
                            // part of the intrinsic op.
                            //
                            auto coords = arg->getOperand(1);
                            auto coordsType = coords->getDataType();

                            auto coordsVecType = as<IRVectorType>(coordsType);
                            IRIntegerValue elementCount = 1;
                            if(coordsVecType)
                            {
                                coordsType = coordsVecType->getElementType();
                                elementCount = GetIntVal(coordsVecType->getElementCount());
                            }

                            SLANG_ASSERT(coordsType->op == kIROp_UIntType);

                            if (elementCount > 1)
                            {
                                m_stream->emit("ivec");
                                m_stream->emit(elementCount);
                            }
                            else
                            {
                                m_stream->emit("int");
                            }

                            m_stream->emit("(");
                            emitIROperand(arg->getOperand(1), mode, getInfo(EmitOp::General));
                            m_stream->emit(")");
                        }
                        else
                        {
                            m_stream->emit("(");
                            emitIROperand(arg, mode, getInfo(EmitOp::General));
                            m_stream->emit(")");
                        }
                    }
                    else
                    {
                        m_stream->emit("(");
                        emitIROperand(arg, mode, getInfo(EmitOp::General));
                        m_stream->emit(")");
                    }
                }
                break;

            // We will use the `$X` case as a prefix for
            // special logic needed when cross-compiling ray-tracing
            // shaders.
            case 'X':
                {
                    SLANG_RELEASE_ASSERT(*cursor);
                    switch(*cursor++)
                    {
                    case 'P':
                        {
                            // The `$XP` case handles looking up
                            // the associated `location` for a variable
                            // used as the argument ray payload at a
                            // trace call site.

                            Index argIndex = 0;
                            SLANG_RELEASE_ASSERT(argCount > argIndex);
                            auto arg = args[argIndex].get();
                            auto argLoad = as<IRLoad>(arg);
                            SLANG_RELEASE_ASSERT(argLoad);
                            auto argVar = argLoad->getOperand(0);
                            m_stream->emit(getRayPayloadLocation(argVar));
                        }
                        break;

                    case 'C':
                        {
                            // The `$XC` case handles looking up
                            // the associated `location` for a variable
                            // used as the argument callable payload at a
                            // call site.

                        Index argIndex = 0;
                            SLANG_RELEASE_ASSERT(argCount > argIndex);
                            auto arg = args[argIndex].get();
                            auto argLoad = as<IRLoad>(arg);
                            SLANG_RELEASE_ASSERT(argLoad);
                            auto argVar = argLoad->getOperand(0);
                            m_stream->emit(getCallablePayloadLocation(argVar));
                        }
                        break;

                    case 'T':
                        {
                            // The `$XT` case handles selecting between
                            // the `gl_HitTNV` and `gl_RayTmaxNV` builtins,
                            // based on what stage we are using:
                            switch( m_context->entryPoint->getStage() )
                            {
                            default:
                                m_stream->emit("gl_RayTmaxNV");
                                break;

                            case Stage::AnyHit:
                            case Stage::ClosestHit:
                                m_stream->emit("gl_HitTNV");
                                break;
                            }
                        }
                        break;

                    default:
                        SLANG_RELEASE_ASSERT(false);
                        break;
                    }
                }
                break;

            default:
                SLANG_UNEXPECTED("bad format in intrinsic definition");
                break;
            }
        }

        // Close any remaining open parens
        for (; openParenCount > 0; --openParenCount)
        {
            m_stream->emit(")");
        }
    }
}

void CLikeSourceEmitter::emitIntrinsicCallExpr(
    IRCall*         inst,
    IRFunc*         func,
    IREmitMode      mode,
    EmitOpInfo const&  inOuterPrec)
{
    auto outerPrec = inOuterPrec;
    bool needClose = false;

    // For a call with N arguments, the instruction will
    // have N+1 operands. We will start consuming operands
    // starting at the index 1.
    UInt operandCount = inst->getOperandCount();
    UInt argCount = operandCount - 1;
    UInt operandIndex = 1;


    //
    if (auto targetIntrinsicDecoration = findTargetIntrinsicDecoration(func))
    {
        emitTargetIntrinsicCallExpr(
            inst,
            func,
            targetIntrinsicDecoration,
            mode,
            outerPrec);
        return;
    }

    // Our current strategy for dealing with intrinsic
    // calls is to "un-mangle" the mangled name, in
    // order to figure out what the user was originally
    // calling. This is a bit messy, and there might
    // be better strategies (including just stuffing
    // a pointer to the original decl onto the callee).

    // If the intrinsic the user is calling is a generic,
    // then the mangled name will have been set on the
    // outer-most generic, and not on the leaf value
    // (which is `func` above), so we need to walk
    // upwards to find it.
    //
    IRInst* valueForName = func;
    for(;;)
    {
        auto parentBlock = as<IRBlock>(valueForName->parent);
        if(!parentBlock)
            break;

        auto parentGeneric = as<IRGeneric>(parentBlock->parent);
        if(!parentGeneric)
            break;

        valueForName = parentGeneric;
    }

    // If we reach this point, we are assuming that the value
    // has some kind of linkage, and thus a mangled name.
    //
    auto linkageDecoration = valueForName->findDecoration<IRLinkageDecoration>();
    SLANG_ASSERT(linkageDecoration);
    auto mangledName = String(linkageDecoration->getMangledName());


    // We will use the `MangledLexer` to
    // help us split the original name into its pieces.
    MangledLexer lexer(mangledName);
    
    // We'll read through the qualified name of the
    // symbol (e.g., `Texture2D<T>.Sample`) and then
    // only keep the last segment of the name (e.g.,
    // the `Sample` part).
    auto name = lexer.readSimpleName();

    // We will special-case some names here, that
    // represent callable declarations that aren't
    // ordinary functions, and thus may use different
    // syntax.
    if(name == "operator[]")
    {
        // The user is invoking a built-in subscript operator

        auto prec = getInfo(EmitOp::Postfix);
        needClose = maybeEmitParens(outerPrec, prec);

        emitIROperand(inst->getOperand(operandIndex++), mode, leftSide(outerPrec, prec));
        m_stream->emit("[");
        emitIROperand(inst->getOperand(operandIndex++), mode, getInfo(EmitOp::General));
        m_stream->emit("]");

        if(operandIndex < operandCount)
        {
            m_stream->emit(" = ");
            emitIROperand(inst->getOperand(operandIndex++), mode, getInfo(EmitOp::General));
        }

        maybeCloseParens(needClose);
        return;
    }

    auto prec = getInfo(EmitOp::Postfix);
    needClose = maybeEmitParens(outerPrec, prec);

    // The mangled function name currently records
    // the number of explicit parameters, and thus
    // doesn't include the implicit `this` parameter.
    // We can compare the argument and parameter counts
    // to figure out whether we have a member function call.
    UInt paramCount = lexer.readParamCount();

    if(argCount != paramCount)
    {
        // Looks like a member function call
        emitIROperand(inst->getOperand(operandIndex), mode, leftSide(outerPrec, prec));
        m_stream->emit(".");
        operandIndex++;
    }
    // fixing issue #602 for GLSL sign function: https://github.com/shader-slang/slang/issues/602
    bool glslSignFix = getSourceStyle() == SourceStyle::GLSL && name == "sign";
    if (glslSignFix)
    {
        if (auto vectorType = as<IRVectorType>(inst->getDataType()))
        {
            m_stream->emit("ivec");
            m_stream->emit(as<IRConstant>(vectorType->getElementCount())->value.intVal);
            m_stream->emit("(");
        }
        else if (auto scalarType = as<IRBasicType>(inst->getDataType()))
        {
            m_stream->emit("int(");
        }
        else
            glslSignFix = false;
    }
    m_stream->emit(name);
    m_stream->emit("(");
    bool first = true;
    for(; operandIndex < operandCount; ++operandIndex )
    {
        if(!first) m_stream->emit(", ");
        emitIROperand(inst->getOperand(operandIndex), mode, getInfo(EmitOp::General));
        first = false;
    }
    m_stream->emit(")");
    if (glslSignFix)
        m_stream->emit(")");
    maybeCloseParens(needClose);
}

void CLikeSourceEmitter::emitIRCallExpr(IRCall* inst, IREmitMode mode, EmitOpInfo outerPrec)
{
    auto funcValue = inst->getOperand(0);

    // Does this function declare any requirements on GLSL version or
    // extensions, which should affect our output?
    if(getSourceStyle() == SourceStyle::GLSL)
    {
        auto decoratedValue = funcValue;
        while (auto specInst = as<IRSpecialize>(decoratedValue))
        {
            decoratedValue = getSpecializedValue(specInst);
        }

        for( auto decoration : decoratedValue->getDecorations() )
        {
            switch(decoration->op)
            {
            default:
                break;

            case kIROp_RequireGLSLExtensionDecoration:
                requireGLSLExtension(String(((IRRequireGLSLExtensionDecoration*)decoration)->getExtensionName()));
                break;

            case kIROp_RequireGLSLVersionDecoration:
                requireGLSLVersion(int(((IRRequireGLSLVersionDecoration*)decoration)->getLanguageVersion()));
                break;
            }
        }
    }

    // We want to detect any call to an intrinsic operation,
    // that we can emit it directly without mangling, etc.
    if(auto irFunc = asTargetIntrinsic(funcValue))
    {
        emitIntrinsicCallExpr(inst, irFunc, mode, outerPrec);
    }
    else
    {
        auto prec = getInfo(EmitOp::Postfix);
        bool needClose = maybeEmitParens(outerPrec, prec);

        emitIROperand(funcValue, mode, leftSide(outerPrec, prec));
        m_stream->emit("(");
        UInt argCount = inst->getOperandCount();
        for( UInt aa = 1; aa < argCount; ++aa )
        {
            auto operand = inst->getOperand(aa);
            if (as<IRVoidType>(operand->getDataType()))
                continue;
            if(aa != 1) m_stream->emit(", ");
            emitIROperand(inst->getOperand(aa), mode, getInfo(EmitOp::General));
        }
        m_stream->emit(")");

        maybeCloseParens(needClose);
    }
}

static const char* _getGLSLVectorCompareFunctionName(IROp op)
{
    // Glsl vector comparisons use functions...
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/equal.xhtml

    switch (op)
    {
    case kIROp_Eql:     return "equal";
    case kIROp_Neq:     return "notEqual";
    case kIROp_Greater: return "greaterThan";
    case kIROp_Less:    return "lessThan";
    case kIROp_Geq:     return "greaterThanEqual";
    case kIROp_Leq:     return "lessThanEqual";
    default:    return nullptr;
    }
}

void CLikeSourceEmitter::_maybeEmitGLSLCast(IRType* castType, IRInst* inst, IREmitMode mode)
{
    // Wrap in cast if a cast type is specified
    if (castType)
    {
        emitIRType(castType);
        m_stream->emit("(");

        // Emit the operand
        emitIROperand(inst, mode, getInfo(EmitOp::General));

        m_stream->emit(")");
    }
    else
    {
        // Emit the operand
        emitIROperand(inst, mode, getInfo(EmitOp::General));
    }
}

void CLikeSourceEmitter::emitNot(IRInst* inst, IREmitMode mode, EmitOpInfo& ioOuterPrec, bool* outNeedClose)
{
    IRInst* operand = inst->getOperand(0);

    if (getSourceStyle() == SourceStyle::GLSL)
    {
        if (auto vectorType = as<IRVectorType>(operand->getDataType()))
        {
            // Handle as a function call
            auto prec = getInfo(EmitOp::Postfix);
            *outNeedClose = maybeEmitParens(ioOuterPrec, prec);

            m_stream->emit("not(");
            emitIROperand(operand, mode, getInfo(EmitOp::General));
            m_stream->emit(")");
            return;
        }
    }

    auto prec = getInfo(EmitOp::Prefix);
    *outNeedClose = maybeEmitParens(ioOuterPrec, prec);

    m_stream->emit("!");
    emitIROperand(operand, mode, rightSide(prec, ioOuterPrec));
}


void CLikeSourceEmitter::emitComparison(IRInst* inst, IREmitMode mode, EmitOpInfo& ioOuterPrec, const EmitOpInfo& opPrec, bool* needCloseOut)
{        
    if (getSourceStyle() == SourceStyle::GLSL)
    {
        IRInst* left = inst->getOperand(0);
        IRInst* right = inst->getOperand(1);

        auto leftVectorType = as<IRVectorType>(left->getDataType());
        auto rightVectorType = as<IRVectorType>(right->getDataType());

        // If either side is a vector handle as a vector
        if (leftVectorType || rightVectorType)
        {
            const char* funcName = _getGLSLVectorCompareFunctionName(inst->op);
            SLANG_ASSERT(funcName);

            // Determine the vector type
            const auto vecType = leftVectorType ? leftVectorType : rightVectorType;

            // Handle as a function call
            auto prec = getInfo(EmitOp::Postfix);
            *needCloseOut = maybeEmitParens(ioOuterPrec, prec);

            m_stream->emit(funcName);
            m_stream->emit("(");
            _maybeEmitGLSLCast((leftVectorType ? nullptr : vecType), left, mode);
            m_stream->emit(",");
            _maybeEmitGLSLCast((rightVectorType ? nullptr : vecType), right, mode);
            m_stream->emit(")");

            return;
        }
    }

    *needCloseOut = maybeEmitParens(ioOuterPrec, opPrec);

    emitIROperand(inst->getOperand(0), mode, leftSide(ioOuterPrec, opPrec));
    m_stream->emit(" ");
    m_stream->emit(opPrec.op);
    m_stream->emit(" ");
    emitIROperand(inst->getOperand(1), mode, rightSide(ioOuterPrec, opPrec));
}

    
void CLikeSourceEmitter::emitIRInstExpr(IRInst* inst, IREmitMode mode, const EmitOpInfo&  inOuterPrec)
{
    EmitOpInfo outerPrec = inOuterPrec;
    bool needClose = false;
    switch(inst->op)
    {
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_BoolLit:
        emitIRSimpleValue(inst);
        break;

    case kIROp_Construct:
    case kIROp_makeVector:
    case kIROp_MakeMatrix:
        // Simple constructor call

        switch (getSourceStyle())
        {
            case SourceStyle::HLSL:
            {
                if (inst->getOperandCount() == 1)
                {
                    auto prec = getInfo(EmitOp::Prefix);
                    needClose = maybeEmitParens(outerPrec, prec);

                    // Need to emit as cast for HLSL
                    m_stream->emit("(");
                    emitIRType(inst->getDataType());
                    m_stream->emit(") ");
                    emitIROperand(inst->getOperand(0), mode, rightSide(outerPrec, prec));
                    break;
                }
                /* fallthru*/
            }
            case SourceStyle::GLSL:
            {
                emitIRType(inst->getDataType());
                emitIRArgs(inst, mode);
                break;
            }
            case SourceStyle::CPP:
            case SourceStyle::C:
            {
                if (inst->getOperandCount() == 1)
                {
                    _emitCFunc(BuiltInCOp::Splat, inst->getDataType());
                    emitIRArgs(inst, mode);
                }
                else
                {
                    _emitCFunc(BuiltInCOp::Init, inst->getDataType());
                    emitIRArgs(inst, mode);
                }
                break;
            }
        }
        break;
    case kIROp_constructVectorFromScalar:

        // Simple constructor call
        if( getSourceStyle() == SourceStyle::HLSL )
        {
            auto prec = getInfo(EmitOp::Prefix);
            needClose = maybeEmitParens(outerPrec, prec);

            m_stream->emit("(");
            emitIRType(inst->getDataType());
            m_stream->emit(")");

            emitIROperand(inst->getOperand(0), mode, rightSide(outerPrec,prec));
        }
        else
        {
            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            emitIRType(inst->getDataType());
            m_stream->emit("(");
            emitIROperand(inst->getOperand(0), mode, getInfo(EmitOp::General));
            m_stream->emit(")");
        }
        break;

    case kIROp_FieldExtract:
        {
            // Extract field from aggregate

            IRFieldExtract* fieldExtract = (IRFieldExtract*) inst;

            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            auto base = fieldExtract->getBase();
            emitIROperand(base, mode, leftSide(outerPrec, prec));
            m_stream->emit(".");
            if(getSourceStyle() == SourceStyle::GLSL
                && as<IRUniformParameterGroupType>(base->getDataType()))
            {
                m_stream->emit("_data.");
            }
            m_stream->emit(getIRName(fieldExtract->getField()));
        }
        break;

    case kIROp_FieldAddress:
        {
            // Extract field "address" from aggregate

            IRFieldAddress* ii = (IRFieldAddress*) inst;

            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            auto base = ii->getBase();
            emitIROperand(base, mode, leftSide(outerPrec, prec));
            m_stream->emit(".");
            if(getSourceStyle() == SourceStyle::GLSL
                && as<IRUniformParameterGroupType>(base->getDataType()))
            {
                m_stream->emit("_data.");
            }
            m_stream->emit(getIRName(ii->getField()));
        }
        break;


#define CASE_COMPARE(OPCODE, PREC, OP)                                                          \
    case OPCODE:                                                                            \
        emitComparison(inst,  mode, outerPrec, getInfo(EmitOp::PREC), &needClose);               \
        break

#define CASE(OPCODE, PREC, OP)                                                                  \
    case OPCODE:                                                                            \
        needClose = maybeEmitParens(outerPrec, getInfo(EmitOp::PREC));                                \
        emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, getInfo(EmitOp::PREC)));    \
        m_stream->emit(" " #OP " ");                                                                  \
        emitIROperand(inst->getOperand(1), mode, rightSide(outerPrec, getInfo(EmitOp::PREC)));   \
        break

    CASE(kIROp_Add, Add, +);
    CASE(kIROp_Sub, Sub, -);
    CASE(kIROp_Div, Div, /);
    CASE(kIROp_Mod, Mod, %);

    CASE(kIROp_Lsh, Lsh, <<);
    CASE(kIROp_Rsh, Rsh, >>);

    // TODO: Need to pull out component-wise
    // comparison cases for matrices/vectors
    CASE_COMPARE(kIROp_Eql, Eql, ==);
    CASE_COMPARE(kIROp_Neq, Neq, !=);
    CASE_COMPARE(kIROp_Greater, Greater, >);
    CASE_COMPARE(kIROp_Less, Less, <);
    CASE_COMPARE(kIROp_Geq, Geq, >=);
    CASE_COMPARE(kIROp_Leq, Leq, <=);

    CASE(kIROp_BitXor, BitXor, ^);

    CASE(kIROp_And, And, &&);
    CASE(kIROp_Or,  Or,  ||);

#undef CASE

    // Component-wise multiplication needs to be special cased,
    // because GLSL uses infix `*` to express inner product
    // when working with matrices.
    case kIROp_Mul:
        // Are we targetting GLSL, and are both operands matrices?
        if(getSourceStyle() == SourceStyle::GLSL
            && as<IRMatrixType>(inst->getOperand(0)->getDataType())
            && as<IRMatrixType>(inst->getOperand(1)->getDataType()))
        {
            m_stream->emit("matrixCompMult(");
            emitIROperand(inst->getOperand(0), mode, getInfo(EmitOp::General));
            m_stream->emit(", ");
            emitIROperand(inst->getOperand(1), mode, getInfo(EmitOp::General));
            m_stream->emit(")");
        }
        else
        {
            // Default handling is to just rely on infix
            // `operator*`.
            auto prec = getInfo(EmitOp::Mul);
            needClose = maybeEmitParens(outerPrec, prec);
            emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, prec));
            m_stream->emit(" * ");
            emitIROperand(inst->getOperand(1), mode, rightSide(prec, outerPrec));
        }
        break;

    case kIROp_Not:
        {
            emitNot(inst,  mode, outerPrec, &needClose);
        }
        break;

    case kIROp_Neg:
        {
            auto prec = getInfo(EmitOp::Prefix);
            needClose = maybeEmitParens(outerPrec, prec);

            m_stream->emit("-");
            emitIROperand(inst->getOperand(0), mode, rightSide(prec, outerPrec));
        }
        break;

    case kIROp_BitNot:
        {
            auto prec = getInfo(EmitOp::Prefix);
            needClose = maybeEmitParens(outerPrec, prec);

            if (as<IRBoolType>(inst->getDataType()))
            {
                m_stream->emit("!");
            }
            else
            {
                m_stream->emit("~");
            }
            emitIROperand(inst->getOperand(0), mode, rightSide(prec, outerPrec));
        }
        break;

    case kIROp_BitAnd:
        {
            auto prec = getInfo(EmitOp::BitAnd);
            needClose = maybeEmitParens(outerPrec, prec);

            // TODO: handle a bitwise And of a vector of bools by casting to
            // a uvec and performing the bitwise operation

            emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, prec));

            // Are we targetting GLSL, and are both operands scalar bools?
            // In that case convert the operation to a logical And
            if (getSourceStyle() == SourceStyle::GLSL
                && as<IRBoolType>(inst->getOperand(0)->getDataType())
                && as<IRBoolType>(inst->getOperand(1)->getDataType()))
            {
                m_stream->emit("&&");
            }
            else
            {
                m_stream->emit("&");
            }

            emitIROperand(inst->getOperand(1), mode, rightSide(outerPrec, prec));
        }
        break;

    case kIROp_BitOr:
        {
            auto prec = getInfo(EmitOp::BitOr);
            needClose = maybeEmitParens(outerPrec, prec);

            // TODO: handle a bitwise Or of a vector of bools by casting to
            // a uvec and performing the bitwise operation

            emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, prec));

            // Are we targetting GLSL, and are both operands scalar bools?
            // In that case convert the operation to a logical Or
            if (getSourceStyle() == SourceStyle::GLSL
                && as<IRBoolType>(inst->getOperand(0)->getDataType())
                && as<IRBoolType>(inst->getOperand(1)->getDataType()))
            {
                m_stream->emit("||");
            }
            else
            {
                m_stream->emit("|");
            }

            emitIROperand(inst->getOperand(1), mode, rightSide(outerPrec, prec));
        }
        break;

    case kIROp_Load:
        {
            auto base = inst->getOperand(0);
            emitIROperand(base, mode, outerPrec);
            if(getSourceStyle() == SourceStyle::GLSL
                && as<IRUniformParameterGroupType>(base->getDataType()))
            {
                m_stream->emit("._data");
            }
        }
        break;

    case kIROp_Store:
        {
            auto prec = getInfo(EmitOp::Assign);
            needClose = maybeEmitParens(outerPrec, prec);

            emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, prec));
            m_stream->emit(" = ");
            emitIROperand(inst->getOperand(1), mode, rightSide(prec, outerPrec));
        }
        break;

    case kIROp_Call:
        {
            emitIRCallExpr((IRCall*)inst, mode, outerPrec);
        }
        break;

    case kIROp_GroupMemoryBarrierWithGroupSync:
        m_stream->emit("GroupMemoryBarrierWithGroupSync()");
        break;

    case kIROp_getElement:
    case kIROp_getElementPtr:
    case kIROp_ImageSubscript:
        // HACK: deal with translation of GLSL geometry shader input arrays.
        if(auto decoration = inst->getOperand(0)->findDecoration<IRGLSLOuterArrayDecoration>())
        {
            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            m_stream->emit(decoration->getOuterArrayName());
            m_stream->emit("[");
            emitIROperand(inst->getOperand(1), mode, getInfo(EmitOp::General));
            m_stream->emit("].");
            emitIROperand(inst->getOperand(0), mode, rightSide(prec, outerPrec));
            break;
        }
        else
        {
            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            emitIROperand( inst->getOperand(0), mode, leftSide(outerPrec, prec));
            m_stream->emit("[");
            emitIROperand(inst->getOperand(1), mode, getInfo(EmitOp::General));
            m_stream->emit("]");
        }
        break;

    case kIROp_Mul_Vector_Matrix:
    case kIROp_Mul_Matrix_Vector:
    case kIROp_Mul_Matrix_Matrix:
        if(getSourceStyle() == SourceStyle::GLSL)
        {
            // GLSL expresses inner-product multiplications
            // with the ordinary infix `*` operator.
            //
            // Note that the order of the operands is reversed
            // compared to HLSL (and Slang's internal representation)
            // because the notion of what is a "row" vs. a "column"
            // is reversed between HLSL/Slang and GLSL.
            //
            auto prec = getInfo(EmitOp::Mul);
            needClose = maybeEmitParens(outerPrec, prec);

            emitIROperand(inst->getOperand(1), mode, leftSide(outerPrec, prec));
            m_stream->emit(" * ");
            emitIROperand(inst->getOperand(0), mode, rightSide(prec, outerPrec));
        }
        else
        {
            m_stream->emit("mul(");
            emitIROperand(inst->getOperand(0), mode, getInfo(EmitOp::General));
            m_stream->emit(", ");
            emitIROperand(inst->getOperand(1), mode, getInfo(EmitOp::General));
            m_stream->emit(")");
        }
        break;

    case kIROp_swizzle:
        {
            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            auto ii = (IRSwizzle*)inst;
            emitIROperand(ii->getBase(), mode, leftSide(outerPrec, prec));
            m_stream->emit(".");
            const Index elementCount = Index(ii->getElementCount());
            for (Index ee = 0; ee < elementCount; ++ee)
            {
                IRInst* irElementIndex = ii->getElementIndex(ee);
                SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                IRConstant* irConst = (IRConstant*)irElementIndex;

                UInt elementIndex = (UInt)irConst->value.intVal;
                SLANG_RELEASE_ASSERT(elementIndex < 4);

                char const* kComponents[] = { "x", "y", "z", "w" };
                m_stream->emit(kComponents[elementIndex]);
            }
        }
        break;

    case kIROp_Specialize:
        {
            emitIROperand(inst->getOperand(0), mode, outerPrec);
        }
        break;

    case kIROp_Select:
        {
            if (getSourceStyle() == SourceStyle::GLSL &&
                inst->getOperand(0)->getDataType()->op != kIROp_BoolType)
            {
                // For GLSL, emit a call to `mix` if condition is a vector
                m_stream->emit("mix(");
                emitIROperand(inst->getOperand(2), mode, leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_stream->emit(", ");
                emitIROperand(inst->getOperand(1), mode, leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_stream->emit(", ");
                emitIROperand(inst->getOperand(0), mode, leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
                m_stream->emit(")");
            }
            else
            {
                auto prec = getInfo(EmitOp::Conditional);
                needClose = maybeEmitParens(outerPrec, prec);

                emitIROperand(inst->getOperand(0), mode, leftSide(outerPrec, prec));
                m_stream->emit(" ? ");
                emitIROperand(inst->getOperand(1), mode, prec);
                m_stream->emit(" : ");
                emitIROperand(inst->getOperand(2), mode, rightSide(prec, outerPrec));
            }
        }
        break;

    case kIROp_Param:
        m_stream->emit(getIRName(inst));
        break;

    case kIROp_makeArray:
    case kIROp_makeStruct:
        {
            // TODO: initializer-list syntax may not always
            // be appropriate, depending on the context
            // of the expression.

            m_stream->emit("{ ");
            UInt argCount = inst->getOperandCount();
            for (UInt aa = 0; aa < argCount; ++aa)
            {
                if (aa != 0) m_stream->emit(", ");
                emitIROperand(inst->getOperand(aa), mode, getInfo(EmitOp::General));
            }
            m_stream->emit(" }");
        }
        break;

    case kIROp_BitCast:
        {
            // TODO: we can simplify the logic for arbitrary bitcasts
            // by always bitcasting the source to a `uint*` type (if it
            // isn't already) and then bitcasting that to the destination
            // type (if it isn't already `uint*`.
            //
            // For now we are assuming the source type is *already*
            // a `uint*` type of the appropriate size.
            //
//                auto fromType = extractBaseType(inst->getOperand(0)->getDataType());
            auto toType = extractBaseType(inst->getDataType());
            switch(getSourceStyle())
            {
            case SourceStyle::GLSL:
                switch(toType)
                {
                default:
                    m_stream->emit("/* unhandled */");
                    break;

                case BaseType::UInt:
                    break;

                case BaseType::Int:
                    emitIRType(inst->getDataType());
                    break;

                case BaseType::Float:
                    m_stream->emit("uintBitsToFloat(");
                    break;
                }
                break;

            case SourceStyle::HLSL:
                switch(toType)
                {
                default:
                    m_stream->emit("/* unhandled */");
                    break;

                case BaseType::UInt:
                    break;
                case BaseType::Int:
                    m_stream->emit("(");
                    emitIRType(inst->getDataType());
                    m_stream->emit(")");
                    break;
                case BaseType::Float:
                    m_stream->emit("asfloat");
                    break;
                }
                break;


            default:
                SLANG_UNEXPECTED("unhandled codegen target");
                break;
            }

            m_stream->emit("(");
            emitIROperand(inst->getOperand(0), mode, getInfo(EmitOp::General));
            m_stream->emit(")");
        }
        break;

    default:
        m_stream->emit("/* unhandled */");
        break;
    }
    maybeCloseParens(needClose);
}

BaseType CLikeSourceEmitter::extractBaseType(IRType* inType)
{
    auto type = inType;
    for(;;)
    {
        if(auto irBaseType = as<IRBasicType>(type))
        {
            return irBaseType->getBaseType();
        }
        else if(auto vecType = as<IRVectorType>(type))
        {
            type = vecType->getElementType();
            continue;
        }
        else
        {
            return BaseType::Void;
        }
    }
}

void CLikeSourceEmitter::emitIRInst(IRInst* inst, IREmitMode mode)
{
    try
    {
        _emitIRInst(inst, mode);
    }
    // Don't emit any context message for an explicit `AbortCompilationException`
    // because it should only happen when an error is already emitted.
    catch(AbortCompilationException&) { throw; }
    catch(...)
    {
        m_context->noteInternalErrorLoc(inst->sourceLoc);
        throw;
    }
}

void CLikeSourceEmitter::_emitIRInst(IRInst* inst, IREmitMode mode)
{
    if (shouldFoldIRInstIntoUseSites(inst, mode))
    {
        return;
    }

    m_stream->advanceToSourceLocation(inst->sourceLoc);

    switch(inst->op)
    {
    default:
        emitIRInstResultDecl(inst);
        emitIRInstExpr(inst, mode, getInfo(EmitOp::General));
        m_stream->emit(";\n");
        break;

    case kIROp_undefined:
        {
            auto type = inst->getDataType();
            emitIRType(type, getIRName(inst));
            m_stream->emit(";\n");
        }
        break;

    case kIROp_Var:
        {
            auto ptrType = cast<IRPtrType>(inst->getDataType());
            auto valType = ptrType->getValueType();

            auto name = getIRName(inst);
            emitIRRateQualifiers(inst);
            emitIRType(valType, name);
            m_stream->emit(";\n");
        }
        break;

    case kIROp_Param:
        // Don't emit parameters, since they are declared as part of the function.
        break;

    case kIROp_FieldAddress:
        // skip during code emit, since it should be
        // folded into use site(s)
        break;

    case kIROp_ReturnVoid:
        m_stream->emit("return;\n");
        break;

    case kIROp_ReturnVal:
        m_stream->emit("return ");
        emitIROperand(((IRReturnVal*) inst)->getVal(), mode, getInfo(EmitOp::General));
        m_stream->emit(";\n");
        break;

    case kIROp_discard:
        m_stream->emit("discard;\n");
        break;

    case kIROp_swizzleSet:
        {
            auto ii = (IRSwizzleSet*)inst;
            emitIRInstResultDecl(inst);
            emitIROperand(inst->getOperand(0), mode, getInfo(EmitOp::General));
            m_stream->emit(";\n");

            auto subscriptOuter = getInfo(EmitOp::General);
            auto subscriptPrec = getInfo(EmitOp::Postfix);
            bool needCloseSubscript = maybeEmitParens(subscriptOuter, subscriptPrec);

            emitIROperand(inst, mode, leftSide(subscriptOuter, subscriptPrec));
            m_stream->emit(".");
            UInt elementCount = ii->getElementCount();
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                IRInst* irElementIndex = ii->getElementIndex(ee);
                SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                IRConstant* irConst = (IRConstant*)irElementIndex;

                UInt elementIndex = (UInt)irConst->value.intVal;
                SLANG_RELEASE_ASSERT(elementIndex < 4);

                char const* kComponents[] = { "x", "y", "z", "w" };
                m_stream->emit(kComponents[elementIndex]);
            }
            maybeCloseParens(needCloseSubscript);

            m_stream->emit(" = ");
            emitIROperand(inst->getOperand(1), mode, getInfo(EmitOp::General));
            m_stream->emit(";\n");
        }
        break;

    case kIROp_SwizzledStore:
        {
            auto subscriptOuter = getInfo(EmitOp::General);
            auto subscriptPrec = getInfo(EmitOp::Postfix);
            bool needCloseSubscript = maybeEmitParens(subscriptOuter, subscriptPrec);


            auto ii = cast<IRSwizzledStore>(inst);
            emitIROperand(ii->getDest(), mode, leftSide(subscriptOuter, subscriptPrec));
            m_stream->emit(".");
            UInt elementCount = ii->getElementCount();
            for (UInt ee = 0; ee < elementCount; ++ee)
            {
                IRInst* irElementIndex = ii->getElementIndex(ee);
                SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                IRConstant* irConst = (IRConstant*)irElementIndex;

                UInt elementIndex = (UInt)irConst->value.intVal;
                SLANG_RELEASE_ASSERT(elementIndex < 4);

                char const* kComponents[] = { "x", "y", "z", "w" };
                m_stream->emit(kComponents[elementIndex]);
            }
            maybeCloseParens(needCloseSubscript);

            m_stream->emit(" = ");
            emitIROperand(ii->getSource(), mode, getInfo(EmitOp::General));
            m_stream->emit(";\n");
        }
        break;
    }
}

void CLikeSourceEmitter::emitIRSemantics(VarLayout* varLayout)
{
    if(varLayout->flags & VarLayoutFlag::HasSemantic)
    {
        m_stream->emit(" : ");
        m_stream->emit(varLayout->semanticName);
        if(varLayout->semanticIndex)
        {
            m_stream->emit(varLayout->semanticIndex);
        }
    }
}

void CLikeSourceEmitter::emitIRSemantics(IRInst* inst)
{
    // Don't emit semantics if we aren't translating down to HLSL
    switch (getSourceStyle())
    {
    case SourceStyle::HLSL:
        break;

    default:
        return;
    }

    if (auto semanticDecoration = inst->findDecoration<IRSemanticDecoration>())
    {
        m_stream->emit(" : ");
        m_stream->emit(semanticDecoration->getSemanticName());
        return;
    }

    if(auto layoutDecoration = inst->findDecoration<IRLayoutDecoration>())
    {
        auto layout = layoutDecoration->getLayout();
        if(auto varLayout = as<VarLayout>(layout))
        {
            emitIRSemantics(varLayout);
        }
        else if (auto entryPointLayout = as<EntryPointLayout>(layout))
        {
            if(auto resultLayout = entryPointLayout->resultLayout)
            {
                emitIRSemantics(resultLayout);
            }
        }
    }
}

VarLayout* CLikeSourceEmitter::getVarLayout(IRInst* var)
{
    auto decoration = var->findDecoration<IRLayoutDecoration>();
    if (!decoration)
        return nullptr;

    return (VarLayout*) decoration->getLayout();
}

void CLikeSourceEmitter::emitIRLayoutSemantics(IRInst* inst, char const* uniformSemanticSpelling)
{
    auto layout = getVarLayout(inst);
    if (layout)
    {
        emitHLSLRegisterSemantics(layout, uniformSemanticSpelling);
    }
}

void CLikeSourceEmitter::emitPhiVarAssignments(UInt argCount, IRUse* args, IRBlock* targetBlock)
{
    UInt argCounter = 0;
    for (auto pp = targetBlock->getFirstParam(); pp; pp = pp->getNextParam())
    {
        UInt argIndex = argCounter++;

        if (argIndex >= argCount)
        {
            SLANG_UNEXPECTED("not enough arguments for branch");
            break;
        }

        IRInst* arg = args[argIndex].get();

        auto outerPrec = getInfo(EmitOp::General);
        auto prec = getInfo(EmitOp::Assign);

        emitIROperand(pp, IREmitMode::Default, leftSide(outerPrec, prec));
        m_stream->emit(" = ");
        emitIROperand(arg, IREmitMode::Default, rightSide(prec, outerPrec));
        m_stream->emit(";\n");
    }
}

void CLikeSourceEmitter::emitRegion(Region* inRegion)
{
    // We will use a loop so that we can process sequential (simple)
    // regions iteratively rather than recursively.
    // This is effectively an emulation of tail recursion.
    Region* region = inRegion;
    while(region)
    {
        // What flavor of region are we trying to emit?
        switch(region->getFlavor())
        {
        case Region::Flavor::Simple:
            {
                // A simple region consists of a basic block followed
                // by another region.
                //
                auto simpleRegion = (SimpleRegion*) region;

                // We start by outputting all of the non-terminator
                // instructions in the block.
                //
                auto block = simpleRegion->block;
                auto terminator = block->getTerminator();
                for (auto inst = block->getFirstInst(); inst != terminator; inst = inst->getNextInst())
                {
                    emitIRInst(inst, IREmitMode::Default);
                }

                // Next we have to deal with the terminator instruction
                // itself. In many cases, the terminator will have been
                // turned into a block of its own, but certain cases
                // of terminators are simple enough that we just fold
                // them into the current block.
                //
                m_stream->advanceToSourceLocation(terminator->sourceLoc);
                switch(terminator->op)
                {
                default:
                    // Don't do anything with the terminator, and assume
                    // its behavior has been folded into the next region.
                    break;

                case kIROp_ReturnVal:
                case kIROp_ReturnVoid:
                case kIROp_discard:
                    // For extremely simple terminators, we just handle
                    // them here, so that we don't have to allocate
                    // separate `Region`s for them.
                    emitIRInst(terminator, IREmitMode::Default);
                    break;

                // We will also handle any unconditional branches
                // here, since they may have arguments to pass
                // to the target block (our encoding of SSA
                // "phi" operations).
                //
                // TODO: A better approach would be to move out of SSA
                // as an IR pass, and introduce explicit variables to
                // replace any "phi nodes." This would avoid possible
                // complications if we ever end up in the bad case where
                // one of the block arguments on a branch is also
                // a parameter of the target block, so that the order
                // of operations is important.
                //
                case kIROp_unconditionalBranch:
                    {
                        auto t = (IRUnconditionalBranch*)terminator;
                        UInt argCount = t->getOperandCount();
                        static const UInt kFixedArgCount = 1;
                        emitPhiVarAssignments(
                            argCount - kFixedArgCount,
                            t->getOperands() + kFixedArgCount,
                            t->getTargetBlock());
                    }
                    break;
                case kIROp_loop:
                    {
                        auto t = (IRLoop*) terminator;
                        UInt argCount = t->getOperandCount();
                        static const UInt kFixedArgCount = 3;
                        emitPhiVarAssignments(
                            argCount - kFixedArgCount,
                            t->getOperands() + kFixedArgCount,
                            t->getTargetBlock());

                    }
                    break;
                }

                // If the terminator required a full region to represent
                // its behavior in a structured form, then we will move
                // along to that region now.
                //
                // We do this iteratively rather than recursively, by
                // jumping back to the top of our loop with a new
                // value for `region`.
                //
                region = simpleRegion->nextRegion;
                continue;
            }

        // Break and continue regions are trivial to handle, as long as we
        // don't need to consider multi-level break/continue (which we
        // don't for now).
        case Region::Flavor::Break:
            m_stream->emit("break;\n");
            break;
        case Region::Flavor::Continue:
            m_stream->emit("continue;\n");
            break;

        case Region::Flavor::If:
            {
                auto ifRegion = (IfRegion*) region;

                // TODO: consider simplifying the code in
                // the case where `ifRegion == null`
                // so that we output `if(!condition) { elseRegion }`
                // instead of the current `if(condition) {} else { elseRegion }`

                m_stream->emit("if(");
                emitIROperand(ifRegion->condition, IREmitMode::Default, getInfo(EmitOp::General));
                m_stream->emit(")\n{\n");
                m_stream->indent();
                emitRegion(ifRegion->thenRegion);
                m_stream->dedent();
                m_stream->emit("}\n");

                // Don't emit the `else` region if it would be empty
                //
                if(auto elseRegion = ifRegion->elseRegion)
                {
                    m_stream->emit("else\n{\n");
                    m_stream->indent();
                    emitRegion(elseRegion);
                    m_stream->dedent();
                    m_stream->emit("}\n");
                }

                // Continue with the region after the `if`.
                //
                // TODO: consider just constructing a `SimpleRegion`
                // around an `IfRegion` to handle this sequencing,
                // rather than making `IfRegion` serve as both a
                // conditional and a sequence.
                //
                region = ifRegion->nextRegion;
                continue;
            }
            break;

        case Region::Flavor::Loop:
            {
                auto loopRegion = (LoopRegion*) region;
                auto loopInst = loopRegion->loopInst;

                // If the user applied an explicit decoration to the loop,
                // to control its unrolling behavior, then pass that
                // along in the output code (if the target language
                // supports the semantics of the decoration).
                //
                if (auto loopControlDecoration = loopInst->findDecoration<IRLoopControlDecoration>())
                {
                    switch (loopControlDecoration->getMode())
                    {
                    case kIRLoopControl_Unroll:
                        // Note: loop unrolling control is only available in HLSL, not GLSL
                        if(getSourceStyle() == SourceStyle::HLSL)
                        {
                            m_stream->emit("[unroll]\n");
                        }
                        break;

                    default:
                        break;
                    }
                }

                m_stream->emit("for(;;)\n{\n");
                m_stream->indent();
                emitRegion(loopRegion->body);
                m_stream->dedent();
                m_stream->emit("}\n");

                // Continue with the region after the loop
                region = loopRegion->nextRegion;
                continue;
            }

        case Region::Flavor::Switch:
            {
                auto switchRegion = (SwitchRegion*) region;

                // Emit the start of our statement.
                m_stream->emit("switch(");
                emitIROperand(switchRegion->condition, IREmitMode::Default, getInfo(EmitOp::General));
                m_stream->emit(")\n{\n");

                auto defaultCase = switchRegion->defaultCase;
                for(auto currentCase : switchRegion->cases)
                {
                    for(auto caseVal : currentCase->values)
                    {
                        m_stream->emit("case ");
                        emitIROperand(caseVal, IREmitMode::Default, getInfo(EmitOp::General));
                        m_stream->emit(":\n");
                    }
                    if(currentCase.Ptr() == defaultCase)
                    {
                        m_stream->emit("default:\n");
                    }

                    m_stream->indent();
                    m_stream->emit("{\n");
                    m_stream->indent();
                    emitRegion(currentCase->body);
                    m_stream->dedent();
                    m_stream->emit("}\n");
                    m_stream->dedent();
                }

                m_stream->emit("}\n");

                // Continue with the region after the `switch`
                region = switchRegion->nextRegion;
                continue;
            }
            break;
        }
        break;
    }
}

/// Emit high-level language statements from a structured region tree.
void CLikeSourceEmitter::emitRegionTree(RegionTree* regionTree)
{
    emitRegion(regionTree->rootRegion);
}

bool CLikeSourceEmitter::isDefinition(IRFunc* func)
{
    // For now, we use a simple approach: a function is
    // a definition if it has any blocks, and a declaration otherwise.
    return func->getFirstBlock() != nullptr;
}

String CLikeSourceEmitter::getIRFuncName(IRFunc* func)
{
    if (auto entryPointLayout = asEntryPoint(func))
    {
        // GLSL will always need to use `main` as the
        // name for an entry-point function, but other
        // targets should try to use the original name.
        //
        // TODO: always use `main`, and have any code
        // that wraps this know to use `main` instead
        // of the original entry-point name...
        //
        if (getSourceStyle() != SourceStyle::GLSL)
        {
            return getText(entryPointLayout->entryPoint->getName());
        }

        //

        return "main";
    }
    else
    {
        return getIRName(func);
    }
}

void CLikeSourceEmitter::emitAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib)
{
    assert(attrib);

    attrib->args.getCount();
    if (attrib->args.getCount() != 1)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Attribute expects single parameter");
        return;
    }

    Expr* expr = attrib->args[0];

    auto stringLitExpr = as<StringLiteralExpr>(expr);
    if (!stringLitExpr)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Attribute parameter expecting to be a string ");
        return;
    }

    m_stream->emit("[");
    m_stream->emit(name);
    m_stream->emit("(\"");
    m_stream->emit(stringLitExpr->value);
    m_stream->emit("\")]\n");
}

void CLikeSourceEmitter::emitAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib)
{
    assert(attrib);

    attrib->args.getCount();
    if (attrib->args.getCount() != 1)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Attribute expects single parameter");
        return;
    }

    Expr* expr = attrib->args[0];

    auto intLitExpr = as<IntegerLiteralExpr>(expr);
    if (!intLitExpr)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Attribute expects an int");
        return;
    }

    m_stream->emit("[");
    m_stream->emit(name);
    m_stream->emit("(");
    m_stream->emit(intLitExpr->value);
    m_stream->emit(")]\n");
}

void CLikeSourceEmitter::emitFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib)
{
    SLANG_UNUSED(attrib);

    auto irPatchFunc = irFunc->findDecoration<IRPatchConstantFuncDecoration>();
    assert(irPatchFunc);
    if (!irPatchFunc)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Unable to find [patchConstantFunc(...)] decoration");
        return;
    }

    const String irName = getIRName(irPatchFunc->getFunc());

    m_stream->emit("[patchconstantfunc(\"");
    m_stream->emit(irName);
    m_stream->emit("\")]\n");
}

void CLikeSourceEmitter::emitIREntryPointAttributes_HLSL(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    auto profile = m_context->effectiveProfile;
    auto stage = entryPointLayout->profile.GetStage();

    if(profile.getFamily() == ProfileFamily::DX)
    {
        if(profile.GetVersion() >= ProfileVersion::DX_6_1 )
        {
            char const* stageName = getStageName(stage);
            if(stageName)
            {
                m_stream->emit("[shader(\"");
                m_stream->emit(stageName);
                m_stream->emit("\")]");
            }
        }
    }

    switch (stage)
    {
    case Stage::Compute:
        {
            static const UInt kAxisCount = 3;
            UInt sizeAlongAxis[kAxisCount];

            // TODO: this is kind of gross because we are using a public
            // reflection API function, rather than some kind of internal
            // utility it forwards to...
            spReflectionEntryPoint_getComputeThreadGroupSize(
                (SlangReflectionEntryPoint*)entryPointLayout,
                kAxisCount,
                &sizeAlongAxis[0]);

            m_stream->emit("[numthreads(");
            for (int ii = 0; ii < 3; ++ii)
            {
                if (ii != 0) m_stream->emit(", ");
                m_stream->emit(sizeAlongAxis[ii]);
            }
            m_stream->emit(")]\n");
        }
        break;
    case Stage::Geometry:
    {
        if (auto attrib = entryPointLayout->entryPoint->FindModifier<MaxVertexCountAttribute>())
        {
            m_stream->emit("[maxvertexcount(");
            m_stream->emit(attrib->value);
            m_stream->emit(")]\n");
        }
        if (auto attrib = entryPointLayout->entryPoint->FindModifier<InstanceAttribute>())
        {
            m_stream->emit("[instance(");
            m_stream->emit(attrib->value);
            m_stream->emit(")]\n");
        }
        break;
    }
    case Stage::Domain:
    {
        FuncDecl* entryPoint = entryPointLayout->entryPoint;
        /* [domain("isoline")] */
        if (auto attrib = entryPoint->FindModifier<DomainAttribute>())
        {
            emitAttributeSingleString("domain", entryPoint, attrib);
        }

        break;
    }
    case Stage::Hull:
    {
        // Lists these are only attributes for hull shader
        // https://docs.microsoft.com/en-us/windows/desktop/direct3d11/direct3d-11-advanced-stages-hull-shader-design

        FuncDecl* entryPoint = entryPointLayout->entryPoint;

        /* [domain("isoline")] */
        if (auto attrib = entryPoint->FindModifier<DomainAttribute>())
        {
            emitAttributeSingleString("domain", entryPoint, attrib);
        }
        /* [domain("partitioning")] */
        if (auto attrib = entryPoint->FindModifier<PartitioningAttribute>())
        {
            emitAttributeSingleString("partitioning", entryPoint, attrib);
        }
        /* [outputtopology("line")] */
        if (auto attrib = entryPoint->FindModifier<OutputTopologyAttribute>())
        {
            emitAttributeSingleString("outputtopology", entryPoint, attrib);
        }
        /* [outputcontrolpoints(4)] */
        if (auto attrib = entryPoint->FindModifier<OutputControlPointsAttribute>())
        {
            emitAttributeSingleInt("outputcontrolpoints", entryPoint, attrib);
        }
        /* [patchconstantfunc("HSConst")] */
        if (auto attrib = entryPoint->FindModifier<PatchConstantFuncAttribute>())
        {
            emitFuncDeclPatchConstantFuncAttribute(irFunc, entryPoint, attrib);
        }

        break;
    }
    case Stage::Pixel:
    {
        if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
        {
            m_stream->emit("[earlydepthstencil]\n");
        }
        break;
    }
    // TODO: There are other stages that will need this kind of handling.
    default:
        break;
    }
}

void CLikeSourceEmitter::emitIREntryPointAttributes_GLSL(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    auto profile = entryPointLayout->profile;
    auto stage = profile.GetStage();

    switch (stage)
    {
    case Stage::Compute:
        {
            static const UInt kAxisCount = 3;
            UInt sizeAlongAxis[kAxisCount];

            // TODO: this is kind of gross because we are using a public
            // reflection API function, rather than some kind of internal
            // utility it forwards to...
            spReflectionEntryPoint_getComputeThreadGroupSize(
                (SlangReflectionEntryPoint*)entryPointLayout,
                kAxisCount,
                &sizeAlongAxis[0]);

            m_stream->emit("layout(");
            char const* axes[] = { "x", "y", "z" };
            for (int ii = 0; ii < 3; ++ii)
            {
                if (ii != 0) m_stream->emit(", ");
                m_stream->emit("local_size_");
                m_stream->emit(axes[ii]);
                m_stream->emit(" = ");
                m_stream->emit(sizeAlongAxis[ii]);
            }
            m_stream->emit(") in;");
        }
        break;
    case Stage::Geometry:
    {
        if (auto attrib = entryPointLayout->entryPoint->FindModifier<MaxVertexCountAttribute>())
        {
            m_stream->emit("layout(max_vertices = ");
            m_stream->emit(attrib->value);
            m_stream->emit(") out;\n");
        }
        if (auto attrib = entryPointLayout->entryPoint->FindModifier<InstanceAttribute>())
        {
            m_stream->emit("layout(invocations = ");
            m_stream->emit(attrib->value);
            m_stream->emit(") in;\n");
        }

        for(auto pp : entryPointLayout->entryPoint->GetParameters())
        {
            if(auto inputPrimitiveTypeModifier = pp->FindModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
            {
                if(as<HLSLTriangleModifier>(inputPrimitiveTypeModifier))
                {
                    m_stream->emit("layout(triangles) in;\n");
                }
                else if(as<HLSLLineModifier>(inputPrimitiveTypeModifier))
                {
                    m_stream->emit("layout(lines) in;\n");
                }
                else if(as<HLSLLineAdjModifier>(inputPrimitiveTypeModifier))
                {
                    m_stream->emit("layout(lines_adjacency) in;\n");
                }
                else if(as<HLSLPointModifier>(inputPrimitiveTypeModifier))
                {
                    m_stream->emit("layout(points) in;\n");
                }
                else if(as<HLSLTriangleAdjModifier>(inputPrimitiveTypeModifier))
                {
                    m_stream->emit("layout(triangles_adjacency) in;\n");
                }
            }

            if(auto outputStreamType = as<HLSLStreamOutputType>(pp->type))
            {
                if(as<HLSLTriangleStreamType>(outputStreamType))
                {
                    m_stream->emit("layout(triangle_strip) out;\n");
                }
                else if(as<HLSLLineStreamType>(outputStreamType))
                {
                    m_stream->emit("layout(line_strip) out;\n");
                }
                else if(as<HLSLPointStreamType>(outputStreamType))
                {
                    m_stream->emit("layout(points) out;\n");
                }
            }
        }


    }
    break;
    case Stage::Pixel:
    {
        if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
        {
            // https://www.khronos.org/opengl/wiki/Early_Fragment_Test
            m_stream->emit("layout(early_fragment_tests) in;\n");
        }
        break;
    }
    // TODO: There are other stages that will need this kind of handling.
    default:
        break;
    }
}

void CLikeSourceEmitter::emitIREntryPointAttributes(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    switch(getSourceStyle())
    {
    case SourceStyle::HLSL:
        emitIREntryPointAttributes_HLSL(irFunc, entryPointLayout);
        break;

    case SourceStyle::GLSL:
        emitIREntryPointAttributes_GLSL(irFunc, entryPointLayout);
        break;
    }
}

void CLikeSourceEmitter::emitPhiVarDecls(IRFunc* func)
{
    // We will skip the first block, since its parameters are
    // the parameters of the whole function.
    auto bb = func->getFirstBlock();
    if (!bb)
        return;
    bb = bb->getNextBlock();

    for (; bb; bb = bb->getNextBlock())
    {
        for (auto pp = bb->getFirstParam(); pp; pp = pp->getNextParam())
        {
            emitIRTempModifiers(pp);
            emitIRType(pp->getFullType(), getIRName(pp));
            m_stream->emit(";\n");
        }
    }
}

/// Emit high-level statements for the body of a function.
void CLikeSourceEmitter::emitIRFunctionBody(IRGlobalValueWithCode* code)
{
    // Compute a structured region tree that can represent
    // the control flow of our function.
    //
    RefPtr<RegionTree> regionTree = generateRegionTreeForFunc(
        code,
        m_context->getSink());

    // Now that we've computed the region tree, we have
    // an opportunity to perform some last-minute transformations
    // on the code to make sure it follows our rules.
    //
    // TODO: it would be better to do these transformations earlier,
    // so that we can, e.g., dump the final IR code *before* emission
    // starts, but that gets a bit complicated because we also want
    // to have the region tree available without having to recompute it.
    //
    // For now we are just going to do things the expedient way, but
    // eventually we should allow an IR module to have side-band
    // storage for derived structures like the region tree (and logic
    // for invalidating them when a transformation would break them).
    //
    fixValueScoping(regionTree);

    // Now emit high-level code from that structured region tree.
    //
    emitRegionTree(regionTree);
}

void CLikeSourceEmitter::emitIRSimpleFunc(IRFunc* func)
{
    auto resultType = func->getResultType();

    // Deal with decorations that need
    // to be emitted as attributes
    auto entryPointLayout = asEntryPoint(func);
    if (entryPointLayout)
    {
        emitIREntryPointAttributes(func, entryPointLayout);
    }

    auto name = getIRFuncName(func);

    emitType(resultType, name);

    m_stream->emit("(");
    auto firstParam = func->getFirstParam();
    for( auto pp = firstParam; pp; pp = pp->getNextParam())
    {
        if(pp != firstParam)
            m_stream->emit(", ");

        auto paramName = getIRName(pp);
        auto paramType = pp->getDataType();

        if (getSourceStyle() == SourceStyle::HLSL)
        {
            if (auto layoutDecor = pp->findDecoration<IRLayoutDecoration>())
            {
                Layout* layout = layoutDecor->getLayout();
                VarLayout* varLayout = as<VarLayout>(layout);

                if (varLayout)
                {
                    auto var = varLayout->getVariable();

                    if (auto primTypeModifier = var->FindModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
                    {
                        if (as<HLSLTriangleModifier>(primTypeModifier))
                            m_stream->emit("triangle ");
                        else if (as<HLSLPointModifier>(primTypeModifier))
                            m_stream->emit("point ");
                        else if (as<HLSLLineModifier>(primTypeModifier))
                            m_stream->emit("line ");
                        else if (as<HLSLLineAdjModifier>(primTypeModifier))
                            m_stream->emit("lineadj ");
                        else if (as<HLSLTriangleAdjModifier>(primTypeModifier))
                            m_stream->emit("triangleadj ");
                    }
                }
            }
        }

        emitIRParamType(paramType, paramName);

        emitIRSemantics(pp);
    }
    m_stream->emit(")");

    emitIRSemantics(func);

    // TODO: encode declaration vs. definition
    if(isDefinition(func))
    {
        m_stream->emit("\n{\n");
        m_stream->indent();

        // HACK: forward-declare all the local variables needed for the
        // parameters of non-entry blocks.
        emitPhiVarDecls(func);

        // Need to emit the operations in the blocks of the function
        emitIRFunctionBody(func);

        m_stream->dedent();
        m_stream->emit("}\n\n");
    }
    else
    {
        m_stream->emit(";\n\n");
    }
}

void CLikeSourceEmitter::emitIRParamType(IRType* type, String const& name)
{
    // An `out` or `inout` parameter will have been
    // encoded as a parameter of pointer type, so
    // we need to decode that here.
    //
    if( auto outType = as<IROutType>(type))
    {
        m_stream->emit("out ");
        type = outType->getValueType();
    }
    else if( auto inOutType = as<IRInOutType>(type))
    {
        m_stream->emit("inout ");
        type = inOutType->getValueType();
    }
    else if( auto refType = as<IRRefType>(type))
    {
        // Note: There is no HLSL/GLSL equivalent for by-reference parameters,
        // so we don't actually expect to encounter these in user code.
        m_stream->emit("inout ");
        type = inOutType->getValueType();
    }

    emitIRType(type, name);
}

IRInst* CLikeSourceEmitter::getSpecializedValue(IRSpecialize* specInst)
{
    auto base = specInst->getBase();
    auto baseGeneric = as<IRGeneric>(base);
    if (!baseGeneric)
        return base;

    auto lastBlock = baseGeneric->getLastBlock();
    if (!lastBlock)
        return base;

    auto returnInst = as<IRReturnVal>(lastBlock->getTerminator());
    if (!returnInst)
        return base;

    return returnInst->getVal();
}

void CLikeSourceEmitter::emitIRFuncDecl(IRFunc* func)
{
    // We don't want to emit declarations for operations
    // that only appear in the IR as stand-ins for built-in
    // operations on that target.
    if (isTargetIntrinsic(func))
        return;

    // Finally, don't emit a declaration for an entry point,
    // because it might need meta-data attributes attached
    // to it, and the HLSL compiler will get upset if the
    // forward declaration doesn't *also* have those
    // attributes.
    if(asEntryPoint(func))
        return;


    // A function declaration doesn't have any IR basic blocks,
    // and as a result it *also* doesn't have the IR `param` instructions,
    // so we need to emit a declaration entirely from the type.

    auto funcType = func->getDataType();
    auto resultType = func->getResultType();

    auto name = getIRFuncName(func);

    emitIRType(resultType, name);

    m_stream->emit("(");
    auto paramCount = funcType->getParamCount();
    for(UInt pp = 0; pp < paramCount; ++pp)
    {
        if(pp != 0)
            m_stream->emit(", ");

        String paramName;
        paramName.append("_");
        paramName.append(Int32(pp));
        auto paramType = funcType->getParamType(pp);

        emitIRParamType(paramType, paramName);
    }
    m_stream->emit(");\n\n");
}

EntryPointLayout* CLikeSourceEmitter::getEntryPointLayout(IRFunc* func)
{
    if( auto layoutDecoration = func->findDecoration<IRLayoutDecoration>() )
    {
        return as<EntryPointLayout>(layoutDecoration->getLayout());
    }
    return nullptr;
}

EntryPointLayout* CLikeSourceEmitter::asEntryPoint(IRFunc* func)
{
    if (auto layoutDecoration = func->findDecoration<IRLayoutDecoration>())
    {
        if (auto entryPointLayout = as<EntryPointLayout>(layoutDecoration->getLayout()))
        {
            return entryPointLayout;
        }
    }

    return nullptr;
}

bool CLikeSourceEmitter::isTargetIntrinsic(IRFunc* func)
{
    // For now we do this in an overly simplistic
    // fashion: we say that *any* function declaration
    // (rather then definition) must be an intrinsic:
    return !isDefinition(func);
}

IRFunc* CLikeSourceEmitter::asTargetIntrinsic(IRInst* value)
{
    if(!value)
        return nullptr;

    while (auto specInst = as<IRSpecialize>(value))
    {
        value = getSpecializedValue(specInst);
    }

    if(value->op != kIROp_Func)
        return nullptr;

    IRFunc* func = (IRFunc*) value;
    if(!isTargetIntrinsic(func))
        return nullptr;

    return func;
}

void CLikeSourceEmitter::emitIRFunc(IRFunc* func)
{
    if(!isDefinition(func))
    {
        // This is just a function declaration,
        // and so we want to emit it as such.
        // (Or maybe not emit it at all).

        // We do not emit the declaration for
        // functions that appear to be intrinsics/builtins
        // in the target language.
        if (isTargetIntrinsic(func))
            return;

        emitIRFuncDecl(func);
    }
    else
    {
        // The common case is that what we
        // have is just an ordinary function,
        // and we can emit it as such.
        emitIRSimpleFunc(func);
    }
}

void CLikeSourceEmitter::emitIRStruct(IRStructType* structType)
{
    // If the selected `struct` type is actually an intrinsic
    // on our target, then we don't want to emit anything at all.
    if(auto intrinsicDecoration = findTargetIntrinsicDecoration(structType))
    {
        return;
    }

    m_stream->emit("struct ");
    m_stream->emit(getIRName(structType));
    m_stream->emit("\n{\n");
    m_stream->indent();

    for(auto ff : structType->getFields())
    {
        auto fieldKey = ff->getKey();
        auto fieldType = ff->getFieldType();

        // Filter out fields with `void` type that might
        // have been introduced by legalization.
        if(as<IRVoidType>(fieldType))
            continue;

        // Note: GLSL doesn't support interpolation modifiers on `struct` fields
        if( getSourceStyle() != SourceStyle::GLSL )
        {
            emitInterpolationModifiers(fieldKey, fieldType, nullptr);
        }

        emitIRType(fieldType, getIRName(fieldKey));
        emitIRSemantics(fieldKey);
        m_stream->emit(";\n");
    }

    m_stream->dedent();
    m_stream->emit("};\n\n");
}

void CLikeSourceEmitter::emitIRMatrixLayoutModifiers(VarLayout* layout)
{
    // When a variable has a matrix type, we want to emit an explicit
    // layout qualifier based on what the layout has been computed to be.
    //

    auto typeLayout = layout->typeLayout;
    while(auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
        typeLayout = arrayTypeLayout->elementTypeLayout;

    if (auto matrixTypeLayout = typeLayout.as<MatrixTypeLayout>())
    {
        switch (getSourceStyle())
        {
        case SourceStyle::HLSL:
            switch (matrixTypeLayout->mode)
            {
            case kMatrixLayoutMode_ColumnMajor:
                m_stream->emit("column_major ");
                break;

            case kMatrixLayoutMode_RowMajor:
                m_stream->emit("row_major ");
                break;
            }
            break;

        case SourceStyle::GLSL:
            // Reminder: the meaning of row/column major layout
            // in our semantics is the *opposite* of what GLSL
            // calls them, because what they call "columns"
            // are what we call "rows."
            //
            switch (matrixTypeLayout->mode)
            {
            case kMatrixLayoutMode_ColumnMajor:
                m_stream->emit("layout(row_major)\n");
                break;

            case kMatrixLayoutMode_RowMajor:
                m_stream->emit("layout(column_major)\n");
                break;
            }
            break;

        default:
            break;
        }

    }
}

void CLikeSourceEmitter::maybeEmitGLSLFlatModifier(IRType* valueType)
{
    auto tt = valueType;
    if(auto vecType = as<IRVectorType>(tt))
        tt = vecType->getElementType();
    if(auto vecType = as<IRMatrixType>(tt))
        tt = vecType->getElementType();

    switch(tt->op)
    {
    default:
        break;

    case kIROp_IntType:
    case kIROp_UIntType:
    case kIROp_UInt64Type:
        m_stream->emit("flat ");
        break;
    }
}

void CLikeSourceEmitter::emitInterpolationModifiers(IRInst* varInst, IRType* valueType, VarLayout* layout)
{
    bool isGLSL = (getSourceStyle() == SourceStyle::GLSL);
    bool anyModifiers = false;

    for(auto dd : varInst->getDecorations())
    {
        if(dd->op != kIROp_InterpolationModeDecoration)
            continue;

        auto decoration = (IRInterpolationModeDecoration*)dd;
        auto mode = decoration->getMode();

        switch(mode)
        {
        case IRInterpolationMode::NoInterpolation:
            anyModifiers = true;
            m_stream->emit(isGLSL ? "flat " : "nointerpolation ");
            break;

        case IRInterpolationMode::NoPerspective:
            anyModifiers = true;
            m_stream->emit("noperspective ");
            break;

        case IRInterpolationMode::Linear:
            anyModifiers = true;
            m_stream->emit(isGLSL ? "smooth " : "linear ");
            break;

        case IRInterpolationMode::Sample:
            anyModifiers = true;
            m_stream->emit("sample ");
            break;

        case IRInterpolationMode::Centroid:
            anyModifiers = true;
            m_stream->emit("centroid ");
            break;

        default:
            break;
        }
    }

    // If the user didn't explicitly qualify a varying
    // with integer type, then we need to explicitly
    // add the `flat` modifier for GLSL.
    if(!anyModifiers && isGLSL)
    {
        // Only emit a default `flat` for fragment
        // stage varying inputs.
        //
        // TODO: double-check that this works for
        // signature matching even if the producing
        // stage didn't use `flat`.
        //
        // If this ends up being a problem we can instead
        // output everything with `flat` except for
        // fragment *outputs* (and maybe vertex inputs).
        //
        if(layout && layout->stage == Stage::Fragment
            && layout->FindResourceInfo(LayoutResourceKind::VaryingInput))
        {
            maybeEmitGLSLFlatModifier(valueType);
        }
    }
}

UInt CLikeSourceEmitter::getRayPayloadLocation(IRInst* inst)
{
    auto& map = m_context->mapIRValueToRayPayloadLocation;
    UInt value = 0;
    if(map.TryGetValue(inst, value))
        return value;

    value = map.Count();
    map.Add(inst, value);
    return value;
}

UInt CLikeSourceEmitter::getCallablePayloadLocation(IRInst* inst)
{
    auto& map = m_context->mapIRValueToCallablePayloadLocation;
    UInt value = 0;
    if(map.TryGetValue(inst, value))
        return value;

    value = map.Count();
    map.Add(inst, value);
    return value;
}

void CLikeSourceEmitter::emitGLSLImageFormatModifier(IRInst* var, IRTextureType* resourceType)
{
    // If the user specified a format manually, using `[format(...)]`,
    // then we will respect that format and emit a matching `layout` modifier.
    //
    if(auto formatDecoration = var->findDecoration<IRFormatDecoration>())
    {
        auto format = formatDecoration->getFormat();
        if(format == ImageFormat::unknown)
        {
            // If the user explicitly opts out of having a format, then
            // the output shader will require the extension to support
            // load/store from format-less images.
            //
            // TODO: We should have a validation somewhere in the compiler
            // that atomic operations are only allowed on images with
            // explicit formats (and then only on specific formats).
            // This is really an argument that format should be part of
            // the image *type* (with a "base type" for images with
            // unknown format).
            //
            requireGLSLExtension("GL_EXT_shader_image_load_formatted");
        }
        else
        {
            // If there is an explicit format specified, then we
            // should emit a `layout` modifier using the GLSL name
            // for the format.
            //
            m_stream->emit("layout(");
            m_stream->emit(getGLSLNameForImageFormat(format));
            m_stream->emit(")\n");
        }

        // No matter what, if an explicit `[format(...)]` was given,
        // then we don't need to emit anything else.
        //
        return;
    }


    // When no explicit format is specified, we need to either
    // emit the image as having an unknown format, or else infer
    // a format from the type.
    //
    // For now our default behavior is to infer (so that unmodified
    // HLSL input is more likely to generate valid SPIR-V that
    // runs anywhere), but we provide a flag to opt into
    // treating images without explicit formats as having
    // unknown format.
    //
    if(this->m_context->compileRequest->useUnknownImageFormatAsDefault)
    {
        requireGLSLExtension("GL_EXT_shader_image_load_formatted");
        return;
    }

    // At this point we have a resource type like `RWTexture2D<X>`
    // and we want to infer a reasonable format from the element
    // type `X` that was specified.
    //
    // E.g., if `X` is `float` then we can infer a format like `r32f`,
    // and so forth. The catch of course is that it is possible to
    // specify a shader parameter with a type like `RWTexture2D<float4>` but
    // provide an image at runtime with a format like `rgba8`, so
    // this inference is never guaranteed to give perfect results.
    //
    // If users don't like our inferred result, they need to use a
    // `[format(...)]` attribute to manually specify what they want.
    //
    // TODO: We should consider whether we can expand the space of
    // allowed types for `X` in `RWTexture2D<X>` to include special
    // pseudo-types that act just like, e.g., `float4`, but come
    // with attached/implied format information.
    //
    auto elementType = resourceType->getElementType();
    Int vectorWidth = 1;
    if(auto elementVecType = as<IRVectorType>(elementType))
    {
        if(auto intLitVal = as<IRIntLit>(elementVecType->getElementCount()))
        {
            vectorWidth = (Int) intLitVal->getValue();
        }
        else
        {
            vectorWidth = 0;
        }
        elementType = elementVecType->getElementType();
    }
    if(auto elementBasicType = as<IRBasicType>(elementType))
    {
        m_stream->emit("layout(");
        switch(vectorWidth)
        {
        default: m_stream->emit("rgba");  break;

        case 3:
        {
            // TODO: GLSL doesn't support 3-component formats so for now we are going to
            // default to rgba
            //
            // The SPIR-V spec (https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.pdf)
            // section 3.11 on Image Formats it does not list rgbf32.
            //
            // It seems SPIR-V can support having an image with an unknown-at-compile-time
            // format, so long as the underlying API supports it. Ideally this would mean that we can
            // just drop all these qualifiers when emitting GLSL for Vulkan targets.
            //
            // This raises the question of what to do more long term. For Vulkan hopefully we can just
            // drop the layout. For OpenGL targets it would seem reasonable to have well-defined rules
            // for inferring the format (and just document that 3-component formats map to 4-component formats,
            // but that shouldn't matter because the API wouldn't let the user allocate those 3-component formats anyway),
            // and add an attribute for specifying the format manually if you really want to override our
            // inference (e.g., to specify r11fg11fb10f).

            m_stream->emit("rgba");
            //Emit("rgb");                                
            break;
        }

        case 2:  m_stream->emit("rg");    break;
        case 1:  m_stream->emit("r");     break;
        }
        switch(elementBasicType->getBaseType())
        {
        default:
        case BaseType::Float:   m_stream->emit("32f");  break;
        case BaseType::Half:    m_stream->emit("16f");  break;
        case BaseType::UInt:    m_stream->emit("32ui"); break;
        case BaseType::Int:     m_stream->emit("32i"); break;

        // TODO: Here are formats that are available in GLSL,
        // but that are not handled by the above cases.
        //
        // r11f_g11f_b10f
        //
        // rgba16
        // rgb10_a2
        // rgba8
        // rg16
        // rg8
        // r16
        // r8
        //
        // rgba16_snorm
        // rgba8_snorm
        // rg16_snorm
        // rg8_snorm
        // r16_snorm
        // r8_snorm
        //
        // rgba16i
        // rgba8i
        // rg16i
        // rg8i
        // r16i
        // r8i
        //
        // rgba16ui
        // rgb10_a2ui
        // rgba8ui
        // rg16ui
        // rg8ui
        // r16ui
        // r8ui
        }
        m_stream->emit(")\n");
    }
}

    /// Emit modifiers that should apply even for a declaration of an SSA temporary.
void CLikeSourceEmitter::emitIRTempModifiers(IRInst* temp)
{
    if(temp->findDecoration<IRPreciseDecoration>())
    {
        m_stream->emit("precise ");
    }
}

void CLikeSourceEmitter::emitIRVarModifiers(VarLayout* layout,IRInst* varDecl, IRType* varType)
{
    // Deal with Vulkan raytracing layout stuff *before* we
    // do the check for whether `layout` is null, because
    // the payload won't automatically get a layout applied
    // (it isn't part of the user-visible interface...)
    //
    if(varDecl->findDecoration<IRVulkanRayPayloadDecoration>())
    {
        m_stream->emit("layout(location = ");
        m_stream->emit(getRayPayloadLocation(varDecl));
        m_stream->emit(")\n");
        m_stream->emit("rayPayloadNV\n");
    }
    if(varDecl->findDecoration<IRVulkanCallablePayloadDecoration>())
    {
        m_stream->emit("layout(location = ");
        m_stream->emit(getCallablePayloadLocation(varDecl));
        m_stream->emit(")\n");
        m_stream->emit("callableDataNV\n");
    }

    if(varDecl->findDecoration<IRVulkanHitAttributesDecoration>())
    {
        m_stream->emit("hitAttributeNV\n");
    }

    if(varDecl->findDecoration<IRGloballyCoherentDecoration>())
    {
        switch(getSourceStyle())
        {
        default:
            break;

        case SourceStyle::HLSL:
            m_stream->emit("globallycoherent\n");
            break;

        case SourceStyle::GLSL:
            m_stream->emit("coherent\n");
            break;
        }
    }

    emitIRTempModifiers(varDecl);

    if (!layout)
        return;

    emitIRMatrixLayoutModifiers(layout);

    // As a special case, if we are emitting a GLSL declaration
    // for an HLSL `RWTexture*` then we need to emit a `format` layout qualifier.
    if(getSourceStyle() == SourceStyle::GLSL)
    {
        if(auto resourceType = as<IRTextureType>(unwrapArray(varType)))
        {
            switch(resourceType->getAccess())
            {
            case SLANG_RESOURCE_ACCESS_READ_WRITE:
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                {
                    emitGLSLImageFormatModifier(varDecl, resourceType);
                }
                break;

            default:
                break;
            }
        }
    }

    if(layout->FindResourceInfo(LayoutResourceKind::VaryingInput)
        || layout->FindResourceInfo(LayoutResourceKind::VaryingOutput))
    {
        emitInterpolationModifiers(varDecl, varType, layout);
    }

    if (getSourceStyle() == SourceStyle::GLSL)
    {
        // Layout-related modifiers need to come before the declaration,
        // so deal with them here.
        emitGLSLLayoutQualifiers(layout, nullptr);

        // try to emit an appropriate leading qualifier
        for (auto rr : layout->resourceInfos)
        {
            switch (rr.kind)
            {
            case LayoutResourceKind::Uniform:
            case LayoutResourceKind::ShaderResource:
            case LayoutResourceKind::DescriptorTableSlot:
                m_stream->emit("uniform ");
                break;

            case LayoutResourceKind::VaryingInput:
                {
                    m_stream->emit("in ");
                }
                break;

            case LayoutResourceKind::VaryingOutput:
                {
                    m_stream->emit("out ");
                }
                break;

            case LayoutResourceKind::RayPayload:
                {
                    m_stream->emit("rayPayloadInNV ");
                }
                break;

            case LayoutResourceKind::CallablePayload:
                {
                    m_stream->emit("callableDataInNV ");
                }
                break;

            case LayoutResourceKind::HitAttributes:
                {
                    m_stream->emit("hitAttributeNV ");
                }
                break;

            default:
                continue;
            }

            break;
        }
    }
}

void CLikeSourceEmitter::emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    if(as<IRTextureBufferType>(type))
    {
        m_stream->emit("tbuffer ");
    }
    else
    {
        m_stream->emit("cbuffer ");
    }
    m_stream->emit(getIRName(varDecl));

    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    EmitVarChain blockChain(varLayout);

    EmitVarChain containerChain = blockChain;
    EmitVarChain elementChain = blockChain;

    auto typeLayout = varLayout->typeLayout;
    if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout) )
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

        typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
    }

    emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

    m_stream->emit("\n{\n");
    m_stream->indent();

    auto elementType = type->getElementType();

    emitIRType(elementType, getIRName(varDecl));
    m_stream->emit(";\n");

    m_stream->dedent();
    m_stream->emit("}\n");
}

void CLikeSourceEmitter::emitArrayBrackets(IRType* inType)
{
    // A declaration may require zero, one, or
    // more array brackets. When writing out array
    // brackets from left to right, they represent
    // the structure of the type from the "outside"
    // in (that is, if we have a 5-element array of
    // 3-element arrays we should output `[5][3]`),
    // because of C-style declarator rules.
    //
    // This conveniently means that we can print
    // out all the array brackets with a looping
    // rather than a recursive structure.
    //
    // We will peel the input type like an onion,
    // looking at one layer at a time until we
    // reach a non-array type in the middle.
    //
    IRType* type = inType;
    for(;;)
    {
        if(auto arrayType = as<IRArrayType>(type))
        {
            m_stream->emit("[");
            emitVal(arrayType->getElementCount(), getInfo(EmitOp::General));
            m_stream->emit("]");

            // Continue looping on the next layer in.
            //
            type = arrayType->getElementType();
        }
        else if(auto unsizedArrayType = as<IRUnsizedArrayType>(type))
        {
            m_stream->emit("[]");

            // Continue looping on the next layer in.
            //
            type = unsizedArrayType->getElementType();
        }
        else
        {
            // This layer wasn't an array, so we are done.
            //
            return;
        }
    }
}


void CLikeSourceEmitter::emitGLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    EmitVarChain blockChain(varLayout);

    EmitVarChain containerChain = blockChain;
    EmitVarChain elementChain = blockChain;

    auto typeLayout = varLayout->typeLayout->unwrapArray();
    if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout) )
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

        typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
    }

    /*
    With resources backed by 'buffer' on glsl, we want to output 'readonly' if that is a good match
    for the underlying type. If uniform it's implicit it's readonly

    Here this only happens with isShaderRecord which is a 'constant buffer' (ie implicitly readonly)
    or IRGLSLShaderStorageBufferType which is read write.
    */

    emitGLSLLayoutQualifier(LayoutResourceKind::DescriptorTableSlot, &containerChain);
    emitGLSLLayoutQualifier(LayoutResourceKind::PushConstantBuffer, &containerChain);
    bool isShaderRecord = emitGLSLLayoutQualifier(LayoutResourceKind::ShaderRecord, &containerChain);

    if( isShaderRecord )
    {
        // TODO: A shader record in vk can be potentially read-write. Currently slang doesn't support write access
        // and readonly buffer generates SPIRV validation error.
        m_stream->emit("buffer ");
    }
    else if(as<IRGLSLShaderStorageBufferType>(type))
    {
        // Is writable 
        m_stream->emit("layout(std430) buffer ");
    }
    // TODO: what to do with HLSL `tbuffer` style buffers?
    else
    {
        // uniform is implicitly read only
        m_stream->emit("layout(std140) uniform ");
    }

    // Generate a dummy name for the block
    m_stream->emit("_S");
    m_stream->emit(m_context->uniqueIDCounter++);

    m_stream->emit("\n{\n");
    m_stream->indent();

    auto elementType = type->getElementType();

    emitIRType(elementType, "_data");
    m_stream->emit(";\n");

    m_stream->dedent();
    m_stream->emit("} ");

    m_stream->emit(getIRName(varDecl));

    // If the underlying variable was an array (or array of arrays, etc.)
    // we need to emit all those array brackets here.
    emitArrayBrackets(varDecl->getDataType());

    m_stream->emit(";\n");
}

void CLikeSourceEmitter::emitIRParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    switch (getSourceStyle())
    {
    case SourceStyle::HLSL:
        emitHLSLParameterGroup(varDecl, type);
        break;

    case SourceStyle::GLSL:
        emitGLSLParameterGroup(varDecl, type);
        break;
    }
}

void CLikeSourceEmitter::emitIRVar(IRVar* varDecl)
{
    auto allocatedType = varDecl->getDataType();
    auto varType = allocatedType->getValueType();
//        auto addressSpace = allocatedType->getAddressSpace();

#if 0
    switch( varType->op )
    {
    case kIROp_ConstantBufferType:
    case kIROp_TextureBufferType:
        emitIRParameterGroup(ctx, varDecl, (IRUniformBufferType*) varType);
        return;

    default:
        break;
    }
#endif

    // Need to emit appropriate modifiers here.

    auto layout = getVarLayout(varDecl);

    emitIRVarModifiers(layout, varDecl, varType);

#if 0
    switch (addressSpace)
    {
    default:
        break;

    case kIRAddressSpace_GroupShared:
        emit("groupshared ");
        break;
    }
#endif
    emitIRRateQualifiers(varDecl);

    emitIRType(varType, getIRName(varDecl));

    emitIRSemantics(varDecl);

    emitIRLayoutSemantics(varDecl);

    m_stream->emit(";\n");
}

void CLikeSourceEmitter::emitIRStructuredBuffer_GLSL(IRGlobalParam* varDecl, IRHLSLStructuredBufferTypeBase* structuredBufferType)
{
    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    requireGLSLVersion(430);

    m_stream->emit("layout(std430");

    auto layout = getVarLayout(varDecl);
    if (layout)
    {
        LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
        EmitVarChain chain(layout);

        const UInt index = getBindingOffset(&chain, kind);
        const UInt space = getBindingSpace(&chain, kind);

        m_stream->emit(", binding = ");
        m_stream->emit(index);
        if (space)
        {
            m_stream->emit(", set = ");
            m_stream->emit(space);
        }
    }
        
    m_stream->emit(") ");

    /*
    If the output type is a buffer, and we can determine it is only readonly we can prefix before
    buffer with 'readonly'

    The actual structuredBufferType could be

    HLSLStructuredBufferType                        - This is unambiguously read only
    HLSLRWStructuredBufferType                      - Read write
    HLSLRasterizerOrderedStructuredBufferType       - Allows read/write access
    HLSLAppendStructuredBufferType                  - Write
    HLSLConsumeStructuredBufferType                 - TODO (JS): Its possible that this can be readonly, but we currently don't support on GLSL
    */

    if (as<IRHLSLStructuredBufferType>(structuredBufferType))
    {
        m_stream->emit("readonly ");
    }

    m_stream->emit("buffer ");
    
    // Generate a dummy name for the block
    m_stream->emit("_S");
    m_stream->emit(m_context->uniqueIDCounter++);

    m_stream->emit(" {\n");
    m_stream->indent();


    auto elementType = structuredBufferType->getElementType();
    emitIRType(elementType, "_data[]");
    m_stream->emit(";\n");

    m_stream->dedent();
    m_stream->emit("} ");

    m_stream->emit(getIRName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_stream->emit(";\n");
}

void CLikeSourceEmitter::emitIRByteAddressBuffer_GLSL(IRGlobalParam* varDecl, IRByteAddressBufferTypeBase* byteAddressBufferType)
{
    // TODO: A lot of this logic is copy-pasted from `emitIRStructuredBuffer_GLSL`.
    // It might be worthwhile to share the common code to avoid regressions sneaking
    // in when one or the other, but not both, gets updated.

    // Shader storage buffer is an OpenGL 430 feature
    //
    // TODO: we should require either the extension or the version...
    requireGLSLVersion(430);

    m_stream->emit("layout(std430");

    auto layout = getVarLayout(varDecl);
    if (layout)
    {
        LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
        EmitVarChain chain(layout);

        const UInt index = getBindingOffset(&chain, kind);
        const UInt space = getBindingSpace(&chain, kind);

        m_stream->emit(", binding = ");
        m_stream-> emit(index);
        if (space)
        {
            m_stream->emit(", set = ");
            m_stream->emit(space);
        }
    }

    m_stream->emit(") ");

    /*
    If the output type is a buffer, and we can determine it is only readonly we can prefix before
    buffer with 'readonly'

    HLSLByteAddressBufferType                   - This is unambiguously read only
    HLSLRWByteAddressBufferType                 - Read write
    HLSLRasterizerOrderedByteAddressBufferType  - Allows read/write access
    */

    if (as<IRHLSLByteAddressBufferType>(byteAddressBufferType))
    {
        m_stream->emit("readonly ");
    }

    m_stream->emit("buffer ");

    // Generate a dummy name for the block
    m_stream->emit("_S");
    m_stream->emit(m_context->uniqueIDCounter++);
    m_stream->emit("\n{\n");
    m_stream->indent();

    m_stream->emit("uint _data[];\n");

    m_stream->dedent();
    m_stream->emit("} ");

    m_stream->emit(getIRName(varDecl));
    emitArrayBrackets(varDecl->getDataType());

    m_stream->emit(";\n");
}

void CLikeSourceEmitter::emitIRGlobalVar(IRGlobalVar* varDecl)
{
    auto allocatedType = varDecl->getDataType();
    auto varType = allocatedType->getValueType();

    String initFuncName;
    if (varDecl->getFirstBlock())
    {
        // A global variable with code means it has an initializer
        // associated with it. Eventually we'd like to emit that
        // initializer directly as an expression here, but for
        // now we'll emit it as a separate function.

        initFuncName = getIRName(varDecl);
        initFuncName.append("_init");

        m_stream->emit("\n");
        emitIRType(varType, initFuncName);
        m_stream->emit("()\n{\n");
        m_stream->indent();
        emitIRFunctionBody(varDecl);
        m_stream->dedent();
        m_stream->emit("}\n");
    }

    // An ordinary global variable won't have a layout
    // associated with it, since it is not a shader
    // parameter.
    //
    SLANG_ASSERT(!getVarLayout(varDecl));
    VarLayout* layout = nullptr;

    // An ordinary global variable (which is not a
    // shader parameter) may need special
    // modifiers to indicate it as such.
    //
    switch (getSourceStyle())
    {
    case SourceStyle::HLSL:
        // HLSL requires the `static` modifier on any
        // global variables; otherwise they are assumed
        // to be uniforms.
        m_stream->emit("static ");
        break;

    default:
        break;
    }

    emitIRVarModifiers(layout, varDecl, varType);

    emitIRRateQualifiers(varDecl);
    emitIRType(varType, getIRName(varDecl));

    // TODO: These shouldn't be needed for ordinary
    // global variables.
    //
    emitIRSemantics(varDecl);
    emitIRLayoutSemantics(varDecl);

    if (varDecl->getFirstBlock())
    {
        m_stream->emit(" = ");
        m_stream->emit(initFuncName);
        m_stream->emit("()");
    }

    m_stream->emit(";\n\n");
}

void CLikeSourceEmitter::emitIRGlobalParam(IRGlobalParam* varDecl)
{
    auto rawType = varDecl->getDataType();

    auto varType = rawType;
    if( auto outType = as<IROutTypeBase>(varType) )
    {
        varType = outType->getValueType();
    }
    if (as<IRVoidType>(varType))
        return;

    // When a global shader parameter represents a "parameter group"
    // (either a constant buffer or a parameter block with non-resource
    // data in it), we will prefer to emit it as an ordinary `cbuffer`
    // declaration or `uniform` block, even when emitting HLSL for
    // D3D profiles that support the explicit `ConstantBuffer<T>` type.
    //
    // Alternatively, we could make this choice based on profile, and
    // prefer `ConstantBuffer<T>` on profiles that support it and/or when
    // the input code used that syntax.
    //
    if (auto paramBlockType = as<IRUniformParameterGroupType>(varType))
    {
        emitIRParameterGroup(varDecl, paramBlockType);
        return;
    }

    if(getSourceStyle() == SourceStyle::GLSL)
    {
        // There are a number of types that are (or can be)
        // "first-class" in D3D HLSL, but are second-class in GLSL in
        // that they require explicit global declarations for each value/object,
        // and don't support declaration as ordinary variables.
        //
        // This includes constant buffers (`uniform` blocks) and well as
        // structured and byte-address buffers (both mapping to `buffer` blocks).
        //
        // We intercept these types, and arrays thereof, to produce the required
        // global declarations. This assumes that earlier "legalization" passes
        // already performed the work of pulling fields with these types out of
        // aggregates.
        //
        // Note: this also assumes that these types are not used as function
        // parameters/results, local variables, etc. Additional legalization
        // steps are required to guarantee these conditions.
        //
        if (auto paramBlockType = as<IRUniformParameterGroupType>(unwrapArray(varType)))
        {
            emitGLSLParameterGroup(varDecl, paramBlockType);
            return;
        }
        if( auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(unwrapArray(varType)) )
        {
            emitIRStructuredBuffer_GLSL(varDecl, structuredBufferType);
            return;
        }
        if( auto byteAddressBufferType = as<IRByteAddressBufferTypeBase>(unwrapArray(varType)) )
        {
            emitIRByteAddressBuffer_GLSL(varDecl, byteAddressBufferType);
            return;
        }

        // We want to skip the declaration of any system-value variables
        // when outputting GLSL (well, except in the case where they
        // actually *require* redeclaration...).
        //
        // Note: these won't be variables the user declare explicitly
        // in their code, but rather variables that we generated as
        // part of legalizing the varying input/output signature of
        // an entry point for GL/Vulkan.
        //
        // TODO: This could be handled more robustly by attaching an
        // appropriate decoration to these variables to indicate their
        // purpose.
        //
        if(auto linkageDecoration = varDecl->findDecoration<IRLinkageDecoration>())
        {
            if(linkageDecoration->getMangledName().startsWith("gl_"))
            {
                // The variable represents an OpenGL system value,
                // so we will assume that it doesn't need to be declared.
                //
                // TODO: handle case where we *should* declare the variable.
                return;
            }
        }

        // When emitting unbounded-size resource arrays with GLSL we need
        // to use the `GL_EXT_nonuniform_qualifier` extension to ensure
        // that they are not treated as "implicitly-sized arrays" which
        // are arrays that have a fixed size that just isn't specified
        // at the declaration site (instead being inferred from use sites).
        //
        // While the extension primarily introduces the `nonuniformEXT`
        // qualifier that we use to implement `NonUniformResourceIndex`,
        // it also changes the GLSL language semantics around (resource) array
        // declarations that don't specify a size.
        //
        if( as<IRUnsizedArrayType>(varType) )
        {
            if(isResourceType(unwrapArray(varType)))
            {
                requireGLSLExtension("GL_EXT_nonuniform_qualifier");
            }
        }
    }

    // Need to emit appropriate modifiers here.

    // We expect/require all shader parameters to
    // have some kind of layout information associated with them.
    //
    auto layout = getVarLayout(varDecl);
    SLANG_ASSERT(layout);

    emitIRVarModifiers(layout, varDecl, varType);

    emitIRRateQualifiers(varDecl);
    emitIRType(varType, getIRName(varDecl));

    emitIRSemantics(varDecl);

    emitIRLayoutSemantics(varDecl);

    // A shader parameter cannot have an initializer,
    // so we do need to consider emitting one here.

    m_stream->emit(";\n\n");
}


void CLikeSourceEmitter::emitIRGlobalConstantInitializer(IRGlobalConstant* valDecl)
{
    // We expect to see only a single block
    auto block = valDecl->getFirstBlock();
    SLANG_RELEASE_ASSERT(block);
    SLANG_RELEASE_ASSERT(!block->getNextBlock());

    // We expect the terminator to be a `return`
    // instruction with a value.
    auto returnInst = (IRReturnVal*) block->getLastDecorationOrChild();
    SLANG_RELEASE_ASSERT(returnInst->op == kIROp_ReturnVal);

    // We will emit the value in the `GlobalConstant` mode, which
    // more or less says to fold all instructions into their use
    // sites, so that we end up with a single expression tree even
    // in cases that would otherwise trip up our analysis.
    //
    // Note: We are emitting the value as an *operand* here instead
    // of directly calling `emitIRInstExpr` because we need to handle
    // cases where the value might *need* to emit as a named referenced
    // (e.g., when it names another constant directly).
    //
    emitIROperand(returnInst->getVal(), IREmitMode::GlobalConstant, getInfo(EmitOp::General));
}

void CLikeSourceEmitter::emitIRGlobalConstant(IRGlobalConstant* valDecl)
{
    auto valType = valDecl->getDataType();

    if( getSourceStyle() != SourceStyle::GLSL )
    {
        m_stream->emit("static ");
    }
    m_stream->emit("const ");
    emitIRRateQualifiers(valDecl);
    emitIRType(valType, getIRName(valDecl));

    if (valDecl->getFirstBlock())
    {
        // There is an initializer (which we expect for
        // any global constant...).

        m_stream->emit(" = ");

        // We need to emit the entire initializer as
        // a single expression.
        emitIRGlobalConstantInitializer(valDecl);
    }


    m_stream->emit(";\n");
}

void CLikeSourceEmitter::emitIRGlobalInst(IRInst* inst)
{
    m_stream->advanceToSourceLocation(inst->sourceLoc);

    switch(inst->op)
    {
    case kIROp_Func:
        emitIRFunc((IRFunc*) inst);
        break;

    case kIROp_GlobalVar:
        emitIRGlobalVar((IRGlobalVar*) inst);
        break;

    case kIROp_GlobalParam:
        emitIRGlobalParam((IRGlobalParam*) inst);
        break;

    case kIROp_GlobalConstant:
        emitIRGlobalConstant((IRGlobalConstant*) inst);
        break;

    case kIROp_Var:
        emitIRVar((IRVar*) inst);
        break;

    case kIROp_StructType:
        emitIRStruct(cast<IRStructType>(inst));
        break;

    default:
        break;
    }
}

void CLikeSourceEmitter::ensureInstOperand(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel)
{
    if(!inst) return;

    if(inst->getParent() == ctx->moduleInst)
    {
        ensureGlobalInst(ctx, inst, requiredLevel);
    }
}

void CLikeSourceEmitter::ensureInstOperandsRec(ComputeEmitActionsContext* ctx, IRInst* inst)
{
    ensureInstOperand(ctx, inst->getFullType());

    UInt operandCount = inst->operandCount;
    for(UInt ii = 0; ii < operandCount; ++ii)
    {
        // TODO: there are some special cases we can add here,
        // to avoid outputting full definitions in cases that
        // can get by with forward declarations.
        //
        // For example, true pointer types should (in principle)
        // only need the type they point to to be forward-declared.
        // Similarly, a `call` instruction only needs the callee
        // to be forward-declared, etc.

        ensureInstOperand(ctx, inst->getOperand(ii));
    }

    for(auto child : inst->getDecorationsAndChildren())
    {
        ensureInstOperandsRec(ctx, child);
    }
}

void CLikeSourceEmitter::ensureGlobalInst(ComputeEmitActionsContext* ctx, IRInst* inst, EmitAction::Level requiredLevel)
{
    // Skip certain instructions, since they
    // don't affect output.
    switch(inst->op)
    {
    case kIROp_WitnessTable:
    case kIROp_Generic:
        return;

    default:
        break;
    }

    // Have we already processed this instruction?
    EmitAction::Level existingLevel;
    if(ctx->mapInstToLevel.TryGetValue(inst, existingLevel))
    {
        // If we've already emitted it suitably,
        // then don't worry about it.
        if(existingLevel >= requiredLevel)
            return;
    }

    EmitAction action;
    action.level = requiredLevel;
    action.inst = inst;

    if(requiredLevel == EmitAction::Level::Definition)
    {
        if(ctx->openInsts.Contains(inst))
        {
            SLANG_UNEXPECTED("circularity during codegen");
            return;
        }

        ctx->openInsts.Add(inst);

        ensureInstOperandsRec(ctx, inst);

        ctx->openInsts.Remove(inst);
    }

    ctx->mapInstToLevel[inst] = requiredLevel;
    ctx->actions->add(action);
}

void CLikeSourceEmitter::computeIREmitActions(IRModule* module, List<EmitAction>& ioActions)
{
    ComputeEmitActionsContext ctx;
    ctx.moduleInst = module->getModuleInst();
    ctx.actions = &ioActions;

    for(auto inst : module->getGlobalInsts())
    {
        if( as<IRType>(inst) )
        {
            // Don't emit a type unless it is actually used.
            continue;
        }

        ensureGlobalInst(&ctx, inst, EmitAction::Level::Definition);
    }
}

void CLikeSourceEmitter::executeIREmitActions(List<EmitAction> const& actions)
{
    for(auto action : actions)
    {
        switch(action.level)
        {
        case EmitAction::Level::ForwardDeclaration:
            emitIRFuncDecl(cast<IRFunc>(action.inst));
            break;

        case EmitAction::Level::Definition:
            emitIRGlobalInst(action.inst);
            break;
        }
    }
}

void CLikeSourceEmitter::emitIRModule(IRModule* module)
{
    // The IR will usually come in an order that respects
    // dependencies between global declarations, but this
    // isn't guaranteed, so we need to be careful about
    // the order in which we emit things.

    List<EmitAction> actions;

    computeIREmitActions(module, actions);
    executeIREmitActions(actions);
}

} // namespace Slang
