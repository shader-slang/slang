// emit.cpp
#include "emit.h"

#include "../core/slang-writer.h"
#include "ir-bind-existentials.h"
#include "ir-dce.h"
#include "ir-entry-point-uniforms.h"
#include "ir-glsl-legalize.h"
#include "ir-insts.h"
#include "ir-link.h"
#include "ir-restructure.h"
#include "ir-restructure-scoping.h"
#include "ir-specialize.h"
#include "ir-specialize-resources.h"
#include "ir-ssa.h"
#include "ir-union.h"
#include "ir-validate.h"
#include "legalize-types.h"
#include "lower-to-ir.h"
#include "mangle.h"
#include "name.h"
#include "syntax.h"
#include "type-layout.h"
#include "visitor.h"

#include "slang-source-stream.h"

#include <assert.h>

namespace Slang {

enum class BuiltInCOp
{
    Splat,                  //< Splat a single value to all values of a vector or matrix type
    Init,                   //< Initialize with parameters (must match the type)
};

struct ExtensionUsageTracker
{
    // Record the GLSL extnsions we have already emitted a `#extension` for
    HashSet<String> glslExtensionsRequired;
    StringBuilder glslExtensionRequireLines;

    ProfileVersion profileVersion = ProfileVersion::GLSL_110;

    bool hasHalfExtension = false;
};

void requireGLSLExtension(
    ExtensionUsageTracker*  tracker,
    String const&           name)
{
    if (tracker->glslExtensionsRequired.Contains(name))
        return;

    StringBuilder& sb = tracker->glslExtensionRequireLines;

    sb.append("#extension ");
    sb.append(name);
    sb.append(" : require\n");

    tracker->glslExtensionsRequired.Add(name);
}

void requireGLSLVersionImpl(
    ExtensionUsageTracker*  tracker,
    ProfileVersion          version)
{
    // Check if this profile is newer
    if ((UInt)version > (UInt)tracker->profileVersion)
    {
        tracker->profileVersion = version;
    }
}

void requireGLSLHalfExtension(ExtensionUsageTracker* tracker)
{
    if (!tracker->hasHalfExtension)
    {
        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_16bit_storage.txt
        requireGLSLExtension(tracker, "GL_EXT_shader_16bit_storage");

        // https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_shader_explicit_arithmetic_types.txt
        requireGLSLExtension(tracker, "GL_EXT_shader_explicit_arithmetic_types");
        
        tracker->hasHalfExtension = true;
    }
}


// Shared state for an entire emit session
struct SharedEmitContext
{
    DiagnosticSink* getSink() { return compileRequest->getSink(); }
    LineDirectiveMode getLineDirectiveMode() { return compileRequest->getLineDirectiveMode(); }
    SourceManager* getSourceManager() { return compileRequest->getSourceManager(); }
    void noteInternalErrorLoc(SourceLoc loc) { return getSink()->noteInternalErrorLoc(loc); }

    BackEndCompileRequest* compileRequest = nullptr;

    // The entry point we are being asked to compile
    EntryPoint* entryPoint;

    // The layout for the entry point
    EntryPointLayout*   entryPointLayout;

    // The target language we want to generate code for
    CodeGenTarget target;

    // Where source is written to
    SourceStream* stream;  
     
    // We only want to emit each `import`ed module one time, so
    // we maintain a set of already-emitted modules.
    HashSet<ModuleDecl*> modulesAlreadyEmitted;

    // We track the original global-scope layout so that we can
    // find layout information for `import`ed parameters.
    //
    // TODO: This will probably change if we represent imports
    // explicitly in the layout data.
    StructTypeLayout*   globalStructLayout;

    ProgramLayout*      programLayout;

    ModuleDecl*  program;

    ExtensionUsageTracker extensionUsageTracker;

    UInt uniqueIDCounter = 1;
    Dictionary<IRInst*, UInt> mapIRValueToID;
    Dictionary<Decl*, UInt> mapDeclToID;

    HashSet<String> irDeclsVisited;

    HashSet<String> irTupleTypes;

    // The "effective" profile that is being used to emit code,
    // combining information from the target and entry point.
    Profile effectiveProfile;

    // Map a string name to the number of times we have seen this
    // name used so far during code emission.
    Dictionary<String, UInt> uniqueNameCounters;

    // Map an IR instruction to the name that we've decided
    // to use for it when emitting code.
    Dictionary<IRInst*, String> mapInstToName;

    Dictionary<IRInst*, UInt> mapIRValueToRayPayloadLocation;
    Dictionary<IRInst*, UInt> mapIRValueToCallablePayloadLocation;
};
//

enum EPrecedence
{
#define LEFT(NAME)                      \
    kEPrecedence_##NAME##_Left,     \
    kEPrecedence_##NAME##_Right

#define RIGHT(NAME)                     \
    kEPrecedence_##NAME##_Right,    \
    kEPrecedence_##NAME##_Left

#define NONASSOC(NAME)                  \
    kEPrecedence_##NAME##_Left,     \
    kEPrecedence_##NAME##_Right = kEPrecedence_##NAME##_Left

    NONASSOC(None),
    LEFT(Comma),

    NONASSOC(General),

    RIGHT(Assign),

    RIGHT(Conditional),

    LEFT(Or),
    LEFT(And),
    LEFT(BitOr),
    LEFT(BitXor),
    LEFT(BitAnd),

    LEFT(Equality),
    LEFT(Relational),
    LEFT(Shift),
    LEFT(Additive),
    LEFT(Multiplicative),
    RIGHT(Prefix),
    LEFT(Postfix),
    NONASSOC(Atomic),
};

// Info on an op for emit purposes
struct EOpInfo
{
    char const* op;
    EPrecedence leftPrecedence;
    EPrecedence rightPrecedence;
};

#define OP(NAME, TEXT, PREC) \
static const EOpInfo kEOp_##NAME = { TEXT, kEPrecedence_##PREC##_Left, kEPrecedence_##PREC##_Right, }

OP(None,        "",     None);

OP(Comma,       ",",    Comma);

OP(General,     "",     General);

OP(Assign,      "=",    Assign);
OP(AddAssign,   "+=",   Assign);
OP(SubAssign,   "-=",   Assign);
OP(MulAssign,   "*=",   Assign);
OP(DivAssign,   "/=",   Assign);
OP(ModAssign,   "%=",   Assign);
OP(LshAssign,   "<<=",  Assign);
OP(RshAssign,   ">>=",  Assign);
OP(OrAssign,    "|=",   Assign);
OP(AndAssign,   "&=",   Assign);
OP(XorAssign,   "^=",   Assign);

OP(Conditional, "?:",   Conditional);

OP(Or,          "||",   Or);
OP(And,         "&&",   And);
OP(BitOr,       "|",    BitOr);
OP(BitXor,      "^",    BitXor);
OP(BitAnd,      "&",    BitAnd);

OP(Eql,         "==",   Equality);
OP(Neq,         "!=",   Equality);

OP(Less,        "<",    Relational);
OP(Greater,     ">",    Relational);
OP(Leq,         "<=",   Relational);
OP(Geq,         ">=",   Relational);

OP(Lsh,         "<<",   Shift);
OP(Rsh,         ">>",   Shift);

OP(Add,         "+",    Additive);
OP(Sub,         "-",    Additive);

OP(Mul,         "*",    Multiplicative);
OP(Div,         "/",    Multiplicative);
OP(Mod,         "%",    Multiplicative);

OP(Prefix,      "",     Prefix);
OP(Postfix,     "",     Postfix);
OP(Atomic,      "",     Atomic);

#undef OP

// Table to allow data-driven lookup of an op based on its
// name (to assist when outputting unchecked operator calls)
static EOpInfo const* const kInfixOpInfos[] =
{
    &kEOp_Comma,
    &kEOp_Assign,
    &kEOp_AddAssign,
    &kEOp_SubAssign,
    &kEOp_MulAssign,
    &kEOp_DivAssign,
    &kEOp_ModAssign,
    &kEOp_LshAssign,
    &kEOp_RshAssign,
    &kEOp_OrAssign,
    &kEOp_AndAssign,
    &kEOp_XorAssign,
    &kEOp_Or,
    &kEOp_And,
    &kEOp_BitOr,
    &kEOp_BitXor,
    &kEOp_BitAnd,
    &kEOp_Eql,
    &kEOp_Neq,
    &kEOp_Less,
    &kEOp_Greater,
    &kEOp_Leq,
    &kEOp_Geq,
    &kEOp_Lsh,
    &kEOp_Rsh,
    &kEOp_Add,
    &kEOp_Sub,
    &kEOp_Mul,
    &kEOp_Div,
    &kEOp_Mod,
};

//

// represents a declarator for use in emitting types
struct EDeclarator
{
    enum class Flavor
    {
        name,
        Array,
        UnsizedArray,
    };
    Flavor flavor;
    EDeclarator* next = nullptr;

    // Used for `Flavor::name`
    Name*       name;
    SourceLoc   loc;

    // Used for `Flavor::Array`
    IRInst* elementCount;
};

struct EmitVisitor
{
    SharedEmitContext* context;
    SourceStream* stream;

    EmitVisitor(SharedEmitContext* context)
        : context(context),
        stream(context->stream)
    {}

    SourceManager* getSourceManager()
    {
        return context->getSourceManager();
    }

    DiagnosticSink* getSink()
    {
        return context->getSink();
    }

    //
    // Types
    //

    void EmitDeclarator(EDeclarator* declarator)
    {
        if (!declarator) return;

        stream->emit(" ");

        switch (declarator->flavor)
        {
        case EDeclarator::Flavor::name:
            stream->emitName(declarator->name, declarator->loc);
            break;

        case EDeclarator::Flavor::Array:
            EmitDeclarator(declarator->next);
            stream->emit("[");
            if(auto elementCount = declarator->elementCount)
            {
                EmitVal(elementCount, kEOp_General);
            }
            stream->emit("]");
            break;

        case EDeclarator::Flavor::UnsizedArray:
            EmitDeclarator(declarator->next);
            stream->emit("[]");
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unknown declarator flavor");
            break;
        }
    }

    void emitGLSLTypePrefix(
        IRType* type,
        bool promoteHalfToFloat = false)
    {
        switch (type->op)
        {
        case kIROp_FloatType:
            // no prefix
            break;

        case kIROp_Int8Type:    stream->emit("i8");     break;
        case kIROp_Int16Type:   stream->emit("i16");    break;
        case kIROp_IntType:     stream->emit("i");      break;
        case kIROp_Int64Type:   stream->emit("i64");    break;

        case kIROp_UInt8Type:   stream->emit("u8");     break;
        case kIROp_UInt16Type:  stream->emit("u16");    break;
        case kIROp_UIntType:    stream->emit("u");      break;
        case kIROp_UInt64Type:  stream->emit("u64");    break;

        case kIROp_BoolType:    stream->emit("b");		break;

        case kIROp_HalfType:
        {
            _requireHalf();
            if (promoteHalfToFloat)
            {
                // no prefix
            }
            else
            {
                stream->emit("f16");
            }
            break;
        }
        case kIROp_DoubleType:  stream->emit("d");		break;

        case kIROp_VectorType:
            emitGLSLTypePrefix(cast<IRVectorType>(type)->getElementType(), promoteHalfToFloat);
            break;

        case kIROp_MatrixType:
            emitGLSLTypePrefix(cast<IRMatrixType>(type)->getElementType(), promoteHalfToFloat);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled GLSL type prefix");
            break;
        }
    }

    void emitHLSLTextureType(
        IRTextureTypeBase* texType)
    {
        switch(texType->getAccess())
        {
        case SLANG_RESOURCE_ACCESS_READ:
            break;

        case SLANG_RESOURCE_ACCESS_READ_WRITE:
            stream->emit("RW");
            break;

        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            stream->emit("RasterizerOrdered");
            break;

        case SLANG_RESOURCE_ACCESS_APPEND:
            stream->emit("Append");
            break;

        case SLANG_RESOURCE_ACCESS_CONSUME:
            stream->emit("Consume");
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            break;
        }

        switch (texType->GetBaseShape())
        {
        case TextureFlavor::Shape::Shape1D:		stream->emit("Texture1D");		break;
        case TextureFlavor::Shape::Shape2D:		stream->emit("Texture2D");		break;
        case TextureFlavor::Shape::Shape3D:		stream->emit("Texture3D");		break;
        case TextureFlavor::Shape::ShapeCube:	stream->emit("TextureCube");	break;
        case TextureFlavor::Shape::ShapeBuffer:  stream->emit("Buffer");         break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            break;
        }

        if (texType->isMultisample())
        {
            stream->emit("MS");
        }
        if (texType->isArray())
        {
            stream->emit("Array");
        }
        stream->emit("<");
        EmitType(texType->getElementType());
        stream->emit(" >");
    }

    void emitGLSLTextureOrTextureSamplerType(
        IRTextureTypeBase*  type,
        char const*         baseName)
    {
        if (type->getElementType()->op == kIROp_HalfType)
        {
            // Texture access is always as float types if half is specified

        }
        else
        {
            emitGLSLTypePrefix(type->getElementType(), true);
        }

        stream->emit(baseName);
        switch (type->GetBaseShape())
        {
        case TextureFlavor::Shape::Shape1D:		stream->emit("1D");		break;
        case TextureFlavor::Shape::Shape2D:		stream->emit("2D");		break;
        case TextureFlavor::Shape::Shape3D:		stream->emit("3D");		break;
        case TextureFlavor::Shape::ShapeCube:	stream->emit("Cube");	break;
        case TextureFlavor::Shape::ShapeBuffer:	stream->emit("Buffer");	break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            break;
        }

        if (type->isMultisample())
        {
            stream->emit("MS");
        }
        if (type->isArray())
        {
            stream->emit("Array");
        }
    }

    void emitGLSLTextureType(
        IRTextureType* texType)
    {
        switch(texType->getAccess())
        {
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            emitGLSLTextureOrTextureSamplerType(texType, "image");
            break;

        default:
            emitGLSLTextureOrTextureSamplerType(texType, "texture");
            break;
        }
    }

    void emitGLSLTextureSamplerType(
        IRTextureSamplerType* type)
    {
        emitGLSLTextureOrTextureSamplerType(type, "sampler");
    }

    void emitGLSLImageType(
        IRGLSLImageType* type)
    {
        emitGLSLTextureOrTextureSamplerType(type, "image");
    }

