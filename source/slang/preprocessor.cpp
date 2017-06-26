// preprocessor.cpp
#include "preprocessor.h"

#include "compiler.h"
#include "diagnostics.h"
#include "lexer.h"
// Needed so that we can construct modifier syntax to represent GLSL directives
#include "syntax.h"

#include <assert.h>

// This file provides an implementation of a simple C-style preprocessor.
// It does not aim for 100% compatibility with any particular preprocessor
// specification, but the goal is to have it accept the most common
// idioms for using the preprocessor, found in shader code in the wild.


namespace Slang {

// State of a preprocessor conditional, which can change when
// we encounter directives like `#elif` or `#endif`
enum class PreprocessorConditionalState
{
    Before, // We have not yet seen a branch with a `true` condition.
    During, // We are inside the branch with a `true` condition.
    After,  // We have already seen the branch with a `true` condition.
};

// Represents a preprocessor conditional that we are currently
// nested inside.
struct PreprocessorConditional
{
    // The next outer conditional in the current file/stream, or NULL.
    PreprocessorConditional*        parent;

    // The directive token that started the conditional (an `#if` or `#ifdef`)
    Token                           ifToken;

    // The `#else` directive token, if one has been seen (otherwise `TokenType::Unknown`)
    Token                           elseToken;

    // The state of the conditional
    PreprocessorConditionalState    state;
};

struct PreprocessorMacro;

struct PreprocessorEnvironment
{
    // The "outer" environment, to be used if lookup in this env fails
    PreprocessorEnvironment*                parent = NULL;

    // Macros defined in this environment
    Dictionary<String, PreprocessorMacro*>  macros;

    ~PreprocessorEnvironment();
};

// Input tokens can either come from source text, or from macro expansion.
// In general, input streams can be nested, so we have to keep a conceptual
// stack of input.

// A stream of input tokens to be consumed
struct PreprocessorInputStream
{
    // The next input stream up the stack, if any.
    PreprocessorInputStream*        parent;

    // The deepest preprocessor conditional active for this stream.
    PreprocessorConditional*        conditional;

    // Environment to use when looking up macros
    PreprocessorEnvironment*        environment;

    // Reader for pre-tokenized input
    TokenReader                     tokenReader;

    // If we are clobbering source locations with `#line`, then
    // the state is tracked here:

    // Are we overriding source locations?
    bool                            isOverridingSourceLoc;

    // What is the file name we are overriding to?
    String                          overrideFileName;

    // What is the relative offset to apply to any line numbers?
    int                             overrideLineOffset;

    // Destructor is virtual so that we can clean up
    // after concrete subtypes.
    virtual ~PreprocessorInputStream() = default;
};

struct SourceTextInputStream : PreprocessorInputStream
{
    // The pre-tokenized input
    TokenList           lexedTokens;
};

struct MacroExpansion : PreprocessorInputStream
{
    // The macro we will expand
    PreprocessorMacro*  macro;
};

struct ObjectLikeMacroExpansion : MacroExpansion
{
};

struct FunctionLikeMacroExpansion : MacroExpansion
{
    // Environment for macro arguments
    PreprocessorEnvironment     argumentEnvironment;
};

// An enumeration for the diferent types of macros
enum class PreprocessorMacroFlavor
{
    ObjectLike,
    FunctionArg,
    FunctionLike,
};

// In the current design (which we may want to re-consider),
// a macro is a specialized flavor of input stream, that
// captures the token list in its expansion, and then
// can be "played back."
struct PreprocessorMacro
{
    // The name under which the macro was `#define`d
    Token                       nameToken;

    // Parameters of the macro, in case of a function-like macro
    List<Token>                 params;

    // The tokens that make up the macro body
    TokenList                   tokens;

    // The flavor of macro
    PreprocessorMacroFlavor     flavor;

    // The environment in which this macro needs to be expanded.
    // For ordinary macros this will be the global environment,
    // while for function-like macro arguments, it will be
    // the environment of the macro invocation.
    PreprocessorEnvironment*    environment;
};

// State of the preprocessor
struct Preprocessor
{
    // diagnostics sink to use when writing messages
    DiagnosticSink*                         sink;

    // An external callback interface to use when looking
    // for files in a `#include` directive
    IncludeHandler*                         includeHandler;

    // Current input stream (top of the stack of input)
    PreprocessorInputStream*                inputStream;

    // Currently-defined macros
    PreprocessorEnvironment                 globalEnv;

    // A pre-allocated token that can be returned to
    // represent end-of-input situations.
    Token                                   endOfFileToken;

    // The translation unit that is being parsed
    TranslationUnitRequest*                 translationUnit;

    TranslationUnitRequest* getTranslationUnit()
    {
        return translationUnit;
    }

    ProgramSyntaxNode* getSyntax()
    {
        return getTranslationUnit()->SyntaxNode.Ptr();
    }

    CompileRequest* getCompileRequest()
    {
        return getTranslationUnit()->compileRequest;
    }
};

// Convenience routine to access the diagnostic sink
static DiagnosticSink* GetSink(Preprocessor* preprocessor)
{
    return preprocessor->sink;
}

//
// Forward declarations
//

static void DestroyConditional(PreprocessorConditional* conditional);
static void DestroyMacro(Preprocessor* preprocessor, PreprocessorMacro* macro);

//
// Basic Input Handling
//

// Create a fresh input stream
static void  InitializeInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    inputStream->parent = NULL;
    inputStream->conditional = NULL;
    inputStream->environment = &preprocessor->globalEnv;
}

// Destroy an input stream
static void DestroyInputStream(Preprocessor* /*preprocessor*/, PreprocessorInputStream* inputStream)
{
    delete inputStream;
}

// Create an input stream to represent a pre-tokenized input file.
// TODO(tfoley): pre-tokenizing files isn't going to work in the long run.
static PreprocessorInputStream* CreateInputStreamForSource(Preprocessor* preprocessor, String const& source, String const& fileName)
{
    SourceTextInputStream* inputStream = new SourceTextInputStream();
    InitializeInputStream(preprocessor, inputStream);

    // Use existing `Lexer` to generate a token stream.
    Lexer lexer(fileName, source, GetSink(preprocessor));
    inputStream->lexedTokens = lexer.lexAllTokens();
    inputStream->tokenReader = TokenReader(inputStream->lexedTokens);

    return inputStream;
}



static void PushInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    inputStream->parent = preprocessor->inputStream;
    preprocessor->inputStream = inputStream;
}

// Called when we reach the end of an input stream.
// Performs some validation and then destroys the input stream if required.
static void EndInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    // If there are any conditionals that weren't completed, then it is an error
    if (inputStream->conditional)
    {
        PreprocessorConditional* conditional = inputStream->conditional;

        GetSink(preprocessor)->diagnose(conditional->ifToken.Position, Diagnostics::endOfFileInPreprocessorConditional);

        while (conditional)
        {
            PreprocessorConditional* parent = conditional->parent;
            DestroyConditional(conditional);
            conditional = parent;
        }
    }

    DestroyInputStream(preprocessor, inputStream);
}

// Potentially clobber source location information based on `#line`
static Token PossiblyOverrideSourceLoc(PreprocessorInputStream* inputStream, Token const& token)
{
    Token result = token;
    if( inputStream->isOverridingSourceLoc )
    {
        result.Position.FileName = inputStream->overrideFileName;
        result.Position.Line += inputStream->overrideLineOffset;
    }
    return result;
}

// Consume one token from an input stream
static Token AdvanceRawToken(PreprocessorInputStream* inputStream)
{
    return PossiblyOverrideSourceLoc(inputStream, inputStream->tokenReader.AdvanceToken());
}

// Peek one token from an input stream
static Token PeekRawToken(PreprocessorInputStream* inputStream)
{
    return PossiblyOverrideSourceLoc(inputStream, inputStream->tokenReader.PeekToken());
}

// Peek one token type from an input stream
static TokenType PeekRawTokenType(PreprocessorInputStream* inputStream)
{
    return inputStream->tokenReader.PeekTokenType();
}


