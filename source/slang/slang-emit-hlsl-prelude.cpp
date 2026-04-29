// slang-emit-hlsl-prelude.cpp
#include "slang-emit-hlsl.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

const char* HLSLSourceEmitter::m_BuiltinPrelude64BitCast = R"(
uint64_t _slang_asuint64(double x)
{
    uint32_t low;
    uint32_t high;
    asuint(x, low, high);
    return ((uint64_t)high << 32) | low;
}

double _slang_asdouble(uint64_t x)
{
    uint32_t low = x & 0xFFFFFFFF;
    uint32_t high = x >> 32;
    return asdouble(low, high);
}
)";

const char* HLSLSourceEmitter::m_CoopVecPrelude_sm610 = R"(
template<
    typename OutElTy,
    dx::linalg::ComponentEnum MatDT,
    uint MatM,
    uint MatK,
    dx::linalg::MatrixLayoutEnum LoadLayout,
    dx::linalg::ComponentEnum InputDT,
    uint InputVecDim,
    typename BufTy,
    typename InElTy>
vector<OutElTy, MatM> __slang_linalg_Mul(
    BufTy matBuf,
    uint matOff,
    uint matStr,
    vector<InElTy, InputVecDim> input)
{
    using MatTy = dx::linalg::Matrix<
        MatDT,
        MatM,
        MatK,
        dx::linalg::MatrixUse::A,
        dx::linalg::MatrixScope::Thread>;
    MatTy mat = MatTy::template Load<LoadLayout>(matBuf, matOff, matStr);
    return dx::linalg::Multiply<OutElTy>(
        mat,
        dx::linalg::MakeInterpretedVector<InputDT>(input));
}

template<
    typename OutElTy,
    dx::linalg::ComponentEnum MatDT,
    uint MatM,
    uint MatK,
    dx::linalg::MatrixLayoutEnum LoadLayout,
    dx::linalg::ComponentEnum InputDT,
    uint InputVecDim,
    typename BiasElTy,
    uint BiasVecDim,
    typename MatBufTy,
    typename BiasBufTy,
    typename InElTy>
vector<OutElTy, MatM> __slang_linalg_MulAdd(
    MatBufTy matBuf,
    uint matOff,
    uint matStr,
    BiasBufTy biasBuf,
    uint biasOff,
    vector<InElTy, InputVecDim> input)
{
    using MatTy = dx::linalg::Matrix<
        MatDT,
        MatM,
        MatK,
        dx::linalg::MatrixUse::A,
        dx::linalg::MatrixScope::Thread>;
    MatTy mat = MatTy::template Load<LoadLayout>(matBuf, matOff, matStr);
    using BiasVecTy = vector<BiasElTy, BiasVecDim>;
    BiasVecTy biasVec = biasBuf.template Load<BiasVecTy>(biasOff);
    return dx::linalg::MultiplyAdd<OutElTy>(
        mat,
        dx::linalg::MakeInterpretedVector<InputDT>(input),
        biasVec);
}

template<
    typename ElTy,
    dx::linalg::ComponentEnum MatDT,
    uint MatM,
    uint MatN,
    dx::linalg::MatrixLayoutEnum /*IgnoredMemoryLayout*/,
    typename BufTy>
void __slang_linalg_OuterProductAccumulate(
    vector<ElTy, MatM> a,
    vector<ElTy, MatN> b,
    BufTy matBuf,
    uint matOff,
    uint /*IgnoredMatrixStride*/)
{
    using AccTy = dx::linalg::Matrix<
        MatDT,
        MatM,
        MatN,
        dx::linalg::MatrixUse::Accumulator,
        dx::linalg::MatrixScope::Thread>;
    AccTy acc = dx::linalg::OuterProduct<MatDT>(a, b);
    acc.InterlockedAccumulate(matBuf, matOff);
}

template<typename ElTy, uint N, typename BufTy>
void __slang_linalg_VectorAccumulate(vector<ElTy, N> inputVec, BufTy buffer, uint offset)
{
    // No dx::linalg wrapper for cooperative-vector reduce-sum yet; approximate with per-lane load/add/store
    for (uint i = 0; i < N; ++i)
    {
        uint byteOffset = offset + i * uint(sizeof(ElTy));
        ElTy cur = buffer.template Load<ElTy>(byteOffset);
        buffer.template Store<ElTy>(byteOffset, cur + inputVec[i]);
    }
}
)";

