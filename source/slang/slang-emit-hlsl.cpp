// slang-emit-hlsl.cpp
#include "slang-emit-hlsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

void HLSLSourceEmitter::_emitHLSLDecorationSingleString(const char* name, IRFunc* entryPoint, IRStringLit* val)
{
    SLANG_UNUSED(entryPoint);
    assert(val);

    m_writer->emit("[");
    m_writer->emit(name);
    m_writer->emit("(\"");
    m_writer->emit(val->getStringSlice());
    m_writer->emit("\")]\n");
}

void HLSLSourceEmitter::_emitHLSLDecorationSingleInt(const char* name, IRFunc* entryPoint, IRIntLit* val)
{
    SLANG_UNUSED(entryPoint);
    SLANG_ASSERT(val);

    auto intVal = getIntVal(val);

    m_writer->emit("[");
    m_writer->emit(name);
    m_writer->emit("(");
    m_writer->emit(intVal);
    m_writer->emit(")]\n");
}

void HLSLSourceEmitter::_emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling)
{
    if (!chain)
        return;
    if (!chain->varLayout->usesResourceKind(kind))
        return;

    UInt index = getBindingOffset(chain, kind);
    UInt space = getBindingSpace(chain, kind);

    switch (kind)
    {
        case LayoutResourceKind::Uniform:
        {
            UInt offset = index;

            // The HLSL `c` register space is logically grouped in 16-byte registers,
            // while we try to traffic in byte offsets. That means we need to pick
            // a register number, based on the starting offset in 16-byte register
            // units, and then a "component" within that register, based on 4-byte
            // offsets from there. We cannot support more fine-grained offsets than that.

            m_writer->emit(" : ");
            m_writer->emit(uniformSemanticSpelling);
            m_writer->emit("(c");

            // Size of a logical `c` register in bytes
            auto registerSize = 16;

            // Size of each component of a logical `c` register, in bytes
            auto componentSize = 4;

            size_t startRegister = offset / registerSize;
            m_writer->emit(int(startRegister));

            size_t byteOffsetInRegister = offset % registerSize;

            // If this field doesn't start on an even register boundary,
            // then we need to emit additional information to pick the
            // right component to start from
            if (byteOffsetInRegister != 0)
            {
                // The value had better occupy a whole number of components.
                SLANG_RELEASE_ASSERT(byteOffsetInRegister % componentSize == 0);

                size_t startComponent = byteOffsetInRegister / componentSize;

                static const char* kComponentNames[] = { "x", "y", "z", "w" };
                m_writer->emit(".");
                m_writer->emit(kComponentNames[startComponent]);
            }
            m_writer->emit(")");
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
            m_writer->emit(" : register(");
            switch (kind)
            {
                case LayoutResourceKind::ConstantBuffer:
                    m_writer->emit("b");
                    break;
                case LayoutResourceKind::ShaderResource:
                    m_writer->emit("t");
                    break;
                case LayoutResourceKind::UnorderedAccess:
                    m_writer->emit("u");
                    break;
                case LayoutResourceKind::SamplerState:
                    m_writer->emit("s");
                    break;
                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled HLSL register type");
                    break;
            }
            m_writer->emit(index);
            if (space)
            {
                m_writer->emit(", space");
                m_writer->emit(space);
            }
            m_writer->emit(")");
        }
    }
}

void HLSLSourceEmitter::_emitHLSLRegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling)
{
    if (!chain) return;

    auto layout = chain->varLayout;

    switch (getSourceLanguage())
    {
        default:
            return;

        case SourceLanguage::HLSL:
            break;
    }

    for (auto rr : layout->getOffsetAttrs())
    {
        _emitHLSLRegisterSemantic(rr->getResourceKind(), chain, uniformSemanticSpelling);
    }
}

void HLSLSourceEmitter::_emitHLSLRegisterSemantics(IRVarLayout* varLayout, char const* uniformSemanticSpelling)
{
    if (!varLayout)
        return;

    EmitVarChain chain(varLayout);
    _emitHLSLRegisterSemantics(&chain, uniformSemanticSpelling);
}