// Read one token in "raw" mode (meaning don't expand macros)
static Token AdvanceRawToken(Preprocessor* preprocessor)
{
    for (;;)
    {
        // Look at the input stream on top of the stack
        PreprocessorInputStream* inputStream = preprocessor->inputStream;

        // If there isn't one, then there is no more input left to read.
        if (!inputStream)
        {
            return preprocessor->endOfFileToken;
        }

        // The top-most input stream may be at its end
        if (PeekRawTokenType(inputStream) == TokenType::EndOfFile)
        {
            // If there is another stream remaining, switch to it
            if (inputStream->parent)
            {
                preprocessor->inputStream = inputStream->parent;
                EndInputStream(preprocessor, inputStream);
                continue;
            }
        }

        // Everything worked, so read a token from the top-most stream
        return AdvanceRawToken(inputStream);
    }
}

// Return the next token in "raw" mode, but don't advance the
// current token state.
static Token PeekRawToken(Preprocessor* preprocessor)
{
    // We need to find the strema that `advanceRawToken` would read from.
    PreprocessorInputStream* inputStream = preprocessor->inputStream;
    for (;;)
    {
        if (!inputStream)
        {
            // No more input streams left to read
            return preprocessor->endOfFileToken;
        }

        // The top-most input stream may be at its end, so
        // look one entry up the stack (don't actually pop
        // here, since we are just peeking)
        if (PeekRawTokenType(inputStream) == TokenType::EndOfFile)
        {
            if (inputStream->parent)
            {
                inputStream = inputStream->parent;
                continue;
            }
        }

        // Everything worked, so the token we just peeked is fine.
        return PeekRawToken(inputStream);
    }
}

// Without advancing preprocessor state, look *two* raw tokens ahead
// (This is only needed in order to determine when we are possibly
// expanding a function-style macro)
TokenType PeekSecondRawTokenType(Preprocessor* preprocessor)
{
    // We need to find the strema that `advanceRawToken` would read from.
    PreprocessorInputStream* inputStream = preprocessor->inputStream;
    int count = 1;
    for (;;)
    {
        if (!inputStream)
        {
            // No more input streams left to read
            return TokenType::EndOfFile;
        }

        // The top-most input stream may be at its end, so
        // look one entry up the stack (don't actually pop
        // here, since we are just peeking)

        TokenReader reader = inputStream->tokenReader;
        if (reader.PeekTokenType() == TokenType::EndOfFile)
        {
            inputStream = inputStream->parent;
            continue;
        }

        if (count)
        {
            count--;

            // Note: we are advancing our temporary
            // copy of the token reader
            reader.AdvanceToken();
            if (reader.PeekTokenType() == TokenType::EndOfFile)
            {
                inputStream = inputStream->parent;
                continue;
            }
        }

        // Everything worked, so peek a token from the top-most stream
        return reader.PeekTokenType();
    }
}


// Get the location of the current (raw) token
static CodePosition PeekLoc(Preprocessor* preprocessor)
{
    return PeekRawToken(preprocessor).Position;
}

// Get the `TokenType` of the current (raw) token
static TokenType PeekRawTokenType(Preprocessor* preprocessor)
{
    return PeekRawToken(preprocessor).Type;
}

//
// Macros
//

// Create a macro
static PreprocessorMacro* CreateMacro(Preprocessor* preprocessor)
{
    // TODO(tfoley): Allocate these more intelligently.
    // For example, consider pooling them on the preprocessor.

    PreprocessorMacro* macro = new PreprocessorMacro();
    macro->flavor = PreprocessorMacroFlavor::ObjectLike;
    macro->environment = &preprocessor->globalEnv;
    return macro;
}

// Destroy a macro
static void DestroyMacro(Preprocessor* /*preprocessor*/, PreprocessorMacro* macro)
{
    delete macro;
}


// Find the currently-defined macro of the given name, or return NULL
static PreprocessorMacro* LookupMacro(PreprocessorEnvironment* environment, String const& name)
{
    for(PreprocessorEnvironment* e = environment; e; e = e->parent)
    {
        PreprocessorMacro* macro = NULL;
        if (e->macros.TryGetValue(name, macro))
            return macro;
    }

    return NULL;
}

static PreprocessorEnvironment* GetCurrentEnvironment(Preprocessor* preprocessor)
{
    // The environment we will use for looking up a macro is assocaited
    // with the current input stream (because it may include entries
    // for macro arguments).
    //
    // We need to be careful, though, when we are at the end of an
    // input stream (e.g., representing one argument), so that we
    // don't use its environment.

    PreprocessorInputStream* inputStream = preprocessor->inputStream;

    for(;;)
    {
        // If there is no input stream that isn't at its end,
        // then fall back to the global environment.
        if (!inputStream)
            return &preprocessor->globalEnv;

        // If the current input stream is at its end, then
        // fall back to its parent stream.
        if (inputStream->tokenReader.PeekTokenType() == TokenType::EndOfFile)
        {
            inputStream = inputStream->parent;
            continue;
        }

        // If we've found an active stream that isn't at its end,
        // then use that for lookup.
        return inputStream->environment;
    }
}

static PreprocessorMacro* LookupMacro(Preprocessor* preprocessor, String const& name)
{
    return LookupMacro(GetCurrentEnvironment(preprocessor), name);
}

// A macro is "busy" if it is currently being used for expansion.
// A macro cannot be expanded again while busy, to avoid infinite recursion.
static bool IsMacroBusy(PreprocessorMacro* /*macro*/)
{
    // TODO: need to implement this correctly
    //
    // The challenge here is that we are implementing expansion
    // for argumenst to function-like macros in a "lazy" fashion.
    //
    // The letter of the spec is that we should macro expand
    // each argument *before* substitution, and then go and
    // macro-expand the substituted body. This means that we
    // can invoke a macro as part of an argument to an
    // invocation of the same macro:
    //
    //     FOO( 1, FOO(22), 333 );
    //
    // In our implementation, the "inner" invocation of `FOO`
    // gets expanded at the point where it gets referenced
    // in the body of the "outer" invocation of `FOO`.
    // Doing things this way leads to greatly simplified
    // code for handling expansion.
    //
    // A proper implementation of `IsMacroBusy` needs to
    // take context into account, so that it bans recursive
    // use of a macro when it occurs (indirectly) through
    // the *body* of the expansion, but not when it occcurs
    // only through an *argument*.
    return false;
}

//
// Reading Tokens With Expansion
//

static void InitializeMacroExpansion(
    Preprocessor*       preprocessor,
    MacroExpansion*     expansion,
    PreprocessorMacro*  macro)
{
    InitializeInputStream(preprocessor, expansion);
    expansion->environment = macro->environment;
    expansion->macro = macro;
    expansion->tokenReader = TokenReader(macro->tokens);
}

static void PushMacroExpansion(
    Preprocessor*   preprocessor,
    MacroExpansion* expansion)
{
    PushInputStream(preprocessor, expansion);
}

static void AddEndOfStreamToken(
    Preprocessor*       preprocessor,
    PreprocessorMacro*  macro)
{
    Token token = PeekRawToken(preprocessor);
    token.Type = TokenType::EndOfFile;
    macro->tokens.mTokens.Add(token);
}

