// emit.cpp
#include "emit.h"

#include "ast-legalize.h"
#include "ir-insts.h"
#include "legalize-types.h"
#include "lower-to-ir.h"
#include "mangle.h"
#include "name.h"
#include "syntax.h"
#include "type-layout.h"
#include "visitor.h"

#include <assert.h>

// Note: using C++ stdio just to get a locale-independent
// way to format floating-point values.
//
// TODO: Go ahead and implement teh Dragon4 algorithm so
// that we can print floating-point values to arbitrary
// precision as needed.
#include <sstream>

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

    UInt uniqueIDCounter = 1;
    Dictionary<IRValue*, UInt> mapIRValueToID;
    Dictionary<Decl*, UInt> mapDeclToID;

    HashSet<String> irDeclsVisited;

    Dictionary<IRBlock*, IRBlock*> irMapContinueTargetToLoopHead;

    HashSet<String> irTupleTypes;

    // Map used to tell AST lowering what decls are represented by IR.
    HashSet<Decl*>* irDeclSetForAST = nullptr;
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
        auto len = textEnd - textBegin;
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

    void emit(UnownedStringSlice const& text)
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

    void emitName(
        Decl*               decl,
        SourceLoc const&    loc)
    {
        if(auto name = decl->getName())
            emitName(name, loc);

        Emit("_S");
        Emit(getID(decl));
    }

    void emitName(
        Decl*               decl)
    {
        emitName(decl, SourceLoc());
    }

    void Emit(IntegerLiteralValue value)
    {
        char buffer[32];
        sprintf(buffer, "%lld", (long long int)value);
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
        // There are a few different requirements here that we need to deal with:
        //
        // 1) We need to print somethign that is valid syntax in the target language
        //    (this means that hex floats are off the table for now)
        //
        // 2) We need our printing to be independent of the current global locale in C,
        //    so that we don't depend on the application leaving it as the default,
        //    and we also don't revert any changes they make.
        //    (this means that `sprintf` and friends are off the table)
        //
        // 3) We need to be sure that floating-point literals specified by the user will
        //    "round-trip" and turn into the same value when parsed back in. This means
        //    that we need to print a reasonable number of digits of precision.
        //
        // For right now, the easiest option that can balance these is to use
        // the C++ standard library `iostream`s, because they support an explicit locale,
        // and can (hopefully) print floating-point numbers accurately.
        //
        // Eventually, the right move here would be to implement proper floating-point
        // number formatting ourselves, but that would require extensive testing to
        // make sure we get it right.

        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream.setf(std::ios::fixed,std::ios::floatfield);
        stream.precision(20);
        stream << value;

        Emit(stream.str().c_str());
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
            switch (basicElementType->baseType)
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

    UNEXPECTED(IRBasicBlockType);
    UNEXPECTED(PtrType);

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
        switch (basicType->baseType)
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


        emitTypeImpl(arrayType->baseType, &arrayDeclarator);
    }

    void visitGroupSharedType(GroupSharedType* type, TypeEmitArg const& arg)
    {
        Emit("groupshared ");
        emitTypeImpl(type->valueType, arg.declarator);
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

    void EmitType(RefPtr<Type> type, String const& name)
    {
        // HACK: the rest of the code wants a `Name`,
        // so we'll create one for a bit...
        Name tempName;
        tempName.text = name;

        EmitType(type, SourceLoc(), &tempName, SourceLoc());
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

        // Is the expression referencing a uniform parameter group,
        // but *not* a `ParameterBlock<T>`?
        if (auto parameterBlockType = e->type->As<ParameterBlockType>())
        {
            return false;
        }
        if (auto uniformParameterGroupType = e->type->As<UniformParameterGroupType>())
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
                expr = typeCastExpr->Arguments[0];
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

    void emitSimpleCallArgs(
        RefPtr<InvokeExpr>  callExpr)
    {
        Emit("(");
        UInt argCount = callExpr->Arguments.Count();
        for (UInt aa = 0; aa < argCount; ++aa)
        {
            if (aa != 0) Emit(", ");
            EmitExpr(callExpr->Arguments[aa]);
        }
        Emit(")");
    }

    void emitTypeOrExpr(
        Type*   type,
        Expr*   expr)
    {
        if (type && !type->As<ErrorType>())
        {
            EmitType(type);
        }
        else
        {
            emitTypeBasedOnExpr(expr, nullptr);
        }
    }

    void emitSimpleConstructorCallExpr(
        RefPtr<InvokeExpr>  callExpr,
        EOpInfo             outerPrec)
    {
        if(context->shared->target == CodeGenTarget::HLSL)
        {
            // HLSL needs to special-case a constructor call with a single argument.
            if(callExpr->Arguments.Count() == 1)
            {
                auto prec = kEOp_Prefix;
                bool needClose = MaybeEmitParens(outerPrec, prec);

                Emit("(");
                emitTypeOrExpr(callExpr->type.type, callExpr->FunctionExpr);
                Emit(") ");

                EmitExprWithPrecedence(callExpr->Arguments[0], rightSide(outerPrec, prec));

                if(needClose) Emit(")");
                return;
            }
        }


        // Default handling is to emit what amounts to an ordinary call,
        // but using the type of the expression directly as the "function" to call.
        auto prec = kEOp_Postfix;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        emitTypeOrExpr(callExpr->type.type, callExpr->FunctionExpr);

        emitSimpleCallArgs(callExpr);

        if (needClose)
        {
            Emit(")");
        }
    }

    void emitSimpleSubscriptCallExpr(
        RefPtr<InvokeExpr>  callExpr,
        EOpInfo             /*outerPrec*/)
    {
        auto funcExpr = callExpr->FunctionExpr;

        // We expect any subscript operation to be invoked as a member,
        // so the function expression had better be in the correct form.
        auto memberExpr = funcExpr.As<MemberExpr>();
        if(!memberExpr)
        {
            SLANG_UNEXPECTED("subscript needs base expression");
        }

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
    }

    // Emit a call expression that doesn't involve any special cases,
    // just an expression of the form `f(a0, a1, ...)`
    void emitSimpleCallExpr(
        RefPtr<InvokeExpr>  callExpr,
        EOpInfo                             outerPrec)
    {
        // We will first check if this represents a constructor call,
        // since those may need to be handled differently.

        auto funcExpr = callExpr->FunctionExpr;
        if (auto funcDeclRefExpr = funcExpr.As<DeclRefExpr>())
        {
            auto declRef = funcDeclRefExpr->declRef;
            if (auto ctorDeclRef = declRef.As<ConstructorDecl>())
            {
                emitSimpleConstructorCallExpr(callExpr, outerPrec);
                return;
            }

            if(auto acessorDeclRef = declRef.As<AccessorDecl>())
            {
                declRef = acessorDeclRef.GetParent();
            }

            if(auto subscriptDeclRef = declRef.As<SubscriptDecl>())
            {
                emitSimpleSubscriptCallExpr(callExpr, outerPrec);
                return;
            }

        }

        // Once we've ruled out constructor calls, we can move on
        // to just emitting an ordinary calll expression.

        auto prec = kEOp_Postfix;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        EmitExprWithPrecedence(funcExpr, leftSide(outerPrec, prec));

        emitSimpleCallArgs(callExpr);

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
            if (!funcDecl)
            {
                emitUncheckedCallExpr(callExpr, funcDeclRefExpr->name, arg);
                return;
            }
            // Note: We check for a "target intrinsic" modifier that flags the
            // operation as having a custom elaboration for a specific target
            // *before* we check for an "intrinsic op." The basic problem is
            // that a single operation could have both finds of modifiers on it.
            // The "target" intrinsic modifier tags something expansion during
            // our current source-to-source translation approach, while an
            // intrinsic op is needed for helping things lower to our IR.
            //
            // We need to check for this case first to make sure that when a
            // function gets an intrinsic op added it doesn't break existing
            // cross-compilation logic.
            //
            // The long term fix will be to not use the AST-based cross-compilation
            // logic (which has all kinds of problems) and instead use the IR
            // exclusively, at which point the notion of a "target intrinsic" modifier
            // goes away (although we may have something similar to express how
            // a particular op should lower/expand for a given target).
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

                // If we fall through here, we will treat the call like any other.
            }
            else if (auto intrinsicOpModifier = funcDecl->FindModifier<IntrinsicOpModifier>())
            {
                switch (intrinsicOpModifier->op)
                {
#define CASE(NAME, OP) case kIROp_##NAME: EmitBinExpr(outerPrec, kEOp_##NAME, #OP, callExpr); return
                    CASE(Mul, *);
                    CASE(Div, / );
                    CASE(Mod, %);
                    CASE(Add, +);
                    CASE(Sub, -);
                    CASE(Lsh, << );
                    CASE(Rsh, >> );
                    CASE(Eql, == );
                    CASE(Neq, != );
                    CASE(Greater, > );
                    CASE(Less, < );
                    CASE(Geq, >= );
                    CASE(Leq, <= );
                    CASE(BitAnd, &);
                    CASE(BitXor, ^);
                    CASE(BitOr, | );
                    CASE(And, &&);
                    CASE(Or, || );
#undef CASE

#define CASE(NAME, OP) case kIRPseudoOp_##NAME: EmitBinAssignExpr(outerPrec, kEOp_##NAME, #OP, callExpr); return
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

                case kIRPseudoOp_Sequence: EmitBinExpr(outerPrec, kEOp_Comma, ",", callExpr); return;

#define CASE(NAME, OP) case NAME: EmitUnaryExpr(outerPrec, kEOp_Prefix, #OP, "", callExpr); return
                    CASE(kIRPseudoOp_Pos, +);
                    CASE(kIROp_Neg, -);
                    CASE(kIROp_Not, !);
                    CASE(kIRPseudoOp_BitNot, ~);
#undef CASE

#define CASE(NAME, OP) case kIRPseudoOp_##NAME: EmitUnaryAssignExpr(outerPrec, kEOp_Prefix, #OP, "", callExpr); return
                    CASE(PreInc, ++);
                    CASE(PreDec, --);
#undef CASE

#define CASE(NAME, OP) case kIRPseudoOp_##NAME: EmitUnaryAssignExpr(outerPrec, kEOp_Postfix, "", #OP, callExpr); return
                    CASE(PostInc, ++);
                    CASE(PostDec, --);
#undef CASE

                case kIROp_Dot:
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

                case kIROp_Mul_Matrix_Matrix:
                case kIROp_Mul_Vector_Matrix:
                case kIROp_Mul_Matrix_Vector:
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

                // If none of the above matched, then we don't have a specific
                // case implemented to handle the opcode on the callee.
                //
                // We do one more special-case check here, in case the operation
                // is a "subscript" (array indexing) operation, because we'd
                // generally like to reproduce those as array indexing operations
                // in the output if that is how they were written in the input.
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

    void visitStaticMemberExpr(StaticMemberExpr* memberExpr, ExprEmitArg const& arg)
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
            EmitDeclRef(memberExpr->declRef);
//            emit(memberExpr->declRef.GetName());
        }

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
            EmitDeclRef(memberExpr->declRef);
//            emit(memberExpr->declRef.GetName());
        }

        if(needClose) Emit(")");
    }

    void visitThisExpr(ThisExpr* /*expr*/, ExprEmitArg const& arg)
    {
        auto prec = kEOp_Atomic;
        auto outerPrec = arg.outerPrec;
        bool needClose = MaybeEmitParens(outerPrec, prec);

        Emit("this");

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
        ExprVisitorWithArg::dispatch(castExpr->Arguments[0], arg);
    }

    void visitTypeCastExpr(TypeCastExpr* castExpr, ExprEmitArg const& arg)
    {
        // We emit a type cast expression as a constructor call
        emitSimpleConstructorCallExpr(castExpr, arg.outerPrec);

#if 0
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
#endif
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
            // Emit whatever attributes the user might have attached,
            // whether or not we think they make semantic sense.
            //
            Emit("[");
            emit(attr->getName());
            Emit("]");
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

            Emit("do\n");
            EmitBlockStmt(doWhileStmt->Statement);
            Emit(" while(");
            EmitExpr(doWhileStmt->Predicate);
            Emit(");\n");
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
            Emit("default:\n");
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
        // Are we emitting an AST in a context where some declarations
        // are actually stored as IR code?
        if(auto irDeclSet = context->shared->irDeclSetForAST)
        {
            Decl* decl = declRef.getDecl();
            if(irDeclSet->Contains(decl))
            {
                emit(getIRName(declRef));
                return;
            }
        }


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

            GenericSubstitution* subst = declRef.substitutions.As<GenericSubstitution>().Ptr();
            if (!subst)
                return;

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

    void visitAssocTypeDecl(AssocTypeDecl * /*assocType*/, DeclEmitArg const&)
    {
        SLANG_UNREACHABLE("visitAssocTypeDecl in EmitVisitor");
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
        if(decl->HasModifier<ImplicitParameterGroupElementTypeModifier>())
            return;

        Emit("struct ");
        emitName(decl->getNameAndLoc());
        Emit("\n{\n");

        // TODO(tfoley): Need to hoist members functions, etc. out to global scope
        EmitDeclsInContainer(decl);

        Emit("};\n");
    }

    // Shared emit logic for variable declarations (used for parameters, locals, globals, fields)

    void emitVarDeclHead(DeclRef<VarDeclBase> declRef)
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
    }

    void emitVarDeclInit(DeclRef<VarDeclBase> declRef)
    {
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

    void EmitVarDeclCommon(DeclRef<VarDeclBase> declRef)
    {
        emitVarDeclHead(declRef);
        emitVarDeclInit(declRef);
    }

    // Shared emit logic for variable declarations (used for parameters, locals, globals, fields)
    void EmitVarDeclCommon(RefPtr<VarDeclBase> decl)
    {
        EmitVarDeclCommon(DeclRef<Decl>(decl.Ptr(), nullptr).As<VarDeclBase>());
    }

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

    // Emit a single `regsiter` semantic, as appropriate for a given resource-type-specific layout info
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
            break;

        case LayoutResourceKind::RegisterSpace:
        case LayoutResourceKind::GenericResource:
            // ignore
            break;
        default:
            {
                Emit(": register(");
                switch( kind )
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
                Emit(index);
                if(space)
                {
                    Emit(", space");
                    Emit(space);
                }
                Emit(")");
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

        switch( context->shared->target )
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

    void emitHLSLParameterBlockDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterBlockType>      parameterBlockType,
        RefPtr<VarLayout>               varLayout)
    {
        EmitVarChain blockChain(varLayout);

        Emit("cbuffer ");

        emitName(varDecl);

        // We expect to always have layout information
        varLayout = maybeFetchLayout(varDecl, varLayout);
        SLANG_RELEASE_ASSERT(varLayout);

        // We expect the layout to be for a parameter group type...
        RefPtr<ParameterGroupTypeLayout> bufferLayout = varLayout->typeLayout.As<ParameterGroupTypeLayout>();
        SLANG_RELEASE_ASSERT(bufferLayout);

        RefPtr<VarLayout> containerVarLayout = bufferLayout->containerVarLayout;
        EmitVarChain containerChain(containerVarLayout, &blockChain);

        RefPtr<VarLayout> elementVarLayout = bufferLayout->elementVarLayout;
        EmitVarChain elementChain(elementVarLayout, &blockChain);

        EmitSemantics(varDecl, kESemanticMask_None);

        emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

        Emit("\n{\n");

        // The user wrote this declaration as, e.g.:
        //
        //      ParameterBlock<Foo> gFoo;
        //
        // and we are desugaring it into something like:
        //
        //      cbuffer anon0 { Foo gFoo; }
        //

        RefPtr<Type> elementType = parameterBlockType->elementType;

        EmitType(elementType, varDecl->getName());

        // The layout for the field ends up coming from the layout
        // for the parameter block as a whole.
        emitHLSLParameterGroupFieldLayoutSemantics(&elementChain);

        Emit(";\n");
        Emit("}\n");
    }

    void emitHLSLParameterGroupDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterGroupType>      parameterGroupType,
        RefPtr<VarLayout>               varLayout)
    {
        if( auto parameterBlockType = parameterGroupType->As<ParameterBlockType>())
        {
            emitHLSLParameterBlockDecl(varDecl, parameterBlockType, varLayout);
            return;
        }
        if( auto textureBufferType = parameterGroupType->As<TextureBufferType>() )
        {
            Emit("tbuffer ");
        }
        else
        {
            Emit("cbuffer ");
        }

        EmitVarChain blockChain(varLayout);

        // The data type that describes where stuff in the constant buffer should go
        RefPtr<Type> dataType = parameterGroupType->elementType;

        // We expect to always have layout information
        varLayout = maybeFetchLayout(varDecl, varLayout);
        SLANG_RELEASE_ASSERT(varLayout);

        // We expect the layout to be for a structured type...
        RefPtr<ParameterGroupTypeLayout> bufferLayout = varLayout->typeLayout.As<ParameterGroupTypeLayout>();
        SLANG_RELEASE_ASSERT(bufferLayout);

        auto containerVarLayout = bufferLayout->containerVarLayout;
        EmitVarChain containerChain(containerVarLayout, &blockChain);

        auto elementVarLayout = bufferLayout->elementVarLayout;
        EmitVarChain elementChain(elementVarLayout, &blockChain);

        RefPtr<StructTypeLayout> structTypeLayout = bufferLayout->elementVarLayout->typeLayout.As<StructTypeLayout>();
        SLANG_RELEASE_ASSERT(structTypeLayout);


        Emit(" ");
        if( auto reflectionNameModifier = varDecl->FindModifier<ParameterGroupReflectionName>() )
        {
            emitName(reflectionNameModifier->nameAndLoc);
        }
        else
        {
            emitName(varDecl->nameAndLoc);
        }

        EmitSemantics(varDecl, kESemanticMask_None);

        emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

        Emit("\n{\n");

        // We expect the data type to be a user-defined `struct` type,
        // but it might also be a "filtered" type that represents the
        // case where only some fields of the original type are valid
        // to appear inside of a `struct`.
        if (auto declRefType = dataType->As<DeclRefType>())
        {
            if (auto structRef = declRefType->declRef.As<StructDecl>())
            {
                int fieldCounter = 0;

                for (auto field : getMembersOfType<StructField>(structRef))
                {
                    int fieldIndex = fieldCounter++;

                    // Skip fields that have `void` type, since these represent
                    // declarations that got legalized out of existence.
                    if(GetType(field)->Equals(getSession()->getVoidType()))
                        continue;

                    emitVarDeclHead(field);

                    RefPtr<VarLayout> fieldLayout = structTypeLayout->fields[fieldIndex];
                    SLANG_RELEASE_ASSERT(fieldLayout->varDecl.GetName() == field.GetName());

                    // Emit explicit layout annotations for every field
                    emitHLSLParameterGroupFieldLayoutSemantics(fieldLayout, &elementChain);

                    emitVarDeclInit(field);

                    Emit(";\n");
                }
            }
            else
            {
                SLANG_UNEXPECTED("unexpected element type for parameter group");
            }
        }
        else
        {
            SLANG_UNEXPECTED("unexpected element type for parameter group");
        }
        Emit("}\n");
    }

    void emitGLSLLayoutQualifier(
        LayoutResourceKind  kind,
        EmitVarChain*       chain)
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
                // Explicit offsets require a GLSL extension (which
                // is not universally supported, it seems) or a new
                // enough GLSL version (which we don't want to
                // universall require), so for right now we
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

                    Emit("layout(offset = ");
                    Emit(index);
                    Emit(")\n");
                }
            }
            break;

        case LayoutResourceKind::VertexInput:
        case LayoutResourceKind::FragmentOutput:
            Emit("layout(location = ");
            Emit(index);
            Emit(")\n");
            break;

        case LayoutResourceKind::SpecializationConstant:
            Emit("layout(constant_id = ");
            Emit(index);
            Emit(")\n");
            break;

        case LayoutResourceKind::ConstantBuffer:
        case LayoutResourceKind::ShaderResource:
        case LayoutResourceKind::UnorderedAccess:
        case LayoutResourceKind::SamplerState:
        case LayoutResourceKind::DescriptorTableSlot:
            Emit("layout(binding = ");
            Emit(index);
            if(space)
            {
                Emit(", set = ");
                Emit(space);
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
        EmitVarChain*                   inChain,
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

    void emitGLSLParameterBlockDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterBlockType>      parameterBlockType,
        RefPtr<VarLayout>               varLayout)
    {
        EmitVarChain blockChain(varLayout);

        RefPtr<ParameterGroupTypeLayout> bufferLayout = varLayout->typeLayout.As<ParameterGroupTypeLayout>();
        SLANG_RELEASE_ASSERT(bufferLayout);

        auto containerVarLayout = bufferLayout->containerVarLayout;
        EmitVarChain containerChain(containerVarLayout, &blockChain);

        auto elementVarLayout = bufferLayout->elementVarLayout;
        EmitVarChain elementChain(elementVarLayout, &blockChain);

        EmitModifiers(varDecl);
        emitGLSLLayoutQualifiers(containerVarLayout, &blockChain);
        Emit("uniform ");

        emitName(varDecl);

        Emit("\n{\n");


        RefPtr<Type> elementType = parameterBlockType->elementType;

        EmitType(elementType, varDecl->getName());
        Emit(";\n");

        Emit("};\n");
    }

    void emitGLSLParameterGroupDecl(
        RefPtr<VarDeclBase>             varDecl,
        RefPtr<ParameterGroupType>      parameterGroupType,
        RefPtr<VarLayout>               varLayout)
    {
        if( auto parameterBlockType = parameterGroupType->As<ParameterBlockType>())
        {
            emitGLSLParameterBlockDecl(varDecl, parameterBlockType, varLayout);
            return;
        }

        // The data type that describes where stuff in the constant buffer should go
        RefPtr<Type> dataType = parameterGroupType->elementType;

        // We expect the layout, if present, to be for a structured type...
        RefPtr<StructTypeLayout> structTypeLayout;

        EmitVarChain blockChain;
        if (varLayout)
        {
            blockChain = EmitVarChain(varLayout);

            auto typeLayout = varLayout->typeLayout;
            if (auto bufferLayout = typeLayout.As<ParameterGroupTypeLayout>())
            {
                typeLayout = bufferLayout->elementVarLayout->getTypeLayout();

                emitGLSLLayoutQualifiers(bufferLayout->containerVarLayout, &blockChain);
            }
            else
            {
                // Fallback: we somehow have a messed up layout
                emitGLSLLayoutQualifiers(varLayout, nullptr);
            }

            // We expect the element type to be structured.
            structTypeLayout = typeLayout.As<StructTypeLayout>();
            SLANG_RELEASE_ASSERT(structTypeLayout);
        }


        EmitModifiers(varDecl);

        // Emit an apprpriate declaration keyword based on the kind of block
        if (parameterGroupType->As<GLSLShaderStorageBufferType>())
        {
            Emit("buffer");
        }
        // Note: tested `buffer` case before `uniform`, since `GLSLShaderStorageBufferType`
        // is also a subclass of `UniformParameterGroupType`.
        else if (parameterGroupType->As<UniformParameterGroupType>())
        {
            Emit("uniform");
        }
        else if (parameterGroupType->As<GLSLInputParameterGroupType>())
        {
            Emit("in");
        }
        else if (parameterGroupType->As<GLSLOutputParameterGroupType>())
        {
            Emit("out");
        }
        else
        {
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), varDecl, "unhandled GLSL shader parameter kind");
            Emit("uniform");
        }

        if( auto reflectionNameModifier = varDecl->FindModifier<ParameterGroupReflectionName>() )
        {
            Emit(" ");
            emitName(reflectionNameModifier->nameAndLoc);
        }

        Emit("\n{\n");

        // We expect the data type to be a user-defined `struct` type,
        // but it might also be a "filtered" type that represents the
        // case where only some fields of the original type are valid
        // to appear inside of a `struct`.
        if (auto declRefType = dataType->As<DeclRefType>())
        {

            if (auto structRef = declRefType->declRef.As<StructDecl>())
            {
                int fieldCounter = 0;
                for (auto field : getMembersOfType<StructField>(structRef))
                {
                    int fieldIndex = fieldCounter++;

                    // Skip fields that have `void` type, since these represent
                    // declarations that got legalized out of existence.
                    if(GetType(field)->Equals(getSession()->getVoidType()))
                        continue;

                    if (structTypeLayout)
                    {
                        RefPtr<VarLayout> fieldLayout = structTypeLayout->fields[fieldIndex];
                        //            assert(fieldLayout);

                        // TODO(tfoley): We may want to emit *some* of these,
                        // some of the time...
            //            emitGLSLLayoutQualifiers(fieldLayout);
                    }


                    EmitVarDeclCommon(field);

                    Emit(";\n");
                }
            }
            else
            {
                SLANG_UNEXPECTED("unexpected element type for parameter group");
            }
        }
        else
        {
            SLANG_UNEXPECTED("unexpected element type for parameter group");
        }



        Emit("}");

        if( varDecl->getNameLoc().isValid() )
        {
            Emit(" ");
            emitName(varDecl->getName());
        }

        Emit(";\n");
    }

    void emitParameterGroupDecl(
        RefPtr<VarDeclBase>			varDecl,
        RefPtr<ParameterGroupType>  parameterGroupType,
        RefPtr<VarLayout>           layout)
    {
        switch(context->shared->target)
        {
        case CodeGenTarget::HLSL:
            emitHLSLParameterGroupDecl(varDecl, parameterGroupType, layout);
            break;

        case CodeGenTarget::GLSL:
            emitGLSLParameterGroupDecl(varDecl, parameterGroupType, layout);
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

        // Skip fields that have `void` type, since these may be introduced
        // as part of type leglaization.
        if(decl->getType()->Equals(getSession()->getVoidType()))
            return;

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
        if (auto parameterGroupType = decl->type->As<ParameterGroupType>())
        {
            emitParameterGroupDecl(decl, parameterGroupType, layout);
            return;
        }


        if (context->shared->target == CodeGenTarget::GLSL)
        {
            if (decl->HasModifier<InModifier>())
            {
                emitGLSLLayoutQualifiers(layout, nullptr, LayoutResourceKind::VertexInput);
            }
            else if (decl->HasModifier<OutModifier>())
            {
                emitGLSLLayoutQualifiers(layout, nullptr, LayoutResourceKind::FragmentOutput);
            }
            else
            {
                emitGLSLLayoutQualifiers(layout, nullptr);
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

        emitVarDeclHead(makeDeclRef(decl.Ptr()));
        emitHLSLRegisterSemantics(layout);
        emitVarDeclInit(makeDeclRef(decl.Ptr()));

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

    // Utility code for generating unique IDs as needed
    // during the emit process (e.g., for declarations
    // that didn't origianlly have names, but now need to).

    UInt allocateUniqueID()
    {
        return context->shared->uniqueIDCounter++;
    }

    UInt getID(Decl* decl)
    {
        auto& mapDeclToID = context->shared->mapDeclToID;

        UInt id = 0;
        if(mapDeclToID.TryGetValue(decl, id))
            return id;

        id = allocateUniqueID();
        mapDeclToID.Add(decl, id);
        return id;
    }

    // IR-level emit logc

    UInt getID(IRValue* value)
    {
        auto& mapIRValueToID = context->shared->mapIRValueToID;

        UInt id = 0;
        if (mapIRValueToID.TryGetValue(value, id))
            return id;

        id = allocateUniqueID();
        mapIRValueToID.Add(value, id);
        return id;
    }

    String getIRName(Decl* decl)
    {
        // TODO: need a flag to get rid of the step that adds
        // a prefix here, so that we can get "clean" output
        // when needed.
        //

        String name;
        if (!(context->shared->entryPoint->compileRequest->compileFlags & SLANG_COMPILE_FLAG_NO_MANGLING))
        {
            name.append("_s");
        }
        name.append(getText(decl->getName()));
        return name;
    }

    String getIRName(DeclRefBase const& declRef)
    {
        // It is a bit ugly, but we need a deterministic way
        // to get a name for things when emitting from the IR
        // that won't conflict with any keywords, builtins, etc.
        // in the target language.
        //
        // Eventually we should accomplish this by using
        // mangled names everywhere, but that complicates things
        // when we are also using direct comparison to fxc/glslang
        // output for some of our tests.
        //

        String name;
        if (context->shared->entryPoint->compileRequest->compileFlags & SLANG_COMPILE_FLAG_NO_MANGLING)
        {
            name.append(getText(declRef.GetName()));
        }
        else
        {
            name.append(getMangledName(declRef));
        }
        return name;
    }

    String getGLSLSystemValueName(
        VarLayout*  varLayout)
    {
        auto semanticName = varLayout->systemValueSemantic;
        semanticName = semanticName.ToLower();
        
        if(semanticName == "sv_position")
        {
            return "gl_Position";
        }
        else if(semanticName == "sv_target")
        {
            return "";
        }
        else
        {
            return "gl_Unknown";
        }
    }

    String getIRName(
        IRValue*        inst)
    {
        switch(inst->op)
        {
        case kIROp_decl_ref:
            {
                auto irDeclRef = (IRDeclRef*) inst;
                return getIRName(irDeclRef->declRef);
            }
            break;

        default:
            break;
        }

        if(getTarget(context) == CodeGenTarget::GLSL)
        {
            if(auto layoutMod = inst->findDecoration<IRLayoutDecoration>())
            {
                auto layout = layoutMod->layout;
                if(auto varLayout = layout.As<VarLayout>())
                {
                    if(varLayout->systemValueSemantic.Length() != 0)
                    {
                        auto translated = getGLSLSystemValueName(varLayout);
                        if(translated.Length())
                            return translated;
                    }
                }
            }
        }

        if(auto decoration = inst->findDecoration<IRHighLevelDeclDecoration>())
        {
            auto decl = decoration->decl;
            if (auto reflectionNameMod = decl->FindModifier<ParameterGroupReflectionName>())
            {
                return getText(reflectionNameMod->nameAndLoc.name);
            }

            if ((context->shared->entryPoint->compileRequest->compileFlags & SLANG_COMPILE_FLAG_NO_MANGLING))
            {
                return getIRName(decl);
            }
        }

        switch (inst->op)
        {
        case kIROp_global_var:
        case kIROp_Func:
            {
                auto& mangledName = ((IRGlobalValue*)inst)->mangledName;
                if(mangledName.Length() != 0)
                    return mangledName;
            }
            break;

        default:
            break;
        }

        StringBuilder sb;
        sb << "_S";
        sb << getID(inst);
        return sb.ProduceString();
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
        EmitContext*    ctx,
        IRDeclaratorInfo*   declarator)
    {
        if(!declarator)
            return;

        switch( declarator->flavor )
        {
        case IRDeclaratorInfo::Flavor::Simple:
            emit(" ");
            emit(*declarator->name);
            break;

        case IRDeclaratorInfo::Flavor::Ptr:
            emit("*");
            emitDeclarator(ctx, declarator->next);
            break;

        case IRDeclaratorInfo::Flavor::Array:
            emitDeclarator(ctx, declarator->next);
            emit("[");
            emitIROperand(ctx, declarator->elementCount);
            emit("]");
            break;
        }
    }

    void emitIRSimpleValue(
        EmitContext*    /*context*/,
        IRInst*         inst)
    {
        switch(inst->op)
        {
        case kIROp_IntLit:
            emit(((IRConstant*) inst)->u.intVal);
            break;

        case kIROp_FloatLit:
            emit(((IRConstant*) inst)->u.floatVal);
            break;

        case kIROp_boolConst:
            {
                bool val = ((IRConstant*)inst)->u.intVal != 0;
                emit(val ? "true" : "false");
            }
            break;

        default:
            SLANG_UNIMPLEMENTED_X("val case for emit");
            break;
        }

    }

#if 0
    void emitIRSimpleType(
        EmitContext*    context,
        IRType*         type)
    {
        switch(type->op)
        {
    #define CASE(ID, NAME) \
        case kIROp_##ID: emit(#NAME); break

        CASE(Float32Type,   float);
        CASE(Int32Type,     int);
        CASE(UInt32Type,    uint);
        CASE(VoidType,      void);
        CASE(BoolType,      bool);

    #undef CASE


        case kIROp_VectorType:
            emitIRVectorType(context, (IRVectorType*) type);
            break;

        case kIROp_MatrixType:
            emitIRMatrixType(context, (IRMatrixType*) type);
            break;

        case kIROp_StructType:
            emit(getName(type));
            break;

        case kIROp_TextureType:
            {
                auto textureType = (IRTextureType*) type;

                switch (context->shared->target)
                {
                case CodeGenTarget::HLSL:
                    // TODO: actually look at the flavor and emit the right name
                    emit("Texture2D");
                    emit("<");
                    emitIRType(context, textureType->getElementType(), nullptr);
                    emit(">");
                    break;

                case CodeGenTarget::GLSL:
                    // TODO: actually look at the flavor and emit the right name
                    // TODO: look at element type to emit the right prefix
                    emit("texture2D");
                    break;

                default:
                    SLANG_UNEXPECTED("codegen target");
                    break;
                }
            }
            break;

        case kIROp_ConstantBufferType:
            {
                auto tt = (IRConstantBufferType*) type;
                emit("ConstantBuffer<");
                emitIRType(context, tt->getElementType(), nullptr);
                emit(">");
            }
            break;

        case kIROp_TextureBufferType:
            {
                auto tt = (IRTextureBufferType*) type;
                emit("ConstantBuffer<");
                emitIRType(context, tt->getElementType(), nullptr);
                emit(">");
            }
            break;

        case kIROp_readWriteStructuredBufferType:
            {
                auto tt = (IRBufferType*) type;
                emit("RWStructuredBuffer<");
                emitIRType(context, tt->getElementType(), nullptr);
                emit(">");
            }
            break;

        case kIROp_structuredBufferType:
            {
                auto tt = (IRBufferType*) type;
                emit("StructuredBuffer<");
                emitIRType(context, tt->getElementType(), nullptr);
                emit(">");
            }
            break;

        case kIROp_SamplerType:
            {
                // TODO: actually look at the flavor and emit the right name
                emit("SamplerState");
            }
            break;

        case kIROp_TypeType:
            {
                // Note: this should actually be an error case, since
                // type-level operands shouldn't be exposed in generated
                // code, but I'm allowing this now to make the output
                // a bit more clear.
                emit("Type");
            }
            break;


        default:
            SLANG_UNIMPLEMENTED_X("type case for emit");
            break;
        }

    }
#endif

    CodeGenTarget getTarget(EmitContext* ctx)
    {
        return ctx->shared->target;
    }

#if 0
    void emitGLSLTypePrefix(
        EmitContext*    context,
        IRType*   type)
    {
        switch(type->op)
        {
        case kIROp_UInt32Type:
            emit("u");
            break;

        case kIROp_Int32Type:
            emit("i");
            break;

        case kIROp_BoolType:
            emit("b");
            break;

        case kIROp_Float32Type:
            // no prefix
            break;

        case kIROp_Float64Type:
            emit("d");
            break;

        // TODO: we should handle vector and matrix types here
        // and recurse into them to find the elemnt type,
        // just as a convenience.

        default:
            SLANG_UNEXPECTED("case for GLSL type prefix");
            break;
        }
    }
#endif

#if 0
    void emitIRType(
        EmitContext*        context,
        IRType*             type,
        IRDeclaratorInfo*   declarator)
    {
        switch (type->op)
        {
        case kIROp_PtrType:
            {
                auto ptrType = (IRPtrType*)type;

                IRDeclaratorInfo ptrDeclarator;
                ptrDeclarator.flavor = IRDeclaratorInfo::Flavor::Ptr;
                ptrDeclarator.next = declarator;
                emitIRType(context, ptrType->getValueType(), &ptrDeclarator);
            }
            break;

        case kIROp_arrayType:
            {
                auto arrayType = (IRArrayType*)type;

                IRDeclaratorInfo arrayDeclarator;
                arrayDeclarator.flavor = IRDeclaratorInfo::Flavor::Array;
                arrayDeclarator.elementCount = arrayType->getElementCount();
                arrayDeclarator.next = declarator;
                emitIRType(context, arrayType->getElementType(), &arrayDeclarator);
            }
            break;

        default:
            emitIRSimpleType(context, type);
            emitDeclarator(context, declarator);
            break;
        }
    }

    void emitIRType(
        EmitContext*    context,
        IRType*         type,
        String const&   name)
    {
        IRDeclaratorInfo declarator;
        declarator.flavor = IRDeclaratorInfo::Flavor::Simple;
        declarator.name = &name;

        emitIRType(context, type, &declarator);
    }

    void emitIRType(
        EmitContext*    context,
        IRType*         type)
    {
        emitIRType(context, type, (IRDeclaratorInfo*) nullptr);
    }
#endif

    bool shouldFoldIRInstIntoUseSites(
        EmitContext*    ctx,
        IRValue*        inst)
    {
        // Certain opcodes should always be folded in
        switch( inst->op )
        {
        default:
            break;

        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_boolConst:
        case kIROp_FieldAddress:
        case kIROp_getElementPtr:
        case kIROp_specialize:
            return true;
        }

        // Certain *types* will usually want to be folded in,
        // because they aren't allowed as types for temporary
        // variables.
        auto type = inst->getType();
        if(type->As<UniformParameterGroupType>())
        {
            // TODO: we need to be careful here, because
            // HLSL shader model 6 allows these as explicit
            // types.
            return true;
        }
        else if(type->As<TextureTypeBase>())
        {
            // GLSL doesn't allow texture/resource types to
            // be used as first-class values, so we need
            // to fold them into their use sites in all cases
            if(getTarget(ctx) == CodeGenTarget::GLSL)
                return true;
        }

        // By default we will *not* fold things into their use sites.
        return false;
    }

    bool isDerefBaseImplicit(
        EmitContext*    /*context*/,
        IRValue*        inst)
    {
        auto type = inst->getType();

        if(type->As<UniformParameterGroupType>())
        {
            // TODO: we need to be careful here, because
            // HLSL shader model 6 allows these as explicit
            // types.
            return true;
        }

        return false;
    }



    void emitIROperand(
        EmitContext*    ctx,
        IRValue*         inst)
    {
        if( shouldFoldIRInstIntoUseSites(ctx, inst) )
        {
            emit("(");
            emitIRInstExpr(ctx, inst);
            emit(")");
            return;
        }

        switch(inst->op)
        {
        case 0: // nothing yet
        default:
            emit(getIRName(inst));
            break;
        }
    }

    void emitIRArgs(
        EmitContext*    ctx,
        IRInst*         inst)
    {
        UInt argCount = inst->argCount;
        IRUse* args = inst->getArgs();

        emit("(");
        for(UInt aa = 0; aa < argCount; ++aa)
        {
            if(aa != 0) emit(", ");
            emitIROperand(ctx, args[aa].usedValue);
        }
        emit(")");
    }

    void emitIRType(
        EmitContext*    /*context*/,
        IRType*         type,
        String const&   name)
    {
        EmitType(type, name);
    }

    void emitIRType(
        EmitContext*    /*context*/,
        IRType*         type,
        Name*           name)
    {
        EmitType(type, name);
    }

    void emitIRType(
        EmitContext*    /*context*/,
        IRType*         type)
    {
        EmitType(type);
    }

    void emitIRInstResultDecl(
        EmitContext*    ctx,
        IRInst*         inst)
    {
        auto type = inst->getType();
        if(!type)
            return;

        if (type->Equals(getSession()->getVoidType()))
            return;

        emitIRType(ctx, type, getIRName(inst));
        emit(" = ");
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
                // TODO: need to read a named type
                // here...
                break;
            }
        }

        void readVal()
        {
            // TODO: handle other cases here
            readType();
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

    void emitIntrinsicCallExpr(
        EmitContext*    ctx,
        IRCall*         inst,
        IRFunc*         func)
    {
        // For a call with N arguments, the instruction will
        // have N+1 operands. We will start consuming operands
        // starting at the index 1.
        UInt operandCount = inst->getArgCount();
        UInt argCount = operandCount - 1;
        UInt operandIndex = 1;

        // Our current strategy for dealing with intrinsic
        // calls is to "un-mangle" the mangled name, in
        // order to figure out what the user was originally
        // calling. This is a bit messy, and there might
        // be better strategies (including just stuffing
        // a pointer to the original decl onto the callee).

        UnmangleContext um(func->mangledName);
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
            emit("(");
            emitIROperand(ctx, inst->getArg(operandIndex++));
            emit(")[");
            emitIROperand(ctx, inst->getArg(operandIndex++));
            emit("]");

            if(operandIndex < operandCount)
            {
                emit(" = ");
                emitIROperand(ctx, inst->getArg(operandIndex++));
            }
            return;
        }

        // The mangled function name currently records
        // the number of explicit parameters, and thus
        // doesn't include the implicit `this` parameter.
        // We can compare the argument and parameter counts
        // to figure out whether we have a member function call.
        UInt paramCount = um.readParamCount();

        if(argCount != paramCount)
        {
            // Looks like a member function call
            emit("(");
            emitIROperand(ctx, inst->getArg(operandIndex));
            emit(").");

            operandIndex++;
        }

        emit(name);
        emit("(");
        bool first = true;
        for(; operandIndex < operandCount; ++operandIndex )
        {
            if(!first) emit(", ");
            emitIROperand(ctx, inst->getArg(operandIndex));
            first = false;
        }
        emit(")");
    }

    void emitIRCallExpr(
        EmitContext*    ctx,
        IRCall*         inst)
    {
        // We want to detect any call to an intrinsic operation,
        // that we can emit it directly without mangling, etc.
        auto funcValue = inst->getArg(0);
        if(auto irFunc = asTargetIntrinsic(ctx, funcValue))
        {
            emitIntrinsicCallExpr(ctx, inst, irFunc);
        }
        else
        {
            emitIROperand(ctx, funcValue);
            emit("(");
            UInt argCount = inst->getArgCount();
            for( UInt aa = 1; aa < argCount; ++aa )
            {
                if(aa != 1) emit(", ");
                emitIROperand(ctx, inst->getArg(aa));
            }
            emit(")");
        }
    }

    void emitIRInstExpr(
        EmitContext*    ctx,
        IRValue*        value)
    {
        IRInst* inst = (IRInst*) value;
        switch(value->op)
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_boolConst:
            emitIRSimpleValue(ctx, inst);
            break;

        case kIROp_Construct:
            // Simple constructor call
            if( inst->getArgCount() == 1 && getTarget(ctx) == CodeGenTarget::HLSL)
            {
                // Need to emit as cast for HLSL
                emit("(");
                emitIRType(ctx, inst->getType());
                emit(") ");
                emitIROperand(ctx, inst->getArg(0));
            }
            else
            {
                emitIRType(ctx, inst->getType());
                emitIRArgs(ctx, inst);
            }
            break;

        case kIROp_constructVectorFromScalar:
            // Simple constructor call
            if( getTarget(ctx) == CodeGenTarget::HLSL )
            {
                emit("(");
                emitIRType(ctx, inst->getType());
                emit(")");
            }
            else
            {
                emitIRType(ctx, inst->getType());
            }
            emit("(");
            emitIROperand(ctx, inst->getArg(0));
            emit(")");
            break;

        case kIROp_FieldExtract:
            {
                // Extract field from aggregate

                IRFieldExtract* fieldExtract = (IRFieldExtract*) inst;

                emitIROperand(ctx, fieldExtract->getBase());
                emit(".");
                emit(getIRName(fieldExtract->getField()));
            }
            break;

        case kIROp_FieldAddress:
            {
                // Extract field "address" from aggregate

                IRFieldAddress* ii = (IRFieldAddress*) inst;

                if (!isDerefBaseImplicit(ctx, ii->getBase()))
                {
                    emitIROperand(ctx, ii->getBase());
                    emit(".");
                }

                emit(getIRName(ii->getField()));
            }
            break;

#define CASE(OPCODE, OP)                                \
        case OPCODE:                                    \
            emitIROperand(ctx, inst->getArg(0));    \
            emit(" " #OP " ");                          \
            emitIROperand(ctx, inst->getArg(1));    \
            break

        CASE(kIROp_Add, +);
        CASE(kIROp_Sub, -);
        CASE(kIROp_Div, /);
        CASE(kIROp_Mod, %);

        CASE(kIROp_Lsh, <<);
        CASE(kIROp_Rsh, >>);

        // TODO: Need to pull out component-wise
        // comparison cases for matrices/vectors
        CASE(kIROp_Eql, ==);
        CASE(kIROp_Neq, !=);
        CASE(kIROp_Greater, >);
        CASE(kIROp_Less, <);
        CASE(kIROp_Geq, >=);
        CASE(kIROp_Leq, <=);

        CASE(kIROp_BitAnd, &);
        CASE(kIROp_BitXor, ^);
        CASE(kIROp_BitOr, |);

        CASE(kIROp_And, &&);
        CASE(kIROp_Or, ||);

#undef CASE

        // Component-wise ultiplication needs to be special cased,
        // because GLSL uses infix `*` to express inner product
        // when working with matrices.
        case kIROp_Mul:
            // Are we targetting GLSL, and is this a matrix product?
            if(getTarget(ctx) == CodeGenTarget::GLSL
                && inst->type->As<MatrixExpressionType>())
            {
                emit("matrixCompMult(");
                emitIROperand(ctx, inst->getArg(0));
                emit(", ");
                emitIROperand(ctx, inst->getArg(1));
                emit(")");
            }
            else
            {
                // Default handling is to just rely on infix
                // `operator*`.
                emitIROperand(ctx, inst->getArg(0));
                emit(" * ");
                emitIROperand(ctx, inst->getArg(1));
            }
            break;

        case kIROp_Not:
            {
                if (inst->getType()->Equals(getSession()->getBoolType()))
                {
                    emit("!");
                }
                else
                {
                    emit("~");
                }
                emitIROperand(ctx, inst->getArg(0));
            }
            break;

        case kIROp_Neg:
            {
                emit("-");
                emitIROperand(ctx, inst->getArg(0));
            }
            break;

        case kIROp_Sample:
            emitIROperand(ctx, inst->getArg(0));
            emit(".Sample(");
            emitIROperand(ctx, inst->getArg(1));
            emit(", ");
            emitIROperand(ctx, inst->getArg(2));
            emit(")");
            break;

        case kIROp_SampleGrad:
            // argument 0 is the instruction's type
            emitIROperand(ctx, inst->getArg(0));
            emit(".SampleGrad(");
            emitIROperand(ctx, inst->getArg(1));
            emit(", ");
            emitIROperand(ctx, inst->getArg(2));
            emit(", ");
            emitIROperand(ctx, inst->getArg(3));
            emit(", ");
            emitIROperand(ctx, inst->getArg(4));
            emit(")");
            break;

        case kIROp_Load:
            // TODO: this logic will really only work for a simple variable reference...
            emitIROperand(ctx, inst->getArg(0));
            break;

        case kIROp_Store:
            // TODO: this logic will really only work for a simple variable reference...
            emitIROperand(ctx, inst->getArg(0));
            emit(" = ");
            emitIROperand(ctx, inst->getArg(1));
            break;

        case kIROp_Call:
            {
                emitIRCallExpr(ctx, (IRCall*)inst);
            }
            break;

        case kIROp_BufferLoad:
            emitIROperand(ctx, inst->getArg(0));
            emit("[");
            emitIROperand(ctx, inst->getArg(1));
            emit("]");
            break;

        case kIROp_BufferStore:
            emitIROperand(ctx, inst->getArg(0));
            emit("[");
            emitIROperand(ctx, inst->getArg(1));
            emit("] = ");
            emitIROperand(ctx, inst->getArg(2));
            break;

        case kIROp_GroupMemoryBarrierWithGroupSync:
            emit("GroupMemoryBarrierWithGroupSync()");
            break;

        case kIROp_getElement:
        case kIROp_getElementPtr:
            emitIROperand(ctx, inst->getArg(0));
            emit("[");
            emitIROperand(ctx, inst->getArg(1));
            emit("]");
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
                emitIROperand(ctx, inst->getArg(1));
                emit(" * ");
                emitIROperand(ctx, inst->getArg(0));
            }
            else
            {
                emit("mul(");
                emitIROperand(ctx, inst->getArg(0));
                emit(", ");
                emitIROperand(ctx, inst->getArg(1));
                emit(")");
            }
            break;

        case kIROp_swizzle:
            {
                auto ii = (IRSwizzle*)inst;
                emitIROperand(ctx, ii->getBase());
                emit(".");
                UInt elementCount = ii->getElementCount();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    IRValue* irElementIndex = ii->getElementIndex(ee);
                    assert(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;

                    UInt elementIndex = (UInt)irConst->u.intVal;
                    assert(elementIndex < 4);

                    char const* kComponents[] = { "x", "y", "z", "w" };
                    emit(kComponents[elementIndex]);
                }
            }
            break;

        case kIROp_specialize:
            {
                emitIROperand(ctx, inst->getArg(0));
            }
            break;

        case kIROp_Select:
            {
                emitIROperand(ctx, inst->getArg(0));
                emit(" ? ");
                emitIROperand(ctx, inst->getArg(1));
                emit(" : ");
                emitIROperand(ctx, inst->getArg(2));
            }
            break;

        default:
            emit("/* unhandled */");
            break;
        }
    }

    void emitIRInst(
        EmitContext*    ctx,
        IRInst*         inst)
    {
        if (shouldFoldIRInstIntoUseSites(ctx, inst))
        {
            return;
        }

        switch(inst->op)
        {
        default:
            emitIRInstResultDecl(ctx, inst);
            emitIRInstExpr(ctx, inst);
            emit(";\n");
            break;

        case kIROp_Var:
            {
                auto ptrType = inst->getType();
                auto valType = ((PtrType*)ptrType)->getValueType();

                auto name = getIRName(inst);
                emitIRType(ctx, valType, name);
                emit(";\n");
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
            emit("return;\n");
            break;

        case kIROp_ReturnVal:
            emit("return ");
            emitIROperand(ctx, ((IRReturnVal*) inst)->getVal());
            emit(";\n");
            break;

        case kIROp_discard:
            emit("discard;\n");
            break;

        case kIROp_swizzleSet:
            {
                auto ii = (IRSwizzleSet*)inst;
                emitIRInstResultDecl(ctx, inst);
                emitIROperand(ctx, inst->getArg(0));
                emit(";\n");
                emitIROperand(ctx, inst);
                emit(".");
                UInt elementCount = ii->getElementCount();
                for (UInt ee = 0; ee < elementCount; ++ee)
                {
                    IRValue* irElementIndex = ii->getElementIndex(ee);
                    assert(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;

                    UInt elementIndex = (UInt)irConst->u.intVal;
                    assert(elementIndex < 4);

                    char const* kComponents[] = { "x", "y", "z", "w" };
                    emit(kComponents[elementIndex]);
                }
                emit(" = ");
                emitIROperand(ctx, inst->getArg(1));
                emit(";\n");
            }
            break;
        }
    }

    void emitIRSemantics(
        EmitContext*    ctx,
        IRValue*         inst)
    {
        // Don't emit semantics if we aren't translating down to HLSL
        switch (ctx->shared->target)
        {
        case CodeGenTarget::HLSL:
            break;

        default:
            return;
        }

        if(auto layoutDecoration = inst->findDecoration<IRLayoutDecoration>())
        {
            if(auto varLayout = layoutDecoration->layout.As<VarLayout>())
            {
                if(varLayout->flags & VarLayoutFlag::HasSemantic)
                {
                    Emit(" : ");
                    emit(varLayout->semanticName);
                    if(varLayout->semanticIndex)
                    {
                        Emit(varLayout->semanticIndex);
                    }

                    return;
                }
            }
        }

        // TODO(tfoley): should we ever need to use the high-level declaration
        // for this? It seems like the wrong approach...

        auto decoration = inst->findDecoration<IRHighLevelDeclDecoration>();
        if( decoration )
        {
            EmitSemantics(decoration->decl);
        }
    }

    VarLayout* getVarLayout(
        EmitContext*    /*context*/,
        IRValue*        var)
    {
        auto decoration = var->findDecoration<IRLayoutDecoration>();
        if (!decoration)
            return nullptr;

        return (VarLayout*) decoration->layout.Ptr();
    }

    void emitIRLayoutSemantics(
        EmitContext*    ctx,
        IRValue*        inst,
        char const*     uniformSemanticSpelling = "register")
    {
        auto layout = getVarLayout(ctx, inst);
        if (layout)
        {
            emitHLSLRegisterSemantics(layout, uniformSemanticSpelling);
        }
    }

    // We want to emit a range of code in the IR, represented
    // by the blocks that are logically in the interval [begin, end)
    // which we consider as a single-entry multiple-exit region.
    //
    // Note: because there are multiple exists, control flow
    // may exit this region with operations that do *not* branch
    // to `end`, but such non-local control flow will hopefully
    // be captured.
    // 
    void emitIRStmtsForBlocks(
        EmitContext*    ctx,
        IRBlock*        begin,
        IRBlock*        end)
    {
        IRBlock* block = begin;
        while(block != end)
        {
            // Start by emitting the non-terminator instructions in the block.
            auto terminator = block->getLastInst();
            assert(isTerminatorInst(terminator));
            for (auto inst = block->getFirstInst(); inst != terminator; inst = inst->getNextInst())
            {
                emitIRInst(ctx, inst);
            }

            // Now look at the terminator instruction, which will tell us what we need to emit next.

            switch (terminator->op)
            {
            default:
                SLANG_UNEXPECTED("terminator inst");
                return;

            case kIROp_unreachable:
                return;

            case kIROp_ReturnVal:
            case kIROp_ReturnVoid:
            case kIROp_discard:
                emitIRInst(ctx, terminator);
                return;

            case kIROp_if:
                {
                    // One-sided `if` statement
                    auto t = (IRIf*)terminator;

                    auto trueBlock = t->getTrueBlock();
                    auto afterBlock = t->getAfterBlock();

                    emit("if(");
                    emitIROperand(ctx, t->getCondition());
                    emit(")\n{\n");
                    emitIRStmtsForBlocks(
                        ctx,
                        trueBlock,
                        afterBlock);
                    emit("}\n");

                    // Continue with the block after the `if`
                    block = afterBlock;
                }
                break;

            case kIROp_ifElse:
                {
                    // Two-sided `if` statement
                    auto t = (IRIfElse*)terminator;

                    auto trueBlock = t->getTrueBlock();
                    auto falseBlock = t->getFalseBlock();
                    auto afterBlock = t->getAfterBlock();

                    emit("if(");
                    emitIROperand(ctx, t->getCondition());
                    emit(")\n{\n");
                    emitIRStmtsForBlocks(
                        ctx,
                        trueBlock,
                        afterBlock);
                    emit("}\nelse\n{\n");
                    emitIRStmtsForBlocks(
                        ctx,
                        falseBlock,
                        afterBlock);
                    emit("}\n");

                    // Continue with the block after the `if`
                    block = afterBlock;
                }
                break;

            case kIROp_loop:
                {
                    // Header for a `while` or `for` loop
                    auto t = (IRLoop*)terminator;

                    auto targetBlock = t->getTargetBlock();
                    auto breakBlock = t->getBreakBlock();
                    auto continueBlock = t->getContinueBlock();

                    if (auto loopControlDecoration = t->findDecoration<IRLoopControlDecoration>())
                    {
                        switch (loopControlDecoration->mode)
                        {
                        case kIRLoopControl_Unroll:
                            emit("[unroll]\n");
                            break;

                        default:
                            break;
                        }
                    }

                    // The challenging case for a loop is when
                    // there is a `continue` block that we
                    // need to deal with.
                    //
                    if (continueBlock == targetBlock)
                    {
                        // There is no continue block, so
                        // we only need to emit an endless
                        // loop and then manually `break`
                        // out of it in the right place(s)
                        emit("for(;;)\n{\n");

                        emitIRStmtsForBlocks(
                            ctx,
                            targetBlock,
                            nullptr);

                        emit("}\n");
                    }
                    else
                    {
                        // Okay, we've got a `continue` block,
                        // which means we really want to emit
                        // something akin to:
                        //
                        //     for(;; <continueBlock>) { <bodyBlock> }
                        //
                        // In principle this isn't so bad, since the
                        // first case is just interVal [`continueBlock`, `targetBlock`)
                        // and the latter is the interval [`targetBlock`, `continueBlock`).
                        //
                        // The challenge of course is that a `for` statement
                        // only supports *expressions* in the continue part,
                        // and we might have expanded things into multiple
                        // instructions (especially if we inlined or desugared anything).
                        //
                        // There are a variety of ways we can support lowering this,
                        // but for now we are going to do something expedient
                        // that mimics what `fxc` seems to do:
                        //
                        // - Output loop body as `for(;;) { <bodyBlock> <continueBlock> }`
                        // - At any `continue` site, output `{ <continueBlock>; continue; }`
                        //
                        // This isn't ideal because it leads to code duplication, but
                        // it matches what `fxc` does so hopefully it will be the
                        // best option for our tests.
                        //

                        emit("for(;;)\n{\n");

                        // Register information so that `continue` sites
                        // can do the right thing:
                        ctx->shared->irMapContinueTargetToLoopHead.Add(continueBlock, targetBlock);


                        emitIRStmtsForBlocks(
                            ctx,
                            targetBlock,
                            nullptr);

                        emit("}\n");

                    }

                    // Continue with the block after the loop
                    block = breakBlock;
                }
                break;

            case kIROp_break:
                emit("break;\n");
                return;

            case kIROp_continue:
                // With out current strategy for outputting loops,
                // just outputting an AST-level `continue` here won't
                // actually execute the statements in the continue block.
                //
                // Instead, we have to manually output those statements
                // directly here, and *then* do an AST-level `continue`.
                //
                // This leads to code duplication when we have multiple
                // `continue` sites in the original program, but it avoids
                // introducing additional temporaries for control flow.
                {
                    auto continueInst = (IRContinue*) terminator;
                    auto targetBlock = continueInst->getTargetBlock();
                    IRBlock* loopHead = nullptr;
                    ctx->shared->irMapContinueTargetToLoopHead.TryGetValue(targetBlock, loopHead);
                    SLANG_ASSERT(loopHead);
                    emitIRStmtsForBlocks(
                        ctx,
                        targetBlock,
                        loopHead);
                    emit("continue;\n");
                }
                return;

            case kIROp_loopTest:
                {
                    // Loop condition being tested
                    auto t = (IRLoopTest*)terminator;

                    auto afterBlock = t->getTrueBlock();

                    emit("if(");
                    emitIROperand(ctx, t->getCondition());
                    emit(")\n{} else break;\n");

                    // Continue with the block after the test
                    block = afterBlock;
                }
                break;

            case kIROp_unconditionalBranch:
                {
                    // Unconditional branch as part of normal
                    // control flow. This is either a forward
                    // edge to the "next" block in an ordinary
                    // block, or a backward edge to the top
                    // of a loop.
                    auto t = (IRUnconditionalBranch*)terminator;
                    block = t->getTargetBlock();
                }
                break;

            case kIROp_conditionalBranch:
                // Note: We currently do not generate any plain
                // `conditionalBranch` instructions when lowering
                // to IR, because these would not have the annotations
                // needed to be able to emit high-level control
                // flow from them.
                SLANG_UNEXPECTED("terminator inst");
                return;


            case kIROp_switch:
                {
                    // A `switch` instruction will always translate
                    // to a `switch` statement, but we need to
                    // take some care to emit the `case`s in ways
                    // that avoid code duplication.

                    // TODO: Eventually, the "right" way to handle `switch`
                    // statements while being more robust about Duff's Device, etc.
                    // would be to register each of the case labels in a lookup
                    // table, and then walk the blocks in the region between
                    // the `switch` and the `break` and then whenever we see a block
                    // that matches one of the registered labels, emit the appropriate
                    // `case ...:` or `default:` label.

                    auto t = (IRSwitch*) terminator;

                    // Extract the fixed arguments.
                    auto conditionVal = t->getCondition();
                    auto breakLabel = t->getBreakLabel();
                    auto defaultLabel = t->getDefaultLabel();

                    // We need to track whether we've dealt with
                    // the `default` case already.
                    bool defaultLabelHandled = false;

                    // If the `default` case just branches to
                    // the join point, then we don't need to
                    // do anything with it.
                    if(defaultLabel == breakLabel)
                        defaultLabelHandled = true;

                    // Emit the start of our statement.
                    emit("switch(");
                    emitIROperand(ctx, conditionVal);
                    emit(")\n{\n");

                    // Now iterate over the `case`s of the branch
                    UInt caseIndex = 0;
                    UInt caseCount = t->getCaseCount();
                    while(caseIndex < caseCount)
                    {
                        // We are going to extract one case here,
                        // but we might need to fold additional
                        // cases into it, if they share the
                        // same label.
                        //
                        // Note: this makes assumptions that the
                        // IR code generator orders cases such
                        // that: (1) cases with the same label
                        // are consecutive, and (2) any case
                        // that "falls through" to another must
                        // come right before it in the list.
                        auto caseVal = t->getCaseValue(caseIndex);
                        auto caseLabel = t->getCaseLabel(caseIndex);
                        caseIndex++;

                        // Emit the `case ...:` for this case, and any
                        // others that share the same label
                        for(;;)
                        {
                            emit("case ");
                            emitIROperand(ctx, caseVal);
                            emit(":\n");

                            if(caseIndex >= caseCount)
                                break;

                            auto nextCaseLabel = t->getCaseLabel(caseIndex);
                            if(nextCaseLabel != caseLabel)
                                break;

                            caseVal = t->getCaseValue(caseIndex);
                            caseIndex++;
                        }

                        // The label for the current `case` might also
                        // be the label used by the `default` case, so
                        // check for that here.
                        if(caseLabel == defaultLabel)
                        {
                            emit("default:\n");
                            defaultLabelHandled = true;
                        }

                        // Now we need to emit the statements that make
                        // up this case. The 99% case will be that it
                        // will terminate with a `break` (or a `return`,
                        // `continue`, etc.) and so we can pass in
                        // `nullptr` for the ending block.
                        IRBlock* caseEndLabel = nullptr;

                        // However, there is also the possibility that
                        // this case will fall through to the next, and
                        // so we need to prepare for that possibility here.
                        //
                        // If there is a next case, then we will set its
                        // label up as the "end" label when emitting
                        // the statements inside the block.
                        if(caseIndex < caseCount)
                        {
                            caseEndLabel = t->getCaseLabel(caseIndex);
                        }

                        // Now emit the statements for this case.
                        emit("{\n");
                        emitIRStmtsForBlocks(ctx, caseLabel, caseEndLabel);
                        emit("}\n");
                    }

                    // If we've gone through all the cases and haven't
                    // managed to encounter the `default:` label,
                    // then assume it is a distinct case and handle it here.
                    if(!defaultLabelHandled)
                    {
                        emit("default:\n");
                        emit("{\n");
                        emitIRStmtsForBlocks(ctx, defaultLabel, breakLabel);
                        emit("break;\n");
                        emit("}\n");
                    }

                    emit("}\n");
                    block = breakLabel;

                }
                break;
            }

            // If we reach this point, then we've emitted
            // one block, and we have a new block where
            // control flow continues.
            //
            // We need to handle a special case here,
            // when control flow jumps back to the
            // starting block of the range we were
            // asked to work with:
            if (block == begin) return;
        }
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

    void emitIRSimpleFunc(
        EmitContext*    ctx,
        IRFunc*         func)
    {
        auto resultType = func->getResultType();

        // Deal with decorations that need
        // to be emitted as attributes
        auto entryPointLayout = asEntryPoint(func);
        if (entryPointLayout)
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

                    emit("[numthreads(");
                    for (int ii = 0; ii < 3; ++ii)
                    {
                        if (ii != 0) emit(", ");
                        Emit(sizeAlongAxis[ii]);
                    }
                    emit(")]\n");
                }
                break;

            // TODO: There are other stages that will need this kind of handling.
            default:
                break;
            }
        }

        auto name = getIRFuncName(func);

        emitIRType(ctx, resultType, name);

        emit("(");
        auto firstParam = func->getFirstParam();
        for( auto pp = firstParam; pp; pp = pp->getNextParam() )
        {
            if(pp != firstParam)
                emit(", ");

            auto paramName = getIRName(pp);
            auto paramType = pp->getType();
            emitIRParamType(ctx, paramType, paramName);

            emitIRSemantics(ctx, pp);
        }
        emit(")");


        emitIRSemantics(ctx, func);

        // TODO: encode declaration vs. definition
        if(isDefinition(func))
        {
            emit("\n{\n");

            // Need to emit the operations in the blocks of the function

            emitIRStmtsForBlocks(ctx, func->getFirstBlock(), nullptr);

            emit("}\n");
        }
        else
        {
            emit(";\n");
        }
    }

    void emitIRParamType(
        EmitContext*    ctx,
        Type*           type,
        String const&   name)
    {
        // An `out` or `inout` parameter will have been
        // encoded as a parameter of pointer type, so
        // we need to decode that here.
        //
        if( auto outType = type->As<OutType>() )
        {
            emit("out ");
            type = outType->getValueType();
        }
        else if( auto inOutType = type->As<InOutType>() )
        {
            emit("inout ");
            type = inOutType->getValueType();
        }

        emitIRType(ctx, type, name);
    }

    void emitIRFuncDecl(
        EmitContext*    ctx,
        IRFunc*         func)
    {
        // We don't want to declare generic functions,
        // because none of our targets actually support them.
        if(func->genericDecl)
            return;

        // We also don't want to emit declarations for operations
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

        auto funcType = func->getType();
        auto resultType = func->getResultType();

        auto name = getIRFuncName(func);

        emitIRType(ctx, resultType, name);

        emit("(");
        auto paramCount = funcType->getParamCount();
        for(UInt pp = 0; pp < paramCount; ++pp)
        {
            if(pp != 0)
                emit(", ");

            String paramName;
            paramName.append("_");
            paramName.append(pp);
            auto paramType = funcType->getParamType(pp);

            emitIRParamType(ctx, paramType, paramName);
        }
        emit(");\n");
    }

    EntryPointLayout* getEntryPointLayout(
        EmitContext*    /*context*/,
        IRFunc*         func)
    {
        if( auto layoutDecoration = func->findDecoration<IRLayoutDecoration>() )
        {
            return layoutDecoration->layout.As<EntryPointLayout>();
        }
        return nullptr;
    }

    EntryPointLayout* asEntryPoint(IRFunc* func)
    {
        if (auto layoutDecoration = func->findDecoration<IRLayoutDecoration>())
        {
            if (auto entryPointLayout = layoutDecoration->layout.As<EntryPointLayout>())
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
        EmitContext*    /*ctxt*/,
        IRFunc*         func)
    {
        // For now we do this in an overly simplistic
        // fashion: we say that *any* function declaration
        // (rather then definition) must be an intrinsic:
        return !isDefinition(func);
    }

    // Check whether a given value names a target intrinsic,
    // and return the IR function representing the instrinsic
    // if it does.
    IRFunc* asTargetIntrinsic(
        EmitContext*    ctxt,
        IRValue*        value)
    {
        if(!value)
            return nullptr;

        if(value->op == kIROp_specialize)
        {
            value = ((IRSpecialize*) value)->genericVal.usedValue;
        }

        if(value->op != kIROp_Func)
            return nullptr;

        IRFunc* func = (IRFunc*) value;
        if(!isTargetIntrinsic(ctxt, func))
            return nullptr;

        return func;
    }

    void emitIRFunc(
        EmitContext*    ctx,
        IRFunc*         func)
    {
        if(func->genericDecl)
        {
            Emit("/* ");
            emitIRFuncDecl(ctx, func);
            Emit(" */\n");
            return;
        }

        if(!isDefinition(func))
        {
            // This is just a function declaration,
            // and so we want to emit it as such.
            // (Or maybe not emit it at all).

            // We do not emit the declaration for
            // functions that appear to be intrinsics/builtins
            // in the target langugae.
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

#if 0
    void emitIRStruct(
        EmitContext*    context,
        IRStructDecl*   structType)
    {
        emit("struct ");
        emit(getName(structType));
        emit("\n{\n");

        for(auto ff = structType->getFirstField(); ff; ff = ff->getNextField())
        {
            auto fieldType = ff->getFieldType();
            emitIRType(context, fieldType, getName(ff));

            emitIRSemantics(context, ff);

            emit(";\n");
        }
        emit("};\n");
    }
#endif

    void emitIRVarModifiers(
        EmitContext*    ctx,
        VarLayout*      layout)
    {
        if (!layout)
            return;

        auto target = ctx->shared->target;

        // We need to handle the case where the variable has
        // a matrix type, and has been given a non-standard
        // layout attribute (for HLSL, `row_major` is the
        // non-standard layout).
        //
        if (auto matrixTypeLayout = layout->typeLayout.As<MatrixTypeLayout>())
        {
            switch (target)
            {
            case CodeGenTarget::HLSL:
                switch (matrixTypeLayout->mode)
                {
                case kMatrixLayoutMode_ColumnMajor:
                    if(target == CodeGenTarget::GLSL)
                    emit("column_major ");
                    break;

                case kMatrixLayoutMode_RowMajor:
                    emit("row_major ");
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
                    if(target == CodeGenTarget::GLSL)
                    emit("layout(row_major)\n");
                    break;

                case kMatrixLayoutMode_RowMajor:
                    emit("layout(column_major)\n");
                    break;
                }
                break;

            default:
                break;
            }
            
        }

        if (ctx->shared->target == CodeGenTarget::GLSL)
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
                    emit("uniform ");
                    break;

                case LayoutResourceKind::VertexInput:
                    emit("in ");
                    break;

                case LayoutResourceKind::FragmentOutput:
                    emit("out ");
                    break;

                default:
                    continue;
                }

                break;
            }
        }
    }

    void emitHLSLParameterBlock(
        EmitContext*        ctx,
        IRGlobalVar*        varDecl,
        ParameterBlockType* type)
    {
        emit("cbuffer ");

        // Generate a dummy name for the block
        emit("_S");
        Emit(ctx->shared->uniqueIDCounter++);

        auto varLayout = getVarLayout(ctx, varDecl);
        assert(varLayout);

        EmitVarChain blockChain(varLayout);

        EmitVarChain containerChain = blockChain;
        EmitVarChain elementChain = blockChain;

        auto typeLayout = varLayout->typeLayout;
        if( auto parameterGroupTypeLayout = typeLayout.As<ParameterGroupTypeLayout>() )
        {
            containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
            elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

            typeLayout = parameterGroupTypeLayout->elementVarLayout->getTypeLayout();
        }

        emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

        emit("\n{\n");

        auto elementType = type->getElementType();


        emitIRType(ctx, elementType, getIRName(varDecl));

        emitHLSLParameterGroupFieldLayoutSemantics(&elementChain);
        emit(";\n");

        emit("}\n");
    }

    void emitHLSLParameterGroup(
        EmitContext*                ctx,
        IRGlobalVar*                varDecl,
        UniformParameterGroupType*  type)
    {
        if(auto parameterBlockType = type->As<ParameterBlockType>())
        {
            emitHLSLParameterBlock(ctx, varDecl, parameterBlockType);
            return;
        }

        emit("cbuffer ");
        emit(getIRName(varDecl));

        auto varLayout = getVarLayout(ctx, varDecl);
        assert(varLayout);

        EmitVarChain blockChain(varLayout);

        EmitVarChain containerChain = blockChain;
        EmitVarChain elementChain = blockChain;

        auto typeLayout = varLayout->typeLayout;
        if( auto parameterGroupTypeLayout = typeLayout.As<ParameterGroupTypeLayout>() )
        {
            containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
            elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

            typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
        }

        emitHLSLRegisterSemantic(LayoutResourceKind::ConstantBuffer, &containerChain);

        emit("\n{\n");

        auto elementType = type->getElementType();


        if(auto declRefType = elementType->As<DeclRefType>())
        {
            if(auto structDeclRef = declRefType->declRef.As<StructDecl>())
            {
                auto structTypeLayout = typeLayout.As<StructTypeLayout>();
                assert(structTypeLayout);

                UInt fieldIndex = 0;
                for(auto ff : GetFields(structDeclRef))
                {
                    // TODO: need a plan to deal with the case where the IR-level
                    // `struct` type might not match the high-level type, so that
                    // the numbering of fields is different.
                    //
                    // The right plan is probably to require that the lowering pass
                    // create a fresh layout for any type/variable that it splits
                    // in this fashion, so that the layout information it attaches
                    // can always be assumed to apply to the actual instruciton.
                    //

                    auto fieldLayout = structTypeLayout->fields[fieldIndex++];

                    auto fieldType = GetType(ff);
                    if(fieldType->Equals(getSession()->getVoidType()))
                        continue;

                    emitIRVarModifiers(ctx, fieldLayout);

                    emitIRType(ctx, fieldType, getIRName(ff));

                    emitHLSLParameterGroupFieldLayoutSemantics(fieldLayout, &elementChain);

                    emit(";\n");
                }
            }
        }
        else
        {
            emit("/* unexpected */");
        }

        emit("}\n");
    }

    void emitGLSLParameterGroup(
        EmitContext*                ctx,
        IRGlobalVar*                varDecl,
        UniformParameterGroupType*  type)
    {
        auto varLayout = getVarLayout(ctx, varDecl);
        assert(varLayout);

        EmitVarChain blockChain(varLayout);

        EmitVarChain containerChain = blockChain;
        EmitVarChain elementChain = blockChain;

        auto typeLayout = varLayout->typeLayout;
        if( auto parameterGroupTypeLayout = typeLayout.As<ParameterGroupTypeLayout>() )
        {
            containerChain = EmitVarChain(parameterGroupTypeLayout->containerVarLayout, &blockChain);
            elementChain = EmitVarChain(parameterGroupTypeLayout->elementVarLayout, &blockChain);

            typeLayout = parameterGroupTypeLayout->elementVarLayout->typeLayout;
        }

        emitGLSLLayoutQualifier(LayoutResourceKind::DescriptorTableSlot, &containerChain);

        if(type->As<GLSLShaderStorageBufferType>())
        {
            emit("layout(std430) buffer ");
        }
        // TODO: what to do with HLSL `tbuffer` style buffers?
        else
        {
            emit("layout(std140) uniform ");
        }

        emit(getIRName(varDecl));

        emit("\n{\n");

        auto elementType = type->getElementType();

        if(auto declRefType = elementType->As<DeclRefType>())
        {
            if(auto structDeclRef = declRefType->declRef.As<StructDecl>())
            {
                auto structTypeLayout = typeLayout.As<StructTypeLayout>();
                assert(structTypeLayout);

                UInt fieldIndex = 0;
                for(auto ff : GetFields(structDeclRef))
                {
                    // TODO: need a plan to deal with the case where the IR-level
                    // `struct` type might not match the high-level type, so that
                    // the numbering of fields is different.
                    //
                    // The right plan is probably to require that the lowering pass
                    // create a fresh layout for any type/variable that it splits
                    // in this fashion, so that the layout information it attaches
                    // can always be assumed to apply to the actual instruciton.
                    //

                    auto fieldLayout = structTypeLayout->fields[fieldIndex++];

                    auto fieldType = GetType(ff);
                    if(fieldType->Equals(getSession()->getVoidType()))
                        continue;

                    emitIRVarModifiers(ctx, fieldLayout);

                    emitIRType(ctx, fieldType, getIRName(ff));

//                    emitHLSLParameterGroupFieldLayoutSemantics(layout, fieldLayout);

                    emit(";\n");
                }
            }
        }
        else
        {
            emit("/* unexpected */");
        }

        // TODO: we should consider always giving parameter blocks
        // names when outputting GLSL, since that shouldn't affect
        // the semantics of things, and will reduce the risk of
        // collisions in the global namespace...

        emit("};\n");
    }

    void emitIRParameterGroup(
        EmitContext*                ctx,
        IRGlobalVar*                varDecl,
        UniformParameterGroupType*  type)
    {
        switch (ctx->shared->target)
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
        EmitContext*    ctx,
        IRVar*          varDecl)
    {
        auto allocatedType = varDecl->getType();
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
        
        emitIRVarModifiers(ctx, layout);

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

        emitIRType(ctx, varType, getIRName(varDecl));

        emitIRSemantics(ctx, varDecl);

        emitIRLayoutSemantics(ctx, varDecl);

        emit(";\n");
    }

    void emitIRGlobalVar(
        EmitContext*    ctx,
        IRGlobalVar*    varDecl)
    {
        auto allocatedType = varDecl->getType();
        auto varType = allocatedType->getValueType();

        String initFuncName;
        if (varDecl->firstBlock)
        {
            // A global variable with code means it has an initializer
            // associated with it. Eventually we'd like to emit that
            // initializer directly as an expression here, but for
            // now we'll emit it as a separate function.

            initFuncName = getIRName(varDecl);
            initFuncName.append("_init");
            emitIRType(ctx, varType, initFuncName);
            Emit("()\n{\n");
            emitIRStmtsForBlocks(ctx, varDecl->firstBlock, nullptr);
            Emit("}\n");
        }


        if (auto paramBlockType = varType->As<UniformParameterGroupType>())
        {
            emitIRParameterGroup(
                ctx,
                varDecl,
                paramBlockType);
            return;
        }

        // Need to emit appropriate modifiers here.

        auto layout = getVarLayout(ctx, varDecl);

        if (!layout)
        {
            // A global variable without a layout is just an
            // ordinary global variable, and may need special
            // modifiers to indicate it as such.
            switch (getTarget(ctx))
            {
            case CodeGenTarget::HLSL:
                // HLSL requires the `static` modifier on any
                // global variables; otherwise they are assumed
                // to be uniforms.
                Emit("static ");
                break;

            default:
                break;
            }
        }

        emitIRVarModifiers(ctx, layout);

        emitIRType(ctx, varType, getIRName(varDecl));

        emitIRSemantics(ctx, varDecl);

        emitIRLayoutSemantics(ctx, varDecl);

        if (varDecl->firstBlock)
        {
            Emit(" = ");
            emit(initFuncName);
            Emit("()");
        }

        emit(";\n");
    }

    void emitIRGlobalInst(
        EmitContext*    ctx,
        IRGlobalValue*  inst)
    {
        // TODO: need to be able to `switch` on the IR opcode here,
        // so there is some work to be done.
        switch(inst->op)
        {
        case kIROp_Func:
            emitIRFunc(ctx, (IRFunc*) inst);
            break;

        case kIROp_global_var:
            emitIRGlobalVar(ctx, (IRGlobalVar*) inst);
            break;

        case kIROp_Var:
            emitIRVar(ctx, (IRVar*) inst);
            break;

        default:
            break;
        }
    }

    void ensureStructDecl(
        EmitContext*        ctx,
        DeclRef<StructDecl> declRef)
    {
        auto mangledName = getMangledName(declRef);
        if(ctx->shared->irDeclsVisited.Contains(mangledName))
            return;

        ctx->shared->irDeclsVisited.Add(mangledName);

        // First emit any types used by fields of this type
        for( auto ff : GetFields(declRef) )
        {
            if(ff.getDecl()->HasModifier<HLSLStaticModifier>())
                continue;

            auto fieldType = GetType(ff);
            emitIRUsedType(ctx, fieldType);
        }

        Emit("struct ");
        EmitDeclRef(declRef);
        Emit("\n{\n");
        for( auto ff : GetFields(declRef) )
        {
            if(ff.getDecl()->HasModifier<HLSLStaticModifier>())
                continue;

            auto fieldType = GetType(ff);

            // Skip `void` fields that might have been created by legalization.
            if(fieldType->Equals(getSession()->getVoidType()))
                continue;

            emitIRType(ctx, fieldType, getIRName(ff));

            EmitSemantics(ff.getDecl());

            emit(";\n");
        }
        Emit("};\n");
    }

    void emitIRUsedDeclRef(
        EmitContext*    ctx,
        DeclRef<Decl>   declRef)
    {
        auto decl = declRef.getDecl();

        if(decl->HasModifier<BuiltinTypeModifier>()
            || decl->HasModifier<MagicTypeModifier>())
        {
            return;
        }

        if( auto structDeclRef = declRef.As<StructDecl>() )
        {
            //
            ensureStructDecl(ctx, structDeclRef);
        }
    }

    // A type is going to be used by the IR, so
    // make sure that we have emitted whatever
    // it needs.
    void emitIRUsedType(
        EmitContext*    ctx,
        Type*           type)
    {
        if(type->As<BasicExpressionType>())
        {}
        else if(type->As<VectorExpressionType>())
        {}
        else if(type->As<MatrixExpressionType>())
        {}
        else if(auto arrayType = type->As<ArrayExpressionType>())
        {
            emitIRUsedType(ctx, arrayType->baseType);
        }
        else if( auto textureType = type->As<TextureTypeBase>() )
        {
            emitIRUsedType(ctx, textureType->elementType);
        }
        else if( auto genericType = type->As<BuiltinGenericType>() )
        {
            emitIRUsedType(ctx, genericType->elementType);
        }
        else if( auto ptrType = type->As<PtrTypeBase>() )
        {
            emitIRUsedType(ctx, ptrType->getValueType());
        }
        else if(type->As<SamplerStateType>() )
        {
        }
        else if( auto declRefType = type->As<DeclRefType>() )
        {
            auto declRef = declRefType->declRef;
            emitIRUsedDeclRef(ctx, declRef);
        }
        else
        {}
    }

    void emitIRUsedTypesForValue(
        EmitContext*    ctx,
        IRValue*        value)
    {
        if(!value) return;
        switch( value->op )
        {
        case kIROp_Func:
            {
                auto irFunc = (IRFunc*) value;
                emitIRUsedType(ctx, irFunc->getResultType());
                for( auto bb = irFunc->getFirstBlock(); bb; bb = bb->getNextBlock() )
                {
                    for( auto pp = bb->getFirstParam(); pp; pp = pp->getNextParam() )
                    {
                        emitIRUsedTypesForValue(ctx, pp);
                    }

                    for( auto ii = bb->getFirstInst(); ii; ii = ii->getNextInst() )
                    {
                        emitIRUsedTypesForValue(ctx, ii);
                    }
                }
            }
            break;

        default:
            {
                emitIRUsedType(ctx, value->type);
            }
            break;
        }
    }

    void emitIRUsedTypesForModule(
        EmitContext*    ctx,
        IRModule*       module)
    {
        for( auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue() )
        {
            emitIRUsedTypesForValue(ctx, gv);
        }
    }

    void emitIRModule(
        EmitContext*    ctx,
        IRModule*       module)
    {
        emitIRUsedTypesForModule(ctx, module);

        // Before we emit code, we need to forward-declare
        // all of our functions so that we don't have to
        // sort them by dependencies.
        for( auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue() )
        {
            if(gv->op != kIROp_Func)
                continue;

            auto func = (IRFunc*) gv;
            emitIRFuncDecl(ctx, func);
        }



        for( auto gv = module->getFirstGlobalValue(); gv; gv = gv->getNextValue() )
        {
            emitIRGlobalInst(ctx, gv);
        }
    }


};

//


//

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

// Given a layout computed for a whole program, find
// the corresponding layout to use when looking up
// variables at the global scope.
//
// It might be that the global scope was logically
// mapped to a constant buffer, so that we need
// to "unwrap" that declaration to get at the
// actual struct type inside.
StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout)
{
    auto globalScopeLayout = programLayout->globalScopeLayout->typeLayout;
    if( auto gs = globalScopeLayout.As<StructTypeLayout>() )
    {
        return gs.Ptr();
    }
    else if( auto globalConstantBufferLayout = globalScopeLayout.As<ParameterGroupTypeLayout>() )
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

        auto elementTypeLayout = globalConstantBufferLayout->offsetElementTypeLayout;
        auto elementTypeStructLayout = elementTypeLayout.As<StructTypeLayout>();

        // We expect all constant buffers to contain `struct` types for now
        SLANG_RELEASE_ASSERT(elementTypeStructLayout);

        return elementTypeStructLayout.Ptr();
    }
    else
    {
        SLANG_UNEXPECTED("uhandled global-scope binding layout");
        return nullptr;
    }
}

