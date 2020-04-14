// slang-emit-cuda.cpp
#include "slang-emit-cuda.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

static bool _isSingleNameBasicType(IROp op)
{
    switch (op)
    {
        case kIROp_Int64Type:   
        case kIROp_UInt8Type: 
        case kIROp_UInt16Type:
        case kIROp_UIntType: 
        case kIROp_UInt64Type:
        {
            return false;
        }
        default: return true;

    }
}

/* static */ UnownedStringSlice CUDASourceEmitter::getBuiltinTypeName(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:    return UnownedStringSlice("void");
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
    // Not clear how to do this yet
    if (texType->isMultisample())
    {
        return SLANG_FAIL;
    }

    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ:
        {
            outName << "CUtexObject";
            return SLANG_OK;
        }
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
        {
            outName << "CUsurfObject";
            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

SlangResult CUDASourceEmitter::calcScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type, StringBuilder& outBuilder)
{
    typedef HLSLIntrinsic::Op Op;

    UnownedStringSlice funcName;
    
    switch (op)
    {
        case Op::FRem:
        {
            if (type->op == kIROp_FloatType || type->op == kIROp_DoubleType)
            {
                funcName = HLSLIntrinsic::getInfo(op).funcName;
            }
            break;
        }
        default: break;
    }

    if (funcName.getLength())
    {
        outBuilder << funcName;
        if (type->op == kIROp_FloatType)
        {
            outBuilder << "f";
        }
        return SLANG_OK;
    }

    // Defer to the supers impl
    return Super::calcScalarFuncName(op, type, outBuilder);
}

SlangResult CUDASourceEmitter::calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
{
    SLANG_UNUSED(target);

    if (target == CodeGenTarget::CSource)
    {
        return Super::calcTypeName(type, target, out);
    }

    // We allow C source, because if we need a name 
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
            if (prefix.getLength() <= 0)
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

            switch (type->op)
            {
                case kIROp_SamplerStateType:                    out << "SamplerState"; return SLANG_OK;
                case kIROp_SamplerComparisonStateType:          out << "SamplerComparisonState"; return SLANG_OK;
                default: break;
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

void CUDASourceEmitter::emitCall(const HLSLIntrinsic* specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec)
{
    switch (specOp->op)
    {
        case HLSLIntrinsic::Op::Init:
        {
            // For CUDA vector types we construct with make_

            auto writer = m_writer;

            IRType* retType = specOp->returnType;

            if (IRVectorType* vecType = as<IRVectorType>(retType))
            {
                if (numOperands == GetIntVal(vecType->getElementCount()))
                {
                    // Get the type name
                    writer->emit("make_");
                    emitType(retType);
                    writer->emitChar('(');

                    for (int i = 0; i < numOperands; ++i)
                    {
                        if (i > 0)
                        {
                            writer->emit(", ");
                        }
                        emitOperand(operands[i].get(), getInfo(EmitOp::General));
                    }

                    writer->emitChar(')');
                    return;
                }
            }
            // Just use the default
            break;
        }
        default: break;
    }

    return Super::emitCall(specOp, inst, operands, numOperands, inOuterPrec);
}

void CUDASourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    if (decl->getMode() == kIRLoopControl_Unroll)
    {
        m_writer->emit("#pragma unroll\n");
    }
}

static bool _areEquivalent(IRType* a, IRType* b)
{
    if (a == b)
    {
        return true;
    }
    if (a->op != b->op)
    {
        return false;
    }

    switch (a->op)
    {
        case kIROp_VectorType:
        {
            IRVectorType* vecA = static_cast<IRVectorType*>(a);
            IRVectorType* vecB = static_cast<IRVectorType*>(b);
            return GetIntVal(vecA->getElementCount()) == GetIntVal(vecB->getElementCount()) &&
                _areEquivalent(vecA->getElementType(), vecB->getElementType());
        }
        case kIROp_MatrixType:
        {
            IRMatrixType* matA = static_cast<IRMatrixType*>(a);
            IRMatrixType* matB = static_cast<IRMatrixType*>(b);
            return GetIntVal(matA->getColumnCount()) == GetIntVal(matB->getColumnCount()) &&
                GetIntVal(matA->getRowCount()) == GetIntVal(matB->getRowCount()) && 
                _areEquivalent(matA->getElementType(), matB->getElementType());
        }
        default:
        {
            return as<IRBasicType>(a) != nullptr;
        }
    }
}

void CUDASourceEmitter::_emitInitializerListValue(IRType* dstType, IRInst* value)
{
    // When constructing a matrix or vector from a single value this is handled by the default path

    switch (value->op)
    {
        case kIROp_Construct:
        case kIROp_MakeMatrix:
        case kIROp_makeVector:
        {
            IRType* type = value->getDataType();

            // If the types are the same, we can can just break down and use
            if (_areEquivalent(dstType, type))
            {
                if (auto vecType = as<IRVectorType>(type))
                {
                    if (UInt(GetIntVal(vecType->getElementCount())) == value->getOperandCount())
                    {
                        _emitInitializerList(vecType->getElementType(), value->getOperands(), value->getOperandCount());
                        return;
                    }
                }
                else if (auto matType = as<IRMatrixType>(type))
                {
                    const Index colCount = Index(GetIntVal(matType->getColumnCount()));
                    const Index rowCount = Index(GetIntVal(matType->getRowCount()));

                    // TODO(JS): If num cols = 1, then it *doesn't* actually return a vector.
                    // That could be argued is an error because we want swizzling or [] to work.
                    IRType* rowType = m_typeSet.addVectorType(matType->getElementType(), int(colCount));
                    IRVectorType* rowVectorType = as<IRVectorType>(rowType);
                    const Index operandCount = Index(value->getOperandCount());

                    // Can init, with vectors.
                    // For now special case if the rowVectorType is not actually a vector (when elementSize == 1)
                    if (operandCount == rowCount || rowVectorType == nullptr)
                    {
                        // We have to output vectors

                        // Emit the braces for the Matrix struct, contains an row array.
                        m_writer->emit("{\n");
                        m_writer->indent();
                        _emitInitializerList(rowType, value->getOperands(), rowCount);
                        m_writer->dedent();
                        m_writer->emit("\n}");
                        return;
                    }
                    else if (operandCount == rowCount * colCount)
                    {
                        // Handle if all are explicitly defined
                        IRType* elementType = matType->getElementType();                                        
                        IRUse* operands = value->getOperands();

                        // Emit the braces for the Matrix struct, and the array of rows
                        m_writer->emit("{\n");
                        m_writer->indent();
                        m_writer->emit("{\n");
                        m_writer->indent();
                        for (Index i = 0; i < rowCount; ++i)
                        {
                            if (i != 0) m_writer->emit(", ");
                            _emitInitializerList(elementType, operands, colCount);
                            operands += colCount;
                        }
                        m_writer->dedent();
                        m_writer->emit("\n}");
                        m_writer->dedent();
                        m_writer->emit("\n}");
                        return;
                    }
                }
            }
                      
            break;
        }
    }

    // All other cases we just use the default emitting - might not work on arrays defined in global scope on CUDA though
    emitOperand(value, getInfo(EmitOp::General));
}

void CUDASourceEmitter::_emitInitializerList(IRType* elementType, IRUse* operands, Index operandCount)
{
    m_writer->emit("{\n");
    m_writer->indent();

    for (Index i = 0; i < operandCount; ++i)
    {
        if (i != 0) m_writer->emit(", ");
        _emitInitializerListValue(elementType, operands[i].get());
    }

    m_writer->dedent();
    m_writer->emit("\n}");
}

bool CUDASourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch(inst->op)
    {
        case kIROp_Construct:
        {
            // Simple constructor call
            // On CUDA some of the built in types can't be used as constructors directly

            IRType* type = inst->getDataType();
            if (auto basicType = as<IRBasicType>(type) && !_isSingleNameBasicType(type->op))
            {
                m_writer->emit("(");
                emitType(inst->getDataType());
                m_writer->emit(")");
                emitArgs(inst);
                return true;
            }
            break;
        }
        case kIROp_makeArray:
        {
            IRType* dataType = inst->getDataType();
            IRArrayType* arrayType = as<IRArrayType>(dataType);

            IRType* elementType = arrayType->getElementType();

            // Emit braces for the FixedArray struct. 
            m_writer->emit("{\n");
            m_writer->indent();

            _emitInitializerList(elementType, inst->getOperands(), Index(inst->getOperandCount()));

            m_writer->dedent();
            m_writer->emit("\n}");
            return true;
        }
        default: break;
    }

    return Super::tryEmitInstExprImpl(inst, inOuterPrec);
}

void CUDASourceEmitter::handleCallExprDecorationsImpl(IRInst* funcValue)
{
    // Does this function declare any requirements on GLSL version or
    // extensions, which should affect our output?

    auto decoratedValue = funcValue;
    while (auto specInst = as<IRSpecialize>(decoratedValue))
    {
        decoratedValue = getSpecializedValue(specInst);
    }

    for (auto decoration : decoratedValue->getDecorations())
    {
        if( auto smDecoration = as<IRRequireCUDASMVersionDecoration>(decoration))
        {
            SemanticVersion version;
            version.setFromInteger(SemanticVersion::IntegerType(smDecoration->getCUDASMVersion()));

            if (version > m_extensionTracker->m_smVersion)
            {
                m_extensionTracker->m_smVersion = version;
            }
        }
    }
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
        m_writer->emit("__shared__ ");
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
    // Skip the CPP impl - as it does some processing we don't need here for entry points.
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

    {
        List<const HLSLIntrinsic*> intrinsics;
        m_intrinsicSet.getIntrinsics(intrinsics);
        // Emit all the intrinsics that were used
        for (auto intrinsic : intrinsics)
        {
            _maybeEmitSpecializedOperationDefinition(intrinsic);
        }
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

    // Output group shared variables

    {
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && action.inst->op == kIROp_GlobalVar && as<IRGroupSharedRate>(action.inst->getRate()))
            {
                emitGlobalInst(action.inst);   
            }
        }
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
            if (action.level == EmitAction::Level::Definition && action.inst->op == kIROp_GlobalVar && !as<IRGroupSharedRate>(action.inst->getRate()))
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

            if (entryPointDecor)
            {
                // We have an entry-point function in the IR module, which we
                // will want to emit as a `__global__` function in the generated
                // CUDA C++.
                //
                // The most common case will be a compute kernel, in which case
                // we will emit the function more or less as-is, including
                // usingits original name as the name of the global symbol.
                //
                String funcName = getName(func);
                String globalSymbolName = funcName;

                // We also suport emitting ray tracing kernels for use with
                // OptiX, and in that case the name of the global symbol
                // must be prefixed to indicate to the OptiX runtime what
                // stage it is to be compiled for.
                //
                auto stage = entryPointDecor->getProfile().GetStage();
                switch( stage )
                {
                default:
                    break;

            #define CASE(STAGE, PREFIX) \
                case Stage::STAGE: globalSymbolName = #PREFIX + funcName; break

                CASE(RayGeneration, __raygen__);
                // TODO: Add the other ray tracing shader stages here.
            #undef CASE
                }

                if( stage != Stage::Compute )
                {
                    // Non-compute shaders (currently just OptiX ray tracing kernels)
                    // require parameter data that is shared across multiple kernels
                    // (which in our case is the global-scope shader parameters)
                    // to be passed using a global `__constant__` variable.
                    //
                    // The use of `"C"` linkage here is required because the name
                    // of this symbol must be passed to the OptiX API when creating
                    // a pipeline that uses this compiled module. The exact name
                    // used here (`SLANG_globalParams`) is thus a part of the
                    // binary interface for Slang->OptiX translation.
                    //
                    m_writer->emit("extern \"C\" { __constant__ UniformState SLANG_globalParams; }\n");
                }

                // As a convenience for anybody reading the generated
                // CUDA C++ code, we will prefix a compute kernel
                // with the information from the `[numthreads(...)]`
                // attribute in the source.
                //
                if(stage == Stage::Compute)
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
                }

                m_writer->emit("extern \"C\" __global__  ");
               
                auto resultType = func->getResultType();

                // Emit the actual function
                emitEntryPointAttributes(func, entryPointDecor);
                emitType(resultType, globalSymbolName);

                if( stage == Stage::Compute )
                {
                    // CUDA compute shaders take all of their parameters explicitly as
                    // part of the entry-point parameter list. This means that the
                    // data representing Slang shader parameters at both the global
                    // and entry-point scopes needs to be passed as parameters.
                    //
                    // At the binary level, our generated CUDA compute kernels will take
                    // two pointer parameters: the first points to the per-entry-point
                    // `uniform` parameter data, and the second poinst to the global-scope
                    // parameter data (if any).
                    //
                    m_writer->emit("(UniformEntryPointParams* entryPointShaderParameters, UniformState* uniformState)");
                }
                else
                {
                    // Non-compute shaders (currently just OptiX ray tracing kernels)
                    // rely on other mechanisms for parameter passing, and thus use
                    // an empty parameter list on the kernel declaration.
                    //
                    m_writer->emit("()");
                }

                emitSemantics(func);
                m_writer->emit("\n{\n");
                m_writer->indent();

                // Initialize when constructing so that globals are zeroed
                m_writer->emit("Context context = {};\n");

                // The global-scope parameter data got passed in differently depending on whether we have
                // a compute shader or a ray-tracing shader, so we need to alter how we initialize
                // the pointer in our `context` based on the stage.
                //
                if( stage == Stage::Compute )
                {
                    m_writer->emit("context.uniformState = uniformState;\n");
                }
                else
                {
                    m_writer->emit("context.uniformState = &SLANG_globalParams;\n");
                }

                if (entryPointGlobalParams)
                {
                    auto varDecl = entryPointGlobalParams;
                    auto rawType = varDecl->getDataType();
                    auto varType = rawType;

                    m_writer->emit("context.");
                    m_writer->emit(getName(varDecl));
                    m_writer->emit(" =  (");
                    emitType(varType);
                    m_writer->emit("*)");

                    // Similar to the case for global parameter data above, the entry-point
                    // uniform parameter data gets passed in differently for compute kernels
                    // vs. ray-tracing kernels, and we need to handle the two cases here.
                    //
                    if( stage == Stage::Compute )
                    {
                        // In the compute case, the entry-point uniform parameters came
                        // in as an explicit parameter on the CUDA kernel, and we simply
                        // cast it to the expected type here.
                        //
                        m_writer->emit("entryPointShaderParameters");
                    }
                    else
                    {
                        // In the ray-tracing case, the entry-point uniform parameters
                        // implicitly map to the contents of the Shader Binding Table
                        // (SBT) entry for the entry point instance being invoked.
                        //
                        // The OptiX API provides an accessor function to get a pointer
                        // to the SBT data for the current entry, and we cast the result
                        // of that to the expected type.
                        //
                        m_writer->emit("optixGetSbtDataPointer()");
                    }
                    m_writer->emit(";\n");
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