// Check whether the current token on the given input stream should be
// treated as a macro invocation, and if so set up state for expanding
// that macro.
static void MaybeBeginMacroExpansion(
    Preprocessor*               preprocessor )
{
    // We iterate because the first token in the expansion of one
    // macro may be another macro invocation.
    for (;;)
    {
        // Look at the next token ahead of us
        Token const& token = PeekRawToken(preprocessor);

        // Not an identifier? Can't be a macro.
        if (token.Type != TokenType::Identifier)
            return;

        // Look for a macro with the given name.
        String name = token.Content;
        PreprocessorMacro* macro = LookupMacro(preprocessor, name);

        // Not a macro? Can't be an invocation.
        if (!macro)
            return;

        // If the macro is busy (already being expanded),
        // don't try to trigger recursive expansion
        if (IsMacroBusy(macro))
            return;

        // A function-style macro invocation should only match
        // if the token *after* the identifier is `(`. This
        // requires more lookahead than we usually have/need
        if (macro->flavor == PreprocessorMacroFlavor::FunctionLike)
        {
            if(PeekSecondRawTokenType(preprocessor) != TokenType::LParent)
                return;

            // Consume the token that triggered macro expansion
            AdvanceRawToken(preprocessor);

            // Consume the opening `(`
            Token leftParen = AdvanceRawToken(preprocessor);

            FunctionLikeMacroExpansion* expansion = new FunctionLikeMacroExpansion();
            InitializeMacroExpansion(preprocessor, expansion, macro);
            expansion->argumentEnvironment.parent = &preprocessor->globalEnv;
            expansion->environment = &expansion->argumentEnvironment;

            // Try to read any arguments present.
            int paramCount = macro->params.Count();
            int argIndex = 0;

            switch (PeekRawTokenType(preprocessor))
            {
            case TokenType::EndOfFile:
            case TokenType::RParent:
                // No arguments.
                break;

            default:
                // At least one argument
                while(argIndex < paramCount)
                {
                    // Read an argument

                    // Create the argument, represented as a special flavor of macro
                    PreprocessorMacro* arg = CreateMacro(preprocessor);
                    arg->flavor = PreprocessorMacroFlavor::FunctionArg;
                    arg->environment = GetCurrentEnvironment(preprocessor);

                    // Associate the new macro with its parameter name
                    Token paramToken = macro->params[argIndex];
                    String const& paramName = paramToken.Content;
                    arg->nameToken = paramToken;
                    expansion->argumentEnvironment.macros[paramName] = arg;
                    argIndex++;

                    // Read tokens for the argument

                    // We track the nesting depth, since we don't break
                    // arguments on a `,` nested in balanced parentheses
                    //
                    int nesting = 0;
                    for (;;)
                    {
                        switch (PeekRawTokenType(preprocessor))
                        {
                        case TokenType::EndOfFile:
                            // if we reach the end of the file,
                            // then we have an error, and need to
                            // bail out
                            AddEndOfStreamToken(preprocessor, arg);
                            goto doneWithAllArguments;

                        case TokenType::RParent:
                            // If we see a right paren when we aren't nested
                            // then we are at the end of an argument
                            if (nesting == 0)
                            {
                                AddEndOfStreamToken(preprocessor, arg);
                                goto doneWithAllArguments;
                            }
                            // Otherwise we decrease our nesting depth, add
                            // the token, and keep going
                            nesting--;
                            break;

                        case TokenType::Comma:
                            // If we see a comma when we aren't nested
                            // then we are at the end of an argument
                            if (nesting == 0)
                            {
                                AddEndOfStreamToken(preprocessor, arg);
                                AdvanceRawToken(preprocessor);
                                goto doneWithArgument;
                            }
                            // Otherwise we add it as a normal token
                            break;

                        case TokenType::LParent:
                            // If we see a left paren then we need to
                            // increase our tracking of nesting
                            nesting++;
                            break;

                        default:
                            break;
                        }

                        // Add the token and continue parsing.
                        arg->tokens.mTokens.Add(AdvanceRawToken(preprocessor));
                    }
                doneWithArgument: {}
                    // We've parsed an argument and should move onto
                    // the next one.
                }
                break;
            }
        doneWithAllArguments:
            // TODO: handle possible varargs

            // Expect closing right paren
            if (PeekRawTokenType(preprocessor) == TokenType::RParent)
            {
                AdvanceRawToken(preprocessor);
            }
            else
            {
                GetSink(preprocessor)->diagnose(PeekLoc(preprocessor), Diagnostics::expectedTokenInMacroArguments, TokenType::RParent, PeekRawTokenType(preprocessor));
            }

            int argCount = argIndex;
            if (argCount != paramCount)
            {
                // TODO: diagnose
                throw 99;
            }

            // We are ready to expand.
            PushMacroExpansion(preprocessor, expansion);
        }
        else
        {
            // Consume the token that triggered macro expansion
            AdvanceRawToken(preprocessor);

            // Object-like macros are the easy case.
            ObjectLikeMacroExpansion* expansion = new ObjectLikeMacroExpansion();
            InitializeMacroExpansion(preprocessor, expansion, macro);
            PushMacroExpansion(preprocessor, expansion);
        }
    }
}

// Read one token with macro-expansion enabled.
static Token AdvanceToken(Preprocessor* preprocessor)
{
top:
    // Check whether we need to macro expand at the cursor.
    MaybeBeginMacroExpansion(preprocessor);

    // Read a raw token (now that expansion has been triggered)
    Token token = AdvanceRawToken(preprocessor);

    // Check if we need to perform token pasting
    if (PeekRawTokenType(preprocessor) != TokenType::PoundPound)
    {
        // If we aren't token pasting, then we are done
        return token;
    }
    else
    {
        // We are pasting tokens, which could get messy

        StringBuilder sb;
        sb << token.Content;

        while (PeekRawTokenType(preprocessor) == TokenType::PoundPound)
        {
            // Consume the `##`
            AdvanceRawToken(preprocessor);

            // Possibly macro-expand the next token
            MaybeBeginMacroExpansion(preprocessor);

            // Read the next raw token (now that expansion has been triggered)
            Token nextToken = AdvanceRawToken(preprocessor);

            sb << nextToken.Content;
        }

        // Now re-lex the input
        PreprocessorInputStream* inputStream = CreateInputStreamForSource(preprocessor, sb.ProduceString(), "token paste");
        if (inputStream->tokenReader.GetCount() != 1)
        {
            // We expect a token paste to produce a single token
            // TODO(tfoley): emit a diagnostic here
        }

        PushInputStream(preprocessor, inputStream);
        goto top;
    }
}

// Read one token with macro-expansion enabled.
//
// Note that because triggering macro expansion may
// involve changing the input-stream state, this
// operation *can* have side effects.
static Token PeekToken(Preprocessor* preprocessor)
{
    // Check whether we need to macro expand at the cursor.
    MaybeBeginMacroExpansion(preprocessor);

    // Peek a raw token (now that expansion has been triggered)
    return PeekRawToken(preprocessor);

    // TODO: need a plan for how to handle token pasting
    // here without it being onerous. Would be nice if we
    // didn't have to re-do pasting on a "peek"...
}

// Peek the type of the next token, including macro expansion.
static TokenType PeekTokenType(Preprocessor* preprocessor)
{
    return PeekToken(preprocessor).Type;
}

//
// Preprocessor Directives
//

// When reading a preprocessor directive, we use a context
// to wrap the direct preprocessor routines defines so far.
//
// One of the most important things the directive context
// does is give us a convenient way to read tokens with
// a guarantee that we won't read past the end of a line.
struct PreprocessorDirectiveContext
{
    // The preprocessor that is parsing the directive.
    Preprocessor*   preprocessor;

    // The directive token (e.g., the `if` in `#if`).
    // Useful for reference in diagnostic messages.
    Token           directiveToken;

    // Has any kind of parse error been encountered in
    // the directive so far?
    bool            parseError;

    // Have we done the necessary checks at the end
    // of the directive already?
    bool            haveDoneEndOfDirectiveChecks;
};

// Get the token for  the preprocessor directive being parsed.
inline Token const& GetDirective(PreprocessorDirectiveContext* context)
{
    return context->directiveToken;
}

// Get the name of the directive being parsed.
inline String const& GetDirectiveName(PreprocessorDirectiveContext* context)
{
    return context->directiveToken.Content;
}

// Get the location of the directive being parsed.
inline CodePosition const& GetDirectiveLoc(PreprocessorDirectiveContext* context)
{
    return context->directiveToken.Position;
}

// Wrapper to get the diagnostic sink in the context of a directive.
static inline DiagnosticSink* GetSink(PreprocessorDirectiveContext* context)
{
    return GetSink(context->preprocessor);
}

// Wrapper to get a "current" location when parsing a directive
static CodePosition PeekLoc(PreprocessorDirectiveContext* context)
{
    return PeekLoc(context->preprocessor);
}

// Wrapper to look up a macro in the context of a directive.
static PreprocessorMacro* LookupMacro(PreprocessorDirectiveContext* context, String const& name)
{
    return LookupMacro(context->preprocessor, name);
}