void legalizeTypes(
    TypeLegalizationContext*    context,
    IRModule*                   module);

String emitEntryPoint(
    EntryPointRequest*  entryPoint,
    ProgramLayout*      programLayout,
    CodeGenTarget       target,
    TargetRequest*      targetRequest)
{
    auto translationUnit = entryPoint->getTranslationUnit();
   
    SharedEmitContext sharedContext;
    sharedContext.target = target;
    sharedContext.finalTarget = targetRequest->target;
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
    StructTypeLayout* globalStructLayout = getGlobalStructLayout(programLayout);
    sharedContext.globalStructLayout = globalStructLayout;

    auto translationUnitSyntax = translationUnit->SyntaxNode.Ptr();

    EmitContext context;
    context.shared = &sharedContext;

    EmitVisitor visitor(&context);

    // Depending on how the compiler was invoked, we may need to perform
    // some amount of preocessing on the code before we can emit it.
    //
    // We try to partition the cases we need to handle into a few broad
    // categories, each of which is reflected as a different code path
    // below:
    //
    // 1. "Full rewriter" mode, where the user provides HLSL/GLSL, opts
    //    out of semantic checking, and doesn't make use of any Slang
    //    code via `import`.
    //
    // 2. "Partial rewriter" modes, where the user starts with HLSL/GLSL
    //    and opts out of checking for that code, but also imports some
    //    Slang code which may need cross-compilation. They may also
    //    need us to rewrite the AST for some of their HLSL/GLSL function
    //    bodies to make things work. This actually has two main sub-modes:
    //
    //    a) "Without IR." If the user doesn't opt into using the IR, then
    //    the imported Slang code gets translated to the target languge
    //    via the same AST-to-AST pass that legalized the user's code. This
    //    mode will eventually go away, but it is the main one used right now.
    //
    //    b) "With IR." If the user opts into using the IR, then we need to
    //    apply the AST-to-AST pass to their HLSL/GLSL code, but *also* use
    //    the IR to compile everything else.
    //
    // 3. "Full IR" mode, where we can assume all the input code is in Slang
    //    (or the subset of HLSL we understand) that has undergone full
    //    semantic checking, and the user has opted into using the IR.
    //
    // We'll try to detect the cases here, starting with case (1):
    //
    if ((translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING)
        && translationUnit->compileRequest->loadedModulesList.Count() == 0)
    {
        // The user has opted out of semantic checking for their own code
        // (in the "main" module), and also hasn't `import`ed any Slang
        // modules that would require cross-compilation.
        //
        // Our goal in this mode is to print out the AST we parsed and
        // hopefully reproduce something as close to the original as possible.
        //
        // The only deviation we *want* from the original code is that we will
        // add new parameter binding annotations.

        sharedContext.program = translationUnitSyntax;
        visitor.EmitDeclsInContainerUsingLayout(
            translationUnitSyntax,
            globalStructLayout);
    }
    //
    // Next we will check for case (2a):
    else if (!(translationUnit->compileRequest->compileFlags & SLANG_COMPILE_FLAG_USE_IR))
    {
        TypeLegalizationContext typeLegalizationContext;
        typeLegalizationContext.session = entryPoint->compileRequest->mSession;

        // This case means the user has opted out of using the IR (so we can't use the
        // cases below), but they either turned on semantic checking *or* imported some
        // Slang code, so they can't use the case above.
        //
        // Note: This case should go away completely once the IR is able to be relied
        // upon for all cross-compilation scenarios.

        // We will apply our AST-to-AST legalization pass before we emit
        // any code, and we will emit code for the AST that comes out
        // of this pass instead of the original.

        // We perform legalization of the program before emitting *anything*,
        // because the lowering process might change how we emit some
        // boilerplate at the start of the ouput for GLSL (e.g., what
        // version we require).

        List<Decl*> astDecls;
        findDeclsUsedByASTEntryPoint(
            entryPoint,
            target,
            nullptr,
            astDecls);

        auto lowered = lowerEntryPoint(
            entryPoint,
            programLayout,
            target,
            &sharedContext.extensionUsageTracker,
            nullptr,
            &typeLegalizationContext,
            astDecls);
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
    //
    // The remaining cases all require the use of our IR, and so there
    // are certain steps that need to be shared.
    else
    {
        TypeLegalizationContext typeLegalizationContext;
        typeLegalizationContext.session = entryPoint->compileRequest->mSession;

        // We are going to create a fresh IR module that we will use to
        // clone any code needed by the user's entry point.
        IRSpecializationState* irSpecializationState = createIRSpecializationState(
            entryPoint,
            programLayout,
            target,
            targetRequest);
        IRModule* irModule = getIRModule(irSpecializationState);

        typeLegalizationContext.irModule = irModule;

        List<Decl*> astDecls;
        if(translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING)
        {
            // We are in case (2b), where the main module is in unchecked
            // HLSL/GLSL that we need to "rewrite," and any library code
            // is in Slang that will need to be cross-compiled via the IR.

            // We first need to walk the AST part of the code to look
            // for any places where it references declarations that
            // are implemented in the IR, so that we can be sure to
            // generate suitable IR code for them.

            findDeclsUsedByASTEntryPoint(
                entryPoint,
                target,
                irSpecializationState,
                astDecls);
        }
        else
        {
            // We are in case (3), where all of the code is in Slang, and
            // has already been lowered to IR as part of the front-end
            // compilation work. We thus start by cloning any code needed
            // by the entry point over to our fresh IR module.

            specializeIRForEntryPoint(
                irSpecializationState,
                entryPoint);
        }

        // If the user specified the flag that they want us to dump
        // IR, then do it here, for the target-specific, but
        // un-specialized IR.
        if (translationUnit->compileRequest->shouldDumpIR)
        {
            dumpIR(irModule);
        }

        // Next, we need to ensure that the code we emit for
        // the target doesn't contain any operations that would
        // be illegal on the target platform. For example,
        // none of our target supports generics, or interfaces,
        // so we need to specialize those away.
        //
        specializeGenerics(irModule);

        // Debugging code for IR transformations...
#if 0
        fprintf(stderr, "### SPECIALIZED:\n");
        dumpIR(lowered);
        fprintf(stderr, "###\n");
#endif

        // After we've fully specialized all generics, and
        // "devirtualized" all the calls through interfaces,
        // we need to ensure that the code only uses types
        // that are legal on the chosen target.
        // 
        legalizeTypes(
            &typeLegalizationContext,
            irModule);

        //  Debugging output of legalization
#if 0
        fprintf(stderr, "### LEGALIZED:\n");
        dumpIR(lowered);
        fprintf(stderr, "###\n");
#endif

        LoweredEntryPoint lowered;
        if(translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING)
        {
            // In the (2b) case, once we have legalized the IR code,
            // we now need to go in and legalize the AST code.
            // This order is important because when referring to a variable
            // that is defined in the IR, we need to legalize it first (which
            // might split it into many decls) before we can legalize an AST
            // expression that references that decl (which will also need
            // to get split).
            //
            // We don't have to worry about references in the other direction;
            // we don't allow the user to define something in unchecked AST
            // code and then use it from the IR shader library.

            lowered = lowerEntryPoint(
                entryPoint,
                programLayout,
                target,
                &sharedContext.extensionUsageTracker,
                irSpecializationState,
                &typeLegalizationContext,
                astDecls);
        }

        // When emitting IR-based declarations, we wnat to
        // track which decls have already been lowered.
        sharedContext.irDeclSetForAST = &lowered.irDecls;

        // After all of the required optimization and legalization
        // passes have been performed, we can emit target code from
        // the IR module.
        //
        // TODO: do we want to emit directly from IR, or translate the
        // IR back into AST for emission?
        visitor.emitIRModule(&context, irModule);

        // If we are in case (2b) and the user *also* has AST-based code
        // that we need to output, we'll do it now.
        if (translationUnit->compileFlags & SLANG_COMPILE_FLAG_NO_CHECKING)
        {
            // First make sure that we've emitted any types that were declared
            // in the IR, but then subsequently only used by the AST
            for( auto decl : lowered.irDecls )
            {
                visitor.emitIRUsedDeclRef(&context, makeDeclRef(decl));
            }

            visitor.EmitDeclsInContainer(lowered.program);
        }

        // TODO: need to clean up the IR module here
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
