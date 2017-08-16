// emit.cpp
#include "emit.h"

#include "lower.h"
#include "lower-to-ir.h"
#include "name.h"
#include "syntax.h"
#include "type-layout.h"
#include "visitor.h"

#include <assert.h>

#ifdef _WIN32
#include <d3dcompiler.h>
#pragma warning(disable:4996)
#endif

namespace Slang {

struct ExtensionUsageTracker
{
    // Record the GLSL extnsions we have already emitted a `#extension` for
    HashSet<String> glslExtensionsRequired;
    StringBuilder glslExtensionRequireLines;
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



// Shared state for an entire emit session
struct SharedEmitContext
{
    // The entry point we are being asked to compile
    EntryPointRequest* entryPoint;

    // The layout for the entry point
    EntryPointLayout*   entryPointLayout;

    // The target language we want to generate code for
    CodeGenTarget target;

    // The final code generation target
    //
    // For example, `target` might be `GLSL`, while `finalTarget` might be `SPIRV`
    CodeGenTarget finalTarget;

    // The string of code we've built so far
    StringBuilder sb;

    // Current source position for tracking purposes...
    HumaneSourceLoc loc;
    HumaneSourceLoc nextSourceLocation;
    bool needToUpdateSourceLocation;

    // For GLSL output, we can't emit traidtional `#line` directives
    // with a file path in them, so we maintain a map that associates
    // each path with a unique integer, and then we output those
    // instead.
    Dictionary<String, int> mapGLSLSourcePathToID;
    int glslSourceIDCount = 0;

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

    bool                needHackSamplerForTexelFetch = false;

