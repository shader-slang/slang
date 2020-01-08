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
    outName << _getTypeName(texType->getElementType());
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

void CUDASourceEmitter::emitSpecializedOperationDefinition(const HLSLIntrinsic* specOp)
{
    m_writer->emit("__device__ ");
    Super::emitSpecializedOperationDefinition(specOp);
}

SlangResult CUDASourceEmitter::calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
{
    SLANG_UNUSED(target);
    SLANG_ASSERT(target == CodeGenTarget::CUDASource);

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
        case kIROp_HLSLStructuredBufferType:
        {
            auto bufferType = as<IRHLSLStructuredBufferType>(type);
            out << "const ";
            calcTypeName(bufferType->getElementType(), target, out);
            out << "* ";
            return SLANG_OK;
        }
        case kIROp_HLSLRWStructuredBufferType:
        {
            auto bufferType = as<IRHLSLRWStructuredBufferType>(type);
            calcTypeName(bufferType->getElementType(), target, out);
            out << "* ";
            return SLANG_OK;
        }

#if 0
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            auto elementType = matType->getElementType();
            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            out << "Matrix<" << getBuiltinTypeName(elementType->op) << ", " << rowCount << ", " << colCount << ">";
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

            break;
        }
    }

    return Super::calcTypeName(type, target, out);
}

void CUDASourceEmitter::emitLayoutSemanticsImpl(IRInst* inst, char const* uniformSemanticSpelling)
{
    Super::emitLayoutSemanticsImpl(inst, uniformSemanticSpelling);
}

void CUDASourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    Super::emitParameterGroupImpl(varDecl, type);
}

void CUDASourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    SLANG_UNUSED(irFunc);
    SLANG_UNUSED(entryPointDecor);
}

void CUDASourceEmitter::emitOperandImpl(IRInst* inst, EmitOpInfo const& outerPrec)
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

    Super::emitOperandImpl(inst, outerPrec);
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
                m_writer->emit(_getTypeName(inst->getDataType()));
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
    m_writer->emit(_getTypeName(type));
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
    // Mark as run on device. Don't need to worry about entry point, as that is output separtely to call the __device_ implementation
    m_writer->emit("__device__ ");    

    CLikeSourceEmitter::emitSimpleFuncImpl(func);
}

void CUDASourceEmitter::emitSemanticsImpl(IRInst* inst)
{
    Super::emitSemanticsImpl(inst);
}

void CUDASourceEmitter::emitInterpolationModifiersImpl(IRInst* varInst, IRType* valueType, IRVarLayout* layout)
{
    Super::emitInterpolationModifiersImpl(varInst, valueType, layout);
}

void CUDASourceEmitter::emitVarDecorationsImpl(IRInst* varDecl)
{
    Super::emitVarDecorationsImpl(varDecl);
}

void CUDASourceEmitter::emitMatrixLayoutModifiersImpl(IRVarLayout* layout)
{
    Super::emitMatrixLayoutModifiersImpl(layout);
}

void CUDASourceEmitter::emitPreprocessorDirectivesImpl()
{
    SourceWriter* writer = getSourceWriter();

    writer->emit("\n");

    {
        List<IRType*> types;
        m_typeSet.getTypes(IRTypeSet::Kind::Matrix, types);

        // Emit the type definitions
        for (auto type : types)
        {
            emitTypeDefinition(type);
        }
    }

    // Emit all the intrinsics that were used
    for (const auto& keyValue : m_intrinsicNameMap)
    {
        emitSpecializedOperationDefinition(keyValue.Key);
    }
}

void CUDASourceEmitter::emitModuleImpl(IRModule* module)
{
    // Setup all built in types used in the module
    m_typeSet.addAllBuiltinTypes(module);
    // If any matrix types are used, then we need appropriate vector types too.
    m_typeSet.addVectorForMatrixTypes();

    // We need to add some vector intrinsics - used for calculating thread ids 
    {
        IRType* type = m_typeSet.addVectorType(m_typeSet.getBuilder().getBasicType(BaseType::UInt), 3);
        IRType* args[] = { type, type };

        _addIntrinsic(HLSLIntrinsic::Op::Add,  type, args, SLANG_COUNT_OF(args));
        _addIntrinsic(HLSLIntrinsic::Op::Mul,  type, args, SLANG_COUNT_OF(args));
    }

    // TODO(JS): We may need to generate types (for example for matrices)

    // TODO(JS): We need to determine which functions we need to inline

    // The IR will usually come in an order that respects
    // dependencies between global declarations, but this
    // isn't guaranteed, so we need to be careful about
    // the order in which we emit things.

    List<EmitAction> actions;

    computeEmitActions(module, actions);


    _emitForwardDeclarations(actions);

    IRGlobalParam* entryPointGlobalParams = nullptr;

    // Output the global parameters in a 'UniformState' structure
    {
        m_writer->emit("struct UniformState\n{\n");
        m_writer->indent();

        // We need these to be prefixed by __device__
        _emitUniformStateMembers(actions, &entryPointGlobalParams);

        m_writer->dedent();
        m_writer->emit("\n};\n\n");
    }

    // Output the 'Context' which will be used for execution
    {
        m_writer->emit("struct Context\n{\n");
        m_writer->indent();

        m_writer->emit("UniformState* uniformState;\n");

        if (entryPointGlobalParams)
        {
            emitGlobalInst(entryPointGlobalParams);
        }

        // Output all the thread locals 
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && _isVariable(action.inst->op))
            {
                emitGlobalInst(action.inst);
            }
        }

        // Finally output the functions as methods on the context
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && as<IRFunc>(action.inst))
            {
                emitGlobalInst(action.inst);
            }
        }

        m_writer->dedent();
        m_writer->emit("};\n\n");
    }

    // Finally we need to output dll entry points

    for (auto action : actions)
    {
        if (action.level == EmitAction::Level::Definition && as<IRFunc>(action.inst))
        {
            IRFunc* func = as<IRFunc>(action.inst);

            IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>();

            if (entryPointDecor && entryPointDecor->getProfile().GetStage() == Stage::Compute)
            {
                Int sizeAlongAxis[kThreadGroupAxisCount];
                getComputeThreadGroupSize(func, sizeAlongAxis);

                // 
                m_writer->emit("// [numthreads(");
                for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
                {
                    if (ii != 0) m_writer->emit(", ");
                    m_writer->emit(sizeAlongAxis[ii]);
                }
                m_writer->emit(")]\n");

                String funcName = getName(func);

                m_writer->emit("extern \"C\" __global__  ");
               
                auto resultType = func->getResultType();

                // Emit the actual function
                emitEntryPointAttributes(func, entryPointDecor);
                emitType(resultType, funcName);

                m_writer->emit("(UniformEntryPointParams* params, UniformState* uniformState)");
                emitSemantics(func);
                m_writer->emit("\n{\n");
                m_writer->indent();

                // Initialize when constructing so that globals are zeroed
                m_writer->emit("Context context = {};\n");
                m_writer->emit("context.uniformState = uniformState;\n");

                if (entryPointGlobalParams)
                {
                    auto varDecl = entryPointGlobalParams;
                    auto rawType = varDecl->getDataType();

                    auto varType = rawType;

                    m_writer->emit("context.");
                    m_writer->emit(getName(varDecl));
                    m_writer->emit(" =  (");
                    emitType(varType);
                    m_writer->emit("*)params; \n");
                }

                m_writer->emit("context.");
                m_writer->emit(funcName);
                m_writer->emit("();\n");

                m_writer->dedent();
                m_writer->emit("}\n");
            }
        }
    }
    
}


} // namespace Slang
