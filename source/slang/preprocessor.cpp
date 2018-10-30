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
    Dictionary<Name*, PreprocessorMacro*>  macros;

    ~PreprocessorEnvironment();
};

// Input tokens can either come from source text, or from macro expansion.
// In general, input streams can be nested, so we have to keep a conceptual
// stack of input.

struct PrimaryInputStream;

// A stream of input tokens to be consumed
struct PreprocessorInputStream
{
    // The primary input stream that is the parent to this one,
    // or NULL if this stream is itself a primary stream.
    PrimaryInputStream*             primaryStream;

    // The next input stream up the stack, if any.
    PreprocessorInputStream*        parent;

    // Environment to use when looking up macros
    PreprocessorEnvironment*        environment;

    // Destructor is virtual so that we can clean up
    // after concrete subtypes.
    virtual ~PreprocessorInputStream() = default;
};

// A "primary" input stream represents the top-level context of a file
// being parsed, and tracks things like preprocessor conditional state
struct PrimaryInputStream : PreprocessorInputStream
{
    // The next *primary* input stream up the stack
    PrimaryInputStream*             parentPrimaryInputStream;

    // The deepest preprocessor conditional active for this stream.
    PreprocessorConditional*        conditional;

    // The lexer state that will provide input
    Lexer lexer;

    // One token of lookahead
    Token token;
};

// A "secondary" input stream represents code that is being expanded
// into the current scope, but which had already been tokenized before.
//
struct PretokenizedInputStream : PreprocessorInputStream
{
    // Reader for pre-tokenized input
    TokenReader                     tokenReader;
};

// A pre-tokenized input stream that will only be used once, and which
// therefore owns the memory for its tokens.
struct SimpleTokenInputStream : PretokenizedInputStream
{
    // A list of raw tokens that will provide input
    TokenList lexedTokens;
};

struct MacroExpansion : PretokenizedInputStream
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
    NameLoc                     nameAndLoc;

    // Parameters of the macro, in case of a function-like macro
    List<NameLoc>               params;

    // The tokens that make up the macro body
    TokenList                   tokens;

    // The flavor of macro
    PreprocessorMacroFlavor     flavor;

    // The environment in which this macro needs to be expanded.
    // For ordinary macros this will be the global environment,
    // while for function-like macro arguments, it will be
    // the environment of the macro invocation.
    PreprocessorEnvironment*    environment;

    //
    Name* getName()
    {
        return nameAndLoc.name;
    }

    SourceLoc getLoc()
    {
        return nameAndLoc.loc;
    }
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

    // Any paths that have issued `#pragma once` directives to
    // stop them from being included again.
    HashSet<String>                         pragmaOncePaths;


    TranslationUnitRequest* getTranslationUnit()
    {
        return translationUnit;
    }

    ModuleDecl* getSyntax()
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
static bool IsSkipping(Preprocessor* preprocessor);

//
// Basic Input Handling
//

// Create a fresh input stream
static void  initializeInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    inputStream->parent = NULL;
    inputStream->environment = &preprocessor->globalEnv;
}

static void initializePrimaryInputStream(Preprocessor* preprocessor, PrimaryInputStream* inputStream)
{
    initializeInputStream(preprocessor, inputStream);
    inputStream->primaryStream = inputStream;
    inputStream->conditional = NULL;
}

// Destroy an input stream
static void destroyInputStream(Preprocessor* /*preprocessor*/, PreprocessorInputStream* inputStream)
{
    delete inputStream;
}

static NamePool* getNamePool(Preprocessor* preprocessor)
{
    return preprocessor->translationUnit->compileRequest->getNamePool();
}

// Create an input stream to represent a pre-tokenized input file.
// TODO(tfoley): pre-tokenizing files isn't going to work in the long run.
static PreprocessorInputStream* CreateInputStreamForSource(
    Preprocessor*   preprocessor,
    SourceView*     sourceView)
{
    PrimaryInputStream* inputStream = new PrimaryInputStream();
    initializePrimaryInputStream(preprocessor, inputStream);

    // initialize the embedded lexer so that it can generate a token stream
    inputStream->lexer.initialize(sourceView, GetSink(preprocessor), getNamePool(preprocessor));
    inputStream->token = inputStream->lexer.lexToken();

    return inputStream;
}