const char* HLSLSourceEmitter::m_CoopVecPrelude_sm609 = R"(
template<
    typename OutElTy,
    dx::linalg::DataType MatDT,
    uint MatM,
    uint MatK,
    dx::linalg::MatrixLayout MatML,
    bool MatTranspose,
    dx::linalg::DataType InputDT,
    uint InputVecDim,
    typename BufTy,
    typename InElTy,
    bool OutputIsUnsigned,
    bool InputIsUnsigned>
vector<OutElTy, MatM> __slang_linalg_Mul(
    BufTy matBuf,
    uint matOff,
    uint matStr,
    vector<InElTy, InputVecDim> input)
{
    vector<OutElTy, MatM> __slang_r = (vector<OutElTy, MatM>)0;
    __builtin_MatVecMul(__slang_r, OutputIsUnsigned, input, InputIsUnsigned, InputDT, matBuf, matOff,
        MatDT, MatM, MatK, MatML, MatTranspose, matStr);
    return __slang_r;
}

template<
    typename OutElTy,
    dx::linalg::DataType MatDT,
    uint MatM,
    uint MatK,
    dx::linalg::MatrixLayout MatML,
    bool MatTranspose,
    dx::linalg::DataType InputDT,
    uint InputVecDim,
    dx::linalg::DataType BiasDT,
    typename MatBufTy,
    typename BiasBufTy,
    typename InElTy,
    bool OutputIsUnsigned,
    bool InputIsUnsigned>
vector<OutElTy, MatM> __slang_linalg_MulAdd(
    MatBufTy matBuf,
    uint matOff,
    uint matStr,
    BiasBufTy biasBuf,
    uint biasOff,
    vector<InElTy, InputVecDim> input)
{
    vector<OutElTy, MatM> __slang_r = (vector<OutElTy, MatM>)0;
    __builtin_MatVecMulAdd(__slang_r, OutputIsUnsigned, input, InputIsUnsigned, InputDT, matBuf,
        matOff, MatDT, MatM, MatK, MatML, MatTranspose, matStr, biasBuf, biasOff, BiasDT);
    return __slang_r;
}

template<typename ElTy, dx::linalg::DataType MatDT, uint MatM, uint MatN, dx::linalg::MatrixLayout MatML, typename BufTy>
void __slang_linalg_OuterProductAccumulate(
    vector<ElTy, MatM> a,
    vector<ElTy, MatN> b,
    BufTy matBuf,
    uint matOff,
    uint matStr)
{
    __builtin_OuterProductAccumulate(a, b, matBuf, matOff, MatDT, MatML, matStr);
}

template<typename ElTy, uint N, typename BufTy>
void __slang_linalg_VectorAccumulate(vector<ElTy, N> inputVec, BufTy buffer, uint offset)
{
    __builtin_VectorAccumulate(inputVec, buffer, offset);
}
)";

const char* HLSLSourceEmitter::m_CoopMatPrelude = R"(
template<
    dx::linalg::ComponentEnum CT,
    int M,
    int N,
    dx::linalg::MatrixUseEnum U,
    dx::linalg::MatrixScopeEnum S>
dx::linalg::Matrix<CT, M, N, U, S> __slang_cm_add(
    dx::linalg::Matrix<CT, M, N, U, S> a,
    dx::linalg::Matrix<CT, M, N, U, S> b)
{
    dx::linalg::Matrix<CT, M, N, U, S> c;
    int len = a.Length();
    for (int i = 0; i < len; i++)
        c.Set(i, a.Get(i) + b.Get(i));
    return c;
}

template<
    dx::linalg::ComponentEnum CT,
    int M,
    int N,
    dx::linalg::MatrixUseEnum U,
    dx::linalg::MatrixScopeEnum S>
dx::linalg::Matrix<CT, M, N, U, S> __slang_cm_sub(
    dx::linalg::Matrix<CT, M, N, U, S> a,
    dx::linalg::Matrix<CT, M, N, U, S> b)
{
    dx::linalg::Matrix<CT, M, N, U, S> c;
    int len = a.Length();
    for (int i = 0; i < len; i++)
        c.Set(i, a.Get(i) - b.Get(i));
    return c;
}

template<
    dx::linalg::ComponentEnum CT,
    int M,
    int N,
    dx::linalg::MatrixUseEnum U,
    dx::linalg::MatrixScopeEnum S>
dx::linalg::Matrix<CT, M, N, U, S> __slang_cm_mul(
    dx::linalg::Matrix<CT, M, N, U, S> a,
    dx::linalg::Matrix<CT, M, N, U, S> b)
{
    dx::linalg::Matrix<CT, M, N, U, S> c;
    int len = a.Length();
    for (int i = 0; i < len; i++)
        c.Set(i, a.Get(i) * b.Get(i));
    return c;
}

