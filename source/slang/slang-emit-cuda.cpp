// slang-emit-cuda.cpp
#include "slang-emit-cuda.h"

#include "core/slang-writer.h"
#include "slang-emit-source-writer.h"
#include "slang-intrinsic-expand.h"
#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"


namespace Slang
{

static void emitUnsupportedTargetIntrinsicExpr(
    CUDASourceEmitter* emitter,
    IRInst* inst,
    const char* operation,
    SourceLoc location)
{
    emitter->getSink()->diagnose(
        Diagnostics::UnsupportedTargetIntrinsic{.operation = operation, .location = location});
    emitter->getSourceWriter()->emit("(");
    emitter->emitType(inst->getDataType());
    emitter->getSourceWriter()->emit("{})");
}

static UnownedStringSlice getOptixCoopVecComponentTypeName(int componentType)
{
    switch (componentType)
    {
    case SLANG_SCALAR_TYPE_FLOAT_E4M3:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_FLOAT8_E4M3");
    case SLANG_SCALAR_TYPE_FLOAT_E5M2:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_FLOAT8_E5M2");
    case SLANG_SCALAR_TYPE_FLOAT16:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_FLOAT16");
    case SLANG_SCALAR_TYPE_FLOAT32:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_FLOAT32");
    case SLANG_SCALAR_TYPE_INT8:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_INT8");
    case SLANG_SCALAR_TYPE_INT32:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_INT32");
    case SLANG_SCALAR_TYPE_UINT8:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_UINT8");
    case SLANG_SCALAR_TYPE_UINT32:
        return UnownedStringSlice("OPTIX_COOP_VEC_ELEM_TYPE_UINT32");
    default:
        return UnownedStringSlice();
    }
}

static UnownedStringSlice getOptixCoopVecMatrixLayoutName(int matrixLayout)
{
    switch (matrixLayout)
    {
    case SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR:
        return UnownedStringSlice("OPTIX_COOP_VEC_MATRIX_LAYOUT_ROW_MAJOR");
    case SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR:
        return UnownedStringSlice("OPTIX_COOP_VEC_MATRIX_LAYOUT_COLUMN_MAJOR");
    case SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL:
        return UnownedStringSlice("OPTIX_COOP_VEC_MATRIX_LAYOUT_INFERENCING_OPTIMAL");
    case SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL:
        return UnownedStringSlice("OPTIX_COOP_VEC_MATRIX_LAYOUT_TRAINING_OPTIMAL");
    default:
        SLANG_UNEXPECTED("invalid OptiX cooperative vector matrix layout");
    }
}

struct FragmentShape
{
    int m, n, k;

    bool isValid() const { return m > 0 && n > 0 && k > 0; }
};

inline FragmentShape computeShapeCombination(uint32_t matrixUse, uint32_t row, uint32_t col);
static bool coopMatMulAddTypeCombinationIsValid(IROp aType, IROp bType, IROp cType, IROp dType);

static CUDAExtensionTracker::BaseTypeFlags _findBaseTypesUsed(IRModule* module)
{
    typedef CUDAExtensionTracker::BaseTypeFlags Flags;

    // All basic types are hoistable so must be in global scope.
    Flags baseTypesUsed = 0;

    auto moduleInst = module->getModuleInst();

    // Search all the insts in global scope, for BasicTypes
    for (auto inst : moduleInst->getChildren())
    {
        if (auto basicType = as<IRBasicType>(inst))
        {
            // Get the base type, and set the bit
            const auto baseTypeEnum = basicType->getBaseType();

            baseTypesUsed |= Flags(1) << int(baseTypeEnum);
        }
    }

    return baseTypesUsed;
}

void CUDAExtensionTracker::finalize()
{
    if (isBaseTypeRequired(BaseType::Half))
    {
        // The cuda_fp16.hpp header indicates the need is for version 5.3, but when this is tried
        // NVRTC says it cannot load builtins.
        // The lowest version that this does work for is 6.0, so that's what we use here.

        // https://docs.nvidia.com/cuda/nvrtc/index.html#group__options
        requireSMVersion(SemanticVersion(6, 0));
    }
}

UnownedStringSlice CUDASourceEmitter::getBuiltinTypeName(IROp op)
{
    switch (op)
    {
    case kIROp_VoidType:
        return UnownedStringSlice("void");
    case kIROp_BoolType:
        return UnownedStringSlice("bool");

    case kIROp_Int8Type:
        return UnownedStringSlice("char");
    case kIROp_Int16Type:
        return UnownedStringSlice("short");
    case kIROp_IntType:
        return UnownedStringSlice("int");
    case kIROp_Int64Type:
        return UnownedStringSlice("longlong");

    case kIROp_UInt8Type:
        return UnownedStringSlice("uchar");
    case kIROp_UInt16Type:
        return UnownedStringSlice("ushort");
    case kIROp_UIntType:
        return UnownedStringSlice("uint");
    case kIROp_UInt64Type:
        return UnownedStringSlice("ulonglong");

    case kIROp_IntPtrType:
        if (getPointerSize(getTargetReq()) == sizeof(uint64_t))
            return UnownedStringSlice("int64_t");
        else
            return UnownedStringSlice("int");

    case kIROp_UIntPtrType:
        if (getPointerSize(getTargetReq()) == sizeof(uint64_t))
            return UnownedStringSlice("uint64_t");
        else
            return UnownedStringSlice("uint");

    case kIROp_HalfType:
        return UnownedStringSlice("__half");

    case kIROp_FloatType:
        return UnownedStringSlice("float");
    case kIROp_DoubleType:
        return UnownedStringSlice("double");
    case kIROp_FloatE4M3Type:
        return UnownedStringSlice("__nv_fp8_e4m3");
    case kIROp_FloatE5M2Type:
        return UnownedStringSlice("__nv_fp8_e5m2");
    case kIROp_BFloat16Type:
        return UnownedStringSlice("__nv_bfloat16");
    default:
        return UnownedStringSlice();
    }
}


UnownedStringSlice CUDASourceEmitter::getVectorPrefix(IROp op)
{
    switch (op)
    {
    case kIROp_BoolType:
        return UnownedStringSlice("bool");

    case kIROp_Int8Type:
        return UnownedStringSlice("char");
    case kIROp_Int16Type:
        return UnownedStringSlice("short");
    case kIROp_IntType:
        return UnownedStringSlice("int");
    case kIROp_Int64Type:
        return UnownedStringSlice("longlong");

    case kIROp_UInt8Type:
        return UnownedStringSlice("uchar");
    case kIROp_UInt16Type:
        return UnownedStringSlice("ushort");
    case kIROp_UIntType:
        return UnownedStringSlice("uint");
    case kIROp_UInt64Type:
        return UnownedStringSlice("ulonglong");

    case kIROp_IntPtrType:
        if (getPointerSize(getTargetReq()) == sizeof(uint64_t))
            return UnownedStringSlice("longlong");
        else
            return UnownedStringSlice("int");

    case kIROp_UIntPtrType:
        if (getPointerSize(getTargetReq()) == sizeof(uint64_t))
            return UnownedStringSlice("ulonglong");
        else
            return UnownedStringSlice("uint");

    case kIROp_HalfType:
        m_extensionTracker->requireBaseType(BaseType::Half);
        return UnownedStringSlice("__half");

    case kIROp_FloatE4M3Type:
        m_extensionTracker->requireFp8();
        return UnownedStringSlice("__nv_fp8_e4m3");
    case kIROp_FloatE5M2Type:
        m_extensionTracker->requireFp8();
        return UnownedStringSlice("__nv_fp8_e5m2");
    case kIROp_BFloat16Type:
        m_extensionTracker->requireBfloat16();
        return UnownedStringSlice("__nv_bfloat16");

    case kIROp_FloatType:
        return UnownedStringSlice("float");
    case kIROp_DoubleType:
        return UnownedStringSlice("double");
    default:
        return UnownedStringSlice();
    }
}

void CUDASourceEmitter::emitTempModifiers(IRInst* temp)
{
    CPPSourceEmitter::emitTempModifiers(temp);
    if (as<IRModuleInst>(temp->getParent()))
    {
        m_writer->emit("__device__ ");
    }
}

SlangResult CUDASourceEmitter::_calcCUDATextureTypeName(
    IRTextureTypeBase* texType,
    StringBuilder& outName)
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
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
    case SLANG_RESOURCE_ACCESS_WRITE:
        {
            outName << "CUsurfObject";
            return SLANG_OK;
        }
    default:
        break;
    }
    return SLANG_FAIL;
}

