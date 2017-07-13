// emit.cpp
#include "emit.h"

#include "lower.h"
#include "syntax.h"
#include "type-layout.h"
#include "visitor.h"

#include <assert.h>

#ifdef _WIN32
#include <d3dcompiler.h>
#pragma warning(disable:4996)
#endif

namespace Slang {

// Shared state for an entire emit session
struct SharedEmitContext
{
    // The target language we want to generate code for
    CodeGenTarget target;

    // The final code generation target
    //
    // For example, `target` might be `GLSL`, while `finalTarget` might be `SPIRV`
    CodeGenTarget finalTarget;

    // The string of code we've built so far
    StringBuilder sb;

    // Current source position for tracking purposes...
    CodePosition loc;
    CodePosition nextSourceLocation;
    bool needToUpdateSourceLocation;

    // For GLSL output, we can't emit traidtional `#line` directives
    // with a file path in them, so we maintain a map that associates
    // each path with a unique integer, and then we output those
    // instead.
    Dictionary<String, int> mapGLSLSourcePathToID;
    int glslSourceIDCount = 0;

    // We only want to emit each `import`ed module one time, so
    // we maintain a set of already-emitted modules.
    HashSet<ProgramSyntaxNode*> modulesAlreadyEmitted;

    // We track the original global-scope layout so that we can
    // find layout information for `import`ed parameters.
    //
    // TODO: This will probably change if we represent imports
    // explicitly in the layout data.
    StructTypeLayout*   globalStructLayout;

    ProgramLayout*      programLayout;

    ProgramSyntaxNode*  program;

    bool                needHackSamplerForTexelFetch = false;

    // Record the GLSL extnsions we have already emitted a `#extension` for
    HashSet<String> glslExtensionsRequired;
    StringBuilder glslExtensionRequireLines;
};

struct EmitContext
{
    // The shared context that is in effect
    SharedEmitContext* shared;
};

//

static String getStringOrIdentifierTokenValue(
    Token const& token)
{
    switch(token.Type)
    {
    default:
        assert(!"unexpected");
        return "";

    case TokenType::Identifier:
        return token.Content;

    case TokenType::StringLiteral:
        return getStringLiteralTokenValue(token);
        break;
    }
}


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

#if 0

    kEPrecedence_None,
    kEPrecedence_Comma,

    kEPrecedence_Assign,
    kEPrecedence_AddAssign = kEPrecedence_Assign,
    kEPrecedence_SubAssign = kEPrecedence_Assign,
    kEPrecedence_MulAssign = kEPrecedence_Assign,
    kEPrecedence_DivAssign = kEPrecedence_Assign,
    kEPrecedence_ModAssign = kEPrecedence_Assign,
    kEPrecedence_LshAssign = kEPrecedence_Assign,
    kEPrecedence_RshAssign = kEPrecedence_Assign,
    kEPrecedence_OrAssign = kEPrecedence_Assign,
    kEPrecedence_AndAssign = kEPrecedence_Assign,
    kEPrecedence_XorAssign = kEPrecedence_Assign,

    kEPrecedence_General = kEPrecedence_Assign,

    kEPrecedence_Conditional, // "ternary"
    kEPrecedence_Or,
    kEPrecedence_And,
    kEPrecedence_BitOr,
    kEPrecedence_BitXor,
    kEPrecedence_BitAnd,

    kEPrecedence_Eql,
    kEPrecedence_Neq = kEPrecedence_Eql,

    kEPrecedence_Less,
    kEPrecedence_Greater = kEPrecedence_Less,
    kEPrecedence_Leq = kEPrecedence_Less,
    kEPrecedence_Geq = kEPrecedence_Less,

    kEPrecedence_Lsh,
    kEPrecedence_Rsh = kEPrecedence_Lsh,

    kEPrecedence_Add,
    kEPrecedence_Sub = kEPrecedence_Add,

    kEPrecedence_Mul,
    kEPrecedence_Div = kEPrecedence_Mul,
    kEPrecedence_Mod = kEPrecedence_Mul,

    kEPrecedence_Prefix,
    kEPrecedence_Postfix,
    kEPrecedence_Atomic = kEPrecedence_Postfix

#endif

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
        Name,
        Array,
        UnsizedArray,
    };
    Flavor flavor;
    EDeclarator* next = nullptr;

    // Used for `Flavor::Name`
    String name;
    CodePosition loc;

    // Used for `Flavor::Array`
    IntVal* elementCount;
};

struct TypeEmitArg
{
    EDeclarator* declarator;
};

struct ExprEmitArg
{
    EOpInfo outerPrec;
};

struct DeclEmitArg
{
    VarLayout*      layout;
};

