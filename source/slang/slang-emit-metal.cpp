// slang-emit-metal.cpp
#include "slang-emit-metal.h"

#include "../core/slang-writer.h"

#include "slang-ir-util.h"
#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

static const char* kMetalBuiltinPreludeMatrixCompMult = R"(
template<typename T, int A, int B>
matrix<T,A,B> _slang_matrixCompMult(matrix<T,A,B> m1, matrix<T,A,B> m2)
{
    matrix<T,A,B> result;
    for (int i = 0; i < A; i++)
        result[i] = m1[i] * m2[i];
    return result;
}
)";

static const char* kMetalBuiltinPreludeMatrixReshape = R"(
template<int A, int B, typename T, int N, int M>
matrix<T,A,B> _slang_matrixReshape(matrix<T,N,M> m)
{
    matrix<T,A,B> result = T(0);
    for (int i = 0; i < min(A,N); i++)
        for (int j = 0; j < min(B,M); j++)
            result[i] = m[i][j];
    return result;
}
)";

static const char* kMetalBuiltinPreludeVectorReshape = R"(
template<int A, typename T, int N>
vec<T,A> _slang_vectorReshape(vec<T,N> v)
{
    vec<T,A> result = T(0);
    for (int i = 0; i < min(A,N); i++)
        result[i] = v[i];
    return result;
}
)";

void MetalSourceEmitter::_emitHLSLDecorationSingleString(const char* name, IRFunc* entryPoint, IRStringLit* val)
{
    SLANG_UNUSED(entryPoint);
    assert(val);

    m_writer->emit("[[");
    m_writer->emit(name);
    m_writer->emit("(\"");
    m_writer->emit(val->getStringSlice());
    m_writer->emit("\")]]\n");
}

void MetalSourceEmitter::_emitHLSLDecorationSingleInt(const char* name, IRFunc* entryPoint, IRIntLit* val)
{
    SLANG_UNUSED(entryPoint);
    SLANG_ASSERT(val);

    auto intVal = getIntVal(val);

    m_writer->emit("[[");
    m_writer->emit(name);
    m_writer->emit("(");
    m_writer->emit(intVal);
    m_writer->emit(")]]\n");
}

void MetalSourceEmitter::_emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    // Metal does not allow shader parameters declared as global variables, so we shouldn't see this.
    SLANG_UNUSED(varDecl);
    SLANG_UNUSED(type);
    SLANG_ASSERT(!"Metal does not allow shader parameters declared as global variables.");
}

void MetalSourceEmitter::_emitHLSLTextureType(IRTextureTypeBase* texType)
{
    if (getIntVal(texType->getIsShadowInst()) != 0)
    {
        m_writer->emit("depth");
    }
    else
    {
        m_writer->emit("texture");
    }

    switch (texType->GetBaseShape())
    {
        case SLANG_TEXTURE_1D:		m_writer->emit("1d");		break;
        case SLANG_TEXTURE_2D:		m_writer->emit("2d");		break;
        case SLANG_TEXTURE_3D:		m_writer->emit("3d");		break;
        case SLANG_TEXTURE_CUBE:	m_writer->emit("cube");	    break;
        case SLANG_TEXTURE_BUFFER:  m_writer->emit("_buffer");           break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            break;
    }

    if (texType->isMultisample())
    {
        m_writer->emit("_ms");
    }
    if (texType->isArray())
    {
        m_writer->emit("_array");
    }
    m_writer->emit("<");
    emitType(getVectorElementType(texType->getElementType()));
    m_writer->emit(", ");
    
    switch (texType->getAccess())
    {
    case SLANG_RESOURCE_ACCESS_READ:
        m_writer->emit("access::sample");
        break;

    case SLANG_RESOURCE_ACCESS_READ_WRITE:
    case SLANG_RESOURCE_ACCESS_APPEND:
    case SLANG_RESOURCE_ACCESS_CONSUME:
    case SLANG_RESOURCE_ACCESS_FEEDBACK:
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        m_writer->emit("access::read_write");
        break;
    default:
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
        break;
    }

    m_writer->emit(">");
}