    ExtensionUsageTracker extensionUsageTracker;
};

struct EmitContext
{
    // The shared context that is in effect
    SharedEmitContext* shared;
};

//

void requireGLSLVersion(
    EntryPointRequest*  entryPoint,
    ProfileVersion      version)
{
    auto profile = entryPoint->profile;
    if (profile.getFamily() == ProfileFamily::GLSL)
    {
        // Check if this profile is newer
        if ((UInt)version > (UInt)profile.GetVersion())
        {
            profile.setVersion(version);
            entryPoint->profile = profile;
        }
    }
    else
    {
        // Non-GLSL target? Set it to a GLSL one.
        profile.setVersion(version);
        entryPoint->profile = profile;
    }
}

//

static String getStringOrIdentifierTokenValue(
    Token const& token)
{
    switch(token.type)
    {
    default:
        SLANG_UNEXPECTED("needed an identifier or string literal");
        break;

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

    Session* getSession()
    {
        return context->shared->entryPoint->compileRequest->mSession;
    }

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
        context->shared->loc.column += len;
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
                context->shared->loc.line++;
                context->shared->loc.column = 1;

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

    void emit(Name* name)
    {
        emit(getText(name));
    }

    void emit(NameLoc const& nameAndLoc)
    {
        advanceToSourceLocation(nameAndLoc.loc);
        emit(getText(nameAndLoc.name));
    }

    void emitName(
        Name*               name,
        SourceLoc const&    loc)
    {
        advanceToSourceLocation(loc);
        emit(name);
    }

    void emitName(NameLoc const& nameAndLoc)
    {
        emitName(nameAndLoc.name, nameAndLoc.loc);
    }

    void emitName(Name* name)
    {
        emitName(name, SourceLoc());
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
        HumaneSourceLoc const& sourceLocation)
    {
        emitRawText("\n#line ");

        char buffer[16];
        sprintf(buffer, "%llu", (unsigned long long)sourceLocation.line);
        emitRawText(buffer);

        emitRawText(" ");

        bool shouldUseGLSLStyleLineDirective = false;

        auto mode = context->shared->entryPoint->compileRequest->lineDirectiveMode;
        switch (mode)
        {
        case LineDirectiveMode::None:
            SLANG_UNEXPECTED("should not trying to emit '#line' directive");
            return;

        case LineDirectiveMode::Default:
        default:
            // To try to make the default behavior reasonable, we will
            // always use C-style line directives (to give the user
            // good source locations on error messages from downstream
            // compilers) *unless* they requested raw GLSL as the
            // output (in which case we want to maximize compatibility
            // with downstream tools).
            if (context->shared->finalTarget == CodeGenTarget::GLSL)
            {
                shouldUseGLSLStyleLineDirective = true;
            }
            break;

        case LineDirectiveMode::Standard:
            break;

        case LineDirectiveMode::GLSL:
            shouldUseGLSLStyleLineDirective = true;
            break;
        }

        if(shouldUseGLSLStyleLineDirective)
        {
            auto path = sourceLocation.getPath();

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
            for(auto c : sourceLocation.getPath())
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
        HumaneSourceLoc const& sourceLocation)
    {
        emitLineDirective(sourceLocation);

        HumaneSourceLoc newLoc = sourceLocation;
        newLoc.column = 1;

        context->shared->loc = newLoc;
    }

    void emitLineDirectiveIfNeeded(
        HumaneSourceLoc const& sourceLocation)
    {
        // Don't do any of this work if the user has requested that we
        // not emit line directives.
        auto mode = context->shared->entryPoint->compileRequest->lineDirectiveMode;
        if (mode == LineDirectiveMode::None)
            return;

        // Ignore invalid source locations
        if(sourceLocation.line <= 0)
            return;

        // If we are currently emitting code at a source location with
        // a differnet file or line, *or* if the source location is
        // somehow later on the line than what we want to emit,
        // then we need to emit a new `#line` directive.
        if(sourceLocation.path != context->shared->loc.path
            || sourceLocation.line != context->shared->loc.line
            || sourceLocation.column < context->shared->loc.column)
        {
            // Special case: if we are in the same file, and within a small number
            // of lines of the target location, then go ahead and output newlines
            // to get us caught up.
            enum { kSmallLineCount = 3 };
            auto lineDiff = sourceLocation.line - context->shared->loc.line;
            if(sourceLocation.path == context->shared->loc.path
                && sourceLocation.line > context->shared->loc.line
                && lineDiff <= kSmallLineCount)
            {
                for(int ii = 0; ii < lineDiff; ++ii )
                {
                    Emit("\n");
                }
                SLANG_RELEASE_ASSERT(sourceLocation.line == context->shared->loc.line);
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
        if(sourceLocation.column > context->shared->loc.column)
        {
            Slang::Int delta = sourceLocation.column - context->shared->loc.column;
            for( int ii = 0; ii < delta; ++ii )
            {
                emitRawText(" ");
            }
            context->shared->loc.column = sourceLocation.column;
        }
    }

    void advanceToSourceLocation(
        HumaneSourceLoc const& sourceLocation)
    {
        // Skip invalid locations
        if(sourceLocation.line <= 0)
            return;

        context->shared->needToUpdateSourceLocation = true;
        context->shared->nextSourceLocation = sourceLocation;
    }

    SourceManager* getSourceManager()
    {
        return context->shared->entryPoint->compileRequest->getSourceManager();
    }

    void advanceToSourceLocation(
        SourceLoc const& sourceLocation)
    {
        advanceToSourceLocation(getSourceManager()->getHumaneLoc(sourceLocation));
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
        auto mode = context->shared->entryPoint->compileRequest->lineDirectiveMode;
        if (mode == LineDirectiveMode::None)
            return;

        if ((mode == LineDirectiveMode::None)
            || !token.loc.isValid())
        {
            // If we don't have the original position info, or we are in the
            // mode where the user didn't want line directives, we need to play
            // it safe and emit whitespace to line things up nicely

            if(token.flags & TokenFlag::AtStartOfLine)
                Emit("\n");
            // TODO(tfoley): macro expansion can currently lead to whitespace getting dropped,
            // so we will just insert it aggressively, to play it safe.
            else //  if(token.flags & TokenFlag::AfterWhitespace)
                Emit(" ");
        }
        else
        {
            // If location information is available, and we are emitting
            // such information, then just advance our tracking location
            // to the right place.
            advanceToSourceLocation(token.loc);
        }

        // Emit the raw textual content of the token
        emit(token.Content);
    }

    DiagnosticSink* getSink()
    {
        return &context->shared->entryPoint->compileRequest->mSink;
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unknown type of integer constant value");
        }
    }

    void EmitDeclarator(EDeclarator* declarator)
    {
        if (!declarator) return;

        Emit(" ");

        switch (declarator->flavor)
        {
        case EDeclarator::Flavor::name:
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unknown declarator flavor");
            break;
        }
    }

    void emitGLSLTypePrefix(
        RefPtr<Type>  type)
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
            case BaseType::Double:	Emit("d");		break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled GLSL type prefix");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled GLSL type prefix");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            break;
        }

        switch (texType->GetBaseShape())
        {
        case TextureType::Shape1D:		Emit("Texture1D");		break;
        case TextureType::Shape2D:		Emit("Texture2D");		break;
        case TextureType::Shape3D:		Emit("Texture3D");		break;
        case TextureType::ShapeCube:	Emit("TextureCube");	break;
        case TextureType::ShapeBuffer:  Emit("Buffer");         break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see combined texture-sampler types");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "this target should see GLSL image types");
            break;
        }
    }

    void emitTypeImpl(RefPtr<Type> type, EDeclarator* declarator)
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
        case BaseType::Double:	Emit("double");		break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled scalar type");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
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
            case SamplerStateType::Flavor::SamplerState:			Emit("SamplerState");			break;
            case SamplerStateType::Flavor::SamplerComparisonState:	Emit("SamplerComparisonState");	break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                break;
            }
            break;

        case CodeGenTarget::GLSL:
            switch (samplerStateType->flavor)
            {
            case SamplerStateType::Flavor::SamplerState:			Emit("sampler");		break;
            case SamplerStateType::Flavor::SamplerComparisonState:	Emit("samplerShadow");	break;
            default:
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled sampler state flavor");
                break;
            }
            break;
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
        RefPtr<Type>        type,
        SourceLoc const&    typeLoc,
        Name*               name,
        SourceLoc const&    nameLoc)
    {
        advanceToSourceLocation(typeLoc);

        EDeclarator nameDeclarator;
        nameDeclarator.flavor = EDeclarator::Flavor::name;
        nameDeclarator.name = name;
        nameDeclarator.loc = nameLoc;
        emitTypeImpl(type, &nameDeclarator);
    }

    void EmitType(RefPtr<Type> type, Name* name)
    {
        EmitType(type, SourceLoc(), name, SourceLoc());
    }

    void EmitType(RefPtr<Type> type)
    {
        emitTypeImpl(type, nullptr);
    }