    void emitTextureType(
        IRTextureType*  texType)
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLTextureType(texType);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLTextureType(texType);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
            break;
        }
    }

    void emitTextureSamplerType(
        IRTextureSamplerType*   type)
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
            emitGLSLTextureSamplerType(type);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see combined texture-sampler types");
            break;
        }
    }

    void emitImageType(
        IRGLSLImageType*    type)
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLTextureType(type);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLImageType(type);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see GLSL image types");
            break;
        }
    }

    static IROp _getCType(IROp op)
    {
        switch (op)
        {
            case kIROp_VoidType:
            case kIROp_BoolType:
            {
                return op;
            }
            case kIROp_Int8Type:
            case kIROp_Int16Type:
            case kIROp_IntType:
            case kIROp_UInt8Type:
            case kIROp_UInt16Type:
            case kIROp_UIntType:
            {
                // Promote all these to Int
                return kIROp_IntType;
            }
            case kIROp_Int64Type:
            case kIROp_UInt64Type:
            {
                // Promote all these to Int16, we can just vary the call to make these work
                return kIROp_Int64Type;
            }
            case kIROp_DoubleType:
            {
                return kIROp_DoubleType;
            }
            case kIROp_HalfType:
            case kIROp_FloatType:
            {
                // Promote both to float
                return kIROp_FloatType;
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled type");
                return kIROp_undefined;
            }
        }
    }

    static UnownedStringSlice _getCTypeVecPostFix(IROp op)
    {
        switch (op)
        {
            case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("B");
            case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I");
            case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F");
            case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
            case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
            default:                    return UnownedStringSlice::fromLiteral("?");
        }
    }

    static UnownedStringSlice _getCTypeName(IROp op)
    {
        switch (op)
        {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("Bool");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I32");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F32");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
        }
    }

    void _emitCVecType(IROp op, Int size)
    {
        stream->emit("Vec");
        const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
        stream->emit(postFix);
        if (postFix.size() > 1)
        {
            stream->emit("_");
        }
        stream->emit(size);
    }

    void _emitCMatType(IROp op, IRIntegerValue rowCount, IRIntegerValue colCount)
    {
        stream->emit("Mat");
        const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
        stream->emit(postFix);
        if (postFix.size() > 1)
        {
            stream->emit("_");
        }
        stream->emit(rowCount);
        stream->emit(colCount);
    }

    void _emitCFunc(BuiltInCOp cop, IRType* type)
    {
        emitSimpleTypeImpl(type);
        stream->emit("_");

        switch (cop)
        {
            case BuiltInCOp::Init:  stream->emit("init");
            case BuiltInCOp::Splat: stream->emit("splat"); break;
        }
    }

    void emitVectorTypeName(IRType* elementType, IRIntegerValue elementCount)
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                if (elementCount > 1)
                {
                    emitGLSLTypePrefix(elementType);
                    stream->emit("vec");
                    stream->emit(elementCount);
                }
                else
                {
                    emitSimpleTypeImpl(elementType);
                }
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            stream->emit("vector<");
            EmitType(elementType);
            stream->emit(",");
            stream->emit(elementCount);
            stream->emit(">");
            break;

        case CodeGenTarget::CSource:
        case CodeGenTarget::CPPSource:
            _emitCVecType(elementType->op, elementCount);
            break;

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
            break;
        }
    }

    void emitVectorTypeImpl(IRVectorType* vecType)
    {
        IRInst* elementCountInst = vecType->getElementCount();
        if (elementCountInst->op != kIROp_IntLit)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "Expecting an integral size for vector size");
            return;
        }

        const IRConstant* irConst = (const IRConstant*)elementCountInst;
        const IRIntegerValue elementCount = irConst->value.intVal;
        if (elementCount <= 0)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "Vector size must be greater than 0");
            return;
        }

        auto* elementType = vecType->getElementType();

        emitVectorTypeName(elementType, elementCount);
    }

    void emitMatrixTypeImpl(IRMatrixType* matType)
    {
        switch(context->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                emitGLSLTypePrefix(matType->getElementType());
                stream->emit("mat");
                EmitVal(matType->getRowCount(), kEOp_General);
                // TODO(tfoley): only emit the next bit
                // for non-square matrix
                stream->emit("x");
                EmitVal(matType->getColumnCount(), kEOp_General);
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            stream->emit("matrix<");
            EmitType(matType->getElementType());
            stream->emit(",");
            EmitVal(matType->getRowCount(), kEOp_General);
            stream->emit(",");
            EmitVal(matType->getColumnCount(), kEOp_General);
            stream->emit("> ");
            break;

        case CodeGenTarget::CPPSource:
        case CodeGenTarget::CSource:
        {
            const auto rowCount = static_cast<const IRConstant*>(matType->getRowCount())->value.intVal;
            const auto colCount = static_cast<const IRConstant*>(matType->getColumnCount())->value.intVal;

            _emitCMatType(matType->getElementType()->op, rowCount, colCount);
            break;
        }

        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
            break;
        }
    }

    void emitSamplerStateType(IRSamplerStateTypeBase* samplerStateType)
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
        default:
            switch (samplerStateType->op)
            {
            case kIROp_SamplerStateType:			stream->emit("SamplerState");			break;
            case kIROp_SamplerComparisonStateType:	stream->emit("SamplerComparisonState");	break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                break;
            }
            break;

        case CodeGenTarget::GLSL:
            switch (samplerStateType->op)
            {
            case kIROp_SamplerStateType:			stream->emit("sampler");		break;
            case kIROp_SamplerComparisonStateType:	stream->emit("samplerShadow");	break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                break;
            }
            break;
            break;
        }
    }

    void emitStructuredBufferType(IRHLSLStructuredBufferTypeBase* type)
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
        default:
            {
                switch (type->op)
                {
                case kIROp_HLSLStructuredBufferType:                    stream->emit("StructuredBuffer");                   break;
                case kIROp_HLSLRWStructuredBufferType:                  stream->emit("RWStructuredBuffer");                 break;
                case kIROp_HLSLRasterizerOrderedStructuredBufferType:   stream->emit("RasterizerOrderedStructuredBuffer");  break;
                case kIROp_HLSLAppendStructuredBufferType:              stream->emit("AppendStructuredBuffer");             break;
                case kIROp_HLSLConsumeStructuredBufferType:             stream->emit("ConsumeStructuredBuffer");            break;

                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled structured buffer type");
                    break;
                }

                stream->emit("<");
                EmitType(type->getElementType());
                stream->emit(" >");
            }
            break;

        case CodeGenTarget::GLSL:
            // TODO: We desugar global variables with structured-buffer type into GLSL
            // `buffer` declarations, but we don't currently handle structured-buffer types
            // in other contexts (e.g., as function parameters). The simplest thing to do
            // would be to emit a `StructuredBuffer<Foo>` as `Foo[]` and `RWStructuredBuffer<Foo>`
            // as `in out Foo[]`, but that is starting to get into the realm of transformations
            // that should really be handled during legalization, rather than during emission.
            //
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "structured buffer type used unexpectedly");
            break;
        }
    }

    void emitUntypedBufferType(IRUntypedBufferResourceType* type)
    {
        switch(context->target)
        {
        case CodeGenTarget::HLSL:
        default:
            {
                switch (type->op)
                {
                case kIROp_HLSLByteAddressBufferType:                   stream->emit("ByteAddressBuffer");                  break;
                case kIROp_HLSLRWByteAddressBufferType:                 stream->emit("RWByteAddressBuffer");                break;
                case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  stream->emit("RasterizerOrderedByteAddressBuffer"); break;
                case kIROp_RaytracingAccelerationStructureType:         stream->emit("RaytracingAccelerationStructure");    break;

                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                    break;
                }
            }
            break;

        case CodeGenTarget::GLSL:
            {
                switch (type->op)
                {
                case kIROp_RaytracingAccelerationStructureType:
                    requireGLSLExtension("GL_NV_ray_tracing");
                    stream->emit("accelerationStructureNV");
                    break;

                // TODO: These "translations" are obviously wrong for GLSL.
                case kIROp_HLSLByteAddressBufferType:                   stream->emit("ByteAddressBuffer");                  break;
                case kIROp_HLSLRWByteAddressBufferType:                 stream->emit("RWByteAddressBuffer");                break;
                case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  stream->emit("RasterizerOrderedByteAddressBuffer"); break;

                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled buffer type");
                    break;
                }
            }
            break;
        }
    }

    void _requireHalf()
    {
        if (getTarget(context) == CodeGenTarget::GLSL)
        {
            requireGLSLHalfExtension(&context->extensionUsageTracker);
        }
    }

    void emitSimpleTypeImpl(IRType* type)
    {
        switch (type->op)
        {
        default:
            break;

        case kIROp_VoidType:    stream->emit("void");       return;
        case kIROp_BoolType:    stream->emit("bool");       return;

        case kIROp_Int8Type:    stream->emit("int8_t");     return;
        case kIROp_Int16Type:   stream->emit("int16_t");    return;
        case kIROp_IntType:     stream->emit("int");        return;
        case kIROp_Int64Type:   stream->emit("int64_t");    return;

        case kIROp_UInt8Type:   stream->emit("uint8_t");    return;
        case kIROp_UInt16Type:  stream->emit("uint16_t");   return;
        case kIROp_UIntType:    stream->emit("uint");       return;
        case kIROp_UInt64Type:  stream->emit("uint64_t");   return;

        case kIROp_HalfType:
        {
            _requireHalf();
            if (getTarget(context) == CodeGenTarget::GLSL)
            {
                stream->emit("float16_t");
            }
            else
            {
                stream->emit("half");
            }
            return;
        }
        case kIROp_FloatType:   stream->emit("float");      return;
        case kIROp_DoubleType:  stream->emit("double");     return;

        case kIROp_VectorType:
            emitVectorTypeImpl((IRVectorType*)type);
            return;

        case kIROp_MatrixType:
            emitMatrixTypeImpl((IRMatrixType*)type);
            return;

        case kIROp_SamplerStateType:
        case kIROp_SamplerComparisonStateType:
            emitSamplerStateType(cast<IRSamplerStateTypeBase>(type));
            return;

        case kIROp_StructType:
            stream->emit(getIRName(type));
            return;
        }

        // TODO: Ideally the following should be data-driven,
        // based on meta-data attached to the definitions of
        // each of these IR opcodes.

        if (auto texType = as<IRTextureType>(type))
        {
            emitTextureType(texType);
            return;
        }
        else if (auto textureSamplerType = as<IRTextureSamplerType>(type))
        {
            emitTextureSamplerType(textureSamplerType);
            return;
        }
        else if (auto imageType = as<IRGLSLImageType>(type))
        {
            emitImageType(imageType);
            return;
        }
        else if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(type))
        {
            emitStructuredBufferType(structuredBufferType);
            return;
        }
        else if(auto untypedBufferType = as<IRUntypedBufferResourceType>(type))
        {
            emitUntypedBufferType(untypedBufferType);
            return;
        }

        // HACK: As a fallback for HLSL targets, assume that the name of the
        // instruction being used is the same as the name of the HLSL type.
        if(context->target == CodeGenTarget::HLSL)
        {
            auto opInfo = getIROpInfo(type->op);
            stream->emit(opInfo.name);
            UInt operandCount = type->getOperandCount();
            if(operandCount)
            {
                stream->emit("<");
                for(UInt ii = 0; ii < operandCount; ++ii)
                {
                    if(ii != 0) stream->emit(", ");
                    EmitVal(type->getOperand(ii), kEOp_General);
                }
                stream->emit(" >");
            }

            return;
        }

        SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type");
    }

    void emitArrayTypeImpl(IRArrayType* arrayType, EDeclarator* declarator)
    {
        EDeclarator arrayDeclarator;
        arrayDeclarator.flavor = EDeclarator::Flavor::Array;
        arrayDeclarator.next = declarator;
        arrayDeclarator.elementCount = arrayType->getElementCount();

        emitTypeImpl(arrayType->getElementType(), &arrayDeclarator);
    }

    void emitUnsizedArrayTypeImpl(IRUnsizedArrayType* arrayType, EDeclarator* declarator)
    {
        EDeclarator arrayDeclarator;
        arrayDeclarator.flavor = EDeclarator::Flavor::UnsizedArray;
        arrayDeclarator.next = declarator;

        emitTypeImpl(arrayType->getElementType(), &arrayDeclarator);
    }

    void emitTypeImpl(IRType* type, EDeclarator* declarator)
    {
        switch (type->op)
        {
        default:
            emitSimpleTypeImpl(type);
            EmitDeclarator(declarator);
            break;

        case kIROp_RateQualifiedType:
            {
                auto rateQualifiedType = cast<IRRateQualifiedType>(type);
                emitTypeImpl(rateQualifiedType->getValueType(), declarator);
            }
            break;

        case kIROp_ArrayType:
            emitArrayTypeImpl(cast<IRArrayType>(type), declarator);
            break;

        case kIROp_UnsizedArrayType:
            emitUnsizedArrayTypeImpl(cast<IRUnsizedArrayType>(type), declarator);
            break;
        }

    }

    void EmitType(
        IRType*             type,
        SourceLoc const&    typeLoc,
        Name*               name,
        SourceLoc const&    nameLoc)
    {
        stream->advanceToSourceLocation(typeLoc);

        EDeclarator nameDeclarator;
        nameDeclarator.flavor = EDeclarator::Flavor::name;
        nameDeclarator.name = name;
        nameDeclarator.loc = nameLoc;
        emitTypeImpl(type, &nameDeclarator);
    }

    void EmitType(IRType* type, Name* name)
    {
        EmitType(type, SourceLoc(), name, SourceLoc());
    }

    void EmitType(IRType* type, String const& name)
    {
        // HACK: the rest of the code wants a `Name`,
        // so we'll create one for a bit...
        Name tempName;
        tempName.text = name;

        EmitType(type, SourceLoc(), &tempName, SourceLoc());
    }


    void EmitType(IRType* type)
    {
        emitTypeImpl(type, nullptr);
    }

    //
    // Expressions
    //

    bool maybeEmitParens(EOpInfo& outerPrec, EOpInfo prec)
    {
        bool needParens = (prec.leftPrecedence <= outerPrec.leftPrecedence)
            || (prec.rightPrecedence <= outerPrec.rightPrecedence);

        if (needParens)
        {
            stream->emit("(");

            outerPrec = kEOp_None;
        }
        return needParens;
    }

    void maybeCloseParens(bool needClose)
    {
        if(needClose) stream->emit(")");
    }

    bool isTargetIntrinsicModifierApplicable(
        String const& targetName)
    {
        switch(context->target)
        {
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
            return false;

        case CodeGenTarget::CSource: return targetName == "c";
        case CodeGenTarget::CPPSource: return targetName == "cpp";
        case CodeGenTarget::GLSL: return targetName == "glsl";
        case CodeGenTarget::HLSL: return targetName == "hlsl";
        }
    }

    void EmitType(IRType* type, Name* name, SourceLoc const& nameLoc)
    {
        EmitType(
            type,
            SourceLoc(),
            name,
            nameLoc);
    }

    void EmitType(IRType* type, NameLoc const& nameAndLoc)
    {
        EmitType(type, nameAndLoc.name, nameAndLoc.loc);
    }

    bool isTargetIntrinsicModifierApplicable(
        IRTargetIntrinsicDecoration*    decoration)
    {
        auto targetName = String(decoration->getTargetName());

        // If no target name was specified, then the modifier implicitly
        // applies to all targets.
        if(targetName.getLength() == 0)
            return true;

        return isTargetIntrinsicModifierApplicable(targetName);
    }

    void emitStringLiteral(
        String const&   value)
    {
        stream->emit("\"");
        for (auto c : value)
        {
            // TODO: This needs a more complete implementation,
            // especially if we want to support Unicode.

            char buffer[] = { c, 0 };
            switch (c)
            {
            default:
                stream->emit(buffer);
                break;

            case '\"': stream->emit("\\\"");
            case '\'': stream->emit("\\\'");
            case '\\': stream->emit("\\\\");
            case '\n': stream->emit("\\n");
            case '\r': stream->emit("\\r");
            case '\t': stream->emit("\\t");
            }
        }
        stream->emit("\"");
    }

    EOpInfo leftSide(EOpInfo const& outerPrec, EOpInfo const& prec)
    {
        EOpInfo result;
        result.op = nullptr;
        result.leftPrecedence = outerPrec.leftPrecedence;
        result.rightPrecedence = prec.leftPrecedence;
        return result;
    }

    EOpInfo rightSide(EOpInfo const& prec, EOpInfo const& outerPrec)
    {
        EOpInfo result;
        result.op = nullptr;
        result.leftPrecedence = prec.rightPrecedence;
        result.rightPrecedence = outerPrec.rightPrecedence;
        return result;
    }

    void requireGLSLExtension(String const& name)
    {
        Slang::requireGLSLExtension(&context->extensionUsageTracker, name);
    }

    void requireGLSLVersion(ProfileVersion version)
    {
        if (context->target != CodeGenTarget::GLSL)
            return;

        Slang::requireGLSLVersionImpl(&context->extensionUsageTracker, version);
    }

    void requireGLSLVersion(int version)
    {
        switch (version)
        {
    #define CASE(NUMBER) \
        case NUMBER: requireGLSLVersion(ProfileVersion::GLSL_##NUMBER); break

        CASE(110);
        CASE(120);
        CASE(130);
        CASE(140);
        CASE(150);
        CASE(330);
        CASE(400);
        CASE(410);
        CASE(420);
        CASE(430);
        CASE(440);
        CASE(450);

    #undef CASE
        }
    }

    void setSampleRateFlag()
    {
        context->entryPointLayout->flags |= EntryPointLayout::Flag::usesAnySampleRateInput;
    }

    void doSampleRateInputCheck(Name* name)
    {
        auto text = getText(name);
        if (text == "gl_SampleID")
        {
            setSampleRateFlag();
        }
    }

    void EmitVal(
        IRInst*         val,
        EOpInfo const&  outerPrec)
    {
        if(auto type = as<IRType>(val))
        {
            EmitType(type);
        }
        else
        {
            emitIRInstExpr(context, val, IREmitMode::Default, outerPrec);
        }
    }

    typedef unsigned int ESemanticMask;
    enum
    {
        kESemanticMask_None = 0,

        kESemanticMask_NoPackOffset = 1 << 0,

        kESemanticMask_Default = kESemanticMask_NoPackOffset,
    };

    // A chain of variables to use for emitting semantic/layout info
    struct EmitVarChain
    {
        VarLayout*      varLayout;
        EmitVarChain*   next;

        EmitVarChain()
            : varLayout(0)
            , next(0)
        {}

        EmitVarChain(VarLayout* varLayout)
            : varLayout(varLayout)
            , next(0)
        {}

        EmitVarChain(VarLayout* varLayout, EmitVarChain* next)
            : varLayout(varLayout)
            , next(next)
        {}
    };

    UInt getBindingOffset(EmitVarChain* chain, LayoutResourceKind kind)
    {
        UInt offset = 0;
        for(auto cc = chain; cc; cc = cc->next)
        {
            if(auto resInfo = cc->varLayout->FindResourceInfo(kind))
            {
                offset += resInfo->index;
            }
        }
        return offset;
    }

    UInt getBindingSpace(EmitVarChain* chain, LayoutResourceKind kind)
    {
        UInt space = 0;
        for(auto cc = chain; cc; cc = cc->next)
        {
            auto varLayout = cc->varLayout;
            if(auto resInfo = varLayout->FindResourceInfo(kind))
            {
                space += resInfo->space;
            }
            if(auto resInfo = varLayout->FindResourceInfo(LayoutResourceKind::RegisterSpace))
            {
                space += resInfo->index;
            }
        }
        return space;
    }

    // Emit a single `register` semantic, as appropriate for a given resource-type-specific layout info
    void emitHLSLRegisterSemantic(
        LayoutResourceKind  kind,
        EmitVarChain*       chain,
        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
        char const* uniformSemanticSpelling = "register")
    {
        if(!chain)
            return;
        if(!chain->varLayout->FindResourceInfo(kind))
            return;

        UInt index = getBindingOffset(chain, kind);
        UInt space = getBindingSpace(chain, kind);

        switch(kind)
        {
        case LayoutResourceKind::Uniform:
            {
                UInt offset = index;

                // The HLSL `c` register space is logically grouped in 16-byte registers,
                // while we try to traffic in byte offsets. That means we need to pick
                // a register number, based on the starting offset in 16-byte register
                // units, and then a "component" within that register, based on 4-byte
                // offsets from there. We cannot support more fine-grained offsets than that.

                stream->emit(" : ");
                stream->emit(uniformSemanticSpelling);
                stream->emit("(c");

                // Size of a logical `c` register in bytes
                auto registerSize = 16;

                // Size of each component of a logical `c` register, in bytes
                auto componentSize = 4;

                size_t startRegister = offset / registerSize;
                stream->emit(int(startRegister));

                size_t byteOffsetInRegister = offset % registerSize;

                // If this field doesn't start on an even register boundary,
                // then we need to emit additional information to pick the
                // right component to start from
                if (byteOffsetInRegister != 0)
                {
                    // The value had better occupy a whole number of components.
                    SLANG_RELEASE_ASSERT(byteOffsetInRegister % componentSize == 0);

                    size_t startComponent = byteOffsetInRegister / componentSize;

                    static const char* kComponentNames[] = {"x", "y", "z", "w"};
                    stream->emit(".");
                    stream->emit(kComponentNames[startComponent]);
                }
                stream->emit(")");
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
                stream->emit(" : register(");
                switch( kind )
                {
                case LayoutResourceKind::ConstantBuffer:
                    stream->emit("b");
                    break;
                case LayoutResourceKind::ShaderResource:
                    stream->emit("t");
                    break;
                case LayoutResourceKind::UnorderedAccess:
                    stream->emit("u");
                    break;
                case LayoutResourceKind::SamplerState:
                    stream->emit("s");
                    break;
                default:
                    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled HLSL register type");
                    break;
                }
                stream->emit(index);
                if(space)
                {
                    stream->emit(", space");
                    stream->emit(space);
                }
                stream->emit(")");
            }
        }
    }

    // Emit all the `register` semantics that are appropriate for a particular variable layout
    void emitHLSLRegisterSemantics(
        EmitVarChain*       chain,
        char const*         uniformSemanticSpelling = "register")
    {
        if (!chain) return;

        auto layout = chain->varLayout;

        switch( context->target )
        {
        default:
            return;

        case CodeGenTarget::HLSL:
            break;
        }

        for( auto rr : layout->resourceInfos )
        {
            emitHLSLRegisterSemantic(rr.kind, chain, uniformSemanticSpelling);
        }
    }

    void emitHLSLRegisterSemantics(
        VarLayout*  varLayout,
        char const* uniformSemanticSpelling = "register")
    {
        if(!varLayout)
            return;

        EmitVarChain chain(varLayout);
        emitHLSLRegisterSemantics(&chain, uniformSemanticSpelling);
    }

    void emitHLSLParameterGroupFieldLayoutSemantics(
        EmitVarChain*       chain)
    {
        if(!chain)
            return;

        auto layout = chain->varLayout;
        for( auto rr : layout->resourceInfos )
        {
            emitHLSLRegisterSemantic(rr.kind, chain, "packoffset");
        }
    }


    void emitHLSLParameterGroupFieldLayoutSemantics(
        RefPtr<VarLayout>   fieldLayout,
        EmitVarChain*       inChain)
    {
        EmitVarChain chain(fieldLayout, inChain);
        emitHLSLParameterGroupFieldLayoutSemantics(&chain);
    }

    bool emitGLSLLayoutQualifier(
        LayoutResourceKind  kind,
        EmitVarChain*       chain)
    {
        if(!chain)
            return false;
        if(!chain->varLayout->FindResourceInfo(kind))
            return false;

        UInt index = getBindingOffset(chain, kind);
        UInt space = getBindingSpace(chain, kind);
        switch(kind)
        {
        case LayoutResourceKind::Uniform:
            {
                // Explicit offsets require a GLSL extension (which
                // is not universally supported, it seems) or a new
                // enough GLSL version (which we don't want to
                // universally require), so for right now we
                // won't actually output explicit offsets for uniform
                // shader parameters.
                //
                // TODO: We should fix this so that we skip any
                // extra work for parameters that are laid out as
                // expected by the default rules, but do *something*
                // for parameters that need non-default layout.
                //
                // Using the `GL_ARB_enhanced_layouts` feature is one
                // option, but we should also be able to do some
                // things by introducing padding into the declaration
                // (padding insertion would probably be best done at
                // the IR level).
                bool useExplicitOffsets = false;
                if (useExplicitOffsets)
                {
                    requireGLSLExtension("GL_ARB_enhanced_layouts");

                    stream->emit("layout(offset = ");
                    stream->emit(index);
                    stream->emit(")\n");
                }
            }
            break;

        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            stream->emit("layout(location = ");
            stream->emit(index);
            stream->emit(")\n");
            break;

        case LayoutResourceKind::SpecializationConstant:
            stream->emit("layout(constant_id = ");
            stream->emit(index);
            stream->emit(")\n");
            break;

        case LayoutResourceKind::ConstantBuffer:
        case LayoutResourceKind::ShaderResource:
        case LayoutResourceKind::UnorderedAccess:
        case LayoutResourceKind::SamplerState:
        case LayoutResourceKind::DescriptorTableSlot:
            stream->emit("layout(binding = ");
            stream->emit(index);
            if(space)
            {
                stream->emit(", set = ");
                stream->emit(space);
            }
            stream->emit(")\n");
            break;

        case LayoutResourceKind::PushConstantBuffer:
            stream->emit("layout(push_constant)\n");
            break;
        case LayoutResourceKind::ShaderRecord:
            stream->emit("layout(shaderRecordNV)\n");
            break;

        }
        return true;
    }

    void emitGLSLLayoutQualifiers(
        RefPtr<VarLayout>               layout,
        EmitVarChain*                   inChain,
        LayoutResourceKind              filter = LayoutResourceKind::None)
    {
        if(!layout) return;

        switch( context->target )
        {
        default:
            return;

        case CodeGenTarget::GLSL:
            break;
        }

        EmitVarChain chain(layout, inChain);

        for( auto info : layout->resourceInfos )
        {
            // Skip info that doesn't match our filter
            if (filter != LayoutResourceKind::None
                && filter != info.kind)
            {
                continue;
            }

            emitGLSLLayoutQualifier(info.kind, &chain);
        }
    }

    void emitGLSLVersionDirective()
    {
        auto effectiveProfile = context->effectiveProfile;
        if(effectiveProfile.getFamily() == ProfileFamily::GLSL)
        {
            requireGLSLVersion(effectiveProfile.GetVersion());
        }

        // HACK: We aren't picking GLSL versions carefully right now,
        // and so we might end up only requiring the initial 1.10 version,
        // even though even basic functionality needs a higher version.
        //
        // For now, we'll work around this by just setting the minimum required
        // version to a high one:
        //
        // TODO: Either correctly compute a minimum required version, or require
        // the user to specify a version as part of the target.
        requireGLSLVersionImpl(&context->extensionUsageTracker, ProfileVersion::GLSL_450);

        auto requiredProfileVersion = context->extensionUsageTracker.profileVersion;
        switch (requiredProfileVersion)
        {
#define CASE(TAG, VALUE)    \
        case ProfileVersion::TAG: stream->emit("#version " #VALUE "\n"); return

        CASE(GLSL_110, 110);
        CASE(GLSL_120, 120);
        CASE(GLSL_130, 130);
        CASE(GLSL_140, 140);
        CASE(GLSL_150, 150);
        CASE(GLSL_330, 330);
        CASE(GLSL_400, 400);
        CASE(GLSL_410, 410);
        CASE(GLSL_420, 420);
        CASE(GLSL_430, 430);
        CASE(GLSL_440, 440);
        CASE(GLSL_450, 450);
        CASE(GLSL_460, 460);
#undef CASE

        default:
            break;
        }

        // No information is available for us to guess a profile,
        // so it seems like we need to pick one out of thin air.
        //
        // Ideally we should infer a minimum required version based
        // on the constructs we have seen used in the user's code
        //
        // For now we just fall back to a reasonably recent version.

        stream->emit("#version 420\n");
    }

    void emitGLSLPreprocessorDirectives()
    {
        switch(context->target)
        {
        // Don't emit this stuff unless we are targetting GLSL
        default:
            return;

        case CodeGenTarget::GLSL:
            break;
        }

        emitGLSLVersionDirective();
    }

    /// Emit directives to control overall layout computation for the emitted code.
    void emitLayoutDirectives(TargetRequest* targetReq)
    {
        // We are going to emit the target-language-specific directives
        // needed to get the default matrix layout to match what was requested
        // for the given target.
        //
        // Note: we do not rely on the defaults for the target language,
        // because a user could take the HLSL/GLSL generated by Slang and pass
        // it to another compiler with non-default options specified on
        // the command line, leading to all kinds of trouble.
        //
        // TODO: We need an approach to "global" layout directives that will work
        // in the presence of multiple modules. If modules A and B were each
        // compiled with different assumptions about how layout is performed,
        // then types/variables defined in those modules should be emitted in
        // a way that is consistent with that layout...

        auto matrixLayoutMode = targetReq->getDefaultMatrixLayoutMode();

        switch(context->target)
        {
        default:
            return;

        case CodeGenTarget::GLSL:
            // Reminder: the meaning of row/column major layout
            // in our semantics is the *opposite* of what GLSL
            // calls them, because what they call "columns"
            // are what we call "rows."
            //
            switch(matrixLayoutMode)
            {
            case kMatrixLayoutMode_RowMajor:
            default:
                stream->emit("layout(column_major) uniform;\n");
                stream->emit("layout(column_major) buffer;\n");
                break;

            case kMatrixLayoutMode_ColumnMajor:
                stream->emit("layout(row_major) uniform;\n");
                stream->emit("layout(row_major) buffer;\n");
                break;
            }
            break;

        case CodeGenTarget::HLSL:
            switch(matrixLayoutMode)
            {
            case kMatrixLayoutMode_RowMajor:
            default:
                stream->emit("#pragma pack_matrix(row_major)\n");
                break;

            case kMatrixLayoutMode_ColumnMajor:
                stream->emit("#pragma pack_matrix(column_major)\n");
                break;
            }
            break;
        }
    }

    // Utility code for generating unique IDs as needed
    // during the emit process (e.g., for declarations
    // that didn't origianlly have names, but now need to).

    UInt allocateUniqueID()
    {
        return context->uniqueIDCounter++;
    }

    // IR-level emit logc

    UInt getID(IRInst* value)
    {
        auto& mapIRValueToID = context->mapIRValueToID;

        UInt id = 0;
        if (mapIRValueToID.TryGetValue(value, id))
            return id;

        id = allocateUniqueID();
        mapIRValueToID.Add(value, id);
        return id;
    }

    /// "Scrub" a name so that it complies with restrictions of the target language.
    String scrubName(
        String const& name)
    {
        // We will use a plain `U` as a dummy character to insert
        // whenever we need to insert things to make a string into
        // valid name.
        //
        char const* dummyChar = "U";

        // Special case a name that is the empty string, just in case.
        if(name.getLength() == 0)
            return dummyChar;

        // Otherwise, we are going to walk over the name byte by byte
        // and write some legal characters to the output as we go.
        StringBuilder sb;

        if(getTarget(context) == CodeGenTarget::GLSL)
        {
            // GLSL reserverse all names that start with `gl_`,
            // so if we are in danger of collision, then make
            // our name start with a dummy character instead.
            if(name.startsWith("gl_"))
            {
                sb.append(dummyChar);
            }
        }

        // We will also detect user-defined names that
        // might overlap with our convention for mangled names,
        // to avoid an possible collision.
        if(name.startsWith("_S"))
        {
            sb.Append(dummyChar);
        }

        // TODO: This is where we might want to consult
        // a dictionary of reserved words for the chosen target
        //
        //  if(isReservedWord(name)) { sb.Append(dummyChar); }
        //

        // We need to track the previous byte in
        // order to detect consecutive underscores for GLSL.
        int prevChar = -1;

        for(auto c : name)
        {
            // We will treat a dot character just like an underscore
            // for the purposes of producing a scrubbed name, so
            // that we translate `SomeType.someMethod` into
            // `SomeType_someMethod`.
            //
            // By handling this case at the top of this loop, we
            // ensure that a `.`-turned-`_` is handled just like
            // a `_` in the original name, and will be properly
            // scrubbed for GLSL output.
            //
            if(c == '.')
            {
                c = '_';
            }

            if(((c >= 'a') && (c <= 'z'))
                || ((c >= 'A') && (c <= 'Z')))
            {
                // Ordinary ASCII alphabetic characters are assumed
                // to always be okay.
            }
            else if((c >= '0') && (c <= '9'))
            {
                // We don't want to allow a digit as the first
                // byte in a name, since the result wouldn't
                // be a valid identifier in many target languages.
                if(prevChar == -1)
                {
                    sb.append(dummyChar);
                }
            }
            else if(c == '_')
            {
                // We will collapse any consecutive sequence of `_`
                // characters into a single one (this means that
                // some names that were unique in the original
                // code might not resolve to unique names after
                // scrubbing, but that was true in general).

                if(prevChar == '_')
                {
                    // Skip this underscore, so we don't output
                    // more than one in a row.
                    continue;
                }
            }
            else
            {
                // If we run into a character that wouldn't normally
                // be allowed in an identifier, we need to translate
                // it into something that *is* valid.
                //
                // Our solution for now will be very clumsy: we will
                // emit `x` and then the hexadecimal version of
                // the byte we were given.
                sb.append("x");
                sb.append(uint32_t((unsigned char) c), 16);

                // We don't want to apply the default handling below,
                // so skip to the top of the loop now.
                prevChar = c;
                continue;
            }

            sb.append(c);
            prevChar = c;
        }

        return sb.ProduceString();
    }

    String generateIRName(
        IRInst* inst)
    {
        // If the instruction names something
        // that should be emitted as a target intrinsic,
        // then use that name instead.
        if(auto intrinsicDecoration = findTargetIntrinsicDecoration(context, inst))
        {
            return String(intrinsicDecoration->getDefinition());
        }

        // If we have a name hint on the instruction, then we will try to use that
        // to provide the actual name in the output code.
        //
        // We need to be careful that the name follows the rules of the target language,
        // so there is a "scrubbing" step that needs to be applied here.
        //
        // We also need to make sure that the name won't collide with other declarations
        // that might have the same name hint applied, so we will still unique
        // them by appending the numeric ID of the instruction.
        //
        // TODO: Find cases where we can drop the suffix safely.
        //
        // TODO: When we start having to handle symbols with external linkage for
        // things like DXIL libraries, we will need to *not* use the friendly
        // names for stuff that should be link-able.
        //
        if(auto nameHintDecoration = inst->findDecoration<IRNameHintDecoration>())
        {
            // The name we output will basically be:
            //
            //      <nameHint>_<uniqueID>
            //
            // Except that we will "scrub" the name hint first,
            // and we will omit the underscore if the (scrubbed)
            // name hint already ends with one.
            //

            String nameHint = nameHintDecoration->getName();
            nameHint = scrubName(nameHint);

            StringBuilder sb;
            sb.append(nameHint);

            // Avoid introducing a double underscore
            if(!nameHint.endsWith("_"))
            {
                sb.append("_");
            }

            String key = sb.ProduceString();
            UInt count = 0;
            context->uniqueNameCounters.TryGetValue(key, count);

            context->uniqueNameCounters[key] = count+1;

            sb.append(Int32(count));
            return sb.ProduceString();
        }




        // If the instruction has a mangled name, then emit using that.
        if(auto linkageDecoration = inst->findDecoration<IRLinkageDecoration>())
        {
            return linkageDecoration->getMangledName();
        }

        // Otherwise fall back to a construct temporary name
        // for the instruction.
        StringBuilder sb;
        sb << "_S";
        sb << Int32(getID(inst));

        return sb.ProduceString();
    }

    String getIRName(
        IRInst*        inst)
    {
        String name;
        if(!context->mapInstToName.TryGetValue(inst, name))
        {
            name = generateIRName(inst);
            context->mapInstToName.Add(inst, name);
        }
        return name;
    }

    struct IRDeclaratorInfo
    {
        enum class Flavor
        {
            Simple,
            Ptr,
            Array,
        };

        Flavor flavor;
        IRDeclaratorInfo*   next;
        union
        {
            String const* name;
            IRInst* elementCount;
        };
    };

    void emitDeclarator(
        SharedEmitContext*    ctx,
        IRDeclaratorInfo*   declarator)
    {
        if(!declarator)
            return;

        switch( declarator->flavor )
        {
        case IRDeclaratorInfo::Flavor::Simple:
            stream->emit(" ");
            stream->emit(*declarator->name);
            break;

        case IRDeclaratorInfo::Flavor::Ptr:
            stream->emit("*");
            emitDeclarator(ctx, declarator->next);
            break;

        case IRDeclaratorInfo::Flavor::Array:
            emitDeclarator(ctx, declarator->next);
            stream->emit("[");
            emitIROperand(ctx, declarator->elementCount, IREmitMode::Default, kEOp_General);
            stream->emit("]");
            break;
        }
    }

    void emitIRSimpleValue(
        SharedEmitContext*    /*context*/,
        IRInst*         inst)
    {
        switch(inst->op)
        {
        case kIROp_IntLit:
            stream->emit(((IRConstant*) inst)->value.intVal);
            break;

        case kIROp_FloatLit:
            stream->emit(((IRConstant*) inst)->value.floatVal);
            break;

        case kIROp_BoolLit:
            {
                bool val = ((IRConstant*)inst)->value.intVal != 0;
                stream->emit(val ? "true" : "false");
            }
            break;

        default:
            SLANG_UNIMPLEMENTED_X("val case for emit");
            break;
        }

    }

    CodeGenTarget getTarget(SharedEmitContext* ctx)
    {
        return ctx->target;
    }

    // Hack to allow IR emit for global constant to override behavior
    enum class IREmitMode
    {
        Default,
        GlobalConstant,
    };

    bool shouldFoldIRInstIntoUseSites(
        SharedEmitContext*    ctx,
        IRInst*        inst,
        IREmitMode      mode)
    {
        // Certain opcodes should never/always be folded in
        switch( inst->op )
        {
        default:
            break;

        // Never fold these in, because they represent declarations
        //
        case kIROp_Var:
        case kIROp_GlobalVar:
        case kIROp_GlobalConstant:
        case kIROp_GlobalParam:
        case kIROp_Param:
        case kIROp_Func:
            return false;

        // Always fold these in, because they are trivial
        //
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_BoolLit:
            return true;

        // Always fold these in, because their results
        // cannot be represented in the type system of
        // our current targets.
        //
        // TODO: when we add C/C++ as an optional target,
        // we could consider lowering insts that result
        // in pointers directly.
        //
        case kIROp_FieldAddress:
        case kIROp_getElementPtr:
        case kIROp_Specialize:
            return true;
        }

        // Always fold when we are inside a global constant initializer
        if (mode == IREmitMode::GlobalConstant)
            return true;

        switch( inst->op )
        {
        default:
            break;

        // HACK: don't fold these in because we currently lower
        // them to initializer lists, which aren't allowed in
        // general expression contexts.
        //
        // Note: we are doing this check *after* the check for `GlobalConstant`
        // mode, because otherwise we'd fail to emit initializer lists in
        // the main place where we want/need them.
        //
        case kIROp_makeStruct:
        case kIROp_makeArray:
            return false;

        }

        // Instructions with specific result *types* will usually
        // want to be folded in, because they aren't allowed as types
        // for temporary variables.
        auto type = inst->getDataType();

        // Unwrap any layers of array-ness from the type, so that
        // we can look at the underlying data type, in case we
        // should *never* expose a value of that type
        while (auto arrayType = as<IRArrayTypeBase>(type))
        {
            type = arrayType->getElementType();
        }

        // Don't allow temporaries of pointer types to be created.
        if(as<IRPtrTypeBase>(type))
        {
            return true;
        }

        // First we check for uniform parameter groups,
        // because a `cbuffer` or GLSL `uniform` block
        // does not have a first-class type that we can
        // pass around.
        //
        // TODO: We need to ensure that type legalization
        // cleans up cases where we use a parameter group
        // or parameter block type as a function parameter...
        //
        if(as<IRUniformParameterGroupType>(type))
        {
            // TODO: we need to be careful here, because
            // HLSL shader model 6 allows these as explicit
            // types.
            return true;
        }
        //
        // The stream-output and patch types need to be handled
        // too, because they are not really first class (especially
        // not in GLSL, but they also seem to confuse the HLSL
        // compiler when they get used as temporaries).
        //
        else if (as<IRHLSLStreamOutputType>(type))
        {
            return true;
        }
        else if (as<IRHLSLPatchType>(type))
        {
            return true;
        }


        // GLSL doesn't allow texture/resource types to
        // be used as first-class values, so we need
        // to fold them into their use sites in all cases
        if (getTarget(ctx) == CodeGenTarget::GLSL)
        {
            if(as<IRResourceTypeBase>(type))
            {
                return true;
            }
            else if(as<IRHLSLStructuredBufferTypeBase>(type))
            {
                return true;
            }
            else if(as<IRUntypedBufferResourceType>(type))
            {
                return true;
            }
            else if(as<IRSamplerStateTypeBase>(type))
            {
                return true;
            }
        }

        // If the instruction is at global scope, then it might represent
        // a constant (e.g., the value of an enum case).
        //
        if(as<IRModuleInst>(inst->getParent()))
        {
            if(!inst->mightHaveSideEffects())
                return true;
        }

        // Having dealt with all of the cases where we *must* fold things
        // above, we can now deal with the more general cases where we
        // *should not* fold things.

        // Don't fold somethin with no users:
        if(!inst->hasUses())
            return false;

        // Don't fold something that has multiple users:
        if(inst->hasMoreThanOneUse())
            return false;

        // Don't fold something that might have side effects:
        if(inst->mightHaveSideEffects())
            return false;

        // Don't fold instructions that are marked `[precise]`.
        // This could in principle be extended to any other
        // decorations that affect the semantics of an instruction
        // in ways that require a temporary to be introduced.
        //
        if(inst->findDecoration<IRPreciseDecoration>())
            return false;

        // Okay, at this point we know our instruction must have a single use.
        auto use = inst->firstUse;
        SLANG_ASSERT(use);
        SLANG_ASSERT(!use->nextUse);

        auto user = use->getUser();

        // We'd like to figure out if it is safe to fold our instruction into `user`

        // First, let's make sure they are in the same block/parent:
        if(inst->getParent() != user->getParent())
            return false;

        // Now let's look at all the instructions between this instruction
        // and the user. If any of them might have side effects, then lets
        // bail out now.
        for(auto ii = inst->getNextInst(); ii != user; ii = ii->getNextInst())
        {
            if(!ii)
            {
                // We somehow reached the end of the block without finding
                // the user, which doesn't make sense if uses dominate
                // defs. Let's just play it safe and bail out.
                return false;
            }

            if(ii->mightHaveSideEffects())
                return false;
        }

        // Okay, if we reach this point then the user comes later in
        // the same block, and there are no instructions with side
        // effects in between, so it seems safe to fold things in.
        return true;
    }

    void emitIROperand(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode,
        EOpInfo const&  outerPrec)
    {
        if( shouldFoldIRInstIntoUseSites(ctx, inst, mode) )
        {
            emitIRInstExpr(ctx, inst, mode, outerPrec);
            return;
        }

        switch(inst->op)
        {
        case 0: // nothing yet
        default:
            stream->emit(getIRName(inst));
            break;
        }
    }

    void emitIRArgs(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode)
    {
        UInt argCount = inst->getOperandCount();
        IRUse* args = inst->getOperands();

        stream->emit("(");
        for(UInt aa = 0; aa < argCount; ++aa)
        {
            if(aa != 0) stream->emit(", ");
            emitIROperand(ctx, args[aa].get(), mode, kEOp_General);
        }
        stream->emit(")");
    }

    void emitIRType(
        SharedEmitContext*    /*context*/,
        IRType*         type,
        String const&   name)
    {
        EmitType(type, name);
    }

    void emitIRType(
        SharedEmitContext*    /*context*/,
        IRType*         type,
        Name*           name)
    {
        EmitType(type, name);
    }

    void emitIRType(
        SharedEmitContext*    /*context*/,
        IRType*         type)
    {
        EmitType(type);
    }

    void emitIRRateQualifiers(
        SharedEmitContext*    ctx,
        IRRate*         rate)
    {
        if(!rate) return;

        if(as<IRConstExprRate>(rate))
        {
            switch( getTarget(ctx) )
            {
            case CodeGenTarget::GLSL:
                stream->emit("const ");
                break;

            default:
                break;
            }
        }

        if (as<IRGroupSharedRate>(rate))
        {
            switch( getTarget(ctx) )
            {
            case CodeGenTarget::HLSL:
                stream->emit("groupshared ");
                break;

            case CodeGenTarget::GLSL:
                stream->emit("shared ");
                break;

            default:
                break;
            }
        }
    }

    void emitIRRateQualifiers(
        SharedEmitContext*    ctx,
        IRInst*         value)
    {
        emitIRRateQualifiers(ctx, value->getRate());
    }


    void emitIRInstResultDecl(
        SharedEmitContext*    ctx,
        IRInst*         inst)
    {
        auto type = inst->getDataType();
        if(!type)
            return;

        if (as<IRVoidType>(type))
            return;

        emitIRTempModifiers(ctx, inst);

        emitIRRateQualifiers(ctx, inst);

        emitIRType(ctx, type, getIRName(inst));
        stream->emit(" = ");
    }

    class UnmangleContext
    {
    private:
        char const* cursor_  = nullptr;
        char const* begin_   = nullptr;
        char const* end_     = nullptr;

        bool isDigit(char c)
        {
            return (c >= '0') && (c <= '9');
        }

        char peek()
        {
            return *cursor_;
        }

        char get()
        {
            return *cursor_++;
        }

        void expect(char c)
        {
            if(peek() == c)
            {
                get();
            }
            else
            {
                // ERROR!
                SLANG_UNEXPECTED("mangled name error");
            }
        }

        void expect(char const* str)
        {
            while(char c = *str++)
                expect(c);
        }

    public:
        UnmangleContext()
        {}

        UnmangleContext(String const& str)
            : cursor_(str.begin())
            , begin_(str.begin())
            , end_(str.end())
        {}

        // Call at the beginning of a mangled name,
        // to strip off the main prefix
        void startUnmangling()
        {
            expect("_S");
        }

        UInt readCount()
        {
            int c = peek();
            if(!isDigit((char)c))
            {
                SLANG_UNEXPECTED("bad name mangling");
                UNREACHABLE_RETURN(0);
            }
            get();

            if(c == '0')
                return 0;

            UInt count = 0;
            for(;;)
            {
                count = count*10 + c - '0';
                c = peek();
                if(!isDigit((char)c))
                    return count;

                get();
            }
        }

        void readGenericParam()
        {
            switch(peek())
            {
            case 'T':
            case 'C':
                get();
                break;

            case 'v':
                get();
                readType();
                break;

            default:
                SLANG_UNEXPECTED("bad name mangling");
                break;
            }
        }

        void readGenericParams()
        {
            expect("g");
            UInt paramCount = readCount();
            for(UInt pp = 0; pp < paramCount; pp++)
            {
                readGenericParam();
            }
        }

        void readSimpleIntVal()
        {
            int c = peek();
            if(isDigit((char)c))
            {
                get();
            }
            else
            {
                readVal();
            }
        }


        UnownedStringSlice readRawStringSegment()
        {
            // Read the length part
            UInt count = readCount();
            if(count > UInt(end_ - cursor_))
            {
                SLANG_UNEXPECTED("bad name mangling");
                UNREACHABLE_RETURN(UnownedStringSlice());
            }

            auto result = UnownedStringSlice(cursor_, cursor_ + count);
            cursor_ += count;
            return result;
        }

        void readNamedType()
        {
            // TODO: handle types with more complicated names
            readRawStringSegment();
        }

        void readType()
        {
            int c = peek();
            switch(c)
            {
            case 'V':
            case 'b':
            case 'i':
            case 'u':
            case 'U':
            case 'h':
            case 'f':
            case 'd':
                get();
                break;

            case 'v':
                get();
                readSimpleIntVal();
                readType();
                break;

            default:
                readNamedType();
                break;
            }
        }

        void readVal()
        {
            switch(peek())
            {
            case 'k':
                get();
                readCount();
                break;

            case 'K':
                get();
                readRawStringSegment();
                break;

            default:
                readType();
                break;
            }

        }

        void readGenericArg()
        {
            readVal();
        }

        void readGenericArgs()
        {
            expect("G");
            UInt argCount = readCount();
            for(UInt aa = 0; aa < argCount; aa++)
            {
                readGenericArg();
            }
        }

        void readExtensionSpec()
        {
            expect("X");
            readType();
        }

        UnownedStringSlice readSimpleName()
        {
            UnownedStringSlice result;
            for(;;)
            {
                int c = peek();

                if(c == 'g')
                {
                    readGenericParams();
                    continue;
                }
                else if(c == 'G')
                {
                    readGenericArgs();
                    continue;
                }
                else if(c == 'X')
                {
                    readExtensionSpec();
                    continue;
                }

                if(!isDigit((char)c))
                    return result;

                // Read the length part
                UInt count = readCount();
                if(count > UInt(end_ - cursor_))
                {
                    SLANG_UNEXPECTED("bad name mangling");
                    UNREACHABLE_RETURN(result);
                }

                result = UnownedStringSlice(cursor_, cursor_ + count);
                cursor_ += count;
            }
        }

        UInt readParamCount()
        {
            expect("p");
            UInt count = readCount();
            expect("p");
            return count;
        }
    };

    IRTargetIntrinsicDecoration* findTargetIntrinsicDecoration(
        SharedEmitContext*    /* ctx */,
        IRInst*         inst)
    {
        for(auto dd : inst->getDecorations())
        {
            if (dd->op != kIROp_TargetIntrinsicDecoration)
                continue;

            auto targetIntrinsic = (IRTargetIntrinsicDecoration*)dd;
            if (isTargetIntrinsicModifierApplicable(targetIntrinsic))
                return targetIntrinsic;
        }

        return nullptr;
    }

    // Check if the string being used to define a target intrinsic
    // is an "ordinary" name, such that we can simply emit a call
    // to the new name with the arguments of the old operation.
    bool isOrdinaryName(String const& name)
    {
        char const* cursor = name.begin();
        char const* end = name.end();

        while(cursor != end)
        {
            int c = *cursor++;
            if( (c >= 'a') && (c <= 'z') ) continue;
            if( (c >= 'A') && (c <= 'Z') ) continue;
            if( c == '_' ) continue;

            return false;
        }
        return true;
    }

    void emitTargetIntrinsicCallExpr(
        SharedEmitContext*                    ctx,
        IRCall*                         inst,
        IRFunc*                         /* func */,
        IRTargetIntrinsicDecoration*    targetIntrinsic,
        IREmitMode                      mode,
        EOpInfo const&                  inOuterPrec)
    {
        auto outerPrec = inOuterPrec;

        IRUse* args = inst->getOperands();
        Index argCount = inst->getOperandCount();

        // First operand was the function to be called
        args++;
        argCount--;

        auto name = String(targetIntrinsic->getDefinition());

        if(isOrdinaryName(name))
        {
            // Simple case: it is just an ordinary name, so we call it like a builtin.
            auto prec = kEOp_Postfix;
            bool needClose = maybeEmitParens(outerPrec, prec);

            stream->emit(name);
            stream->emit("(");
            for (Index aa = 0; aa < argCount; ++aa)
            {
                if (aa != 0) stream->emit(", ");
                emitIROperand(ctx, args[aa].get(), mode, kEOp_General);
            }
            stream->emit(")");

            maybeCloseParens(needClose);
            return;
        }
        else
        {
            int openParenCount = 0;

            const auto returnType = inst->getDataType();

            // If it returns void -> then we don't need parenthesis 
            if (as<IRVoidType>(returnType) == nullptr)
            {
                stream->emit("(");
                openParenCount++;
            }

            // General case: we are going to emit some more complex text.

            char const* cursor = name.begin();
            char const* end = name.end();
            while(cursor != end)
            {
                char c = *cursor++;
                if( c != '$' )
                {
                    // Not an escape sequence
                    stream->emitRawTextSpan(&c, &c+1);
                    continue;
                }

                SLANG_RELEASE_ASSERT(cursor != end);

                char d = *cursor++;

                switch (d)
                {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    {
                        // Simple case: emit one of the direct arguments to the call
                        Index argIndex = d - '0';
                        SLANG_RELEASE_ASSERT((0 <= argIndex) && (argIndex < argCount));
                        stream->emit("(");
                        emitIROperand(ctx, args[argIndex].get(), mode, kEOp_General);
                        stream->emit(")");
                    }
                    break;

                case 'p':
                    {
                        // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                        // then this form will pair up the t and s arguments as needed for a GLSL
                        // texturing operation.
                        SLANG_RELEASE_ASSERT(argCount >= 2);

                        auto textureArg = args[0].get();
                        auto samplerArg = args[1].get();

                        if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                        {
                            emitGLSLTextureOrTextureSamplerType(baseTextureType, "sampler");

                            if (auto samplerType = as<IRSamplerStateTypeBase>(samplerArg->getDataType()))
                            {
                                if (as<IRSamplerComparisonStateType>(samplerType))
                                {
                                    stream->emit("Shadow");
                                }
                            }

                            stream->emit("(");
                            emitIROperand(ctx, textureArg, mode, kEOp_General);
                            stream->emit(",");
                            emitIROperand(ctx, samplerArg, mode, kEOp_General);
                            stream->emit(")");
                        }
                        else
                        {
                            SLANG_UNEXPECTED("bad format in intrinsic definition");
                        }
                    }
                    break;

                case 'c':
                    {
                        // When doing texture access in glsl the result may need to be cast.
                        // In particular if the underlying texture is 'half' based, glsl only accesses (read/write)
                        // as float. So we need to cast to a half type on output.
                        // When storing into a texture it is still the case the value written must be half - but
                        // we don't need to do any casting there as half is coerced to float without a problem.
                        SLANG_RELEASE_ASSERT(argCount >= 1);
                        
                        auto textureArg = args[0].get();
                        if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                        {
                            auto elementType = baseTextureType->getElementType();
                            IRBasicType* underlyingType = nullptr;
                            if (auto basicType = as<IRBasicType>(elementType))
                            {
                                underlyingType = basicType;
                            }
                            else if (auto vectorType = as<IRVectorType>(elementType))
                            {
                                underlyingType = as<IRBasicType>(vectorType->getElementType());
                            }

                            // We only need to output a cast if the underlying type is half.
                            if (underlyingType && underlyingType->op == kIROp_HalfType)
                            {
                                emitSimpleTypeImpl(elementType);
                                stream->emit("(");
                                openParenCount++;
                            }
                        }    
                    }
                    break;

                case 'z':
                    {
                        // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                        // where `t` is a `Texture*<T>`, then this is the step where we try to
                        // properly swizzle the output of the equivalent GLSL call into the right
                        // shape.
                        SLANG_RELEASE_ASSERT(argCount >= 1);

                        auto textureArg = args[0].get();
                        if (auto baseTextureType = as<IRTextureType>(textureArg->getDataType()))
                        {
                            auto elementType = baseTextureType->getElementType();
                            if (auto basicType = as<IRBasicType>(elementType))
                            {
                                // A scalar result is expected
                                stream->emit(".x");
                            }
                            else if (auto vectorType = as<IRVectorType>(elementType))
                            {
                                // A vector result is expected
                                auto elementCount = GetIntVal(vectorType->getElementCount());

                                if (elementCount < 4)
                                {
                                    char const* swiz[] = { "", ".x", ".xy", ".xyz", "" };
                                    stream->emit(swiz[elementCount]);
                                }
                            }
                            else
                            {
                                // What other cases are possible?
                            }
                        }
                        else
                        {
                            SLANG_UNEXPECTED("bad format in intrinsic definition");
                        }
                    }
                    break;

                case 'N':
                    {
                        // Extract the element count from a vector argument so that
                        // we can use it in the constructed expression.

                        SLANG_RELEASE_ASSERT(*cursor >= '0' && *cursor <= '9');
                        Index argIndex = (*cursor++) - '0';
                        SLANG_RELEASE_ASSERT(argCount > argIndex);

                        auto vectorArg = args[argIndex].get();
                        if (auto vectorType = as<IRVectorType>(vectorArg->getDataType()))
                        {
                            auto elementCount = GetIntVal(vectorType->getElementCount());
                            stream->emit(elementCount);
                        }
                        else
                        {
                            SLANG_UNEXPECTED("bad format in intrinsic definition");
                        }
                    }
                    break;

                case 'V':
                    {
                        // Take an argument of some scalar/vector type and pad
                        // it out to a 4-vector with the same element type
                        // (this is the inverse of `$z`).
                        //
                        SLANG_RELEASE_ASSERT(*cursor >= '0' && *cursor <= '9');
                        Index argIndex = (*cursor++) - '0';
                        SLANG_RELEASE_ASSERT(argCount > argIndex);

                        auto arg = args[argIndex].get();
                        IRIntegerValue elementCount = 1;
                        IRType* elementType = arg->getDataType();
                        if (auto vectorType = as<IRVectorType>(elementType))
                        {
                            elementCount = GetIntVal(vectorType->getElementCount());
                            elementType = vectorType->getElementType();
                        }

                        if(elementCount == 4)
                        {
                            // In the simple case, the operand is already a 4-vector,
                            // so we can just emit it as-is.
                            emitIROperand(ctx, arg, mode, kEOp_General);
                        }
                        else
                        {
                            // Otherwise, we need to construct a 4-vector from the
                            // value we have, padding it out with zero elements as
                            // needed.
                            //
                            emitVectorTypeName(elementType, 4);
                            stream->emit("(");
                            emitIROperand(ctx, arg, mode, kEOp_General);
                            for(IRIntegerValue ii = elementCount; ii < 4; ++ii)
                            {
                                stream->emit(", ");
                                if(getTarget(ctx) == CodeGenTarget::GLSL)
                                {
                                    emitSimpleTypeImpl(elementType);
                                    stream->emit("(0)");
                                }
                                else
                                {
                                    stream->emit("0");
                                }
                            }
                            stream->emit(")");
                        }
                    }
                    break;

                case 'a':
                    {
                        // We have an operation that needs to lower to either
                        // `atomic*` or `imageAtomic*` for GLSL, depending on
                        // whether its first operand is a subscript into an
                        // array. This `$a` is the first `a` in `atomic`,
                        // so we will replace it accordingly.
                        //
                        // TODO: This distinction should be made earlier,
                        // with the front-end picking the right overload
                        // based on the "address space" of the argument.

                        Index argIndex = 0;
                        SLANG_RELEASE_ASSERT(argCount > argIndex);

                        auto arg = args[argIndex].get();
                        if(arg->op == kIROp_ImageSubscript)
                        {
                            stream->emit("imageA");
                        }
                        else
                        {
                            stream->emit("a");
                        }
                    }
                    break;

                case 'A':
                    {
                        // We have an operand that represents the destination
                        // of an atomic operation in GLSL, and it should
                        // be lowered based on whether it is an ordinary l-value,
                        // or an image subscript. In the image subscript case
                        // this operand will turn into multiple arguments
                        // to the `imageAtomic*` function.
                        //

                        Index argIndex = 0;
                        SLANG_RELEASE_ASSERT(argCount > argIndex);

                        auto arg = args[argIndex].get();
                        if(arg->op == kIROp_ImageSubscript)
                        {
                            if(getTarget(ctx) == CodeGenTarget::GLSL)
                            {
                                // TODO: we don't handle the multisample
                                // case correctly here, where the last
                                // component of the image coordinate needs
                                // to be broken out into its own argument.
                                //
                                stream->emit("(");
                                emitIROperand(ctx, arg->getOperand(0), mode, kEOp_General);
                                stream->emit("), ");

                                // The coordinate argument will have been computed
                                // as a `vector<uint, N>` because that is how the
                                // HLSL image subscript operations are defined.
                                // In contrast, the GLSL `imageAtomic*` operations
                                // expect `vector<int, N>` coordinates, so we
                                // hill hackily insert the conversion here as
                                // part of the intrinsic op.
                                //
                                auto coords = arg->getOperand(1);
                                auto coordsType = coords->getDataType();

                                auto coordsVecType = as<IRVectorType>(coordsType);
                                IRIntegerValue elementCount = 1;
                                if(coordsVecType)
                                {
                                    coordsType = coordsVecType->getElementType();
                                    elementCount = GetIntVal(coordsVecType->getElementCount());
                                }

                                SLANG_ASSERT(coordsType->op == kIROp_UIntType);

                                if (elementCount > 1)
                                {
                                    stream->emit("ivec");
                                    stream->emit(elementCount);
                                }
                                else
                                {
                                    stream->emit("int");
                                }

                                stream->emit("(");
                                emitIROperand(ctx, arg->getOperand(1), mode, kEOp_General);
                                stream->emit(")");
                            }
                            else
                            {
                                stream->emit("(");
                                emitIROperand(ctx, arg, mode, kEOp_General);
                                stream->emit(")");
                            }
                        }
                        else
                        {
                            stream->emit("(");
                            emitIROperand(ctx, arg, mode, kEOp_General);
                            stream->emit(")");
                        }
                    }
                    break;

                // We will use the `$X` case as a prefix for
                // special logic needed when cross-compiling ray-tracing
                // shaders.
                case 'X':
                    {
                        SLANG_RELEASE_ASSERT(*cursor);
                        switch(*cursor++)
                        {
                        case 'P':
                            {
                                // The `$XP` case handles looking up
                                // the associated `location` for a variable
                                // used as the argument ray payload at a
                                // trace call site.

                                Index argIndex = 0;
                                SLANG_RELEASE_ASSERT(argCount > argIndex);
                                auto arg = args[argIndex].get();
                                auto argLoad = as<IRLoad>(arg);
                                SLANG_RELEASE_ASSERT(argLoad);
                                auto argVar = argLoad->getOperand(0);
                                stream->emit(getRayPayloadLocation(ctx, argVar));
                            }
                            break;

                        case 'C':
                            {
                                // The `$XC` case handles looking up
                                // the associated `location` for a variable
                                // used as the argument callable payload at a
                                // call site.

                            Index argIndex = 0;
                                SLANG_RELEASE_ASSERT(argCount > argIndex);
                                auto arg = args[argIndex].get();
                                auto argLoad = as<IRLoad>(arg);
                                SLANG_RELEASE_ASSERT(argLoad);
                                auto argVar = argLoad->getOperand(0);
                                stream->emit(getCallablePayloadLocation(ctx, argVar));
                            }
                            break;

                        case 'T':
                            {
                                // The `$XT` case handles selecting between
                                // the `gl_HitTNV` and `gl_RayTmaxNV` builtins,
                                // based on what stage we are using:
                                switch( ctx->entryPoint->getStage() )
                                {
                                default:
                                    stream->emit("gl_RayTmaxNV");
                                    break;

                                case Stage::AnyHit:
                                case Stage::ClosestHit:
                                    stream->emit("gl_HitTNV");
                                    break;
                                }
                            }
                            break;

                        default:
                            SLANG_RELEASE_ASSERT(false);
                            break;
                        }
                    }
                    break;

                default:
                    SLANG_UNEXPECTED("bad format in intrinsic definition");
                    break;
                }
            }

            // Close any remaining open parens
            for (; openParenCount > 0; --openParenCount)
            {
                stream->emit(")");
            }
        }
    }

    void emitIntrinsicCallExpr(
        SharedEmitContext*    ctx,
        IRCall*         inst,
        IRFunc*         func,
        IREmitMode      mode,
        EOpInfo const&  inOuterPrec)
    {
        auto outerPrec = inOuterPrec;
        bool needClose = false;

        // For a call with N arguments, the instruction will
        // have N+1 operands. We will start consuming operands
        // starting at the index 1.
        UInt operandCount = inst->getOperandCount();
        UInt argCount = operandCount - 1;
        UInt operandIndex = 1;


        //
        if (auto targetIntrinsicDecoration = findTargetIntrinsicDecoration(ctx, func))
        {
            emitTargetIntrinsicCallExpr(
                ctx,
                inst,
                func,
                targetIntrinsicDecoration,
                mode,
                outerPrec);
            return;
        }

        // Our current strategy for dealing with intrinsic
        // calls is to "un-mangle" the mangled name, in
        // order to figure out what the user was originally
        // calling. This is a bit messy, and there might
        // be better strategies (including just stuffing
        // a pointer to the original decl onto the callee).

        // If the intrinsic the user is calling is a generic,
        // then the mangled name will have been set on the
        // outer-most generic, and not on the leaf value
        // (which is `func` above), so we need to walk
        // upwards to find it.
        //
        IRInst* valueForName = func;
        for(;;)
        {
            auto parentBlock = as<IRBlock>(valueForName->parent);
            if(!parentBlock)
                break;

            auto parentGeneric = as<IRGeneric>(parentBlock->parent);
            if(!parentGeneric)
                break;

            valueForName = parentGeneric;
        }

        // If we reach this point, we are assuming that the value
        // has some kind of linkage, and thus a mangled name.
        //
        auto linkageDecoration = valueForName->findDecoration<IRLinkageDecoration>();
        SLANG_ASSERT(linkageDecoration);
        auto mangledName = String(linkageDecoration->getMangledName());


        // We will use the `UnmangleContext` utility to
        // help us split the original name into its pieces.
        UnmangleContext um(mangledName);
        um.startUnmangling();

        // We'll read through the qualified name of the
        // symbol (e.g., `Texture2D<T>.Sample`) and then
        // only keep the last segment of the name (e.g.,
        // the `Sample` part).
        auto name = um.readSimpleName();

        // We will special-case some names here, that
        // represent callable declarations that aren't
        // ordinary functions, and thus may use different
        // syntax.
        if(name == "operator[]")
        {
            // The user is invoking a built-in subscript operator

            auto prec = kEOp_Postfix;
            needClose = maybeEmitParens(outerPrec, prec);

            emitIROperand(ctx, inst->getOperand(operandIndex++), mode, leftSide(outerPrec, prec));
            stream->emit("[");
            emitIROperand(ctx, inst->getOperand(operandIndex++), mode, kEOp_General);
            stream->emit("]");

            if(operandIndex < operandCount)
            {
                stream->emit(" = ");
                emitIROperand(ctx, inst->getOperand(operandIndex++), mode, kEOp_General);
            }

            maybeCloseParens(needClose);
            return;
        }

        auto prec = kEOp_Postfix;
        needClose = maybeEmitParens(outerPrec, prec);

        // The mangled function name currently records
        // the number of explicit parameters, and thus
        // doesn't include the implicit `this` parameter.
        // We can compare the argument and parameter counts
        // to figure out whether we have a member function call.
        UInt paramCount = um.readParamCount();

        if(argCount != paramCount)
        {
            // Looks like a member function call
            emitIROperand(ctx, inst->getOperand(operandIndex), mode, leftSide(outerPrec, prec));
            stream->emit(".");
            operandIndex++;
        }
        // fixing issue #602 for GLSL sign function: https://github.com/shader-slang/slang/issues/602
        bool glslSignFix = ctx->target == CodeGenTarget::GLSL && name == "sign";
        if (glslSignFix)
        {
            if (auto vectorType = as<IRVectorType>(inst->getDataType()))
            {
                stream->emit("ivec");
                stream->emit(as<IRConstant>(vectorType->getElementCount())->value.intVal);
                stream->emit("(");
            }
            else if (auto scalarType = as<IRBasicType>(inst->getDataType()))
            {
                stream->emit("int(");
            }
            else
                glslSignFix = false;
        }
        stream->emit(name);
        stream->emit("(");
        bool first = true;
        for(; operandIndex < operandCount; ++operandIndex )
        {
            if(!first) stream->emit(", ");
            emitIROperand(ctx, inst->getOperand(operandIndex), mode, kEOp_General);
            first = false;
        }
        stream->emit(")");
        if (glslSignFix)
            stream->emit(")");
        maybeCloseParens(needClose);
    }

    void emitIRCallExpr(
        SharedEmitContext*    ctx,
        IRCall*         inst,
        IREmitMode      mode,
        EOpInfo         outerPrec)
    {
        auto funcValue = inst->getOperand(0);

        // Does this function declare any requirements on GLSL version or
        // extensions, which should affect our output?
        if(getTarget(ctx) == CodeGenTarget::GLSL)
        {
            auto decoratedValue = funcValue;
            while (auto specInst = as<IRSpecialize>(decoratedValue))
            {
                decoratedValue = getSpecializedValue(specInst);
            }

            for( auto decoration : decoratedValue->getDecorations() )
            {
                switch(decoration->op)
                {
                default:
                    break;

                case kIROp_RequireGLSLExtensionDecoration:
                    requireGLSLExtension(String(((IRRequireGLSLExtensionDecoration*)decoration)->getExtensionName()));
                    break;

                case kIROp_RequireGLSLVersionDecoration:
                    requireGLSLVersion(int(((IRRequireGLSLVersionDecoration*)decoration)->getLanguageVersion()));
                    break;
                }
            }
        }

        // We want to detect any call to an intrinsic operation,
        // that we can emit it directly without mangling, etc.
        if(auto irFunc = asTargetIntrinsic(ctx, funcValue))
        {
            emitIntrinsicCallExpr(ctx, inst, irFunc, mode, outerPrec);
        }
        else
        {
            auto prec = kEOp_Postfix;
            bool needClose = maybeEmitParens(outerPrec, prec);

            emitIROperand(ctx, funcValue, mode, leftSide(outerPrec, prec));
            stream->emit("(");
            UInt argCount = inst->getOperandCount();
            for( UInt aa = 1; aa < argCount; ++aa )
            {
                auto operand = inst->getOperand(aa);
                if (as<IRVoidType>(operand->getDataType()))
                    continue;
                if(aa != 1) stream->emit(", ");
                emitIROperand(ctx, inst->getOperand(aa), mode, kEOp_General);
            }
            stream->emit(")");

            maybeCloseParens(needClose);
        }
    }

    static const char* getGLSLVectorCompareFunctionName(IROp op)
    {
        // Glsl vector comparisons use functions...
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/equal.xhtml

        switch (op)
        {
        case kIROp_Eql:     return "equal";
        case kIROp_Neq:     return "notEqual";
        case kIROp_Greater: return "greaterThan";
        case kIROp_Less:    return "lessThan";
        case kIROp_Geq:     return "greaterThanEqual";
        case kIROp_Leq:     return "lessThanEqual";
        default:    return nullptr;
        }
    }

    void _maybeEmitGLSLCast(SharedEmitContext* ctx, IRType* castType, IRInst* inst, IREmitMode mode)
    {
        // Wrap in cast if a cast type is specified
        if (castType)
        {
            emitIRType(ctx, castType);
            stream->emit("(");

            // Emit the operand
            emitIROperand(ctx, inst, mode, kEOp_General);

            stream->emit(")");
        }
        else
        {
            // Emit the operand
            emitIROperand(ctx, inst, mode, kEOp_General);
        }
    }

    void emitNot(SharedEmitContext* ctx, IRInst* inst, IREmitMode mode, EOpInfo& ioOuterPrec, bool* outNeedClose)
    {
        IRInst* operand = inst->getOperand(0);

        if (getTarget(ctx) == CodeGenTarget::GLSL)
        {
            if (auto vectorType = as<IRVectorType>(operand->getDataType()))
            {
                // Handle as a function call
                auto prec = kEOp_Postfix;
                *outNeedClose = maybeEmitParens(ioOuterPrec, prec);

                stream->emit("not(");
                emitIROperand(ctx, operand, mode, kEOp_General);
                stream->emit(")");
                return;
            }
        }

        auto prec = kEOp_Prefix;
        *outNeedClose = maybeEmitParens(ioOuterPrec, prec);

        stream->emit("!");
        emitIROperand(ctx, operand, mode, rightSide(prec, ioOuterPrec));
    }


    void emitComparison(SharedEmitContext* ctx, IRInst* inst, IREmitMode mode, EOpInfo& ioOuterPrec, const EOpInfo& opPrec, bool* needCloseOut)
    {        
        if (getTarget(ctx) == CodeGenTarget::GLSL)
        {
            IRInst* left = inst->getOperand(0);
            IRInst* right = inst->getOperand(1);

            auto leftVectorType = as<IRVectorType>(left->getDataType());
            auto rightVectorType = as<IRVectorType>(right->getDataType());

            // If either side is a vector handle as a vector
            if (leftVectorType || rightVectorType)
            {
                const char* funcName = getGLSLVectorCompareFunctionName(inst->op);
                SLANG_ASSERT(funcName);

                // Determine the vector type
                const auto vecType = leftVectorType ? leftVectorType : rightVectorType;

                // Handle as a function call
                auto prec = kEOp_Postfix;
                *needCloseOut = maybeEmitParens(ioOuterPrec, prec);

                stream->emit(funcName);
                stream->emit("(");
                _maybeEmitGLSLCast(ctx, (leftVectorType ? nullptr : vecType), left, mode);
                stream->emit(",");
                _maybeEmitGLSLCast(ctx, (rightVectorType ? nullptr : vecType), right, mode);
                stream->emit(")");

                return;
            }
        }

        *needCloseOut = maybeEmitParens(ioOuterPrec, opPrec);

        emitIROperand(ctx, inst->getOperand(0), mode, leftSide(ioOuterPrec, opPrec));
        stream->emit(" ");
        stream->emit(opPrec.op);
        stream->emit(" ");
        emitIROperand(ctx, inst->getOperand(1), mode, rightSide(ioOuterPrec, opPrec));
    }

    
    void emitIRInstExpr(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode,
        EOpInfo const&  inOuterPrec)
    {
        EOpInfo outerPrec = inOuterPrec;
        bool needClose = false;
        switch(inst->op)
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_BoolLit:
            emitIRSimpleValue(ctx, inst);
            break;

        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_MakeMatrix:
            // Simple constructor call

            switch (getTarget(ctx))
            {
                case CodeGenTarget::HLSL:
                {
                    if (inst->getOperandCount() == 1)
                    {
                        auto prec = kEOp_Prefix;
                        needClose = maybeEmitParens(outerPrec, prec);

                        // Need to emit as cast for HLSL
                        stream->emit("(");
                        emitIRType(ctx, inst->getDataType());
                        stream->emit(") ");
                        emitIROperand(ctx, inst->getOperand(0), mode, rightSide(outerPrec, prec));
                        break;
                    }
                    /* fallthru*/
                }
                case CodeGenTarget::GLSL:
                {
                    emitIRType(ctx, inst->getDataType());
                    emitIRArgs(ctx, inst, mode);
                    break;
                }
                case CodeGenTarget::CPPSource:
                case CodeGenTarget::CSource:
                {
                    if (inst->getOperandCount() == 1)
                    {
                        _emitCFunc(BuiltInCOp::Splat, inst->getDataType());
                        emitIRArgs(ctx, inst, mode);
                    }
                    else
                    {
                        _emitCFunc(BuiltInCOp::Init, inst->getDataType());
                        emitIRArgs(ctx, inst, mode);
                    }
                    break;
                }
            }
            break;
        case kIROp_constructVectorFromScalar:

            // Simple constructor call
            if( getTarget(ctx) == CodeGenTarget::HLSL )
            {
                auto prec = kEOp_Prefix;
                needClose = maybeEmitParens(outerPrec, prec);

                stream->emit("(");
                emitIRType(ctx, inst->getDataType());
                stream->emit(")");

                emitIROperand(ctx, inst->getOperand(0), mode, rightSide(outerPrec,prec));
            }
            else
            {
                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                emitIRType(ctx, inst->getDataType());
                stream->emit("(");
                emitIROperand(ctx, inst->getOperand(0), mode, kEOp_General);
                stream->emit(")");
            }
            break;

        case kIROp_FieldExtract:
            {
                // Extract field from aggregate

                IRFieldExtract* fieldExtract = (IRFieldExtract*) inst;

                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                auto base = fieldExtract->getBase();
                emitIROperand(ctx, base, mode, leftSide(outerPrec, prec));
                stream->emit(".");
                if(getTarget(ctx) == CodeGenTarget::GLSL
                    && as<IRUniformParameterGroupType>(base->getDataType()))
                {
                    stream->emit("_data.");
                }
                stream->emit(getIRName(fieldExtract->getField()));
            }
            break;

        case kIROp_FieldAddress:
            {
                // Extract field "address" from aggregate

                IRFieldAddress* ii = (IRFieldAddress*) inst;

                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                auto base = ii->getBase();
                emitIROperand(ctx, base, mode, leftSide(outerPrec, prec));
                stream->emit(".");
                if(getTarget(ctx) == CodeGenTarget::GLSL
                    && as<IRUniformParameterGroupType>(base->getDataType()))
                {
                    stream->emit("_data.");
                }
                stream->emit(getIRName(ii->getField()));
            }
            break;


#define CASE_COMPARE(OPCODE, PREC, OP)                                                          \
        case OPCODE:                                                                            \
            emitComparison(ctx, inst,  mode, outerPrec, kEOp_##PREC, &needClose);               \
            break

#define CASE(OPCODE, PREC, OP)                                                                  \
        case OPCODE:                                                                            \
            needClose = maybeEmitParens(outerPrec, kEOp_##PREC);                                \
            emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, kEOp_##PREC));    \
            stream->emit(" " #OP " ");                                                                  \
            emitIROperand(ctx, inst->getOperand(1), mode, rightSide(outerPrec, kEOp_##PREC));   \
            break

        CASE(kIROp_Add, Add, +);
        CASE(kIROp_Sub, Sub, -);
        CASE(kIROp_Div, Div, /);
        CASE(kIROp_Mod, Mod, %);

        CASE(kIROp_Lsh, Lsh, <<);
        CASE(kIROp_Rsh, Rsh, >>);

        // TODO: Need to pull out component-wise
        // comparison cases for matrices/vectors
        CASE_COMPARE(kIROp_Eql, Eql, ==);
        CASE_COMPARE(kIROp_Neq, Neq, !=);
        CASE_COMPARE(kIROp_Greater, Greater, >);
        CASE_COMPARE(kIROp_Less, Less, <);
        CASE_COMPARE(kIROp_Geq, Geq, >=);
        CASE_COMPARE(kIROp_Leq, Leq, <=);

        CASE(kIROp_BitXor, BitXor, ^);

        CASE(kIROp_And, And, &&);
        CASE(kIROp_Or,  Or,  ||);

#undef CASE

        // Component-wise ultiplication needs to be special cased,
        // because GLSL uses infix `*` to express inner product
        // when working with matrices.
        case kIROp_Mul:
            // Are we targetting GLSL, and are both operands matrices?
            if(getTarget(ctx) == CodeGenTarget::GLSL
                && as<IRMatrixType>(inst->getOperand(0)->getDataType())
                && as<IRMatrixType>(inst->getOperand(1)->getDataType()))
            {
                stream->emit("matrixCompMult(");
                emitIROperand(ctx, inst->getOperand(0), mode, kEOp_General);
                stream->emit(", ");
                emitIROperand(ctx, inst->getOperand(1), mode, kEOp_General);
                stream->emit(")");
            }
            else
            {
                // Default handling is to just rely on infix
                // `operator*`.
                auto prec = kEOp_Mul;
                needClose = maybeEmitParens(outerPrec, prec);
                emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));
                stream->emit(" * ");
                emitIROperand(ctx, inst->getOperand(1), mode, rightSide(prec, outerPrec));
            }
            break;

        case kIROp_Not:
            {
                emitNot(ctx, inst,  mode, outerPrec, &needClose);
            }
            break;

        case kIROp_Neg:
            {
                auto prec = kEOp_Prefix;
                needClose = maybeEmitParens(outerPrec, prec);

                stream->emit("-");
                emitIROperand(ctx, inst->getOperand(0), mode, rightSide(prec, outerPrec));
            }
            break;

        case kIROp_BitNot:
            {
                auto prec = kEOp_Prefix;
                needClose = maybeEmitParens(outerPrec, prec);

                if (as<IRBoolType>(inst->getDataType()))
                {
                    stream->emit("!");
                }
                else
                {
                    stream->emit("~");
                }
                emitIROperand(ctx, inst->getOperand(0), mode, rightSide(prec, outerPrec));
            }
            break;

        case kIROp_BitAnd:
            {
                auto prec = kEOp_BitAnd;
                needClose = maybeEmitParens(outerPrec, prec);

                // TODO: handle a bitwise And of a vector of bools by casting to
                // a uvec and performing the bitwise operation

                emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));

                // Are we targetting GLSL, and are both operands scalar bools?
                // In that case convert the operation to a logical And
                if (getTarget(ctx) == CodeGenTarget::GLSL
                    && as<IRBoolType>(inst->getOperand(0)->getDataType())
                    && as<IRBoolType>(inst->getOperand(1)->getDataType()))
                {
                    stream->emit("&&");
                }
                else
                {
                    stream->emit("&");
                }

                emitIROperand(ctx, inst->getOperand(1), mode, rightSide(outerPrec, prec));
            }
            break;

        case kIROp_BitOr:
            {
                auto prec = kEOp_BitOr;
                needClose = maybeEmitParens(outerPrec, prec);

                // TODO: handle a bitwise Or of a vector of bools by casting to
                // a uvec and performing the bitwise operation

                emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));

                // Are we targetting GLSL, and are both operands scalar bools?
                // In that case convert the operation to a logical Or
                if (getTarget(ctx) == CodeGenTarget::GLSL
                    && as<IRBoolType>(inst->getOperand(0)->getDataType())
                    && as<IRBoolType>(inst->getOperand(1)->getDataType()))
                {
                    stream->emit("||");
                }
                else
                {
                    stream->emit("|");
                }

                emitIROperand(ctx, inst->getOperand(1), mode, rightSide(outerPrec, prec));
            }
            break;

        case kIROp_Load:
            {
                auto base = inst->getOperand(0);
                emitIROperand(ctx, base, mode, outerPrec);
                if(getTarget(ctx) == CodeGenTarget::GLSL
                    && as<IRUniformParameterGroupType>(base->getDataType()))
                {
                    stream->emit("._data");
                }
            }
            break;

        case kIROp_Store:
            {
                auto prec = kEOp_Assign;
                needClose = maybeEmitParens(outerPrec, prec);

                emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));
                stream->emit(" = ");
                emitIROperand(ctx, inst->getOperand(1), mode, rightSide(prec, outerPrec));
            }
            break;

        case kIROp_Call:
            {
                emitIRCallExpr(ctx, (IRCall*)inst, mode, outerPrec);
            }
            break;

        case kIROp_GroupMemoryBarrierWithGroupSync:
            stream->emit("GroupMemoryBarrierWithGroupSync()");
            break;

        case kIROp_getElement:
        case kIROp_getElementPtr:
        case kIROp_ImageSubscript:
            // HACK: deal with translation of GLSL geometry shader input arrays.
            if(auto decoration = inst->getOperand(0)->findDecoration<IRGLSLOuterArrayDecoration>())
            {
                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                stream->emit(decoration->getOuterArrayName());
                stream->emit("[");
                emitIROperand(ctx, inst->getOperand(1), mode, kEOp_General);
                stream->emit("].");
                emitIROperand(ctx, inst->getOperand(0), mode, rightSide(prec, outerPrec));
                break;
            }
            else
            {
                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));
                stream->emit("[");
                emitIROperand(ctx, inst->getOperand(1), mode, kEOp_General);
                stream->emit("]");
            }
            break;

        case kIROp_Mul_Vector_Matrix:
        case kIROp_Mul_Matrix_Vector:
        case kIROp_Mul_Matrix_Matrix:
            if(getTarget(ctx) == CodeGenTarget::GLSL)
            {
                // GLSL expresses inner-product multiplications
                // with the ordinary infix `*` operator.
                //
                // Note that the order of the operands is reversed
                // compared to HLSL (and Slang's internal representation)
                // because the notion of what is a "row" vs. a "column"
                // is reversed between HLSL/Slang and GLSL.
                //
                auto prec = kEOp_Mul;
                needClose = maybeEmitParens(outerPrec, prec);

                emitIROperand(ctx, inst->getOperand(1), mode, leftSide(outerPrec, prec));
                stream->emit(" * ");
                emitIROperand(ctx, inst->getOperand(0), mode, rightSide(prec, outerPrec));
            }
            else
            {
                stream->emit("mul(");
                emitIROperand(ctx, inst->getOperand(0), mode, kEOp_General);
                stream->emit(", ");
                emitIROperand(ctx, inst->getOperand(1), mode, kEOp_General);
                stream->emit(")");
            }
            break;

        case kIROp_swizzle:
            {
                auto prec = kEOp_Postfix;
                needClose = maybeEmitParens(outerPrec, prec);

                auto ii = (IRSwizzle*)inst;
                emitIROperand(ctx, ii->getBase(), mode, leftSide(outerPrec, prec));
                stream->emit(".");
                const Index elementCount = Index(ii->getElementCount());
                for (Index ee = 0; ee < elementCount; ++ee)
                {
                    IRInst* irElementIndex = ii->getElementIndex(ee);
                    SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;

                    UInt elementIndex = (UInt)irConst->value.intVal;
                    SLANG_RELEASE_ASSERT(elementIndex < 4);

                    char const* kComponents[] = { "x", "y", "z", "w" };
                    stream->emit(kComponents[elementIndex]);
                }
            }
            break;

        case kIROp_Specialize:
            {
                emitIROperand(ctx, inst->getOperand(0), mode, outerPrec);
            }
            break;

        case kIROp_Select:
            {
                if (getTarget(ctx) == CodeGenTarget::GLSL &&
                    inst->getOperand(0)->getDataType()->op != kIROp_BoolType)
                {
                    // For GLSL, emit a call to `mix` if condition is a vector
                    stream->emit("mix(");
                    emitIROperand(ctx, inst->getOperand(2), mode, leftSide(kEOp_General, kEOp_General));
                    stream->emit(", ");
                    emitIROperand(ctx, inst->getOperand(1), mode, leftSide(kEOp_General, kEOp_General));
                    stream->emit(", ");
                    emitIROperand(ctx, inst->getOperand(0), mode, leftSide(kEOp_General, kEOp_General));
                    stream->emit(")");
                }
                else
                {
                    auto prec = kEOp_Conditional;
                    needClose = maybeEmitParens(outerPrec, prec);

                    emitIROperand(ctx, inst->getOperand(0), mode, leftSide(outerPrec, prec));
                    stream->emit(" ? ");
                    emitIROperand(ctx, inst->getOperand(1), mode, prec);
                    stream->emit(" : ");
                    emitIROperand(ctx, inst->getOperand(2), mode, rightSide(prec, outerPrec));
                }
            }
            break;

        case kIROp_Param:
            stream->emit(getIRName(inst));
            break;

        case kIROp_makeArray:
        case kIROp_makeStruct:
            {
                // TODO: initializer-list syntax may not always
                // be appropriate, depending on the context
                // of the expression.

                stream->emit("{ ");
                UInt argCount = inst->getOperandCount();
                for (UInt aa = 0; aa < argCount; ++aa)
                {
                    if (aa != 0) stream->emit(", ");
                    emitIROperand(ctx, inst->getOperand(aa), mode, kEOp_General);
                }
                stream->emit(" }");
            }
            break;

        case kIROp_BitCast:
            {
                // TODO: we can simplify the logic for arbitrary bitcasts
                // by always bitcasting the source to a `uint*` type (if it
                // isn't already) and then bitcasting that to the destination
                // type (if it isn't already `uint*`.
                //
                // For now we are assuming the source type is *already*
                // a `uint*` type of the appropriate size.
                //
//                auto fromType = extractBaseType(inst->getOperand(0)->getDataType());
                auto toType = extractBaseType(inst->getDataType());
                switch(getTarget(ctx))
                {
                case CodeGenTarget::GLSL:
                    switch(toType)
                    {
                    default:
                        stream->emit("/* unhandled */");
                        break;

                    case BaseType::UInt:
                        break;

                    case BaseType::Int:
                        emitIRType(ctx, inst->getDataType());
                        break;

                    case BaseType::Float:
                        stream->emit("uintBitsToFloat(");
                        break;
                    }
                    break;

                case CodeGenTarget::HLSL:
                    switch(toType)
                    {
                    default:
                        stream->emit("/* unhandled */");
                        break;

                    case BaseType::UInt:
                        break;
                    case BaseType::Int:
                        stream->emit("(");
                        emitIRType(ctx, inst->getDataType());
                        stream->emit(")");
                        break;
                    case BaseType::Float:
                        stream->emit("asfloat");
                        break;
                    }
                    break;


                default:
                    SLANG_UNEXPECTED("unhandled codegen target");
                    break;
                }

                stream->emit("(");
                emitIROperand(ctx, inst->getOperand(0), mode, kEOp_General);
                stream->emit(")");
            }
            break;

        default:
            stream->emit("/* unhandled */");
            break;
        }
        maybeCloseParens(needClose);
    }

    BaseType extractBaseType(IRType* inType)
    {
        auto type = inType;
        for(;;)
        {
            if(auto irBaseType = as<IRBasicType>(type))
            {
                return irBaseType->getBaseType();
            }
            else if(auto vecType = as<IRVectorType>(type))
            {
                type = vecType->getElementType();
                continue;
            }
            else
            {
                return BaseType::Void;
            }
        }
    }

    void emitIRInst(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode)
    {
        try
        {
            emitIRInstImpl(ctx, inst, mode);
        }
        // Don't emit any context message for an explicit `AbortCompilationException`
        // because it should only happen when an error is already emitted.
        catch(AbortCompilationException&) { throw; }
        catch(...)
        {
            ctx->noteInternalErrorLoc(inst->sourceLoc);
            throw;
        }
    }

    void emitIRInstImpl(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        IREmitMode      mode)
    {
        if (shouldFoldIRInstIntoUseSites(ctx, inst, mode))
        {
            return;
        }

        stream->advanceToSourceLocation(inst->sourceLoc);

        switch(inst->op)
        {
        default:
            emitIRInstResultDecl(ctx, inst);
            emitIRInstExpr(ctx, inst, mode, kEOp_General);
            stream->emit(";\n");
            break;

        case kIROp_undefined:
            {
                auto type = inst->getDataType();
                emitIRType(ctx, type, getIRName(inst));
                stream->emit(";\n");
            }
            break;

        case kIROp_Var:
            {
                auto ptrType = cast<IRPtrType>(inst->getDataType());
                auto valType = ptrType->getValueType();

                auto name = getIRName(inst);
                emitIRRateQualifiers(ctx, inst);
                emitIRType(ctx, valType, name);
                stream->emit(";\n");
            }
            break;

        case kIROp_Param:
            // Don't emit parameters, since they are declared as part of the function.
            break;

        case kIROp_FieldAddress:
            // skip during code emit, since it should be
            // folded into use site(s)
            break;

        case kIROp_ReturnVoid:
            stream->emit("return;\n");
            break;

        case kIROp_ReturnVal:
            stream->emit("return ");
            emitIROperand(ctx, ((IRReturnVal*) inst)->getVal(), mode, kEOp_General);
            stream->emit(";\n");
            break;

        case kIROp_discard:
            stream->emit("discard;\n");
            break;

        case kIROp_swizzleSet:
            {
                auto ii = (IRSwizzleSet*)inst;
                emitIRInstResultDecl(ctx, inst);
                emitIROperand(ctx, inst->getOperand(0), mode, kEOp_General);
                stream->emit(";\n");

                auto subscriptOuter = kEOp_General;
                auto subscriptPrec = kEOp_Postfix;
                bool needCloseSubscript = maybeEmitParens(subscriptOuter, subscriptPrec);

                emitIROperand(ctx, inst, mode, leftSide(subscriptOuter, subscriptPrec));
                stream->emit(".");
                UInt elementCount = ii->getElementCount();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    IRInst* irElementIndex = ii->getElementIndex(ee);
                    SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;

                    UInt elementIndex = (UInt)irConst->value.intVal;
                    SLANG_RELEASE_ASSERT(elementIndex < 4);

                    char const* kComponents[] = { "x", "y", "z", "w" };
                    stream->emit(kComponents[elementIndex]);
                }
                maybeCloseParens(needCloseSubscript);

                stream->emit(" = ");
                emitIROperand(ctx, inst->getOperand(1), mode, kEOp_General);
                stream->emit(";\n");
            }
            break;

        case kIROp_SwizzledStore:
            {
                auto subscriptOuter = kEOp_General;
                auto subscriptPrec = kEOp_Postfix;
                bool needCloseSubscript = maybeEmitParens(subscriptOuter, subscriptPrec);


                auto ii = cast<IRSwizzledStore>(inst);
                emitIROperand(ctx, ii->getDest(), mode, leftSide(subscriptOuter, subscriptPrec));
                stream->emit(".");
                UInt elementCount = ii->getElementCount();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    IRInst* irElementIndex = ii->getElementIndex(ee);
                    SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;

                    UInt elementIndex = (UInt)irConst->value.intVal;
                    SLANG_RELEASE_ASSERT(elementIndex < 4);

                    char const* kComponents[] = { "x", "y", "z", "w" };
                    stream->emit(kComponents[elementIndex]);
                }
                maybeCloseParens(needCloseSubscript);

                stream->emit(" = ");
                emitIROperand(ctx, ii->getSource(), mode, kEOp_General);
                stream->emit(";\n");
            }
            break;
        }
    }

    void emitIRSemantics(
        SharedEmitContext*,
        VarLayout*      varLayout)
    {
        if(varLayout->flags & VarLayoutFlag::HasSemantic)
        {
            stream->emit(" : ");
            stream->emit(varLayout->semanticName);
            if(varLayout->semanticIndex)
            {
                stream->emit(varLayout->semanticIndex);
            }
        }
    }

    void emitIRSemantics(
        SharedEmitContext*    ctx,
        IRInst*         inst)
    {
        // Don't emit semantics if we aren't translating down to HLSL
        switch (ctx->target)
        {
        case CodeGenTarget::HLSL:
            break;

        default:
            return;
        }

        if (auto semanticDecoration = inst->findDecoration<IRSemanticDecoration>())
        {
            stream->emit(" : ");
            stream->emit(semanticDecoration->getSemanticName());
            return;
        }

        if(auto layoutDecoration = inst->findDecoration<IRLayoutDecoration>())
        {
            auto layout = layoutDecoration->getLayout();
            if(auto varLayout = as<VarLayout>(layout))
            {
                emitIRSemantics(ctx, varLayout);
            }
            else if (auto entryPointLayout = as<EntryPointLayout>(layout))
            {
                if(auto resultLayout = entryPointLayout->resultLayout)
                {
                    emitIRSemantics(ctx, resultLayout);
                }
            }
        }
    }

    VarLayout* getVarLayout(
        SharedEmitContext*    /*context*/,
        IRInst*         var)
    {
        auto decoration = var->findDecoration<IRLayoutDecoration>();
        if (!decoration)
            return nullptr;

        return (VarLayout*) decoration->getLayout();
    }

    void emitIRLayoutSemantics(
        SharedEmitContext*    ctx,
        IRInst*         inst,
        char const*     uniformSemanticSpelling = "register")
    {
        auto layout = getVarLayout(ctx, inst);
        if (layout)
        {
            emitHLSLRegisterSemantics(layout, uniformSemanticSpelling);
        }
    }

    // When we are about to traverse an edge from one block to another,
    // we need to emit the assignments that conceptually occur "along"
    // the edge. In traditional SSA these are the phi nodes in the
    // target block, while in our representation these use the arguments
    // to the branch instruction to fill in the parameters of the target.
    void emitPhiVarAssignments(
        SharedEmitContext*    ctx,
        UInt            argCount,
        IRUse*          args,
        IRBlock*        targetBlock)
    {
        UInt argCounter = 0;
        for (auto pp = targetBlock->getFirstParam(); pp; pp = pp->getNextParam())
        {
            UInt argIndex = argCounter++;

            if (argIndex >= argCount)
            {
                SLANG_UNEXPECTED("not enough arguments for branch");
                break;
            }

            IRInst* arg = args[argIndex].get();

            auto outerPrec = kEOp_General;
            auto prec = kEOp_Assign;

            emitIROperand(ctx, pp, IREmitMode::Default, leftSide(outerPrec, prec));
            stream->emit(" = ");
            emitIROperand(ctx, arg, IREmitMode::Default, rightSide(prec, outerPrec));
            stream->emit(";\n");
        }
    }

    /// Emit high-level language statements from a structrured region.
    void emitRegion(
        SharedEmitContext*    ctx,
        Region*         inRegion)
    {
        // We will use a loop so that we can process sequential (simple)
        // regions iteratively rather than recursively.
        // This is effectively an emulation of tail recursion.
        Region* region = inRegion;
        while(region)
        {
            // What flavor of region are we trying to emit?
            switch(region->getFlavor())
            {
            case Region::Flavor::Simple:
                {
                    // A simple region consists of a basic block followed
                    // by another region.
                    //
                    auto simpleRegion = (SimpleRegion*) region;

                    // We start by outputting all of the non-terminator
                    // instructions in the block.
                    //
                    auto block = simpleRegion->block;
                    auto terminator = block->getTerminator();
                    for (auto inst = block->getFirstInst(); inst != terminator; inst = inst->getNextInst())
                    {
                        emitIRInst(ctx, inst, IREmitMode::Default);
                    }

                    // Next we have to deal with the terminator instruction
                    // itself. In many cases, the terminator will have been
                    // turned into a block of its own, but certain cases
                    // of terminators are simple enough that we just fold
                    // them into the current block.
                    //
                    stream->advanceToSourceLocation(terminator->sourceLoc);
                    switch(terminator->op)
                    {
                    default:
                        // Don't do anything with the terminator, and assume
                        // its behavior has been folded into the next region.
                        break;

                    case kIROp_ReturnVal:
                    case kIROp_ReturnVoid:
                    case kIROp_discard:
                        // For extremely simple terminators, we just handle
                        // them here, so that we don't have to allocate
                        // separate `Region`s for them.
                        emitIRInst(ctx, terminator, IREmitMode::Default);
                        break;

                    // We will also handle any unconditional branches
                    // here, since they may have arguments to pass
                    // to the target block (our encoding of SSA
                    // "phi" operations).
                    //
                    // TODO: A better approach would be to move out of SSA
                    // as an IR pass, and introduce explicit variables to
                    // replace any "phi nodes." This would avoid possible
                    // complications if we ever end up in the bad case where
                    // one of the block arguments on a branch is also
                    // a paremter of the target block, so that the order
                    // of operations is important.
                    //
                    case kIROp_unconditionalBranch:
                        {
                            auto t = (IRUnconditionalBranch*)terminator;
                            UInt argCount = t->getOperandCount();
                            static const UInt kFixedArgCount = 1;
                            emitPhiVarAssignments(
                                ctx,
                                argCount - kFixedArgCount,
                                t->getOperands() + kFixedArgCount,
                                t->getTargetBlock());
                        }
                        break;
                    case kIROp_loop:
                        {
                            auto t = (IRLoop*) terminator;
                            UInt argCount = t->getOperandCount();
                            static const UInt kFixedArgCount = 3;
                            emitPhiVarAssignments(
                                ctx,
                                argCount - kFixedArgCount,
                                t->getOperands() + kFixedArgCount,
                                t->getTargetBlock());

                        }
                        break;
                    }

                    // If the terminator required a full region to represent
                    // its behavior in a structured form, then we will move
                    // along to that region now.
                    //
                    // We do this iteratively rather than recursively, by
                    // jumping back to the top of our loop with a new
                    // value for `region`.
                    //
                    region = simpleRegion->nextRegion;
                    continue;
                }

            // Break and continue regions are trivial to handle, as long as we
            // don't need to consider multi-level break/continue (which we
            // don't for now).
            case Region::Flavor::Break:
                stream->emit("break;\n");
                break;
            case Region::Flavor::Continue:
                stream->emit("continue;\n");
                break;

            case Region::Flavor::If:
                {
                    auto ifRegion = (IfRegion*) region;

                    // TODO: consider simplifying the code in
                    // the case where `ifRegion == null`
                    // so that we output `if(!condition) { elseRegion }`
                    // instead of the current `if(condition) {} else { elseRegion }`

                    stream->emit("if(");
                    emitIROperand(ctx, ifRegion->condition, IREmitMode::Default, kEOp_General);
                    stream->emit(")\n{\n");
                    stream->indent();
                    emitRegion(ctx, ifRegion->thenRegion);
                    stream->dedent();
                    stream->emit("}\n");

                    // Don't emit the `else` region if it would be empty
                    //
                    if(auto elseRegion = ifRegion->elseRegion)
                    {
                        stream->emit("else\n{\n");
                        stream->indent();
                        emitRegion(ctx, elseRegion);
                        stream->dedent();
                        stream->emit("}\n");
                    }

                    // Continue with the region after the `if`.
                    //
                    // TODO: consider just constructing a `SimpleRegion`
                    // around an `IfRegion` to handle this sequencing,
                    // rather than making `IfRegion` serve as both a
                    // conditional and a sequence.
                    //
                    region = ifRegion->nextRegion;
                    continue;
                }
                break;

            case Region::Flavor::Loop:
                {
                    auto loopRegion = (LoopRegion*) region;
                    auto loopInst = loopRegion->loopInst;

                    // If the user applied an explicit decoration to the loop,
                    // to control its unrolling behavior, then pass that
                    // along in the output code (if the target language
                    // supports the semantics of the decoration).
                    //
                    if (auto loopControlDecoration = loopInst->findDecoration<IRLoopControlDecoration>())
                    {
                        switch (loopControlDecoration->getMode())
                        {
                        case kIRLoopControl_Unroll:
                            // Note: loop unrolling control is only available in HLSL, not GLSL
                            if(getTarget(ctx) == CodeGenTarget::HLSL)
                            {
                                stream->emit("[unroll]\n");
                            }
                            break;

                        default:
                            break;
                        }
                    }

                    stream->emit("for(;;)\n{\n");
                    stream->indent();
                    emitRegion(ctx, loopRegion->body);
                    stream->dedent();
                    stream->emit("}\n");

                    // Continue with the region after the loop
                    region = loopRegion->nextRegion;
                    continue;
                }

            case Region::Flavor::Switch:
                {
                    auto switchRegion = (SwitchRegion*) region;

                    // Emit the start of our statement.
                    stream->emit("switch(");
                    emitIROperand(ctx, switchRegion->condition, IREmitMode::Default, kEOp_General);
                    stream->emit(")\n{\n");

                    auto defaultCase = switchRegion->defaultCase;
                    for(auto currentCase : switchRegion->cases)
                    {
                        for(auto caseVal : currentCase->values)
                        {
                            stream->emit("case ");
                            emitIROperand(ctx, caseVal, IREmitMode::Default, kEOp_General);
                            stream->emit(":\n");
                        }
                        if(currentCase.Ptr() == defaultCase)
                        {
                            stream->emit("default:\n");
                        }

                        stream->indent();
                        stream->emit("{\n");
                        stream->indent();
                        emitRegion(ctx, currentCase->body);
                        stream->dedent();
                        stream->emit("}\n");
                        stream->dedent();
                    }

                    stream->emit("}\n");

                    // Continue with the region after the `switch`
                    region = switchRegion->nextRegion;
                    continue;
                }
                break;
            }
            break;
        }
    }

    /// Emit high-level language statements from a structured region tree.
    void emitRegionTree(
        SharedEmitContext*    ctx,
        RegionTree*     regionTree)
    {
        emitRegion(ctx, regionTree->rootRegion);
    }

    // Is an IR function a definition? (otherwise it is a declaration)
    bool isDefinition(IRFunc* func)
    {
        // For now, we use a simple approach: a function is
        // a definition if it has any blocks, and a declaration otherwise.
        return func->getFirstBlock() != nullptr;
    }

    String getIRFuncName(
        IRFunc* func)
    {
        if (auto entryPointLayout = asEntryPoint(func))
        {
            // GLSL will always need to use `main` as the
            // name for an entry-point function, but other
            // targets should try to use the original name.
            //
            // TODO: always use `main`, and have any code
            // that wraps this know to use `main` instead
            // of the original entry-point name...
            //
            if (getTarget(context) != CodeGenTarget::GLSL)
            {
                return getText(entryPointLayout->entryPoint->getName());
            }

            //

            return "main";
        }
        else
        {
            return getIRName(func);
        }
    }

    void emitAttributeSingleString(const char* name, FuncDecl* entryPoint, Attribute* attrib)
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

        stream->emit("[");
        stream->emit(name);
        stream->emit("(\"");
        stream->emit(stringLitExpr->value);
        stream->emit("\")]\n");
    }

    void emitAttributeSingleInt(const char* name, FuncDecl* entryPoint, Attribute* attrib)
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

        stream->emit("[");
        stream->emit(name);
        stream->emit("(");
        stream->emit(intLitExpr->value);
        stream->emit(")]\n");
    }

    void emitFuncDeclPatchConstantFuncAttribute(IRFunc* irFunc, FuncDecl* entryPoint, PatchConstantFuncAttribute* attrib)
    {
        SLANG_UNUSED(attrib);

        auto irPatchFunc = irFunc->findDecoration<IRPatchConstantFuncDecoration>();
        assert(irPatchFunc);
        if (!irPatchFunc)
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), entryPoint->loc, "Unable to find [patchConstantFunc(...)] decoration");
            return;
        }

        const String irName = getIRName(irPatchFunc->getFunc());

        stream->emit("[patchconstantfunc(\"");
        stream->emit(irName);
        stream->emit("\")]\n");
    }

    void emitIREntryPointAttributes_HLSL(
        IRFunc*             irFunc,
        SharedEmitContext*        ctx,
        EntryPointLayout*   entryPointLayout)
    {
        auto profile = ctx->effectiveProfile;
        auto stage = entryPointLayout->profile.GetStage();

        if(profile.getFamily() == ProfileFamily::DX)
        {
            if(profile.GetVersion() >= ProfileVersion::DX_6_1 )
            {
                char const* stageName = getStageName(stage);
                if(stageName)
                {
                    stream->emit("[shader(\"");
                    stream->emit(stageName);
                    stream->emit("\")]");
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

                stream->emit("[numthreads(");
                for (int ii = 0; ii < 3; ++ii)
                {
                    if (ii != 0) stream->emit(", ");
                    stream->emit(sizeAlongAxis[ii]);
                }
                stream->emit(")]\n");
            }
            break;
        case Stage::Geometry:
        {
            if (auto attrib = entryPointLayout->entryPoint->FindModifier<MaxVertexCountAttribute>())
            {
                stream->emit("[maxvertexcount(");
                stream->emit(attrib->value);
                stream->emit(")]\n");
            }
            if (auto attrib = entryPointLayout->entryPoint->FindModifier<InstanceAttribute>())
            {
                stream->emit("[instance(");
                stream->emit(attrib->value);
                stream->emit(")]\n");
            }
            break;
        }
        case Stage::Domain:
        {
            FuncDecl* entryPoint = entryPointLayout->entryPoint;
            /* [domain("isoline")] */
            if (auto attrib = entryPoint->FindModifier<DomainAttribute>())
            {
                emitAttributeSingleString("domain", entryPoint, attrib);
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
                emitAttributeSingleString("domain", entryPoint, attrib);
            }
            /* [domain("partitioning")] */
            if (auto attrib = entryPoint->FindModifier<PartitioningAttribute>())
            {
                emitAttributeSingleString("partitioning", entryPoint, attrib);
            }
            /* [outputtopology("line")] */
            if (auto attrib = entryPoint->FindModifier<OutputTopologyAttribute>())
            {
                emitAttributeSingleString("outputtopology", entryPoint, attrib);
            }
            /* [outputcontrolpoints(4)] */
            if (auto attrib = entryPoint->FindModifier<OutputControlPointsAttribute>())
            {
                emitAttributeSingleInt("outputcontrolpoints", entryPoint, attrib);
            }
            /* [patchconstantfunc("HSConst")] */
            if (auto attrib = entryPoint->FindModifier<PatchConstantFuncAttribute>())
            {
                emitFuncDeclPatchConstantFuncAttribute(irFunc, entryPoint, attrib);
            }

            break;
        }
        case Stage::Pixel:
        {
            if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
            {
                stream->emit("[earlydepthstencil]\n");
            }
            break;
        }
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
        }
    }

    void emitIREntryPointAttributes_GLSL(
        IRFunc*             irFunc,
        SharedEmitContext*        /*ctx*/,
        EntryPointLayout*   entryPointLayout)
    {
        auto profile = entryPointLayout->profile;
        auto stage = profile.GetStage();

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

                stream->emit("layout(");
                char const* axes[] = { "x", "y", "z" };
                for (int ii = 0; ii < 3; ++ii)
                {
                    if (ii != 0) stream->emit(", ");
                    stream->emit("local_size_");
                    stream->emit(axes[ii]);
                    stream->emit(" = ");
                    stream->emit(sizeAlongAxis[ii]);
                }
                stream->emit(") in;");
            }
            break;
        case Stage::Geometry:
        {
            if (auto attrib = entryPointLayout->entryPoint->FindModifier<MaxVertexCountAttribute>())
            {
                stream->emit("layout(max_vertices = ");
                stream->emit(attrib->value);
                stream->emit(") out;\n");
            }
            if (auto attrib = entryPointLayout->entryPoint->FindModifier<InstanceAttribute>())
            {
                stream->emit("layout(invocations = ");
                stream->emit(attrib->value);
                stream->emit(") in;\n");
            }

            for(auto pp : entryPointLayout->entryPoint->GetParameters())
            {
                if(auto inputPrimitiveTypeModifier = pp->FindModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
                {
                    if(as<HLSLTriangleModifier>(inputPrimitiveTypeModifier))
                    {
                        stream->emit("layout(triangles) in;\n");
                    }
                    else if(as<HLSLLineModifier>(inputPrimitiveTypeModifier))
                    {
                        stream->emit("layout(lines) in;\n");
                    }
                    else if(as<HLSLLineAdjModifier>(inputPrimitiveTypeModifier))
                    {
                        stream->emit("layout(lines_adjacency) in;\n");
                    }
                    else if(as<HLSLPointModifier>(inputPrimitiveTypeModifier))
                    {
                        stream->emit("layout(points) in;\n");
                    }
                    else if(as<HLSLTriangleAdjModifier>(inputPrimitiveTypeModifier))
                    {
                        stream->emit("layout(triangles_adjacency) in;\n");
                    }
                }

                if(auto outputStreamType = as<HLSLStreamOutputType>(pp->type))
                {
                    if(as<HLSLTriangleStreamType>(outputStreamType))
                    {
                        stream->emit("layout(triangle_strip) out;\n");
                    }
                    else if(as<HLSLLineStreamType>(outputStreamType))
                    {
                        stream->emit("layout(line_strip) out;\n");
                    }
                    else if(as<HLSLPointStreamType>(outputStreamType))
                    {
                        stream->emit("layout(points) out;\n");
                    }
                }
            }


        }
        break;
        case Stage::Pixel:
        {
            if (irFunc->findDecoration<IREarlyDepthStencilDecoration>())
            {
                // https://www.khronos.org/opengl/wiki/Early_Fragment_Test
                stream->emit("layout(early_fragment_tests) in;\n");
            }
            break;
        }
        // TODO: There are other stages that will need this kind of handling.
        default:
            break;
        }
    }

    void emitIREntryPointAttributes(
        IRFunc*             irFunc,
        SharedEmitContext*        ctx,
        EntryPointLayout*   entryPointLayout)
    {
        switch(getTarget(ctx))
        {
        case CodeGenTarget::HLSL:
            emitIREntryPointAttributes_HLSL(irFunc, ctx, entryPointLayout);
            break;

        case CodeGenTarget::GLSL:
            emitIREntryPointAttributes_GLSL(irFunc, ctx, entryPointLayout);
            break;
        }
    }

    void emitPhiVarDecls(
        SharedEmitContext*    ctx,
        IRFunc*         func)
    {
        // We will skip the first block, since its parameters are
        // the parameters of the whole function.
        auto bb = func->getFirstBlock();
        if (!bb)
            return;
        bb = bb->getNextBlock();

        for (; bb; bb = bb->getNextBlock())
        {
            for (auto pp = bb->getFirstParam(); pp; pp = pp->getNextParam())
            {
                emitIRTempModifiers(ctx, pp);
                emitIRType(ctx, pp->getFullType(), getIRName(pp));
                stream->emit(";\n");
            }
        }
    }

    /// Emit high-level statements for the body of a function.
    void emitIRFunctionBody(
        SharedEmitContext*            ctx,
        IRGlobalValueWithCode*  code)
    {
        // Compute a structured region tree that can represent
        // the control flow of our function.
        //
        RefPtr<RegionTree> regionTree = generateRegionTreeForFunc(
            code,
            ctx->getSink());

        // Now that we've computed the region tree, we have
        // an opportunity to perform some last-minute transformations
        // on the code to make sure it follows our rules.
        //
        // TODO: it would be better to do these transformations earlier,
        // so that we can, e.g., dump the final IR code *before* emission
        // starts, but that gets a bit complicated because we also want
        // to have the region tree available without having to recompute it.
        //
        // For now we are just going to do things the expedient way, but
        // eventually we should allow an IR module to have side-band
        // storage for derived structures like the region tree (and logic
        // for invalidating them when a transformation would break them).
        //
        fixValueScoping(regionTree);

        // Now emit high-level code from that structured region tree.
        //
        emitRegionTree(ctx, regionTree);
    }

    void emitIRSimpleFunc(
        SharedEmitContext*    ctx,
        IRFunc*         func)
    {
        auto resultType = func->getResultType();

        // Deal with decorations that need
        // to be emitted as attributes
        auto entryPointLayout = asEntryPoint(func);
        if (entryPointLayout)
        {
            emitIREntryPointAttributes(func, ctx, entryPointLayout);
        }

        const CodeGenTarget target = ctx->target;

        auto name = getIRFuncName(func);

        EmitType(resultType, name);

        stream->emit("(");
        auto firstParam = func->getFirstParam();
        for( auto pp = firstParam; pp; pp = pp->getNextParam())
        {
            if(pp != firstParam)
                stream->emit(", ");

            auto paramName = getIRName(pp);
            auto paramType = pp->getDataType();

            if (target == CodeGenTarget::HLSL)
            {
                if (auto layoutDecor = pp->findDecoration<IRLayoutDecoration>())
                {
                    Layout* layout = layoutDecor->getLayout();
                    VarLayout* varLayout = as<VarLayout>(layout);

                    if (varLayout)
                    {
                        auto var = varLayout->getVariable();

                        if (auto primTypeModifier = var->FindModifier<HLSLGeometryShaderInputPrimitiveTypeModifier>())
                        {
                            if (as<HLSLTriangleModifier>(primTypeModifier))
                                stream->emit("triangle ");
                            else if (as<HLSLPointModifier>(primTypeModifier))
                                stream->emit("point ");
                            else if (as<HLSLLineModifier>(primTypeModifier))
                                stream->emit("line ");
                            else if (as<HLSLLineAdjModifier>(primTypeModifier))
                                stream->emit("lineadj ");
                            else if (as<HLSLTriangleAdjModifier>(primTypeModifier))
                                stream->emit("triangleadj ");
                        }
                    }
                }
            }

            emitIRParamType(ctx, paramType, paramName);

            emitIRSemantics(ctx, pp);
        }
        stream->emit(")");


        emitIRSemantics(ctx, func);

        // TODO: encode declaration vs. definition
        if(isDefinition(func))
        {
            stream->emit("\n{\n");
            stream->indent();

            // HACK: forward-declare all the local variables needed for the
            // parameters of non-entry blocks.
            emitPhiVarDecls(ctx, func);

            // Need to emit the operations in the blocks of the function
            emitIRFunctionBody(ctx, func);

            stream->dedent();
            stream->emit("}\n\n");
        }
        else
        {
            stream->emit(";\n\n");
        }
    }

    void emitIRParamType(
        SharedEmitContext*    ctx,
        IRType*         type,
        String const&   name)
    {
        // An `out` or `inout` parameter will have been
        // encoded as a parameter of pointer type, so
        // we need to decode that here.
        //
        if( auto outType = as<IROutType>(type))
        {
            stream->emit("out ");
            type = outType->getValueType();
        }
        else if( auto inOutType = as<IRInOutType>(type))
        {
            stream->emit("inout ");
            type = inOutType->getValueType();
        }
        else if( auto refType = as<IRRefType>(type))
        {
            // Note: There is no HLSL/GLSL equivalent for by-reference parameters,
            // so we don't actually expect to encounter these in user code.
            stream->emit("inout ");
            type = inOutType->getValueType();
        }

        emitIRType(ctx, type, name);
    }

    IRInst* getSpecializedValue(IRSpecialize* specInst)
    {
        auto base = specInst->getBase();
        auto baseGeneric = as<IRGeneric>(base);
        if (!baseGeneric)
            return base;

        auto lastBlock = baseGeneric->getLastBlock();
        if (!lastBlock)
            return base;

        auto returnInst = as<IRReturnVal>(lastBlock->getTerminator());
        if (!returnInst)
            return base;

        return returnInst->getVal();
    }

    void emitIRFuncDecl(
        SharedEmitContext*    ctx,
        IRFunc*         func)
    {
        // We don't want to emit declarations for operations
        // that only appear in the IR as stand-ins for built-in
        // operations on that target.
        if (isTargetIntrinsic(ctx, func))
            return;

        // Finally, don't emit a declaration for an entry point,
        // because it might need meta-data attributes attached
        // to it, and the HLSL compiler will get upset if the
        // forward declaration doesn't *also* have those
        // attributes.
        if(asEntryPoint(func))
            return;


        // A function declaration doesn't have any IR basic blocks,
        // and as a result it *also* doesn't have the IR `param` instructions,
        // so we need to emit a declaration entirely from the type.

        auto funcType = func->getDataType();
        auto resultType = func->getResultType();

        auto name = getIRFuncName(func);

        emitIRType(ctx, resultType, name);

        stream->emit("(");
        auto paramCount = funcType->getParamCount();
        for(UInt pp = 0; pp < paramCount; ++pp)
        {
            if(pp != 0)
                stream->emit(", ");

            String paramName;
            paramName.append("_");
            paramName.append(Int32(pp));
            auto paramType = funcType->getParamType(pp);

            emitIRParamType(ctx, paramType, paramName);
        }
        stream->emit(");\n\n");
    }

    EntryPointLayout* getEntryPointLayout(
        SharedEmitContext*    /*context*/,
        IRFunc*         func)
    {
        if( auto layoutDecoration = func->findDecoration<IRLayoutDecoration>() )
        {
            return as<EntryPointLayout>(layoutDecoration->getLayout());
        }
        return nullptr;
    }

    EntryPointLayout* asEntryPoint(IRFunc* func)
    {
        if (auto layoutDecoration = func->findDecoration<IRLayoutDecoration>())
        {
            if (auto entryPointLayout = as<EntryPointLayout>(layoutDecoration->getLayout()))
            {
                return entryPointLayout;
            }
        }

        return nullptr;
    }

    // Detect if the given IR function represents a
    // declaration of an intrinsic/builtin for the
    // current code-generation target.
    bool isTargetIntrinsic(
        SharedEmitContext*    /*ctxt*/,
        IRFunc*         func)
    {
        // For now we do this in an overly simplistic
        // fashion: we say that *any* function declaration
        // (rather then definition) must be an intrinsic:
        return !isDefinition(func);
    }

    // Check whether a given value names a target intrinsic,
    // and return the IR function representing the intrinsic
    // if it does.
    IRFunc* asTargetIntrinsic(
        SharedEmitContext*    ctxt,
        IRInst*         value)
    {
        if(!value)
            return nullptr;

        while (auto specInst = as<IRSpecialize>(value))
        {
            value = getSpecializedValue(specInst);
        }

        if(value->op != kIROp_Func)
            return nullptr;

        IRFunc* func = (IRFunc*) value;
        if(!isTargetIntrinsic(ctxt, func))
            return nullptr;

        return func;
    }

    void emitIRFunc(
        SharedEmitContext*    ctx,
        IRFunc*         func)
    {
        if(!isDefinition(func))
        {
            // This is just a function declaration,
            // and so we want to emit it as such.
            // (Or maybe not emit it at all).

            // We do not emit the declaration for
            // functions that appear to be intrinsics/builtins
            // in the target language.
            if (isTargetIntrinsic(ctx, func))
                return;

            emitIRFuncDecl(ctx, func);
        }
        else
        {
            // The common case is that what we
            // have is just an ordinary function,
            // and we can emit it as such.
            emitIRSimpleFunc(ctx, func);
        }
    }

    void emitIRStruct(
        SharedEmitContext*    ctx,
        IRStructType*   structType)
    {
        // If the selected `struct` type is actually an intrinsic
        // on our target, then we don't want to emit anything at all.
        if(auto intrinsicDecoration = findTargetIntrinsicDecoration(ctx, structType))
        {
            return;
        }

        stream->emit("struct ");
        stream->emit(getIRName(structType));
        stream->emit("\n{\n");
        stream->indent();

        for(auto ff : structType->getFields())
        {
            auto fieldKey = ff->getKey();
            auto fieldType = ff->getFieldType();

            // Filter out fields with `void` type that might
            // have been introduced by legalization.
            if(as<IRVoidType>(fieldType))
                continue;

            // Note: GLSL doesn't support interpolation modifiers on `struct` fields
            if( ctx->target != CodeGenTarget::GLSL )
            {
                emitInterpolationModifiers(ctx, fieldKey, fieldType, nullptr);
            }

            emitIRType(ctx, fieldType, getIRName(fieldKey));
            emitIRSemantics(ctx, fieldKey);
            stream->emit(";\n");
        }

        stream->dedent();
        stream->emit("};\n\n");
    }

    void emitIRMatrixLayoutModifiers(
        SharedEmitContext*    ctx,
        VarLayout*      layout)
    {
        // When a variable has a matrix type, we want to emit an explicit
        // layout qualifier based on what the layout has been computed to be.
        //

        auto typeLayout = layout->typeLayout;
        while(auto arrayTypeLayout = as<ArrayTypeLayout>(typeLayout))
            typeLayout = arrayTypeLayout->elementTypeLayout;

        if (auto matrixTypeLayout = typeLayout.as<MatrixTypeLayout>())
        {
            auto target = ctx->target;

            switch (target)
            {
            case CodeGenTarget::HLSL:
                switch (matrixTypeLayout->mode)
                {
                case kMatrixLayoutMode_ColumnMajor:
                    stream->emit("column_major ");
                    break;

                case kMatrixLayoutMode_RowMajor:
                    stream->emit("row_major ");
                    break;
                }
                break;

            case CodeGenTarget::GLSL:
                // Reminder: the meaning of row/column major layout
                // in our semantics is the *opposite* of what GLSL
                // calls them, because what they call "columns"
                // are what we call "rows."
                //
                switch (matrixTypeLayout->mode)
                {
                case kMatrixLayoutMode_ColumnMajor:
                    stream->emit("layout(row_major)\n");
                    break;

                case kMatrixLayoutMode_RowMajor:
                    stream->emit("layout(column_major)\n");
                    break;
                }
                break;

            default:
                break;
            }

        }

    }

    // Emit the `flat` qualifier if the underlying type
    // of the variable is an integer type.
    void maybeEmitGLSLFlatModifier(
        SharedEmitContext*,
        IRType*           valueType)
    {
        auto tt = valueType;
        if(auto vecType = as<IRVectorType>(tt))
            tt = vecType->getElementType();
        if(auto vecType = as<IRMatrixType>(tt))
            tt = vecType->getElementType();

        switch(tt->op)
        {
        default:
            break;

        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
            stream->emit("flat ");
            break;
        }
    }

    void emitInterpolationModifiers(
        SharedEmitContext*    ctx,
        IRInst*         varInst,
        IRType*         valueType,
        VarLayout*      layout)
    {
        bool isGLSL = (ctx->target == CodeGenTarget::GLSL);
        bool anyModifiers = false;

        for(auto dd : varInst->getDecorations())
        {
            if(dd->op != kIROp_InterpolationModeDecoration)
                continue;

            auto decoration = (IRInterpolationModeDecoration*)dd;
            auto mode = decoration->getMode();

            switch(mode)
            {
            case IRInterpolationMode::NoInterpolation:
                anyModifiers = true;
                stream->emit(isGLSL ? "flat " : "nointerpolation ");
                break;

            case IRInterpolationMode::NoPerspective:
                anyModifiers = true;
                stream->emit("noperspective ");
                break;

            case IRInterpolationMode::Linear:
                anyModifiers = true;
                stream->emit(isGLSL ? "smooth " : "linear ");
                break;

            case IRInterpolationMode::Sample:
                anyModifiers = true;
                stream->emit("sample ");
                break;

            case IRInterpolationMode::Centroid:
                anyModifiers = true;
                stream->emit("centroid ");
                break;

            default:
                break;
            }
        }

        // If the user didn't explicitly qualify a varying
        // with integer type, then we need to explicitly
        // add the `flat` modifier for GLSL.
        if(!anyModifiers && isGLSL)
        {
            // Only emit a default `flat` for fragment
            // stage varying inputs.
            //
            // TODO: double-check that this works for
            // signature matching even if the producing
            // stage didn't use `flat`.
            //
            // If this ends up being a problem we can instead
            // output everything with `flat` except for
            // fragment *outputs* (and maybe vertex inputs).
            //
            if(layout && layout->stage == Stage::Fragment
                && layout->FindResourceInfo(LayoutResourceKind::VaryingInput))
            {
                maybeEmitGLSLFlatModifier(ctx, valueType);
            }
        }
    }

    UInt getRayPayloadLocation(
        SharedEmitContext*    ctx,
        IRInst*         inst)
    {
        auto& map = ctx->mapIRValueToRayPayloadLocation;
        UInt value = 0;
        if(map.TryGetValue(inst, value))
            return value;

        value = map.Count();
        map.Add(inst, value);
        return value;
    }

    UInt getCallablePayloadLocation(
        SharedEmitContext*    ctx,
        IRInst*         inst)
    {
        auto& map = ctx->mapIRValueToCallablePayloadLocation;
        UInt value = 0;
        if(map.TryGetValue(inst, value))
            return value;

        value = map.Count();
        map.Add(inst, value);
        return value;
    }

    void emitGLSLImageFormatModifier(
        IRInst*         var,
        IRTextureType*  resourceType)
    {
        // If the user specified a format manually, using `[format(...)]`,
        // then we will respect that format and emit a matching `layout` modifier.
        //
        if(auto formatDecoration = var->findDecoration<IRFormatDecoration>())
        {
            auto format = formatDecoration->getFormat();
            if(format == ImageFormat::unknown)
            {
                // If the user explicitly opts out of having a format, then
                // the output shader will require the extension to support
                // load/store from format-less images.
                //
                // TODO: We should have a validation somewhere in the compiler
                // that atomic operations are only allowed on images with
                // explicit formats (and then only on specific formats).
                // This is really an argument that format should be part of
                // the image *type* (with a "base type" for images with
                // unknown format).
                //
                requireGLSLExtension("GL_EXT_shader_image_load_formatted");
            }
            else
            {
                // If there is an explicit format specified, then we
                // should emit a `layout` modifier using the GLSL name
                // for the format.
                //
                stream->emit("layout(");
                stream->emit(getGLSLNameForImageFormat(format));
                stream->emit(")\n");
            }

            // No matter what, if an explicit `[format(...)]` was given,
            // then we don't need to emit anything else.
            //
            return;
        }


        // When no explicit format is specified, we need to either
        // emit the image as having an unknown format, or else infer
        // a format from the type.
        //
        // For now our default behavior is to infer (so that unmodified
        // HLSL input is more likely to generate valid SPIR-V that
        // runs anywhere), but we provide a flag to opt into
        // treating images without explicit formats as having
        // unknown format.
        //
        if(this->context->compileRequest->useUnknownImageFormatAsDefault)
        {
            requireGLSLExtension("GL_EXT_shader_image_load_formatted");
            return;
        }

        // At this point we have a resource type like `RWTexture2D<X>`
        // and we want to infer a reasonable format from the element
        // type `X` that was specified.
        //
        // E.g., if `X` is `float` then we can infer a format like `r32f`,
        // and so forth. The catch of course is that it is possible to
        // specify a shader parameter with a type like `RWTexture2D<float4>` but
        // provide an image at runtime with a format like `rgba8`, so
        // this inference is never guaranteed to give perfect results.
        //
        // If users don't like our inferred result, they need to use a
        // `[format(...)]` attribute to manually specify what they want.
        //
        // TODO: We should consider whether we can expand the space of
        // allowed types for `X` in `RWTexture2D<X>` to include special
        // pseudo-types that act just like, e.g., `float4`, but come
        // with attached/implied format information.
        //
        auto elementType = resourceType->getElementType();
        Int vectorWidth = 1;
        if(auto elementVecType = as<IRVectorType>(elementType))
        {
            if(auto intLitVal = as<IRIntLit>(elementVecType->getElementCount()))
            {
                vectorWidth = (Int) intLitVal->getValue();
            }
            else
            {
                vectorWidth = 0;
            }
            elementType = elementVecType->getElementType();
        }
        if(auto elementBasicType = as<IRBasicType>(elementType))
        {
            stream->emit("layout(");
            switch(vectorWidth)
            {
            default: stream->emit("rgba");  break;

            case 3:
            {
                // TODO: GLSL doesn't support 3-component formats so for now we are going to
                // default to rgba
                //
                // The SPIR-V spec (https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.pdf)
                // section 3.11 on Image Formats it does not list rgbf32.
                //
                // It seems SPIR-V can support having an image with an unknown-at-compile-time
                // format, so long as the underlying API supports it. Ideally this would mean that we can
                // just drop all these qualifiers when emitting GLSL for Vulkan targets.
                //
                // This raises the question of what to do more long term. For Vulkan hopefully we can just
                // drop the layout. For OpenGL targets it would seem reasonable to have well-defined rules
                // for inferring the format (and just document that 3-component formats map to 4-component formats,
                // but that shouldn't matter because the API wouldn't let the user allocate those 3-component formats anyway),
                // and add an attribute for specifying the format manually if you really want to override our
                // inference (e.g., to specify r11fg11fb10f).

                stream->emit("rgba");
                //Emit("rgb");                                
                break;
            }

            case 2:  stream->emit("rg");    break;
            case 1:  stream->emit("r");     break;
            }
            switch(elementBasicType->getBaseType())
            {
            default:
            case BaseType::Float:   stream->emit("32f");  break;
            case BaseType::Half:    stream->emit("16f");  break;
            case BaseType::UInt:    stream->emit("32ui"); break;
            case BaseType::Int:     stream->emit("32i"); break;

            // TODO: Here are formats that are available in GLSL,
            // but that are not handled by the above cases.
            //
            // r11f_g11f_b10f
            //
            // rgba16
            // rgb10_a2
            // rgba8
            // rg16
            // rg8
            // r16
            // r8
            //
            // rgba16_snorm
            // rgba8_snorm
            // rg16_snorm
            // rg8_snorm
            // r16_snorm
            // r8_snorm
            //
            // rgba16i
            // rgba8i
            // rg16i
            // rg8i
            // r16i
            // r8i
            //
            // rgba16ui
            // rgb10_a2ui
            // rgba8ui
            // rg16ui
            // rg8ui
            // r16ui
            // r8ui
            }
            stream->emit(")\n");
        }
    }

        /// Emit modifiers that should apply even for a declaration of an SSA temporary.
    void emitIRTempModifiers(
        SharedEmitContext*    ctx,
        IRInst*         temp)
    {
        SLANG_UNUSED(ctx);

        if(temp->findDecoration<IRPreciseDecoration>())
        {
            stream->emit("precise ");
        }
    }

    void emitIRVarModifiers(
        SharedEmitContext*    ctx,
        VarLayout*      layout,
        IRInst*         varDecl,
        IRType*         varType)
    {
        // Deal with Vulkan raytracing layout stuff *before* we
        // do the check for whether `layout` is null, because
        // the payload won't automatically get a layout applied
        // (it isn't part of the user-visible interface...)
        //
        if(varDecl->findDecoration<IRVulkanRayPayloadDecoration>())
        {
            stream->emit("layout(location = ");
            stream->emit(getRayPayloadLocation(ctx, varDecl));
            stream->emit(")\n");
            stream->emit("rayPayloadNV\n");
        }
        if(varDecl->findDecoration<IRVulkanCallablePayloadDecoration>())
        {
            stream->emit("layout(location = ");
            stream->emit(getCallablePayloadLocation(ctx, varDecl));
            stream->emit(")\n");
            stream->emit("callableDataNV\n");
        }

        if(varDecl->findDecoration<IRVulkanHitAttributesDecoration>())
        {
            stream->emit("hitAttributeNV\n");
        }

        if(varDecl->findDecoration<IRGloballyCoherentDecoration>())
        {
            switch(getTarget(context))
            {
            default:
                break;

            case CodeGenTarget::HLSL:
                stream->emit("globallycoherent\n");
                break;

            case CodeGenTarget::GLSL:
                stream->emit("coherent\n");
                break;
            }
        }

        emitIRTempModifiers(ctx, varDecl);

        if (!layout)
            return;

        emitIRMatrixLayoutModifiers(ctx, layout);

        // As a special case, if we are emitting a GLSL declaration
        // for an HLSL `RWTexture*` then we need to emit a `format` layout qualifier.
        if(getTarget(context) == CodeGenTarget::GLSL)
        {
            if(auto resourceType = as<IRTextureType>(unwrapArray(varType)))
            {
                switch(resourceType->getAccess())
                {
                case SLANG_RESOURCE_ACCESS_READ_WRITE:
                case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                    {
                        emitGLSLImageFormatModifier(varDecl, resourceType);
                    }
                    break;

                default:
                    break;
                }
            }
        }

        if(layout->FindResourceInfo(LayoutResourceKind::VaryingInput)
            || layout->FindResourceInfo(LayoutResourceKind::VaryingOutput))
        {
            emitInterpolationModifiers(ctx, varDecl, varType, layout);
        }

        if (ctx->target == CodeGenTarget::GLSL)
        {
            // Layout-related modifiers need to come before the declaration,
            // so deal with them here.
            emitGLSLLayoutQualifiers(layout, nullptr);

            // try to emit an appropriate leading qualifier
            for (auto rr : layout->resourceInfos)
            {
                switch (rr.kind)
                {
                case LayoutResourceKind::Uniform:
                case LayoutResourceKind::ShaderResource:
                case LayoutResourceKind::DescriptorTableSlot:
                    stream->emit("uniform ");
                    break;

                case LayoutResourceKind::VaryingInput:
                    {
                        stream->emit("in ");
                    }
                    break;

                case LayoutResourceKind::VaryingOutput:
                    {
                        stream->emit("out ");
                    }
                    break;

                case LayoutResourceKind::RayPayload:
                    {
                        stream->emit("rayPayloadInNV ");
                    }
                    break;

                case LayoutResourceKind::CallablePayload:
                    {
                        stream->emit("callableDataInNV ");
                    }
                    break;

                case LayoutResourceKind::HitAttributes:
                    {
                        stream->emit("hitAttributeNV ");
                    }
                    break;

                default:
                    continue;
                }

                break;
            }
        }
    }

    void emitHLSLParameterGroup(
        SharedEmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type)
    {
        if(as<IRTextureBufferType>(type))
        {
            stream->emit("tbuffer ");
        }
        else
        {
            stream->emit("cbuffer ");
        }
        stream->emit(getIRName(varDecl));

        auto varLayout = getVarLayout(ctx, varDecl);
        SLANG_RELEASE_ASSERT(varLayout);

        EmitVarChain blockChain(varLayout);

        EmitVarChain containerChain = blockChain;
        EmitVarChain elementChain = blockChain;

        auto typeLayout = varLayout->typeLayout;
        if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout) )
        {
            containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
            elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

            typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
        }

        emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

        stream->emit("\n{\n");
        stream->indent();

        auto elementType = type->getElementType();

        emitIRType(ctx, elementType, getIRName(varDecl));
        stream->emit(";\n");

        stream->dedent();
        stream->emit("}\n");
    }

        /// Emit the array brackets that go on the end of a declaration of the given type.
    void emitArrayBrackets(
        SharedEmitContext*    ctx,
        IRType*         inType)
    {
        SLANG_UNUSED(ctx);

        // A declaration may require zero, one, or
        // more array brackets. When writing out array
        // brackets from left to right, they represent
        // the structure of the type from the "outside"
        // in (that is, if we have a 5-element array of
        // 3-element arrays we should output `[5][3]`),
        // because of C-style declarator rules.
        //
        // This conveniently means that we can print
        // out all the array brackets with a looping
        // rather than a recursive structure.
        //
        // We will peel the input type like an onion,
        // looking at one layer at a time until we
        // reach a non-array type in the middle.
        //
        IRType* type = inType;
        for(;;)
        {
            if(auto arrayType = as<IRArrayType>(type))
            {
                stream->emit("[");
                EmitVal(arrayType->getElementCount(), kEOp_General);
                stream->emit("]");

                // Continue looping on the next layer in.
                //
                type = arrayType->getElementType();
            }
            else if(auto unsizedArrayType = as<IRUnsizedArrayType>(type))
            {
                stream->emit("[]");

                // Continue looping on the next layer in.
                //
                type = unsizedArrayType->getElementType();
            }
            else
            {
                // This layer wasn't an array, so we are done.
                //
                return;
            }
        }
    }


    void emitGLSLParameterGroup(
        SharedEmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type)
    {
        auto varLayout = getVarLayout(ctx, varDecl);
        SLANG_RELEASE_ASSERT(varLayout);

        EmitVarChain blockChain(varLayout);

        EmitVarChain containerChain = blockChain;
        EmitVarChain elementChain = blockChain;

        auto typeLayout = varLayout->typeLayout->unwrapArray();
        if( auto parameterGroupTypeLayout = as<ParameterGroupTypeLayout>(typeLayout) )
        {
            containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
            elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

            typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
        }

        /*
        With resources backed by 'buffer' on glsl, we want to output 'readonly' if that is a good match
        for the underlying type. If uniform it's implicit it's readonly

        Here this only happens with isShaderRecord which is a 'constant buffer' (ie implicitly readonly)
        or IRGLSLShaderStorageBufferType which is read write.
        */

        emitGLSLLayoutQualifier(LayoutResourceKind::DescriptorTableSlot, &containerChain);
        emitGLSLLayoutQualifier(LayoutResourceKind::PushConstantBuffer, &containerChain);
        bool isShaderRecord = emitGLSLLayoutQualifier(LayoutResourceKind::ShaderRecord, &containerChain);

        if( isShaderRecord )
        {
            // TODO: A shader record in vk can be potentially read-write. Currently slang doesn't support write access
            // and readonly buffer generates SPIRV validation error.
            stream->emit("buffer ");
        }
        else if(as<IRGLSLShaderStorageBufferType>(type))
        {
            // Is writable 
            stream->emit("layout(std430) buffer ");
        }
        // TODO: what to do with HLSL `tbuffer` style buffers?
        else
        {
            // uniform is implicitly read only
            stream->emit("layout(std140) uniform ");
        }

        // Generate a dummy name for the block
        stream->emit("_S");
        stream->emit(ctx->uniqueIDCounter++);

        stream->emit("\n{\n");
        stream->indent();

        auto elementType = type->getElementType();

        emitIRType(ctx, elementType, "_data");
        stream->emit(";\n");

        stream->dedent();
        stream->emit("} ");

        stream->emit(getIRName(varDecl));

        // If the underlying variable was an array (or array of arrays, etc.)
        // we need to emit all those array brackets here.
        emitArrayBrackets(ctx, varDecl->getDataType());

        stream->emit(";\n");
    }

    void emitIRParameterGroup(
        SharedEmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRUniformParameterGroupType*    type)
    {
        switch (ctx->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLParameterGroup(ctx, varDecl, type);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLParameterGroup(ctx, varDecl, type);
            break;
        }
    }

    void emitIRVar(
        SharedEmitContext*    ctx,
        IRVar*          varDecl)
    {
        auto allocatedType = varDecl->getDataType();
        auto varType = allocatedType->getValueType();
//        auto addressSpace = allocatedType->getAddressSpace();

#if 0
        switch( varType->op )
        {
        case kIROp_ConstantBufferType:
        case kIROp_TextureBufferType:
            emitIRParameterGroup(ctx, varDecl, (IRUniformBufferType*) varType);
            return;

        default:
            break;
        }
#endif

        // Need to emit appropriate modifiers here.

        auto layout = getVarLayout(ctx, varDecl);

        emitIRVarModifiers(ctx, layout, varDecl, varType);

#if 0
        switch (addressSpace)
        {
        default:
            break;

        case kIRAddressSpace_GroupShared:
            emit("groupshared ");
            break;
        }
#endif
        emitIRRateQualifiers(ctx, varDecl);

        emitIRType(ctx, varType, getIRName(varDecl));

        emitIRSemantics(ctx, varDecl);

        emitIRLayoutSemantics(ctx, varDecl);

        stream->emit(";\n");
    }

    void emitIRStructuredBuffer_GLSL(
        SharedEmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRHLSLStructuredBufferTypeBase* structuredBufferType)
    {
        // Shader storage buffer is an OpenGL 430 feature
        //
        // TODO: we should require either the extension or the version...
        requireGLSLVersion(430);

        stream->emit("layout(std430");

        auto layout = getVarLayout(ctx, varDecl);
        if (layout)
        {
            LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
            EmitVarChain chain(layout);

            const UInt index = getBindingOffset(&chain, kind);
            const UInt space = getBindingSpace(&chain, kind);

            stream->emit(", binding = ");
            stream->emit(index);
            if (space)
            {
                stream->emit(", set = ");
                stream->emit(space);
            }
        }
        
        stream->emit(") ");

        /*
        If the output type is a buffer, and we can determine it is only readonly we can prefix before
        buffer with 'readonly'

        The actual structuredBufferType could be

        HLSLStructuredBufferType                        - This is unambiguously read only
        HLSLRWStructuredBufferType                      - Read write
        HLSLRasterizerOrderedStructuredBufferType       - Allows read/write access
        HLSLAppendStructuredBufferType                  - Write
        HLSLConsumeStructuredBufferType                 - TODO (JS): Its possible that this can be readonly, but we currently don't support on GLSL
        */

        if (as<IRHLSLStructuredBufferType>(structuredBufferType))
        {
            stream->emit("readonly ");
        }

        stream->emit("buffer ");
    
        // Generate a dummy name for the block
        stream->emit("_S");
        stream->emit(ctx->uniqueIDCounter++);

        stream->emit(" {\n");
        stream->indent();


        auto elementType = structuredBufferType->getElementType();
        emitIRType(ctx, elementType, "_data[]");
        stream->emit(";\n");

        stream->dedent();
        stream->emit("} ");

        stream->emit(getIRName(varDecl));
        emitArrayBrackets(ctx, varDecl->getDataType());

        stream->emit(";\n");
    }

    void emitIRByteAddressBuffer_GLSL(
        SharedEmitContext*                    ctx,
        IRGlobalParam*                  varDecl,
        IRByteAddressBufferTypeBase*    byteAddressBufferType)
    {
        // TODO: A lot of this logic is copy-pasted from `emitIRStructuredBuffer_GLSL`.
        // It might be worthwhile to share the common code to avoid regressions sneaking
        // in when one or the other, but not both, gets updated.

        // Shader storage buffer is an OpenGL 430 feature
        //
        // TODO: we should require either the extension or the version...
        requireGLSLVersion(430);

        stream->emit("layout(std430");

        auto layout = getVarLayout(ctx, varDecl);
        if (layout)
        {
            LayoutResourceKind kind = LayoutResourceKind::DescriptorTableSlot;
            EmitVarChain chain(layout);

            const UInt index = getBindingOffset(&chain, kind);
            const UInt space = getBindingSpace(&chain, kind);

            stream->emit(", binding = ");
            stream-> emit(index);
            if (space)
            {
                stream->emit(", set = ");
                stream->emit(space);
            }
        }

        stream->emit(") ");

        /*
        If the output type is a buffer, and we can determine it is only readonly we can prefix before
        buffer with 'readonly'

        HLSLByteAddressBufferType                   - This is unambiguously read only
        HLSLRWByteAddressBufferType                 - Read write
        HLSLRasterizerOrderedByteAddressBufferType  - Allows read/write access
        */

        if (as<IRHLSLByteAddressBufferType>(byteAddressBufferType))
        {
            stream->emit("readonly ");
        }

        stream->emit("buffer ");

        // Generate a dummy name for the block
        stream->emit("_S");
        stream->emit(ctx->uniqueIDCounter++);
        stream->emit("\n{\n");
        stream->indent();

        stream->emit("uint _data[];\n");

        stream->dedent();
        stream->emit("} ");

        stream->emit(getIRName(varDecl));
        emitArrayBrackets(ctx, varDecl->getDataType());

        stream->emit(";\n");
    }

    void emitIRGlobalVar(
        SharedEmitContext*    ctx,
        IRGlobalVar*    varDecl)
    {
        auto allocatedType = varDecl->getDataType();
        auto varType = allocatedType->getValueType();

        String initFuncName;
        if (varDecl->getFirstBlock())
        {
            // A global variable with code means it has an initializer
            // associated with it. Eventually we'd like to emit that
            // initializer directly as an expression here, but for
            // now we'll emit it as a separate function.

            initFuncName = getIRName(varDecl);
            initFuncName.append("_init");

            stream->emit("\n");
            emitIRType(ctx, varType, initFuncName);
            stream->emit("()\n{\n");
            stream->indent();
            emitIRFunctionBody(ctx, varDecl);
            stream->dedent();
            stream->emit("}\n");
        }

        // An ordinary global variable won't have a layout
        // associated with it, since it is not a shader
        // parameter.
        //
        SLANG_ASSERT(!getVarLayout(ctx, varDecl));
        VarLayout* layout = nullptr;

        // An ordinary global variable (which is not a
        // shader parameter) may need special
        // modifiers to indicate it as such.
        //
        switch (getTarget(ctx))
        {
        case CodeGenTarget::HLSL:
            // HLSL requires the `static` modifier on any
            // global variables; otherwise they are assumed
            // to be uniforms.
            stream->emit("static ");
            break;

        default:
            break;
        }

        emitIRVarModifiers(ctx, layout, varDecl, varType);

        emitIRRateQualifiers(ctx, varDecl);
        emitIRType(ctx, varType, getIRName(varDecl));

        // TODO: These shouldn't be needed for ordinary
        // global variables.
        //
        emitIRSemantics(ctx, varDecl);
        emitIRLayoutSemantics(ctx, varDecl);

        if (varDecl->getFirstBlock())
        {
            stream->emit(" = ");
            stream->emit(initFuncName);
            stream->emit("()");
        }

        stream->emit(";\n\n");
    }

    void emitIRGlobalParam(
        SharedEmitContext*    ctx,
        IRGlobalParam*  varDecl)
    {
        auto rawType = varDecl->getDataType();

        auto varType = rawType;
        if( auto outType = as<IROutTypeBase>(varType) )
        {
            varType = outType->getValueType();
        }
        if (as<IRVoidType>(varType))
            return;

        // When a global shader parameter represents a "parameter group"
        // (either a constant buffer or a parameter block with non-resource
        // data in it), we will prefer to emit it as an ordinary `cbuffer`
        // declaration or `uniform` block, even when emitting HLSL for
        // D3D profiles that support the explicit `ConstantBuffer<T>` type.
        //
        // Alternatively, we could make this choice based on profile, and
        // prefer `ConstantBuffer<T>` on profiles that support it and/or when
        // the input code used that syntax.
        //
        if (auto paramBlockType = as<IRUniformParameterGroupType>(varType))
        {
            emitIRParameterGroup(
                ctx,
                varDecl,
                paramBlockType);
            return;
        }

        if(getTarget(ctx) == CodeGenTarget::GLSL)
        {
            // There are a number of types that are (or can be)
            // "first-class" in D3D HLSL, but are second-class in GLSL in
            // that they require explicit global declarations for each value/object,
            // and don't support declaration as ordinary variables.
            //
            // This includes constant buffers (`uniform` blocks) and well as
            // structured and byte-address buffers (both mapping to `buffer` blocks).
            //
            // We intercept these types, and arrays thereof, to produce the required
            // global declarations. This assumes that earlier "legalization" passes
            // already performed the work of pulling fields with these types out of
            // aggregates.
            //
            // Note: this also assumes that these types are not used as function
            // parameters/results, local variables, etc. Additional legalization
            // steps are required to guarantee these conditions.
            //
            if (auto paramBlockType = as<IRUniformParameterGroupType>(unwrapArray(varType)))
            {
                emitGLSLParameterGroup(
                    ctx,
                    varDecl,
                    paramBlockType);
                return;
            }
            if( auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(unwrapArray(varType)) )
            {
                emitIRStructuredBuffer_GLSL(
                    ctx,
                    varDecl,
                    structuredBufferType);
                return;
            }
            if( auto byteAddressBufferType = as<IRByteAddressBufferTypeBase>(unwrapArray(varType)) )
            {
                emitIRByteAddressBuffer_GLSL(
                    ctx,
                    varDecl,
                    byteAddressBufferType);
                return;
            }

            // We want to skip the declaration of any system-value variables
            // when outputting GLSL (well, except in the case where they
            // actually *require* redeclaration...).
            //
            // Note: these won't be variables the user declare explicitly
            // in their code, but rather variables that we generated as
            // part of legalizing the varying input/output signature of
            // an entry point for GL/Vulkan.
            //
            // TODO: This could be handled more robustly by attaching an
            // appropriate decoration to these variables to indicate their
            // purpose.
            //
            if(auto linkageDecoration = varDecl->findDecoration<IRLinkageDecoration>())
            {
                if(linkageDecoration->getMangledName().startsWith("gl_"))
                {
                    // The variable represents an OpenGL system value,
                    // so we will assume that it doesn't need to be declared.
                    //
                    // TODO: handle case where we *should* declare the variable.
                    return;
                }
            }

            // When emitting unbounded-size resource arrays with GLSL we need
            // to use the `GL_EXT_nonuniform_qualifier` extension to ensure
            // that they are not treated as "implicitly-sized arrays" which
            // are arrays that have a fixed size that just isn't specified
            // at the declaration site (instead being inferred from use sites).
            //
            // While the extension primarily introduces the `nonuniformEXT`
            // qualifier that we use to implement `NonUniformResourceIndex`,
            // it also changes the GLSL language semantics around (resource) array
            // declarations that don't specify a size.
            //
            if( as<IRUnsizedArrayType>(varType) )
            {
                if(isResourceType(unwrapArray(varType)))
                {
                    requireGLSLExtension("GL_EXT_nonuniform_qualifier");
                }
            }
        }

        // Need to emit appropriate modifiers here.

        // We expect/require all shader parameters to
        // have some kind of layout information associted with them.
        //
        auto layout = getVarLayout(ctx, varDecl);
        SLANG_ASSERT(layout);

        emitIRVarModifiers(ctx, layout, varDecl, varType);

        emitIRRateQualifiers(ctx, varDecl);
        emitIRType(ctx, varType, getIRName(varDecl));

        emitIRSemantics(ctx, varDecl);

        emitIRLayoutSemantics(ctx, varDecl);

        // A shader parameter cannot have an initializer,
        // so we do need to consider emitting one here.

        stream->emit(";\n\n");
    }


    void emitIRGlobalConstantInitializer(
        SharedEmitContext*        ctx,
        IRGlobalConstant*   valDecl)
    {
        // We expect to see only a single block
        auto block = valDecl->getFirstBlock();
        SLANG_RELEASE_ASSERT(block);
        SLANG_RELEASE_ASSERT(!block->getNextBlock());

        // We expect the terminator to be a `return`
        // instruction with a value.
        auto returnInst = (IRReturnVal*) block->getLastDecorationOrChild();
        SLANG_RELEASE_ASSERT(returnInst->op == kIROp_ReturnVal);

        // We will emit the value in the `GlobalConstant` mode, which
        // more or less says to fold all instructions into their use
        // sites, so that we end up with a single expression tree even
        // in cases that would otherwise trip up our analysis.
        //
        // Note: We are emitting the value as an *operand* here instead
        // of directly calling `emitIRInstExpr` because we need to handle
        // cases where the value might *need* to emit as a named referenced
        // (e.g., when it names another constant directly).
        //
        emitIROperand(ctx, returnInst->getVal(), IREmitMode::GlobalConstant, kEOp_General);
    }

    void emitIRGlobalConstant(
        SharedEmitContext*        ctx,
        IRGlobalConstant*   valDecl)
    {
        auto valType = valDecl->getDataType();

        if( ctx->target != CodeGenTarget::GLSL )
        {
            stream->emit("static ");
        }
        stream->emit("const ");
        emitIRRateQualifiers(ctx, valDecl);
        emitIRType(ctx, valType, getIRName(valDecl));

        if (valDecl->getFirstBlock())
        {
            // There is an initializer (which we expect for
            // any global constant...).

            stream->emit(" = ");

            // We need to emit the entire initializer as
            // a single expression.
            emitIRGlobalConstantInitializer(ctx, valDecl);
        }


        stream->emit(";\n");
    }

    void emitIRGlobalInst(
        SharedEmitContext*    ctx,
        IRInst*         inst)
    {
        stream->advanceToSourceLocation(inst->sourceLoc);

        switch(inst->op)
        {
        case kIROp_Func:
            emitIRFunc(ctx, (IRFunc*) inst);
            break;

        case kIROp_GlobalVar:
            emitIRGlobalVar(ctx, (IRGlobalVar*) inst);
            break;

        case kIROp_GlobalParam:
            emitIRGlobalParam(ctx, (IRGlobalParam*) inst);
            break;

        case kIROp_GlobalConstant:
            emitIRGlobalConstant(ctx, (IRGlobalConstant*) inst);
            break;

        case kIROp_Var:
            emitIRVar(ctx, (IRVar*) inst);
            break;

        case kIROp_StructType:
            emitIRStruct(ctx, cast<IRStructType>(inst));
            break;

        default:
            break;
        }
    }

    // An action to be performed during code emit.
    struct EmitAction
    {
        enum Level
        {
            ForwardDeclaration,
            Definition,
        };
        Level   level;
        IRInst* inst;
    };

    struct ComputeEmitActionsContext
    {
        IRInst*             moduleInst;
        HashSet<IRInst*>    openInsts;
        Dictionary<IRInst*, EmitAction::Level> mapInstToLevel;
        List<EmitAction>*   actions;
    };

    void ensureInstOperand(
        ComputeEmitActionsContext*  ctx,
        IRInst*                     inst,
        EmitAction::Level           requiredLevel = EmitAction::Level::Definition)
    {
        if(!inst) return;

        if(inst->getParent() == ctx->moduleInst)
        {
            ensureGlobalInst(ctx, inst, requiredLevel);
        }
    }

    void ensureInstOperandsRec(
        ComputeEmitActionsContext*  ctx,
        IRInst*                     inst)
    {
        ensureInstOperand(ctx, inst->getFullType());

        UInt operandCount = inst->operandCount;
        for(UInt ii = 0; ii < operandCount; ++ii)
        {
            // TODO: there are some special cases we can add here,
            // to avoid outputting full definitions in cases that
            // can get by with forward declarations.
            //
            // For example, true pointer types should (in principle)
            // only need the type they point to to be forward-declared.
            // Similarly, a `call` instruction only needs the callee
            // to be forward-declared, etc.

            ensureInstOperand(ctx, inst->getOperand(ii));
        }

        for(auto child : inst->getDecorationsAndChildren())
        {
            ensureInstOperandsRec(ctx, child);
        }
    }

    void ensureGlobalInst(
        ComputeEmitActionsContext*  ctx,
        IRInst*                     inst,
        EmitAction::Level           requiredLevel)
    {
        // Skip certain instrutions, since they
        // don't affect output.
        switch(inst->op)
        {
        case kIROp_WitnessTable:
        case kIROp_Generic:
            return;

        default:
            break;
        }

        // Have we already processed this instruction?
        EmitAction::Level existingLevel;
        if(ctx->mapInstToLevel.TryGetValue(inst, existingLevel))
        {
            // If we've already emitted it suitably,
            // then don't worry about it.
            if(existingLevel >= requiredLevel)
                return;
        }

        EmitAction action;
        action.level = requiredLevel;
        action.inst = inst;

        if(requiredLevel == EmitAction::Level::Definition)
        {
            if(ctx->openInsts.Contains(inst))
            {
                SLANG_UNEXPECTED("circularity during codegen");
                return;
            }

            ctx->openInsts.Add(inst);

            ensureInstOperandsRec(ctx, inst);

            ctx->openInsts.Remove(inst);
        }

        ctx->mapInstToLevel[inst] = requiredLevel;
        ctx->actions->add(action);
    }

    void computeIREmitActions(
        IRModule*           module,
        List<EmitAction>&   ioActions)
    {
        ComputeEmitActionsContext ctx;
        ctx.moduleInst = module->getModuleInst();
        ctx.actions = &ioActions;

        for(auto inst : module->getGlobalInsts())
        {
            if( as<IRType>(inst) )
            {
                // Don't emit a type unless it is actually used.
                continue;
            }

            ensureGlobalInst(&ctx, inst, EmitAction::Level::Definition);
        }
    }

    void executeIREmitActions(
        SharedEmitContext*                ctx,
        List<EmitAction> const&     actions)
    {
        for(auto action : actions)
        {
            switch(action.level)
            {
            case EmitAction::Level::ForwardDeclaration:
                emitIRFuncDecl(ctx, cast<IRFunc>(action.inst));
                break;

            case EmitAction::Level::Definition:
                emitIRGlobalInst(ctx, action.inst);
                break;
            }
        }
    }

    void emitIRModule(
        SharedEmitContext*    ctx,
        IRModule*       module)
    {
        // The IR will usually come in an order that respects
        // dependencies between global declarations, but this
        // isn't guaranteed, so we need to be careful about
        // the order in which we emit things.

        List<EmitAction> actions;

        computeIREmitActions(module, actions);
        executeIREmitActions(ctx, actions);
    }
};