void HLSLSourceEmitter::_emitHLSLParameterGroupFieldLayoutSemantics(EmitVarChain* chain)
{
    if (!chain)
        return;

    auto layout = chain->varLayout;
    for (auto rr : layout->getOffsetAttrs())
    {
        _emitHLSLRegisterSemantic(rr->getResourceKind(), chain, "packoffset");
    }
}

void HLSLSourceEmitter::_emitHLSLParameterGroupFieldLayoutSemantics(IRVarLayout* fieldLayout, EmitVarChain* inChain)
{
    EmitVarChain chain(fieldLayout, inChain);
    _emitHLSLParameterGroupFieldLayoutSemantics(&chain);
}

void HLSLSourceEmitter::_emitHLSLParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    if (as<IRTextureBufferType>(type))
    {
        m_writer->emit("tbuffer ");
    }
    else
    {
        m_writer->emit("cbuffer ");
    }
    m_writer->emit(getName(varDecl));

    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    EmitVarChain blockChain(varLayout);

    EmitVarChain containerChain = blockChain;
    EmitVarChain elementChain = blockChain;

    auto typeLayout = varLayout->getTypeLayout();
    if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->getContainerVarLayout(), &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->getElementVarLayout(), &blockChain);

        typeLayout = parameterGroupTypeLayout->getElementVarLayout()->getTypeLayout();
    }

    _emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

    auto elementType = type->getElementType();
    if (hasExplicitConstantBufferOffset(type))
    {
        // If the user has provided any explicit `packoffset` modifiers,
        // we have to unwrap the struct and emit the fields directly.
        emitStructDeclarationsBlock(as<IRStructType>(elementType), true);
        m_writer->emit("\n");
        return;
    }
    
    
    m_writer->emit("\n{\n");
    m_writer->indent();


    emitType(elementType, getName(varDecl));
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("}\n");
}

void HLSLSourceEmitter::_emitHLSLTextureType(IRTextureTypeBase* texType)
{
    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ:
            break;

        case SLANG_RESOURCE_ACCESS_READ_WRITE:
            m_writer->emit("RW");
            break;

        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            m_writer->emit("RasterizerOrdered");
            break;

        case SLANG_RESOURCE_ACCESS_APPEND:
            m_writer->emit("Append");
            break;

        case SLANG_RESOURCE_ACCESS_CONSUME:
            m_writer->emit("Consume");
            break;

        case SLANG_RESOURCE_ACCESS_FEEDBACK:
            m_writer->emit("Feedback");
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            break;
    }

    switch (texType->GetBaseShape())
    {
        case SLANG_TEXTURE_1D:		m_writer->emit("Texture1D");		break;
        case SLANG_TEXTURE_2D:		m_writer->emit("Texture2D");		break;
        case SLANG_TEXTURE_3D:		m_writer->emit("Texture3D");		break;
        case SLANG_TEXTURE_CUBE:	m_writer->emit("TextureCube");	    break;
        case SLANG_TEXTURE_BUFFER:  m_writer->emit("Buffer");           break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            break;
    }

    if (texType->isMultisample())
    {
        m_writer->emit("MS");
    }
    if (texType->isArray())
    {
        m_writer->emit("Array");
    }
    m_writer->emit("<");
    emitType(texType->getElementType());
    auto sampleCount = as<IRIntLit>(texType->getSampleCount());
    if (sampleCount->getValue() != 0)
    {
        m_writer->emit(", ");
        m_writer->emit(sampleCount->getValue());
    }
    m_writer->emit(" >");
}

void HLSLSourceEmitter::emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling)
{
    auto layout = getVarLayout(inst); 
    if (layout)
    {
        _emitHLSLRegisterSemantics(layout, uniformSemanticSpelling);
    }
}

void HLSLSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    _emitHLSLParameterGroup(varDecl, type);
}

void HLSLSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    auto profile = m_effectiveProfile;
    auto stage = entryPointDecor->getProfile().getStage();

    if (profile.getFamily() == ProfileFamily::DX)
    {
        if (profile.getVersion() >= ProfileVersion::DX_6_1)
        {
            char const* stageName = getStageName(stage);
            if (stageName)
            {
                m_writer->emit("[shader(\"");
                m_writer->emit(stageName);
                m_writer->emit("\")]");
            }
        }
    }

    auto emitNumThreadsAttribute = [&]()
        {
            Int sizeAlongAxis[kThreadGroupAxisCount];
            getComputeThreadGroupSize(irFunc, sizeAlongAxis);

            m_writer->emit("[numthreads(");
            for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit(sizeAlongAxis[ii]);
            }
            m_writer->emit(")]\n");
        };

    switch (stage)
    {
        case Stage::Compute:
        {
            emitNumThreadsAttribute();
        }
        break;
        case Stage::Geometry:
        {
            if (auto decor = irFunc->findDecoration<IRMaxVertexCountDecoration>())
            {
                auto count = getIntVal(decor->getCount());
                m_writer->emit("[maxvertexcount(");
                m_writer->emit(Int(count));
                m_writer->emit(")]\n");
            }

            if (auto decor = irFunc->findDecoration<IRInstanceDecoration>())
            {
                auto count = getIntVal(decor->getCount());
                m_writer->emit("[instance(");
                m_writer->emit(Int(count));
                m_writer->emit(")]\n");
            }
            break;
        }
        case Stage::Domain:
        {
            /* [domain("isoline")] */
            if (auto decor = irFunc->findDecoration<IRDomainDecoration>())
            {
                _emitHLSLDecorationSingleString("domain", irFunc, decor->getDomain());
            }
            break;
        }
        case Stage::Hull:
        {
            // Lists these are only attributes for hull shader
            // https://docs.microsoft.com/en-us/windows/desktop/direct3d11/direct3d-11-advanced-stages-hull-shader-design

            /* [domain("isoline")] */
            if (auto decor = irFunc->findDecoration<IRDomainDecoration>())
            {
                _emitHLSLDecorationSingleString("domain", irFunc, decor->getDomain());
            }

            /* [domain("partitioning")] */
            if (auto decor = irFunc->findDecoration<IRPartitioningDecoration>())
            {
                _emitHLSLDecorationSingleString("partitioning", irFunc, decor->getPartitioning());
            }

            /* [outputtopology("line")] */
            if (auto decor = irFunc->findDecoration<IROutputTopologyDecoration>())
            {
                _emitHLSLDecorationSingleString("outputtopology", irFunc, decor->getTopology());
            }

            /* [outputcontrolpoints(4)] */
            if (auto decor = irFunc->findDecoration<IROutputControlPointsDecoration>())
            {
                _emitHLSLDecorationSingleInt("outputcontrolpoints", irFunc, decor->getControlPointCount());
            }

            /* [patchconstantfunc("HSConst")] */
            if (auto decor = irFunc->findDecoration<IRPatchConstantFuncDecoration>())
            {
                const String irName = getName(decor->getFunc());

                m_writer->emit("[patchconstantfunc(\"");
                m_writer->emit(irName);
                m_writer->emit("\")]\n");
            }

            break;
        }
        case Stage::Pixel:
        {
            if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
            {
                m_writer->emit("[earlydepthstencil]\n");
            }
            break;
        }
        case Stage::Mesh:
        {
            emitNumThreadsAttribute();
            if (auto decor = irFunc->findDecoration<IROutputTopologyDecoration>())
            {
                // TODO: Ellie validate here/elsewhere, what's allowed here is
                // different from the tesselator
                // The naming here is plural, so add an 's'
                _emitHLSLDecorationSingleString("outputtopology", irFunc, decor->getTopology());
            }
            break;
        }
        case Stage::Amplification:
        {
            emitNumThreadsAttribute();
            break;
        }
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
    }
}