// Determine if we have read everthing on the directive's line.
static bool IsEndOfLine(PreprocessorDirectiveContext* context)
{
    return PeekRawToken(context->preprocessor).Type == TokenType::EndOfDirective;
}

// Peek one raw token in a directive, without going past the end of the line.
static Token PeekRawToken(PreprocessorDirectiveContext* context)
{
    return PeekRawToken(context->preprocessor);
}

// Read one raw token in a directive, without going past the end of the line.
static Token AdvanceRawToken(PreprocessorDirectiveContext* context)
{
    if (IsEndOfLine(context))
        return PeekRawToken(context);
    return AdvanceRawToken(context->preprocessor);
}

// Peek next raw token type, without going past the end of the line.
static TokenType PeekRawTokenType(PreprocessorDirectiveContext* context)
{
    return PeekRawTokenType(context->preprocessor);
}

// Read one token, with macro-expansion, without going past the end of the line.
static Token AdvanceToken(PreprocessorDirectiveContext* context)
{
    if (IsEndOfLine(context))
        return PeekRawToken(context);
    return AdvanceToken(context->preprocessor);
}

// Peek one token, with macro-expansion, without going past the end of the line.
static Token PeekToken(PreprocessorDirectiveContext* context)
{
    if (IsEndOfLine(context))
        context->preprocessor->endOfFileToken;
    return PeekToken(context->preprocessor);
}

// Peek next token type, with macro-expansion, without going past the end of the line.
static TokenType PeekTokenType(PreprocessorDirectiveContext* context)
{
    if (IsEndOfLine(context))
        return TokenType::EndOfDirective;
    return PeekTokenType(context->preprocessor);
}

// Skip to the end of the line (useful for recovering from errors in a directive)
static void SkipToEndOfLine(PreprocessorDirectiveContext* context)
{
    while(!IsEndOfLine(context))
    {
        AdvanceRawToken(context);
    }
}

static bool ExpectRaw(PreprocessorDirectiveContext* context, TokenType tokenType, DiagnosticInfo const& diagnostic, Token* outToken = NULL)
{
    if (PeekRawTokenType(context) != tokenType)
    {
        // Only report the first parse error within a directive
        if (!context->parseError)
        {
            GetSink(context)->diagnose(PeekLoc(context), diagnostic, tokenType, GetDirectiveName(context));
        }
        context->parseError = true;
        return false;
    }
    Token const& token = AdvanceRawToken(context);
    if (outToken)
        *outToken = token;
    return true;
}

static bool Expect(PreprocessorDirectiveContext* context, TokenType tokenType, DiagnosticInfo const& diagnostic, Token* outToken = NULL)
{
    if (PeekTokenType(context) != tokenType)
    {
        // Only report the first parse error within a directive
        if (!context->parseError)
        {
            GetSink(context)->diagnose(PeekLoc(context), diagnostic, tokenType, GetDirectiveName(context));
            context->parseError = true;
        }
        return false;
    }
    Token const& token = AdvanceToken(context);
    if (outToken)
        *outToken = token;
    return true;
}



//
// Preprocessor Conditionals
//

// Determine whether the current preprocessor state means we
// should be skipping tokens.
static bool IsSkipping(Preprocessor* preprocessor)
{
    PreprocessorInputStream* inputStream = preprocessor->inputStream;
    if (!inputStream) return false;

    // If we are not inside a preprocessor conditional, then don't skip
    PreprocessorConditional* conditional = inputStream->conditional;
    if (!conditional) return false;

    // skip tokens unless the conditional is inside its `true` case
    return conditional->state != PreprocessorConditionalState::During;
}

// Wrapper for use inside directives
static inline bool IsSkipping(PreprocessorDirectiveContext* context)
{
    return IsSkipping(context->preprocessor);
}

// Create a preprocessor conditional
static PreprocessorConditional* CreateConditional(Preprocessor* /*preprocessor*/)
{
    // TODO(tfoley): allocate these more intelligently (for example,
    // pool them on the `Preprocessor`.
    return new PreprocessorConditional();
}

// Destroy a preprocessor conditional.
static void DestroyConditional(PreprocessorConditional* conditional)
{
    delete conditional;
}

// Start a preprocessor conditional, with an initial enable/disable state.
static void beginConditional(
    PreprocessorDirectiveContext*   context,
    PreprocessorInputStream*        inputStream,
    bool                            enable)
{
    Preprocessor* preprocessor = context->preprocessor;
    assert(inputStream);

    PreprocessorConditional* conditional = CreateConditional(preprocessor);

    conditional->ifToken = context->directiveToken;

    // Set state of this condition appropriately.
    //
    // Default to the "haven't yet seen a `true` branch" state.
    PreprocessorConditionalState state = PreprocessorConditionalState::Before;
    //
    // If we are nested inside a `false` branch of another condition, then
    // we never want to enable, so we act as if we already *saw* the `true` branch.
    //
    if (IsSkipping(preprocessor)) state = PreprocessorConditionalState::After;
    //
    // Similarly, if we ran into any parse errors when dealing with the
    // opening directive, then things are probably screwy and we should just
    // skip all the branches.
    if (IsSkipping(preprocessor)) state = PreprocessorConditionalState::After;
    //
    // Otherwise, if our condition was true, then set us to be inside the `true` branch
    else if (enable) state = PreprocessorConditionalState::During;

    conditional->state = state;

    // Push conditional onto the stack
    conditional->parent = inputStream->conditional;
    inputStream->conditional = conditional;
}

// Start a preprocessor conditional, with an initial enable/disable state.
static void beginConditional(
    PreprocessorDirectiveContext*   context,
    bool                            enable)
{
    beginConditional(context, context->preprocessor->inputStream, enable);
}

//
// Preprocessor Conditional Expressions
//

// Conditional expressions are always of type `int`
typedef int PreprocessorExpressionValue;

// Forward-declaretion
static PreprocessorExpressionValue ParseAndEvaluateExpression(PreprocessorDirectiveContext* context);

// Parse a unary (prefix) expression inside of a preprocessor directive.
static PreprocessorExpressionValue ParseAndEvaluateUnaryExpression(PreprocessorDirectiveContext* context)
{
    switch (PeekTokenType(context))
    {
    // handle prefix unary ops
    case TokenType::OpSub:
        AdvanceToken(context);
        return -ParseAndEvaluateUnaryExpression(context);
    case TokenType::OpNot:
        AdvanceToken(context);
        return !ParseAndEvaluateUnaryExpression(context);
    case TokenType::OpBitNot:
        AdvanceToken(context);
        return ~ParseAndEvaluateUnaryExpression(context);

    // handle parenthized sub-expression
    case TokenType::LParent:
        {
            Token leftParen = AdvanceToken(context);
            PreprocessorExpressionValue value = ParseAndEvaluateExpression(context);
            if (!Expect(context, TokenType::RParent, Diagnostics::expectedTokenInPreprocessorExpression))
            {
                GetSink(context)->diagnose(leftParen.Position, Diagnostics::seeOpeningToken, leftParen);
            }
            return value;
        }

    case TokenType::IntLiterial:
        return StringToInt(AdvanceToken(context).Content);

    case TokenType::Identifier:
        {
            Token token = AdvanceToken(context);
            if (token.Content == "defined")
            {
                // handle `defined(someName)`

                // Possibly parse a `(`
                Token leftParen;
                if (PeekRawTokenType(context) == TokenType::LParent)
                {
                    leftParen = AdvanceRawToken(context);
                }

                // Expect an identifier
                Token nameToken;
                if (!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInDefinedExpression, &nameToken))
                {
                    return 0;
                }
                String name = nameToken.Content;

                // If we saw an opening `(`, then expect one to close
                if (leftParen.Type != TokenType::Unknown)
                {
                    if(!ExpectRaw(context, TokenType::RParent, Diagnostics::expectedTokenInDefinedExpression))
                    {
                        GetSink(context)->diagnose(leftParen.Position, Diagnostics::seeOpeningToken, leftParen);
                        return 0;
                    }
                }

                return LookupMacro(context, name) != NULL;
            }

            // An identifier here means it was not defined as a macro (or
            // it is defined, but as a function-like macro. These should
            // just evaluate to zero (possibly with a warning)
            return 0;
        }

    default:
        GetSink(context)->diagnose(PeekLoc(context), Diagnostics::syntaxErrorInPreprocessorExpression);
        return 0;
    }
}