SlangResult CUDASourceEmitter::calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
{
    SLANG_UNUSED(target);

    // The names CUDA produces are all compatible with 'C' (ie they aren't templated types)
    SLANG_ASSERT(
        target == CodeGenTarget::CUDASource || target == CodeGenTarget::CUDAHeader ||
        target == CodeGenTarget::CSource);

    switch (type->getOp())
    {
    case kIROp_PtrType:
    case kIROp_NativePtrType:
        {
            auto ptrType = cast<IRPtrTypeBase>(type);
            if (auto unsizedArrayType = as<IRUnsizedArrayType>(ptrType->getValueType()))
            {
                SLANG_RETURN_ON_FAIL(calcTypeName(unsizedArrayType->getElementType(), target, out));
                out << "**";
                return SLANG_OK;
            }
            break;
        }
    case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto vecCount = int(getIntVal(vecType->getElementCount()));
            const IROp elemType = vecType->getElementType()->getOp();

            UnownedStringSlice prefix = getVectorPrefix(elemType);
            if (prefix.getLength() <= 0)
            {
                return SLANG_FAIL;
            }
            out << prefix << vecCount;
            return SLANG_OK;
        }
    case kIROp_TensorViewType:
        {
            out << "TensorView";
            return SLANG_OK;
        }
    case kIROp_CoopVectorType:
        {
            if (isOptixCoopVec)
            {
                auto coopVecType = static_cast<IRCoopVectorType*>(type);
                auto elemCount = int(getIntVal(coopVecType->getElementCount()));
                auto elemType = coopVecType->getElementType();

                out << "OptixCoopVec<" << getBuiltinTypeName(elemType->getOp()) << ", " << elemCount
                    << ">";
                return SLANG_OK;
            }
            SLANG_DIAGNOSE_UNEXPECTED(
                getSink(),
                SourceLoc(),
                "Cooperative vectors should have been lowered before reaching CUDA emit for "
                "non-OptiX targets");
            return SLANG_FAIL;
        }
    case kIROp_RaytracingAccelerationStructureType:
    case kIROp_HitObjectType:
        {
            out << "OptixTraversableHandle";
            return SLANG_OK;
        }
    case kIROp_CoopMatrixType:
        {
            auto coopType = as<IRCoopMatrixType>(type);
            auto result = emitWMMAFragmentType(coopType, out);
            m_extensionTracker->requireSMVersion(SemanticVersion(8, 0));
            // FP8 mma instructions (mma.sync.m16n8k16 with .e4m3/.e5m2) were
            // introduced in PTX ISA 8.7 / SM 8.9 (Ada Lovelace).  Earlier SM
            // targets reject the PTX as invalid at JIT time.
            auto elemOp = coopType->getElementType()->getOp();
            if (elemOp == kIROp_FloatE4M3Type || elemOp == kIROp_FloatE5M2Type)
                m_extensionTracker->requireSMVersion(SemanticVersion(8, 9));
            return result;
        }
    case kIROp_FloatE4M3Type:
        out << "__nv_fp8_e4m3";
        m_extensionTracker->requireFp8();
        return SLANG_OK;
    case kIROp_FloatE5M2Type:
        out << "__nv_fp8_e5m2";
        m_extensionTracker->requireFp8();
        return SLANG_OK;
    case kIROp_BFloat16Type:
        out << "__nv_bfloat16";
        m_extensionTracker->requireBfloat16();
        return SLANG_OK;
    default:
        {
            if (isNominalOp(type->getOp()))
            {
                out << getName(type);
                return SLANG_OK;
            }

            if (IRBasicType::isaImpl(type->getOp()))
            {
                out << getBuiltinTypeName(type->getOp());
                return SLANG_OK;
            }

            if (auto texType = as<IRTextureTypeBase>(type))
            {
                return _calcCUDATextureTypeName(texType, out);
            }

            switch (type->getOp())
            {
            case kIROp_SamplerStateType:
                out << "SamplerState";
                return SLANG_OK;
            case kIROp_SamplerComparisonStateType:
                out << "SamplerComparisonState";
                return SLANG_OK;
            default:
                break;
            }

            break;
        }
    }

    return Super::calcTypeName(type, target, out);
}

void CUDASourceEmitter::emitLayoutSemanticsImpl(
    IRInst* inst,
    char const* uniformSemanticSpelling,
    EmitLayoutSemanticOption layoutSemanticOption)
{
    Super::emitLayoutSemanticsImpl(inst, uniformSemanticSpelling, layoutSemanticOption);
}

void CUDASourceEmitter::emitParameterGroupImpl(
    IRGlobalParam* varDecl,
    IRUniformParameterGroupType* type)
{
    auto elementType = type->getElementType();

    m_writer->emit("extern \"C\" __constant__ ");
    emitType(elementType, "SLANG_globalParams");
    m_writer->emit(";\n");

    m_writer->emit("#define ");
    m_writer->emit(getName(varDecl));
    m_writer->emit(" (&SLANG_globalParams)\n");
}

void CUDASourceEmitter::emitEntryPointAttributesImpl(
    IRFunc* irFunc,
    IREntryPointDecoration* entryPointDecor)
{
    SLANG_UNUSED(irFunc);
    SLANG_UNUSED(entryPointDecor);
}

void CUDASourceEmitter::emitFunctionPreambleImpl(IRInst* inst)
{
    if (!inst)
        return;
    if (inst->findDecoration<IREntryPointDecoration>())
    {
        m_writer->emit("extern \"C\" __global__ ");
        return;
    }

    if (inst->findDecoration<IRCudaKernelDecoration>())
    {
        m_writer->emit("__global__ ");
    }
    else if (inst->findDecoration<IRCudaHostDecoration>())
    {
        m_writer->emit("__host__ ");
    }
    else
    {
        m_writer->emit("__device__ ");

        // `__noinline__` is a declaration specifier, so it belongs in this specifier
        // sequence. Kernels are call-graph roots with no caller to be inlined into, so the
        // request is honoured for ordinary device functions only.
        if (inst->findDecoration<IRNoInlineDecoration>())
        {
            m_writer->emit("__noinline__ ");
        }
    }
}

String CUDASourceEmitter::generateEntryPointNameImpl(IREntryPointDecoration* entryPointDecor)
{
    // We have an entry-point function in the IR module, which we
    // will want to emit as a `__global__` function in the generated
    // CUDA C++.
    //
    // The most common case will be a compute kernel, in which case
    // we will emit the function more or less as-is, including
    // usingits original name as the name of the global symbol.
    //
    String funcName = Super::generateEntryPointNameImpl(entryPointDecor);
    String globalSymbolName = funcName;

    // We also suport emitting ray tracing kernels for use with
    // OptiX, and in that case the name of the global symbol
    // must be prefixed to indicate to the OptiX runtime what
    // stage it is to be compiled for.
    //
    auto stage = entryPointDecor->getProfile().getStage();
    switch (stage)
    {
    default:
        break;

#define CASE(STAGE, PREFIX)                    \
    case Stage::STAGE:                         \
        globalSymbolName = #PREFIX + funcName; \
        break

        // Optix 7 Guide, Section 6.1 (Program input)
        //
        // > The input PTX should include one or more NVIDIA OptiX programs.
        // > The type of program affects how the program can be used during
        // > the execution of the pipeline. These program types are specified
        // by prefixing the program name with the following:
        //
        // >    Program type        Function name prefix
        CASE(RayGeneration, __raygen__);
        CASE(Intersection, __intersection__);
        CASE(AnyHit, __anyhit__);
        CASE(ClosestHit, __closesthit__);
        CASE(Miss, __miss__);
        CASE(Callable, __direct_callable__);
        //
        // There are two stages (or "program types") supported by OptiX
        // that Slang currently cannot target:
        //
        // CASE(ContinuationCallable,   __continuation_callable__);
        // CASE(Exception,              __exception__);
        //
#undef CASE
    }

    return globalSymbolName;
}

void CUDASourceEmitter::emitGlobalRTTISymbolPrefix()
{
    m_writer->emit("__constant__ ");
}

void CUDASourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    if (decl->getMode() == kIRLoopControl_Unroll)
    {
        m_writer->emit("#pragma unroll\n");
    }
}