static PrimaryInputStream* asPrimaryInputStream(PreprocessorInputStream* inputStream)
{
    auto primaryStream = inputStream->primaryStream;
    if(primaryStream == inputStream)
        return primaryStream;
    return nullptr;
}


static void PushInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    inputStream->parent = preprocessor->inputStream;
    if(!asPrimaryInputStream(inputStream))
        inputStream->primaryStream = preprocessor->inputStream->primaryStream;
    preprocessor->inputStream = inputStream;
}

// Called when we reach the end of an input stream.
// Performs some validation and then destroys the input stream if required.
static void EndInputStream(Preprocessor* preprocessor, PreprocessorInputStream* inputStream)
{
    if(auto primaryStream = asPrimaryInputStream(inputStream))
    {
        // If there are any conditionals that weren't completed, then it is an error
        if (primaryStream->conditional)
        {
            PreprocessorConditional* conditional = primaryStream->conditional;

            GetSink(preprocessor)->diagnose(conditional->ifToken.loc, Diagnostics::endOfFileInPreprocessorConditional);

            while (conditional)
            {
                PreprocessorConditional* parent = conditional->parent;
                DestroyConditional(conditional);
                conditional = parent;
            }
        }
    }

    destroyInputStream(preprocessor, inputStream);
}

// Consume one token from an input stream
static Token AdvanceRawToken(PreprocessorInputStream* inputStream, LexerFlags lexerFlags = 0)
{
    if( auto primaryStream = asPrimaryInputStream(inputStream) )
    {
        auto result = primaryStream->token;
        primaryStream->token = primaryStream->lexer.lexToken(lexerFlags);
        return result;
    }
    else
    {
        PretokenizedInputStream* pretokenized = (PretokenizedInputStream*) inputStream;
        return pretokenized->tokenReader.AdvanceToken();
    }
}

// Peek one token from an input stream
static Token PeekRawToken(PreprocessorInputStream* inputStream)
{
    if( auto primaryStream = asPrimaryInputStream(inputStream) )
    {
        return primaryStream->token;
    }
    else
    {
        PretokenizedInputStream* pretokenized = (PretokenizedInputStream*) inputStream;
        return pretokenized->tokenReader.PeekToken();
    }
}

// Peek one token type from an input stream
static TokenType PeekRawTokenType(PreprocessorInputStream* inputStream)
{
    if( auto primaryStream = asPrimaryInputStream(inputStream) )
    {
        return primaryStream->token.type;
    }
    else
    {
        PretokenizedInputStream* pretokenized = (PretokenizedInputStream*) inputStream;
        return pretokenized->tokenReader.PeekTokenType();
    }
}


// Read one token in "raw" mode (meaning don't expand macros)
static Token AdvanceRawToken(Preprocessor* preprocessor, LexerFlags lexerFlags = 0)
{
    for(;;)
    {
        // Look at the input stream on top of the stack
        PreprocessorInputStream* inputStream = preprocessor->inputStream;

        // If there isn't one, then there is no more input left to read.
        if(!inputStream)
        {
            return preprocessor->endOfFileToken;
        }

        // The top-most input stream may be at its end
        if(PeekRawTokenType(inputStream) == TokenType::EndOfFile)
        {
            // If there is another stream remaining, switch to it
            if(inputStream->parent)
            {
                preprocessor->inputStream = inputStream->parent;
                EndInputStream(preprocessor, inputStream);
                continue;
            }
        }

        // Everything worked, so read a token from the top-most stream
        return AdvanceRawToken(
            inputStream,
            lexerFlags | (IsSkipping(preprocessor) ? kLexerFlag_IgnoreInvalid : 0));
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

// Get the location of the current (raw) token
static SourceLoc PeekLoc(Preprocessor* preprocessor)
{
    return PeekRawToken(preprocessor).loc;
}

// Get the `TokenType` of the current (raw) token
static TokenType PeekRawTokenType(Preprocessor* preprocessor)
{
    return PeekRawToken(preprocessor).type;
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
static PreprocessorMacro* LookupMacro(PreprocessorEnvironment* environment, Name* name)
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
        if (PeekRawTokenType(inputStream) == TokenType::EndOfFile)
        {
            inputStream = inputStream->parent;
            continue;
        }

        // If we've found an active stream that isn't at its end,
        // then use that for lookup.
        return inputStream->environment;
    }
}

static PreprocessorMacro* LookupMacro(Preprocessor* preprocessor, Name* name)
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
    initializeInputStream(preprocessor, expansion);

    expansion->parent = preprocessor->inputStream;
    expansion->primaryStream = preprocessor->inputStream->primaryStream;

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
    token.type = TokenType::EndOfFile;
    macro->tokens.mTokens.Add(token);
}

