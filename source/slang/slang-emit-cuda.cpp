// slang-emit-cuda.cpp
#include "slang-emit-cuda.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

/* static */ UnownedStringSlice CUDASourceEmitter::getBuiltinTypeName(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:    return UnownedStringSlice("void");
        case kIROp_BoolType:    return UnownedStringSlice("bool");

        case kIROp_Int8Type:    return UnownedStringSlice("char");
        case kIROp_Int16Type:   return UnownedStringSlice("short");
        case kIROp_IntType:     return UnownedStringSlice("int");
        case kIROp_Int64Type:   return UnownedStringSlice("long long");

        case kIROp_UInt8Type:   return UnownedStringSlice("unsigned char");
        case kIROp_UInt16Type:  return UnownedStringSlice("unsigned short");
        case kIROp_UIntType:    return UnownedStringSlice("unsigned int");
        case kIROp_UInt64Type:  return UnownedStringSlice("unsigned long long");

            // Not clear just yet how we should handle half... we want all processing as float probly, but when reading/writing to memory converting
        case kIROp_HalfType:    return UnownedStringSlice("half");

        case kIROp_FloatType:   return UnownedStringSlice("float");
        case kIROp_DoubleType:  return UnownedStringSlice("double");
        default:                return UnownedStringSlice();
    }
}


/* static */ UnownedStringSlice CUDASourceEmitter::getVectorPrefix(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:    return UnownedStringSlice("bool");

        case kIROp_Int8Type:    return UnownedStringSlice("char");
        case kIROp_Int16Type:   return UnownedStringSlice("short");
        case kIROp_IntType:     return UnownedStringSlice("int");
        case kIROp_Int64Type:   return UnownedStringSlice("longlong");

        case kIROp_UInt8Type:   return UnownedStringSlice("uchar");
        case kIROp_UInt16Type:  return UnownedStringSlice("ushort");
        case kIROp_UIntType:    return UnownedStringSlice("uint");
        case kIROp_UInt64Type:  return UnownedStringSlice("ulonglong");

            // Not clear just yet how we should handle half... we want all processing as float probly, but when reading/writing to memory converting
        case kIROp_HalfType:    return UnownedStringSlice("half");

        case kIROp_FloatType:   return UnownedStringSlice("float");
        case kIROp_DoubleType:  return UnownedStringSlice("double");
        default:                return UnownedStringSlice();
    }
}

SlangResult CUDASourceEmitter::_calcCUDATextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName)
{
    // texture<float, cudaTextureType2D, cudaReadModeElementType> texRef;

    // Not clear how to do this yet
    if (texType->isMultisample() || texType->isArray())
    {
        return SLANG_FAIL;
    }

    outName << "texture<";
    outName << _getCUDATypeName(texType->getElementType());
    outName << ", ";

    switch (texType->GetBaseShape())
    {
        case TextureFlavor::Shape::Shape1D:		outName << "cudaTextureType1D";		break;
        case TextureFlavor::Shape::Shape2D:		outName << "cudaTextureType2D";		break;
        case TextureFlavor::Shape::Shape3D:		outName << "cudaTextureType3D";		break;
        case TextureFlavor::Shape::ShapeCube:	outName << "cudaTextureTypeCubemap";	break;
        case TextureFlavor::Shape::ShapeBuffer: outName << "Buffer";         break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            return SLANG_FAIL;
    }

    outName << ", ";

    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ:
        {
            // Other value is cudaReadModeNormalizedFloat 

            outName << "cudaReadModeElementType";
            break;
        }
        default:
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            return SLANG_FAIL;
        }
    }

    outName << ">";
    return SLANG_OK;
}

