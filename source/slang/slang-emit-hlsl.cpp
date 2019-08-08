// slang-emit-hlsl.cpp
#include "slang-emit-hlsl.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {


void HLSLSourceEmitter::_emitHLSLAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib)
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

    m_writer->emit("[");
    m_writer->emit(name);
    m_writer->emit("(\"");
    m_writer->emit(stringLitExpr->value);
    m_writer->emit("\")]\n");
}

void HLSLSourceEmitter::_emitHLSLAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib)
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

    m_writer->emit("[");
    m_writer->emit(name);
    m_writer->emit("(");
    m_writer->emit(intLitExpr->value);
    m_writer->emit(")]\n");
}

void HLSLSourceEmitter::_emitHLSLRegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling)
{
    if (!chain)
        return;
    if (!chain->varLayout->FindResourceInfo(kind))
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

    for (auto rr : layout->resourceInfos)
    {
        _emitHLSLRegisterSemantic(rr.kind, chain, uniformSemanticSpelling);
    }
}

void HLSLSourceEmitter::_emitHLSLRegisterSemantics(VarLayout* varLayout, char const* uniformSemanticSpelling)
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
    for (auto rr : layout->resourceInfos)
    {
        _emitHLSLRegisterSemantic(rr.kind, chain, "packoffset");
    }
}

void HLSLSourceEmitter::_emitHLSLParameterGroupFieldLayoutSemantics(RefPtr<VarLayout> fieldLayout, EmitVarChain* inChain)
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

    auto typeLayout = varLayout->typeLayout;
    if (auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout))
    {
        containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
        elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

        typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
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

void HLSLSourceEmitter::_emitHLSLEntryPointAttributes(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    auto profile = m_effectiveProfile;
    auto stage = entryPointLayout->profile.GetStage();

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
            static const UInt kAxisCount = 3;
            UInt sizeAlongAxis[kAxisCount];

            // TODO: this is kind of gross because we are using a public
            // reflection API function, rather than some kind of internal
            // utility it forwards to...
            spReflectionEntryPoint_getComputeThreadGroupSize(
                (SlangReflectionEntryPoint*)entryPointLayout,
                kAxisCount,
                &sizeAlongAxis[0]);

            m_writer->emit("[numthreads(");
            for (int ii = 0; ii < 3; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit(sizeAlongAxis[ii]);
            }
            m_writer->emit(")]\n");
        }
        break;
        case Stage::Geometry:
        {
            if (auto attrib = entryPointLayout->getFuncDecl()->FindModifier<MaxVertexCountAttribute>())
            {
                m_writer->emit("[maxvertexcount(");
                m_writer->emit(attrib->value);
                m_writer->emit(")]\n");
            }
            if (auto attrib = entryPointLayout->getFuncDecl()->FindModifier<InstanceAttribute>())
            {
                m_writer->emit("[instance(");
                m_writer->emit(attrib->value);
                m_writer->emit(")]\n");
            }
            break;
        }
        case Stage::Domain:
        {
            FuncDecl* entryPoint = entryPointLayout->entryPoint;
            /* [domain("isoline")] */
            if (auto attrib = entryPoint->FindModifier<DomainAttribute>())
            {
                _emitHLSLAttributeSingleString("domain", entryPoint, attrib);
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
                _emitHLSLAttributeSingleString("domain", entryPoint, attrib);
            }
            /* [domain("partitioning")] */
            if (auto attrib = entryPoint->FindModifier<PartitioningAttribute>())
            {
                _emitHLSLAttributeSingleString("partitioning", entryPoint, attrib);
            }
            /* [outputtopology("line")] */
            if (auto attrib = entryPoint->FindModifier<OutputTopologyAttribute>())
            {
                _emitHLSLAttributeSingleString("outputtopology", entryPoint, attrib);
            }
            /* [outputcontrolpoints(4)] */
            if (auto attrib = entryPoint->FindModifier<OutputControlPointsAttribute>())
            {
                _emitHLSLAttributeSingleInt("outputcontrolpoints", entryPoint, attrib);
            }
            /* [patchconstantfunc("HSConst")] */
            if (auto attrib = entryPoint->FindModifier<PatchConstantFuncAttribute>())
            {
                _emitHLSLFuncDeclPatchConstantFuncAttribute(irFunc, entryPoint, attrib);
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

void HLSLSourceEmitter::_emitHLSLFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib)
{
    SLANG_UNUSED(attrib);

    auto irPatchFunc = irFunc->findDecoration<IRPatchConstantFuncDecoration>();
    assert(irPatchFunc);
    if (!irPatchFunc)
    {
        SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Unable to find [patchConstantFunc(...)] decoration");
        return;
    }

    const String irName = getName(irPatchFunc->getFunc());

    m_writer->emit("[patchconstantfunc(\"");
    m_writer->emit(irName);
    m_writer->emit("\")]\n");
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

void HLSLSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    _emitHLSLEntryPointAttributes(irFunc, entryPointLayout);
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
            auto toType = extractBaseType(inst->getDataType());
            switch (toType)
            {
                default:
                    m_writer->emit("/* unhandled */");
                    break;
                case BaseType::UInt:
                    break;
                case BaseType::Int:
                    m_writer->emit("(");
                    emitType(inst->getDataType());
                    m_writer->emit(")");
                    break;
                case BaseType::Float:
                    m_writer->emit("asfloat");
                    break;
            }

            m_writer->emit("(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
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
        if (auto varLayout = as<VarLayout>(layout))
        {
            emitSemantics(varLayout);
        }
        else if (auto entryPointLayout = as<EntryPointLayout>(layout))
        {
            if (auto resultLayout = entryPointLayout->resultLayout)
            {
                emitSemantics(resultLayout);
            }
        }
    }
}

void HLSLSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    if (auto layoutDecor = param->findDecoration<IRLayoutDecoration>())
    {
        Layout* layout = layoutDecor->getLayout();
        VarLayout* varLayout = as<VarLayout>(layout);

        if (varLayout)
        {
            auto var = varLayout->getVariable();

            if (auto primTypeModifier = var->FindModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
            {
                if (as<HLSLTriangleModifier>(primTypeModifier))
                    m_writer->emit("triangle ");
                else if (as<HLSLPointModifier>(primTypeModifier))
                    m_writer->emit("point ");
                else if (as<HLSLLineModifier>(primTypeModifier))
                    m_writer->emit("line ");
                else if (as<HLSLLineAdjModifier>(primTypeModifier))
                    m_writer->emit("lineadj ");
                else if (as<HLSLTriangleAdjModifier>(primTypeModifier))
                    m_writer->emit("triangleadj ");
            }
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

void HLSLSourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, VarLayout* layout)
{
    SLANG_UNUSED(layout);
    SLANG_UNUSED(valueType);

    for (auto dd : varInst->getDecorations())
    {
        if (dd->op != kIROp_InterpolationModeDecoration)
            continue;

        auto decoration = (IRInterpolationModeDecoration*)dd;
  
        UnownedStringSlice modeText = _getInterpolationModifierText(decoration->getMode());
        if (modeText.size() > 0)
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

void HLSLSourceEmitter::emitMatrixLayoutModifiersImpl(VarLayout* layout)
{
    // When a variable has a matrix type, we want to emit an explicit
    // layout qualifier based on what the layout has been computed to be.
    //

    auto typeLayout = layout->typeLayout;
    while (auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
        typeLayout = arrayTypeLayout->elementTypeLayout;

    if (auto matrixTypeLayout = typeLayout.as<MatrixTypeLayout>())
    {
        switch (matrixTypeLayout->mode)
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