static SimpleTokenInputStream* createSimpleInputStream(
    Preprocessor*   preprocessor,
    Token const&    token)
{
    SimpleTokenInputStream* inputStream = new SimpleTokenInputStream();
    initializeInputStream(preprocessor, inputStream);

    inputStream->lexedTokens.mTokens.Add(token);

    Token eofToken;
    eofToken.type = TokenType::EndOfFile;
    eofToken.loc = token.loc;
    eofToken.flags = TokenFlag::AfterWhitespace | TokenFlag::AtStartOfLine;
    inputStream->lexedTokens.mTokens.Add(eofToken);

    inputStream->tokenReader = TokenReader(inputStream->lexedTokens);

    return inputStream;
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
        Token token = PeekRawToken(preprocessor);

        // Not an identifier? Can't be a macro.
        if (token.type != TokenType::Identifier)
            return;

        // Look for a macro with the given name.
        Name* name = token.getName();
        PreprocessorMacro* macro = LookupMacro(preprocessor, name);

        // Not a macro? Can't be an invocation.
        if (!macro)
            return;

        // If the macro is busy (already being expanded),
        // don't try to trigger recursive expansion
        if (IsMacroBusy(macro))
            return;

        // We might already have looked at this token,
        // and need to suppress expansion
        if (token.flags & TokenFlag::SuppressMacroExpansion)
            return;

        // A function-style macro invocation should only match
        // if the token *after* the identifier is `(`. This
        // requires more lookahead than we usually have/need
        if (macro->flavor == PreprocessorMacroFlavor::FunctionLike)
        {
            // Consume the token that (possibly) triggered macro expansion
            AdvanceRawToken(preprocessor);

            // Look at the next token, and see if it is an opening `(`
            // that indicates we should actually expand a macro.
            if(PeekRawTokenType(preprocessor) != TokenType::LParent)
            {
                // In this case, we are in a bit of a mess, because we have
                // consumed the token that named the macro, but we need to
                // make sure that token (and not whatever came after it)
                // gets returned to the user.
                //
                // To work around this we will construct a short-lived input
                // stream just to handle that one token, and also set
                // a flag on the token to keep us from doing this logic again.

                token.flags |= TokenFlag::SuppressMacroExpansion;

                SimpleTokenInputStream* simpleStream = createSimpleInputStream(preprocessor, token);
                PushInputStream(preprocessor, simpleStream);
                return;
            }

            // Consume the opening `(`
            Token leftParen = AdvanceRawToken(preprocessor);

            FunctionLikeMacroExpansion* expansion = new FunctionLikeMacroExpansion();
            InitializeMacroExpansion(preprocessor, expansion, macro);
            expansion->argumentEnvironment.parent = &preprocessor->globalEnv;
            expansion->environment = &expansion->argumentEnvironment;

            // Try to read any arguments present.
            UInt paramCount = macro->params.Count();
            UInt argIndex = 0;

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
                    NameLoc paramNameAndLoc = macro->params[argIndex];
                    Name* paramName = paramNameAndLoc.name;
                    arg->nameAndLoc = paramNameAndLoc;
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

            UInt argCount = argIndex;
            if (argCount != paramCount)
            {
                GetSink(preprocessor)->diagnose(PeekLoc(preprocessor), Diagnostics::wrongNumberOfArgumentsToMacro, paramCount, argCount);
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

        SourceManager* sourceManager = preprocessor->getCompileRequest()->getSourceManager();

        // We create a dummy file to represent the token-paste operation
        PathInfo pathInfo = PathInfo::makeTokenPaste();
       
        SourceFile* sourceFile = sourceManager->createSourceFile(pathInfo, sb.ProduceString());

        SourceView* sourceView = sourceManager->createSourceView(sourceFile);

        Lexer lexer;
        lexer.initialize(sourceView, GetSink(preprocessor), getNamePool(preprocessor));

        SimpleTokenInputStream* inputStream = new SimpleTokenInputStream();
        initializeInputStream(preprocessor, inputStream);

        inputStream->lexedTokens = lexer.lexAllTokens();
        inputStream->tokenReader = TokenReader(inputStream->lexedTokens);

        // We expect the reuslt of lexing to be two tokens: one for the actual value,
        // and one for the end-of-input marker.
        if (inputStream->tokenReader.GetCount() != 2)
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
    return PeekToken(preprocessor).type;
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
inline SourceLoc const& GetDirectiveLoc(PreprocessorDirectiveContext* context)
{
    return context->directiveToken.loc;
}

// Wrapper to get the diagnostic sink in the context of a directive.
static inline DiagnosticSink* GetSink(PreprocessorDirectiveContext* context)
{
    return GetSink(context->preprocessor);
}

// Wrapper to get a "current" location when parsing a directive
static SourceLoc PeekLoc(PreprocessorDirectiveContext* context)
{
    return PeekLoc(context->preprocessor);
}

// Wrapper to look up a macro in the context of a directive.
static PreprocessorMacro* LookupMacro(PreprocessorDirectiveContext* context, Name* name)
{
    return LookupMacro(context->preprocessor, name);
}

// Determine if we have read everthing on the directive's line.
static bool IsEndOfLine(PreprocessorDirectiveContext* context)
{
    return PeekRawToken(context->preprocessor).type == TokenType::EndOfDirective;
}

// Peek one raw token in a directive, without going past the end of the line.
static Token PeekRawToken(PreprocessorDirectiveContext* context)
{
    return PeekRawToken(context->preprocessor);
}

// Read one raw token in a directive, without going past the end of the line.
static Token AdvanceRawToken(PreprocessorDirectiveContext* context, LexerFlags lexerFlags = 0)
{
    if (IsEndOfLine(context))
        return PeekRawToken(context);
    return AdvanceRawToken(context->preprocessor, lexerFlags);
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
        return context->preprocessor->endOfFileToken;
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

    PrimaryInputStream* primaryStream = inputStream->primaryStream;
    if(!primaryStream) return false;

    // If we are not inside a preprocessor conditional, then don't skip
    PreprocessorConditional* conditional = primaryStream->conditional;
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
    SLANG_ASSERT(inputStream);

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
    auto primaryStream = inputStream->primaryStream;
    conditional->parent = primaryStream->conditional;
    primaryStream->conditional = conditional;
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
                GetSink(context)->diagnose(leftParen.loc, Diagnostics::seeOpeningToken, leftParen);
            }
            return value;
        }

    case TokenType::IntegerLiteral:
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
                Name* name = nameToken.getName();

                // If we saw an opening `(`, then expect one to close
                if (leftParen.type != TokenType::Unknown)
                {
                    if(!ExpectRaw(context, TokenType::RParent, Diagnostics::expectedTokenInDefinedExpression))
                    {
                        GetSink(context)->diagnose(leftParen.loc, Diagnostics::seeOpeningToken, leftParen);
                        return 0;
                    }
                }

                return LookupMacro(context, name) != NULL;
            }

            // An identifier here means it was not defined as a macro (or
            // it is defined, but as a function-like macro. These should
            // just evaluate to zero (possibly with a warning)
            GetSink(context)->diagnose(token.loc, Diagnostics::undefinedIdentifierInPreprocessorExpression, token.getName());
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
    switch (opToken.type)
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
    switch (opToken.type)
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
                GetSink(context)->diagnose(opToken.loc, Diagnostics::divideByZeroInPreprocessorExpression);
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
                GetSink(context)->diagnose(opToken.loc, Diagnostics::divideByZeroInPreprocessorExpression);
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
    case TokenType::OpGeq:      return left >= right ? 1 : 0;
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
    Name* name = nameToken.getName();

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
    Name* name = nameToken.getName();

    // Check if the name is defined.
    beginConditional(context, LookupMacro(context, name) == NULL);
}