bool HLSLSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
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
                m_writer->emit("(");
                emitType(inst->getDataType());
                m_writer->emit(") ");
                emitOperand(inst->getOperand(0), rightSide(outerPrec, prec));

                maybeCloseParens(needClose);
                // Handled
                return true;
            }
            break;
        }
        case kIROp_BitCast:
        {
            // For simplicity, we will handle all bit-cast operations
            // by first casting the "from" type to an intermediate
            // integer type to hold the bits, and then convert *the*
            // type over to the desired "to" type.
            //
            // A fundamental invariant that must be guaranteed
            // by earlier steps is that a bit-cast instruction
            // is only generated when the "from" and "to" types
            // have the same size, and (in the case where they
            // are vectors) number of elements.
            //
            // In textual order, the conversion to the "to" type
            // comes first.
            //
            auto toType = extractBaseType(inst->getDataType());
            switch (toType)
            {
                default:
                    diagnoseUnhandledInst(inst);
                    break;

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
                case BaseType::Bool:
                    // Because the intermediate type will always
                    // be an integer type, we can convert to
                    // another integer type of the same size
                    // via a cast.
                    m_writer->emit("(");
                    emitType(inst->getDataType());
                    m_writer->emit(")");
                    break;
                case BaseType::Half:
                    m_writer->emit("asfloat16");
                    break;
                case BaseType::Float:
                    // Note: at present HLSL only supports
                    // reinterpreting integer bits as a `float`.
                    //
                    // There is no current function (it seems)
                    // for bit-casting an `int16_t` to a `half`.
                    //
                    // TODO: There is an `asdouble` function
                    // for converting two 32-bit integer values into
                    // one `double`. We could use that for
                    // bit casts of 64-bit values with a bit of
                    // extra work, but doing so might be best
                    // handled in an IR pass that legalizes
                    // bit-casts.
                    //
                    m_writer->emit("asfloat");
                    break;
            }
            m_writer->emit("(");
            int closeCount = 1;

            auto fromType = extractBaseType(inst->getOperand(0)->getDataType());
            switch( fromType )
            {
                default:
                    diagnoseUnhandledInst(inst);
                    break;

                case BaseType::UInt:
                case BaseType::Int:
                case BaseType::Bool:
                    break;
                case BaseType::UInt16:
                case BaseType::Int16:
                    break;
                case BaseType::Float:
                    m_writer->emit("asuint(");
                    closeCount++;
                    break;

                case BaseType::Half:
                    m_writer->emit("asuint16(");
                    closeCount++;
                    break;
            }

            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));

            while(closeCount--)
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
            // HLSL byte-address buffers have two kinds of `Load` operations.
            //
            // First we have the `Load`, `Load2`, `Load3`, and `Load4` operations,
            // which are *not* generic/templated, and always return a scalar
            // or vector of `uint`. These are available on all profiles that
            // support byte-address buffers.
            //
            // Second we have the `Load<T>` generic, which itself comes in
            // two flavors. The basic version can only handle the case where `T`
            // is a scalar or vector, but can handle more types than the
            // non-generic operations. The more complex version can handle
            // aggregate tyeps as well, but we don't need to worry about
            // that because we will have legalized such operations out
            // already.
            //
            // Our task here is thus to pick between `Load`/`Load2`/`Load3`/`Load4`
            // or `Load<T>`, always preferring the functions that are more
            // universally available.
            //
            // We will thus inspect the type that is being loaded,
            // and determine if it is a scalar or vector, and then
            // if the elemnet type of that scalar/vector is `uint`.
            //
            auto elementType = inst->getDataType();
            IRIntegerValue elementCount = 1;
            if( auto vecType = as<IRVectorType>(elementType) )
            {
                if( auto elementCountInst = as<IRIntLit>(vecType->getElementCount()) )
                {
                    elementType = vecType->getElementType();
                    elementCount = elementCountInst->getValue();
                }
            }

            if( elementType->getOp() == kIROp_UIntType )
            {
                // If we are in the case that can use `Load`/`Load2`/`Load3`/`Load4`,
                // then we will always prefer to use it.
                //
                auto outerPrec = inOuterPrec;
                auto prec = getInfo(EmitOp::Postfix);
                bool needClose = maybeEmitParens(outerPrec, prec);

                emitOperand(inst->getOperand(0), leftSide(outerPrec, prec));
                m_writer->emit(".Load");
                if( elementCount != 1 )
                {
                    m_writer->emit(elementCount);
                }
                m_writer->emit("(");
                emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
                m_writer->emit(")");

                maybeCloseParens(needClose);
                return true;
            }

            // Otherwise we fall back to the base case, which
            // is already handled by the base `CLikeSourceEmitter`
            return false;
        }
        case kIROp_ByteAddressBufferStore:
        {
            // Similar to the case for a load, we want to specialize
            // the generated code for the case where we store a `uint`
            // or a vector of `uint`.
            //
            auto elementType = inst->getDataType();
            IRIntegerValue elementCount = 1;
            if( auto vecType = as<IRVectorType>(elementType) )
            {
                if( auto elementCountInst = as<IRIntLit>(vecType->getElementCount()) )
                {
                    elementType = vecType->getElementType();
                    elementCount = elementCountInst->getValue();
                }
            }
            if( elementType->getOp() == kIROp_UIntType )
            {
                auto outerPrec = inOuterPrec;
                auto prec = getInfo(EmitOp::Postfix);
                bool needClose = maybeEmitParens(outerPrec, prec);

                emitOperand(inst->getOperand(0), leftSide(outerPrec, prec));
                m_writer->emit(".Store");
                if( elementCount != 1 )
                {
                    m_writer->emit(elementCount);
                }
                m_writer->emit("(");
                emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
                m_writer->emit(", ");
                emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
                m_writer->emit(")");

                maybeCloseParens(needClose);
                return true;
            }

            // Otherwise we fall back to the base case, which
            // is already handled by the base `CLikeSourceEmitter`
            return false;
        }
        break;

        default: break;
    }
    // Not handled
    return false;
}

void HLSLSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
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

void HLSLSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
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

/* virtual */void HLSLSourceEmitter::emitFuncDecorationsImpl(IRFunc* func)
{
    // Specially handle export, as we don't want to emit it multiple times
    if (getTargetReq()->isWholeProgramRequest() && 
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

void HLSLSourceEmitter::emitIfDecorationsImpl(IRIfElse* ifInst)
{
    if (ifInst->findDecorationImpl(kIROp_BranchDecoration))
    {
        m_writer->emit("[branch]\n");
    }
    else if (ifInst->findDecorationImpl(kIROp_FlattenDecoration))
    {
        m_writer->emit("[flatten]\n");
    }
}

void HLSLSourceEmitter::emitSwitchDecorationsImpl(IRSwitch* switchInst)
{
    if (switchInst->findDecorationImpl(kIROp_BranchDecoration))
    {
        m_writer->emit("[branch]\n");
    }
}

void HLSLSourceEmitter::emitFuncDecorationImpl(IRDecoration* decoration)
{
    switch( decoration->getOp() )
    {
    case kIROp_NoInlineDecoration:
        m_writer->emit("[noinline]\n");
        break;

    default:
        break;
    }
}


void HLSLSourceEmitter::emitSimpleValueImpl(IRInst* inst)
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

void HLSLSourceEmitter::emitSimpleTypeImpl(IRType* type)
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
#if SLANG_PTR_IS_64
        case kIROp_IntPtrType:
            m_writer->emit("int64_t");
            return;
        case kIROp_UIntPtrType:
            m_writer->emit("uint64_t");
            return;
#else
        case kIROp_IntPtrType:
            m_writer->emit("int");
            return;
        case kIROp_UIntPtrType:
            m_writer->emit("uint");
            return;
#endif
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

            // TODO(tfoley): should really emit these with sugar
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
            auto samplerStateType = cast<IRSamplerStateTypeBase>(type);

            switch (samplerStateType->getOp())
            {
                case kIROp_SamplerStateType:			m_writer->emit("SamplerState");			break;
                case kIROp_SamplerComparisonStateType:	m_writer->emit("SamplerComparisonState");	break;
                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                    break;
            }
            return;
        }
        case kIROp_NativeStringType:
        case kIROp_StringType: 
        {
            m_writer->emit("int"); 
            return;
        }
        case kIROp_RayQueryType:
        {
            m_writer->emit("RayQuery<");
            emitSimpleValue(type->getOperand(0));
            m_writer->emit(" >");
            return;
        }
        case kIROp_HitObjectType:
        {
            m_writer->emit("NvHitObject");
            return;
        }
        default: break;
    }

    // TODO: Ideally the following should be data-driven,
    // based on meta-data attached to the definitions of
    // each of these IR opcodes.
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
        switch (structuredBufferType->getOp())
        {
            case kIROp_HLSLStructuredBufferType:                    m_writer->emit("StructuredBuffer");                   break;
            case kIROp_HLSLRWStructuredBufferType:                  m_writer->emit("RWStructuredBuffer");                 break;
            case kIROp_HLSLRasterizerOrderedStructuredBufferType:   m_writer->emit("RasterizerOrderedStructuredBuffer");  break;
            case kIROp_HLSLAppendStructuredBufferType:              m_writer->emit("AppendStructuredBuffer");             break;
            case kIROp_HLSLConsumeStructuredBufferType:             m_writer->emit("ConsumeStructuredBuffer");            break;

            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled structured buffer type");
                break;
        }

        m_writer->emit("<");
        emitType(structuredBufferType->getElementType());
        m_writer->emit(" >");

        return;
    }
    else if (const auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        switch (type->getOp())
        {
            case kIROp_HLSLByteAddressBufferType:                   m_writer->emit("ByteAddressBuffer");                  break;
            case kIROp_HLSLRWByteAddressBufferType:                 m_writer->emit("RWByteAddressBuffer");                break;
            case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  m_writer->emit("RasterizerOrderedByteAddressBuffer"); break;
            case kIROp_RaytracingAccelerationStructureType:         m_writer->emit("RaytracingAccelerationStructure");    break;

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

void HLSLSourceEmitter::emitRateQualifiersAndAddressSpaceImpl(IRRate* rate, [[maybe_unused]] IRIntegerValue addressSpace)
{
    if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("groupshared ");
    }
}

void HLSLSourceEmitter::emitSemanticsImpl(IRInst* inst, bool allowOffsets)
{
    if (auto semanticDecoration = inst->findDecoration<IRSemanticDecoration>())
    {
        m_writer->emit(" : ");
        m_writer->emit(semanticDecoration->getSemanticName());
        return;
    }
    else if (auto packOffsetDecoration = inst->findDecoration<IRPackOffsetDecoration>())
    {
        if (allowOffsets)
        {
            m_writer->emit(" : packoffset(c");
            m_writer->emit(packOffsetDecoration->getRegisterOffset()->getValue());
            if (packOffsetDecoration->getComponentOffset())
            {
                switch (packOffsetDecoration->getComponentOffset()->getValue())
                {
                case 0:
                    break;
                case 1:
                    m_writer->emit(".y");
                    break;
                case 2:
                    m_writer->emit(".z");
                    break;
                case 3:
                    m_writer->emit(".w");
                    break;
                }
            }
            m_writer->emit(")");
            return;
        }
    }

    if( auto readAccessSemantic = inst->findDecoration<IRStageReadAccessDecoration>())
        _emitStageAccessSemantic(readAccessSemantic, "read");
    if( auto writeAccessSemantic = inst->findDecoration<IRStageWriteAccessDecoration>())
        _emitStageAccessSemantic(writeAccessSemantic, "write");

    if (auto layoutDecoration = inst->findDecoration<IRLayoutDecoration>())
    {
        auto layout = layoutDecoration->getLayout();
        if (auto varLayout = as<IRVarLayout>(layout))
        {
            emitSemanticsUsingVarLayout(varLayout);
        }
        else if (auto entryPointLayout = as<IREntryPointLayout>(layout))
        {
            if (auto resultLayout = entryPointLayout->getResultLayout())
            {
                emitSemanticsUsingVarLayout(resultLayout);
            }
        }
    }
}

void HLSLSourceEmitter::_emitStageAccessSemantic(IRStageAccessDecoration* decoration, const char* name)
{
    Int stageCount = decoration->getStageCount();
    if(stageCount == 0)
        return;

    m_writer->emit(" : ");
    m_writer->emit(name);
    m_writer->emit("(");
    for( Int i = 0; i < stageCount; ++i )
    {
        if(i != 0) m_writer->emit(", ");
        m_writer->emit(decoration->getStageName(i));
    }
    m_writer->emit(")");
}

void HLSLSourceEmitter::emitPostKeywordTypeAttributesImpl(IRInst* inst)
{
    if( const auto payloadDecoration = inst->findDecoration<IRPayloadDecoration>() )
    {
        m_writer->emit("[payload] ");
    }
}

void HLSLSourceEmitter::_emitPrefixTypeAttr(IRAttr* attr)
{
    switch( attr->getOp() )
    {
    default:
        Super::_emitPrefixTypeAttr(attr);
        break;

    case kIROp_UNormAttr: m_writer->emit("unorm "); break;
    case kIROp_SNormAttr: m_writer->emit("snorm "); break;
    }
}

void HLSLSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    // A mesh shader input payload has it's own weird stuff going on, handled
    // in emitMeshShaderModifiers, skip this bit which will introduce an
    // invalid "groupshared" keyword.
    if (!param->findDecoration<IRHLSLMeshPayloadDecoration>())
        emitRateQualifiersAndAddressSpace(param);

    if (auto decor = param->findDecoration<IRGeometryInputPrimitiveTypeDecoration>())
    {
        switch (decor->getOp())
        {
            case kIROp_TriangleInputPrimitiveTypeDecoration:             m_writer->emit("triangle "); break;
            case kIROp_PointInputPrimitiveTypeDecoration:                m_writer->emit("point "); break;
            case kIROp_LineInputPrimitiveTypeDecoration:                 m_writer->emit("line "); break;
            case kIROp_LineAdjInputPrimitiveTypeDecoration:              m_writer->emit("lineadj "); break;
            case kIROp_TriangleAdjInputPrimitiveTypeDecoration:          m_writer->emit("triangleadj "); break;
            default: SLANG_ASSERT(!"Unknown primitive type"); break;
        }
    }

    Super::emitSimpleFuncParamImpl(param);
}