template<
    dx::linalg::ComponentEnum CT,
    int M,
    int N,
    dx::linalg::MatrixUseEnum U,
    dx::linalg::MatrixScopeEnum S>
dx::linalg::Matrix<CT, M, N, U, S> __slang_cm_div(
    dx::linalg::Matrix<CT, M, N, U, S> a,
    dx::linalg::Matrix<CT, M, N, U, S> b)
{
    dx::linalg::Matrix<CT, M, N, U, S> c;
    int len = a.Length();
    for (int i = 0; i < len; i++)
        c.Set(i, a.Get(i) / b.Get(i));
    return c;
}

template<
    dx::linalg::ComponentEnum CT,
    int M,
    int N,
    dx::linalg::MatrixUseEnum U,
    dx::linalg::MatrixScopeEnum S>
dx::linalg::Matrix<CT, M, N, U, S> __slang_cm_neg(dx::linalg::Matrix<CT, M, N, U, S> a)
{
    dx::linalg::Matrix<CT, M, N, U, S> c;
    int len = a.Length();
    for (int i = 0; i < len; i++)
        c.Set(i, -a.Get(i));
    return c;
}

// dx::linalg::Matrix::MultiplyAccumulate() is a void mutating method, but kIROp_CoopMatMulAdd
// is a value-producing IR instruction whose result may be consumed by subsequent instructions.
// This wrapper adapts the void mutation into a value-returning function so the emitter can
// treat the operation as a single expression.
template<
    dx::linalg::ComponentEnum CT_A,
    dx::linalg::ComponentEnum CT_B,
    dx::linalg::ComponentEnum CT_D,
    int RM,
    int RK,
    int RN,
    dx::linalg::MatrixScopeEnum RS>
dx::linalg::Matrix<CT_D, RM, RN, dx::linalg::MatrixUse::Accumulator, RS>
__slang_cm_muladd(
    dx::linalg::Matrix<CT_A, RM, RK, dx::linalg::MatrixUse::A, RS> a,
    dx::linalg::Matrix<CT_B, RK, RN, dx::linalg::MatrixUse::B, RS> b,
    dx::linalg::Matrix<CT_D, RM, RN, dx::linalg::MatrixUse::Accumulator, RS> c)
{
    c.MultiplyAccumulate(a, b);
    return c;
}
)";

/* static */ const char* HLSLSourceEmitter::getCoopMatComponentTypeName(
    IROp elementTypeOp,
    DiagnosticSink* sink,
    SourceLoc loc)
{
    switch (elementTypeOp)
    {
    case kIROp_Int8Type:
        return "dx::linalg::ComponentType::I8";
    case kIROp_UInt8Type:
        return "dx::linalg::ComponentType::U8";
    case kIROp_Int16Type:
        return "dx::linalg::ComponentType::I16";
    case kIROp_UInt16Type:
        return "dx::linalg::ComponentType::U16";
    case kIROp_IntType:
        return "dx::linalg::ComponentType::I32";
    case kIROp_UIntType:
        return "dx::linalg::ComponentType::U32";
    case kIROp_Int64Type:
        return "dx::linalg::ComponentType::I64";
    case kIROp_UInt64Type:
        return "dx::linalg::ComponentType::U64";
    case kIROp_HalfType:
        return "dx::linalg::ComponentType::F16";
    case kIROp_FloatType:
        return "dx::linalg::ComponentType::F32";
    case kIROp_DoubleType:
        return "dx::linalg::ComponentType::F64";
    case kIROp_FloatE4M3Type:
        return "dx::linalg::ComponentType::F8_E4M3";
    case kIROp_FloatE5M2Type:
        return "dx::linalg::ComponentType::F8_E5M2";
    case kIROp_BFloat16Type:
        sink->diagnose(Diagnostics::UnsupportedCoopMatElementTypeForHlsl{
            .typeName = String(getIROpInfo(elementTypeOp).name),
            .location = loc});
        return nullptr;
    default:
        SLANG_UNEXPECTED("Unexpected element type opcode for cooperative matrix.");
    }
}

/* static */ const char* HLSLSourceEmitter::getCoopMatMatrixUseName(IRIntegerValue useVal)
{
    switch (useVal)
    {
    case SLANG_COOPERATIVE_MATRIX_USE_A:
        return "dx::linalg::MatrixUse::A";
    case SLANG_COOPERATIVE_MATRIX_USE_B:
        return "dx::linalg::MatrixUse::B";
    case SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR:
        return "dx::linalg::MatrixUse::Accumulator";
    default:
        SLANG_UNEXPECTED("Unexpected matrix use value for cooperative matrix.");
    }
}