// Determine the precedence level of an infix operator
// for use in parsing preprocessor conditionals.
static int GetInfixOpPrecedence(Token const& opToken)
{
    // If token is on another line, it is not part of the
    // expression
    if (opToken.flags & TokenFlag::AtStartOfLine)
        return -1;

    // otherwise we look at the token type to figure
    // out what precednece it should be parse with
    switch (opToken.Type)
    {
    default:
        // tokens that aren't infix operators should
        // cause us to stop parsing an expression
        return -1;

    case TokenType::OpMul:     return 10;
    case TokenType::OpDiv:     return 10;
    case TokenType::OpMod:     return 10;

    case TokenType::OpAdd:     return 9;
    case TokenType::OpSub:     return 9;

    case TokenType::OpLsh:     return 8;
    case TokenType::OpRsh:     return 8;

    case TokenType::OpLess:    return 7;
    case TokenType::OpGreater: return 7;
    case TokenType::OpLeq:     return 7;
    case TokenType::OpGeq:     return 7;

    case TokenType::OpEql:     return 6;
    case TokenType::OpNeq:     return 6;

    case TokenType::OpBitAnd:  return 5;
    case TokenType::OpBitOr:   return 4;
    case TokenType::OpBitXor:  return 3;
    case TokenType::OpAnd:     return 2;
    case TokenType::OpOr:      return 1;
    }
};

// Evaluate one infix operation in a preprocessor
// conditional expression
static PreprocessorExpressionValue EvaluateInfixOp(
    PreprocessorDirectiveContext*   context,
    Token const&                    opToken,
    PreprocessorExpressionValue     left,
    PreprocessorExpressionValue     right)
{
    switch (opToken.Type)
    {
    default:
//        SLANG_INTERNAL_ERROR(getSink(preprocessor), opToken);
        return 0;
        break;

    case TokenType::OpMul:     return left * right;
    case TokenType::OpDiv:
    {
        if (right == 0)
        {
            if (!context->parseError)
            {
                GetSink(context)->diagnose(opToken.Position, Diagnostics::divideByZeroInPreprocessorExpression);
            }
            return 0;
        }
        return left / right;
    }
    case TokenType::OpMod:
    {
        if (right == 0)
        {
            if (!context->parseError)
            {
                GetSink(context)->diagnose(opToken.Position, Diagnostics::divideByZeroInPreprocessorExpression);
            }
            return 0;
        }
        return left % right;
    }
    case TokenType::OpAdd:      return left +  right;
    case TokenType::OpSub:      return left -  right;
    case TokenType::OpLsh:      return left << right;
    case TokenType::OpRsh:      return left >> right;
    case TokenType::OpLess:     return left <  right ? 1 : 0;
    case TokenType::OpGreater:  return left >  right ? 1 : 0;
    case TokenType::OpLeq:      return left <= right ? 1 : 0;
    case TokenType::OpGeq:      return left <= right ? 1 : 0;
    case TokenType::OpEql:      return left == right ? 1 : 0;
    case TokenType::OpNeq:      return left != right ? 1 : 0;
    case TokenType::OpBitAnd:   return left & right;
    case TokenType::OpBitOr:    return left | right;
    case TokenType::OpBitXor:   return left ^ right;
    case TokenType::OpAnd:      return left && right;
    case TokenType::OpOr:       return left || right;
    }
}

// Parse the rest of an infix preprocessor expression with
// precedence greater than or equal to the given `precedence` argument.
// The value of the left-hand-side expression is provided as
// an argument.
// This is used to form a simple recursive-descent expression parser.
static PreprocessorExpressionValue ParseAndEvaluateInfixExpressionWithPrecedence(
    PreprocessorDirectiveContext* context,
    PreprocessorExpressionValue left,
    int precedence)
{
    for (;;)
    {
        // Look at the next token, and see if it is an operator of
        // high enough precedence to be included in our expression
        Token opToken = PeekToken(context);
        int opPrecedence = GetInfixOpPrecedence(opToken);

        // If it isn't an operator of high enough precendece, we are done.
        if(opPrecedence < precedence)
            break;

        // Otherwise we need to consume the operator token.
        AdvanceToken(context);

        // Next we parse a right-hand-side expression by starting with
        // a unary expression and absorbing and many infix operators
        // as possible with strictly higher precedence than the operator
        // we found above.
        PreprocessorExpressionValue right = ParseAndEvaluateUnaryExpression(context);
        for (;;)
        {
            // Look for an operator token
            Token rightOpToken = PeekToken(context);
            int rightOpPrecedence = GetInfixOpPrecedence(rightOpToken);

            // If no operator was found, or the operator wasn't high
            // enough precedence to fold into the right-hand-side,
            // exit this loop.
            if (rightOpPrecedence <= opPrecedence)
                break;

            // Now invoke the parser recursively, passing in our
            // existing right-hand side to form an even larger one.
            right = ParseAndEvaluateInfixExpressionWithPrecedence(
                context,
                right,
                rightOpPrecedence);
        }

        // Now combine the left- and right-hand sides using
        // the operator we found above.
        left = EvaluateInfixOp(context, opToken, left, right);
    }
    return left;
}

// Parse a complete (infix) preprocessor expression, and return its value
static PreprocessorExpressionValue ParseAndEvaluateExpression(PreprocessorDirectiveContext* context)
{
    // First read in the left-hand side (or the whole expression in the unary case)
    PreprocessorExpressionValue value = ParseAndEvaluateUnaryExpression(context);

    // Try to read in trailing infix operators with correct precedence
    return ParseAndEvaluateInfixExpressionWithPrecedence(context, value, 0);
}

// Handle a `#if` directive
static void HandleIfDirective(PreprocessorDirectiveContext* context)
{
    // Record current inpu stream in case preprocessor expression
    // changes the input stream to a macro expansion while we
    // are parsing.
    auto inputStream = context->preprocessor->inputStream;

    // Parse a preprocessor expression.
    PreprocessorExpressionValue value = ParseAndEvaluateExpression(context);

    // Begin a preprocessor block, enabled based on the expression.
    beginConditional(context, inputStream, value != 0);
}

// Handle a `#ifdef` directive
static void HandleIfDefDirective(PreprocessorDirectiveContext* context)
{
    // Expect a raw identifier, so we can check if it is defined
    Token nameToken;
    if(!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    String name = nameToken.Content;

    // Check if the name is defined.
    beginConditional(context, LookupMacro(context, name) != NULL);
}

// Handle a `#ifndef` directive
static void HandleIfNDefDirective(PreprocessorDirectiveContext* context)
{
    // Expect a raw identifier, so we can check if it is defined
    Token nameToken;
    if(!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    String name = nameToken.Content;

    // Check if the name is defined.
    beginConditional(context, LookupMacro(context, name) == NULL);
}

// Handle a `#else` directive
static void HandleElseDirective(PreprocessorDirectiveContext* context)
{
    PreprocessorInputStream* inputStream = context->preprocessor->inputStream;
    assert(inputStream);

    // if we aren't inside a conditional, then error
    PreprocessorConditional* conditional = inputStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    // if we've already seen a `#else`, then it is an error
    if (conditional->elseToken.Type != TokenType::Unknown)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveAfterElse, GetDirectiveName(context));
        GetSink(context)->diagnose(conditional->elseToken.Position, Diagnostics::seeDirective);
        return;
    }
    conditional->elseToken = context->directiveToken;

    switch (conditional->state)
    {
    case PreprocessorConditionalState::Before:
        conditional->state = PreprocessorConditionalState::During;
        break;

    case PreprocessorConditionalState::During:
        conditional->state = PreprocessorConditionalState::After;
        break;

    default:
        break;
    }
}