void CUDASourceEmitter::_emitInitializerListValue(IRType* dstType, IRInst* value)
{
    // When constructing a matrix or vector from a single value this is handled by the default path

    switch (value->getOp())
    {
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
        {
            IRType* type = value->getDataType();

            // If the types are the same, we can can just break down and use
            if (dstType == type)
            {
                if (auto vecType = as<IRVectorType>(type))
                {
                    if (UInt(getIntVal(vecType->getElementCount())) == value->getOperandCount())
                    {
                        emitType(type);
                        _emitInitializerList(
                            vecType->getElementType(),
                            value->getOperands(),
                            value->getOperandCount());
                        return;
                    }
                }
                else if (auto matType = as<IRMatrixType>(type))
                {
                    const Index colCount = Index(getIntVal(matType->getColumnCount()));
                    const Index rowCount = Index(getIntVal(matType->getRowCount()));

                    // TODO(JS): If num cols = 1, then it *doesn't* actually return a vector.
                    // That could be argued is an error because we want swizzling or [] to work.
                    IRBuilder builder(matType->getModule());
                    builder.setInsertBefore(matType);
                    const Index operandCount = Index(value->getOperandCount());

                    // Can init, with vectors.
                    // For now special case if the rowVectorType is not actually a vector (when
                    // elementSize == 1)
                    if (operandCount == rowCount)
                    {
                        // Emit the braces for the Matrix struct, and then each row vector in its
                        // own line.
                        emitType(matType);
                        m_writer->emit("{\n");
                        m_writer->indent();
                        for (Index i = 0; i < rowCount; ++i)
                        {
                            if (i != 0)
                                m_writer->emit(",\n");
                            emitType(matType->getElementType());
                            m_writer->emit(colCount);
                            _emitInitializerList(
                                matType->getElementType(),
                                value->getOperand(i)->getOperands(),
                                colCount);
                        }
                        m_writer->dedent();
                        m_writer->emit("\n}");
                        return;
                    }
                    else if (operandCount == rowCount * colCount)
                    {
                        // Handle if all are explicitly defined
                        IRType* elementType = matType->getElementType();
                        IRUse* operands = value->getOperands();

                        // Emit the braces for the Matrix struct, and the elements of each row in
                        // its own line.
                        emitType(matType);
                        m_writer->emit("{\n");
                        m_writer->indent();
                        for (Index i = 0; i < rowCount; ++i)
                        {
                            if (i != 0)
                                m_writer->emit(",\n");
                            _emitInitializerListContent(elementType, operands, colCount);
                            operands += colCount;
                        }
                        m_writer->dedent();
                        m_writer->emit("\n}");
                        return;
                    }
                }
            }

            break;
        }
    }

    // All other cases we just use the default emitting - might not work on arrays defined in global
    // scope on CUDA though
    emitOperand(value, getInfo(EmitOp::General));
}

void CUDASourceEmitter::_emitInitializerListContent(
    IRType* elementType,
    IRUse* operands,
    Index operandCount)
{
    for (Index i = 0; i < operandCount; ++i)
    {
        if (i != 0)
            m_writer->emit(", ");
        _emitInitializerListValue(elementType, operands[i].get());
    }
}


void CUDASourceEmitter::_emitInitializerList(
    IRType* elementType,
    IRUse* operands,
    Index operandCount)
{
    m_writer->emit("{\n");
    m_writer->indent();

    _emitInitializerListContent(elementType, operands, operandCount);

    m_writer->dedent();
    m_writer->emit("\n}");
}

void CUDASourceEmitter::emitIntrinsicCallExprImpl(
    IRCall* inst,
    UnownedStringSlice intrinsicDefinition,
    IRInst* intrinsicInst,
    EmitOpInfo const& inOuterPrec)
{
    // This works around the problem, where some intrinsics that require the "half" type enabled
    // don't use the half/float16_t type. For example `f16tof32` can operate on float16_t *and*
    // uint. If the input is uint, although we are using the half feature (as far as CUDA is
    // concerned), the half/float16_t type is not visible/directly used.
    if (intrinsicDefinition.startsWith(toSlice("__half")))
    {
        m_extensionTracker->requireBaseType(BaseType::Half);
    }

    Super::emitIntrinsicCallExprImpl(inst, intrinsicDefinition, intrinsicInst, inOuterPrec);
}

// The subset of PTX `sured` surface-reduction operations Slang can lower to.
// Only the ops that ptxas accepts as a `sured.b.<op>` are listed; the mnemonic
// forms the `<op>` part of the C++ helper name (`__slang_surface_reduce_<op>_<ctype>`)
// and the PTX op token, so it must match `prelude/slang-cuda-prelude.h` exactly.
enum class CUDASurfaceReduceOp
{
    Add,
    Min,
    Max,
    And,
    Or,
};

// The result of classifying a texture-texel atomic for CUDA emission. Either the
// atomic can be lowered to a `sured` (all `supported*` fields are populated) or
// it cannot, in which case `E41405` is the right response. The classifier is the
// single source of truth for both the `sured` path and the diagnostic, so they
// can never disagree about which cases are supported.
struct CUDATextureAtomicClass
{
    bool supported = false;

    // Populated only when `supported` is true:
    CUDASurfaceReduceOp op = CUDASurfaceReduceOp::Add;
    IRInst* value = nullptr;
    UnownedStringSlice ctype;      // PTX channel-type token: u32/s32/u64/s64/b32.
    Int geomDimensions = 0;        // 1, 2, or 3.
    Int64 byteXStride = 0;         // Backing-element size, for byte-addressing x.
    Int64 componentByteOffset = 0; // Channel byte offset for a vector-texel component.
};

// Map an atomic opcode to a `sured`-supported reduction op, or fail. `AtomicSub`
// / `Inc` / `Dec` are intentionally *not* mapped: they are not reachable from
// `Interlocked*` on a texture today, and `sured` has no subtract, so they take
// the unsupported path if they ever appear.
static bool _getCUDASurfaceReduceOp(IROp op, CUDASurfaceReduceOp& outOp)
{
    switch (op)
    {
    case kIROp_AtomicAdd:
        outOp = CUDASurfaceReduceOp::Add;
        return true;
    case kIROp_AtomicMin:
        outOp = CUDASurfaceReduceOp::Min;
        return true;
    case kIROp_AtomicMax:
        outOp = CUDASurfaceReduceOp::Max;
        return true;
    case kIROp_AtomicAnd:
        outOp = CUDASurfaceReduceOp::And;
        return true;
    case kIROp_AtomicOr:
        outOp = CUDASurfaceReduceOp::Or;
        return true;
    default:
        return false;
    }
}

// Return the geometry dimension count (1/2/3) of a non-array, non-multisample
// texture whose subscript backs `sured`, or 0 for a shape `sured` cannot address
// (cube, buffer, array, multisample).
static Int _getCUDASuredGeomDimensions(IRTextureTypeBase* texType)
{
    if (texType->isArray() || texType->isMultisample())
        return 0;
    switch (texType->GetBaseShape())
    {
    case SLANG_TEXTURE_1D:
        return 1;
    case SLANG_TEXTURE_2D:
        return 2;
    case SLANG_TEXTURE_3D:
        return 3;
    default:
        return 0;
    }
}