//


//

EntryPointLayout* findEntryPointLayout(
    ProgramLayout*  programLayout,
    EntryPoint*     entryPoint)
{
    for( auto entryPointLayout : programLayout->entryPoints )
    {
        if(entryPointLayout->entryPoint->getName() != entryPoint->getName())
            continue;

        // TODO: We need to be careful about this check, since it relies on
        // the profile information in the layout matching that in the request.
        //
        // What we really seem to want here is some dictionary mapping the
        // `EntryPoint` directly to the `EntryPointLayout`, and maybe
        // that is precisely what we should build...
        //
        if(entryPointLayout->profile != entryPoint->getProfile())
            continue;

        // TODO: can't easily filter on translation unit here...
        // Ideally the `EntryPoint` should get filled in with a pointer
        // the specific function declaration that represents the entry point.

        return entryPointLayout.Ptr();
    }

    return nullptr;
}

    /// Given a layout computed for a scope, get the layout to use when lookup up variables.
    ///
    /// A scope (such as the global scope of a program) groups its
    /// parameters into a pseudo-`struct` type for layout purposes,
    /// and in some cases that type will in turn be wrapped in a
    /// `ConstantBuffer` type to indicate that the parameters needed
    /// an implicit constant buffer to be allocated.
    ///
    /// This function "unwraps" the type layout to find the structure
    /// type layout that must be stored inside.
    ///