// Handle a `#else` directive
static void HandleElseDirective(PreprocessorDirectiveContext* context)
{
    PreprocessorInputStream* inputStream = context->preprocessor->inputStream;
    SLANG_ASSERT(inputStream);

    // if we aren't inside a conditional, then error
    PreprocessorConditional* conditional = inputStream->primaryStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    // if we've already seen a `#else`, then it is an error
    if (conditional->elseToken.type != TokenType::Unknown)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveAfterElse, GetDirectiveName(context));
        GetSink(context)->diagnose(conditional->elseToken.loc, Diagnostics::seeDirective);
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
    SLANG_ASSERT(inputStream);

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
    PreprocessorConditional* conditional = inputStream->primaryStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    // if we've already seen a `#else`, then it is an error
    if (conditional->elseToken.type != TokenType::Unknown)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveAfterElse, GetDirectiveName(context));
        GetSink(context)->diagnose(conditional->elseToken.loc, Diagnostics::seeDirective);
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
    SLANG_ASSERT(inputStream);

    // if we aren't inside a conditional, then error
    PreprocessorConditional* conditional = inputStream->primaryStream->conditional;
    if (!conditional)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::directiveWithoutIf, GetDirectiveName(context));
        return;
    }

    inputStream->primaryStream->conditional = conditional->parent;
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