/* static */ const char* HLSLSourceEmitter::getCoopMatMatrixScopeName(
    IRIntegerValue scopeVal,
    DiagnosticSink* sink,
    SourceLoc loc)
{
    // SM6.10 dx::linalg only supports Wave (subgroup), ThreadGroup (workgroup), and Thread
    // (invocation) scopes. Device-wide and cross-device scopes have no dx::linalg equivalent
    // and are intentionally not handled here.
    switch ((Slang::MemoryScope)scopeVal)
    {
    case Slang::MemoryScope::Subgroup:
        return "dx::linalg::MatrixScope::Wave";
    case Slang::MemoryScope::Workgroup:
        return "dx::linalg::MatrixScope::ThreadGroup";
    case Slang::MemoryScope::Invocation:
        return "dx::linalg::MatrixScope::Thread";
    default:
        sink->diagnose(
            Diagnostics::UnsupportedCoopMatScopeForHlsl{.scopeVal = scopeVal, .location = loc});
        return nullptr;
    }
}

/* static */ UnownedStringSlice HLSLSourceEmitter::getCoopVecComponentType_enum(
    SlangScalarType slangValue,
    IRIntegerValue inputInterpretationPackingFactor,
    bool sm610OrAbove)
{
    if (inputInterpretationPackingFactor != 1)
    {
        switch (slangValue)
        {
        case SLANG_SCALAR_TYPE_INT8:
            return UnownedStringSlice(sm610OrAbove ? "PackedS8x32" : "DATA_TYPE_SINT8_T4_PACKED");
        case SLANG_SCALAR_TYPE_UINT8:
            return UnownedStringSlice(sm610OrAbove ? "PackedU8x32" : "DATA_TYPE_UINT8_T4_PACKED");
        default:
            SLANG_UNEXPECTED(
                "Unsupported packed cooperative vector input interpretation for HLSL emission");
        }
    }

    switch (slangValue)
    {
    case SLANG_SCALAR_TYPE_FLOAT_E4M3:
        return UnownedStringSlice(sm610OrAbove ? "F8_E4M3" : "DATA_TYPE_FLOAT8_E4M3");
    case SLANG_SCALAR_TYPE_FLOAT_E5M2:
        return UnownedStringSlice(sm610OrAbove ? "F8_E5M2" : "DATA_TYPE_FLOAT8_E5M2");
    case SLANG_SCALAR_TYPE_FLOAT16:
        return UnownedStringSlice(sm610OrAbove ? "F16" : "DATA_TYPE_FLOAT16");
    case SLANG_SCALAR_TYPE_FLOAT32:
        return UnownedStringSlice(sm610OrAbove ? "F32" : "DATA_TYPE_FLOAT32");
    case SLANG_SCALAR_TYPE_INT8:
        return UnownedStringSlice(sm610OrAbove ? "I8" : "DATA_TYPE_SINT8");
    case SLANG_SCALAR_TYPE_INT16:
        return UnownedStringSlice(sm610OrAbove ? "I16" : "DATA_TYPE_SINT16");
    case SLANG_SCALAR_TYPE_INT32:
        return UnownedStringSlice(sm610OrAbove ? "I32" : "DATA_TYPE_SINT32");
    case SLANG_SCALAR_TYPE_UINT8:
        return UnownedStringSlice(sm610OrAbove ? "U8" : "DATA_TYPE_UINT8");
    case SLANG_SCALAR_TYPE_UINT16:
        return UnownedStringSlice(sm610OrAbove ? "U16" : "DATA_TYPE_UINT16");
    case SLANG_SCALAR_TYPE_UINT32:
        return UnownedStringSlice(sm610OrAbove ? "U32" : "DATA_TYPE_UINT32");
    default:
        SLANG_UNEXPECTED("Unsupported cooperative vector component type for HLSL emission");
    }
}

/* static */ UnownedStringSlice HLSLSourceEmitter::getInterpolationModifier_keyword(
    IRInterpolationMode mode)
{
    switch (mode)
    {
    case IRInterpolationMode::PerVertex:
    case IRInterpolationMode::NoInterpolation:
        return UnownedStringSlice::fromLiteral("nointerpolation");
    case IRInterpolationMode::NoPerspective:
        return UnownedStringSlice::fromLiteral("noperspective");
    case IRInterpolationMode::Linear:
        return UnownedStringSlice::fromLiteral("linear");
    case IRInterpolationMode::Sample:
        return UnownedStringSlice::fromLiteral("sample");
    case IRInterpolationMode::Centroid:
        return UnownedStringSlice::fromLiteral("centroid");
    default:
        return UnownedStringSlice();
    }
}

} // namespace Slang
