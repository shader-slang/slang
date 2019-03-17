// slang-stdlib.cpp

#include "compiler.h"
#include "ir.h"
#include "syntax.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

namespace Slang
{
    String Session::getStdlibPath()
    {
        if(stdlibPath.Length() != 0)
            return stdlibPath;

        StringBuilder pathBuilder;
        for( auto cc = __FILE__; *cc; ++cc )
        {
            switch( *cc )
            {
            case '\n':
            case '\t':
            case '\\':
                pathBuilder << "\\";
                ; // fall-thru
            default:
                pathBuilder << *cc;
                break;
            }
        }
        stdlibPath = pathBuilder.ProduceString();

        return stdlibPath;
    }

    // We are going to generate the stdlib source code from a more compact
    // description. For example, we need to generate all the `operator`
    // declarations for the basic unary and binary math operations on
    // builtin types. To do this, we will make a big array of all these
    // types, and associate them with data on their categories/capabilities
    // so that we generate only the correct operations.
    //
    enum
    {
        SINT_MASK   = 1 << 0,
        FLOAT_MASK  = 1 << 1,
        BOOL_RESULT = 1 << 2,
        BOOL_MASK   = 1 << 3,
        UINT_MASK   = 1 << 4,
        ASSIGNMENT  = 1 << 5,
        POSTFIX     = 1 << 6,

        INT_MASK = SINT_MASK | UINT_MASK,
        ARITHMETIC_MASK = INT_MASK | FLOAT_MASK,
        LOGICAL_MASK = INT_MASK | BOOL_MASK,
        ANY_MASK = INT_MASK | FLOAT_MASK | BOOL_MASK,
    };

    // We are going to declare initializers that allow for conversion between
    // all of our base types, and we need a way to priotize those conversion
    // by giving them different costs. Rather than maintain a hard-coded table
    // of N^2 costs for N basic types, we are going to try to do things a bit
    // more systematically.
    //
    // Every base type will be given a "kind" and a "rank" for conversion.
    // The kind will classify it as signed/unsigned/float, and the rank will
    // classify it by its logical bit size (with a distinct rank for pointer-sized
    // types that logically sits between 32- and 64-bit types).
    //
    enum BaseTypeConversionKind : uint8_t
    {
        kBaseTypeConversionKind_Signed,
        kBaseTypeConversionKind_Unsigned,
        kBaseTypeConversionKind_Float,
        kBaseTypeConversionKind_Error,
    };
    enum BaseTypeConversionRank : uint8_t
    {
        kBaseTypeConversionRank_Bool,
        kBaseTypeConversionRank_Int8,
        kBaseTypeConversionRank_Int16,
        kBaseTypeConversionRank_Int32,
        kBaseTypeConversionRank_IntPtr,
        kBaseTypeConversionRank_Int64,
        kBaseTypeConversionRank_Error,
    };

    // Here we declare the table of all our builtin types, so that we can generate all the relevant declarations.
    //
    struct BaseTypeInfo
    {
        char const* name;
        BaseType	tag;
        unsigned    flags;
        BaseTypeConversionKind conversionKind;
        BaseTypeConversionRank conversionRank;
    };
    static const BaseTypeInfo kBaseTypes[] = {
        // TODO: `void` really shouldn't be in the `BaseType` enumeration, since it behaves so differently across the board
        { "void",	BaseType::Void,     0,          kBaseTypeConversionKind_Error,      kBaseTypeConversionRank_Error},

        { "bool",	BaseType::Bool,     BOOL_MASK,  kBaseTypeConversionKind_Unsigned,   kBaseTypeConversionRank_Bool },

        { "int8_t",	    BaseType::Int8,     SINT_MASK,  kBaseTypeConversionKind_Signed,     kBaseTypeConversionRank_Int8},
        { "int16_t",	BaseType::Int16,    SINT_MASK,  kBaseTypeConversionKind_Signed,     kBaseTypeConversionRank_Int16},
        { "int",	    BaseType::Int,      SINT_MASK,  kBaseTypeConversionKind_Signed,     kBaseTypeConversionRank_Int32},
        { "int64_t",	BaseType::Int64,    SINT_MASK,  kBaseTypeConversionKind_Signed,     kBaseTypeConversionRank_Int64},

        { "half",	BaseType::Half,     FLOAT_MASK, kBaseTypeConversionKind_Float,      kBaseTypeConversionRank_Int16},
        { "float",	BaseType::Float,    FLOAT_MASK, kBaseTypeConversionKind_Float,      kBaseTypeConversionRank_Int32},
        { "double",	BaseType::Double,   FLOAT_MASK, kBaseTypeConversionKind_Float,      kBaseTypeConversionRank_Int64},

        { "uint8_t",	BaseType::UInt8,    UINT_MASK,  kBaseTypeConversionKind_Unsigned,   kBaseTypeConversionRank_Int8},
        { "uint16_t",	BaseType::UInt16,   UINT_MASK,  kBaseTypeConversionKind_Unsigned,   kBaseTypeConversionRank_Int16},
        { "uint",	    BaseType::UInt,     UINT_MASK,  kBaseTypeConversionKind_Unsigned,   kBaseTypeConversionRank_Int32},
        { "uint64_t",   BaseType::UInt64,   UINT_MASK,  kBaseTypeConversionKind_Unsigned,   kBaseTypeConversionRank_Int64},
    };