void MetalSourceEmitter::emitFuncParamLayoutImpl(IRInst* param)
{
    auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecoration)
        return;
    auto layout = as<IRVarLayout>(layoutDecoration->getLayout());
    if (!layout)
        return;

    for (auto rr : layout->getOffsetAttrs())
    {
        switch (rr->getResourceKind())
        {
        case LayoutResourceKind::MetalTexture:
            if (as<IRTextureTypeBase>(param->getDataType()) || as<IRTextureBufferType>(param->getDataType()))
            {
                m_writer->emit(" [[texture(");
                m_writer->emit(rr->getOffset());
                m_writer->emit(")]]");
            }
            break;
        case LayoutResourceKind::MetalBuffer:
            if (as<IRPtrTypeBase>(param->getDataType()) || as<IRHLSLStructuredBufferTypeBase>(param->getDataType()) ||
                as<IRByteAddressBufferTypeBase>(param->getDataType()) ||
                as<IRUniformParameterGroupType>(param->getDataType()))
            {
                m_writer->emit(" [[buffer(");
                m_writer->emit(rr->getOffset());
                m_writer->emit(")]]");
            }
            break;
        case LayoutResourceKind::SamplerState:
            if (as<IRSamplerStateTypeBase>(param->getDataType()))
            {
                m_writer->emit(" [[sampler(");
                m_writer->emit(rr->getOffset());
                m_writer->emit(")]]");
            }
            break;
        case LayoutResourceKind::VaryingInput:
            m_writer->emit(" [[stage_in]]");
            break;
        case LayoutResourceKind::MetalPayload:
            m_writer->emit(" [[payload]]");
            break;
        }
    }
    if (!maybeEmitSystemSemantic(param))
    {
        if (auto sysSemanticAttr = layout->findSystemValueSemanticAttr())
            _emitUserSemantic(sysSemanticAttr->getName(), sysSemanticAttr->getIndex());
    }
}

void MetalSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    _emitHLSLParameterGroup(varDecl, type);
}

void MetalSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    auto stage = entryPointDecor->getProfile().getStage();

    switch (stage)
    {
    case Stage::Fragment:
        m_writer->emit("[[fragment]] ");
        break;
    case Stage::Vertex:
        m_writer->emit("[[vertex]] ");
        break;
    case Stage::Compute:
        m_writer->emit("[[kernel]] ");
        break;
    case Stage::Mesh:
        m_writer->emit("[[mesh]] ");
        break;
    case Stage::Amplification:
        m_writer->emit("[[object]] ");
        break;
    default:
        SLANG_ABORT_COMPILATION("unsupported stage.");
    }

    switch (stage)
    {
    case Stage::Pixel:
        {
            if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
            {
                m_writer->emit("[[early_fragment_tests]]\n");
            }
            break;
        }
        default:
            break;
    }
}

void MetalSourceEmitter::ensurePrelude(const char* preludeText)
{
    IRStringLit* stringLit;
    if (!m_builtinPreludes.tryGetValue(preludeText, stringLit))
    {
        IRBuilder builder(m_irModule);
        stringLit = builder.getStringValue(UnownedStringSlice(preludeText));
        m_builtinPreludes[preludeText] = stringLit;
    }
    m_requiredPreludes.add(stringLit);
}

void MetalSourceEmitter::emitMemoryOrderOperand(IRInst* inst)
{
    auto memoryOrder = (IRMemoryOrder)getIntVal(inst);
    switch (memoryOrder)
    {
    case kIRMemoryOrder_Relaxed:
        m_writer->emit("memory_order_relaxed");
        break;
    default:
        m_writer->emit("memory_order_seq_cst");
        break;
    }
}