struct EmitVisitor
    : TypeVisitorWithArg<EmitVisitor, TypeEmitArg>
    , ExprVisitorWithArg<EmitVisitor, ExprEmitArg>
    , DeclVisitorWithArg<EmitVisitor, DeclEmitArg>
{
    EmitContext* context;
    EmitVisitor(EmitContext* context)
        : context(context)
    {}

    // Low-level emit logic

    void emitRawTextSpan(char const* textBegin, char const* textEnd)
    {
        // TODO(tfoley): Need to make "corelib" not use `int` for pointer-sized things...
        auto len = int(textEnd - textBegin);

        context->shared->sb.Append(textBegin, len);
    }

    void emitRawText(char const* text)
    {
        emitRawTextSpan(text, text + strlen(text));
    }

    void emitTextSpan(char const* textBegin, char const* textEnd)
    {
        // If the source location has changed in a way that required update,
        // do it now!
        flushSourceLocationChange();

        // Emit the raw text
        emitRawTextSpan(textBegin, textEnd);

        // Update our logical position
        // TODO(tfoley): Need to make "corelib" not use `int` for pointer-sized things...
        auto len = int(textEnd - textBegin);
        context->shared->loc.Col += len;
    }

    void Emit(char const* textBegin, char const* textEnd)
    {
        char const* spanBegin = textBegin;

        char const* spanEnd = spanBegin;
        for(;;)
        {
            if(spanEnd == textEnd)
            {
                // We have a whole range of text waiting to be flushed
                emitTextSpan(spanBegin, spanEnd);
                return;
            }

            auto c = *spanEnd++;

            if( c == '\n' )
            {
                // At the end of a line, we need to update our tracking
                // information on code positions
                emitTextSpan(spanBegin, spanEnd);
                context->shared->loc.Line++;
                context->shared->loc.Col = 1;

                // Start a new span for emit purposes
                spanBegin = spanEnd;
            }
        }
    }

    void Emit(char const* text)
    {
        Emit(text, text + strlen(text));
    }

    void emit(String const& text)
    {
        Emit(text.begin(), text.end());
    }

    void emitName(
        String const&       inName,
        CodePosition const& loc)
    {
        String name = inName;

        advanceToSourceLocation(loc);
        emit(name);
    }

    void emitName(Token const& nameToken)
    {
        emitName(nameToken.Content, nameToken.Position);
    }

    void emitName(String const& name)
    {
        emitName(name, CodePosition());
    }

    void Emit(IntegerLiteralValue value)
    {
        char buffer[32];
        sprintf(buffer, "%lld", value);
        Emit(buffer);
    }


    void Emit(UInt value)
    {
        char buffer[32];
        sprintf(buffer, "%llu", (unsigned long long)(value));
        Emit(buffer);
    }

    void Emit(int value)
    {
        char buffer[16];
        sprintf(buffer, "%d", value);
        Emit(buffer);
    }

    void Emit(double value)
    {
        // TODO(tfoley): need to print things in a way that can round-trip
        char buffer[128];
        sprintf(buffer, "%.20ff", value);
        Emit(buffer);
    }


    // Emit a `#line` directive to the output.
    // Doesn't udpate state of source-location tracking.
    void emitLineDirective(
        CodePosition const& sourceLocation)
    {
        emitRawText("\n#line ");

        char buffer[16];
        sprintf(buffer, "%d", sourceLocation.Line);
        emitRawText(buffer);

        emitRawText(" ");

        bool shouldUseGLSLStyleLineDirective = false;

        // TODO: Eventually we should give he user a proper API
        // and command-line mechanism to control what kind of line
        // directives we output (and to turn the feature off
        // completely), but for now we always emit C-style line
        // directives *unless* the final target language is raw
        // GLSL code (all other targets will eventually pass
        // through glslang, which supports C-style line directives).
        if (context->shared->finalTarget == CodeGenTarget::GLSL)
        {
            shouldUseGLSLStyleLineDirective = true;
        }

        if(shouldUseGLSLStyleLineDirective)
        {
            auto path = sourceLocation.FileName;

            // GLSL doesn't support the traditional form of a `#line` directive without
            // an extension. Rather than depend on that extension we will output
            // a directive in the traditional GLSL fashion.
            //
            // TODO: Add some kind of configuration where we require the appropriate
            // extension and then emit a traditional line directive.

            int id = 0;
            if(!context->shared->mapGLSLSourcePathToID.TryGetValue(path, id))
            {
                id = context->shared->glslSourceIDCount++;
                context->shared->mapGLSLSourcePathToID.Add(path, id);
            }

            sprintf(buffer, "%d", id);
            emitRawText(buffer);
        }
        else
        {
            // The simple case is to emit the path for the current source
            // location. We need to be a little bit careful with this,
            // because the path might include backslash characters if we
            // are on Windows, and we want to canonicalize those over
            // to forward slashes.
            //
            // TODO: Canonicalization like this should be done centrally
            // in a module that tracks source files.

            emitRawText("\"");
            for(auto c : sourceLocation.FileName)
            {
                char charBuffer[] = { c, 0 };
                switch(c)
                {
                default:
                    emitRawText(charBuffer);
                    break;

                // The incoming file path might use `/` and/or `\\` as
                // a directory separator. We want to canonicalize this.
                //
                // TODO: should probably canonicalize paths to not use backslash somewhere else
                // in the compilation pipeline...
                case '\\':
                    emitRawText("/");
                    break;
                }
            }
            emitRawText("\"");
        }

        emitRawText("\n");
    }

    // Emit a `#line` directive to the output, and also
    // ensure that source location tracking information
    // is correct based on the directive we just output.
    void emitLineDirectiveAndUpdateSourceLocation(
        CodePosition const& sourceLocation)
    {
        emitLineDirective(sourceLocation);
    
        context->shared->loc.FileName = sourceLocation.FileName;
        context->shared->loc.Line = sourceLocation.Line;
        context->shared->loc.Col = 1;
    }

    void emitLineDirectiveIfNeeded(
        CodePosition const& sourceLocation)
    {
        // Ignore invalid source locations
        if(sourceLocation.Line <= 0)
            return;

        // If we are currently emitting code at a source location with
        // a differnet file or line, *or* if the source location is
        // somehow later on the line than what we want to emit,
        // then we need to emit a new `#line` directive.
        if(sourceLocation.FileName != context->shared->loc.FileName
            || sourceLocation.Line != context->shared->loc.Line
            || sourceLocation.Col < context->shared->loc.Col)
        {
            // Special case: if we are in the same file, and within a small number
            // of lines of the target location, then go ahead and output newlines
            // to get us caught up.
            enum { kSmallLineCount = 3 };
            auto lineDiff = sourceLocation.Line - context->shared->loc.Line;
            if(sourceLocation.FileName == context->shared->loc.FileName
                && sourceLocation.Line > context->shared->loc.Line
                && lineDiff <= kSmallLineCount)
            {
                for(int ii = 0; ii < lineDiff; ++ii )
                {
                    Emit("\n");
                }
                assert(sourceLocation.Line == context->shared->loc.Line);
            }
            else
            {
                // Go ahead and output a `#line` directive to get us caught up
                emitLineDirectiveAndUpdateSourceLocation(sourceLocation);
            }
        }

        // Now indent up to the appropriate column, so that error messages
        // that reference columns will be correct.
        //
        // TODO: This logic does not take into account whether indentation
        // came in as spaces or tabs, so there is necessarily going to be
        // coupling between how the downstream compiler counts columns,
        // and how we do.
        if(sourceLocation.Col > context->shared->loc.Col)
        {
            int delta = sourceLocation.Col - context->shared->loc.Col;
            for( int ii = 0; ii < delta; ++ii )
            {
                emitRawText(" ");
            }
            context->shared->loc.Col = sourceLocation.Col;
        }
    }

    void advanceToSourceLocation(
        CodePosition const& sourceLocation)
    {
        // Skip invalid locations
        if(sourceLocation.Line <= 0)
            return;

        context->shared->needToUpdateSourceLocation = true;
        context->shared->nextSourceLocation = sourceLocation;
    }

    void flushSourceLocationChange()
    {
        if(!context->shared->needToUpdateSourceLocation)
            return;

        // Note: the order matters here, because trying to update
        // the source location may involve outputting text that
        // advances the location, and outputting text is what
        // triggers this flush operation.
        context->shared->needToUpdateSourceLocation = false;
        emitLineDirectiveIfNeeded(context->shared->nextSourceLocation);
    }

    void emitTokenWithLocation(Token const& token)
    {
        if( token.Position.FileName.Length() != 0 )
        {
            advanceToSourceLocation(token.Position);
        }
        else
        {
            // If we don't have the original position info, we need to play
            // it safe and emit whitespace to line things up nicely

            if(token.flags & TokenFlag::AtStartOfLine)
                Emit("\n");
            // TODO(tfoley): macro expansion can currently lead to whitespace getting dropped,
            // so we will just insert it aggressively, to play it safe.
            else //  if(token.flags & TokenFlag::AfterWhitespace)
                Emit(" ");
        }

        // Emit the raw textual content of the token
        emit(token.Content);
    }


    //
    // Types
    //

    void Emit(RefPtr<IntVal> val)
    {
        if(auto constantIntVal = val.As<ConstantIntVal>())
        {
            Emit(constantIntVal->value);
        }
        else if(auto varRefVal = val.As<GenericParamIntVal>())
        {
            EmitDeclRef(varRefVal->declRef);
        }
        else
        {
            assert(!"unimplemented");
        }
    }

    void EmitDeclarator(EDeclarator* declarator)
    {
        if (!declarator) return;

        Emit(" ");

        switch (declarator->flavor)
        {
        case EDeclarator::Flavor::Name:
            emitName(declarator->name, declarator->loc);
            break;

        case EDeclarator::Flavor::Array:
            EmitDeclarator(declarator->next);
            Emit("[");
            if(auto elementCount = declarator->elementCount)
            {
                Emit(elementCount);
            }
            Emit("]");
            break;

        case EDeclarator::Flavor::UnsizedArray:
            EmitDeclarator(declarator->next);
            Emit("[]");
            break;

        default:
            assert(!"unreachable");
            break;
        }
    }

    void emitGLSLTypePrefix(
        RefPtr<ExpressionType>  type)
    {
        if(auto basicElementType = type->As<BasicExpressionType>())
        {
            switch (basicElementType->BaseType)
            {
            case BaseType::Float:
                // no prefix
                break;

            case BaseType::Int:		Emit("i");		break;
            case BaseType::UInt:	Emit("u");		break;
            case BaseType::Bool:	Emit("b");		break;
            default:
                assert(!"unreachable");
                break;
            }
        }
        else if(auto vectorType = type->As<VectorExpressionType>())
        {
            emitGLSLTypePrefix(vectorType->elementType);
        }
        else if(auto matrixType = type->As<MatrixExpressionType>())
        {
            emitGLSLTypePrefix(matrixType->getElementType());
        }
        else
        {
            assert(!"unreachable");
        }
    }

    void emitHLSLTextureType(
        RefPtr<TextureTypeBase> texType)
    {
        switch(texType->getAccess())
        {
        case SLANG_RESOURCE_ACCESS_READ:
            break;

        case SLANG_RESOURCE_ACCESS_READ_WRITE:
            Emit("RW");
            break;

        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            Emit("RasterizerOrdered");
            break;

        case SLANG_RESOURCE_ACCESS_APPEND:
            Emit("Append");
            break;

        case SLANG_RESOURCE_ACCESS_CONSUME:
            Emit("Consume");
            break;

        default:
            assert(!"unreachable");
            break;
        }

        switch (texType->GetBaseShape())
        {
        case TextureType::Shape1D:		Emit("Texture1D");		break;
        case TextureType::Shape2D:		Emit("Texture2D");		break;
        case TextureType::Shape3D:		Emit("Texture3D");		break;
        case TextureType::ShapeCube:	Emit("TextureCube");	break;
        default:
            assert(!"unreachable");
            break;
        }

        if (texType->isMultisample())
        {
            Emit("MS");
        }
        if (texType->isArray())
        {
            Emit("Array");
        }
        Emit("<");
        EmitType(texType->elementType);
        Emit(" >");
    }

    void emitGLSLTextureOrTextureSamplerType(
        RefPtr<TextureTypeBase> type,
        char const*             baseName)
    {
        emitGLSLTypePrefix(type->elementType);

        Emit(baseName);
        switch (type->GetBaseShape())
        {
        case TextureType::Shape1D:		Emit("1D");		break;
        case TextureType::Shape2D:		Emit("2D");		break;
        case TextureType::Shape3D:		Emit("3D");		break;
        case TextureType::ShapeCube:	Emit("Cube");	break;
        case TextureType::ShapeBuffer:	Emit("Buffer");	break;
        default:
            assert(!"unreachable");
            break;
        }

        if (type->isMultisample())
        {
            Emit("MS");
        }
        if (type->isArray())
        {
            Emit("Array");
        }
    }

    void emitGLSLTextureType(
        RefPtr<TextureType> texType)
    {
        emitGLSLTextureOrTextureSamplerType(texType, "texture");
    }

    void emitGLSLTextureSamplerType(
        RefPtr<TextureSamplerType>  type)
    {
        emitGLSLTextureOrTextureSamplerType(type, "sampler");
    }

    void emitGLSLImageType(
        RefPtr<GLSLImageType>   type)
    {
        emitGLSLTextureOrTextureSamplerType(type, "image");
    }

    void emitTextureType(
        RefPtr<TextureType> texType)
    {
        switch(context->shared->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLTextureType(texType);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLTextureType(texType);
            break;

        default:
            assert(!"unreachable");
            break;
        }
    }

    void emitTextureSamplerType(
        RefPtr<TextureSamplerType>  type)
    {
        switch(context->shared->target)
        {
        case CodeGenTarget::GLSL:
            emitGLSLTextureSamplerType(type);
            break;

        default:
            assert(!"unreachable");
            break;
        }
    }

    void emitImageType(
        RefPtr<GLSLImageType>   type)
    {
        switch(context->shared->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLTextureType(type);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLImageType(type);
            break;

        default:
            assert(!"unreachable");
            break;
        }
    }

    void emitTypeImpl(RefPtr<ExpressionType> type, EDeclarator* declarator)
    {
        TypeEmitArg arg;
        arg.declarator = declarator;

        TypeVisitorWithArg::dispatch(type, arg);
    }

#define UNEXPECTED(NAME) \
    void visit##NAME(NAME*, TypeEmitArg const& arg) \
    { Emit(#NAME); EmitDeclarator(arg.declarator); }

    UNEXPECTED(ErrorType);
    UNEXPECTED(OverloadGroupType);
    UNEXPECTED(FuncType);
    UNEXPECTED(TypeType);
    UNEXPECTED(GenericDeclRefType);
    UNEXPECTED(InitializerListType);

#undef UNEXPECTED

    void visitNamedExpressionType(NamedExpressionType* type, TypeEmitArg const& arg)
    {
        // Named types are valid for GLSL
        if (context->shared->target == CodeGenTarget::GLSL)
        {
            emitTypeImpl(GetType(type->declRef), arg.declarator);
            return;
        }

        EmitDeclRef(type->declRef);
        EmitDeclarator(arg.declarator);
    }

    void visitBasicExpressionType(BasicExpressionType* basicType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        switch (basicType->BaseType)
        {
        case BaseType::Void:	Emit("void");		break;
        case BaseType::Int:		Emit("int");		break;
        case BaseType::Float:	Emit("float");		break;
        case BaseType::UInt:	Emit("uint");		break;
        case BaseType::Bool:	Emit("bool");		break;
        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(declarator);
    }

    void visitVectorExpressionType(VectorExpressionType* vecType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        switch(context->shared->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                emitGLSLTypePrefix(vecType->elementType);
                Emit("vec");
                Emit(vecType->elementCount);
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            Emit("vector<");
            EmitType(vecType->elementType);
            Emit(",");
            Emit(vecType->elementCount);
            Emit(">");
            break;

        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(declarator);
    }

    void visitMatrixExpressionType(MatrixExpressionType* matType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        switch(context->shared->target)
        {
        case CodeGenTarget::GLSL:
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
            {
                emitGLSLTypePrefix(matType->getElementType());
                Emit("mat");
                Emit(matType->getRowCount());
                // TODO(tfoley): only emit the next bit
                // for non-square matrix
                Emit("x");
                Emit(matType->getColumnCount());
            }
            break;

        case CodeGenTarget::HLSL:
            // TODO(tfoley): should really emit these with sugar
            Emit("matrix<");
            EmitType(matType->getElementType());
            Emit(",");
            Emit(matType->getRowCount());
            Emit(",");
            Emit(matType->getColumnCount());
            Emit("> ");
            break;

        default:
            assert(!"unreachable");
            break;
        }

        EmitDeclarator(declarator);
    }

    void visitTextureType(TextureType* texType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        emitTextureType(texType);
        EmitDeclarator(declarator);
    }

    void visitTextureSamplerType(TextureSamplerType* textureSamplerType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        emitTextureSamplerType(textureSamplerType);
        EmitDeclarator(declarator);
    }

    void visitGLSLImageType(GLSLImageType* imageType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        emitImageType(imageType);
        EmitDeclarator(declarator);
    }

    void visitSamplerStateType(SamplerStateType* samplerStateType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        switch(context->shared->target)
        {
        case CodeGenTarget::HLSL:
        default:
            switch (samplerStateType->flavor)
            {
            case SamplerStateType::Flavor::SamplerState:			Emit("SamplerState");				break;
            case SamplerStateType::Flavor::SamplerComparisonState:	Emit("SamplerComparisonState");	break;
            default:
                assert(!"unreachable");
                break;
            }
            break;

        case CodeGenTarget::GLSL:
            Emit("sampler");
            break;
        }

        EmitDeclarator(declarator);
    }

    void visitDeclRefType(DeclRefType* declRefType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;
        EmitDeclRef(declRefType->declRef);
        EmitDeclarator(declarator);
    }

    void visitArrayExpressionType(ArrayExpressionType* arrayType, TypeEmitArg const& arg)
    {
        auto declarator = arg.declarator;

        EDeclarator arrayDeclarator;
        arrayDeclarator.next = declarator;

        if(arrayType->ArrayLength)
        {
            arrayDeclarator.flavor = EDeclarator::Flavor::Array;
            arrayDeclarator.elementCount = arrayType->ArrayLength.Ptr();
        }
        else
        {
            arrayDeclarator.flavor = EDeclarator::Flavor::UnsizedArray;
        }


        emitTypeImpl(arrayType->BaseType, &arrayDeclarator);
    }

    void EmitType(
        RefPtr<ExpressionType>  type,
        CodePosition const&     typeLoc,
        String const&           name,
        CodePosition const&     nameLoc)
    {
        advanceToSourceLocation(typeLoc);

        EDeclarator nameDeclarator;
        nameDeclarator.flavor = EDeclarator::Flavor::Name;
        nameDeclarator.name = name;
        nameDeclarator.loc = nameLoc;
        emitTypeImpl(type, &nameDeclarator);
    }


    void EmitType(RefPtr<ExpressionType> type, Token const& nameToken)
    {
        EmitType(type, CodePosition(), nameToken.Content, nameToken.Position);
    }

    void EmitType(RefPtr<ExpressionType> type)
    {
        emitTypeImpl(type, nullptr);
    }

    void emitTypeBasedOnExpr(ExpressionSyntaxNode* expr, EDeclarator* declarator)
    {
        if (auto subscriptExpr = dynamic_cast<IndexExpressionSyntaxNode*>(expr))
        {
            // Looks like an array
            emitTypeBasedOnExpr(subscriptExpr->BaseExpression, declarator);
            Emit("[");
            if (auto indexExpr = subscriptExpr->IndexExpression)
            {
                EmitExpr(indexExpr);
            }
            Emit("]");
        }
        else
        {
            // Default case
            EmitExpr(expr);
            EmitDeclarator(declarator);
        }
    }

    void EmitType(TypeExp const& typeExp, String const& name, CodePosition const& nameLoc)
    {
        if (!typeExp.type || typeExp.type->As<ErrorType>())
        {
            assert(typeExp.exp);

            EDeclarator nameDeclarator;
            nameDeclarator.flavor = EDeclarator::Flavor::Name;
            nameDeclarator.name = name;
            nameDeclarator.loc = nameLoc;

            emitTypeBasedOnExpr(typeExp.exp, &nameDeclarator);
        }
        else
        {
            EmitType(typeExp.type,
                typeExp.exp ? typeExp.exp->Position : CodePosition(),
                name, nameLoc);
        }
    }

    void EmitType(TypeExp const& typeExp, Token const& nameToken)
    {
        EmitType(typeExp, nameToken.Content, nameToken.Position);
    }

    void EmitType(TypeExp const& typeExp, String const& name)
    {
        EmitType(typeExp, name, CodePosition());
    }

    void emitTypeExp(TypeExp const& typeExp)
    {
        // TODO: we need to handle cases where the type part of things is bad...
        emitTypeImpl(typeExp.type, nullptr);
    }

    //
    // Expressions
    //

    // Determine if an expression should not be emitted when it is the base of
    // a member reference expression.
    bool IsBaseExpressionImplicit(RefPtr<ExpressionSyntaxNode> expr)
    {
        // HACK(tfoley): For now, anything with a constant-buffer type should be
        // left implicit.

        // Look through any dereferencing that took place
        RefPtr<ExpressionSyntaxNode> e = expr;
        while (auto derefExpr = e.As<DerefExpr>())
        {
            e = derefExpr->base;
        }

        if (auto declRefExpr = e.As<DeclRefExpr>())
        {
            auto decl = declRefExpr->declRef.getDecl();
            if (decl && decl->HasModifier<TransparentModifier>())
                return true;
        }

        // Is the expression referencing a constant buffer?
        if (auto cbufferType = e->Type->As<ConstantBufferType>())
        {
            return true;
        }

        return false;
    }

#if 0
    void EmitPostfixExpr(RefPtr<ExpressionSyntaxNode> expr)
    {
        EmitExprWithPrecedence(expr, kEOp_Postfix);
    }
#endif

    void EmitExpr(RefPtr<ExpressionSyntaxNode> expr)
    {
        EmitExprWithPrecedence(expr, kEOp_General);
    }

    bool MaybeEmitParens(EOpInfo& outerPrec, EOpInfo prec)
    {
        bool needParens = (prec.leftPrecedence <= outerPrec.leftPrecedence)
            || (prec.rightPrecedence <= outerPrec.rightPrecedence);

        if (needParens)
        {
            Emit("(");

            outerPrec = kEOp_None;
        }
        return needParens;
    }

    // When we are going to emit an expression in an l-value context,
    // we may need to ignore certain constructs that the type-checker
    // might have introduced, but which interfere with our ability
    // to use it effectively in the target language
    RefPtr<ExpressionSyntaxNode> prepareLValueExpr(
        RefPtr<ExpressionSyntaxNode>    expr)
    {
        for(;;)
        {
            if(auto typeCastExpr = expr.As<TypeCastExpressionSyntaxNode>())
            {
                expr = typeCastExpr->Expression;
            }
            // TODO: any other cases?
            else
            {
                return expr;
            }
        }

    }

    void emitInfixExprImpl(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* op,
        RefPtr<InvokeExpressionSyntaxNode> binExpr,
        bool isAssign)
    {
        bool needsClose = MaybeEmitParens(outerPrec, prec);

        auto left = binExpr->Arguments[0];
        if(isAssign)
        {
            left = prepareLValueExpr(left);
        }

        EmitExprWithPrecedence(left, leftSide(outerPrec, prec));
        Emit(" ");
        Emit(op);
        Emit(" ");
        EmitExprWithPrecedence(binExpr->Arguments[1], rightSide(prec, outerPrec));
        if (needsClose)
        {
            Emit(")");
        }
    }

    void EmitBinExpr(EOpInfo outerPrec, EOpInfo prec, char const* op, RefPtr<InvokeExpressionSyntaxNode> binExpr)
    {
        emitInfixExprImpl(outerPrec, prec, op, binExpr, false);
    }

    void EmitBinAssignExpr(EOpInfo outerPrec, EOpInfo prec, char const* op, RefPtr<InvokeExpressionSyntaxNode> binExpr)
    {
        emitInfixExprImpl(outerPrec, prec, op, binExpr, true);
    }

    void emitUnaryExprImpl(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* preOp,
        char const* postOp,
        RefPtr<InvokeExpressionSyntaxNode> expr,
        bool isAssign)
    {
        bool needsClose = MaybeEmitParens(outerPrec, prec);
        Emit(preOp);

        auto arg = expr->Arguments[0];
        if(isAssign)
        {
            arg = prepareLValueExpr(arg);
        }

        if (preOp)
        {
            EmitExprWithPrecedence(arg, rightSide(prec, outerPrec));
        }
        else
        {
            assert(postOp);
            EmitExprWithPrecedence(arg, leftSide(outerPrec, prec));
        }

        Emit(postOp);
        if (needsClose)
        {
            Emit(")");
        }
    }

    void EmitUnaryExpr(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* preOp,
        char const* postOp,
        RefPtr<InvokeExpressionSyntaxNode> expr)
    {
        emitUnaryExprImpl(outerPrec, prec, preOp, postOp, expr, false);
    }

    void EmitUnaryAssignExpr(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* preOp,
        char const* postOp,
        RefPtr<InvokeExpressionSyntaxNode> expr)
    {
        emitUnaryExprImpl(outerPrec, prec, preOp, postOp, expr, true);
    }

    // Determine if a target intrinsic modifer is applicable to the target
    // we are currently emitting code for.
    bool isTargetIntrinsicModifierApplicable(
        RefPtr<TargetIntrinsicModifier> modifier)
    {
        auto const& targetToken = modifier->targetToken;

        // If no target name was specified, then the modifier implicitly
        // applies to all targets.
        if(targetToken.Type == TokenType::Unknown)
            return true;

        // Otherwise, we need to check if the target name matches what
        // we expect.
        auto const& targetName = targetToken.Content;

        switch(context->shared->target)
        {
        default:
            assert(!"unexpected");
            return false;

        case CodeGenTarget::GLSL: return targetName == "glsl";
        case CodeGenTarget::HLSL: return targetName == "hlsl";
        }
    }

    // Find an intrinsic modifier appropriate to the current compilation target.
    //
    // If there are multiple such modifiers, this should return the best one.
    RefPtr<TargetIntrinsicModifier> findTargetIntrinsicModifier(
        RefPtr<ModifiableSyntaxNode>    syntax)
    {
        RefPtr<TargetIntrinsicModifier> bestModifier;
        for(auto m : syntax->GetModifiersOfType<TargetIntrinsicModifier>())
        {
            if(!isTargetIntrinsicModifierApplicable(m))
                continue;

            // For now "better"-ness is defined as: a modifier
            // with a specified target is better than one without
            // (it is more specific)
            if(!bestModifier || bestModifier->targetToken.Type == TokenType::Unknown)
            {
                bestModifier = m;
            }
        }

        return bestModifier;
    }

    // Emit a call expression that doesn't involve any special cases,
    // just an expression of the form `f(a0, a1, ...)`
    void emitSimpleCallExpr(
        RefPtr<InvokeExpressionSyntaxNode>  callExpr,
        EOpInfo                             outerPrec)
    {
        auto prec = kEOp_Postfix;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        auto funcExpr = callExpr->FunctionExpr;
        if (auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>())
        {
            auto declRef = funcDeclRefExpr->declRef;
            if (auto ctorDeclRef = declRef.As<ConstructorDecl>())
            {
                // We really want to emit a reference to the type begin constructed
                EmitType(callExpr->Type);
            }
            else
            {
                // default case: just emit the decl ref
                EmitExprWithPrecedence(funcExpr, leftSide(outerPrec, prec));
            }
        }
        else
        {
            // default case: just emit the expression
            EmitExprWithPrecedence(funcExpr, leftSide(outerPrec, prec));
        }

        Emit("(");
        UInt argCount = callExpr->Arguments.Count();
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            if (aa != 0) Emit(", ");
            EmitExpr(callExpr->Arguments[aa]);
        }
        Emit(")");

        if (needClose)
        {
            Emit(")");
        }
    }

    void emitStringLiteral(
        String const&   value)
    {
        emit("\"");
        for (auto c : value)
        {
            // TODO: This needs a more complete implementation,
            // especially if we want to support Unicode.

            char buffer[] = { c, 0 };
            switch (c)
            {
            default:
                emit(buffer);
                break;

            case '\"': emit("\\\"");
            case '\'': emit("\\\'");
            case '\\': emit("\\\\");
            case '\n': emit("\\n");
            case '\r': emit("\\r");
            case '\t': emit("\\t");
            }
        }
        emit("\"");
    }

    EOpInfo leftSide(EOpInfo const& outerPrec, EOpInfo const& prec)
    {
        EOpInfo result;
        result.leftPrecedence = outerPrec.leftPrecedence;
        result.rightPrecedence = prec.leftPrecedence;
        return result;
    }

    EOpInfo rightSide(EOpInfo const& prec, EOpInfo const& outerPrec)
    {
        EOpInfo result;
        result.leftPrecedence = prec.rightPrecedence;
        result.rightPrecedence = outerPrec.rightPrecedence;
        return result;
    }

    void EmitExprWithPrecedence(RefPtr<ExpressionSyntaxNode> expr, EOpInfo outerPrec)
    {
        ExprEmitArg arg;
        arg.outerPrec = outerPrec;

        ExprVisitorWithArg::dispatch(expr, arg);
    }

    void EmitExprWithPrecedence(RefPtr<ExpressionSyntaxNode> expr, EPrecedence leftPrec, EPrecedence rightPrec)
    {
        EOpInfo outerPrec;
        outerPrec.leftPrecedence = leftPrec;
        outerPrec.rightPrecedence = rightPrec;
    }


#define UNEXPECTED(NAME)                        \
    void visit##NAME(NAME*, ExprEmitArg const&) \
    { Emit(#NAME); }

    UNEXPECTED(GenericAppExpr);

#undef UNEXPECTED

    void visitSharedTypeExpr(SharedTypeExpr* expr, ExprEmitArg const&)
    {
        emitTypeExp(expr->base);
    }

    void visitSelectExpressionSyntaxNode(SelectExpressionSyntaxNode* selectExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Conditional;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, kEOp_Conditional);

        // TODO(tfoley): Need to ver the precedence here...

        EmitExprWithPrecedence(selectExpr->Arguments[0], leftSide(outerPrec, prec));
        Emit(" ? ");
        EmitExprWithPrecedence(selectExpr->Arguments[1], prec);
        Emit(" : ");
        EmitExprWithPrecedence(selectExpr->Arguments[2], rightSide(prec, outerPrec));

        if(needClose) Emit(")");
    }

    void visitParenExpr(ParenExpr* expr, ExprEmitArg const&)
    {
        Emit("(");
        EmitExprWithPrecedence(expr->base, kEOp_None);
        Emit(")");
    }

    void visitAssignExpr(AssignExpr* assignExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Assign;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);
        EmitExprWithPrecedence(assignExpr->left, leftSide(outerPrec, prec));
        Emit(" = ");
        EmitExprWithPrecedence(assignExpr->right, rightSide(prec, outerPrec));
        if(needClose) Emit(")");
    }

    void emitUncheckedCallExpr(
        RefPtr<InvokeExpressionSyntaxNode>  callExpr,
        String const&                       funcName,
        ExprEmitArg const&                  arg)
    {
        auto outerPrec = arg.outerPrec;
        auto funcExpr = callExpr->FunctionExpr;

        // This can occur when we are dealing with unchecked input syntax,
        // because we are in "rewriter" mode. In this case we should go
        // ahead and emit things in the form that they were written.
        if( auto infixExpr = callExpr.As<InfixExpr>() )
        {
            auto prec = kEOp_Comma;
            for (auto opInfo : kInfixOpInfos)
            {
                if (funcName == opInfo->op)
                {
                    prec = *opInfo;
                    break;
                }
            }

            EmitBinExpr(
                outerPrec,
                prec,
                funcName.Buffer(),
                callExpr);
        }
        else if( auto prefixExpr = callExpr.As<PrefixExpr>() )
        {
            EmitUnaryExpr(
                outerPrec,
                kEOp_Prefix,
                funcName.Buffer(),
                "",
                callExpr);
        }
        else if(auto postfixExpr = callExpr.As<PostfixExpr>())
        {
            EmitUnaryExpr(
                outerPrec,
                kEOp_Postfix,
                "",
                funcName.Buffer(),
                callExpr);
        }
        else
        {
            bool needClose = MaybeEmitParens(outerPrec, kEOp_Postfix);

            EmitExpr(funcExpr);

            Emit("(");
            UInt argCount = callExpr->Arguments.Count();
            for (UInt aa = 0; aa < argCount; ++aa)
            {
                if (aa != 0) Emit(", ");
                EmitExpr(callExpr->Arguments[aa]);
            }
            Emit(")");

            if (needClose) Emit(")");
        }
    }

    void requireGLSLExtension(String const& name)
    {
        if (context->shared->glslExtensionsRequired.Contains(name))
            return;

        StringBuilder& sb = context->shared->glslExtensionRequireLines;

        sb.append("#extension ");
        sb.append(name);
        sb.append(" : require\n");

        context->shared->glslExtensionsRequired.Add(name);
    }

    void visitInvokeExpressionSyntaxNode(
        RefPtr<InvokeExpressionSyntaxNode>  callExpr,
        ExprEmitArg const& arg)
    {
        auto outerPrec = arg.outerPrec;

        auto funcExpr = callExpr->FunctionExpr;
        if (auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>())
        {
            auto funcDeclRef = funcDeclRefExpr->declRef;
            auto funcDecl = funcDeclRef.getDecl();
            if(!funcDecl)
            {
                emitUncheckedCallExpr(callExpr, funcDeclRefExpr->name, arg);
                return;
            }
            else if (auto intrinsicOpModifier = funcDecl->FindModifier<IntrinsicOpModifier>())
            {
                switch (intrinsicOpModifier->op)
                {
    #define CASE(NAME, OP) case IntrinsicOp::NAME: EmitBinExpr(outerPrec, kEOp_##NAME, #OP, callExpr); return
                CASE(Mul, *);
                CASE(Div, / );
                CASE(Mod, %);
                CASE(Add, +);
                CASE(Sub, -);
                CASE(Lsh, << );
                CASE(Rsh, >> );
                CASE(Eql, == );
                CASE(Neq, != );
                CASE(Greater, >);
                CASE(Less, <);
                CASE(Geq, >= );
                CASE(Leq, <= );
                CASE(BitAnd, &);
                CASE(BitXor, ^);
                CASE(BitOr, | );
                CASE(And, &&);
                CASE(Or, || );
    #undef CASE

    #define CASE(NAME, OP) case IntrinsicOp::NAME: EmitBinAssignExpr(outerPrec, kEOp_##NAME, #OP, callExpr); return
                CASE(Assign, =);
                CASE(AddAssign, +=);
                CASE(SubAssign, -=);
                CASE(MulAssign, *=);
                CASE(DivAssign, /=);
                CASE(ModAssign, %=);
                CASE(LshAssign, <<=);
                CASE(RshAssign, >>=);
                CASE(OrAssign, |=);
                CASE(AndAssign, &=);
                CASE(XorAssign, ^=);
    #undef CASE

            case IntrinsicOp::Sequence: EmitBinExpr(outerPrec, kEOp_Comma, ",", callExpr); return;

    #define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryExpr(outerPrec, kEOp_Prefix, #OP, "", callExpr); return
                CASE(Pos, +);
                CASE(Neg, -);
                CASE(Not, !);
                CASE(BitNot, ~);
    #undef CASE

    #define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryAssignExpr(outerPrec, kEOp_Prefix, #OP, "", callExpr); return
                CASE(PreInc, ++);
                CASE(PreDec, --);
    #undef CASE

    #define CASE(NAME, OP) case IntrinsicOp::NAME: EmitUnaryAssignExpr(outerPrec, kEOp_Postfix, "", #OP, callExpr); return
                CASE(PostInc, ++);
                CASE(PostDec, --);
    #undef CASE

                case IntrinsicOp::InnerProduct_Vector_Vector:
                    // HLSL allows `mul()` to be used as a synonym for `dot()`,
                    // so we need to translate to `dot` for GLSL
                    if (context->shared->target == CodeGenTarget::GLSL)
                    {
                        Emit("dot(");
                        EmitExpr(callExpr->Arguments[0]);
                        Emit(", ");
                        EmitExpr(callExpr->Arguments[1]);
                        Emit(")");
                        return;
                    }
                    break;

                case IntrinsicOp::InnerProduct_Matrix_Matrix:
                case IntrinsicOp::InnerProduct_Matrix_Vector:
                case IntrinsicOp::InnerProduct_Vector_Matrix:
                    // HLSL exposes these with the `mul()` function, while GLSL uses ordinary
                    // `operator*`.
                    //
                    // The other critical detail here is that the way we handle matrix
                    // conventions requires that the operands to the product be swapped.
                    if (context->shared->target == CodeGenTarget::GLSL)
                    {
                        Emit("((");
                        EmitExpr(callExpr->Arguments[1]);
                        Emit(") * (");
                        EmitExpr(callExpr->Arguments[0]);
                        Emit("))");
                        return;
                    }
                    break;

                default:
                    break;
                }
            }
            else if(auto targetIntrinsicModifier = findTargetIntrinsicModifier(funcDecl))
            {
                if (context->shared->target == CodeGenTarget::GLSL)
                {
                    // Does this intrinsic requie a particular GLSL extension that wouldn't be available by default?
                    if (auto requiredGLSLExtensionModifier = funcDecl->FindModifier<RequiredGLSLExtensionModifier>())
                    {
                        // If so, we had better request the extension.
                        requireGLSLExtension(requiredGLSLExtensionModifier->extensionNameToken.Content);
                    }
                }


                if(targetIntrinsicModifier->definitionToken.Type != TokenType::Unknown)
                {
                    auto name = getStringOrIdentifierTokenValue(targetIntrinsicModifier->definitionToken);

                    if(name.IndexOf('$') == -1)
                    {
                        // Simple case: it is just an ordinary name, so we call it like a builtin.
                        //
                        // TODO: this case could probably handle things like operators, for generality?

                        emit(name);
                        Emit("(");
                        UInt argCount = callExpr->Arguments.Count();
                        for (UInt aa = 0; aa < argCount; ++aa)
                        {
                            if (aa != 0) Emit(", ");
                            EmitExpr(callExpr->Arguments[aa]);
                        }
                        Emit(")");
                        return;
                    }
                    else
                    {
                        // General case: we are going to emit some more complex text.

                        UInt argCount = callExpr->Arguments.Count();

                        Emit("(");

                        char const* cursor = name.begin();
                        char const* end = name.end();
                        while(cursor != end)
                        {
                            char c = *cursor++;
                            if( c != '$' )
                            {
                                // Not an escape sequence
                                emitRawTextSpan(&c, &c+1);
                                continue;
                            }

                            assert(cursor != end);

                            char d = *cursor++;

                            switch (d)
                            {
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9':
                                {
                                    // Simple case: emit one of the direct arguments to the call
                                    UInt argIndex = d - '0';
                                    assert((0 <= argIndex) && (argIndex < argCount));
                                    Emit("(");
                                    EmitExpr(callExpr->Arguments[argIndex]);
                                    Emit(")");
                                }
                                break;

                            case 'o':
                                // For a call using object-oriented syntax, this
                                // expands to the "base" object used for the call
                                if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpressionSyntaxNode>())
                                {
                                    Emit("(");
                                    EmitExpr(memberExpr->BaseExpression);
                                    Emit(")");
                                }
                                else
                                {
                                    assert(!"unexpected");
                                }
                                break;

                            case 'p':
                                // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                                // then this form will pair up the t and s arguments as needed for a GLSL
                                // texturing operation.
                                assert(argCount > 0);
                                if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpressionSyntaxNode>())
                                {
                                    auto base = memberExpr->BaseExpression;
                                    if (auto baseTextureType = base->Type->As<TextureType>())
                                    {
                                        emitGLSLTextureOrTextureSamplerType(baseTextureType, "sampler");
                                        Emit("(");
                                        EmitExpr(memberExpr->BaseExpression);
                                        Emit(",");
                                        EmitExpr(callExpr->Arguments[0]);
                                        Emit(")");
                                    }
                                    else
                                    {
                                        assert(!"unexpected");
                                    }

                                }
                                else
                                {
                                    assert(!"unexpected");
                                }
                                break;

                            case 'P':
                                {
                                    // Okay, we need a collosal hack to deal with the fact that GLSL `texelFetch()`
                                    // for Vulkan seems to be completely broken by design. It's signature wants
                                    // a `sampler2D` for consistency with its peers, but the actual SPIR-V operation
                                    // ignores the sampler paart of it, and just used the `texture2D` part.
                                    //
                                    // The HLSL equivalent (e.g., `Texture2D.Load()`) doesn't provide a sampler
                                    // argument, so we seemingly need to conjure one out of thin air. :(
                                    //
                                    // We are going to hack this *hard* for now.

                                    // Try to find a suitable sampler-type shader parameter in the global scope
                                    // (fingers crossed)
                                    RefPtr<VarDeclBase> samplerVar;
                                    for (auto dd : context->shared->program->Members)
                                    {
                                        if (auto varDecl = dd.As<VarDeclBase>())
                                        {
                                            if (auto samplerType = varDecl->Type.type->As<SamplerStateType>())
                                            {
                                                samplerVar = varDecl;
                                                break;
                                            }
                                        }
                                    }

                                    if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpressionSyntaxNode>())
                                    {
                                        auto base = memberExpr->BaseExpression;
                                        if (auto baseTextureType = base->Type->As<TextureType>())
                                        {
                                            emitGLSLTextureOrTextureSamplerType(baseTextureType, "sampler");
                                            Emit("(");
                                            EmitExpr(memberExpr->BaseExpression);
                                            Emit(",");
                                            if (samplerVar)
                                            {
                                                EmitDeclRef(makeDeclRef(samplerVar.Ptr()));
                                            }
                                            else
                                            {
                                                Emit("SLANG_hack_samplerForTexelFetch");
                                                context->shared->needHackSamplerForTexelFetch = true;
                                            }
                                            Emit(")");
                                        }
                                        else
                                        {
                                            assert(!"unexpected");
                                        }

                                    }
                                    else
                                    {
                                        assert(!"unexpected");
                                    }
                                }
                                break;

                            default:
                                assert(!"unexpected");
                                break;
                            }
                        }

                        Emit(")");
                    }

                    return;
                }

                // TODO: emit as approperiate for this target

                // We might be calling an intrinsic subscript operation,
                // and should desugar it accordingly
                if(auto subscriptDeclRef = funcDeclRef.As<SubscriptDecl>())
                {
                    // We expect any subscript operation to be invoked as a member,
                    // so the function expression had better be in the correct form.
                    if(auto memberExpr = funcExpr.As<MemberExpressionSyntaxNode>())
                    {

                        Emit("(");
                        EmitExpr(memberExpr->BaseExpression);
                        Emit(")[");
                        UInt argCount = callExpr->Arguments.Count();
                        for (UInt aa = 0; aa < argCount; ++aa)
                        {
                            if (aa != 0) Emit(", ");
                            EmitExpr(callExpr->Arguments[aa]);
                        }
                        Emit("]");
                        return;
                    }
                }
            }
        }
        else if (auto overloadedExpr = funcExpr.As<OverloadedExpr>())
        {
            emitUncheckedCallExpr(callExpr, overloadedExpr->lookupResult2.getName(), arg);
            return;
        }

        // Fall through to default handling...
        emitSimpleCallExpr(callExpr, outerPrec);
    }



    void visitMemberExpressionSyntaxNode(MemberExpressionSyntaxNode* memberExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Postfix;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        // TODO(tfoley): figure out a good way to reference
        // declarations that might be generic and/or might
        // not be generated as lexically nested declarations...

        // TODO(tfoley): also, probably need to special case
        // this for places where we are using a built-in...

        auto base = memberExpr->BaseExpression;
        if (IsBaseExpressionImplicit(base))
        {
            // don't emit the base expression
        }
        else
        {
            EmitExprWithPrecedence(memberExpr->BaseExpression, leftSide(outerPrec, prec));
            Emit(".");
        }

        if (!memberExpr->declRef)
        {
            // This case arises when checking didn't find anything, but we were
            // in "rewrite" mode so we blazed ahead anyway.
            emitName(memberExpr->name);
        }
        else
        {
            emit(memberExpr->declRef.GetName());
        }

        if(needClose) Emit(")");
    }

    void visitSwizzleExpr(SwizzleExpr* swizExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Postfix;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        EmitExprWithPrecedence(swizExpr->base, leftSide(outerPrec, prec));
        Emit(".");
        static const char* kComponentNames[] = { "x", "y", "z", "w" };
        int elementCount = swizExpr->elementCount;
        for (int ee = 0; ee < elementCount; ++ee)
        {
            Emit(kComponentNames[swizExpr->elementIndices[ee]]);
        }

        if(needClose) Emit(")");
    }

    void visitIndexExpressionSyntaxNode(IndexExpressionSyntaxNode* subscriptExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Postfix;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        EmitExprWithPrecedence(subscriptExpr->BaseExpression, leftSide(outerPrec, prec));
        Emit("[");
        if (auto indexExpr = subscriptExpr->IndexExpression)
        {
            EmitExpr(indexExpr);
        }
        Emit("]");

        if(needClose) Emit(")");
    }

    void visitOverloadedExpr(OverloadedExpr* expr, ExprEmitArg const&)
    {
        emitName(expr->lookupResult2.getName());
    }

    void visitVarExpressionSyntaxNode(VarExpressionSyntaxNode* varExpr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Atomic;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, kEOp_Atomic);

        // TODO: This won't be valid if we had to generate a qualified
        // reference for some reason.
        advanceToSourceLocation(varExpr->Position);

        // Because of the "rewriter" use case, it is possible that we will
        // be trying to emit an expression that hasn't been wired up to
        // any associated declaration. In that case, we will just emit
        // the variable name.
        //
        // TODO: A better long-term solution here is to have a distinct
        // case for an "unchecked" `NameExpr` that doesn't include
        // a declaration reference.

        if(varExpr->declRef)
        {
            EmitDeclRef(varExpr->declRef);
        }
        else
        {
            emit(varExpr->name);
        }

        if(needClose) Emit(")");
    }

    void visitDerefExpr(DerefExpr* derefExpr, ExprEmitArg const& arg)
    {
        // TODO(tfoley): dereference shouldn't always be implicit
        ExprVisitorWithArg::dispatch(derefExpr->base, arg);
    }

    void visitConstantExpressionSyntaxNode(ConstantExpressionSyntaxNode* litExpr, ExprEmitArg const& arg)
    {
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, kEOp_Atomic);

        char const* suffix = "";
        auto type = litExpr->Type.type;
        switch (litExpr->ConstType)
        {
        case ConstantExpressionSyntaxNode::ConstantType::Int:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(litExpr->token);
                break;
            }
            if(type->Equals(ExpressionType::GetInt()))
            {}
            else if(type->Equals(ExpressionType::GetUInt()))
            {
                suffix = "u";
            }
            else
            {
                assert(!"unimplemented");
            }
            Emit(litExpr->integerValue);
            Emit(suffix);
            break;


        case ConstantExpressionSyntaxNode::ConstantType::Float:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(litExpr->token);
                break;
            }
            if(type->Equals(ExpressionType::GetFloat()))
            {}
            else if(type->Equals(ExpressionType::getDoubleType()))
            {
                suffix = "l";
            }
            else
            {
                assert(!"unimplemented");
            }
            Emit(litExpr->floatingPointValue);
            Emit(suffix);
            break;

        case ConstantExpressionSyntaxNode::ConstantType::Bool:
            Emit(litExpr->integerValue ? "true" : "false");
            break;
        case ConstantExpressionSyntaxNode::ConstantType::String:
            emitStringLiteral(litExpr->stringValue);
            break;
        default:
            assert(!"unreachable");
            break;
        }
        if(needClose) Emit(")");
    }

    void visitHiddenImplicitCastExpr(HiddenImplicitCastExpr* castExpr, ExprEmitArg const& arg)
    {
        // This was an implicit cast inserted in code parsed in "rewriter" mode,
        // so we don't want to output it and change what the user's code looked like.
        ExprVisitorWithArg::dispatch(castExpr->Expression, arg);
    }

    void visitTypeCastExpressionSyntaxNode(TypeCastExpressionSyntaxNode* castExpr, ExprEmitArg const& arg)
    {
        bool needClose = false;
        switch(context->shared->target)
        {
        case CodeGenTarget::GLSL:
            // GLSL requires constructor syntax for all conversions
            EmitType(castExpr->Type);
            Emit("(");
            EmitExpr(castExpr->Expression);
            Emit(")");
            break;

        default:
            // HLSL (and C/C++) prefer cast syntax
            // (In fact, HLSL doesn't allow constructor syntax for some conversions it allows as a cast)
            {
                auto prec = kEOp_Prefix;
                auto outerPrec = arg.outerPrec;
                needClose = MaybeEmitParens(outerPrec, prec);

                Emit("(");
                EmitType(castExpr->Type);
                Emit(")(");
                EmitExpr(castExpr->Expression);
                Emit(")");
            }
            break;
        }
        if(needClose) Emit(")");
    }

    void visitInitializerListExpr(InitializerListExpr* expr, ExprEmitArg const&)
    {
        Emit("{ ");
        for(auto& arg : expr->args)
        {
            EmitExpr(arg);
            Emit(", ");
        }
        Emit("}");
    }

    //
    // Statements
    //

    // Emit a statement as a `{}`-enclosed block statement, but avoid adding redundant
    // curly braces if the statement is itself a block statement.
    void EmitBlockStmt(RefPtr<StatementSyntaxNode> stmt)
    {
        // TODO(tfoley): support indenting
        Emit("{\n");
        if( auto blockStmt = stmt.As<BlockStmt>() )
        {
            EmitStmt(blockStmt->body);
        }
        else
        {
            EmitStmt(stmt);
        }
        Emit("}\n");
    }

    void EmitLoopAttributes(RefPtr<StatementSyntaxNode> decl)
    {
        // Don't emit these attributes for GLSL, because it doesn't understand them
        if (context->shared->target == CodeGenTarget::GLSL)
            return;

        // TODO(tfoley): There really ought to be a semantic checking step for attributes,
        // that turns abstract syntax into a concrete hierarchy of attribute types (e.g.,
        // a specific `LoopModifier` or `UnrollModifier`).

        for(auto attr : decl->GetModifiersOfType<HLSLUncheckedAttribute>())
        {
            if(attr->nameToken.Content == "loop")
            {
                Emit("[loop]");
            }
            else if(attr->nameToken.Content == "unroll")
            {
                Emit("[unroll]");
            }
        }
    }

    void EmitUnparsedStmt(RefPtr<UnparsedStmt> stmt)
    {
        // TODO: actually emit the tokens that made up the statement...
        Emit("{\n");
        for( auto& token : stmt->tokens )
        {
            emitTokenWithLocation(token);
        }
        Emit("}\n");
    }

    void EmitStmt(RefPtr<StatementSyntaxNode> stmt)
    {
        // TODO(tfoley): this shouldn't occur, but sometimes
        // lowering will get confused by an empty function body...
        if (!stmt)
            return;

        // Try to ensure that debugging can find the right location
        advanceToSourceLocation(stmt->Position);

        if (auto blockStmt = stmt.As<BlockStmt>())
        {
            EmitBlockStmt(blockStmt);
            return;
        }
        else if (auto seqStmt = stmt.As<SeqStmt>())
        {
            for (auto ss : seqStmt->stmts)
            {
                EmitStmt(ss);
            }
            return;
        }
        else if( auto unparsedStmt = stmt.As<UnparsedStmt>() )
        {
            EmitUnparsedStmt(unparsedStmt);
            return;
        }
        else if (auto exprStmt = stmt.As<ExpressionStatementSyntaxNode>())
        {
            EmitExpr(exprStmt->Expression);
            Emit(";\n");
            return;
        }
        else if (auto returnStmt = stmt.As<ReturnStatementSyntaxNode>())
        {
            Emit("return");
            if (auto expr = returnStmt->Expression)
            {
                Emit(" ");
                EmitExpr(expr);
            }
            Emit(";\n");
            return;
        }
        else if (auto declStmt = stmt.As<VarDeclrStatementSyntaxNode>())
        {
            EmitDecl(declStmt->decl);
            return;
        }
        else if (auto ifStmt = stmt.As<IfStatementSyntaxNode>())
        {
            Emit("if(");
            EmitExpr(ifStmt->Predicate);
            Emit(")\n");
            EmitBlockStmt(ifStmt->PositiveStatement);
            if(auto elseStmt = ifStmt->NegativeStatement)
            {
                Emit("\nelse\n");
                EmitBlockStmt(elseStmt);
            }
            return;
        }
        else if (auto forStmt = stmt.As<ForStatementSyntaxNode>())
        {
            // We are going to always take a `for` loop like:
            //
            //    for(A; B; C) { D }
            //
            // and emit it as:
            //
            //    { A; for(; B; C) { D } }
            //
            // This ensures that we are robust against any kind
            // of statement appearing in `A`, including things
            // that might occur due to lowering steps.
            //

            // The one wrinkle is that HLSL implements the
            // bad approach to scoping a `for` loop variable,
            // so we need to avoid those outer `{...}` when
            // we are emitting code that was written in HLSL.
            //
            bool brokenScoping = false;
            if (forStmt.As<UnscopedForStmt>())
            {
                brokenScoping = true;
            }

            auto initStmt = forStmt->InitialStatement;
            if(initStmt)
            {
                if(!brokenScoping)
                    Emit("{\n");
                EmitStmt(initStmt);
            }

            EmitLoopAttributes(forStmt);

            Emit("for(;");
            if (auto testExp = forStmt->PredicateExpression)
            {
                EmitExpr(testExp);
            }
            Emit(";");
            if (auto incrExpr = forStmt->SideEffectExpression)
            {
                EmitExpr(incrExpr);
            }
            Emit(")\n");
            EmitBlockStmt(forStmt->Statement);

            if (initStmt)
            {
                if(!brokenScoping)
                    Emit("}\n");
            }

            return;
        }
        else if (auto whileStmt = stmt.As<WhileStatementSyntaxNode>())
        {
            EmitLoopAttributes(whileStmt);

            Emit("while(");
            EmitExpr(whileStmt->Predicate);
            Emit(")\n");
            EmitBlockStmt(whileStmt->Statement);
            return;
        }
        else if (auto doWhileStmt = stmt.As<DoWhileStatementSyntaxNode>())
        {
            EmitLoopAttributes(doWhileStmt);

            Emit("do(");
            EmitBlockStmt(doWhileStmt->Statement);
            Emit(" while(");
            EmitExpr(doWhileStmt->Predicate);
            Emit(")\n");
            return;
        }
        else if (auto discardStmt = stmt.As<DiscardStatementSyntaxNode>())
        {
            Emit("discard;\n");
            return;
        }
        else if (auto emptyStmt = stmt.As<EmptyStatementSyntaxNode>())
        {
            return;
        }
        else if (auto switchStmt = stmt.As<SwitchStmt>())
        {
            Emit("switch(");
            EmitExpr(switchStmt->condition);
            Emit(")\n");
            EmitBlockStmt(switchStmt->body);
            return;
        }
        else if (auto caseStmt = stmt.As<CaseStmt>())
        {
            Emit("case ");
            EmitExpr(caseStmt->expr);
            Emit(":\n");
            return;
        }
        else if (auto defaultStmt = stmt.As<DefaultStmt>())
        {
            Emit("default:{}\n");
            return;
        }
        else if (auto breakStmt = stmt.As<BreakStatementSyntaxNode>())
        {
            Emit("break;\n");
            return;
        }
        else if (auto continueStmt = stmt.As<ContinueStatementSyntaxNode>())
        {
            Emit("continue;\n");
            return;
        }

        throw "unimplemented";

    }

    //
    // Declaration References
    //

    // Declaration References

    void EmitVal(RefPtr<Val> val)
    {
        if (auto type = val.As<ExpressionType>())
        {
            EmitType(type);
        }
        else if (auto intVal = val.As<IntVal>())
        {
            Emit(intVal);
        }
        else
        {
            // Note(tfoley): ignore unhandled cases for semantics for now...
    //		assert(!"unimplemented");
        }
    }

    void EmitDeclRef(DeclRef<Decl> declRef)
    {
        // TODO: need to qualify a declaration name based on parent scopes/declarations

        // Emit the name for the declaration itself
        emitName(declRef.GetName());

        // If the declaration is nested directly in a generic, then
        // we need to output the generic arguments here
        auto parentDeclRef = declRef.GetParent();
        if (auto genericDeclRef = parentDeclRef.As<GenericDecl>())
        {
            // Only do this for declarations of appropriate flavors
            if(auto funcDeclRef = declRef.As<FunctionDeclBase>())
            {
                // Don't emit generic arguments for functions, because HLSL doesn't allow them
                return;
            }

            Substitutions* subst = declRef.substitutions.Ptr();
            Emit("<");
            UInt argCount = subst->args.Count();
            for (UInt aa = 0; aa < argCount; ++aa)
            {
                if (aa != 0) Emit(",");
                EmitVal(subst->args[aa]);
            }
            Emit(" >");
        }

    }


    //
    // Declarations
    //

    void emitDeclImpl(
        Decl*           decl,
        VarLayout*      layout)
    {
        // Don't emit code for declarations that came from the stdlib.
        //
        // TODO(tfoley): We probably need to relax this eventually,
        // since different targets might have different sets of builtins.
        if (decl->HasModifier<FromStdLibModifier>())
            return;

        // Try to ensure that debugging can find the right location
        advanceToSourceLocation(decl->Position);

        DeclEmitArg arg;
        arg.layout = layout;

        DeclVisitorWithArg::dispatch(decl, arg);
    }