// This is junk.. 
static UnownedStringSlice _getCUDAResourceTypePrefix(IROp op)
{
    switch (op)
    {
        case kIROp_HLSLStructuredBufferType:            return UnownedStringSlice::fromLiteral("StructuredBuffer");
        case kIROp_HLSLRWStructuredBufferType:          return UnownedStringSlice::fromLiteral("RWStructuredBuffer");
        case kIROp_HLSLRWByteAddressBufferType:         return UnownedStringSlice::fromLiteral("RWByteAddressBuffer");
        case kIROp_HLSLByteAddressBufferType:           return UnownedStringSlice::fromLiteral("ByteAddressBuffer");
        case kIROp_SamplerStateType:                    return UnownedStringSlice::fromLiteral("SamplerState");
        case kIROp_SamplerComparisonStateType:                  return UnownedStringSlice::fromLiteral("SamplerComparisonState");
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:   return UnownedStringSlice::fromLiteral("RasterizerOrderedStructuredBuffer");
        case kIROp_HLSLAppendStructuredBufferType:              return UnownedStringSlice::fromLiteral("AppendStructuredBuffer");
        case kIROp_HLSLConsumeStructuredBufferType:             return UnownedStringSlice::fromLiteral("ConsumeStructuredBuffer");
        case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  return UnownedStringSlice::fromLiteral("RasterizerOrderedByteAddressBuffer");
        case kIROp_RaytracingAccelerationStructureType:         return UnownedStringSlice::fromLiteral("RaytracingAccelerationStructure");

        default:                                        return UnownedStringSlice();
    }
}

SlangResult CUDASourceEmitter::_calcCUDATypeName(IRType* type, StringBuilder& out)
{
    switch (type->op)
    {
        case kIROp_HalfType:
        {
            // Special case half
            out << getBuiltinTypeName(kIROp_FloatType);
            return SLANG_OK;
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto vecCount = int(GetIntVal(vecType->getElementCount()));
            const IROp elemType = vecType->getElementType()->op;

            UnownedStringSlice prefix = getVectorPrefix(elemType);
            if (prefix.size() <= 0)
            {
                return SLANG_FAIL;
            }
            out << prefix << vecCount;
            return SLANG_OK;
        }
#if 0
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            auto elementType = matType->getElementType();
            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            if (target == CodeGenTarget::CPPSource)
            {
                out << "Matrix<" << getBuiltinTypeName(elementType->op) << ", " << rowCount << ", " << colCount << ">";
            }
            else
            {
                out << "Mat";
                const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elementType->op));
                out << postFix;
                if (postFix.size() > 1)
                {
                    out << "_";
                }
                out << rowCount;
                out << colCount;
            }
            return SLANG_OK;
        }
        case kIROp_ArrayType:
        {
            auto arrayType = static_cast<IRArrayType*>(type);
            auto elementType = arrayType->getElementType();
            int elementCount = int(GetIntVal(arrayType->getElementCount()));

            out << "FixedArray<";
            SLANG_RETURN_ON_FAIL(_calcTypeName(elementType, target, out));
            out << ", " << elementCount << ">";
            return SLANG_OK;
        }
        case kIROp_UnsizedArrayType:
        {
            auto arrayType = static_cast<IRUnsizedArrayType*>(type);
            auto elementType = arrayType->getElementType();

            out << "Array<";
            SLANG_RETURN_ON_FAIL(_calcTypeName(elementType, target, out));
            out << ">";
            return SLANG_OK;
        }
#endif
        default:
        {
            if (isNominalOp(type->op))
            {
                out << getName(type);
                return SLANG_OK;
            }

            if (IRBasicType::isaImpl(type->op))
            {
                out << getBuiltinTypeName(type->op);
                return SLANG_OK;
            }

            if (auto texType = as<IRTextureTypeBase>(type))
            {
                // We don't support TextureSampler, so ignore that
                if (texType->op != kIROp_TextureSamplerType)
                {
                    return _calcCUDATextureTypeName(texType, out);
                }
            }

            // If _getResourceTypePrefix returns something, we assume can output any specialization after it in order.
            {
                UnownedStringSlice prefix = _getCUDAResourceTypePrefix(type->op);
                if (prefix.size() > 0)
                {
                    auto oldWriter = m_writer;
                    SourceManager* sourceManager = oldWriter->getSourceManager();

                    // TODO(JS): This is a bit of a hack. We don't want to emit the result here,
                    // so we replace the writer, write out the type, grab the contents, and restore the writer

                    SourceWriter writer(sourceManager, LineDirectiveMode::None);
                    m_writer = &writer;

                    m_writer->emit(prefix);

                    // TODO(JS).
                    // Assumes ordering of types matches ordering of operands.

                    UInt operandCount = type->getOperandCount();
                    if (operandCount)
                    {
                        m_writer->emit("<");
                        for (UInt ii = 0; ii < operandCount; ++ii)
                        {
                            if (ii != 0)
                            {
                                m_writer->emit(", ");
                            }
                            emitVal(type->getOperand(ii), getInfo(EmitOp::General));
                        }
                        m_writer->emit(">");
                    }

                    out << writer.getContent();

                    m_writer = oldWriter;
                    return SLANG_OK;
                }
            }

            break;
        }
    }

    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type for CUDA emit");
    return SLANG_FAIL;
}