StructTypeLayout* getScopeStructLayout(
    ScopeLayout*  scopeLayout)
{
    auto scopeTypeLayout = scopeLayout->parametersLayout->typeLayout;

    if( auto constantBufferTypeLayout = as<ParameterGroupTypeLayout>(scopeTypeLayout) )
    {
        scopeTypeLayout = constantBufferTypeLayout->offsetElementTypeLayout;
    }

    if( auto structTypeLayout = as<StructTypeLayout>(scopeTypeLayout) )
    {
        return structTypeLayout;
    }

    SLANG_UNEXPECTED("uhandled global-scope binding layout");
    return nullptr;
}

    /// Given a layout computed for a program, get the layout to use when lookup up variables.
    ///
    /// This is just an alias of `getScopeStructLayout`.
    ///
StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout)
{
    return getScopeStructLayout(programLayout);
}

static void dumpIR(
    BackEndCompileRequest* compileRequest,
    IRModule*       irModule,
    char const*     label)
{
    DiagnosticSinkWriter writerImpl(compileRequest->getSink());
    WriterHelper writer(&writerImpl);

    if(label)
    {
        writer.put("### ");
        writer.put(label);
        writer.put(":\n");
    }

    dumpIR(irModule, writer.getWriter());

    if( label )
    {
        writer.put("###\n");
    }
}