bool MetalSourceEmitter::tryEmitInstStmtImpl(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_discard:
        m_writer->emit("discard_fragment();\n");
        return true;
    case kIROp_MetalAtomicCast:
    {
        auto oldValName = getName(inst);
        auto op0 = inst->getOperand(0);

        m_writer->emit("atomic_");
        emitType(op0->getDataType());
        m_writer->emit(" ");
        m_writer->emit(oldValName);
        m_writer->emit(" = ");

        m_writer->emit("((atomic_");
        emitType(op0->getDataType());
        m_writer->emit(")(");
        emitOperand(op0, getInfo(EmitOp::General));
        m_writer->emit("));\n");
        return true;
    }
    case kIROp_AtomicLoad:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_load_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(1));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicStore:
    {
        m_writer->emit("atomic_store_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicExchange:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_exchange_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicCompareExchange:
    {
        emitType(inst->getDataType(), getName(inst));
        m_writer->emit(";\n{\n");
        emitType(inst->getDataType(), "_metal_cas_comparand");
        m_writer->emit(" = ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(";\n");

        m_writer->emit(getName(inst));
        m_writer->emit(" = atomic_compare_exchange_weak_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", &_metal_cas_comparand, ");
        emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(3));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(4));
        m_writer->emit(");\n}\n");
        return true;
    }
    case kIROp_AtomicAdd:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_add_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicSub:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_sub_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicAnd:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_and_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicOr:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_or_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicXor:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_xor_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicMin:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_min_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicMax:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_max_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitMemoryOrderOperand(inst->getOperand(2));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicInc:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_add_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", 1, ");
        emitMemoryOrderOperand(inst->getOperand(1));
        m_writer->emit(");\n");
        return true;
    }
    case kIROp_AtomicDec:
    {
        emitInstResultDecl(inst);
        m_writer->emit("atomic_fetch_sub_explicit(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", 1, ");
        emitMemoryOrderOperand(inst->getOperand(1));
        m_writer->emit(");\n");
        return true;
    }
    }
    return false;
}