// Classify a texture-texel atomic for CUDA. `imageSubscript` is the already
// recovered `IRImageSubscript` root; `accessChain` is the sequence of component
// indices between the atomic pointer and that root (empty for a scalar texel,
// one constant index for a `RWTexture<vectorN>[coord].c` component). Returns a
// classification with `supported == false` for anything the emitter must
// diagnose instead of lowering.
//
// Backing-format handling is deliberately conservative: CUDA does not run
// `resolveTextureFormat`, so an explicit `IRFormatDecoration` is used when
// present and otherwise the texel element type is used only when it
// unambiguously matches the atomic value type. A packed / narrow / converting
// format, or a sub-32-bit channel, would make the byte offset wrong, so those
// are left unsupported rather than guessed.
static CUDATextureAtomicClass classifyTextureAtomicForCuda(
    IRInst* atomic,
    IRImageSubscript* imageSubscript,
    const List<IRInst*>& accessChain)
{
    CUDATextureAtomicClass result;

    // PTX `sured` has no result register; the reducible overloads discard the
    // prior value. If the result is observed, this is the read-modify-write
    // overload, which `sured` cannot express.
    //
    // `hasUses()` is safe by construction: a result with any remaining use is
    // rejected (E41405), so a *live* result-returning atomic can never lower to a
    // result-discarding `sured` regardless of pass ordering — the worst a missing
    // DCE pass can do is leave a dead result looking live and conservatively
    // reject a case we could otherwise have lowered. Running this after final
    // inlining/DCE is therefore about completeness (recognizing the genuinely
    // result-discarding form once dead consumers are gone), not correctness.
    if (atomic->hasUses())
        return result;

    CUDASurfaceReduceOp op;
    if (!_getCUDASurfaceReduceOp(atomic->getOp(), op))
        return result;

    // The texture type sits behind the image operand (possibly through a
    // pointer, as the Metal emitter also accounts for).
    auto imageType = imageSubscript->getImage()->getDataType();
    auto texType = as<IRTextureTypeBase>(imageType);
    if (!texType)
    {
        if (auto ptrType = as<IRPtrTypeBase>(imageType))
            texType = as<IRTextureTypeBase>(ptrType->getValueType());
    }
    if (!texType)
        return result;

    const Int geom = _getCUDASuredGeomDimensions(texType);
    if (geom == 0)
        return result;

    // The atomic value must be a scalar 32/64-bit integer.
    IRInst* value = atomic->getOperand(1);
    IRType* valueType = value->getDataType();
    bool valueIs64 = false;
    bool valueIsSigned = false;
    switch (valueType->getOp())
    {
    case kIROp_IntType:
        valueIsSigned = true;
        break;
    case kIROp_UIntType:
        break;
    case kIROp_Int64Type:
        valueIs64 = true;
        valueIsSigned = true;
        break;
    case kIROp_UInt64Type:
        valueIs64 = true;
        break;
    default:
        return result; // Non-32/64-bit-integer (float, 8/16-bit, ...).
    }

    // and/or have no 64-bit `sured` form.
    if (valueIs64 && (op == CUDASurfaceReduceOp::And || op == CUDASurfaceReduceOp::Or))
        return result;

    // The channel element type of the texel. For a scalar texel it is the
    // element type; for a vector texel it is that vector's element type.
    IRType* channelType = texType->getElementType();
    Int channelCount = 1;
    if (auto vecType = as<IRVectorType>(channelType))
    {
        channelCount = (Int)getIntVal(vecType->getElementCount());
        channelType = vecType->getElementType();
    }

    // A vector-texel component access is supported only as a single
    // *compile-time-constant* index into the vector, because the channel byte
    // offset it folds into the x coordinate must be a constant. A dynamic
    // component index (`rwTexture[coord][dynamicComp]`) is deliberately left
    // unsupported (E41405) here even though SPIR-V can express it; supporting a
    // runtime component offset is a possible future extension, but for now the
    // conservative narrowing keeps the byte math provably correct. Any deeper
    // chain would also make the offset wrong.
    Int componentIndex = 0;
    if (accessChain.getCount() > 1)
        return result;
    if (accessChain.getCount() == 1)
    {
        if (channelCount <= 1)
            return result; // Component index into a non-vector texel.
        auto indexLit = as<IRIntLit>(accessChain[0]);
        if (!indexLit)
            return result; // Non-constant (dynamic) component index.
        componentIndex = (Int)getIntVal(indexLit);
        if (componentIndex < 0 || componentIndex >= channelCount)
            return result;
    }

    // The channel must itself be a plain 32/64-bit integer matching the value
    // width, so that one channel is exactly one `sured` element.
    Int channelBytes = 0;
    bool channelMatchesValue = false;
    switch (channelType->getOp())
    {
    case kIROp_IntType:
    case kIROp_UIntType:
        channelBytes = 4;
        channelMatchesValue = !valueIs64;
        break;
    case kIROp_Int64Type:
    case kIROp_UInt64Type:
        channelBytes = 8;
        channelMatchesValue = valueIs64;
        break;
    default:
        return result; // Sub-32-bit / non-integer channel: byte math unprovable.
    }
    if (!channelMatchesValue)
        return result;

    // The x coordinate is byte-addressed, so we need the backing-element size,
    // which comes from the texture's true backing format. Prefer an explicit
    // format decoration; fall back to the element type only when it unambiguously
    // matches (no format conversion).
    //
    // Known limitation (shared with the CUDA `surf2Dread`/`surf2Dwrite` paths):
    // `findImageFormatDecoration` can only recover the format from the resource's
    // own inst or a load of a global-parameter field. When the resource reaches
    // this atomic through *indirection* — a function parameter, a call result, or
    // a resource-array element — the concrete resource's `[format(...)]` is not
    // recoverable here (CUDA does not specialize formatted resource parameters),
    // so we fall back to the element-type stride. That is correct whenever the
    // backing format is consistent with the element type (the common case,
    // including the compiler-supplied default format), and wrong only for a
    // *contradictory* declaration whose format has a different representation than
    // the element type — a different byte width and/or scalar type (e.g.
    // `[format("r32ui")] RWTexture2D<uint64_t>`) — accessed through indirection.
    // The surface read/write paths mis-stride that same declaration identically;
    // the real fix is producer-side format recovery across indirection, tracked in
    // issue #12737. This layer catches the case it *can* prove — a directly
    // recoverable format that is incompatible with the element type — in the `if`
    // branch below.
    Int64 byteXStride = 0;
    if (auto formatDecoration = findImageFormatDecoration(imageSubscript->getImage()))
    {
        // The texel element type is what we byte-address against, so the backing
        // format must have the *same representation* — same channel count and
        // scalar base type. A converting/packing format (e.g. `r32f` accessed as
        // `uint`, or `rgba8ui` accessed as `uint4`) has a different byte layout
        // than the element type implies, so the offsets we compute would be
        // wrong; those cases go to E41405 instead of a `sured`.
        // `findImageFormatDecoration` also recovers the decoration from the
        // struct field when the resource lives in a global-parameter block.
        const auto format = formatDecoration->getFormat();
        if (!isImageFormatCompatible(format, texType->getElementType()))
            return result;
        byteXStride = getImageFormatInfo(format).sizeInBytes;
    }
    else
    {
        // No recoverable format: assume the backing format matches the element
        // type used for access (the same assumption `_calcBackingElementSizeInBytes`
        // makes for surface reads/writes). See the known-limitation note above.
        byteXStride = channelCount * channelBytes;
    }

    // Choose the PTX channel-type token. `add` is sign-agnostic (use unsigned);
    // and/or are bitwise (`b32`); min/max honor signedness.
    UnownedStringSlice ctype;
    switch (op)
    {
    case CUDASurfaceReduceOp::Add:
        ctype = valueIs64 ? toSlice("u64") : toSlice("u32");
        break;
    case CUDASurfaceReduceOp::And:
    case CUDASurfaceReduceOp::Or:
        ctype = toSlice("b32");
        break;
    case CUDASurfaceReduceOp::Min:
    case CUDASurfaceReduceOp::Max:
        if (valueIs64)
            ctype = valueIsSigned ? toSlice("s64") : toSlice("u64");
        else
            ctype = valueIsSigned ? toSlice("s32") : toSlice("u32");
        break;
    }

    result.supported = true;
    result.op = op;
    result.value = value;
    result.ctype = ctype;
    result.geomDimensions = geom;
    result.byteXStride = byteXStride;
    result.componentByteOffset = (Int64)componentIndex * channelBytes;
    return result;
}

// Emit the byte-addressed x coordinate for a `sured` call: the texel x scaled by
// the backing-element size, plus the channel byte offset for a vector-texel
// component. This mirrors the `($1).x * $E` scaling the surface write path uses.
// Emit the byte-addressed x coordinate operand of a `sured` call:
// `texelX * byteXStride (+ componentByteOffset)`. The x coordinate is a 32-bit
// `int` — the `sured` PTX operand uses an "r" (32-bit) constraint, and this
// matches the `int`-coordinate convention the existing formatted surface
// read/write helpers (`sust`/`suld`) already use for their byte-addressed x. The
// byte offset therefore overflows only for an implausibly wide surface (a texel x
// past 2^31 / byteXStride, i.e. on the order of 10^8 texels even at the widest
// supported strides), well beyond any real texture width. This intentionally
// shares the surface read/write byte-x convention rather than introducing a new
// one; widening to 64-bit surface coordinates would have to change all of these
// paths together.
void CUDASourceEmitter::_emitSuredByteXCoord(const CUDATextureAtomicClass& info, IRInst* coord)
{
    m_writer->emit("(");
    // For a multi-dimensional coordinate the x component is the first element.
    if (as<IRVectorType>(coord->getDataType()))
    {
        m_writer->emit("(");
        emitOperand(coord, getInfo(EmitOp::Postfix));
        m_writer->emit(").x");
    }
    else
    {
        emitOperand(coord, getInfo(EmitOp::General));
    }
    m_writer->emit(") * ");
    m_writer->emitInt64(info.byteXStride);
    if (info.componentByteOffset != 0)
    {
        m_writer->emit(" + ");
        m_writer->emitInt64(info.componentByteOffset);
    }
}

// Emit the y (and, for 3D, z) texel coordinates of a `sured` call from a vector
// coordinate operand.
void CUDASourceEmitter::_emitSuredCoordComponent(IRInst* coord, const char* component)
{
    m_writer->emit("(");
    emitOperand(coord, getInfo(EmitOp::Postfix));
    m_writer->emit(").");
    m_writer->emit(component);
}