// Handle a `#elif` directive
static void HandleElifDirective(PreprocessorDirectiveContext* context)
{
    // Need to grab current input stream *before* we try to parse
    // the conditional expression.
    PreprocessorInputStream* inputStream = context->preprocessor->inputStream;
    assert(inputStream);

    // HACK(tfoley): handle an empty `elif` like an `else` directive
    //
    // This is the behavior expected by at least one input program.
    // We will eventually want to be pedantic about this.
    // even if t
    if (PeekRawTokenType(context) == TokenType::EndOfDirective)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveExpectsExpression, GetDirectiveName(context));
        HandleElseDirective(context);
        return;
    }

    PreprocessorExpressionValue value = ParseAndEvaluateExpression(context);

    // if we aren't inside a conditional, then error
    PreprocessorConditional* conditional = inputStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    // if we've already seen a `#else`, then it is an error
    if (conditional->elseToken.Type != TokenType::Unknown)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveAfterElse, GetDirectiveName(context));
        GetSink(context)->diagnose(conditional->elseToken.Position, Diagnostics::seeDirective);
        return;
    }

    switch (conditional->state)
    {
    case PreprocessorConditionalState::Before:
        if(value)
            conditional->state = PreprocessorConditionalState::During;
        break;

    case PreprocessorConditionalState::During:
        conditional->state = PreprocessorConditionalState::After;
        break;

    default:
        break;
    }
}

// Handle a `#endif` directive
static void HandleEndIfDirective(PreprocessorDirectiveContext* context)
{
    PreprocessorInputStream* inputStream = context->preprocessor->inputStream;
    assert(inputStream);

    // if we aren't inside a conditional, then error
    PreprocessorConditional* conditional = inputStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    inputStream->conditional = conditional->parent;
    DestroyConditional(conditional);
}

// Helper routine to check that we find the end of a directive where
// we expect it.
//
// Most directives do not need to call this directly, since we have
// a catch-all case in the main `HandleDirective()` funciton.
// The `#include` case will call it directly to avoid complications
// when it switches the input stream.
static void expectEndOfDirective(PreprocessorDirectiveContext* context)
{
    if(context->haveDoneEndOfDirectiveChecks)
        return;

    context->haveDoneEndOfDirectiveChecks = true;

    if (!IsEndOfLine(context))
    {
        // If we already saw a previous parse error, then don't
        // emit another one for the same directive.
        if (!context->parseError)
        {
            GetSink(context)->diagnose(PeekLoc(context), Diagnostics::unexpectedTokensAfterDirective, GetDirectiveName(context));
        }
        SkipToEndOfLine(context);
    }

    // Clear out the end-of-directive token
    AdvanceRawToken(context->preprocessor);
}

// Handle a `#import` directive
static void HandleImportDirective(PreprocessorDirectiveContext* context);

// Handle a `#include` directive
static void HandleIncludeDirective(PreprocessorDirectiveContext* context)
{
    Token pathToken;
    if(!Expect(context, TokenType::StringLiterial, Diagnostics::expectedTokenInPreprocessorDirective, &pathToken))
        return;

    String path = getFileNameTokenValue(pathToken);

    // TODO(tfoley): make this robust in presence of `#line`
    String pathIncludedFrom = GetDirectiveLoc(context).FileName;
    String foundPath;
    String foundSource;

    IncludeHandler* includeHandler = context->preprocessor->includeHandler;
    if (!includeHandler)
    {
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::includeFailed, path);
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::noIncludeHandlerSpecified);
        return;
    }
    auto includeResult = includeHandler->TryToFindIncludeFile(path, pathIncludedFrom, &foundPath, &foundSource);

    switch (includeResult)
    {
    case IncludeResult::NotFound:
    case IncludeResult::Error:
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::includeFailed, path);
        return;

    case IncludeResult::Found:
        break;
    }

    // Do all checking related to the end of this directive before we push a new stream,
    // just to avoid complications where that check would need to deal with
    // a switch of input stream
    expectEndOfDirective(context);

    // Push the new file onto our stack of input streams
    // TODO(tfoley): check if we have made our include stack too deep
    PreprocessorInputStream* inputStream = CreateInputStreamForSource(context->preprocessor, foundSource, foundPath);
    inputStream->parent = context->preprocessor->inputStream;
    context->preprocessor->inputStream = inputStream;
}

