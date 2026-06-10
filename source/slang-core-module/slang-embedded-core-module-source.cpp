#include "slang-compiler.h"
#include "slang-core-module-textures.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

namespace Slang
{
// We are going to generate the core module source code from a more compact
// description. For example, we need to generate all the `operator`
// declarations for the basic unary and binary math operations on
// builtin types. To do this, we will make a big array of all these
// types, and associate them with data on their categories/capabilities
// so that we generate only the correct operations.
//
enum
{
    SINT_MASK = 1 << 0,
    FLOAT_MASK = 1 << 1,
    BOOL_RESULT = 1 << 2,
    BOOL_MASK = 1 << 3,
    UINT_MASK = 1 << 4,

    INT_MASK = SINT_MASK | UINT_MASK,
    ARITHMETIC_MASK = INT_MASK | FLOAT_MASK,
    LOGICAL_MASK = INT_MASK | BOOL_MASK,
    ANY_MASK = INT_MASK | FLOAT_MASK | BOOL_MASK,
};

// Here we declare the table of all our builtin types, so that we can generate all the relevant
// declarations. The scalar conversion-cost policy lives in `getBaseTypeConversionCost` so
// semantic checking and this generator keep one shared ordering.
//
struct BaseTypeConversionInfo
{
    char const* name;
    BaseType tag;
    unsigned flags;
};
// kBaseTypes is only referenced from within the generated *.meta.slang.h includes,
// which are guarded by SLANG_EMBED_CORE_MODULE_SOURCE.
#if SLANG_EMBED_CORE_MODULE_SOURCE
static const BaseTypeConversionInfo kBaseTypes[] = {
    // TODO: `void` really shouldn't be in the `BaseType` enumeration, since it behaves so
    // differently across the board
    {"void", BaseType::Void, 0},

    {"bool", BaseType::Bool, BOOL_MASK},

    {"int8_t", BaseType::Int8, SINT_MASK},
    {"int16_t", BaseType::Int16, SINT_MASK},
    {"int", BaseType::Int, SINT_MASK},
    {"int64_t", BaseType::Int64, SINT_MASK},
    {"intptr_t", BaseType::IntPtr, SINT_MASK},


    {"half", BaseType::Half, FLOAT_MASK},
    {"float", BaseType::Float, FLOAT_MASK},
    {"double", BaseType::Double, FLOAT_MASK},

    {"uint8_t", BaseType::UInt8, UINT_MASK},
    {"uint16_t", BaseType::UInt16, UINT_MASK},
    {"uint", BaseType::UInt, UINT_MASK},
    {"uint64_t", BaseType::UInt64, UINT_MASK},
    {"uintptr_t", BaseType::UIntPtr, UINT_MASK},
};
#endif // SLANG_EMBED_CORE_MODULE_SOURCE

IROp getBaseTypeConversionOp(BaseType toType, BaseType fromType)
{
    if (toType == fromType)
        return kIROp_Nop;

    IROp intrinsicOpCode = kIROp_Nop;
    auto toStyle = getTypeStyle(toType);
    auto fromStyle = getTypeStyle(fromType);
    if (toStyle == kIROp_BoolType)
        toStyle = kIROp_IntType;
    if (fromStyle == kIROp_BoolType)
        fromStyle = kIROp_IntType;
    if (toStyle == kIROp_IntType && fromStyle == kIROp_IntType)
        intrinsicOpCode = kIROp_IntCast;
    if (toStyle == kIROp_IntType && fromStyle == kIROp_FloatType)
        intrinsicOpCode = kIROp_CastFloatToInt;
    if (toStyle == kIROp_FloatType && fromStyle == kIROp_IntType)
        intrinsicOpCode = kIROp_CastIntToFloat;
    if (toStyle == kIROp_FloatType && fromStyle == kIROp_FloatType)
        intrinsicOpCode = kIROp_FloatCast;
    return intrinsicOpCode;
}

struct IntrinsicOpInfo
{
    IROp opCode;
    char const* funcName;
    char const* opName;
    char const* interface;
    unsigned flags;
};

[[maybe_unused]] static const IntrinsicOpInfo intrinsicUnaryOps[] = {
    {kIROp_Neg, "neg", "-", "__BuiltinArithmeticType", ARITHMETIC_MASK},
    {kIROp_Not, "logicalNot", "!", nullptr, BOOL_MASK | BOOL_RESULT},
    {kIROp_BitNot, "not", "~", "__BuiltinLogicalType", INT_MASK},
};

[[maybe_unused]] static const IntrinsicOpInfo intrinsicBinaryOps[] = {
    {kIROp_Add, "add", "+", "__BuiltinArithmeticType", ARITHMETIC_MASK},
    {kIROp_Sub, "sub", "-", "__BuiltinArithmeticType", ARITHMETIC_MASK},
    {kIROp_Mul, "mul", "*", "__BuiltinArithmeticType", ARITHMETIC_MASK},
    {kIROp_Div, "div", "/", "__BuiltinArithmeticType", ARITHMETIC_MASK},
    {kIROp_IRem, "irem", "%", "__BuiltinIntegerType", INT_MASK},
    {kIROp_FRem, "frem", "%", "__BuiltinFloatingPointType", FLOAT_MASK},
    {kIROp_And, "logicalAnd", "&&", nullptr, BOOL_MASK | BOOL_RESULT},
    {kIROp_Or, "logicalOr", "||", nullptr, BOOL_MASK | BOOL_RESULT},
    {kIROp_BitAnd, "and", "&", "__BuiltinLogicalType", LOGICAL_MASK},
    {kIROp_BitOr, "or", "|", "__BuiltinLogicalType", LOGICAL_MASK},
    {kIROp_BitXor, "xor", "^", "__BuiltinLogicalType", LOGICAL_MASK},
    {kIROp_Eql, "eql", "==", "__BuiltinType", ANY_MASK | BOOL_RESULT},
    {kIROp_Neq, "neq", "!=", "__BuiltinType", ANY_MASK | BOOL_RESULT},
    {kIROp_Greater, "greater", ">", "__BuiltinArithmeticType", ARITHMETIC_MASK | BOOL_RESULT},
    {kIROp_Less, "less", "<", "__BuiltinArithmeticType", ARITHMETIC_MASK | BOOL_RESULT},
    {kIROp_Geq, "geq", ">=", "__BuiltinArithmeticType", ARITHMETIC_MASK | BOOL_RESULT},
    {kIROp_Leq, "leq", "<=", "__BuiltinArithmeticType", ARITHMETIC_MASK | BOOL_RESULT},
};

// Integer types that can be used in atomic operations in CUDA.
[[maybe_unused]] static const char* kCudaAtomicIntegerTypes[] =
    {"int", "uint", "uint64_t", "int64_t"};

// Both the following functions use these macros.
// NOTE! They require a variable named path to emit the #line correctly if in source file.
#define SLANG_RAW(TEXT) sb << TEXT;
#define SLANG_SPLICE(EXPR) sb << (EXPR);

#define EMIT_LINE_DIRECTIVE() sb << "#line " << (__LINE__ + 1) << " \"" << path << "\"\n"

#if SLANG_CLANG
// Clang takes around 10 minutes to compile this file with optimizations, with EarlyCSEPass taking
// ~95% of the execution time. Disabling optimizations here reduces the compilation time to seconds
// and has no noticeable impact on run-time performance. These functions are only called once,
// either at build time by slang-bootstrap, or on the first run of the slang-compiler library,
// depending on the value of the SLANG_EMBED_CORE_MODULE CMake option.
#pragma clang optimize off
#endif

ComPtr<ISlangBlob> Session::getCoreLibraryCode()
{
#if SLANG_EMBED_CORE_MODULE_SOURCE
    if (!coreLibraryCode)
    {
        StringBuilder sb;
        const String path = getCoreModulePath();
#include "core.meta.slang.h"
        coreLibraryCode = StringBlob::moveCreate(sb);
    }
#endif
    return coreLibraryCode;
}

ComPtr<ISlangBlob> Session::getHLSLLibraryCode()
{
#if SLANG_EMBED_CORE_MODULE_SOURCE
    if (!hlslLibraryCode)
    {
        const String path = getCoreModulePath();
        StringBuilder sb;
#include "hlsl.meta.slang.h"
        hlslLibraryCode = StringBlob::moveCreate(sb);
    }
#endif
    return hlslLibraryCode;
}

ComPtr<ISlangBlob> Session::getAutodiffLibraryCode()
{
#if SLANG_EMBED_CORE_MODULE_SOURCE
    if (!autodiffLibraryCode)
    {
        const String path = getCoreModulePath();
        StringBuilder sb;
#include "diff.meta.slang.h"
        autodiffLibraryCode = StringBlob::moveCreate(sb);
    }
#endif
    return autodiffLibraryCode;
}

ComPtr<ISlangBlob> Session::getGLSLLibraryCode()
{
#if SLANG_EMBED_CORE_MODULE_SOURCE
    if (!glslLibraryCode)
    {
        const String path = getCoreModulePath();
        StringBuilder sb;
#include "glsl.meta.slang.h"
        glslLibraryCode = StringBlob::moveCreate(sb);
    }
#endif
    return glslLibraryCode;
}
} // namespace Slang

#if SLANG_CLANG
#pragma clang optimize on
#endif