// Handle a `#include` directive
static void HandleIncludeDirective(PreprocessorDirectiveContext* context)
{
    // Consume the directive, and inform the lexer to process the remainder of the line as a file path.
    AdvanceRawToken(context, kLexerFlag_ExpectFileName);

    Token pathToken;
    if(!Expect(context, TokenType::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective, &pathToken))
        return;

    String path = getFileNameTokenValue(pathToken);

    auto directiveLoc = GetDirectiveLoc(context);
    
    PathInfo includedFromPathInfo = context->preprocessor->translationUnit->compileRequest->getSourceManager()->getPathInfo(directiveLoc, SourceLocType::Actual);
    
    IncludeHandler* includeHandler = context->preprocessor->includeHandler;
    if (!includeHandler)
    {
        GetSink(context)->diagnose(pathToken.loc, Diagnostics::includeFailed, path);
        GetSink(context)->diagnose(pathToken.loc, Diagnostics::noIncludeHandlerSpecified);
        return;
    }
    
    /* Find the path relative to the foundPath */
    PathInfo filePathInfo;
    if (SLANG_FAILED(includeHandler->findFile(path, includedFromPathInfo.foundPath, filePathInfo)))
    {
        GetSink(context)->diagnose(pathToken.loc, Diagnostics::includeFailed, path);
        return;
    }

    // We must have a canonical path to be compare
    if (!filePathInfo.hasCanonicalPath())
    {
        GetSink(context)->diagnose(pathToken.loc, Diagnostics::noCanonicalPath, path);
        return;
    }

    // Do all checking related to the end of this directive before we push a new stream,
    // just to avoid complications where that check would need to deal with
    // a switch of input stream
    expectEndOfDirective(context);

    // Check whether we've previously included this file and seen a `#pragma once` directive
    if(context->preprocessor->pragmaOncePaths.Contains(filePathInfo.canonicalPath))
    {
        return;
    }

    // Push the new file onto our stack of input streams
    // TODO(tfoley): check if we have made our include stack too deep
    auto sourceManager = context->preprocessor->getCompileRequest()->getSourceManager();

    // See if this an already loaded source file
    SourceFile* sourceFile = sourceManager->findSourceFileRecursively(filePathInfo.canonicalPath);
    // If not create a new one, and add to the list of known source files
    if (!sourceFile)
    {
        ComPtr<ISlangBlob> foundSourceBlob;
        if (SLANG_FAILED(includeHandler->readFile(filePathInfo.foundPath, foundSourceBlob.writeRef())))
        {
            GetSink(context)->diagnose(pathToken.loc, Diagnostics::includeFailed, path);
            return;
        }

        sourceFile = sourceManager->createSourceFile(filePathInfo, foundSourceBlob);
        sourceManager->addSourceFile(filePathInfo.canonicalPath, sourceFile);
    }

    // This is a new parse (even if it's a pre-existing source file), so create a new SourceUnit
    SourceView* sourceView = sourceManager->createSourceView(sourceFile);

    PreprocessorInputStream* inputStream = CreateInputStreamForSource(context->preprocessor, sourceView);
    inputStream->parent = context->preprocessor->inputStream;
    context->preprocessor->inputStream = inputStream;
}