bool CUDASourceEmitter::tryEmitTextureAtomic(IRInst* inst)
{
    auto atomic = as<IRAtomicOperation>(inst);
    if (!atomic)
        return false;

    // Recover the `IRImageSubscript` root and the component access chain between
    // it and the atomic's pointer (e.g. the `.y` GEP for a vector-texel
    // component). A non-texel atomic (buffer / groupshared) roots at a different
    // opcode, so this returns false and the caller uses the normal path.
    List<IRInst*> accessChain;
    IRInst* root = getRootAddr(atomic->getPtr(), accessChain);
    auto imageSubscript = as<IRImageSubscript>(root);
    if (!imageSubscript)
        return false;

    auto diagnoseUnsupported = [&]()
    {
        auto loc = inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
        getSink()->diagnose(Diagnostics::AtomicOnTextureNotSupportedOnTarget{
            .target = getTargetReq()->getTarget(),
            .location = loc});
    };

    // A multisampled texture atomic is diagnosed earlier by
    // `checkUnsupportedTextureAtomic` (the multisample resource type is not
    // representable here, so it must be caught before type emission); the
    // classifier below rejects it too via `_getCUDASuredGeomDimensions`, so it is
    // never emitted regardless.
    CUDATextureAtomicClass info = classifyTextureAtomicForCuda(atomic, imageSubscript, accessChain);
    if (!info.supported)
    {
        diagnoseUnsupported();
        return true;
    }

    // The prelude helper is named `__slang_surface_reduce_<op>_<ctype>` (a
    // distinct name per channel type rather than an overload set) so that an
    // `unsigned long long` literal cannot become an ambiguous 64-bit overload;
    // see the note in `prelude/slang-cuda-prelude.h`.
    m_writer->emit("__slang_surface_reduce_");
    switch (info.op)
    {
    case CUDASurfaceReduceOp::Add:
        m_writer->emit("add");
        break;
    case CUDASurfaceReduceOp::Min:
        m_writer->emit("min");
        break;
    case CUDASurfaceReduceOp::Max:
        m_writer->emit("max");
        break;
    case CUDASurfaceReduceOp::And:
        m_writer->emit("and");
        break;
    case CUDASurfaceReduceOp::Or:
        m_writer->emit("or");
        break;
    }
    m_writer->emit("_");
    m_writer->emit(info.ctype);
    m_writer->emit("(");
    emitOperand(imageSubscript->getImage(), getInfo(EmitOp::General));
    m_writer->emit(", ");

    IRInst* coord = imageSubscript->getCoord();
    _emitSuredByteXCoord(info, coord);
    if (info.geomDimensions >= 2)
    {
        m_writer->emit(", ");
        _emitSuredCoordComponent(coord, "y");
    }
    if (info.geomDimensions >= 3)
    {
        m_writer->emit(", ");
        _emitSuredCoordComponent(coord, "z");
    }
    m_writer->emit(", ");
    emitOperand(info.value, getInfo(EmitOp::General));
    m_writer->emit(");\n");
    return true;
}

bool CUDASourceEmitter::tryEmitInstStmtImpl(IRInst* inst)
{
    // Intercept every atomic whose destination is a texture texel *before* the
    // opcode-specific emission below: CUDA has no `surfObj[coord]` l-value, so a
    // texel atomic must either lower to a `sured` surface reduction or be
    // diagnosed (E41405) here, never fall through to the invalid buffer path.
    // The classification (and its diagnosis of the unsupported complement) lives
    // at this emit point rather than a pre-emit pass because DCE / inlining run
    // in between, so only the liveness seen here is authoritative.
    if (as<IRAtomicOperation>(inst))
    {
        if (tryEmitTextureAtomic(inst))
            return true;
    }

    switch (inst->getOp())
    {
    case kIROp_StructuredBufferGetDimensions:
        {
            auto count = _generateUniqueName(UnownedStringSlice("_elementCount"));
            auto stride = _generateUniqueName(UnownedStringSlice("_stride"));

            m_writer->emit("uint ");
            m_writer->emit(count);
            m_writer->emit(";\n");
            m_writer->emit("uint ");
            m_writer->emit(stride);
            m_writer->emit(";\n");
            emitOperand(
                inst->getOperand(0),
                leftSide(getInfo(EmitOp::General), getInfo(EmitOp::Postfix)));
            m_writer->emit(".GetDimensions(&");
            m_writer->emit(count);
            m_writer->emit(", &");
            m_writer->emit(stride);
            m_writer->emit(");\n");
            emitInstResultDecl(inst);
            m_writer->emit("make_uint2(");
            m_writer->emit(count);
            m_writer->emit(", ");
            m_writer->emit(stride);
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicLoad:
        {
            emitInstResultDecl(inst);
            emitDereferenceOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(";\n");
            return true;
        }
    case kIROp_AtomicStore:
        {
            emitDereferenceOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(" = ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(";\n");
            return true;
        }
    case kIROp_AtomicExchange:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicExch(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicCompareExchange:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicCAS(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicAdd:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicAdd(");
            bool needCloseTypeCast = false;
            if (inst->getDataType()->getOp() == kIROp_Int64Type)
            {
                m_writer->emit("(unsigned long long*)(");
                needCloseTypeCast = true;
            }
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            if (needCloseTypeCast)
            {
                m_writer->emit(")");
            }
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicSub:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicAdd(");
            bool needCloseTypeCast = false;
            if (inst->getDataType()->getOp() == kIROp_Int64Type)
            {
                m_writer->emit("(unsigned long long*)(");
                needCloseTypeCast = true;
            }
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            if (needCloseTypeCast)
            {
                m_writer->emit(")");
            }
            m_writer->emit(", -(");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit("));\n");
            return true;
        }
    case kIROp_AtomicAnd:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicAnd(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicOr:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicOr(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicXor:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicXor(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicMin:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicMin(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicMax:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicMax(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_AtomicInc:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicAdd(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", 1);\n");
            return true;
        }
    case kIROp_AtomicDec:
        {
            emitInstResultDecl(inst);
            m_writer->emit("atomicAdd(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", -1);\n");
            return true;
        }
    case kIROp_CoopVecMatMulAdd:
        {
            if (!isOptixCoopVec)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation = "cooperative vector matrix multiply-add",
                    .location = inst->sourceLoc});
                _emitInstAsDefaultInitializedVar(inst, inst->getDataType());
                return true;
            }

            emitInstResultDecl(inst);
            emitInstExpr(inst, getInfo(EmitOp::General));
            m_writer->emit(";\n");
            return true;
        }
    case kIROp_CoopMatMulAdd:
        {
            emitInstResultDecl(inst);
            emitInstExpr(inst, getInfo(EmitOp::General));
            m_writer->emit(";\n");
            return true;
        }
    case kIROp_CoopVecOuterProductAccumulate:
        {
            if (!isOptixCoopVec)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation = "cooperative vector outer-product accumulate",
                    .location = inst->sourceLoc});
                m_writer->emit("/* unsupported cooperative vector outer-product accumulate */\n");
                return true;
            }

            auto outerProduct = cast<IRCoopVecOuterProductAccumulate>(inst);
            auto matrixLayout = cast<IRIntLit>(outerProduct->getMemoryLayout())->getValue();
            auto matrixInterpretation =
                cast<IRIntLit>(outerProduct->getMatrixInterpretation())->getValue();

            if (matrixLayout != SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation =
                        "cooperative vector outer-product accumulate requires TrainingOptimal "
                        "matrix layout for OptiX",
                    .location = inst->sourceLoc});
                m_writer->emit("/* unsupported cooperative vector outer-product accumulate */\n");
                return true;
            }

            if (matrixInterpretation != SLANG_SCALAR_TYPE_FLOAT16)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation =
                        "cooperative vector outer-product accumulate requires Float16 matrix "
                        "interpretation for OptiX",
                    .location = inst->sourceLoc});
                m_writer->emit("/* unsupported cooperative vector outer-product accumulate */\n");
                return true;
            }

            m_writer->emit("optixCoopVecOuterProductAccumulate(");
            emitOperand(outerProduct->getA(), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(outerProduct->getB(), getInfo(EmitOp::General));
            m_writer->emit(", (CUdeviceptr)(&(");
            emitOperand(outerProduct->getMatrixPtr(), getInfo(EmitOp::General));
            m_writer->emit(")), ");
            emitOperand(outerProduct->getMatrixOffset(), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(outerProduct->getMatrixStride(), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_CoopVecReduceSumAccumulate:
        {
            if (!isOptixCoopVec)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation = "cooperative vector reduce-sum accumulate",
                    .location = inst->sourceLoc});
                m_writer->emit("/* unsupported cooperative vector reduce-sum accumulate */\n");
                return true;
            }

            auto reduceSum = cast<IRCoopVecReduceSumAccumulate>(inst);
            auto valueType = as<IRCoopVectorType>(reduceSum->getValue()->getDataType());
            SLANG_ASSERT(valueType);
            auto valueElementType = as<IRBasicType>(valueType->getElementType());
            SLANG_ASSERT(valueElementType);
            if (valueElementType->getBaseType() != BaseType::Half &&
                valueElementType->getBaseType() != BaseType::Float)
            {
                getSink()->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                    .operation =
                        "cooperative vector reduce-sum accumulate requires Float16 or Float32 "
                        "vector element type for OptiX",
                    .location = inst->sourceLoc});
                m_writer->emit("/* unsupported cooperative vector reduce-sum accumulate */\n");
                return true;
            }

            m_writer->emit("optixCoopVecReduceSumAccumulate(");
            emitOperand(reduceSum->getValue(), getInfo(EmitOp::General));
            m_writer->emit(", (CUdeviceptr)(&(");
            emitOperand(reduceSum->getBufferPtr(), getInfo(EmitOp::General));
            m_writer->emit(")), ");
            emitOperand(reduceSum->getOffset(), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    case kIROp_SetOptiXPayloadRegister:
        {
            auto idxInst = as<IRIntLit>(inst->getOperand(0));
            IRIntegerValue idx = idxInst->getValue();
            m_writer->emit("optixSetPayload_");
            m_writer->emit(idx);
            m_writer->emit("(");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(");\n");
            return true;
        }
    default:
        return false;
    }
}