bool MetalSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeVectorFromScalar:
        {
            if (inst->getOperandCount() == 1)
            {
                EmitOpInfo outerPrec = inOuterPrec;
                bool needClose = false;

                auto prec = getInfo(EmitOp::Prefix);
                needClose = maybeEmitParens(outerPrec, prec);
                emitType(inst->getDataType());
                m_writer->emit("(");
                emitOperand(inst->getOperand(0), rightSide(outerPrec, prec));
                m_writer->emit(") ");

                maybeCloseParens(needClose);
                return true;
            }
            break;
        }
        case kIROp_MatrixReshape:
        {
            ensurePrelude(kMetalBuiltinPreludeMatrixReshape);
            m_writer->emit("_slang_matrixReshape<");
            auto matrixType = as<IRMatrixType>(inst->getDataType());
            emitOperand(matrixType->getRowCount(), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(matrixType->getColumnCount(), getInfo(EmitOp::General));
            m_writer->emit(">(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_VectorReshape:
        {
            ensurePrelude(kMetalBuiltinPreludeVectorReshape);
            m_writer->emit("_slang_vectorReshape<");
            auto vectorType = as<IRVectorType>(inst->getDataType());
            emitOperand(vectorType->getElementCount(), getInfo(EmitOp::General));
            m_writer->emit(">(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_Mul:
        {
            // Component-wise multiplication needs to be special cased,
            // because Metal uses infix `*` to express inner product
            // when working with matrices.

            // Are both operands matrices?
            if (as<IRMatrixType>(inst->getOperand(0)->getDataType())
                && as<IRMatrixType>(inst->getOperand(1)->getDataType()))
            {
                ensurePrelude(kMetalBuiltinPreludeMatrixCompMult);
                m_writer->emit("_slang_matrixCompMult(");
                emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
                m_writer->emit(", ");
                emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
                m_writer->emit(")");
                return true;
            }
            break;
        }
        case kIROp_FRem:
        {
            m_writer->emit("fmod(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_Select:
        {
            m_writer->emit("select(");
            emitOperand(inst->getOperand(2), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(0), leftSide(getInfo(EmitOp::General), getInfo(EmitOp::General)));
            m_writer->emit(")");
            return true;
        }
        case kIROp_BitCast:
        {
            auto toType = inst->getDataType();
            
            m_writer->emit("as_type<");
            emitType(toType);
            m_writer->emit(">(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_StringLit:
        {
            const auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Slang);

            StringBuilder buf;
            const UnownedStringSlice slice = as<IRStringLit>(inst)->getStringSlice();
            StringEscapeUtil::appendQuoted(handler, slice, buf);

            m_writer->emit(buf);

            return true;
        }
        case kIROp_ByteAddressBufferLoad:
        {
            // This only works for loads of 4-byte values.
            // Other element types should have been lowered by previous legalization passes.
            auto elementType = inst->getDataType();
            auto buffer = inst->getOperand(0);
            auto offset = inst->getOperand(1);
            m_writer->emit("as_type<");
            emitType(elementType);
            m_writer->emit(">(");
            emitOperand(buffer, getInfo(EmitOp::General));
            m_writer->emit("[(");
            emitOperand(offset, getInfo(EmitOp::General));
            m_writer->emit(")>>2])");
            return true;
        }
        case kIROp_ByteAddressBufferStore:
        {
            // This only works for loads of 4-byte values.
            // Other element types should have been lowered by previous legalization passes.
            auto buffer = inst->getOperand(0);
            auto offset = inst->getOperand(1);
            emitOperand(buffer, getInfo(EmitOp::General));
            m_writer->emit("[(");
            emitOperand(offset, getInfo(EmitOp::General));
            m_writer->emit(")>>2] = as_type<uint32_t>(");
            emitOperand(inst->getOperand(inst->getOperandCount() - 1), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        break;
        case kIROp_RWStructuredBufferGetElementPtr:
        {
            EmitOpInfo outerPrec = inOuterPrec;
            bool needClose = false;

            auto prec = getInfo(EmitOp::Add);
            needClose = maybeEmitParens(outerPrec, prec);
            emitOperand(inst->getOperand(0), leftSide(outerPrec, prec));
            m_writer->emit("+");
            emitOperand(inst->getOperand(1), rightSide(outerPrec, prec));
            maybeCloseParens(needClose);
            return true;
        }
        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferLoad:
        {
            auto prec = getInfo(EmitOp::Postfix);
            emitOperand(inst->getOperand(0), leftSide(inOuterPrec, prec));
            m_writer->emit("[");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit("]");
            return true;
        }
        case kIROp_RWStructuredBufferStore:
        {
            auto prec = getInfo(EmitOp::Postfix);
            emitOperand(inst->getOperand(0), leftSide(inOuterPrec, prec));
            m_writer->emit("[");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit("] = ");
            emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
            return true;
        }
        case kIROp_ImageLoad:
        {
            auto imageOp = as<IRImageLoad>(inst);
            emitOperand(imageOp->getImage(), getInfo(EmitOp::General));
            m_writer->emit(".read(");
            emitOperand(imageOp->getCoord(), getInfo(EmitOp::General));
            if(imageOp->hasAuxCoord1())
            {
                m_writer->emit(",");
                emitOperand(imageOp->getAuxCoord1(), getInfo(EmitOp::General));
            }
            if(imageOp->hasAuxCoord2())
            {
                m_writer->emit(",");
                emitOperand(imageOp->getAuxCoord2(), getInfo(EmitOp::General));
            }
            m_writer->emit(")");
            return true;
        }
        case kIROp_ImageStore:
        {
            
            auto imageOp = as<IRImageStore>(inst);
            emitOperand(imageOp->getImage(), getInfo(EmitOp::General));
            m_writer->emit(".write(");
            emitOperand(imageOp->getValue(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitOperand(imageOp->getCoord(), getInfo(EmitOp::General));
            if(imageOp->hasAuxCoord1())
            {
                m_writer->emit(",");
                emitOperand(imageOp->getAuxCoord1(), getInfo(EmitOp::General));
            }
            if(imageOp->hasAuxCoord2())
            {
                m_writer->emit(",");
                emitOperand(imageOp->getAuxCoord2(), getInfo(EmitOp::General));
            }
            m_writer->emit(")");
            return true;
        }
        case kIROp_MetalSetVertex:
        {
            auto setVertex = as<IRMetalSetVertex>(inst);
            m_writer->emit("_slang_mesh.set_vertex(");
            emitOperand(setVertex->getIndex(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitOperand(setVertex->getElementValue(), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_MetalSetPrimitive:
        {
            auto setPrimitive = as<IRMetalSetPrimitive>(inst);
            m_writer->emit("_slang_mesh.set_primitive(");
            emitOperand(setPrimitive->getIndex(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitOperand(setPrimitive->getElementValue(), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        case kIROp_MetalSetIndices:
        {
            auto setIndices = as<IRMetalSetIndices>(inst);
            const auto indices = as<IRVectorType>(setIndices->getElementValue()->getDataType());
            UInt numIndices = as<IRIntLit>(indices->getElementCount())->getValue();
            for(UInt i = 0; i < numIndices; ++i) {
                m_writer->emit("_slang_mesh.set_index(");
                emitOperand(setIndices->getIndex(), getInfo(EmitOp::General));
                m_writer->emit("*");
                m_writer->emitUInt64(numIndices);
                m_writer->emit("+");
                m_writer->emitUInt64(i);
                m_writer->emit(",(");
                emitOperand(setIndices->getElementValue(), getInfo(EmitOp::General));
                m_writer->emit(")[");
                m_writer->emitUInt64(i);
                m_writer->emit("]);\n");
            }
            return true;
        }
        default: break;
    }
    // Not handled
    return false;
}

void MetalSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    emitSimpleTypeImpl(elementType);

    switch (elementType->getOp())
    {
    case kIROp_FloatType:
    case kIROp_HalfType:
    case kIROp_BoolType:
    case kIROp_Int8Type:
    case kIROp_UInt8Type:
    case kIROp_Int16Type:
    case kIROp_UInt16Type:
    case kIROp_IntType:
    case kIROp_UIntType:
    case kIROp_Int64Type:
    case kIROp_UInt64Type:
        if (elementCount > 1)
        {
            m_writer->emit(elementCount);
        }
        break;
    }
}

void MetalSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    // Metal does not support loop control attributes.
    SLANG_UNUSED(decl);
}

static bool _canEmitExport(const Profile& profile)
{
    const auto family = profile.getFamily();
    const auto version = profile.getVersion();
    // Is ita late enough version of shader model to output with 'export'
    return (family == ProfileFamily::DX && version >= ProfileVersion::DX_6_1);
}

/* virtual */void MetalSourceEmitter::emitFuncDecorationsImpl(IRFunc* func)
{
    // Specially handle export, as we don't want to emit it multiple times
    if (getTargetProgram()->getOptionSet().getBoolOption(CompilerOptionName::GenerateWholeProgram) && 
        _canEmitExport(m_effectiveProfile))
    {
        for (auto decoration : func->getDecorations())
        {
            const auto op = decoration->getOp();
            if (op == kIROp_PublicDecoration ||
                op == kIROp_HLSLExportDecoration)
            {
                m_writer->emit("export\n");
                break;
            }
        }
    }

    // Use the default for others
    Super::emitFuncDecorationsImpl(func);
}

void MetalSourceEmitter::emitIfDecorationsImpl(IRIfElse* ifInst)
{
    // Does not apply to metal.
    SLANG_UNUSED(ifInst);
}

void MetalSourceEmitter::emitSwitchDecorationsImpl(IRSwitch* switchInst)
{
    // Does not apply to metal.
    SLANG_UNUSED(switchInst);
}

void MetalSourceEmitter::emitFuncDecorationImpl(IRDecoration* decoration)
{
    // Does not apply to metal.
    SLANG_UNUSED(decoration);
}

void MetalSourceEmitter::emitSimpleValueImpl(IRInst* inst)
{
    switch (inst->getOp())
    {
        case kIROp_FloatLit:
        {
            IRConstant* constantInst = static_cast<IRConstant*>(inst);
            IRConstant::FloatKind kind = constantInst->getFloatKind();
            switch (kind)
            {
                case IRConstant::FloatKind::Nan:
                {
                    m_writer->emit("(0.0 / 0.0)");
                    return;
                }
                case IRConstant::FloatKind::PositiveInfinity:
                {
                    m_writer->emit("(1.0 / 0.0)");
                    return;
                }
                case IRConstant::FloatKind::NegativeInfinity:
                {
                    m_writer->emit("(-1.0 / 0.0)");
                    return;
                }
                default: break;
            }
            break;
        }

        default: break;
    }

    Super::emitSimpleValueImpl(inst);
}

void MetalSourceEmitter::emitParamTypeImpl(IRType* type, String const& name)
{
    emitType(type, name);
}

void MetalSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_IntType:
        case kIROp_UInt8Type:
        case kIROp_UIntType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        case kIROp_HalfType:
        {
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_Int64Type:
            m_writer->emit("long");
            return;
        case kIROp_UInt64Type:
            m_writer->emit("ulong");
            return;
        case kIROp_Int16Type:
            m_writer->emit("short");
            return;
        case kIROp_UInt16Type:
            m_writer->emit("ushort");
            return;
        case kIROp_IntPtrType:
            m_writer->emit("long");
            return;
        case kIROp_UIntPtrType:
            m_writer->emit("ulong");
            return;
        case kIROp_StructType:
            m_writer->emit(getName(type));
            return;

        case kIROp_VectorType:
        {
            auto vecType = (IRVectorType*)type;
            emitVectorTypeNameImpl(vecType->getElementType(), getIntVal(vecType->getElementCount()));
            return;
        }
        case kIROp_MatrixType:
        {
            auto matType = (IRMatrixType*)type;

            // Similar to GLSL, Metal's column-major is really our row-major.
            m_writer->emit("matrix<");
            emitType(matType->getElementType());
            m_writer->emit(",");
            emitVal(matType->getRowCount(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
            m_writer->emit("> ");           
            return;
        }
        case kIROp_SamplerStateType:
        case kIROp_SamplerComparisonStateType:
        {
            m_writer->emit("sampler");
            return;
        }
        case kIROp_NativeStringType:
        case kIROp_StringType: 
        {
            m_writer->emit("int"); 
            return;
        }
        case kIROp_ParameterBlockType:
        case kIROp_ConstantBufferType:
        {
            emitSimpleTypeImpl((IRType*)type->getOperand(0));
            m_writer->emit(" constant*");
            return;
        }
        case kIROp_PtrType:
        case kIROp_InOutType:
        case kIROp_OutType:
        case kIROp_RefType:
        case kIROp_ConstRefType:
        {
            auto ptrType = cast<IRPtrTypeBase>(type);
            if(type->getOp() == kIROp_ConstRefType)
            {
                m_writer->emit("const ");
            }
            emitType((IRType*)ptrType->getValueType());
            switch (ptrType->getAddressSpace())
            {
            case AddressSpace::Global:
                m_writer->emit(" device");
                m_writer->emit("*");
                break;
            case AddressSpace::Uniform:
                m_writer->emit(" constant");
                m_writer->emit("*");
                break;
            case AddressSpace::ThreadLocal:
                m_writer->emit(" thread");
                m_writer->emit("*");
                break;
            case AddressSpace::GroupShared:
                m_writer->emit(" threadgroup");
                m_writer->emit("*");
                break;
            case AddressSpace::MetalObjectData:
                m_writer->emit(" object_data");
                m_writer->emit("*");
                break;
            }
            return;
        }
        case kIROp_ArrayType:
        {
            m_writer->emit("array<");
            emitType((IRType*)type->getOperand(0));
            m_writer->emit(", ");
            emitVal(type->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(">");
            return;
        }
        case kIROp_MetalMeshGridPropertiesType:
        {
            m_writer->emit("mesh_grid_properties ");
            return;
        }
        case kIROp_AtomicType:
        {
            m_writer->emit("atomic<");
            emitSimpleTypeImpl(cast<IRAtomicType>(type)->getElementType());
            m_writer->emit(">");
            return;
        }
        default:
            break;
    }

    if (auto texType = as<IRTextureType>(type))
    {
        _emitHLSLTextureType(texType);
        return;
    }
    else if (as<IRTextureBufferType>(type))
    {
        m_writer->emit("texture_buffer<");
        emitVal(type->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(">");
        return;
    }
    else if (auto imageType = as<IRGLSLImageType>(type))
    {
        _emitHLSLTextureType(imageType);
        return;
    }
    else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
    {
        emitType(structuredBufferType->getElementType());
        m_writer->emit(" device*");
        return;
    }
    else if (const auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        switch (type->getOp())
        {
            case kIROp_HLSLByteAddressBufferType:
            case kIROp_HLSLRWByteAddressBufferType:
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
                m_writer->emit("uint32_t device*");
                break;
            case kIROp_RaytracingAccelerationStructureType:
                m_writer->emit("acceleration_structure<instancing>");
                break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                break;
        }
        return;
    }
    else if (const auto meshType = as<IRMetalMeshType>(type))
    {
        m_writer->emit("metal::mesh<");
        emitType(meshType->getVerticesType());
        m_writer->emit(", ");
        emitType(meshType->getPrimitivesType());
        m_writer->emit(", ");
        emitOperand(meshType->getNumVertices(), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(meshType->getNumPrimitives(), getInfo(EmitOp::General));
        m_writer->emit(", metal::topology::");
        switch(meshType->getTopology()->getValue()) {
        case 1:
            m_writer->emit("point");
            break;
        case 2:
            m_writer->emit("line");
            break;
        case 3:
            m_writer->emit("triangle");
            break;
        }
        m_writer->emit(">");
        return;
    }
    else if(auto specializedType = as<IRSpecialize>(type))
    {
        // If a `specialize` instruction made it this far, then
        // it represents an intrinsic generic type.
        //
        emitSimpleType((IRType*) getSpecializedValue(specializedType));
        m_writer->emit("<");
        UInt argCount = specializedType->getArgCount();
        for (UInt ii = 0; ii < argCount; ++ii)
        {
            if (ii != 0) m_writer->emit(", ");
            emitVal(specializedType->getArg(ii), getInfo(EmitOp::General));
        }
        m_writer->emit(" >");
        return;
    }

    // HACK: As a fallback for HLSL targets, assume that the name of the
    // instruction being used is the same as the name of the HLSL type.
    {
        auto opInfo = getIROpInfo(type->getOp());
        m_writer->emit(opInfo.name);
        UInt operandCount = type->getOperandCount();
        if (operandCount)
        {
            m_writer->emit("<");
            for (UInt ii = 0; ii < operandCount; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                emitVal(type->getOperand(ii), getInfo(EmitOp::General));
            }
            m_writer->emit(" >");
        }
    }
}

void MetalSourceEmitter::_emitType(IRType* type, DeclaratorInfo* declarator)
{
    switch (type->getOp())
    {
    case kIROp_ArrayType:
        emitSimpleType(type);
        emitDeclarator(declarator);
        break;
    default:
        Super::_emitType(type, declarator);
        break;
    }
}

bool MetalSourceEmitter::maybeEmitSystemSemantic(IRInst* inst)
{
    if (auto sysSemanticDecor = inst->findDecoration<IRTargetSystemValueDecoration>())
    {
        m_writer->emit(" [[");
        m_writer->emit(sysSemanticDecor->getSemantic());
        m_writer->emit("]]");
        return true;
    }
    return false;
}

bool MetalSourceEmitter::_emitUserSemantic(UnownedStringSlice semanticName, IRIntegerValue semanticIndex)
{
    if (!semanticName.startsWithCaseInsensitive(toSlice("SV_")))
    {
        m_writer->emit(" [[user(");
        m_writer->emit(String(semanticName).toUpper());
        if (semanticIndex != 0)
        {
            m_writer->emit("_");
            m_writer->emit(semanticIndex);
        }
        m_writer->emit(")]]");
        return true;
    }
    return false;
}

void MetalSourceEmitter::emitSemanticsImpl(IRInst* inst, bool allowOffsets)
{
    SLANG_UNUSED(allowOffsets);
    if (inst->getOp() == kIROp_StructKey)
    {
        // Only emit [[attribute(n)]] on struct keys.
        
        if (maybeEmitSystemSemantic(inst))
            return;

        auto varLayout = findVarLayout(inst);
        bool hasSemantic = false;

        if (varLayout)
        {
            for (auto attr : varLayout->getAllAttrs())
            {
                if (auto offsetAttr = as<IRVarOffsetAttr>(attr))
                {
                    if (offsetAttr->getResourceKind() == LayoutResourceKind::MetalAttribute)
                    {
                        m_writer->emit(" [[attribute(");
                        m_writer->emit(offsetAttr->getOffset());
                        m_writer->emit(")]]");
                        return;
                    }
                }
            }
            for (auto attr : varLayout->getAllAttrs())
            {
               if (auto semanticAttr = as<IRSemanticAttr>(attr))
                {
                    auto semanticName = String(semanticAttr->getName()).toUpper();
                    hasSemantic = _emitUserSemantic(semanticAttr->getName(), semanticAttr->getIndex());
                }
            }

        }
        if (!hasSemantic)
        {
            if (auto semanticDecor = inst->findDecoration<IRSemanticDecoration>())
            {
                _emitUserSemantic(semanticDecor->getSemanticName(), semanticDecor->getSemanticIndex());
            }
        }
    }
}

void MetalSourceEmitter::_emitStageAccessSemantic(IRStageAccessDecoration* decoration, const char* name)
{
    SLANG_UNUSED(decoration);
    SLANG_UNUSED(name);
}

void MetalSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    Super::emitSimpleFuncParamImpl(param);
    emitFuncParamLayoutImpl(param);
}

void MetalSourceEmitter::emitPostDeclarationAttributesForType(IRInst* type)
{
    Super::emitPostDeclarationAttributesForType(type);
    if (auto textureType = as<IRTextureTypeBase>(type))
    {
        if (textureType->getAccess() == SLANG_RESOURCE_ACCESS_RASTER_ORDERED)
        {
            m_writer->emit(" [[raster_order_group(0)]]");
        }
    }
    else if (as<IRHLSLRasterizerOrderedByteAddressBufferType>(type) ||
        as<IRHLSLRasterizerOrderedStructuredBufferType>(type))
    {
        m_writer->emit(" [[raster_order_group(0)]]");
    }
}

static UnownedStringSlice _getInterpolationModifierText(IRInterpolationMode mode)
{
    switch (mode)
    {
        case IRInterpolationMode::PerVertex:
        case IRInterpolationMode::NoInterpolation:      return UnownedStringSlice::fromLiteral("[[flat]]");
        case IRInterpolationMode::NoPerspective:        return UnownedStringSlice::fromLiteral("[[center_no_perspective]]");
        case IRInterpolationMode::Linear:               return UnownedStringSlice::fromLiteral("[[sample_no_perspective]]");
        case IRInterpolationMode::Sample:               return UnownedStringSlice::fromLiteral("[[sample_perspective]]");
        case IRInterpolationMode::Centroid:             return UnownedStringSlice::fromLiteral("[[center_perspective]]");
        default:                                        return UnownedStringSlice();
    }
}

void MetalSourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout)
{
    SLANG_UNUSED(layout);
    SLANG_UNUSED(valueType);

    for (auto dd : varInst->getDecorations())
    {
        if (dd->getOp() != kIROp_InterpolationModeDecoration)
            continue;

        auto decoration = (IRInterpolationModeDecoration*)dd;
  
        UnownedStringSlice modeText = _getInterpolationModifierText(decoration->getMode());
        if (modeText.getLength() > 0)
        {
            m_writer->emit(modeText);
            m_writer->emitChar(' ');
        }
    }
}

void MetalSourceEmitter::emitPackOffsetModifier(IRInst* varInst, IRType* valueType, IRPackOffsetDecoration* layout)
{
    SLANG_UNUSED(varInst);
    SLANG_UNUSED(valueType);
    SLANG_UNUSED(layout);
    // We emit packoffset as a semantic in `emitSemantic`, so nothing to do here.
}

void MetalSourceEmitter::emitRateQualifiersAndAddressSpaceImpl(IRRate* rate, AddressSpace addressSpace)
{
    if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("threadgroup ");
        return;
    }

    switch (addressSpace)
    {
    case AddressSpace::GroupShared:
        m_writer->emit("threadgroup ");
        break;
    case AddressSpace::Uniform:
        m_writer->emit("constant ");
        break;
    case AddressSpace::Global:
        m_writer->emit("device ");
        break;
    case AddressSpace::ThreadLocal:
        m_writer->emit("thread ");
        break;
    case AddressSpace::MetalObjectData:
        m_writer->emit("object_data ");
        break;
    default:
        break;
    }
}


void MetalSourceEmitter::emitMeshShaderModifiersImpl(IRInst* varInst)
{
    SLANG_UNUSED(varInst);
}

void MetalSourceEmitter::emitVarDecorationsImpl(IRInst* varInst)
{
    SLANG_UNUSED(varInst);
}

void MetalSourceEmitter::emitMatrixLayoutModifiersImpl(IRType*)
{
    // Metal only supports column major layout, and we must have
    // already translated all matrix ops to assume column-major
    // at this stage.
}

void MetalSourceEmitter::handleRequiredCapabilitiesImpl(IRInst* inst)
{
    SLANG_UNUSED(inst);
}

void MetalSourceEmitter::emitFrontMatterImpl(TargetRequest*)
{
    m_writer->emit("#include <metal_stdlib>\n");
    m_writer->emit("#include <metal_math>\n");
    m_writer->emit("#include <metal_texture>\n");
    m_writer->emit("using namespace metal;\n");
}

void MetalSourceEmitter::emitGlobalInstImpl(IRInst* inst)
{
    Super::emitGlobalInstImpl(inst);
}

} // namespace Slang