// Handle a `#define` directive
static void HandleDefineDirective(PreprocessorDirectiveContext* context)
{
    Token nameToken;
    if (!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    Name* name = nameToken.getName();

    PreprocessorMacro* macro = CreateMacro(context->preprocessor);
    macro->nameAndLoc = NameLoc(nameToken);

    PreprocessorMacro* oldMacro = LookupMacro(&context->preprocessor->globalEnv, name);
    if (oldMacro)
    {
        GetSink(context)->diagnose(nameToken.loc, Diagnostics::macroRedefinition, name);
        GetSink(context)->diagnose(oldMacro->getLoc(), Diagnostics::seePreviousDefinitionOf, name);

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
        if( token.type == TokenType::EndOfDirective )
        {
            // Last token on line will be turned into a conceptual end-of-file
            // token for the sub-stream that the macro expands into.
            token.type = TokenType::EndOfFile;
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
    if (!ExpectRaw(context, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, &nameToken))
        return;
    Name* name = nameToken.getName();

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
        GetSink(context)->diagnose(nameToken.loc, Diagnostics::macroNotDefined, name);
    }
}

// Handle a `#warning` directive
static void HandleWarningDirective(PreprocessorDirectiveContext* context)
{
    // Consume the directive, and inform the lexer to process the remainder of the line as a custom message.
    AdvanceRawToken(context, kLexerFlag_ExpectDirectiveMessage);

    // Read the message token.
    Token messageToken;
    Expect(context, TokenType::DirectiveMessage, Diagnostics::expectedTokenInPreprocessorDirective, &messageToken);

    // Report the custom error.
    GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::userDefinedWarning, messageToken.Content);
}

// Handle a `#error` directive
static void HandleErrorDirective(PreprocessorDirectiveContext* context)
{
    // Consume the directive, and inform the lexer to process the remainder of the line as a custom message.
    AdvanceRawToken(context, kLexerFlag_ExpectDirectiveMessage);

    // Read the message token.
    Token messageToken;
    Expect(context, TokenType::DirectiveMessage, Diagnostics::expectedTokenInPreprocessorDirective, &messageToken);

    // Report the custom error.
    GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::userDefinedError, messageToken.Content);
}

