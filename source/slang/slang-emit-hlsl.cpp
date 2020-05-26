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

    auto intVal = GetIntVal(val);

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

    switch (getSourceStyle())
    {
        default:
            return;

        case SourceStyle::HLSL:
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

    m_writer->emit("\n{\n");
    m_writer->indent();

    auto elementType = type->getElementType();

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

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            break;
    }

    switch (texType->GetBaseShape())
    {
        case TextureFlavor::Shape::Shape1D:		m_writer->emit("Texture1D");		break;
        case TextureFlavor::Shape::Shape2D:		m_writer->emit("Texture2D");		break;
        case TextureFlavor::Shape::Shape3D:		m_writer->emit("Texture3D");		break;
        case TextureFlavor::Shape::ShapeCube:	m_writer->emit("TextureCube");	break;
        case TextureFlavor::Shape::ShapeBuffer:  m_writer->emit("Buffer");         break;
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
    auto stage = entryPointDecor->getProfile().GetStage();

    if (profile.getFamily() == ProfileFamily::DX)
    {
        if (profile.GetVersion() >= ProfileVersion::DX_6_1)
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

    switch (stage)
    {
        case Stage::Compute:
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
        }
        break;
        case Stage::Geometry:
        {
            if (auto decor = irFunc->findDecoration<IRMaxVertexCountDecoration>())
            {
                auto count = GetIntVal(decor->getCount());
                m_writer->emit("[maxvertexcount(");
                m_writer->emit(Int(count));
                m_writer->emit(")]\n");
            }

            if (auto decor = irFunc->findDecoration<IRInstanceDecoration>())
            {
                auto count = GetIntVal(decor->getCount());
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
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
    }
}

bool HLSLSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->op)
    {
        case kIROp_Construct:
        case kIROp_makeVector:
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
                case BaseType::UInt8:
                case BaseType::UInt16:
                case BaseType::UInt:
                case BaseType::UInt64:
                    // Because the intermediate type will always
                    // be an integer type, we can convert to
                    // another integer type of the same size
                    // via a cast.
                    m_writer->emit("(");
                    emitType(inst->getDataType());
                    m_writer->emit(")");
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
                    break;

                case BaseType::Float:
                    m_writer->emit("asuint(");
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
            IRStringLit* lit = cast<IRStringLit>(inst);
            UnownedStringSlice slice = lit->getStringSlice();
            m_writer->emit(int32_t(getStableHashCode32(slice.begin(), slice.getLength())));
            return true;
        }
        case kIROp_GetStringHash:
        {
            // On GLSL target, the `String` type is just an `int`
            // that is the hash of the string, so we can emit
            // the first operand to `getStringHash` directly.
            //
            EmitOpInfo outerPrec = inOuterPrec;
            emitOperand(inst->getOperand(0), outerPrec);
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

            if( elementType->op == kIROp_UIntType )
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
            if( elementType->op == kIROp_UIntType )
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

void HLSLSourceEmitter::emitLayoutDirectivesImpl(TargetRequest* targetReq)
{
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

void HLSLSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    // TODO(tfoley) : should really emit these with sugar
    m_writer->emit("vector<");
    emitType(elementType);
    m_writer->emit(",");
    m_writer->emit(elementCount);
    m_writer->emit(">");
}

void HLSLSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    if (decl->getMode() == kIRLoopControl_Unroll)
    {
        m_writer->emit("[unroll]\n");
    }
}

void HLSLSourceEmitter::emitSimpleValueImpl(IRInst* inst)
{
    switch (inst->op)
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
    switch (type->op)
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        case kIROp_HalfType:
        {
            m_writer->emit(getDefaultBuiltinTypeName(type->op));
            return;
        }
        case kIROp_StructType:
            m_writer->emit(getName(type));
            return;

        case kIROp_VectorType:
        {
            auto vecType = (IRVectorType*)type;
            emitVectorTypeNameImpl(vecType->getElementType(), GetIntVal(vecType->getElementCount()));
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

            switch (samplerStateType->op)
            {
                case kIROp_SamplerStateType:			m_writer->emit("SamplerState");			break;
                case kIROp_SamplerComparisonStateType:	m_writer->emit("SamplerComparisonState");	break;
                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                    break;
            }
            return;
        }
        case kIROp_StringType: m_writer->emit("int"); return;
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
    else if (auto textureSamplerType = as<IRTextureSamplerType>(type))
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see combined texture-sampler types");
        return;
    }
    else if (auto imageType = as<IRGLSLImageType>(type))
    {
        _emitHLSLTextureType(imageType);
        return;
    }
    else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
    {
        switch (structuredBufferType->op)
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
    else if (auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
    {
        switch (type->op)
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
        auto opInfo = getIROpInfo(type->op);
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

void HLSLSourceEmitter::emitRateQualifiersImpl(IRRate* rate)
{
    if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("groupshared ");
    }
}

void HLSLSourceEmitter::emitSemanticsImpl(IRInst* inst)
{
    if (auto semanticDecoration = inst->findDecoration<IRSemanticDecoration>())
    {
        m_writer->emit(" : ");
        m_writer->emit(semanticDecoration->getSemanticName());
        return;
    }

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

void HLSLSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    if (auto decor = param->findDecoration<IRGeometryInputPrimitiveTypeDecoration>())
    {
        switch (decor->op)
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
        if (dd->op != kIROp_InterpolationModeDecoration)
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


} // namespace Slang