    void emitTypeBasedOnExpr(Expr* expr, EDeclarator* declarator)
    {
        if (auto subscriptExpr = dynamic_cast<IndexExpr*>(expr))
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

    void EmitType(TypeExp const& typeExp, Name* name, SourceLoc const& nameLoc)
    {
        if (!typeExp.type || typeExp.type->As<ErrorType>())
        {
            if (!typeExp.exp)
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unresolved type expression should have expression part");
            }

            EDeclarator nameDeclarator;
            nameDeclarator.flavor = EDeclarator::Flavor::name;
            nameDeclarator.name = name;
            nameDeclarator.loc = nameLoc;

            emitTypeBasedOnExpr(typeExp.exp, &nameDeclarator);
        }
        else
        {
            EmitType(typeExp.type,
                typeExp.exp ? typeExp.exp->loc : SourceLoc(),
                name, nameLoc);
        }
    }

    void EmitType(TypeExp const& typeExp, NameLoc const& nameAndLoc)
    {
        EmitType(typeExp, nameAndLoc.name, nameAndLoc.loc);
    }

    void EmitType(TypeExp const& typeExp, Name* name)
    {
        EmitType(typeExp, name, SourceLoc());
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
    bool IsBaseExpressionImplicit(RefPtr<Expr> expr)
    {
        // HACK(tfoley): For now, anything with a constant-buffer type should be
        // left implicit.

        // Look through any dereferencing that took place
        RefPtr<Expr> e = expr;
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
        if (auto cbufferType = e->type->As<ConstantBufferType>())
        {
            return true;
        }

        return false;
    }

#if 0
    void EmitPostfixExpr(RefPtr<Expr> expr)
    {
        EmitExprWithPrecedence(expr, kEOp_Postfix);
    }
#endif

    void EmitExpr(RefPtr<Expr> expr)
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
    RefPtr<Expr> prepareLValueExpr(
        RefPtr<Expr>    expr)
    {
        for(;;)
        {
            if(auto typeCastExpr = expr.As<TypeCastExpr>())
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
        RefPtr<InvokeExpr> binExpr,
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

    void EmitBinExpr(EOpInfo outerPrec, EOpInfo prec, char const* op, RefPtr<InvokeExpr> binExpr)
    {
        emitInfixExprImpl(outerPrec, prec, op, binExpr, false);
    }

    void EmitBinAssignExpr(EOpInfo outerPrec, EOpInfo prec, char const* op, RefPtr<InvokeExpr> binExpr)
    {
        emitInfixExprImpl(outerPrec, prec, op, binExpr, true);
    }

    void emitUnaryExprImpl(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* preOp,
        char const* postOp,
        RefPtr<InvokeExpr> expr,
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
            SLANG_ASSERT(postOp);
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
        RefPtr<InvokeExpr> expr)
    {
        emitUnaryExprImpl(outerPrec, prec, preOp, postOp, expr, false);
    }

    void EmitUnaryAssignExpr(
        EOpInfo outerPrec,
        EOpInfo prec,
        char const* preOp,
        char const* postOp,
        RefPtr<InvokeExpr> expr)
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
        if(targetToken.type == TokenType::Unknown)
            return true;

        // Otherwise, we need to check if the target name matches what
        // we expect.
        auto const& targetName = targetToken.Content;

        switch(context->shared->target)
        {
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled code generation target");
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
            if(!bestModifier || bestModifier->targetToken.type == TokenType::Unknown)
            {
                bestModifier = m;
            }
        }

        return bestModifier;
    }

    // Emit a call expression that doesn't involve any special cases,
    // just an expression of the form `f(a0, a1, ...)`
    void emitSimpleCallExpr(
        RefPtr<InvokeExpr>  callExpr,
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
                EmitType(callExpr->type);
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

    void EmitExprWithPrecedence(RefPtr<Expr> expr, EOpInfo outerPrec)
    {
        ExprEmitArg arg;
        arg.outerPrec = outerPrec;

        ExprVisitorWithArg::dispatch(expr, arg);
    }

    void EmitExprWithPrecedence(RefPtr<Expr> expr, EPrecedence leftPrec, EPrecedence rightPrec)
    {
        EOpInfo outerPrec;
        outerPrec.leftPrecedence = leftPrec;
        outerPrec.rightPrecedence = rightPrec;
    }

    void visitGenericAppExpr(GenericAppExpr* expr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Postfix;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        EmitExprWithPrecedence(expr->FunctionExpr, leftSide(outerPrec, prec));
        Emit("<");
        bool first = true;
        for(auto aa : expr->Arguments)
        {
            if(!first) Emit(", ");
            EmitExpr(aa);
            first = false;
        }
        Emit(" >");

        if(needClose)
        {
            Emit(")");
        }
    }

    void visitSharedTypeExpr(SharedTypeExpr* expr, ExprEmitArg const&)
    {
        emitTypeExp(expr->base);
    }

    void visitSelectExpr(SelectExpr* selectExpr, ExprEmitArg const& arg)
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
        RefPtr<InvokeExpr>  callExpr,
        Name*               funcName,
        ExprEmitArg const&  arg)
    {
        auto outerPrec = arg.outerPrec;
        auto funcExpr = callExpr->FunctionExpr;

        auto funcNameText = getText(funcName);

        // This can occur when we are dealing with unchecked input syntax,
        // because we are in "rewriter" mode. In this case we should go
        // ahead and emit things in the form that they were written.
        if( auto infixExpr = callExpr.As<InfixExpr>() )
        {
            auto prec = kEOp_Comma;
            for (auto opInfo : kInfixOpInfos)
            {
                if (funcNameText == opInfo->op)
                {
                    prec = *opInfo;
                    break;
                }
            }

            EmitBinExpr(
                outerPrec,
                prec,
                funcNameText.Buffer(),
                callExpr);
        }
        else if( auto prefixExpr = callExpr.As<PrefixExpr>() )
        {
            EmitUnaryExpr(
                outerPrec,
                kEOp_Prefix,
                funcNameText.Buffer(),
                "",
                callExpr);
        }
        else if(auto postfixExpr = callExpr.As<PostfixExpr>())
        {
            EmitUnaryExpr(
                outerPrec,
                kEOp_Postfix,
                "",
                funcNameText.Buffer(),
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
        Slang::requireGLSLExtension(&context->shared->extensionUsageTracker, name);
    }

    void requireGLSLVersion(ProfileVersion version)
    {
        if (context->shared->target != CodeGenTarget::GLSL)
            return;

        auto entryPoint = context->shared->entryPoint;
        Slang::requireGLSLVersion(entryPoint, version);
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

    void visitInvokeExpr(
        RefPtr<InvokeExpr>  callExpr,
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

                    // Does this intrinsic requie a particular GLSL extension that wouldn't be available by default?
                    if (auto requiredGLSLVersionModifier = funcDecl->FindModifier<RequiredGLSLVersionModifier>())
                    {
                        // If so, we had better request the extension.
                        requireGLSLVersion((int) getIntegerLiteralValue(requiredGLSLVersionModifier->versionNumberToken));
                    }
                }


                if(targetIntrinsicModifier->definitionToken.type != TokenType::Unknown)
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

                            SLANG_RELEASE_ASSERT(cursor != end);

                            char d = *cursor++;

                            switch (d)
                            {
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9':
                                {
                                    // Simple case: emit one of the direct arguments to the call
                                    UInt argIndex = d - '0';
                                    SLANG_RELEASE_ASSERT((0 <= argIndex) && (argIndex < argCount));
                                    Emit("(");
                                    EmitExpr(callExpr->Arguments[argIndex]);
                                    Emit(")");
                                }
                                break;

                            case 'o':
                                // For a call using object-oriented syntax, this
                                // expands to the "base" object used for the call
                                if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpr>())
                                {
                                    Emit("(");
                                    EmitExpr(memberExpr->BaseExpression);
                                    Emit(")");
                                }
                                else
                                {
                                    SLANG_UNEXPECTED("bad format in intrinsic definition");
                                }
                                break;

                            case 'p':
                                // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                                // then this form will pair up the t and s arguments as needed for a GLSL
                                // texturing operation.
                                SLANG_RELEASE_ASSERT(argCount > 0);
                                if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpr>())
                                {
                                    auto base = memberExpr->BaseExpression;
                                    if (auto baseTextureType = base->type->As<TextureType>())
                                    {
                                        emitGLSLTextureOrTextureSamplerType(baseTextureType, "sampler");

                                        if (auto samplerType = callExpr->Arguments[0]->type.type->As<SamplerStateType>())
                                        {
                                            if (samplerType->flavor == SamplerStateType::Flavor::SamplerComparisonState)
                                            {
                                                Emit("Shadow");
                                            }
                                        }

                                        Emit("(");
                                        EmitExpr(memberExpr->BaseExpression);
                                        Emit(",");
                                        EmitExpr(callExpr->Arguments[0]);
                                        Emit(")");
                                    }
                                    else
                                    {
                                        SLANG_UNEXPECTED("bad format in intrinsic definition");
                                    }

                                }
                                else
                                {
                                    SLANG_UNEXPECTED("bad format in intrinsic definition");
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
                                            if (auto samplerType = varDecl->type.type->As<SamplerStateType>())
                                            {
                                                samplerVar = varDecl;
                                                break;
                                            }
                                        }
                                    }

                                    if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpr>())
                                    {
                                        auto base = memberExpr->BaseExpression;
                                        if (auto baseTextureType = base->type->As<TextureType>())
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
                                            SLANG_UNEXPECTED("bad format in intrinsic definition");
                                        }

                                    }
                                    else
                                    {
                                        SLANG_UNEXPECTED("bad format in intrinsic definition");
                                    }
                                }
                                break;

