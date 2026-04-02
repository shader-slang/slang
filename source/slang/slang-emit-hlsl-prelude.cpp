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

} // namespace Slang