// Handle a `#define` directive
static void HandleDefineDirective(PreprocessorDirectiveContext* context)
{
    Token nameToken;
    if (!Expect(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    String name = nameToken.Content;

    PreprocessorMacro* macro = CreateMacro(context->preprocessor);
    macro->nameToken = nameToken;

    PreprocessorMacro* oldMacro = LookupMacro(&context->preprocessor->globalEnv, name);
    if (oldMacro)
    {
        GetSink(context)->diagnose(nameToken.Position, Diagnostics::macroRedefinition, name);
        GetSink(context)->diagnose(oldMacro->nameToken.Position, Diagnostics::seePreviousDefinitionOf, name);

        DestroyMacro(context->preprocessor, oldMacro);
    }
    context->preprocessor->globalEnv.macros[name] = macro;

    // If macro name is immediately followed (with no space) by `(`,
    // then we have a function-like macro
    if (PeekRawTokenType(context) == TokenType::LParent)
    {
        if (!(PeekRawToken(context).flags & TokenFlag::AfterWhitespace))
        {
            // This is a function-like macro, so we need to remember that
            // and start capturing parameters
            macro->flavor = PreprocessorMacroFlavor::FunctionLike;

            AdvanceRawToken(context);

            // If there are any parameters, parse them
            if (PeekRawTokenType(context) != TokenType::RParent)
            {
                for (;;)
                {
                    // TODO: handle elipsis (`...`) for varags

                    // A macro parameter name should be a raw identifier
                    Token paramToken;
                    if (!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInMacroParameters, &paramToken))
                        break;

                    // TODO(tfoley): some validation on parameter name.
                    // Certain names (e.g., `defined` and `__VA_ARGS__`
                    // are not allowed to be used as macros or parameters).

                    // Add the parameter to the macro being deifned
                    macro->params.Add(paramToken);

                    // If we see `)` then we are done with arguments
                    if (PeekRawTokenType(context) == TokenType::RParent)
                        break;

                    ExpectRaw(context, TokenType::Comma, Diagnostics::expectedTokenInMacroParameters);
                }
            }

            ExpectRaw(context, TokenType::RParent, Diagnostics::expectedTokenInMacroParameters);
        }
    }

    // consume tokens until end-of-line
    for(;;)
    {
        Token token = AdvanceRawToken(context);
        if( token.Type == TokenType::EndOfDirective )
        {
            // Last token on line will be turned into a conceptual end-of-file
            // token for the sub-stream that the macro expands into.
            token.Type = TokenType::EndOfFile;
            macro->tokens.mTokens.Add(token);
            break;
        }

        // In the ordinary case, we just add the token to the definition
        macro->tokens.mTokens.Add(token);
    }
}

// Handle a `#undef` directive
static void HandleUndefDirective(PreprocessorDirectiveContext* context)
{
    Token nameToken;
    if (!Expect(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    String name = nameToken.Content;

    PreprocessorEnvironment* env = &context->preprocessor->globalEnv;
    PreprocessorMacro* macro = LookupMacro(env, name);
    if (macro != NULL)
    {
        // name was defined, so remove it
        env->macros.Remove(name);

        DestroyMacro(context->preprocessor, macro);
    }
    else
    {
        // name wasn't defined
        GetSink(context)->diagnose(nameToken.Position, Diagnostics::macroNotDefined, name);
    }
}

// Handle a `#warning` directive
static void HandleWarningDirective(PreprocessorDirectiveContext* context)
{
    // TODO: read rest of line without actual tokenization
    GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::userDefinedWarning, "user-defined warning");
    SkipToEndOfLine(context);
}

// Handle a `#error` directive
static void HandleErrorDirective(PreprocessorDirectiveContext* context)
{
    // TODO: read rest of line without actual tokenization
    GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::userDefinedError, "user-defined warning");
    SkipToEndOfLine(context);
}

// Handle a `#line` directive
static void HandleLineDirective(PreprocessorDirectiveContext* context)
{
    int line = 0;
    if (PeekTokenType(context) == TokenType::IntLiterial)
    {
        line = StringToInt(AdvanceToken(context).Content);
    }
    else if (PeekTokenType(context) == TokenType::Identifier
        && PeekToken(context).Content == "default")
    {
        AdvanceToken(context);

        // Stop overiding soure locations.
        context->preprocessor->inputStream->isOverridingSourceLoc = false;
        return;
    }
    else
    {
        GetSink(context)->diagnose(PeekLoc(context), Diagnostics::expected2TokensInPreprocessorDirective,
            TokenType::IntLiterial,
            "default",
            GetDirectiveName(context));
        context->parseError = true;
        return;
    }

    CodePosition directiveLoc = GetDirectiveLoc(context);

    String file;
    if (PeekTokenType(context) == TokenType::EndOfDirective)
    {
        file = directiveLoc.FileName;
    }
    else if (PeekTokenType(context) == TokenType::StringLiterial)
    {
        file = AdvanceToken(context).Content;
    }
    else if (PeekTokenType(context) == TokenType::IntLiterial)
    {
        // Note(tfoley): GLSL allows the "source string" to be indicated by an integer
        // TODO(tfoley): Figure out a better way to handle this, if it matters
        file = AdvanceToken(context).Content;
    }
    else
    {
        Expect(context, TokenType::StringLiterial, Diagnostics::expectedTokenInPreprocessorDirective);
        return;
    }

    PreprocessorInputStream* inputStream = context->preprocessor->inputStream;

    inputStream->isOverridingSourceLoc = true;
    inputStream->overrideFileName = file;
    inputStream->overrideLineOffset = line - (directiveLoc.Line + 1);
}

// Handle a `#pragma` directive
static void HandlePragmaDirective(PreprocessorDirectiveContext* context)
{
    // TODO(tfoley): figure out which pragmas to parse,
    // and which to pass along
    SkipToEndOfLine(context);
}

// Handle a `#version` directive
static void handleGLSLVersionDirective(PreprocessorDirectiveContext* context)
{
    Token versionNumberToken;
    if(!ExpectRaw(
        context,
        TokenType::IntLiterial,
        Diagnostics::expectedTokenInPreprocessorDirective,
        &versionNumberToken))
    {
        return;
    }

    Token glslProfileToken;
    if(PeekTokenType(context) == TokenType::Identifier)
    {
        glslProfileToken = AdvanceToken(context);
    }

    // Need to construct a representation taht we can hook into our compilation result

    auto modifier = new GLSLVersionDirective();
    modifier->versionNumberToken = versionNumberToken;
    modifier->glslProfileToken = glslProfileToken;

    // Attach the modifier to the program we are parsing!

    addModifier(
        context->preprocessor->getSyntax(),
        modifier);
}

// Handle a `#extension` directive, e.g.,
//
//     #extension some_extension_name : enable
//
static void handleGLSLExtensionDirective(PreprocessorDirectiveContext* context)
{
    Token extensionNameToken;
    if(!ExpectRaw(
        context,
        TokenType::Identifier,
        Diagnostics::expectedTokenInPreprocessorDirective,
        &extensionNameToken))
    {
        return;
    }

    if( !ExpectRaw(context, TokenType::Colon, Diagnostics::expectedTokenInPreprocessorDirective) )
    {
        return;
    }

    Token dispositionToken;
    if(!ExpectRaw(
        context,
        TokenType::Identifier,
        Diagnostics::expectedTokenInPreprocessorDirective,
        &dispositionToken))
    {
        return;
    }

    // Need to construct a representation taht we can hook into our compilation result

    auto modifier = new GLSLExtensionDirective();
    modifier->extensionNameToken = extensionNameToken;
    modifier->dispositionToken = dispositionToken;

    // Attach the modifier to the program we are parsing!

    addModifier(
        context->preprocessor->getSyntax(),
        modifier);
}

// Handle an invalid directive
static void HandleInvalidDirective(PreprocessorDirectiveContext* context)
{
    GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::unknownPreprocessorDirective, GetDirectiveName(context));
    SkipToEndOfLine(context);
}

// Callback interface used by preprocessor directives
typedef void (*PreprocessorDirectiveCallback)(PreprocessorDirectiveContext* context);

enum PreprocessorDirectiveFlag : unsigned int
{
    // Should this directive be handled even when skipping disbaled code?
    ProcessWhenSkipping = 1 << 0,
};

// Information about a specific directive
struct PreprocessorDirective
{
    // Name of the directive
    char const*                     name;

    // Callback to handle the directive
    PreprocessorDirectiveCallback   callback;

    unsigned int                    flags;
};

// A simple array of all the directives we know how to handle.
// TODO(tfoley): considering making this into a real hash map,
// and then make it easy-ish for users of the codebase to add
// their own directives as desired.
static const PreprocessorDirective kDirectives[] =
{
    { "if",         &HandleIfDirective,         ProcessWhenSkipping },
    { "ifdef",      &HandleIfDefDirective,      ProcessWhenSkipping },
    { "ifndef",     &HandleIfNDefDirective,     ProcessWhenSkipping },
    { "else",       &HandleElseDirective,       ProcessWhenSkipping },
    { "elif",       &HandleElifDirective,       ProcessWhenSkipping },
    { "endif",      &HandleEndIfDirective,      ProcessWhenSkipping },

    { "import",     &HandleImportDirective,     0 },
    { "include",    &HandleIncludeDirective,    0 },
    { "define",     &HandleDefineDirective,     0 },
    { "undef",      &HandleUndefDirective,      0 },
    { "warning",    &HandleWarningDirective,    0 },
    { "error",      &HandleErrorDirective,      0 },
    { "line",       &HandleLineDirective,       0 },
    { "pragma",     &HandlePragmaDirective,     0 },

    // TODO(tfoley): These are specific to GLSL, and probably
    // shouldn't be enabled for HLSL or Slang
    { "version",    &handleGLSLVersionDirective,    0 },
    { "extension",  &handleGLSLExtensionDirective,  0 },

    { NULL, NULL },
};

static const PreprocessorDirective kInvalidDirective = {
    NULL, &HandleInvalidDirective, 0,
};

// Look up the directive with the given name.
static PreprocessorDirective const* FindDirective(String const& name)
{
    char const* nameStr = name.Buffer();
    for (int ii = 0; kDirectives[ii].name; ++ii)
    {
        if (strcmp(kDirectives[ii].name, nameStr) != 0)
            continue;

        return &kDirectives[ii];
    }

    return &kInvalidDirective;
}

// Process a directive, where the preprocessor has already consumed the
// `#` token that started the directive line.
static void HandleDirective(PreprocessorDirectiveContext* context)
{
    // Try to read the directive name.
    context->directiveToken = PeekRawToken(context);

    TokenType directiveTokenType = GetDirective(context).Type;

    // An empty directive is allowed, and ignored.
    if (directiveTokenType == TokenType::EndOfDirective)
    {
        return;
    }
    // Otherwise the directive name had better be an identifier
    else if (directiveTokenType != TokenType::Identifier)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::expectedPreprocessorDirectiveName);
        SkipToEndOfLine(context);
        return;
    }

    // Consume the directive name token.
    AdvanceRawToken(context);

    // Look up the handler for the directive.
    PreprocessorDirective const* directive = FindDirective(GetDirectiveName(context));

    // If we are skipping disabled code, and the directive is not one
    // of the small number that need to run even in that case, skip it.
    if (IsSkipping(context) && !(directive->flags & PreprocessorDirectiveFlag::ProcessWhenSkipping))
    {
        SkipToEndOfLine(context);
        return;
    }

    // Apply the directive-specific callback
    (directive->callback)(context);

    // We expect the directive callback to consume the entire line, so if
    // it hasn't that is a parse error.
    expectEndOfDirective(context);
}