bool CUDASourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
    case kIROp_MakeVector:
    case kIROp_MakeVectorFromScalar:
        {
            m_writer->emit("make_");
            emitType(inst->getDataType());
            m_writer->emit("(");
            bool isFirst = true;
            char xyzwNames[] = "xyzw";
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto arg = inst->getOperand(i);
                if (auto vectorType = as<IRVectorType>(arg->getDataType()))
                {
                    for (int j = 0; j < cast<IRIntLit>(vectorType->getElementCount())->getValue();
                         j++)
                    {
                        if (isFirst)
                            isFirst = false;
                        else
                            m_writer->emit(", ");
                        auto outerPrec = getInfo(EmitOp::General);
                        auto prec = getInfo(EmitOp::Postfix);
                        emitOperand(arg, leftSide(outerPrec, prec));
                        m_writer->emit(".");
                        m_writer->emitChar(xyzwNames[j]);
                    }
                }
                else
                {
                    if (isFirst)
                        isFirst = false;
                    else
                        m_writer->emit(", ");
                    emitOperand(arg, getInfo(EmitOp::General));
                }
            }
            m_writer->emit(")");
            return true;
        }
    case kIROp_FloatCast:
    case kIROp_CastIntToFloat:
    case kIROp_IntCast:
    case kIROp_CastFloatToInt:
        {
            if (auto dstVectorType = as<IRVectorType>(inst->getDataType()))
            {
                m_writer->emit("make_");
                emitType(inst->getDataType());
                m_writer->emit("(");
                bool isFirst = true;
                char xyzwNames[] = "xyzw";
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    auto arg = inst->getOperand(i);
                    if (auto vectorType = as<IRVectorType>(arg->getDataType()))
                    {
                        for (int j = 0;
                             j < cast<IRIntLit>(vectorType->getElementCount())->getValue();
                             j++)
                        {
                            if (isFirst)
                                isFirst = false;
                            else
                                m_writer->emit(", ");
                            m_writer->emit("(");
                            emitType(dstVectorType->getElementType());
                            m_writer->emit(")");
                            auto outerPrec = getInfo(EmitOp::General);
                            auto prec = getInfo(EmitOp::Postfix);
                            emitOperand(arg, leftSide(outerPrec, prec));
                            m_writer->emit(".");
                            m_writer->emitChar(xyzwNames[j]);
                        }
                    }
                    else
                    {
                        if (isFirst)
                            isFirst = false;
                        else
                            m_writer->emit(", ");
                        m_writer->emit("(");
                        emitType(dstVectorType->getElementType());
                        m_writer->emit(")");
                        emitOperand(arg, getInfo(EmitOp::General));
                    }
                }
                m_writer->emit(")");
                return true;
            }
            else if (const auto matrixType = as<IRMatrixType>(inst->getDataType()); matrixType)
            {
                m_writer->emit("make");
                emitType(inst->getDataType());
                m_writer->emit("(");
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    auto arg = inst->getOperand(i);
                    if (i > 0)
                        m_writer->emit(", ");
                    emitOperand(arg, getInfo(EmitOp::General));
                }
                m_writer->emit(")");
                return true;
            }
            return false;
        }
    case kIROp_MakeMatrix:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_MatrixReshape:
        {
            m_writer->emit("make");
            emitType(inst->getDataType());
            m_writer->emit("(");
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto arg = inst->getOperand(i);
                if (i > 0)
                    m_writer->emit(", ");
                emitOperand(arg, getInfo(EmitOp::General));
            }
            m_writer->emit(")");
            return true;
        }
    case kIROp_MakeCoopMatrixFromScalar:
        {
            StringBuilder typeSB;
            emitWMMAFragmentType(as<IRCoopMatrixType>(inst->getDataType()), typeSB);
            m_writer->emit(typeSB);
            m_writer->emit("(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
    case kIROp_CoopMatMulAdd:
        {
            auto coopMatMulAdd = cast<IRCoopMatMulAdd>(inst);
            auto matA = coopMatMulAdd->getMatA();
            auto matB = coopMatMulAdd->getMatB();
            auto matC = coopMatMulAdd->getMatC();
            auto saturatingAccumulation =
                cast<IRBoolLit>(coopMatMulAdd->getSaturatingAccumulation())->getValue();

            auto aElemType = cast<IRCoopMatrixType>(matA->getDataType())->getElementType();
            auto bElemType = cast<IRCoopMatrixType>(matB->getDataType())->getElementType();
            auto cElemType = cast<IRCoopMatrixType>(matC->getDataType())->getElementType();
            auto dElemType = cast<IRCoopMatrixType>(coopMatMulAdd->getDataType())->getElementType();
            if (!coopMatMulAddTypeCombinationIsValid(
                    aElemType->getOp(),
                    bElemType->getOp(),
                    cElemType->getOp(),
                    dElemType->getOp()))
            {
                auto formatElem = [&](IRType* type) -> String
                {
                    StringBuilder sb;
                    calcTypeName(type, CodeGenTarget::CUDASource, sb);
                    return sb.toString();
                };
                getSink()->diagnose(Diagnostics::CooperativeMatrixInvalidMmaTypeCombination{
                    .aType = formatElem(aElemType),
                    .bType = formatElem(bElemType),
                    .cType = formatElem(cElemType),
                    .dType = formatElem(dElemType),
                    .location = inst->sourceLoc});
                // The DiagnosticSink has already recorded the error, but the
                // surrounding statement-emit path expects an expression to
                // follow `Type _Sname = ` (otherwise we'd emit the syntactically
                // invalid `Type _Sname = ;`).  Emit a default-constructed
                // value of the result type as a placeholder; the recorded
                // error makes the overall compile fail anyway, so the
                // placeholder never reaches NVRTC.
                m_writer->emit("(");
                emitType(inst->getDataType());
                m_writer->emit("{})");
                return true;
            }

            m_writer->emit("Slang_CUDA_WMMA::coopMatMulAdd<");
            emitType(matA->getDataType());
            m_writer->emit("::ElementType, ");
            emitType(matB->getDataType());
            m_writer->emit("::ElementType, ");
            emitType(matC->getDataType());
            m_writer->emit("::ElementType, ");
            emitType(coopMatMulAdd->getDataType());
            m_writer->emit("::ElementType, ");
            emitType(matA->getDataType());
            m_writer->emit("::m_M, ");
            emitType(matA->getDataType());
            m_writer->emit("::m_N, ");
            emitType(matA->getDataType());
            m_writer->emit("::m_K, ");
            m_writer->emit(saturatingAccumulation ? "true" : "false");
            m_writer->emit(">(");
            emitOperand(matA, getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(matB, getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(matC, getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
    case kIROp_CoopVecMatMulAdd:
        {
            // CoopVec matmul ops are always emitted as statements, so non-OptiX handling lives in
            // tryEmitInstStmtImpl().
            SLANG_ASSERT(isOptixCoopVec);

            auto coopVecMatMulAdd = cast<IRCoopVecMatMulAdd>(inst);
            auto inputInterpretationPackingFactor =
                cast<IRIntLit>(coopVecMatMulAdd->getInputInterpretationPackingFactor())->getValue();
            auto inputInterpretation =
                cast<IRIntLit>(coopVecMatMulAdd->getInputInterpretation())->getValue();
            auto matrixInterpretation =
                cast<IRIntLit>(coopVecMatMulAdd->getMatrixInterpretation())->getValue();
            auto biasInterpretation = coopVecMatMulAdd->getBiasInterpretation();
            const bool hasBias = biasInterpretation != nullptr;

            if (inputInterpretationPackingFactor != 1)
            {
                emitUnsupportedTargetIntrinsicExpr(
                    this,
                    inst,
                    "cooperative vector matrix multiply-add with packed input is not implemented "
                    "yet",
                    inst->sourceLoc);
                return true;
            }

            auto inputInterpretationName =
                getOptixCoopVecComponentTypeName((uint32_t)inputInterpretation);
            if (!inputInterpretationName.getLength())
            {
                emitUnsupportedTargetIntrinsicExpr(
                    this,
                    inst,
                    "cooperative vector matrix multiply-add with unsupported OptiX input "
                    "interpretation type",
                    inst->sourceLoc);
                return true;
            }

            auto matrixInterpretationName =
                getOptixCoopVecComponentTypeName((uint32_t)matrixInterpretation);
            if (!matrixInterpretationName.getLength())
            {
                emitUnsupportedTargetIntrinsicExpr(
                    this,
                    inst,
                    "cooperative vector matrix multiply-add with unsupported OptiX matrix "
                    "interpretation type",
                    inst->sourceLoc);
                return true;
            }

            auto matrixLayout = cast<IRIntLit>(coopVecMatMulAdd->getMemoryLayout())->getValue();
            auto matrixLayoutName = getOptixCoopVecMatrixLayoutName((uint32_t)matrixLayout);

            auto transposeValue = cast<IRBoolLit>(coopVecMatMulAdd->getTranspose())->getValue();
            if (transposeValue)
            {
                if (matrixInterpretation != SLANG_SCALAR_TYPE_FLOAT16 ||
                    (matrixLayout != SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL &&
                     matrixLayout != SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL))
                {
                    emitUnsupportedTargetIntrinsicExpr(
                        this,
                        inst,
                        "cooperative vector matrix multiply-add with transpose requires Float16 "
                        "matrix interpretation and InferencingOptimal or TrainingOptimal matrix "
                        "layout for OptiX",
                        inst->sourceLoc);
                    return true;
                }
            }

            UnownedStringSlice biasInterpretationName;
            if (hasBias)
            {
                biasInterpretationName = getOptixCoopVecComponentTypeName(
                    (uint32_t)cast<IRIntLit>(biasInterpretation)->getValue());
                if (!biasInterpretationName.getLength())
                {
                    emitUnsupportedTargetIntrinsicExpr(
                        this,
                        inst,
                        "cooperative vector matrix multiply-add with unsupported OptiX bias "
                        "interpretation type",
                        inst->sourceLoc);
                    return true;
                }
            }

            m_writer->emit("(");
            m_writer->emit("slangOptixCoopVecMatMul<");
            emitType(inst->getDataType());
            m_writer->emit(", ");
            emitType(coopVecMatMulAdd->getInput()->getDataType());
            m_writer->emit(", ");
            m_writer->emit(inputInterpretationName);
            m_writer->emit(", ");
            m_writer->emit(matrixInterpretationName);
            m_writer->emit(", ");
            m_writer->emit(matrixLayoutName);
            if (hasBias)
            {
                m_writer->emit(", ");
                m_writer->emit(biasInterpretationName);
            }
            m_writer->emit(">((");
            emitOperand(coopVecMatMulAdd->getInput(), getInfo(EmitOp::General));
            m_writer->emit("), (CUdeviceptr)(&((");
            emitOperand(coopVecMatMulAdd->getMatrixPtr(), getInfo(EmitOp::General));
            m_writer->emit("))), ");
            emitOperand(coopVecMatMulAdd->getMatrixOffset(), getInfo(EmitOp::General));
            if (hasBias)
            {
                m_writer->emit(", (CUdeviceptr)(&((");
                emitOperand(coopVecMatMulAdd->getBiasPtr(), getInfo(EmitOp::General));
                m_writer->emit("))), ");
                emitOperand(coopVecMatMulAdd->getBiasOffset(), getInfo(EmitOp::General));
            }
            else if (
                as<IRHLSLStructuredBufferTypeBase>(
                    coopVecMatMulAdd->getMatrixPtr()->getDataType()) == nullptr)
            {
                m_writer->emit(", ");
                emitOperand(coopVecMatMulAdd->getTranspose(), getInfo(EmitOp::General));
            }
            m_writer->emit(", ");
            emitOperand(coopVecMatMulAdd->getMatrixStride(), getInfo(EmitOp::General));
            m_writer->emit("))");
            return true;
        }
    case kIROp_MakeArray:
        {
            IRType* dataType = inst->getDataType();
            IRArrayType* arrayType = as<IRArrayType>(dataType);

            IRType* elementType = arrayType->getElementType();

            // Emit braces for the FixedArray struct.
            m_writer->emit("{ ");
            _emitInitializerList(elementType, inst->getOperands(), Index(inst->getOperandCount()));
            m_writer->emit(" }");

            return true;
        }
    case kIROp_WaveMaskBallot:
        {
            m_extensionTracker->requireSMVersion(SemanticVersion(7, 0));

            m_writer->emit("__ballot_sync(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
    case kIROp_WaveMaskMatch:
        {
            m_extensionTracker->requireSMVersion(SemanticVersion(7, 0));

            m_writer->emit("__match_any_sync(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(")");
            return true;
        }
    case kIROp_GetOptiXRayPayloadPtr:
        {
            m_writer->emit("((");
            emitType(inst->getDataType());
            m_writer->emit(")getOptiXRayPayloadPtr())");
            return true;
        }
    case kIROp_GetOptiXHitAttribute:
        {
            auto typeToFetch = inst->getOperand(0);
            auto idxInst = as<IRIntLit>(inst->getOperand(1));
            IRIntegerValue idx = idxInst->getValue();
            if (typeToFetch->getOp() == kIROp_FloatType)
            {
                m_writer->emit("__int_as_float(optixGetAttribute_");
            }
            else
            {
                m_writer->emit("optixGetAttribute_");
            }
            m_writer->emit(idx);
            if (typeToFetch->getOp() == kIROp_FloatType)
            {
                m_writer->emit("())");
            }
            else
            {
                m_writer->emit("()");
            }
            return true;
        }
    case kIROp_GetOptiXSbtDataPtr:
        {
            m_writer->emit("((");
            emitType(inst->getDataType());
            m_writer->emit(")optixGetSbtDataPointer())");
            return true;
        }
    case kIROp_GetOptiXPayloadRegister:
        {
            auto idxInst = as<IRIntLit>(inst->getOperand(0));
            IRIntegerValue idx = idxInst->getValue();
            m_writer->emit("optixGetPayload_");
            m_writer->emit(idx);
            m_writer->emit("()");
            return true;
        }
    case kIROp_DispatchKernel:
        {
            auto dispatchInst = as<IRDispatchKernel>(inst);
            emitOperand(dispatchInst->getBaseFn(), getInfo(EmitOp::Atomic));
            m_writer->emit("<<<");
            emitOperand(dispatchInst->getThreadGroupSize(), getInfo(EmitOp::General));
            m_writer->emit(", ");
            emitOperand(dispatchInst->getDispatchSize(), getInfo(EmitOp::General));
            m_writer->emit(">>>(");
            for (UInt i = 0; i < dispatchInst->getArgCount(); i++)
            {
                if (i > 0)
                    m_writer->emit(", ");
                emitOperand(dispatchInst->getArg(i), getInfo(EmitOp::General));
            }
            m_writer->emit(")");
            return true;
        }
    case kIROp_CUDALDG:
        {
            m_writer->emit("__ldg(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(")");
        }
        return true;
    case kIROp_GetStructuredBufferPtr:
    case kIROp_GetUntypedBufferPtr:
        {
            m_writer->emit("(&(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit(").data)");
            return true;
        }
    default:
        break;
    }

    return Super::tryEmitInstExprImpl(inst, inOuterPrec);
}

void CUDASourceEmitter::handleRequiredCapabilitiesImpl(IRInst* inst)
{
    // Does this function declare any requirements on CUDA capabilities
    // that should affect output?

    for (auto decoration : inst->getDecorations())
    {
        if (auto smDecoration = as<IRRequireCUDASMVersionDecoration>(decoration))
        {
            SemanticVersion version = smDecoration->getCUDASMVersion();
            m_extensionTracker->requireSMVersion(version);
        }
    }
}

void CUDASourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    m_writer->emit(getVectorPrefix(elementType->getOp()));
    m_writer->emit(elementCount);
}

void CUDASourceEmitter::_emitType(IRType* type, DeclaratorInfo* declarator)
{
    // Handle Ptr<T[]> on CUDA, we shouldn't emit it as Array<T>*.
    // Instead, we should emit it as T**.
    // e.g. Array<T>* a;    a[1] == a + sizeof(Array<T>)
    // but T** b;    b[1] == b + sizeof(T*)
    if (type->getOp() == kIROp_PtrType || type->getOp() == kIROp_NativePtrType)
    {
        auto ptrType = cast<IRPtrTypeBase>(type);
        if (auto unsizedArrayType = as<IRUnsizedArrayType>(ptrType->getValueType()))
        {
            PtrDeclaratorInfo outerPtr(declarator);
            PtrDeclaratorInfo innerPtr(&outerPtr);
            _emitType(unsizedArrayType->getElementType(), &innerPtr);
            return;
        }
    }
    Super::_emitType(type, declarator);
}

void CUDASourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_VectorType:
        {
            auto vectorType = as<IRVectorType>(type);
            m_writer->emit(getVectorPrefix(vectorType->getElementType()->getOp()));
            m_writer->emit(as<IRIntLit>(vectorType->getElementCount())->getValue());
            break;
        }
    default:
        m_writer->emit(_getTypeName(type));
        break;
    }
}

void CUDASourceEmitter::emitRateQualifiersAndAddressSpaceImpl(
    IRRate* rate,
    [[maybe_unused]] AddressSpace addressSpace)
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

void CUDASourceEmitter::emitSemanticsImpl(IRInst* inst, bool allowOffsetLayout)
{
    Super::emitSemanticsImpl(inst, allowOffsetLayout);
}

void CUDASourceEmitter::emitInterpolationModifiersImpl(
    IRInst* varInst,
    IRType* valueType,
    IRVarLayout* layout)
{
    Super::emitInterpolationModifiersImpl(varInst, valueType, layout);
}

void CUDASourceEmitter::emitVarDecorationsImpl(IRInst* varDecl)
{
    Super::emitVarDecorationsImpl(varDecl);
}

void CUDASourceEmitter::emitMatrixLayoutModifiersImpl(IRType* varType)
{
    Super::emitMatrixLayoutModifiersImpl(varType);
}

bool CUDASourceEmitter::tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType)
{
    // A global shader parameter in the IR for CUDA output will
    // either be the unique constant buffer that wraps all the
    // global-scope parameters in the original code (which is
    // handled as a special-case before this routine would be
    // called), or it is one of the system-defined varying inputs
    // like `threadIdx`. We won't need to emit anything in the
    // output code for the latter case, so we need to emit
    // nothing here and return `true` so that the base class
    // uses our logic instead of the default.
    //
    SLANG_UNUSED(varDecl);
    SLANG_UNUSED(varType);
    return true;
}


void CUDASourceEmitter::emitModuleImpl(IRModule* module, DiagnosticSink* sink)
{
    // Set up with all of the base types used in the module
    m_extensionTracker->requireBaseTypes(_findBaseTypesUsed(module));

    CLikeSourceEmitter::emitModuleImpl(module, sink);

    // Emit all witness table definitions.
    _emitWitnessTableDefinitions();
}

static bool typeCheck(IROp op, uint32_t matrixUse)
{
    switch (matrixUse)
    {
    case SLANG_COOPERATIVE_MATRIX_USE_A:
    case SLANG_COOPERATIVE_MATRIX_USE_B:
        // PTX m16n8k16 supports f16, bf16, 8-bit integer (s8 / u8), and 8-bit
        // float (e4m3 / e5m2) inputs.
        return op == kIROp_HalfType || op == kIROp_BFloat16Type || op == kIROp_Int8Type ||
               op == kIROp_UInt8Type || op == kIROp_FloatE4M3Type || op == kIROp_FloatE5M2Type;
    case SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR:
        // Union of the legal accumulator element types across all
        // currently-supported A/B element types: half/float (for f16, bf16, f8
        // inputs) and int (for s8 / u8 inputs).  The full A/B/C/D combination
        // is checked separately by `coopMatMulAddTypeCombinationIsValid`
        // before code emission.
        return op == kIROp_HalfType || op == kIROp_FloatType || op == kIROp_IntType;
    }
    return false;
}

// Validate that a `coopMatMulAdd` (A * B + C -> D) is one of the legal
// (AType, BType, CType, DType) tuples for the CUDA backend.  The helper
// templates in `prelude/slang-cuda-prelude.h` only have specializations for
// these tuples; without this check, an illegal combination would compile
// through Slang and only fail later inside NVRTC with a hard-to-read C++
// template error.
static bool coopMatMulAddTypeCombinationIsValid(IROp aType, IROp bType, IROp cType, IROp dType)
{
    // Both A and B must share the same element type — every supported
    // CUDA mma form has matching `.atype` and `.btype`.
    if (aType != bType)
        return false;

    auto isHalfOrFloat = [](IROp t) { return t == kIROp_HalfType || t == kIROp_FloatType; };
    auto isFloat = [](IROp t) { return t == kIROp_FloatType; };
    auto isInt32 = [](IROp t) { return t == kIROp_IntType; };

    switch (aType)
    {
    case kIROp_HalfType:
        // f16 mma supports both f16 and f32 accumulator/output, and CType
        // and DType may be picked independently from {half, float}.
        return isHalfOrFloat(cType) && isHalfOrFloat(dType);
    case kIROp_BFloat16Type:
        // bf16 mma only allows an f32 accumulator and output on PTX.
        return isFloat(cType) && isFloat(dType);
    case kIROp_Int8Type:
    case kIROp_UInt8Type:
        // Integer mma only allows an s32 accumulator and output.
        return isInt32(cType) && isInt32(dType);
    case kIROp_FloatE4M3Type:
    case kIROp_FloatE5M2Type:
        // fp8 mma supports half or float accumulator/output; the prelude only
        // provides specializations where CType == DType for these.
        return (cType == kIROp_HalfType && dType == kIROp_HalfType) ||
               (cType == kIROp_FloatType && dType == kIROp_FloatType);
    default:
        return false;
    }
}

static UnownedStringSlice getMatrixUseName(uint32_t matrixUse)
{
    switch (matrixUse)
    {
    case SLANG_COOPERATIVE_MATRIX_USE_A:
        return UnownedStringSlice("Slang_CUDA_WMMA::MatrixA");
    case SLANG_COOPERATIVE_MATRIX_USE_B:
        return UnownedStringSlice("Slang_CUDA_WMMA::MatrixB");
    case SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR:
        return UnownedStringSlice("Slang_CUDA_WMMA::MatrixC");
    default:
        SLANG_UNEXPECTED("invalid cooperative matrix use");
    }
}

/*
 * Shape Validation Strategy:
 * Maps CoopMat dimensions to the canonical MMA shape (m, n, k).
 * Only m16n16k16 is supported (internally uses 2x mma.sync.m16n8k16).
 *
 * Supported shapes:
 *   - m16n16k16: Matrix A (16x16), Matrix B (16x16), Matrix C/D (16x16)
 */
inline FragmentShape computeShapeCombination(uint32_t /*matrixUse*/, uint32_t row, uint32_t col)
{
    if (row == 16 && col == 16)
        return {16, 16, 16};
    return {0, 0, 0};
}

SlangResult CUDASourceEmitter::emitWMMAFragmentType(
    IRCoopMatrixType* coopMatType,
    StringBuilder& outStr)
{
    uint32_t rowCount = (uint32_t) static_cast<IRIntLit*>(coopMatType->getRowCount())->getValue();
    uint32_t colCount =
        (uint32_t) static_cast<IRIntLit*>(coopMatType->getColumnCount())->getValue();
    uint32_t matrixUse = (uint32_t) static_cast<IRIntLit*>(coopMatType->getMatrixUse())->getValue();

    auto elementType = coopMatType->getElementType();
    StringBuilder elementTypeSB;
    calcTypeName(elementType, CodeGenTarget::CUDASource, elementTypeSB);
    auto typeName = elementTypeSB.toString();

    // TODO: We should add a pass in IR to validate the coop matrix types, such that
    // we can provide better diagnostic messages here.
    if (!typeCheck(elementType->getOp(), matrixUse))
    {
        getSink()->diagnose(Diagnostics::CooperativeMatrixUnsupportedElementType{
            .elementType = typeName,
            .matrixUse = matrixUse == SLANG_COOPERATIVE_MATRIX_USE_A
                             ? "A"
                             : (matrixUse == SLANG_COOPERATIVE_MATRIX_USE_B ? "B" : "C")});
        SLANG_RELEASE_ASSERT(false);
        return SLANG_FAIL;
    }

    outStr << "Slang_CUDA_WMMA::WmmaFragment<";

    FragmentShape shape = computeShapeCombination(matrixUse, rowCount, colCount);
    if (!shape.isValid())
    {
        getSink()->diagnose(Diagnostics::CooperativeMatrixInvalidShape{
            .rowCount = String(rowCount),
            .colCount = String(colCount),
            .matrixUse = matrixUse == SLANG_COOPERATIVE_MATRIX_USE_A
                             ? "A"
                             : (matrixUse == SLANG_COOPERATIVE_MATRIX_USE_B ? "B" : "C")});
        SLANG_RELEASE_ASSERT(false);
        return SLANG_FAIL;
    }

    outStr << typeName << "," << shape.m << ", " << shape.n << ", " << shape.k << ", "
           << getMatrixUseName(matrixUse) << ">";

    return SLANG_OK;
}

} // namespace Slang