// Handle a `#line` directive
static void HandleLineDirective(PreprocessorDirectiveContext* context)
{
    auto inputStream = context->preprocessor->inputStream;

    int line = 0;

    SourceLoc directiveLoc = GetDirectiveLoc(context);

    // `#line <integer-literal> ...`
    if (PeekTokenType(context) == TokenType::IntegerLiteral)
    {
        line = StringToInt(AdvanceToken(context).Content);
    }
    // `#line`
    // `#line default`
    else if (
        PeekTokenType(context) == TokenType::EndOfDirective
        || (PeekTokenType(context) == TokenType::Identifier
            && PeekToken(context).Content == "default"))
    {
        AdvanceToken(context);

        // Stop overriding source locations.
        auto sourceView = inputStream->primaryStream->lexer.sourceView;
        sourceView->addDefaultLineDirective(directiveLoc);
        return;
    }
    else
    {
        GetSink(context)->diagnose(PeekLoc(context), Diagnostics::expected2TokensInPreprocessorDirective,
            TokenType::IntegerLiteral,
            "default",
            GetDirectiveName(context));
        context->parseError = true;
        return;
    }

    auto sourceManager = context->preprocessor->translationUnit->compileRequest->getSourceManager();
    
    String file;
    if (PeekTokenType(context) == TokenType::EndOfDirective)
    {
        file = sourceManager->getPathInfo(directiveLoc).foundPath;
    }
    else if (PeekTokenType(context) == TokenType::StringLiteral)
    {
        file = getStringLiteralTokenValue(AdvanceToken(context));
    }
    else if (PeekTokenType(context) == TokenType::IntegerLiteral)
    {
        // Note(tfoley): GLSL allows the "source string" to be indicated by an integer
        // TODO(tfoley): Figure out a better way to handle this, if it matters
        file = AdvanceToken(context).Content;
    }
    else
    {
        Expect(context, TokenType::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
        return;
    }

    auto sourceView = inputStream->primaryStream->lexer.sourceView;
    sourceView->addLineDirective(directiveLoc, file, line);
}

#define SLANG_PRAGMA_DIRECTIVE_CALLBACK(NAME) \
    void NAME(PreprocessorDirectiveContext* context, Token subDirectiveToken)

// Callback interface used by `#pragma` directives
typedef SLANG_PRAGMA_DIRECTIVE_CALLBACK((*PragmaDirectiveCallback));

SLANG_PRAGMA_DIRECTIVE_CALLBACK(handleUnknownPragmaDirective)
{
    GetSink(context)->diagnose(subDirectiveToken, Diagnostics::unknownPragmaDirectiveIgnored, subDirectiveToken.getName());
    SkipToEndOfLine(context);
    return;
}

SLANG_PRAGMA_DIRECTIVE_CALLBACK(handlePragmaOnceDirective)
{
    // We need to identify the path of the file we are preprocessing,
    // so that we can avoid including it again.
    //
    // Note: for now we are doing a very simplistic check where
    // we use the raw file path as the key for our duplicate checking.
    //
    // TODO: a more refined implementation should probably apply Unicode
    // normalization and case-folding to the path, and then use that
    // plus a hash of the file contents to determine whether things
    // represent the "same" file.
    //
    // TODO: even for our simplistic implementation, we need to add
    // logic to deal with `../` segments in path names to detect
    // trivial cases of the "same" path.
    //
    auto directiveLoc = GetDirectiveLoc(context);
    auto issuedFromPathInfo = context->preprocessor->translationUnit->compileRequest->getSourceManager()->getPathInfo(directiveLoc, SourceLocType::Actual);

    // Must have a canonical path for a #pragma once to work
    if (!issuedFromPathInfo.hasCanonicalPath())
    {
        GetSink(context)->diagnose(subDirectiveToken, Diagnostics::pragmaOnceIgnored);
        return;
    }

    context->preprocessor->pragmaOncePaths.Add(issuedFromPathInfo.canonicalPath);
}

// Information about a specific `#pragma` directive
struct PragmaDirective
{
    // name of the directive
    char const*             name;

    // Callback to handle the directive
    PragmaDirectiveCallback callback;
};

// A simple array of all the  `#pragma` directives we know how to handle.
static const PragmaDirective kPragmaDirectives[] =
{
    { "once", &handlePragmaOnceDirective },

    { NULL, NULL },
};

static const PragmaDirective kUnknownPragmaDirective = {
    NULL, &handleUnknownPragmaDirective,
};

// Look up the `#pragma` directive with the given name.
static PragmaDirective const* findPragmaDirective(String const& name)
{
    char const* nameStr = name.Buffer();
    for (int ii = 0; kPragmaDirectives[ii].name; ++ii)
    {
        if (strcmp(kPragmaDirectives[ii].name, nameStr) != 0)
            continue;

        return &kPragmaDirectives[ii];
    }

    return &kUnknownPragmaDirective;
}

// Handle a `#pragma` directive
static void HandlePragmaDirective(PreprocessorDirectiveContext* context)
{
    // Try to read the sub-directive name.
    Token subDirectiveToken = PeekRawToken(context);

    // The sub-directive had better be an identifier
    if (subDirectiveToken.type != TokenType::Identifier)
    {
        GetSink(context)->diagnose(GetDirectiveLoc(context), Diagnostics::expectedPragmaDirectiveName);
        SkipToEndOfLine(context);
        return;
    }
    AdvanceRawToken(context);

    // Look up the handler for the sub-directive.
    PragmaDirective const* subDirective = findPragmaDirective(subDirectiveToken.getName()->text);

    // Apply the sub-directive-specific callback
    (subDirective->callback)(context, subDirectiveToken);
}

// Handle a `#version` directive
static void handleGLSLVersionDirective(PreprocessorDirectiveContext* context)
{
    Token versionNumberToken;
    if(!ExpectRaw(
        context,
        TokenType::IntegerLiteral,
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

    /// Allow the handler for this directive to advance past the
    /// directive token itself, so that it can control lexer behavior
    /// more closely.
    DontConsumeDirectiveAutomatically = 1 << 1,
};

// Information about a specific directive
struct PreprocessorDirective
{
    // name of the directive
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

    { "include",    &HandleIncludeDirective,    DontConsumeDirectiveAutomatically },
    { "define",     &HandleDefineDirective,     0 },
    { "undef",      &HandleUndefDirective,      0 },
    { "warning",    &HandleWarningDirective,    DontConsumeDirectiveAutomatically },
    { "error",      &HandleErrorDirective,      DontConsumeDirectiveAutomatically },
    { "line",       &HandleLineDirective,       0 },
    { "pragma",     &HandlePragmaDirective,     0 },

    // TODO(tfoley): These are specific to GLSL, and probably
    // shouldn't be enabled for HLSL or Slang
    { "version",    &handleGLSLVersionDirective,    0 },
    { "extension",  &handleGLSLExtensionDirective,  0 },

    { nullptr, nullptr, 0 },
};

static const PreprocessorDirective kInvalidDirective = {
    nullptr, &HandleInvalidDirective, 0,
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

    TokenType directiveTokenType = GetDirective(context).type;

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

    // Look up the handler for the directive.
    PreprocessorDirective const* directive = FindDirective(GetDirectiveName(context));

    // If we are skipping disabled code, and the directive is not one
    // of the small number that need to run even in that case, skip it.
    if (IsSkipping(context) && !(directive->flags & PreprocessorDirectiveFlag::ProcessWhenSkipping))
    {
        SkipToEndOfLine(context);
        return;
    }

    if(!(directive->flags & PreprocessorDirectiveFlag::DontConsumeDirectiveAutomatically))
    {
        // Consume the directive name token.
        AdvanceRawToken(context);
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
        // Depending on what the lookahead token is, we
        // might need to start expanding it.
        //
        // Note: doing this at the start of this loop
        // is important, in case a macro has an empty
        // expansion, and we end up looking at a different
        // token after applying the expansion.
        if(!IsSkipping(preprocessor))
        {
            MaybeBeginMacroExpansion(preprocessor);
        }

        // Look at the next raw token in the input.
        Token const& token = PeekRawToken(preprocessor);
        if (token.type == TokenType::EndOfFile)
            return token;

        // If we have a directive (`#` at start of line) then handle it
        if ((token.type == TokenType::Pound) && (token.flags & TokenFlag::AtStartOfLine))
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
    preprocessor->endOfFileToken.type = TokenType::EndOfFile;
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
        EndInputStream(preprocessor, input);
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
    PathInfo pathInfo = PathInfo::makeCommandLine();
    
    PreprocessorMacro* macro = CreateMacro(preprocessor);

    auto sourceManager = preprocessor->translationUnit->compileRequest->getSourceManager();

    SourceFile* keyFile = sourceManager->createSourceFile(pathInfo, key);
    SourceFile* valueFile = sourceManager->createSourceFile(pathInfo, value);

    SourceView* keyView = sourceManager->createSourceView(keyFile);
    SourceView* valueView = sourceManager->createSourceView(valueFile);

    // Use existing `Lexer` to generate a token stream.
    Lexer lexer;
    lexer.initialize(valueView, GetSink(preprocessor), getNamePool(preprocessor));
    macro->tokens = lexer.lexAllTokens();

    Name* keyName = preprocessor->translationUnit->compileRequest->getNamePool()->getName(key);

    macro->nameAndLoc.name = keyName;
    macro->nameAndLoc.loc = keyView->getRange().begin;
    
    PreprocessorMacro* oldMacro = NULL;
    if (preprocessor->globalEnv.macros.TryGetValue(keyName, oldMacro))
    {
        DestroyMacro(preprocessor, oldMacro);
    }

    preprocessor->globalEnv.macros[keyName] = macro;
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
        if (token.type == TokenType::EndOfFile)
            break;
    }
    return tokens;
}

TokenList preprocessSource(
    SourceFile*                 file,
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

    SourceManager* sourceManager = translationUnit->compileRequest->getSourceManager();

    SourceView* sourceView = sourceManager->createSourceView(file);

    // create an initial input stream based on the provided buffer
    preprocessor.inputStream = CreateInputStreamForSource(&preprocessor, sourceView);

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

}