                            case 'z':
                                // If we are calling a D3D texturing operation in the form t.Foo(s, ...),
                                // where `t` is a `Texture*<T>`, then this is the step where we try to
                                // properly swizzle the output of the equivalent GLSL call into the right
                                // shape.
                                SLANG_RELEASE_ASSERT(argCount > 0);
                                if (auto memberExpr = callExpr->FunctionExpr.As<MemberExpr>())
                                {
                                    auto base = memberExpr->BaseExpression;
                                    if (auto baseTextureType = base->type->As<TextureType>())
                                    {
                                        auto elementType = baseTextureType->elementType;
                                        if (auto basicType = elementType->As<BasicExpressionType>())
                                        {
                                            // A scalar result is expected
                                            Emit(".x");
                                        }
                                        else if (auto vectorType = elementType->As<VectorExpressionType>())
                                        {
                                            // A vector result is expected
                                            auto elementCount = GetIntVal(vectorType->elementCount);

                                            if (elementCount < 4)
                                            {
                                                char const* swiz[] = { "", ".x", ".xy", ".xyz", "" };
                                                Emit(swiz[elementCount]);
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
                                else
                                {
                                    SLANG_UNEXPECTED("bad format in intrinsic definition");
                                }
                                break;


                            default:
                                SLANG_UNEXPECTED("bad format in intrinsic definition");
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
                    if(auto memberExpr = funcExpr.As<MemberExpr>())
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

    void visitAggTypeCtorExpr(AggTypeCtorExpr* expr, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Postfix;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        emitTypeExp(expr->base);
        Emit("(");
        bool first = true;
        for (auto aa : expr->Arguments)
        {
            if (!first) Emit(", ");
            EmitExpr(aa);
            first = false;
        }
        Emit(")");

        if(needClose) Emit(")");
    }

    void visitMemberExpr(MemberExpr* memberExpr, ExprEmitArg const& arg)
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

    void visitIndexExpr(IndexExpr* subscriptExpr, ExprEmitArg const& arg)
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

    void setSampleRateFlag()
    {
        context->shared->entryPointLayout->flags |= EntryPointLayout::Flag::usesAnySampleRateInput;
    }

    void doSampleRateInputCheck(VarDeclBase* decl)
    {
        if (decl->HasModifier<HLSLSampleModifier>())
        {
            setSampleRateFlag();
        }
    }

    void doSampleRateInputCheck(Name* name)
    {
        auto text = getText(name);
        if (text == "gl_SampleID")
        {
            setSampleRateFlag();
        }
    }

    void visitVarExpr(VarExpr* varExpr, ExprEmitArg const& arg)
    {
        doSampleRateInputCheck(varExpr->name);

        auto prec = kEOp_Atomic;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, kEOp_Atomic);

        // TODO: This won't be valid if we had to generate a qualified
        // reference for some reason.
        advanceToSourceLocation(varExpr->loc);

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

    void visitConstantExpr(ConstantExpr* litExpr, ExprEmitArg const& arg)
    {
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, kEOp_Atomic);

        char const* suffix = "";
        auto type = litExpr->type.type;
        switch (litExpr->ConstType)
        {
        case ConstantExpr::ConstantType::Int:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(litExpr->token);
                break;
            }
            if(type->Equals(getSession()->getIntType()))
            {}
            else if(type->Equals(getSession()->getUIntType()))
            {
                suffix = "u";
            }
            else
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), litExpr, "unhandled type for integer literal");
            }
            Emit(litExpr->integerValue);
            Emit(suffix);
            break;


        case ConstantExpr::ConstantType::Float:
            if(!type)
            {
                // Special case for "rewrite" mode
                emitTokenWithLocation(litExpr->token);
                break;
            }
            if(type->Equals(getSession()->getFloatType()))
            {}
            else if(type->Equals(getSession()->getDoubleType()))
            {
                suffix = "l";
            }
            else
            {
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), litExpr, "unhandled type for floating-point literal");
            }
            Emit(litExpr->floatingPointValue);
            Emit(suffix);
            break;

        case ConstantExpr::ConstantType::Bool:
            Emit(litExpr->integerValue ? "true" : "false");
            break;
        case ConstantExpr::ConstantType::String:
            emitStringLiteral(litExpr->stringValue);
            break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), litExpr, "unhandled kind of literal expression");
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

    void visitTypeCastExpr(TypeCastExpr* castExpr, ExprEmitArg const& arg)
    {
        bool needClose = false;
        switch(context->shared->target)
        {
        case CodeGenTarget::GLSL:
            // GLSL requires constructor syntax for all conversions
            EmitType(castExpr->type);
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
                EmitType(castExpr->type);
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
    void EmitBlockStmt(RefPtr<Stmt> stmt)
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

    void EmitLoopAttributes(RefPtr<Stmt> decl)
    {
        // Don't emit these attributes for GLSL, because it doesn't understand them
        if (context->shared->target == CodeGenTarget::GLSL)
            return;

        // TODO(tfoley): There really ought to be a semantic checking step for attributes,
        // that turns abstract syntax into a concrete hierarchy of attribute types (e.g.,
        // a specific `LoopModifier` or `UnrollModifier`).

        for(auto attr : decl->GetModifiersOfType<HLSLUncheckedAttribute>())
        {
            if(getText(attr->getName()) == "loop")
            {
                Emit("[loop]");
            }
            else if(getText(attr->getName()) == "unroll")
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
            if (token.type == TokenType::Identifier)
            {
                doSampleRateInputCheck(token.getName());
            }

            emitTokenWithLocation(token);
        }
        Emit("}\n");
    }

    void EmitStmt(RefPtr<Stmt> stmt)
    {
        // TODO(tfoley): this shouldn't occur, but sometimes
        // lowering will get confused by an empty function body...
        if (!stmt)
            return;

        // Try to ensure that debugging can find the right location
        advanceToSourceLocation(stmt->loc);

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
        else if (auto exprStmt = stmt.As<ExpressionStmt>())
        {
            EmitExpr(exprStmt->Expression);
            Emit(";\n");
            return;
        }
        else if (auto returnStmt = stmt.As<ReturnStmt>())
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
        else if (auto declStmt = stmt.As<DeclStmt>())
        {
            EmitDecl(declStmt->decl);
            return;
        }
        else if (auto ifStmt = stmt.As<IfStmt>())
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
        else if (auto forStmt = stmt.As<ForStmt>())
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
        else if (auto whileStmt = stmt.As<WhileStmt>())
        {
            EmitLoopAttributes(whileStmt);

            Emit("while(");
            EmitExpr(whileStmt->Predicate);
            Emit(")\n");
            EmitBlockStmt(whileStmt->Statement);
            return;
        }
        else if (auto doWhileStmt = stmt.As<DoWhileStmt>())
        {
            EmitLoopAttributes(doWhileStmt);

            Emit("do(");
            EmitBlockStmt(doWhileStmt->Statement);
            Emit(" while(");
            EmitExpr(doWhileStmt->Predicate);
            Emit(")\n");
            return;
        }
        else if (auto discardStmt = stmt.As<DiscardStmt>())
        {
            Emit("discard;\n");
            return;
        }
        else if (auto emptyStmt = stmt.As<EmptyStmt>())
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
        else if (auto breakStmt = stmt.As<BreakStmt>())
        {
            Emit("break;\n");
            return;
        }
        else if (auto continueStmt = stmt.As<ContinueStmt>())
        {
            Emit("continue;\n");
            return;
        }

        SLANG_UNEXPECTED("unhandled statement kind");
    }

    //
    // Declaration References
    //

    // Declaration References

    void EmitVal(RefPtr<Val> val)
    {
        if (auto type = val.As<Type>())
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
        advanceToSourceLocation(decl->loc);

        DeclEmitArg arg;
        arg.layout = layout;

        DeclVisitorWithArg::dispatch(decl, arg);
    }

#define IGNORED(NAME) \
    void visit##NAME(NAME*, DeclEmitArg const&) {}

    // Only used by stdlib
    IGNORED(SyntaxDecl)

    // Don't emit generic decls directly; we will only
    // ever emit particular instantiations of them.
    IGNORED(GenericDecl)
    IGNORED(GenericTypeConstraintDecl)
    IGNORED(GenericValueParamDecl)
    IGNORED(GenericTypeParamDecl)

    // Not epected to appear (probably dead code)
    IGNORED(ClassDecl)

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
    IGNORED(ModuleDecl)

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
        SLANG_RELEASE_ASSERT(context->shared->target != CodeGenTarget::GLSL);

        Emit("typedef ");
        EmitType(decl->type, decl->getNameAndLoc());
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

            emit(mod->getNameAndLoc());
            if(mod->valToken.type != TokenType::Unknown)
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

            advanceToSourceLocation(mod->loc);

            if (0) {}

            #define CASE(TYPE, KEYWORD) \
                else if(auto mod_##TYPE = mod.As<TYPE>()) Emit(#KEYWORD " ")

            #define CASE2(TYPE, HLSL_NAME, GLSL_NAME) \
                else if(auto mod_##TYPE = mod.As<TYPE>()) Emit((context->shared->target == CodeGenTarget::GLSL) ? (#GLSL_NAME " ") : (#HLSL_NAME " "))

            #define CASE2_RAW(TYPE, HLSL_NAME, GLSL_NAME) \
                else if(auto mod_##TYPE = mod.As<TYPE>()) Emit((context->shared->target == CodeGenTarget::GLSL) ? (GLSL_NAME) : (HLSL_NAME))

            CASE(RowMajorLayoutModifier, row_major);
            CASE(ColumnMajorLayoutModifier, column_major);

            CASE2(HLSLNoInterpolationModifier, nointerpolation, flat);
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

            CASE2_RAW(HLSLLinearModifier, "linear ", "");
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
                emit(uncheckedAttr->getNameAndLoc());
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
                emit(simpleModifier->getNameAndLoc());
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
            if(registerSemantic->componentMask.type != TokenType::Unknown)
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
            if(packOffsetSemantic->componentMask.type != TokenType::Unknown)
            {
                Emit(".");
                Emit(packOffsetSemantic->componentMask.Content);
            }
            Emit(")");
    #endif
        }
        else
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), semantic->loc, "unhandled kind of semantic");
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

    void visitStructDecl(RefPtr<StructDecl> decl, DeclEmitArg const&)
    {
        // Don't emit a declaration that was only generated implicitly, for
        // the purposes of semantic checking.
        if(decl->HasModifier<ImplicitParameterBlockElementTypeModifier>())
            return;

        Emit("struct ");
        emitName(decl->getNameAndLoc());
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
            EmitType(declRef.getDecl()->type, declRef.getDecl()->getName());
        }
        else
        {
            EmitType(GetType(declRef), declRef.getDecl()->getName());
        }

        EmitSemantics(declRef.getDecl());

        // TODO(tfoley): technically have to apply substitution here too...
        if (auto initExpr = declRef.getDecl()->initExpr)
        {
            if (declRef.As<ParamDecl>()
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
                SLANG_RELEASE_ASSERT(byteOffsetInRegister % componentSize == 0);

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
                SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled HLSL register type");
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
        SLANG_RELEASE_ASSERT(computedLayout);

        auto varLayout = computedLayout.As<VarLayout>();
        return varLayout;
    }

    void emitHLSLParameterBlockDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterBlockType>      parameterBlockType,
        RefPtr<VarLayout>               layout)
    {
        // The data type that describes where stuff in the constant buffer should go
        RefPtr<Type> dataType = parameterBlockType->elementType;

        // We expect/require the data type to be a user-defined `struct` type
        auto declRefType = dataType->As<DeclRefType>();
        SLANG_RELEASE_ASSERT(declRefType);

        // We expect to always have layout information
        layout = maybeFetchLayout(varDecl, layout);
        SLANG_RELEASE_ASSERT(layout);

        // We expect the layout to be for a structured type...
        RefPtr<ParameterBlockTypeLayout> bufferLayout = layout->typeLayout.As<ParameterBlockTypeLayout>();
        SLANG_RELEASE_ASSERT(bufferLayout);

        RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementTypeLayout.As<StructTypeLayout>();
        SLANG_RELEASE_ASSERT(structTypeLayout);

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
            emitName(reflectionNameModifier->nameAndLoc);
        }

        EmitSemantics(varDecl, kESemanticMask_None);

        auto info = layout->FindResourceInfo(LayoutResourceKind::ConstantBuffer);
        SLANG_RELEASE_ASSERT(info);
        emitHLSLRegisterSemantic(*info);

        Emit("\n{\n");
        if (auto structRef = declRefType->declRef.As<StructDecl>())
        {
            int fieldCounter = 0;

            for (auto field : getMembersOfType<StructField>(structRef))
            {
                int fieldIndex = fieldCounter++;

                EmitVarDeclCommon(field);

                RefPtr<VarLayout> fieldLayout = structTypeLayout->fields[fieldIndex];
                SLANG_RELEASE_ASSERT(fieldLayout->varDecl.GetName() == field.GetName());

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
                        SLANG_RELEASE_ASSERT(cbufferResource);

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

        case LayoutResourceKind::PushConstantBuffer:
            Emit("layout(push_constant)\n");
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
        RefPtr<Type> dataType = parameterBlockType->elementType;

        // We expect/require the data type to be a user-defined `struct` type
        auto declRefType = dataType->As<DeclRefType>();
        SLANG_RELEASE_ASSERT(declRefType);

        // We expect the layout, if present, to be for a structured type...
        RefPtr<StructTypeLayout> structTypeLayout;
        if (layout)
        {

            auto typeLayout = layout->typeLayout;
            if (auto bufferLayout = typeLayout.As<ParameterBlockTypeLayout>())
            {
                typeLayout = bufferLayout->elementTypeLayout;
            }

            structTypeLayout = typeLayout.As<StructTypeLayout>();
            SLANG_RELEASE_ASSERT(structTypeLayout);

            emitGLSLLayoutQualifiers(layout);
        }


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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), varDecl, "unhandled GLSL shader parameter kind");
            Emit("uniform");
        }

        if( auto reflectionNameModifier = varDecl->FindModifier<ParameterBlockReflectionName>() )
        {
            Emit(" ");
            emitName(reflectionNameModifier->nameAndLoc);
        }

        Emit("\n{\n");
        if (auto structRef = declRefType->declRef.As<StructDecl>())
        {
            for (auto field : getMembersOfType<StructField>(structRef))
            {
                if (structTypeLayout)
                {
                    RefPtr<VarLayout> fieldLayout;
                    structTypeLayout->mapVarToLayout.TryGetValue(field.getDecl(), fieldLayout);
                    //            assert(fieldLayout);

                    // TODO(tfoley): We may want to emit *some* of these,
                    // some of the time...
        //            emitGLSLLayoutQualifiers(fieldLayout);
                }

                EmitVarDeclCommon(field);

                Emit(";\n");
            }
        }
        Emit("}");

        if( varDecl->getNameLoc().isValid() )
        {
            Emit(" ");
            emitName(varDecl->getName());
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
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), varDecl, "unhandled code generation target");
            break;
        }
    }

    void visitVarDeclBase(RefPtr<VarDeclBase> decl, DeclEmitArg const& arg)
    {
        // Global variable? Check if it is a sample-rate input.
        if (dynamic_cast<ModuleDecl*>(decl->ParentDecl))
        {
            if (decl->HasModifier<InModifier>())
            {
                doSampleRateInputCheck(decl);
            }
        }

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
        if (auto parameterBlockType = decl->type->As<ParameterBlockType>())
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

    void EmitParamDecl(RefPtr<ParamDecl> decl)
    {
        EmitVarDeclCommon(decl);
    }

    void visitFuncDecl(RefPtr<FuncDecl> decl, DeclEmitArg const&)
    {
        EmitModifiers(decl);

        // TODO: if a function returns an array type, or something similar that
        // isn't allowed by declarator syntax and/or language rules, we could
        // hypothetically wrap things in a `typedef` and work around it.

        EmitType(decl->ReturnType, decl->getNameAndLoc());

        Emit("(");
        bool first = true;
        for (auto paramDecl : decl->getMembersOfType<ParamDecl>())
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

    void emitGLSLVersionDirective(
        ModuleDecl*  program)
    {
        // Did the user provide an explicit `#version` directive in their code?
        if( auto versionDirective = program->FindModifier<GLSLVersionDirective>() )
        {
            // TODO(tfoley): Emit an appropriate `#line` directive...

            Emit("#version ");
            emit(versionDirective->versionNumberToken.Content);
            if(versionDirective->glslProfileToken.type != TokenType::Unknown)
            {
                Emit(" ");
                emit(versionDirective->glslProfileToken.Content);
            }
            Emit("\n");
            return;
        }

        // No explicit version was given. This could be because we are cross-compiling,
        // but it also might just be that they user wrote GLSL without thinking about
        // versions.

        // First, look and see if the target profile gives us a version to use:
        auto profile = context->shared->entryPoint->profile;
        if (profile.getFamily() == ProfileFamily::GLSL)
        {
            switch (profile.GetVersion())
            {
#define CASE(TAG, VALUE)    \
            case ProfileVersion::TAG: Emit("#version " #VALUE "\n"); return

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
#undef CASE

            default:
                break;
            }
        }

        // No information is available for us to guess a profile,
        // so it seems like we need to pick one out of thin air.
        //
        // Ideally we should infer a minimum required version based
        // on the constructs we have seen used in the user's code
        //
        // For now we just fall back to a reasonably recent version.

        Emit("#version 420\n");
    }

    void emitGLSLPreprocessorDirectives(
        RefPtr<ModuleDecl>   program)
    {
        switch(context->shared->target)
        {
        // Don't emit this stuff unless we are targetting GLSL
        default:
            return;

        case CodeGenTarget::GLSL:
            break;
        }

        emitGLSLVersionDirective(program);


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
        else if( auto declGroup = declBase.As<DeclGroup>() )
        {
            for( auto d : declGroup->decls )
                EmitDecl(d);
        }
        else
        {
            SLANG_UNEXPECTED("unhandled declaration kind");
        }
    }
};


EntryPointLayout* findEntryPointLayout(
    ProgramLayout*      programLayout,
    EntryPointRequest*  entryPointRequest)
{
    for( auto entryPointLayout : programLayout->entryPoints )
    {
        if(entryPointLayout->entryPoint->getName() != entryPointRequest->name)
            continue;

        if(entryPointLayout->profile != entryPointRequest->profile)
            continue;

        // TODO: can't easily filter on translation unit here...
        // Ideally the `EntryPointRequest` should get filled in with a pointer
        // the specific function declaration that represents the entry point.

        return entryPointLayout.Ptr();
    }

    return nullptr;
}

String emitEntryPoint(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target)
{
    auto translationUnit = entryPoint->getTranslationUnit();

    SharedEmitContext sharedContext;
    sharedContext.target = target;
    sharedContext.finalTarget = entryPoint->compileRequest->Target;
    sharedContext.entryPoint = entryPoint;

    if (entryPoint)
    {
        sharedContext.entryPointLayout = findEntryPointLayout(
            programLayout,
            entryPoint);
    }

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
    else if( auto globalConstantBufferLayout = globalScopeLayout.As<ParameterBlockTypeLayout>() )
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
        SLANG_RELEASE_ASSERT(elementTypeStructLayout);

        globalStructLayout = elementTypeStructLayout.Ptr();
    }
    else
    {
        SLANG_UNEXPECTED("uhandled global-scope binding layout");
    }
    sharedContext.globalStructLayout = globalStructLayout;

    auto translationUnitSyntax = translationUnit->SyntaxNode.Ptr();

    EmitContext context;
    context.shared = &sharedContext;

    EmitVisitor visitor(&context);

    // Depending on how the compiler was invoked, we may need to perform
    // some amount of preocessing on the code before we can emit it.
    //
    // For our purposes, there are basically three different "modes" we
    // care about:
    //
    // 1. "Full rewriter" mode, where the user provides HLSL/GLSL, and
    //    doesn't make use of any Slang code via `import`.
    //
    // 2. "Partial rewriter" mode, where the user starts with HLSL/GLSL,
    //    but also imports some Slang code, and may need us to rewrite
    //    their HLSL/GLSL function bodies to make things work.
    //
    // 3. "Full" mode, where all of the input code is in Slang (and/or
    //    the subset of HLSL we can fully type-check).
    //
    // We'll try to detect the cases here:
    //
#if 0
    if(!(translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING ))
    {
        // This seems to be case (3), because the user is asking for full
        // checking, and so we can assume we understand the code fully.
        //
        // In this case we want to translate to our intermediate representation
        // and do optimizations/transformations there before we emit final code.
        //

        auto lowered = lowerEntryPointToIR(entryPoint, programLayout, target);

        dumpIR(lowered);

        throw 99;

    }
    else if(translationUnit->compileRequest->loadedModulesList.Count() != 0)
#else
    if(!(translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING )
        || translationUnit->compileRequest->loadedModulesList.Count() != 0)
#endif
    {
        // The user has `import`ed some Slang modules, and so we are in case (2)
        //
        // We need to apply a "rewriting" pass to the code the user wrote,
        // and then emit the result.

        // We perform lowering of the program before emitting *anything*,
        // because the lowering process might change how we emit some
        // boilerplate at the start of the ouput for GLSL (e.g., what
        // version we require).
        auto lowered = lowerEntryPoint(entryPoint, programLayout, target, &sharedContext.extensionUsageTracker);
        sharedContext.program = lowered.program;

        // Note that we emit the main body code of the program *before*
        // we emit any leading preprocessor directives for GLSL.
        // This is to give the emit logic a change to make last-minute
        // adjustments like changing the required GLSL version.
        //
        // TODO: All such adjustments would be better handled during
        // lowering, but that requires having a semantic rather than
        // textual format for the HLSL->GLSL mapping.
        visitor.EmitDeclsInContainer(lowered.program.Ptr());
    }
    else
    {
        // We are in case (1).
        //
        // We should be able to just emit the AST we parsed right back out,
        // along with whatever annotations we added along the way.

        sharedContext.program = translationUnitSyntax;
        visitor.EmitDeclsInContainerUsingLayout(
            translationUnitSyntax,
            globalStructLayout);
    }

    String code = sharedContext.sb.ProduceString();
    sharedContext.sb.Clear();

    // Now that we've emitted the code for all the declaratiosn in the file,
    // it is time to stich together the final output.



    // There may be global-scope modifiers that we should emit now
    visitor.emitGLSLPreprocessorDirectives(translationUnitSyntax);
    String prefix = sharedContext.sb.ProduceString();


    StringBuilder finalResultBuilder;
    finalResultBuilder << prefix;

    finalResultBuilder << sharedContext.extensionUsageTracker.glslExtensionRequireLines.ProduceString();

    if (sharedContext.needHackSamplerForTexelFetch)
    {
        finalResultBuilder
            << "layout(set = 0, binding = "
            << programLayout->bindingForHackSampler
            << ") uniform sampler SLANG_hack_samplerForTexelFetch;\n";
    }

    finalResultBuilder << code;

    String finalResult = finalResultBuilder.ProduceString();

    return finalResult;
}

} // namespace Slang