// Read one token using the full preprocessor, with all its behaviors.
static Token ReadToken(Preprocessor* preprocessor)
{
    for (;;)
    {
        // Look at the next raw token in the input.
        Token const& token = PeekRawToken(preprocessor);

        // If we have a directive (`#` at start of line) then handle it
        if ((token.Type == TokenType::Pound) && (token.flags & TokenFlag::AtStartOfLine))
        {
            // Skip the `#`
            AdvanceRawToken(preprocessor);

            // Create a context for parsing the directive
            PreprocessorDirectiveContext directiveContext;
            directiveContext.preprocessor = preprocessor;
            directiveContext.parseError = false;
            directiveContext.haveDoneEndOfDirectiveChecks = false;

            // Parse and handle the directive
            HandleDirective(&directiveContext);
            continue;
        }

        // otherwise, if we are currently in a skipping mode, then skip tokens
        if (IsSkipping(preprocessor))
        {
            AdvanceRawToken(preprocessor);
            continue;
        }

        // otherwise read a token, which may involve macro expansion
        return AdvanceToken(preprocessor);
    }
}

// intialize a preprocessor context, using the given sink for errros
static void InitializePreprocessor(
    Preprocessor*   preprocessor,
    DiagnosticSink* sink)
{
    preprocessor->sink = sink;
    preprocessor->includeHandler = NULL;
    preprocessor->endOfFileToken.Type = TokenType::EndOfFile;
    preprocessor->endOfFileToken.flags = TokenFlag::AtStartOfLine;
}

// clean up after an environment
PreprocessorEnvironment::~PreprocessorEnvironment()
{
    for (auto pair : this->macros)
    {
        DestroyMacro(NULL, pair.Value);
    }
}

// finalize a preprocessor and free any memory still in use
static void FinalizePreprocessor(
    Preprocessor*   preprocessor)
{
    // Clear out any waiting input streams
    PreprocessorInputStream* input = preprocessor->inputStream;
    while (input)
    {
        PreprocessorInputStream* parent = input->parent;
        DestroyInputStream(preprocessor, input);
        input = parent;
    }

#if 0
    // clean up any macros that were allocated
    for (auto pair : preprocessor->globalEnv.macros)
    {
        DestroyMacro(preprocessor, pair.Value);
    }
#endif
}

// Add a simple macro definition from a string (e.g., for a
// `-D` option passed on the command line
static void DefineMacro(
    Preprocessor*   preprocessor,
    String const&   key,
    String const&   value)
{
    String fileName = "command line";
    PreprocessorMacro* macro = CreateMacro(preprocessor);

    // Use existing `Lexer` to generate a token stream.
    Lexer lexer(fileName, value, GetSink(preprocessor));
    macro->tokens = lexer.lexAllTokens();
    macro->nameToken = Token(TokenType::Identifier, key, 0, 0, 0, fileName);

    PreprocessorMacro* oldMacro = NULL;
    if (preprocessor->globalEnv.macros.TryGetValue(key, oldMacro))
    {
        DestroyMacro(preprocessor, oldMacro);
    }

    preprocessor->globalEnv.macros[key] = macro;
}

// read the entire input into tokens
static TokenList ReadAllTokens(
    Preprocessor*   preprocessor)
{
    TokenList tokens;
    for (;;)
    {
        Token token = ReadToken(preprocessor);

        tokens.mTokens.Add(token);

        // Note: we include the EOF token in the list,
        // since that is expected by the `TokenList` type.
        if (token.Type == TokenType::EndOfFile)
            break;
    }
    return tokens;
}

TokenList preprocessSource(
    String const&               source,
    String const&               fileName,
    DiagnosticSink*             sink,
    IncludeHandler*             includeHandler,
    Dictionary<String, String>  defines,
    TranslationUnitRequest*     translationUnit)
{
    Preprocessor preprocessor;
    InitializePreprocessor(&preprocessor, sink);
    preprocessor.translationUnit = translationUnit;

    preprocessor.includeHandler = includeHandler;
    for (auto p : defines)
    {
        DefineMacro(&preprocessor, p.Key, p.Value);
    }

    // create an initial input stream based on the provided buffer
    preprocessor.inputStream = CreateInputStreamForSource(&preprocessor, source, fileName);

    TokenList tokens = ReadAllTokens(&preprocessor);

    FinalizePreprocessor(&preprocessor);

    // debugging: build the pre-processed source back together
#if 0
    StringBuilder sb;
    for (auto t : tokens)
    {
        if (t.flags & TokenFlag::AtStartOfLine)
        {
            sb << "\n";
        }
        else if (t.flags & TokenFlag::AfterWhitespace)
        {
            sb << " ";
        }
        
        sb << t.Content;
    }

    String s = sb.ProduceString();
#endif

    return tokens;
}

//

// Handle a `#import` directive
static void HandleImportDirective(PreprocessorDirectiveContext* context)
{
    Token pathToken;
    if(!Expect(context, TokenType::StringLiterial, Diagnostics::expectedTokenInPreprocessorDirective, &pathToken))
        return;

    String path = getFileNameTokenValue(pathToken);

    // TODO(tfoley): make this robust in presence of `#line`
    String pathIncludedFrom = GetDirectiveLoc(context).FileName;
    String foundPath;
    String foundSource;

    IncludeHandler* includeHandler = context->preprocessor->includeHandler;
    if (!includeHandler)
    {
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::importFailed, path);
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::noIncludeHandlerSpecified);
        return;
    }
    auto includeResult = includeHandler->TryToFindIncludeFile(path, pathIncludedFrom, &foundPath, &foundSource);

    switch (includeResult)
    {
    case IncludeResult::NotFound:
    case IncludeResult::Error:
        GetSink(context)->diagnose(pathToken.Position, Diagnostics::importFailed, path);
        return;

    case IncludeResult::Found:
        break;
    }

    // Do all checking related to the end of this directive before we push a new stream,
    // just to avoid complications where that check would need to deal with
    // a switch of input stream
    expectEndOfDirective(context);

    // TODO: may want to have some kind of canonicalization step here
    String moduleKey = foundPath;

    // Import code from the chosen file, if needed. We only
    // need to import on the first `#import` directive, and
    // after that we ignore additional `#import`s for the same file.
    {
        auto translationUnit = context->preprocessor->translationUnit;
        auto request = translationUnit->compileRequest;


        // Have we already loaded a module matching this name?
        if (request->mapPathToLoadedModule.TryGetValue(moduleKey))
        {
            // The module has already been loaded, so we don't need to
            // actually tokenize the code here. But note that we *do*
            // go on to insert tokens for an `import` operation into
            // the stream, so it is up to downstream code to avoid
            // re-importing the same thing twice.
        }
        else
        {
            // We are going to preprocess the file using the *same* preprocessor
            // state that is already active. The main alternative would be
            // to construct a fresh preprocessor and use that. The current
            // choice is made so that macros defined in the imported file
            // will be made visible to the importer, rather than disappear
            // when a sub-preprocessor gets finalized.
            auto preprocessor = context->preprocessor;

            // We need to save/restore the input stream, so that we can
            // re-use the preprocessor
            PreprocessorInputStream* savedStream = preprocessor->inputStream;

            // Create an input stream for reading from the imported file
            PreprocessorInputStream* subInputStream = CreateInputStreamForSource(preprocessor, foundSource, foundPath);

            // Now preprocess that stream
            preprocessor->inputStream = subInputStream;
            TokenList subTokens = ReadAllTokens(preprocessor);

            // Restore the previous input stream
            preprocessor->inputStream = savedStream;

            // Now we need to do something with those tokens we read
            request->handlePoundImport(
                moduleKey,
                subTokens);
        }
    }
 
    // Now create a dummy token stream to represent the import request,
    // so that it can be manifest in the user's program
    SourceTextInputStream* inputStream = new SourceTextInputStream();
 
    Token token;
    token.Type = TokenType::PoundImport;
    token.Position = GetDirectiveLoc(context);
    token.flags = 0;
    token.Content = foundPath;
 
    inputStream->lexedTokens.mTokens.Add(token);
 
    token.Type = TokenType::EndOfFile;
    token.flags = TokenFlag::AfterWhitespace | TokenFlag::AtStartOfLine;
    inputStream->lexedTokens.mTokens.Add(token);
 
    inputStream->tokenReader = TokenReader(inputStream->lexedTokens);
 
    inputStream->parent = context->preprocessor->inputStream;
    context->preprocessor->inputStream = inputStream;
}



}