    // Given two base types, we need to be able to compute the cost of converting between them.
    ConversionCost getBaseTypeConversionCost(
        BaseTypeInfo const& toInfo,
        BaseTypeInfo const& fromInfo)
    {
        if(toInfo.conversionKind == fromInfo.conversionKind
            && toInfo.conversionRank == fromInfo.conversionRank)
        {
            // Thse should represent the exact same type.
            return kConversionCost_None;
        }

        // Conversions within the same kind are easist to handle
        if (toInfo.conversionKind == fromInfo.conversionKind)
        {
            // If we are converting to a "larger" type, then
            // we are doing a lossless promotion, and otherwise
            // we are doing a demotion.
            if( toInfo.conversionRank > fromInfo.conversionRank)
                return kConversionCost_RankPromotion;
            else
                return kConversionCost_GeneralConversion;
        }

        // If we are converting from an unsigned integer type to
        // a signed integer type that is guaranteed to be larger,
        // then that is also a lossless promotion.
        //
        // There is one additional wrinkle here, which is that
        // a conversion from a 32-bit unsigned integer to a
        // "pointer-sized" signed integer should be treated
        // as unsafe, because the pointer size might also be
        // 32 bits.
        //
        // The same basic exemption applied when converting
        // *from* a pointer-sized unsigned integer.
        else if(toInfo.conversionKind == kBaseTypeConversionKind_Signed
            && fromInfo.conversionKind == kBaseTypeConversionKind_Unsigned
            && toInfo.conversionRank > fromInfo.conversionRank
            && toInfo.conversionRank != kBaseTypeConversionRank_IntPtr
            && fromInfo.conversionRank != kBaseTypeConversionRank_IntPtr)
        {
            return kConversionCost_UnsignedToSignedPromotion;
        }

        // Conversion from signed to unsigned is always lossy,
        // but it is preferred over conversions from unsigned
        // to signed, for same-size types.
        else if(toInfo.conversionKind == kBaseTypeConversionKind_Unsigned
            && fromInfo.conversionKind == kBaseTypeConversionKind_Signed
            && toInfo.conversionRank >= fromInfo.conversionRank)
        {
            return kConversionCost_SignedToUnsignedConversion;
        }

        // Conversion from an integer to a floating-point type
        // is never considered a promotion (even when the value
        // would fit in the available mantissa bits).
        // If the destination type is at least 32 bits we consider
        // this a reasonably good conversion, though.
        //
        // Note that this means we do *not* consider implicit
        // conversion to `half` as a good conversion, even for small
        // types. This makes sense because we relaly want to prefer
        // conversion to `float` as the default.
        else if (toInfo.conversionKind == kBaseTypeConversionKind_Float
            && toInfo.conversionRank >= kBaseTypeConversionRank_Int32)
        {
            return kConversionCost_IntegerToFloatConversion;
        }

        // All other cases are considered as "general" conversions,
        // where we don't consider any one conversion better than
        // any others.
        else
        {
            return kConversionCost_GeneralConversion;
        }
    }

    struct OpInfo { int32_t opCode; char const* opName; unsigned flags; };