#define IGNORED(NAME) \
    void visit##NAME(NAME*, DeclEmitArg const&) {}

    // Only used by stdlib
    IGNORED(ModifierDecl)

    // Don't emit generic decls directly; we will only
    // ever emit particular instantiations of them.
    IGNORED(GenericDecl)
    IGNORED(GenericTypeConstraintDecl)
    IGNORED(GenericValueParamDecl)
    IGNORED(GenericTypeParamDecl)

    // Not epected to appear (probably dead code)
    IGNORED(ClassSyntaxNode)

    // Not semantically meaningful for emit, or expected
    // to be lowered out of existence before we get here
    IGNORED(InheritanceDecl)
    IGNORED(ExtensionDecl)
    IGNORED(ScopeDecl)

    // Catch-all cases where we handle the types that matter,
    // while others will be lowered out of exitence
    IGNORED(CallableDecl)
    IGNORED(AggTypeDeclBase)

    // Should not appear nested inside other decls
    IGNORED(ProgramSyntaxNode)

#undef IGNORED

    void visitDeclGroup(DeclGroup* declGroup, DeclEmitArg const&)
    {
        for (auto decl : declGroup->decls)
        {
            EmitDecl(decl);
        }
    }

    void visitTypeDefDecl(TypeDefDecl* decl, DeclEmitArg const&)
    {
        // Note(tfoley): any `typedef`s should already have been filtered
        // out if we are generating GLSL.
        assert(context->shared->target != CodeGenTarget::GLSL);

        Emit("typedef ");
        EmitType(decl->Type, decl->Name.Content);
        Emit(";\n");
    }

    void visitImportDecl(ImportDecl* decl, DeclEmitArg const&)
    {
        // When in "rewriter" mode, we need to emit the code of the imported
        // module in-place at the `import` site.

        auto moduleDecl = decl->importedModuleDecl.Ptr();

        // We might import the same module along two different paths,
        // so we need to be careful to only emit each module once
        // per output.
        if(!context->shared->modulesAlreadyEmitted.Contains(moduleDecl))
        {
            // Add the module to our set before emitting it, just
            // in case a circular reference would lead us to
            // infinite recursion (but that shouldn't be allowed
            // in the first place).
            context->shared->modulesAlreadyEmitted.Add(moduleDecl);

            // TODO: do we need to modify the code generation environment at
            // all when doing this recursive emit?

            EmitDeclsInContainerUsingLayout(moduleDecl, context->shared->globalStructLayout);
        }
    }

    void visitEmptyDecl(EmptyDecl* decl, DeclEmitArg const&)
    {
        // GLSL uses empty declarations to carry semantically relevant modifiers,
        // so we can't just skip empty declarations in general

        EmitModifiers(decl);
        Emit(";\n");
    }

    bool shouldSkipModifierForDecl(
        Modifier*   modifier,
        Decl*       decl)
    {
        switch(context->shared->target)
        {
        default:
            break;

        case CodeGenTarget::GLSL:
            {
                // Don't emit interpolation mode modifiers on `struct` fields
                // (only allowed on global or block `in`/`out`)
                if (auto interpolationMod = dynamic_cast<InterpolationModeModifier*>(modifier))
                {
                    if (auto fieldDecl = dynamic_cast<StructField*>(decl))
                    {
                        return true;
                    }
                }

            }
            break;
        }


        return false;
    }

    // Emit any modifiers that should go in front of a declaration
    void EmitModifiers(RefPtr<Decl> decl)
    {
        // Emit any GLSL `layout` modifiers first
        bool anyLayout = false;
        for( auto mod : decl->GetModifiersOfType<GLSLUnparsedLayoutModifier>())
        {
            if(!anyLayout)
            {
                Emit("layout(");
                anyLayout = true;
            }
            else
            {
                Emit(", ");
            }

            emit(mod->nameToken.Content);
            if(mod->valToken.Type != TokenType::Unknown)
            {
                Emit(" = ");
                emit(mod->valToken.Content);
            }
        }
        if(anyLayout)
        {
            Emit(")\n");
        }

        for (auto mod = decl->modifiers.first; mod; mod = mod->next)
        {
            if (shouldSkipModifierForDecl(mod, decl))
                continue;

            advanceToSourceLocation(mod->Position);

            if (0) {}

            #define CASE(TYPE, KEYWORD) \
                else if(auto mod_##TYPE = mod.As<TYPE>()) Emit(#KEYWORD " ")

            #define CASE2(TYPE, HLSL_NAME, GLSL_NAME) \
                else if(auto mod_##TYPE = mod.As<TYPE>()) Emit((context->shared->target == CodeGenTarget::GLSL) ? GLSL_NAME : HLSL_NAME)

            CASE(RowMajorLayoutModifier, row_major);
            CASE(ColumnMajorLayoutModifier, column_major);
            CASE(HLSLNoInterpolationModifier, nointerpolation);
            CASE(HLSLPreciseModifier, precise);
            CASE(HLSLEffectSharedModifier, shared);
            CASE(HLSLGroupSharedModifier, groupshared);
            CASE(HLSLUniformModifier, uniform);
            CASE(HLSLVolatileModifier, volatile);

            CASE(InOutModifier, inout);
            CASE(InModifier, in);
            CASE(OutModifier, out);

            CASE(HLSLPointModifier, point);
            CASE(HLSLLineModifier, line);
            CASE(HLSLTriangleModifier, triangle);
            CASE(HLSLLineAdjModifier, lineadj);
            CASE(HLSLTriangleAdjModifier, triangleadj);

            CASE(HLSLLinearModifier, linear);
            CASE(HLSLSampleModifier, sample);
            CASE(HLSLCentroidModifier, centroid);

            CASE(ConstModifier, const);

            #undef CASE
            #undef CASE2

            else if (auto staticModifier = mod.As<HLSLStaticModifier>())
            {
                // GLSL does not support the `static` keyword.
                // HLSL uses it both to mark global variables as being "thread-local"
                // (rather than shader inputs), and also seems to support function-`static`
                // variables.
                // The latter case needs to be dealt with in lowering anyway, so that
                // we only need to deal with globals here, and GLSL variables
                // don't need a `static` modifier anyway.

                switch(context->shared->target)
                {
                default:
                    Emit("static ");
                    break;

                case CodeGenTarget::GLSL:
                    break;
                }
            }

            // TODO: eventually we should be checked these modifiers, but for
            // now we can emit them unchecked, I guess
            else if (auto uncheckedAttr = mod.As<HLSLAttribute>())
            {
                Emit("[");
                emit(uncheckedAttr->nameToken.Content);
                auto& args = uncheckedAttr->args;
                auto argCount = args.Count();
                if (argCount != 0)
                {
                    Emit("(");
                    for (UInt aa = 0; aa < argCount; ++aa)
                    {
                        if (aa != 0) Emit(", ");
                        EmitExpr(args[aa]);
                    }
                    Emit(")");
                }
                Emit("]");
            }

            else if(auto simpleModifier = mod.As<SimpleModifier>())
            {
                emit(simpleModifier->nameToken.Content);
                Emit(" ");
            }

            else
            {
                // skip any extra modifiers
            }
        }
    }


    typedef unsigned int ESemanticMask;
    enum
    {
        kESemanticMask_None = 0,

        kESemanticMask_NoPackOffset = 1 << 0,

        kESemanticMask_Default = kESemanticMask_NoPackOffset,
    };

    void EmitSemantic(RefPtr<HLSLSemantic> semantic, ESemanticMask /*mask*/)
    {
        if (auto simple = semantic.As<HLSLSimpleSemantic>())
        {
            Emit(": ");
            emit(simple->name.Content);
        }
        else if(auto registerSemantic = semantic.As<HLSLRegisterSemantic>())
        {
            // Don't print out semantic from the user, since we are going to print the same thing our own way...
    #if 0
            Emit(": register(");
            Emit(registerSemantic->registerName.Content);
            if(registerSemantic->componentMask.Type != TokenType::Unknown)
            {
                Emit(".");
                Emit(registerSemantic->componentMask.Content);
            }
            Emit(")");
    #endif
        }
        else if(auto packOffsetSemantic = semantic.As<HLSLPackOffsetSemantic>())
        {
            // Don't print out semantic from the user, since we are going to print the same thing our own way...
    #if 0
            if(mask & kESemanticMask_NoPackOffset)
                return;

            Emit(": packoffset(");
            Emit(packOffsetSemantic->registerName.Content);
            if(packOffsetSemantic->componentMask.Type != TokenType::Unknown)
            {
                Emit(".");
                Emit(packOffsetSemantic->componentMask.Content);
            }
            Emit(")");
    #endif
        }
        else
        {
            assert(!"unimplemented");
        }
    }


    void EmitSemantics(RefPtr<Decl> decl, ESemanticMask mask = kESemanticMask_Default )
    {
        // Don't emit semantics if we aren't translating down to HLSL
        switch (context->shared->target)
        {
        case CodeGenTarget::HLSL:
            break;

        default:
            return;
        }

        for (auto mod = decl->modifiers.first; mod; mod = mod->next)
        {
            auto semantic = mod.As<HLSLSemantic>();
            if (!semantic)
                continue;

            EmitSemantic(semantic, mask);
        }
    }

    void EmitDeclsInContainer(RefPtr<ContainerDecl> container)
    {
        for (auto member : container->Members)
        {
            EmitDecl(member);
        }
    }

    void EmitDeclsInContainerUsingLayout(
        RefPtr<ContainerDecl>       container,
        RefPtr<StructTypeLayout>    containerLayout)
    {
        for (auto member : container->Members)
        {
            RefPtr<VarLayout> memberLayout;
            if( containerLayout->mapVarToLayout.TryGetValue(member.Ptr(), memberLayout) )
            {
                EmitDeclUsingLayout(member, memberLayout);
            }
            else
            {
                // No layout for this decl
                EmitDecl(member);
            }
        }
    }

    void visitStructSyntaxNode(RefPtr<StructSyntaxNode> decl, DeclEmitArg const&)
    {
        // Don't emit a declaration that was only generated implicitly, for
        // the purposes of semantic checking.
        if(decl->HasModifier<ImplicitParameterBlockElementTypeModifier>())
            return;

        Emit("struct ");
        emitName(decl->Name);
        Emit("\n{\n");

        // TODO(tfoley): Need to hoist members functions, etc. out to global scope
        EmitDeclsInContainer(decl);

        Emit("};\n");
    }

    // Shared emit logic for variable declarations (used for parameters, locals, globals, fields)
    void EmitVarDeclCommon(DeclRef<VarDeclBase> declRef)
    {
        EmitModifiers(declRef.getDecl());

        auto type = GetType(declRef);
        if (!type || type->As<ErrorType>())
        {
            EmitType(declRef.getDecl()->Type, declRef.getDecl()->getNameToken());
        }
        else
        {
            EmitType(GetType(declRef), declRef.getDecl()->getNameToken());
        }

        EmitSemantics(declRef.getDecl());

        // TODO(tfoley): technically have to apply substitution here too...
        if (auto initExpr = declRef.getDecl()->Expr)
        {
            if (declRef.As<ParameterSyntaxNode>()
                && context->shared->target == CodeGenTarget::GLSL)
            {
                // Don't emit default parameter values when lowering to GLSL
            }
            else
            {
                Emit(" = ");
                EmitExpr(initExpr);
            }
        }
    }

    // Shared emit logic for variable declarations (used for parameters, locals, globals, fields)
    void EmitVarDeclCommon(RefPtr<VarDeclBase> decl)
    {
        EmitVarDeclCommon(DeclRef<Decl>(decl.Ptr(), nullptr).As<VarDeclBase>());
    }

    // Emit a single `regsiter` semantic, as appropriate for a given resource-type-specific layout info
    void emitHLSLRegisterSemantic(
        VarLayout::ResourceInfo const&  info,

        // Keyword to use in the uniform case (`register` for globals, `packoffset` inside a `cbuffer`)
        char const* uniformSemanticSpelling = "register")
    {
        if( info.kind == LayoutResourceKind::Uniform )
        {
            size_t offset = info.index;

            // The HLSL `c` register space is logically grouped in 16-byte registers,
            // while we try to traffic in byte offsets. That means we need to pick
            // a register number, based on the starting offset in 16-byte register
            // units, and then a "component" within that register, based on 4-byte
            // offsets from there. We cannot support more fine-grained offsets than that.

            Emit(": ");
            Emit(uniformSemanticSpelling);
            Emit("(c");

            // Size of a logical `c` register in bytes
            auto registerSize = 16;

            // Size of each component of a logical `c` register, in bytes
            auto componentSize = 4;

            size_t startRegister = offset / registerSize;
            Emit(int(startRegister));

            size_t byteOffsetInRegister = offset % registerSize;

            // If this field doesn't start on an even register boundary,
            // then we need to emit additional information to pick the
            // right component to start from
            if (byteOffsetInRegister != 0)
            {
                // The value had better occupy a whole number of components.
                assert(byteOffsetInRegister % componentSize == 0);

                size_t startComponent = byteOffsetInRegister / componentSize;

                static const char* kComponentNames[] = {"x", "y", "z", "w"};
                Emit(".");
                Emit(kComponentNames[startComponent]);
            }
            Emit(")");
        }
        else
        {
            Emit(": register(");
            switch( info.kind )
            {
            case LayoutResourceKind::ConstantBuffer:
                Emit("b");
                break;
            case LayoutResourceKind::ShaderResource:
                Emit("t");
                break;
            case LayoutResourceKind::UnorderedAccess:
                Emit("u");
                break;
            case LayoutResourceKind::SamplerState:
                Emit("s");
                break;
            default:
                assert(!"unexpected");
                break;
            }
            Emit(info.index);
            if(info.space)
            {
                Emit(", space");
                Emit(info.space);
            }
            Emit(")");
        }
    }

    // Emit all the `register` semantics that are appropriate for a particular variable layout
    void emitHLSLRegisterSemantics(
        RefPtr<VarLayout>   layout,
        char const*         uniformSemanticSpelling = "register")
    {
        if (!layout) return;

        switch( context->shared->target )
        {
        default:
            return;

        case CodeGenTarget::HLSL:
            break;
        }

        for( auto rr : layout->resourceInfos )
        {
            emitHLSLRegisterSemantic(rr, uniformSemanticSpelling);
        }
    }

    static RefPtr<VarLayout> maybeFetchLayout(
        RefPtr<Decl>        decl,
        RefPtr<VarLayout>   layout)
    {
        // If we have already found layout info, don't go searching
        if (layout) return layout;

        // Otherwise, we need to look and see if computed layout
        // information has been attached to the declaration.
        auto modifier = decl->FindModifier<ComputedLayoutModifier>();
        if (!modifier) return nullptr;

        auto computedLayout = modifier->layout;
        assert(computedLayout);

        auto varLayout = computedLayout.As<VarLayout>();
        return varLayout;
    }

    void emitHLSLParameterBlockDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterBlockType>      parameterBlockType,
        RefPtr<VarLayout>               layout)
    {
        // The data type that describes where stuff in the constant buffer should go
        RefPtr<ExpressionType> dataType = parameterBlockType->elementType;

        // We expect/require the data type to be a user-defined `struct` type
        auto declRefType = dataType->As<DeclRefType>();
        assert(declRefType);

        // We expect to always have layout information
        layout = maybeFetchLayout(varDecl, layout);
        assert(layout);

        // We expect the layout to be for a structured type...
        RefPtr<ParameterBlockTypeLayout> bufferLayout = layout->typeLayout.As<ParameterBlockTypeLayout>();
        assert(bufferLayout);

        RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementTypeLayout.As<StructTypeLayout>();
        assert(structTypeLayout);

        if( auto constantBufferType = parameterBlockType->As<ConstantBufferType>() )
        {
            Emit("cbuffer ");
        }
        else if( auto textureBufferType = parameterBlockType->As<TextureBufferType>() )
        {
            Emit("tbuffer ");
        }

        if( auto reflectionNameModifier = varDecl->FindModifier<ParameterBlockReflectionName>() )
        {
            Emit(" ");
            emitName(reflectionNameModifier->nameToken);
        }

        EmitSemantics(varDecl, kESemanticMask_None);

        auto info = layout->FindResourceInfo(LayoutResourceKind::ConstantBuffer);
        assert(info);
        emitHLSLRegisterSemantic(*info);

        Emit("\n{\n");
        if (auto structRef = declRefType->declRef.As<StructSyntaxNode>())
        {
            int fieldCounter = 0;

            for (auto field : getMembersOfType<StructField>(structRef))
            {
                int fieldIndex = fieldCounter++;

                EmitVarDeclCommon(field);

                RefPtr<VarLayout> fieldLayout = structTypeLayout->fields[fieldIndex];
                assert(fieldLayout->varDecl.GetName() == field.GetName());

                // Emit explicit layout annotations for every field
                for( auto rr : fieldLayout->resourceInfos )
                {
                    auto kind = rr.kind;

                    auto offsetResource = rr;

                    if(kind != LayoutResourceKind::Uniform)
                    {
                        // Add the base index from the cbuffer into the index of the field
                        //
                        // TODO(tfoley): consider maybe not doing this, since it actually
                        // complicates logic around constant buffers...

                        // If the member of the cbuffer uses a resource, it had better
                        // appear as part of the cubffer layout as well.
                        auto cbufferResource = layout->FindResourceInfo(kind);
                        assert(cbufferResource);

                        offsetResource.index += cbufferResource->index;
                        offsetResource.space += cbufferResource->space;
                    }

                    emitHLSLRegisterSemantic(offsetResource, "packoffset");
                }

                Emit(";\n");
            }
        }
        Emit("}\n");
    }

    void emitGLSLLayoutQualifier(
        VarLayout::ResourceInfo const&  info)
    {
        switch(info.kind)
        {
        case LayoutResourceKind::Uniform:
            Emit("layout(offset = ");
            Emit(info.index);
            Emit(")\n");
            break;

        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            Emit("layout(location = ");
            Emit(info.index);
            Emit(")\n");
            break;

        case LayoutResourceKind::SpecializationConstant:
            Emit("layout(constant_id = ");
            Emit(info.index);
            Emit(")\n");
            break;

        case LayoutResourceKind::ConstantBuffer:
        case LayoutResourceKind::ShaderResource:
        case LayoutResourceKind::UnorderedAccess:
        case LayoutResourceKind::SamplerState:
        case LayoutResourceKind::DescriptorTableSlot:
            Emit("layout(binding = ");
            Emit(info.index);
            if(info.space)
            {
                Emit(", set = ");
                Emit(info.space);
            }
            Emit(")\n");
            break;
        }
    }

    void emitGLSLLayoutQualifiers(
        RefPtr<VarLayout>               layout,
        LayoutResourceKind              filter = LayoutResourceKind::None)
    {
        if(!layout) return;

        switch( context->shared->target )
        {
        default:
            return;

        case CodeGenTarget::GLSL:
            break;
        }

        for( auto info : layout->resourceInfos )
        {
            // Skip info that doesn't match our filter
            if (filter != LayoutResourceKind::None
                && filter != info.kind)
            {
                continue;
            }

            emitGLSLLayoutQualifier(info);
        }
    }

    void emitGLSLParameterBlockDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterBlockType>      parameterBlockType,
        RefPtr<VarLayout>               layout)
    {
        // The data type that describes where stuff in the constant buffer should go
        RefPtr<ExpressionType> dataType = parameterBlockType->elementType;

        // We expect/require the data type to be a user-defined `struct` type
        auto declRefType = dataType->As<DeclRefType>();
        assert(declRefType);

        // We expect to always have layout information
        assert(layout);

        // We expect the layout to be for a structured type...
        RefPtr<ParameterBlockTypeLayout> bufferLayout = layout->typeLayout.As<ParameterBlockTypeLayout>();
        assert(bufferLayout);

        RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementTypeLayout.As<StructTypeLayout>();
        assert(structTypeLayout);

        emitGLSLLayoutQualifiers(layout);

        EmitModifiers(varDecl);

        // Emit an apprpriate declaration keyword based on the kind of block
        if (parameterBlockType->As<ConstantBufferType>())
        {
            Emit("uniform");
        }
        else if (parameterBlockType->As<GLSLInputParameterBlockType>())
        {
            Emit("in");
        }
        else if (parameterBlockType->As<GLSLOutputParameterBlockType>())
        {
            Emit("out");
        }
        else if (parameterBlockType->As<GLSLShaderStorageBufferType>())
        {
            Emit("buffer");
        }
        else
        {
            assert(!"unexpected");
            Emit("uniform");
        }

        if( auto reflectionNameModifier = varDecl->FindModifier<ParameterBlockReflectionName>() )
        {
            Emit(" ");
            emitName(reflectionNameModifier->nameToken);
        }

        Emit("\n{\n");
        if (auto structRef = declRefType->declRef.As<StructSyntaxNode>())
        {
            for (auto field : getMembersOfType<StructField>(structRef))
            {
                RefPtr<VarLayout> fieldLayout;
                structTypeLayout->mapVarToLayout.TryGetValue(field.getDecl(), fieldLayout);
    //            assert(fieldLayout);

                // TODO(tfoley): We may want to emit *some* of these,
                // some of the time...
    //            emitGLSLLayoutQualifiers(fieldLayout);

                EmitVarDeclCommon(field);

                Emit(";\n");
            }
        }
        Emit("}");

        if( varDecl->Name.Type != TokenType::Unknown )
        {
            Emit(" ");
            emitName(varDecl->Name);
        }

        Emit(";\n");
    }

    void emitParameterBlockDecl(
        RefPtr<VarDeclBase>			varDecl,
        RefPtr<ParameterBlockType>  parameterBlockType,
        RefPtr<VarLayout>           layout)
    {
        switch(context->shared->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLParameterBlockDecl(varDecl, parameterBlockType, layout);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLParameterBlockDecl(varDecl, parameterBlockType, layout);
            break;

        default:
            assert(!"unexpected");
            break;
        }
    }

    void visitVarDeclBase(RefPtr<VarDeclBase> decl, DeclEmitArg const& arg)
    {
        // Skip fields that have been tuple-ified and don't contribute
        // any fields of "ordinary" type.
        if (auto tupleFieldMod = decl->FindModifier<TupleFieldModifier>())
        {
            if (!tupleFieldMod->hasAnyNonTupleFields)
                return;
        }

        RefPtr<VarLayout> layout = arg.layout;
        layout = maybeFetchLayout(decl, layout);

        // As a special case, a variable using a parameter block type
        // will be translated into a declaration using the more primitive
        // language syntax.
        //
        // TODO(tfoley): Be sure to unwrap arrays here, in the GLSL case.
        //
        // TODO(tfoley): Detect cases where we need to fall back to
        // ordinary variable declaration syntax in HLSL.
        //
        // TODO(tfoley): there might be a better way to detect this, e.g.,
        // with an attribute that gets attached to the variable declaration.
        if (auto parameterBlockType = decl->Type->As<ParameterBlockType>())
        {
            emitParameterBlockDecl(decl, parameterBlockType, layout);
            return;
        }


        if (context->shared->target == CodeGenTarget::GLSL)
        {
            if (decl->HasModifier<InModifier>())
            {
                emitGLSLLayoutQualifiers(layout, LayoutResourceKind::VertexInput);
            }
            else if (decl->HasModifier<OutModifier>())
            {
                emitGLSLLayoutQualifiers(layout, LayoutResourceKind::FragmentOutput);
            }
            else
            {
                emitGLSLLayoutQualifiers(layout);
            }

            // If we have a uniform that wasn't tagged `uniform` in GLSL, then fix that here
            if (layout
                && !decl->HasModifier<HLSLUniformModifier>())
            {
                if (layout->FindResourceInfo(LayoutResourceKind::Uniform)
                    || layout->FindResourceInfo(LayoutResourceKind::DescriptorTableSlot))
                {
                    Emit("uniform ");
                }
            }
        }

        EmitVarDeclCommon(decl);

        emitHLSLRegisterSemantics(layout);

        Emit(";\n");
    }

    void EmitParamDecl(RefPtr<ParameterSyntaxNode> decl)
    {
        EmitVarDeclCommon(decl);
    }

    void visitFunctionSyntaxNode(RefPtr<FunctionSyntaxNode> decl, DeclEmitArg const&)
    {
        EmitModifiers(decl);

        // TODO: if a function returns an array type, or something similar that
        // isn't allowed by declarator syntax and/or language rules, we could
        // hypothetically wrap things in a `typedef` and work around it.

        EmitType(decl->ReturnType, decl->Name);

        Emit("(");
        bool first = true;
        for (auto paramDecl : decl->getMembersOfType<ParameterSyntaxNode>())
        {
            if (!first) Emit(", ");
            EmitParamDecl(paramDecl);
            first = false;
        }
        Emit(")");

        EmitSemantics(decl);

        if (auto bodyStmt = decl->Body)
        {
            EmitBlockStmt(bodyStmt);
        }
        else
        {
            Emit(";\n");
        }
    }

    void emitGLSLPreprocessorDirectives(
        RefPtr<ProgramSyntaxNode>   program)
    {
        switch(context->shared->target)
        {
        // Don't emit this stuff unless we are targetting GLSL
        default:
            return;

        case CodeGenTarget::GLSL:
            break;
        }

        if( auto versionDirective = program->FindModifier<GLSLVersionDirective>() )
        {
            // TODO(tfoley): Emit an appropriate `#line` directive...

            Emit("#version ");
            emit(versionDirective->versionNumberToken.Content);
            if(versionDirective->glslProfileToken.Type != TokenType::Unknown)
            {
                Emit(" ");
                emit(versionDirective->glslProfileToken.Content);
            }
            Emit("\n");
        }
        else
        {
            // No explicit version was given (probably because we are cross-compiling).
            //
            // We need to pick an appropriate version, ideally based on the features
            // that the shader ends up using.
            //
            // For now we just fall back to a reasonably recent version.

            Emit("#version 420\n");
        }

        // TODO: when cross-compiling we may need to output additional `#extension` directives
        // based on the features that we have used.

        for( auto extensionDirective :  program->GetModifiersOfType<GLSLExtensionDirective>() )
        {
            // TODO(tfoley): Emit an appropriate `#line` directive...

            Emit("#extension ");
            emit(extensionDirective->extensionNameToken.Content);
            Emit(" : ");
            emit(extensionDirective->dispositionToken.Content);
            Emit("\n");
        }

        // TODO: handle other cases...
    }

    void EmitDecl(RefPtr<Decl> decl)
    {
        emitDeclImpl(decl, nullptr);
    }

    void EmitDeclUsingLayout(RefPtr<Decl> decl, RefPtr<VarLayout> layout)
    {
        emitDeclImpl(decl, layout);
    }

    void EmitDecl(RefPtr<DeclBase> declBase)
    {
        if( auto decl = declBase.As<Decl>() )
        {
            EmitDecl(decl);
        }
        else if(auto declGroup = declBase.As<DeclGroup>())
        {
            for(auto d : declGroup->decls)
                EmitDecl(d);
        }
        else
        {
            throw "unimplemented";
        }
    }
};