static void dumpIRIfEnabled(
    BackEndCompileRequest* compileRequest,
    IRModule*       irModule,
    char const*     label = nullptr)
{
    if(compileRequest->shouldDumpIR)
    {
        dumpIR(compileRequest, irModule, label);
    }
}

String emitEntryPoint(
    BackEndCompileRequest*  compileRequest,
    EntryPoint*             entryPoint,
    CodeGenTarget           target,
    TargetRequest*          targetRequest)
{
    auto sink = compileRequest->getSink();
    auto program = compileRequest->getProgram();
    auto targetProgram = program->getTargetProgram(targetRequest);
    auto programLayout = targetProgram->getOrCreateLayout(sink);

//    auto translationUnit = entryPoint->getTranslationUnit();

    auto lineDirectiveMode = compileRequest->getLineDirectiveMode();
    // To try to make the default behavior reasonable, we will
    // always use C-style line directives (to give the user
    // good source locations on error messages from downstream
    // compilers) *unless* they requested raw GLSL as the
    // output (in which case we want to maximize compatibility
    // with downstream tools).
    if (lineDirectiveMode ==  LineDirectiveMode::Default && targetRequest->getTarget() == CodeGenTarget::GLSL)
    {
        lineDirectiveMode = LineDirectiveMode::GLSL;
    }

    SourceStream sourceStream(compileRequest->getSourceManager(), lineDirectiveMode );

    SharedEmitContext sharedContext;
    sharedContext.compileRequest = compileRequest;
    sharedContext.target = target;
    sharedContext.entryPoint = entryPoint;
    sharedContext.effectiveProfile = getEffectiveProfile(entryPoint, targetRequest);
    sharedContext.stream = &sourceStream;

    if (entryPoint && programLayout)
    {
        sharedContext.entryPointLayout = findEntryPointLayout(
            programLayout,
            entryPoint);
    }

    sharedContext.programLayout = programLayout;

    // Layout information for the global scope is either an ordinary
    // `struct` in the common case, or a constant buffer in the case
    // where there were global-scope uniforms.
    
    StructTypeLayout* globalStructLayout = programLayout ? getGlobalStructLayout(programLayout) : nullptr;
    sharedContext.globalStructLayout = globalStructLayout;

    EmitVisitor visitor(&sharedContext);

    {
        auto session = targetRequest->getSession();

        // We start out by performing "linking" at the level of the IR.
        // This step will create a fresh IR module to be used for
        // code generation, and will copy in any IR definitions that
        // the desired entry point requires. Along the way it will
        // resolve references to imported/exported symbols across
        // modules, and also select between the definitions of
        // any "profile-overloaded" symbols.
        //
        auto linkedIR = linkIR(
            compileRequest,
            entryPoint,
            programLayout,
            target,
            targetRequest);
        auto irModule = linkedIR.module;
        auto irEntryPoint = linkedIR.entryPoint;

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "LINKED");
#endif

        validateIRModuleIfEnabled(compileRequest, irModule);

        // If the user specified the flag that they want us to dump
        // IR, then do it here, for the target-specific, but
        // un-specialized IR.
        dumpIRIfEnabled(compileRequest, irModule);

        // When there are top-level existential-type parameters
        // to the shader, we need to take the side-band information
        // on how the existential "slots" were bound to concrete
        // types, and use it to introduce additional explicit
        // shader parameters for those slots, to be wired up to
        // use sites.
        //
        bindExistentialSlots(irModule, sink);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "EXISTENTIALS BOUND");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);





        // Now that we've linked the IR code, any layout/binding
        // information has been attached to shader parameters
        // and entry points. Now we are safe to make transformations
        // that might move code without worrying about losing
        // the connection between a parameter and its layout.
        //
        // An easy transformation of this kind is to take uniform
        // parameters of a shader entry point and move them into
        // the global scope instead.
        //
        moveEntryPointUniformParamsToGlobalScope(irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "ENTRY POINT UNIFORMS MOVED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // Desguar any union types, since these will be illegal on
        // various targets.
        //
        desugarUnionTypes(irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "UNIONS DESUGARED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // Next, we need to ensure that the code we emit for
        // the target doesn't contain any operations that would
        // be illegal on the target platform. For example,
        // none of our target supports generics, or interfaces,
        // so we need to specialize those away.
        //
        // Simplification of existential-based and generics-based
        // code may each open up opportunities for the other, so
        // the relevant specialization transformations are handled in a
        // single pass that looks for all simplification opportunities.
        //
        // TODO: We also need to extend this pass so that it will "expose"
        // existential values that are nested inside of other types,
        // so that the simplifications can be applied.
        //
        // TODO: This pass is *also* likely to be the place where we
        // perform specialization of functions based on parameter
        // values that need to be compile-time constants.
        //
        specializeModule(irModule);

        // Debugging code for IR transformations...
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "SPECIALIZED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);


        // Specialization can introduce dead code that could trip
        // up downstream passes like type legalization, so we
        // will run a DCE pass to clean up after the specialization.
        //
        // TODO: Are there other cleanup optimizations we should
        // apply at this point?
        //
        eliminateDeadCode(compileRequest, irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // The Slang language allows interfaces to be used like
        // ordinary types (including placing them in constant
        // buffers and entry-point parameter lists), but then
        // getting them to lay out in a reasonable way requires
        // us to treat fields/variables with interface type
        // *as if* they were pointers to heap-allocated "objects."
        //
        // Specialization will have replaced fields/variables
        // with interface types like `IFoo` with fields/variables
        // with pointer-like types like `ExistentialBox<SomeType>`.
        //
        // We need to legalize these pointer-like types away,
        // which involves two main changes:
        //
        //  1. Any `ExistentialBox<...>` fields need to be moved
        //  out of their enclosing `struct` type, so that the layout
        //  of the enclosing type is computed as if the field had
        //  zero size.
        //
        //  2. Once an `ExistentialBox<X>` has been floated out
        //  of its parent and landed somwhere permanent (e.g., either
        //  a dedicated variable, or a field of constant buffer),
        //  we need to replace it with just an `X`, after which we
        //  will have (more) legal shader code.
        //
        legalizeExistentialTypeLayout(
            irModule,
            sink);
        eliminateDeadCode(compileRequest, irModule);

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "EXISTENTIALS LEGALIZED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // Many of our target languages and/or downstream compilers
        // don't support `struct` types that have resource-type fields.
        // In order to work around this limitation, we will rewrite the
        // IR so that any structure types with resource-type fields get
        // split into a "tuple" that comprises the ordinary fields (still
        // bundles up as a `struct`) and one element for each resource-type
        // field (recursively).
        //
        // What used to be individual variables/parameters/arguments/etc.
        // then become multiple variables/parameters/arguments/etc.
        //
        legalizeResourceTypes(
            irModule,
            sink);
        eliminateDeadCode(compileRequest, irModule);

        //  Debugging output of legalization
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "LEGALIZED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // Once specialization and type legalization have been performed,
        // we should perform some of our basic optimization steps again,
        // to see if we can clean up any temporaries created by legalization.
        // (e.g., things that used to be aggregated might now be split up,
        // so that we can work with the individual fields).
        constructSSA(irModule);

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER SSA");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // After type legalization and subsequent SSA cleanup we expect
        // that any resource types passed to functions are exposed
        // as their own top-level parameters (which might have
        // resource or array-of-...-resource types).
        //
        // Many of our targets place restrictions on how certain
        // resource types can be used, so that having them as
        // function parameters is invalid. To clean this up,
        // we will try to specialize called functions based
        // on the actual resources that are being passed to them
        // at specific call sites.
        //
        // Because the legalization may depend on what target
        // we are compiling for (certain things might be okay
        // for D3D targets that are not okay for Vulkan), we
        // pass down the target request along with the IR.
        //
        specializeResourceParameters(compileRequest, targetRequest, irModule);

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER RESOURCE SPECIALIZATION");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);


        // For GLSL only, we will need to perform "legalization" of
        // the entry point and any entry-point parameters.
        //
        // TODO: We should consider moving this legalization work
        // as late as possible, so that it doesn't affect how other
        // optimization passes need to work.
        //
        switch (target)
        {
        case CodeGenTarget::GLSL:
        {
            legalizeEntryPointForGLSL(
                session,
                irModule,
                irEntryPoint,
                compileRequest->getSink(),
                &sharedContext.extensionUsageTracker);

#if 0
                dumpIRIfEnabled(compileRequest, irModule, "GLSL LEGALIZED");
#endif
                validateIRModuleIfEnabled(compileRequest, irModule);
        }
        break;

        default:
            break;
        }

        // The resource-based specialization pass above
        // may create specialized versions of functions, but
        // it does not try to completely eliminate the original
        // functions, so there might still be invalid code in
        // our IR module.
        //
        // To clean up the code, we will apply a fairly general
        // dead-code-elimination (DCE) pass that only retains
        // whatever code is "live."
        //
        eliminateDeadCode(compileRequest, irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // After all of the required optimization and legalization
        // passes have been performed, we can emit target code from
        // the IR module.
        //
        // TODO: do we want to emit directly from IR, or translate the
        // IR back into AST for emission?
        visitor.emitIRModule(&sharedContext, irModule);
    }

    // Deal with cases where a particular stage requires certain GLSL versions
    // and/or extensions.
    switch( entryPoint->getStage() )
    {
    default:
        break;

    case Stage::AnyHit:
    case Stage::Callable:
    case Stage::ClosestHit:
    case Stage::Intersection:
    case Stage::Miss:
    case Stage::RayGeneration:
        if( target == CodeGenTarget::GLSL )
        {
            requireGLSLExtension(&sharedContext.extensionUsageTracker, "GL_NV_ray_tracing");
            requireGLSLVersionImpl(&sharedContext.extensionUsageTracker, ProfileVersion::GLSL_460);
        }
        break;
    }

    String code = sourceStream.getContent();
    sourceStream.clearContent();

    // Now that we've emitted the code for all the declarations in the file,
    // it is time to stitch together the final output.

    // There may be global-scope modifiers that we should emit now
    visitor.emitGLSLPreprocessorDirectives();

    visitor.emitLayoutDirectives(targetRequest);

    String prefix = sourceStream.getContent();
    
    StringBuilder finalResultBuilder;
    finalResultBuilder << prefix;

    finalResultBuilder << sharedContext.extensionUsageTracker.glslExtensionRequireLines.ProduceString();

    finalResultBuilder << code;

    String finalResult = finalResultBuilder.ProduceString();

    return finalResult;
}

} // namespace Slang