    static const OpInfo unaryOps[] = {
        { kIRPseudoOp_Pos,     "+",    ARITHMETIC_MASK },
        { kIROp_Neg,     "-",    ARITHMETIC_MASK },
        { kIROp_Not,     "!",    BOOL_MASK | BOOL_RESULT },
        { kIROp_BitNot,    "~",    INT_MASK        },
        { kIRPseudoOp_PreInc,  "++",   ARITHMETIC_MASK | ASSIGNMENT },
        { kIRPseudoOp_PreDec,  "--",   ARITHMETIC_MASK | ASSIGNMENT },
        { kIRPseudoOp_PostInc, "++",   ARITHMETIC_MASK | ASSIGNMENT | POSTFIX },
        { kIRPseudoOp_PostDec, "--",   ARITHMETIC_MASK | ASSIGNMENT | POSTFIX },
    };

    static const OpInfo binaryOps[] = {
        { kIROp_Add,     "+",    ARITHMETIC_MASK },
        { kIROp_Sub,     "-",    ARITHMETIC_MASK },
        { kIROp_Mul,     "*",    ARITHMETIC_MASK },
        { kIROp_Div,     "/",    ARITHMETIC_MASK },
        { kIROp_Mod,     "%",    INT_MASK },
        { kIROp_And,     "&&",   BOOL_MASK | BOOL_RESULT},
        { kIROp_Or,      "||",   BOOL_MASK | BOOL_RESULT },
        { kIROp_BitAnd,  "&",    LOGICAL_MASK },
        { kIROp_BitOr,   "|",    LOGICAL_MASK },
        { kIROp_BitXor,  "^",    LOGICAL_MASK },
        { kIROp_Lsh,     "<<",   INT_MASK },
        { kIROp_Rsh,     ">>",   INT_MASK },
        { kIROp_Eql,     "==",   ANY_MASK | BOOL_RESULT },
        { kIROp_Neq,     "!=",   ANY_MASK | BOOL_RESULT },
        { kIROp_Greater, ">",    ARITHMETIC_MASK | BOOL_RESULT },
        { kIROp_Less,    "<",    ARITHMETIC_MASK | BOOL_RESULT },
        { kIROp_Geq,     ">=",   ARITHMETIC_MASK | BOOL_RESULT },
        { kIROp_Leq,     "<=",   ARITHMETIC_MASK | BOOL_RESULT },
        { kIRPseudoOp_AddAssign,     "+=",    ASSIGNMENT | ARITHMETIC_MASK },
        { kIRPseudoOp_SubAssign,     "-=",    ASSIGNMENT | ARITHMETIC_MASK },
        { kIRPseudoOp_MulAssign,     "*=",    ASSIGNMENT | ARITHMETIC_MASK },
        { kIRPseudoOp_DivAssign,     "/=",    ASSIGNMENT | ARITHMETIC_MASK },
        { kIRPseudoOp_ModAssign,     "%=",    ASSIGNMENT | ARITHMETIC_MASK },
        { kIRPseudoOp_AndAssign,     "&=",    ASSIGNMENT | LOGICAL_MASK },
        { kIRPseudoOp_OrAssign,      "|=",    ASSIGNMENT | LOGICAL_MASK },
        { kIRPseudoOp_XorAssign,     "^=",    ASSIGNMENT | LOGICAL_MASK },
        { kIRPseudoOp_LshAssign,     "<<=",   ASSIGNMENT | INT_MASK },
        { kIRPseudoOp_RshAssign,     ">>=",   ASSIGNMENT | INT_MASK },
    };

    String Session::getCoreLibraryCode()
    {
        if (coreLibraryCode.Length() > 0)
            return coreLibraryCode;

        StringBuilder sb;

        String path = getStdlibPath();

#define SLANG_RAW(TEXT) sb << TEXT;
#define SLANG_SPLICE(EXPR) sb << (EXPR);

#define EMIT_LINE_DIRECTIVE() sb << "#line " << (__LINE__+1) << " \"" << path << "\"\n"

        #include "core.meta.slang.h"

        coreLibraryCode = sb.ProduceString();
        return coreLibraryCode;
    }

    String Session::getHLSLLibraryCode()
    {
        if (hlslLibraryCode.Length() > 0)
            return hlslLibraryCode;

        StringBuilder sb;

        #include "hlsl.meta.slang.h"

        hlslLibraryCode = sb.ProduceString();
        return hlslLibraryCode;
    }
}