String emitEntryPoint(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    auto translationUnit = entryPoint->getTranslationUnit();

    SharedEmitContext sharedContext;
    sharedContext.target = target;
    sharedContext.finalTarget = entryPoint->compileRequest->Target;

    sharedContext.programLayout = programLayout;

    // Layout information for the global scope is either an ordinary
    // `struct` in the common case, or a constant buffer in the case
    // where there were global-scope uniforms.
    auto globalScopeLayout = programLayout->globalScopeLayout;
    StructTypeLayout* globalStructLayout = nullptr;
    if( auto gs = globalScopeLayout.As<StructTypeLayout>() )
    {
        globalStructLayout = gs.Ptr();
    }
    else if(auto globalConstantBufferLayout = globalScopeLayout.As<ParameterBlockTypeLayout>())
    {
        // TODO: the `cbuffer` case really needs to be emitted very
        // carefully, but that is beyond the scope of what a simple rewriter
        // can easily do (without semantic analysis, etc.).
        //
        // The crux of the problem is that we need to collect all the
        // global-scope uniforms (but not declarations that don't involve
        // uniform storage...) and put them in a single `cbuffer` declaration,
        // so that we can give it an explicit location. The fields in that
        // declaration might use various type declarations, so we'd really
        // need to emit all the type declarations first, and that involves
        // some large scale reorderings.
        //
        // For now we will punt and just emit the declarations normally,
        // and hope that the global-scope block (`$Globals`) gets auto-assigned
        // the same location that we manually asigned it.

        auto elementTypeLayout = globalConstantBufferLayout->elementTypeLayout;
        auto elementTypeStructLayout = elementTypeLayout.As<StructTypeLayout>();

        // We expect all constant buffers to contain `struct` types for now
        assert(elementTypeStructLayout);

        globalStructLayout = elementTypeStructLayout.Ptr();
    }
    else
    {
        assert(!"unexpected");
    }
    sharedContext.globalStructLayout = globalStructLayout;

    EmitContext context;
    context.shared = &sharedContext;

    EmitVisitor visitor(&context);

    auto translationUnitSyntax = translationUnit->SyntaxNode.Ptr();


    // There may be global-scope modifiers that we should emit now
    visitor.emitGLSLPreprocessorDirectives(translationUnitSyntax);

    String prefix = sharedContext.sb.ProduceString();
    sharedContext.sb.Clear();

    switch(target)
    {
    case CodeGenTarget::GLSL:
        {
            // TODO(tfoley): Need a plan for how to enable/disable these as needed...
//            Emit(context, "#extension GL_GOOGLE_cpp_style_line_directive : require\n");
        }
        break;

    default:
        break;
    }

    auto lowered = lowerEntryPoint(entryPoint, programLayout, target);

    sharedContext.program = lowered.program;

    visitor.EmitDeclsInContainer(lowered.program.Ptr());

#if 0
    if( isRewrite )
    {
        // In rewrite mode, we will just emit the text of the translation unit as given,
        // and not pay attention to the specific entry point that was requested.
        //
        // It is a user error to request GLSL output and have an entry point name
        // other than `main`.
        EmitDeclsInContainerUsingLayout(&context, translationUnitSyntax, globalStructLayout);
    }
    else
    {
        // We are being asked to emit a single entry point in "full" mode.
        emitEntryPoint(&context, entryPoint);
    }
#endif

    String code = sharedContext.sb.ProduceString();

    StringBuilder finalResultBuilder;
    finalResultBuilder << prefix;

    finalResultBuilder << sharedContext.glslExtensionRequireLines.ProduceString();

    if (sharedContext.needHackSamplerForTexelFetch)
    {
        finalResultBuilder << "layout(set = 0, binding = 0) uniform sampler SLANG_hack_samplerForTexelFetch;\n";
    }

    finalResultBuilder << code;

    String finalResult = finalResultBuilder.ProduceString();

    return finalResult;
}

} // namespace Slang