static UnownedStringSlice _getInterpolationModifierText(IRInterpolationMode mode)
{
    switch (mode)
    {
        case IRInterpolationMode::PerVertex:
        case IRInterpolationMode::NoInterpolation:      return UnownedStringSlice::fromLiteral("nointerpolation");
        case IRInterpolationMode::NoPerspective:        return UnownedStringSlice::fromLiteral("noperspective");
        case IRInterpolationMode::Linear:               return UnownedStringSlice::fromLiteral("linear");
        case IRInterpolationMode::Sample:               return UnownedStringSlice::fromLiteral("sample");
        case IRInterpolationMode::Centroid:             return UnownedStringSlice::fromLiteral("centroid");
        default:                                        return UnownedStringSlice();
    }
}

void HLSLSourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout)
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

void HLSLSourceEmitter::emitPackOffsetModifier(IRInst* varInst, IRType* valueType, IRPackOffsetDecoration* layout)
{
    SLANG_UNUSED(varInst);
    SLANG_UNUSED(valueType);
    SLANG_UNUSED(layout);
    // We emit packoffset as a semantic in `emitSemantic`, so nothing to do here.
}

void HLSLSourceEmitter::emitMeshShaderModifiersImpl(IRInst* varInst)
{
    if(auto modifier = varInst->findDecoration<IRMeshOutputDecoration>())
    {
        const char* s =
              as<IRVerticesDecoration>(modifier)   ? "vertices "
            : as<IRIndicesDecoration>(modifier)    ? "indices "
            : as<IRPrimitivesDecoration>(modifier) ? "primitives "
            : nullptr;
        SLANG_ASSERT(s && "Unhandled type of mesh output decoration");
        m_writer->emit(s);
    }
    if(varInst->findDecoration<IRHLSLMeshPayloadDecoration>())
    {
        // DXC requires that mesh payload parameters have "in" specified
        m_writer->emit("in payload ");
    }
}