UnownedStringSlice CUDASourceEmitter::_getCUDATypeName(IRType* type)
{
    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_typeNameMap.TryGetValue(type, handle))
    {
        return m_slicePool.getSlice(handle);
    }

#if 0
    if (type->op == kIROp_MatrixType)
    {
        auto matType = static_cast<IRMatrixType*>(type);

        auto elementType = matType->getElementType();
        const auto rowCount = int(GetIntVal(matType->getRowCount()));
        const auto colCount = int(GetIntVal(matType->getColumnCount()));

        // Make sure the vector type the matrix is built on is added
        useType(_getVecType(elementType, colCount));
    }
#endif

    StringBuilder builder;
    if (SLANG_SUCCEEDED(_calcCUDATypeName(type, builder)))
    {
        handle = m_slicePool.add(builder);
    }

    m_typeNameMap.Add(type, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

void CUDASourceEmitter::_emitCUDADecorationSingleString(const char* name, IRFunc* entryPoint, IRStringLit* val)
{
    SLANG_UNUSED(entryPoint);
    assert(val);

    m_writer->emit("[");
    m_writer->emit(name);
    m_writer->emit("(\"");
    m_writer->emit(val->getStringSlice());
    m_writer->emit("\")]\n");
}

void CUDASourceEmitter::_emitCUDADecorationSingleInt(const char* name, IRFunc* entryPoint, IRIntLit* val)
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

void CUDASourceEmitter::_emitCUDARegisterSemantic(LayoutResourceKind kind, EmitVarChain* chain, char const* uniformSemanticSpelling)
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

void CUDASourceEmitter::_emitCUDARegisterSemantics(EmitVarChain* chain, char const* uniformSemanticSpelling)
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
        _emitCUDARegisterSemantic(rr->getResourceKind(), chain, uniformSemanticSpelling);
    }
}

void CUDASourceEmitter::_emitCUDARegisterSemantics(IRVarLayout* varLayout, char const* uniformSemanticSpelling)
{
    if (!varLayout)
        return;

    EmitVarChain chain(varLayout);
    _emitCUDARegisterSemantics(&chain, uniformSemanticSpelling);
}

void CUDASourceEmitter::_emitCUDAParameterGroupFieldLayoutSemantics(EmitVarChain* chain)
{
    if (!chain)
        return;

    auto layout = chain->varLayout;
    for (auto rr : layout->getOffsetAttrs())
    {
        _emitCUDARegisterSemantic(rr->getResourceKind(), chain, "packoffset");
    }
}

void CUDASourceEmitter::_emitCUDAParameterGroupFieldLayoutSemantics(IRVarLayout* fieldLayout, EmitVarChain* inChain)
{
    EmitVarChain chain(fieldLayout, inChain);
    _emitCUDAParameterGroupFieldLayoutSemantics(&chain);
}

void CUDASourceEmitter::_emitCUDAParameterGroup(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
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

    _emitCUDARegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

    m_writer->emit("\n{\n");
    m_writer->indent();

    auto elementType = type->getElementType();

    emitType(elementType, getName(varDecl));
    m_writer->emit(";\n");

    m_writer->dedent();
    m_writer->emit("}\n");
}

void CUDASourceEmitter::emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling)
{
    auto layout = getVarLayout(inst); 
    if (layout)
    {
        _emitCUDARegisterSemantics(layout, uniformSemanticSpelling);
    }
}

void CUDASourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    _emitCUDAParameterGroup(varDecl, type);
}

void CUDASourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    auto profile = m_effectiveProfile;
    auto stage = entryPointDecor->getProfile().GetStage();

    switch (stage)
    {
        case Stage::Compute:
        {
            Int sizeAlongAxis[kThreadGroupAxisCount];
            getComputeThreadGroupSize(irFunc, sizeAlongAxis);

#if 0
            m_writer->emit("[numthreads(");
            for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit(sizeAlongAxis[ii]);
            }
            m_writer->emit(")]\n");
#endif

            m_writer->emit("__global__ ");
            break;
        }
        
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
    }
}

void CUDASourceEmitter::emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec)
{
    if (shouldFoldInstIntoUseSites(inst))
    {
        emitInstExpr(inst, outerPrec);
        return;
    }

    switch (inst->op)
    {
        case kIROp_Param:
        {
            auto varLayout = getVarLayout(inst);
            if (varLayout)
            {
                if (auto systemValueSemantic = varLayout->findSystemValueSemanticAttr())
                {
                    String semanticNameSpelling = systemValueSemantic->getName();
                    semanticNameSpelling = semanticNameSpelling.toLower();

                    if (semanticNameSpelling == "sv_dispatchthreadid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::DispatchThreadID;
                        m_writer->emit("((blockIdx * blockDim) + threadIdx)");

                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::GroupID;
                        m_writer->emit("blockIdx");
                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupthreadid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::GroupThreadID;
                        m_writer->emit("threadIdx");
                        return;
                    }
                }
            }

            break;
        }
        default: break;
    }
    m_writer->emit(getName(inst));
}

bool CUDASourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->op)
    {
        case kIROp_Construct:
        case kIROp_makeVector:
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
            else
            {
                m_writer->emit("make_");
                m_writer->emit(_getCUDATypeName(inst->getDataType()));
                emitArgs(inst);
                return true;
            }
            break;
        }
        case kIROp_MakeMatrix:
        {
            return false;
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

void CUDASourceEmitter::emitLayoutDirectivesImpl(TargetRequest* targetReq)
{
    SLANG_UNUSED(targetReq);
}

void CUDASourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    m_writer->emit(getVectorPrefix(elementType->op));
    m_writer->emit(elementCount);
}

void CUDASourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    m_writer->emit(_getCUDATypeName(type));
}

void CUDASourceEmitter::emitRateQualifiersImpl(IRRate* rate)
{
    if (as<IRGroupSharedRate>(rate))
    {
        m_writer->emit("groupshared ");
    }
}

void CUDASourceEmitter::emitSimpleFuncParamsImpl(IRFunc* func)
{
    m_writer->emit("(");

    bool hasEmittedParam = false;
    auto firstParam = func->getFirstParam();
    for (auto pp = firstParam; pp; pp = pp->getNextParam())
    {
        auto varLayout = getVarLayout(pp);
        if (varLayout && varLayout->findSystemValueSemanticAttr())
        {
            // If it has a semantic don't output, it will be accessed via a global
            continue;
        }

        if (hasEmittedParam)
            m_writer->emit(", ");

        emitSimpleFuncParamImpl(pp);
        hasEmittedParam = true;
    }

    m_writer->emit(")");
}

void CUDASourceEmitter::emitSimpleFuncImpl(IRFunc* func)
{
    if (IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>())
    {
        // If its an entry point, we let the entry point attribute control the output
        Super::emitSimpleFuncImpl(func);
    }
    else
    {
        // If it's not an entry point mark as device
        m_writer->emit("__device__ ");
        Super::emitSimpleFuncImpl(func);
    }
}

void CUDASourceEmitter::emitSemanticsImpl(IRInst* inst)
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

void CUDASourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout)
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

void CUDASourceEmitter::emitVarDecorationsImpl(IRInst* varDecl)
{
    if (varDecl->findDecoration<IRGloballyCoherentDecoration>())
    {
        m_writer->emit("globallycoherent\n");
    }
}

void CUDASourceEmitter::emitMatrixLayoutModifiersImpl(IRVarLayout* layout)
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
