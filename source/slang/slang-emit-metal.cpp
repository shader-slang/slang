// slang-emit-metal.cpp
#include "slang-emit-metal.h"

#include "../core/slang-writer.h"

#include "slang-ir-util.h"
#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

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

void MetalSourceEmitter::_emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, IRInst* inst, char const* uniformSemanticSpelling)
{
    // Metal does not use explicit binding.
    SLANG_UNUSED(kind);
    SLANG_UNUSED(chain);
    SLANG_UNUSED(inst);
    SLANG_UNUSED(uniformSemanticSpelling);
}

void MetalSourceEmitter::_emitHLSLRegisterSemantics(EmitVarChain* chain, IRInst* inst, char const* uniformSemanticSpelling)
{
    // TODO: implement.
    SLANG_UNUSED(chain);
    SLANG_UNUSED(inst);
    SLANG_UNUSED(uniformSemanticSpelling);
}

void MetalSourceEmitter::_emitHLSLRegisterSemantics(IRVarLayout* varLayout, IRInst* inst, char const* uniformSemanticSpelling)
{
    // TODO: implement.
    SLANG_UNUSED(varLayout);
    SLANG_UNUSED(inst);
    SLANG_UNUSED(uniformSemanticSpelling);
}

void MetalSourceEmitter::_emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain)
{
    // TODO: implement.
    SLANG_UNUSED(chain);
}

void MetalSourceEmitter::_emitHLSLParameterGroupFieldLayoutSemantics(IRVarLayout* fieldLayout, EmitVarChain* inChain)
{
    // TODO: implement.
    SLANG_UNUSED(fieldLayout);
    SLANG_UNUSED(inChain);
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
        case SLANG_TEXTURE_BUFFER:  m_writer->emit("1d");           break;
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

void MetalSourceEmitter::_emitHLSLSubpassInputType(IRSubpassInputType* subpassType)
{
    SLANG_UNUSED(subpassType);
}

void MetalSourceEmitter::emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling)
{
    auto layout = getVarLayout(inst); 
    if (layout)
    {
        _emitHLSLRegisterSemantics(layout, inst, uniformSemanticSpelling);
    }
}

void MetalSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    _emitHLSLParameterGroup(varDecl, type);
}

void MetalSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    auto profile = m_effectiveProfile;
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

bool MetalSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        {
            if (inst->getOperandCount() == 1)
            {
                EmitOpInfo outerPrec = inOuterPrec;
                bool needClose = false;

                auto prec = getInfo(EmitOp::Prefix);
                needClose = maybeEmitParens(outerPrec, prec);

                // Need to emit as cast for HLSL
                emitType(inst->getDataType());
                m_writer->emit("(");
                emitOperand(inst->getOperand(0), rightSide(outerPrec, prec));
                m_writer->emit(") ");

                maybeCloseParens(needClose);
                // Handled
                return true;
            }
            break;
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
            m_writer->emit(")>>2)]");
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
            m_writer->emit(")>>2)] = as_type<uint32_t>(");
            emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
        break;

        default: break;
    }
    // Not handled
    return false;
}

void MetalSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    // In some cases we *need* to use the built-in syntax sugar for vector types,
    // so we will try to emit those whenever possible.
    //
    if( elementCount >= 1 && elementCount <= 4 )
    {
        switch( elementType->getOp() )
        {
        case kIROp_FloatType:
        case kIROp_IntType:
        case kIROp_UIntType:
        // TODO: There are more types that need to be covered here
            emitType(elementType);
            m_writer->emit(elementCount);
            return;

        default:
            break;
        }
    }

    // As a fallback, we will use the `vector<...>` type constructor,
    // although we should not expect to run into types that don't
    // have a sugared form.
    //
    m_writer->emit("vector<");
    emitType(elementType);
    m_writer->emit(",");
    m_writer->emit(elementCount);
    m_writer->emit(">");
}

void MetalSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    switch (decl->getMode())
    {
    case kIRLoopControl_Unroll:
        m_writer->emit("[unroll]\n");
        break;
    case kIRLoopControl_Loop:
        m_writer->emit("[loop]\n");
        break;
    default:
        break;
    }
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

void MetalSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
        case kIROp_UInt8Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
        case kIROp_HalfType:
        {
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_IntPtrType:
            m_writer->emit("int64_t");
            return;
        case kIROp_UIntPtrType:
            m_writer->emit("uint64_t");
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
            emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitVal(matType->getRowCount(), getInfo(EmitOp::General));
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
            m_writer->emit("constant ");
            emitType((IRType*)type->getOperand(0));
            m_writer->emit("*");
            return;
        }
        default: break;
    }

    if (auto texType = as<IRTextureType>(type))
    {
        _emitHLSLTextureType(texType);
        return;
    }
    else if (auto imageType = as<IRGLSLImageType>(type))
    {
        _emitHLSLTextureType(imageType);
        return;
    }
    else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
    {
        m_writer->emit("device ");
        emitType(structuredBufferType->getElementType());
        m_writer->emit("*");
        return;
    }
    else if (const auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        switch (type->getOp())
        {
            case kIROp_HLSLByteAddressBufferType:
            case kIROp_HLSLRWByteAddressBufferType:
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
                m_writer->emit("device ");
                m_writer->emit("uint32_t *");
                break;
            case kIROp_RaytracingAccelerationStructureType:         m_writer->emit("acceleration_structure<instancing>"); break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                break;
        }
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

void MetalSourceEmitter::emitRateQualifiersAndAddressSpaceImpl(IRRate* rate, [[maybe_unused]] IRIntegerValue addressSpace)
{
    if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("threadgroup ");
    }
}

void MetalSourceEmitter::emitSemanticsImpl(IRInst* inst, bool allowOffsets)
{
    // Metal does not use semantics.
    SLANG_UNUSED(inst);
    SLANG_UNUSED(allowOffsets);
}

void MetalSourceEmitter::_emitStageAccessSemantic(IRStageAccessDecoration* decoration, const char* name)
{
    SLANG_UNUSED(decoration);
    SLANG_UNUSED(name);
}

void MetalSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    Super::emitSimpleFuncParamImpl(param);
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

void MetalSourceEmitter::emitMeshShaderModifiersImpl(IRInst* varInst)
{
    SLANG_UNUSED(varInst);
}

void MetalSourceEmitter::emitVarDecorationsImpl(IRInst* varInst)
{
    SLANG_UNUSED(varInst);
}

void MetalSourceEmitter::emitMatrixLayoutModifiersImpl(IRVarLayout*)
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
    
}

void MetalSourceEmitter::emitGlobalInstImpl(IRInst* inst)
{
    Super::emitGlobalInstImpl(inst);
}

} // namespace Slang