void HLSLSourceEmitter::emitVarDecorationsImpl(IRInst* varDecl)
{
    if (varDecl->findDecoration<IRGloballyCoherentDecoration>())
    {
        m_writer->emit("globallycoherent\n");
    }
}

void HLSLSourceEmitter::emitMatrixLayoutModifiersImpl(IRVarLayout* layout)
{
    // When a variable has a matrix type, we want to emit an explicit
    // layout qualifier based on what the layout has been computed to be.
    //

    auto typeLayout = layout->getTypeLayout()->unwrapArray();

    if (auto matrixTypeLayout = as<IRMatrixTypeLayout>(typeLayout))
    {
        switch (matrixTypeLayout->getMode())
        {
            case kMatrixLayoutMode_ColumnMajor:
                m_writer->emit("column_major ");
                break;

            case kMatrixLayoutMode_RowMajor:
                m_writer->emit("row_major ");
                break;
        }
    }
}

void HLSLSourceEmitter::handleRequiredCapabilitiesImpl(IRInst* inst)
{
    if(inst->findDecoration<IRRequiresNVAPIDecoration>())
    {
        m_extensionTracker->m_requiresNVAPI = true;
    }
}

void HLSLSourceEmitter::emitFrontMatterImpl(TargetRequest* targetReq) 
{
    if (m_extensionTracker->m_requiresNVAPI)
    {
        // If the generated code includes implicit NVAPI use,
        // then we need to ensure that NVAPI support is included
        // via the prelude.
        //
        m_writer->emit("#define SLANG_HLSL_ENABLE_NVAPI 1\n");

        // TODO(JS): For now when using NVAPI for generated code we do not want to 
        // use HLSL2021 features, that are typically used for Shader Execution Reordering
        // so we turn on the 'macro' based interface by default
        m_writer->emit(toSlice("#define NV_HITOBJECT_USE_MACRO_API 1\n"));

        // In addition, if the user has informed the Slang compiler of
        // the register/space that it wants to use for NVAPI, then we
        // need to pass along that information to prelude in the
        // generated code, so that it can be picked up by the NVAPI
        // header at the point where it gets included.
        //
        // Note: If the user doesn't inform the Slang compiler where
        // it wants the NVAPI parameter to be bound, then a downstream
        // compiler error is going to occur. We could try to produce
        // our own error message here, but our error is unlikely to
        // be significantly better, and also it is *technically*
        // possible for the user to use Slang to generate HLSL,
        // and then go on to compile it manually via fxc/dxc, where
        // they could pass in these `#define`s using command-line
        // or API options.
        //
        if (auto decor = m_irModule->getModuleInst()->findDecoration<IRNVAPISlotDecoration>())
        {
            m_writer->emit("#define NV_SHADER_EXTN_SLOT ");
            m_writer->emit(decor->getRegisterName());
            m_writer->emit("\n");

            // Note: We only emit a preprocessor directive if the space
            // is not `space0`, because we want to ensure that the output
            // code can compile with fxc when possible (and fxc has no
            // understanding of `space`s).
            //
            auto spaceName = decor->getSpaceName();
            if (spaceName != "space0")
            {
                m_writer->emit("#define NV_SHADER_EXTN_REGISTER_SPACE ");
                m_writer->emit(spaceName);
                m_writer->emit("\n");
            }
        }
    }

    // Emit any layout declarations

    switch (targetReq->getDefaultMatrixLayoutMode())
    {
    case kMatrixLayoutMode_RowMajor:
    default:
        m_writer->emit("#pragma pack_matrix(row_major)\n");
        break;
    case kMatrixLayoutMode_ColumnMajor:
        m_writer->emit("#pragma pack_matrix(column_major)\n");
        break;
    }
}

void HLSLSourceEmitter::emitGlobalInstImpl(IRInst* inst)
{
    if( const auto nvapiDecor = inst->findDecoration<IRNVAPIMagicDecoration>() )
    {
        // When emitting one of the "magic" NVAPI declarations,
        // we will wrap it in a preprocessor conditional that
        // skips it if the NVAPI header is already being included
        // via the prelude. In that case, the definitions from
        // the prelude-included NVAPI will be used instead of
        // those that were processed by the Slang front-end.
        //
        // TODO: In theory we could drop the downstream preprocessor
        // conditional here, and either emit or not emit the
        // instruction based on whether the code needs NVAPI (which
        // is when `SLANG_HLSL_ENABLE_NVAPI` would be set).
        // Such a change would require that we replace the current
        // approach of tracking extension use during emit with an
        // approach that detects requirements as a pure pre-pass.
        //
        // Note: We skip `IRStructKey` instructions here because
        // the fields of the `NvShaderExtnStruct` are also decorated,
        // but field keys don't produce anything in the output, so
        // we'd have conditionals that are wrapping empty lines.
        //
        if( !as<IRStructKey>(inst) )
        {
            m_writer->emit("#ifndef SLANG_HLSL_ENABLE_NVAPI\n");
            Super::emitGlobalInstImpl(inst);
            m_writer->emit("#endif\n");
            return;
        }
    }

    Super::emitGlobalInstImpl(inst);
}



} // namespace Slang
