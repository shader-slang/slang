#include "slang-parser.h"

#include <assert.h>
#include <float.h>

#include "slang-compiler.h"
#include "slang-lookup.h"
#include "slang-visitor.h"

#include "../core/slang-semantic-version.h"

namespace Slang
{
    // pre-declare
    static Name* getName(Parser* parser, String const& text);

    // Helper class useful to build a list of modifiers. 
    struct ModifierListBuilder
    {
        ModifierListBuilder()
        {
            m_next = &m_result;
        }
        void add(Modifier* modifier)
        {
            // Doesn't handle SharedModifiers
            SLANG_ASSERT(as<SharedModifiers>(modifier) == nullptr);

            // Splice at end
            *m_next = modifier;
            m_next = &modifier->next;
        }
        template <typename T>
        T* find() const
        {
            Modifier* cur = m_result;
            while (cur)
            {
                T* castCur = as<T>(cur);
                if (castCur)
                {
                    return castCur;
                }
                cur = cur->next;
            }
            return nullptr;
        }
        template <typename T>
        bool hasType() const
        {
            return find<T>() != nullptr;
        }
        Modifier* getFirst() { return m_result; };
    protected:

        Modifier* m_result = nullptr;
        Modifier** m_next;
    };

    enum Precedence : int
    {
        Invalid = -1,
        Comma,
        Assignment,
        TernaryConditional,
        LogicalOr,
        LogicalAnd,
        BitOr,
        BitXor,
        BitAnd,
        EqualityComparison,
        RelationalComparison,
        BitShift,
        Additive,
        Multiplicative,
        Prefix,
        Postfix,
    };

    // TODO: implement two pass parsing for file reference and struct type recognition

    class Parser
    {
    public:
        NamePool*       namePool;
        SourceLanguage  sourceLanguage;
        ASTBuilder*     astBuilder;

        NamePool* getNamePool() { return namePool; }
        SourceLanguage getSourceLanguage() { return sourceLanguage; }

        int anonymousCounter = 0;

        RefPtr<Scope> outerScope;
        RefPtr<Scope> currentScope;

        TokenReader tokenReader;
        DiagnosticSink* sink;
        int genericDepth = 0;

        // Have we seen any `import` declarations? If so, we need
        // to parse function bodies completely, even if we are in
        // "rewrite" mode.
        bool haveSeenAnyImportDecls = false;

        // Is the parser in a "recovering" state?
        // During recovery we don't emit additional errors, until we find
        // a token that we expected, when we exit recovery.
        bool isRecovering = false;

        void FillPosition(SyntaxNode * node)
        {
            node->loc = tokenReader.peekLoc();
        }
        void PushScope(ContainerDecl* containerDecl)
        {
            RefPtr<Scope> newScope = new Scope();
            newScope->containerDecl = containerDecl;
            newScope->parent = currentScope;

            currentScope = newScope;
        }

        void pushScopeAndSetParent(ContainerDecl* containerDecl)
        {
            containerDecl->parentDecl = currentScope->containerDecl;
            PushScope(containerDecl);
        }

        void PopScope()
        {
            currentScope = currentScope->parent;
        }
        Parser(
            ASTBuilder* inAstBuilder,
            TokenSpan const& _tokens,
            DiagnosticSink * sink,
            RefPtr<Scope> const& outerScope)
            : tokenReader(_tokens)
            , astBuilder(inAstBuilder)
            , sink(sink)
            , outerScope(outerScope)
        {}
        Parser(const Parser & other) = default;

        //Session* getSession() { return m_session; }

        Token ReadToken();
        Token ReadToken(TokenType type);
        Token ReadToken(const char * string);
        bool LookAheadToken(TokenType type, int offset = 0);
        bool LookAheadToken(const char * string, int offset = 0);
        void                                        parseSourceFile(ModuleDecl* program);
        Decl*					ParseStruct();
        ClassDecl*					    ParseClass();
        Stmt*					ParseStatement();
        Stmt*			        parseBlockStatement();
        DeclStmt*			parseVarDeclrStatement(Modifiers modifiers);
        IfStmt*				parseIfStatement();
        ForStmt*				ParseForStatement();
        WhileStmt*			ParseWhileStatement();
        DoWhileStmt*			ParseDoWhileStatement();
        BreakStmt*			ParseBreakStatement();
        ContinueStmt*			ParseContinueStatement();
        ReturnStmt*			ParseReturnStatement();
        ExpressionStmt*		ParseExpressionStatement();
        Expr*				ParseExpression(Precedence level = Precedence::Comma);

        // Parse an expression that might be used in an initializer or argument context, so we should avoid operator-comma
        inline Expr*			ParseInitExpr() { return ParseExpression(Precedence::Assignment); }
        inline Expr*			ParseArgExpr()  { return ParseExpression(Precedence::Assignment); }

        Expr*				ParseLeafExpression();
        ParamDecl*					ParseParameter();
        Expr*				ParseType();
        TypeExp										ParseTypeExp();

        Parser & operator = (const Parser &) = delete;
    };

    // Forward Declarations

        /// Parse declarations making up the body of `parent`, up to the matching `closingToken`
    static void parseDecls(
        Parser*         parser,
        ContainerDecl*  parent,
        TokenType	    closingToken);

        /// Parse a body consisting of declarations enclosed in `{}`, as the children of `parent`.
    static void parseDeclBody(
        Parser*         parser,
        ContainerDecl*  parent);

    static Decl* parseEnumDecl(Parser* parser);

    static Modifier* ParseOptSemantics(
        Parser* parser);

    static void ParseOptSemantics(
        Parser* parser,
        Decl*	decl);

    static DeclBase* ParseDecl(
        Parser*			parser,
        ContainerDecl*	containerDecl);

    static Decl* ParseSingleDecl(
        Parser*			parser,
        ContainerDecl*	containerDecl);

    static void parseModernParamList(
        Parser*         parser,
        CallableDecl*   decl);

    //

    static void Unexpected(
        Parser*     parser)
    {
        // Don't emit "unexpected token" errors if we are in recovering mode
        if (!parser->isRecovering)
        {
            parser->sink->diagnose(parser->tokenReader.peekLoc(), Diagnostics::unexpectedToken,
                parser->tokenReader.peekTokenType());

            // Switch into recovery mode, to suppress additional errors
            parser->isRecovering = true;
        }
    }

    static void Unexpected(
        Parser*     parser,
        char const* expected)
    {
        // Don't emit "unexpected token" errors if we are in recovering mode
        if (!parser->isRecovering)
        {
            parser->sink->diagnose(parser->tokenReader.peekLoc(), Diagnostics::unexpectedTokenExpectedTokenName,
                parser->tokenReader.peekTokenType(),
                expected);

            // Switch into recovery mode, to suppress additional errors
            parser->isRecovering = true;
        }
    }

    static void Unexpected(
        Parser*     parser,
        TokenType    expected)
    {
        // Don't emit "unexpected token" errors if we are in recovering mode
        if (!parser->isRecovering)
        {
            parser->sink->diagnose(parser->tokenReader.peekLoc(), Diagnostics::unexpectedTokenExpectedTokenType,
                parser->tokenReader.peekTokenType(),
                expected);

            // Switch into recovery mode, to suppress additional errors
            parser->isRecovering = true;
        }
    }

    static TokenType SkipToMatchingToken(TokenReader* reader, TokenType tokenType);

    // Skip a singel balanced token, which is either a single token in
    // the common case, or a matched pair of tokens for `()`, `[]`, and `{}`
    static TokenType SkipBalancedToken(
        TokenReader* reader)
    {
        TokenType tokenType = reader->advanceToken().type;
        switch (tokenType)
        {
        default:
            break;

        case TokenType::LParent:    tokenType = SkipToMatchingToken(reader, TokenType::RParent);    break;
        case TokenType::LBrace:     tokenType = SkipToMatchingToken(reader, TokenType::RBrace);     break;
        case TokenType::LBracket:   tokenType = SkipToMatchingToken(reader, TokenType::RBracket);   break;
        }
        return tokenType;
    }

    // Skip balanced
    static TokenType SkipToMatchingToken(
        TokenReader*                reader,
        TokenType    tokenType)
    {
        for (;;)
        {
            if (reader->isAtEnd()) return TokenType::EndOfFile;
            if (reader->peekTokenType() == tokenType)
            {
                reader->advanceToken();
                return tokenType;
            }
            SkipBalancedToken(reader);
        }
    }

    // Is the given token type one that is used to "close" a
    // balanced construct.
    static bool IsClosingToken(TokenType tokenType)
    {
        switch (tokenType)
        {
        case TokenType::EndOfFile:
        case TokenType::RBracket:
        case TokenType::RParent:
        case TokenType::RBrace:
            return true;

        default:
            return false;
        }
    }


    // Expect an identifier token with the given content, and consume it.
    Token Parser::ReadToken(const char* expected)
    {
        if (tokenReader.peekTokenType() == TokenType::Identifier
                && tokenReader.peekToken().getContent() == expected)
        {
            isRecovering = false;
            return tokenReader.advanceToken();
        }

        if (!isRecovering)
        {
            Unexpected(this, expected);
            return tokenReader.peekToken();
        }
        else
        {
            // Try to find a place to recover
            for (;;)
            {
                // The token we expected?
                // Then exit recovery mode and pretend like all is well.
                if (tokenReader.peekTokenType() == TokenType::Identifier
                    && tokenReader.peekToken().getContent() == expected)
                {
                    isRecovering = false;
                    return tokenReader.advanceToken();
                }


                // Don't skip past any "closing" tokens.
                if (IsClosingToken(tokenReader.peekTokenType()))
                {
                    return tokenReader.peekToken();
                }

                // Skip balanced tokens and try again.
                SkipBalancedToken(&tokenReader);
            }
        }
    }

    Token Parser::ReadToken()
    {
        return tokenReader.advanceToken();
    }

    static bool TryRecover(
        Parser*                         parser,
        TokenType const* recoverBefore,
        int                             recoverBeforeCount,
        TokenType const* recoverAfter,
        int                             recoverAfterCount)
    {
        if (!parser->isRecovering)
            return true;

        // Determine if we are looking for common closing tokens,
        // so that we can know whether or we are allowed to skip
        // over them.

        bool lookingForEOF = false;
        bool lookingForRCurly = false;
        bool lookingForRParen = false;
        bool lookingForRSquare = false;

        for (int ii = 0; ii < recoverBeforeCount; ++ii)
        {
            switch (recoverBefore[ii])
            {
            default:
                break;

            case TokenType::EndOfFile:  lookingForEOF       = true; break;
            case TokenType::RBrace:     lookingForRCurly    = true; break;
            case TokenType::RParent:    lookingForRParen    = true; break;
            case TokenType::RBracket:   lookingForRSquare   = true; break;
            }
        }
        for (int ii = 0; ii < recoverAfterCount; ++ii)
        {
            switch (recoverAfter[ii])
            {
            default:
                break;

            case TokenType::EndOfFile:  lookingForEOF       = true; break;
            case TokenType::RBrace:     lookingForRCurly    = true; break;
            case TokenType::RParent:    lookingForRParen    = true; break;
            case TokenType::RBracket:   lookingForRSquare   = true; break;
            }
        }

        TokenReader* tokenReader = &parser->tokenReader;
        for (;;)
        {
            TokenType peek = tokenReader->peekTokenType();

            // Is the next token in our recover-before set?
            // If so, then we have recovered successfully!
            for (int ii = 0; ii < recoverBeforeCount; ++ii)
            {
                if (peek == recoverBefore[ii])
                {
                    parser->isRecovering = false;
                    return true;
                }
            }

            // If we are looking at a token in our recover-after set,
            // then consume it and recover
            for (int ii = 0; ii < recoverAfterCount; ++ii)
            {
                if (peek == recoverAfter[ii])
                {
                    tokenReader->advanceToken();
                    parser->isRecovering = false;
                    return true;
                }
            }

            // Don't try to skip past end of file
            if (peek == TokenType::EndOfFile)
                return false;

            switch (peek)
            {
            // Don't skip past simple "closing" tokens, *unless*
            // we are looking for a closing token
            case TokenType::RParent:
            case TokenType::RBracket:
                if (lookingForRParen || lookingForRSquare || lookingForRCurly || lookingForEOF)
                {
                    // We are looking for a closing token, so it is okay to skip these
                }
                else
                    return false;
                break;

            // Don't skip a `}`, to avoid spurious errors,
            // with the exception of when we are looking for EOF
            case TokenType::RBrace:
                if (lookingForRCurly || lookingForEOF)
                {
                    // We are looking for end-of-file, so it is okay to skip here
                }
                else
                {
                    return false;
                }
            }

            // Skip balanced tokens and try again.
            TokenType skipped = SkipBalancedToken(tokenReader);

            // If we happened to find a matched pair of tokens, and
            // the end of it was a token we were looking for,
            // then recover here
            for (int ii = 0; ii < recoverAfterCount; ++ii)
            {
                if (skipped == recoverAfter[ii])
                {
                    parser->isRecovering = false;
                    return true;
                }
            }
        }
    }

    static bool TryRecoverBefore(
        Parser*                     parser,
        TokenType    before0)
    {
        TokenType recoverBefore[] = { before0 };
        return TryRecover(parser, recoverBefore, 1, nullptr, 0);
    }

    // Default recovery strategy, to use inside `{}`-delimeted blocks.
    static bool TryRecover(
        Parser*                     parser)
    {
        TokenType recoverBefore[] = { TokenType::RBrace };
        TokenType recoverAfter[] = { TokenType::Semicolon };
        return TryRecover(parser, recoverBefore, 1, recoverAfter, 1);
    }

    Token Parser::ReadToken(TokenType expected)
    {
        if (tokenReader.peekTokenType() == expected)
        {
            isRecovering = false;
            return tokenReader.advanceToken();
        }

        if (!isRecovering)
        {
            Unexpected(this, expected);
            return tokenReader.peekToken();
        }
        else
        {
            // Try to find a place to recover
            if (TryRecoverBefore(this, expected))
            {
                isRecovering = false;
                return tokenReader.advanceToken();
            }

            return tokenReader.peekToken();
        }
    }

    bool Parser::LookAheadToken(const char * string, int offset)
    {
        TokenReader r = tokenReader;
        for (int ii = 0; ii < offset; ++ii)
            r.advanceToken();

        return r.peekTokenType() == TokenType::Identifier
            && r.peekToken().getContent() == string;
}

    bool Parser::LookAheadToken(TokenType type, int offset)
    {
        TokenReader r = tokenReader;
        for (int ii = 0; ii < offset; ++ii)
            r.advanceToken();

        return r.peekTokenType() == type;
    }

    // Consume a token and return true it if matches, otherwise false
    bool AdvanceIf(Parser* parser, TokenType tokenType)
    {
        if (parser->LookAheadToken(tokenType))
        {
            parser->ReadToken();
            return true;
        }
        return false;
    }

    // Consume a token and return true it if matches, otherwise false
    bool AdvanceIf(Parser* parser, char const* text)
    {
        if (parser->LookAheadToken(text))
        {
            parser->ReadToken();
            return true;
        }
        return false;
    }

    // Consume a token and return true if it matches, otherwise check
    // for end-of-file and expect that token (potentially producing
    // an error) and return true to maintain forward progress.
    // Otherwise return false.
    bool AdvanceIfMatch(Parser* parser, TokenType tokenType)
    {
        // If we've run into a syntax error, but haven't recovered inside
        // the block, then try to recover here.
        if (parser->isRecovering)
        {
            TryRecoverBefore(parser, tokenType);
        }
        if (AdvanceIf(parser, tokenType))
            return true;
        if (parser->tokenReader.peekTokenType() == TokenType::EndOfFile)
        {
            parser->ReadToken(tokenType);
            return true;
        }
        return false;
    }

    NodeBase* ParseTypeDef(Parser* parser, void* /*userData*/)
    {
        TypeDefDecl* typeDefDecl = parser->astBuilder->create<TypeDefDecl>();

        // TODO(tfoley): parse an actual declarator
        auto type = parser->ParseTypeExp();

        auto nameToken = parser->ReadToken(TokenType::Identifier);
        typeDefDecl->loc = nameToken.loc;

        typeDefDecl->nameAndLoc = NameLoc(nameToken);
        typeDefDecl->type = type;

        return typeDefDecl;
    }

    // Add a modifier to a list of modifiers being built
    static void AddModifier(Modifier*** ioModifierLink, Modifier* modifier)
    {
        Modifier**& modifierLink = *ioModifierLink;

        // We'd like to add the modifier to the end of the list,
        // but we need to be careful, in case there is a "shared"
        // section of modifiers for multiple declarations.
        //
        // TODO: This whole approach is a mess because we are "accidentally quadratic"
        // when adding many modifiers.
        for(;;)
        {
            // At end of the chain? Done.
            if(!*modifierLink)
                break;

            // About to look at shared modifiers? Done.
            Modifier* linkMod = *modifierLink;
            if(as<SharedModifiers>(linkMod))
            {
                break;
            }

            // Otherwise: keep traversing the modifier list.
            modifierLink = &(*modifierLink)->next;
        }

        // Splice the modifier into the linked list

        // We need to deal with the case where the modifier to
        // be spliced in might actually be a modifier *list*,
        // so that we actually want to splice in at the
        // end of the new list...
        auto spliceLink = &modifier->next;
        while(*spliceLink)
            spliceLink = &(*spliceLink)->next;

        // Do the splice.
        *spliceLink = *modifierLink;

        *modifierLink = modifier;
        modifierLink = &modifier->next;
    }

    void addModifier(
        ModifiableSyntaxNode*    syntax,
        Modifier*                modifier)
    {
        auto modifierLink = &syntax->modifiers.first;
        AddModifier(&modifierLink, modifier);
    }

    // 
    // '::'? identifier ('::' identifier)* 
    static Token parseAttributeName(Parser* parser)
    {
        const SourceLoc scopedIdSourceLoc = parser->tokenReader.peekLoc();

        // Strip initial :: if there is one
        const TokenType initialTokenType = parser->tokenReader.peekTokenType();
        if (initialTokenType == TokenType::Scope)
        {
            parser->ReadToken(TokenType::Scope); 
        }

        const Token firstIdentifier = parser->ReadToken(TokenType::Identifier);
        if (initialTokenType != TokenType::Scope && parser->tokenReader.peekTokenType() != TokenType::Scope)
        {
            return firstIdentifier;
        }

        // Build up scoped string
        StringBuilder scopedIdentifierBuilder;
        if (initialTokenType == TokenType::Scope)
        {
            scopedIdentifierBuilder.Append('_'); 
        }
        scopedIdentifierBuilder.Append(firstIdentifier.getContent());

        while (parser->tokenReader.peekTokenType() == TokenType::Scope)
        {
            parser->ReadToken(TokenType::Scope);
            scopedIdentifierBuilder.Append('_'); 
            
            const Token nextIdentifier(parser->ReadToken(TokenType::Identifier));
            scopedIdentifierBuilder.Append(nextIdentifier.getContent());
        }

        // Make a 'token'
        SourceManager* sourceManager = parser->sink->getSourceManager();
        const UnownedStringSlice scopedIdentifier(sourceManager->allocateStringSlice(scopedIdentifierBuilder.getUnownedSlice()));   
        Token token(TokenType::Identifier, scopedIdentifier, scopedIdSourceLoc);

        // Get the name pool
        auto namePool = parser->getNamePool();

        // Since it's an Identifier have to set the name.
        token.setName(namePool->getName(token.getContent()));

        return token;
    }

    // Parse HLSL-style `[name(arg, ...)]` style "attribute" modifiers
    static void ParseSquareBracketAttributes(Parser* parser, Modifier*** ioModifierLink)
    {
        parser->ReadToken(TokenType::LBracket);

        const bool hasDoubleBracket = AdvanceIf(parser, TokenType::LBracket);

        for(;;)
        {
            // Note: When parsing we just construct an AST node for an
            // "unchecked" attribute, and defer all detailed semantic
            // checking until later.
            //
            // An alternative would be to perform lookup of an `AttributeDecl`
            // at this point, similar to what we do for `SyntaxDecl`, but it
            // seems better to not complicate the parsing process any more.
            //

            Token nameToken = parseAttributeName(parser);

            UncheckedAttribute* modifier = parser->astBuilder->create<UncheckedAttribute>();
            modifier->name = nameToken.getName();
            modifier->loc = nameToken.getLoc();
            modifier->scope = parser->currentScope;

            if (AdvanceIf(parser, TokenType::LParent))
            {
                // HLSL-style `[name(arg0, ...)]` attribute

                while (!AdvanceIfMatch(parser, TokenType::RParent))
                {
                    auto arg = parser->ParseArgExpr();
                    if (arg)
                    {
                        modifier->args.add(arg);
                    }

                    if (AdvanceIfMatch(parser, TokenType::RParent))
                        break;

                    parser->ReadToken(TokenType::Comma);
                }
            }
            AddModifier(ioModifierLink, modifier);


            if (AdvanceIfMatch(parser, TokenType::RBracket))
                break;

            parser->ReadToken(TokenType::Comma);
        }

        if (hasDoubleBracket)
        {
            // Read the second ]
            parser->ReadToken(TokenType::RBracket);
        }
    }

    static TokenType peekTokenType(Parser* parser)
    {
        return parser->tokenReader.peekTokenType();
    }

    static Token advanceToken(Parser* parser)
    {
        return parser->ReadToken();
    }

    static Token peekToken(Parser* parser)
    {
        return parser->tokenReader.peekToken();
    }

    static SyntaxDecl* tryLookUpSyntaxDecl(
        Parser* parser,
        Name*   name)
    {
        // Let's look up the name and see what we find.

        auto lookupResult = lookUp(
            parser->astBuilder,
            nullptr, // no semantics visitor available yet
            name,
            parser->currentScope);

        // If we didn't find anything, or the result was overloaded,
        // then we aren't going to be able to extract a single decl.
        if(!lookupResult.isValid() || lookupResult.isOverloaded())
            return nullptr;

        auto decl = lookupResult.item.declRef.getDecl();
        if( auto syntaxDecl = as<SyntaxDecl>(decl) )
        {
            return syntaxDecl;
        }
        else
        {
            return nullptr;
        }
    }

    template<typename T>
    bool tryParseUsingSyntaxDecl(
        Parser*     parser,
        SyntaxDecl* syntaxDecl,
        T**  outSyntax)
    {
        if (!syntaxDecl)
            return false;

        if (!syntaxDecl->syntaxClass.isSubClassOf<T>())
            return false;

        // Consume the token that specified the keyword
        auto keywordToken = advanceToken(parser);

        NodeBase* parsedObject = syntaxDecl->parseCallback(parser, syntaxDecl->parseUserData);
        if (!parsedObject)
        {
            return false;
        }

        auto syntax = as<T>(parsedObject);
        if (syntax)
        {
            if (!syntax->loc.isValid())
            {
                syntax->loc = keywordToken.loc;
            }
        }
        else if (parsedObject)
        {
            // Something was parsed, but it didn't have the expected type!
            SLANG_DIAGNOSE_UNEXPECTED(parser->sink, keywordToken, "parser callback did not return the expected type");
        }

        *outSyntax = syntax;
        return true;
    }

    template<typename T>
    bool tryParseUsingSyntaxDecl(
        Parser*     parser,
        T**  outSyntax)
    {
        if (peekTokenType(parser) != TokenType::Identifier)
            return false;

        auto nameToken = peekToken(parser);
        auto name = nameToken.getName();

        auto syntaxDecl = tryLookUpSyntaxDecl(parser, name);

        if (!syntaxDecl)
            return false;

        return tryParseUsingSyntaxDecl(parser, syntaxDecl, outSyntax);
    }

    static Modifiers ParseModifiers(Parser* parser)
    {
        Modifiers modifiers;
        Modifier** modifierLink = &modifiers.first;
        for (;;)
        {
            SourceLoc loc = parser->tokenReader.peekLoc();

            switch (peekTokenType(parser))
            {
            default:
                // If we don't see a token type that we recognize, then
                // assume we are done with the modifier sequence.
                return modifiers;

            case TokenType::Identifier:
                {
                    // We see an identifier ahead, and it might be the name
                    // of a modifier keyword of some kind.

                    Token nameToken = peekToken(parser);

                    Modifier* parsedModifier = nullptr;
                    if (tryParseUsingSyntaxDecl<Modifier>(parser, &parsedModifier))
                    {
                        parsedModifier->name = nameToken.getName();
                        if (!parsedModifier->loc.isValid())
                        {
                            parsedModifier->loc = nameToken.loc;
                        }

                        AddModifier(&modifierLink, parsedModifier);
                        continue;
                    }

                    // If there was no match for a modifier keyword, then we
                    // must be at the end of the modifier sequence
                    return modifiers;
                }
                break;

            // HLSL uses `[attributeName]` style for its modifiers, which closely
            // matches the C++ `[[attributeName]]` style.
            case TokenType::LBracket:
                ParseSquareBracketAttributes(parser, &modifierLink);
                break;
            }
        }
    }

    static Name* getName(Parser* parser, String const& text)
    {
        return parser->getNamePool()->getName(text);
    }

    static NameLoc expectIdentifier(Parser* parser)
    {
        return NameLoc(parser->ReadToken(TokenType::Identifier));
    }


    static NodeBase* parseImportDecl(
        Parser* parser, void* /*userData*/)
    {
        parser->haveSeenAnyImportDecls = true;

        auto decl = parser->astBuilder->create<ImportDecl>();
        decl->scope = parser->currentScope;

        if (peekTokenType(parser) == TokenType::StringLiteral)
        {
            auto nameToken = parser->ReadToken(TokenType::StringLiteral);
            auto nameString = getStringLiteralTokenValue(nameToken);
            auto moduleName = getName(parser, nameString);

            decl->moduleNameAndLoc = NameLoc(moduleName, nameToken.loc);
        }
        else
        {
            auto moduleNameAndLoc = expectIdentifier(parser);

            // We allow a dotted format for the name, as sugar
            if (peekTokenType(parser) == TokenType::Dot)
            {
                StringBuilder sb;
                sb << getText(moduleNameAndLoc.name);
                while (AdvanceIf(parser, TokenType::Dot))
                {
                    sb << "/";
                    sb << parser->ReadToken(TokenType::Identifier).getContent();
                }

                moduleNameAndLoc.name = getName(parser, sb.ProduceString());
            }

            decl->moduleNameAndLoc = moduleNameAndLoc;
        }

        parser->ReadToken(TokenType::Semicolon);

        return decl;
    }

    static NameLoc ParseDeclName(
        Parser* parser)
    {
        Token nameToken;
        if (AdvanceIf(parser, "operator"))
        {
            nameToken = parser->ReadToken();
            switch (nameToken.type)
            {
            case TokenType::OpAdd: case TokenType::OpSub: case TokenType::OpMul: case TokenType::OpDiv:
            case TokenType::OpMod: case TokenType::OpNot: case TokenType::OpBitNot: case TokenType::OpLsh: case TokenType::OpRsh:
            case TokenType::OpEql: case TokenType::OpNeq: case TokenType::OpGreater: case TokenType::OpLess: case TokenType::OpGeq:
            case TokenType::OpLeq: case TokenType::OpAnd: case TokenType::OpOr: case TokenType::OpBitXor: case TokenType::OpBitAnd:
            case TokenType::OpBitOr: case TokenType::OpInc: case TokenType::OpDec:
            case TokenType::OpAddAssign:
            case TokenType::OpSubAssign:
            case TokenType::OpMulAssign:
            case TokenType::OpDivAssign:
            case TokenType::OpModAssign:
            case TokenType::OpShlAssign:
            case TokenType::OpShrAssign:
            case TokenType::OpOrAssign:
            case TokenType::OpAndAssign:
            case TokenType::OpXorAssign:

            // Note(tfoley): A bit of a hack:
            case TokenType::Comma:
            case TokenType::OpAssign:
                break;

            // Note(tfoley): Even more of a hack!
            case TokenType::QuestionMark:
                if (AdvanceIf(parser, TokenType::Colon))
                {
                    // Concat : onto ?
                    nameToken.setContent(UnownedStringSlice::fromLiteral("?:")); 
                    break;
                }
                ;       // fall-thru
            default:
                parser->sink->diagnose(nameToken.loc, Diagnostics::invalidOperator, nameToken);
                break;
            }

            return NameLoc(
                getName(parser, nameToken.getContent()),
                nameToken.loc);
        }
        else
        {
            nameToken = parser->ReadToken(TokenType::Identifier);
            return NameLoc(nameToken);
        }
    }

    // A "declarator" as used in C-style languages
    struct Declarator : RefObject
    {
        // Different cases of declarator appear as "flavors" here
        enum class Flavor
        {
            name,
            Pointer,
            Array,
        };
        Flavor flavor;
    };

    // The most common case of declarator uses a simple name
    struct NameDeclarator : Declarator
    {
        NameLoc nameAndLoc;
    };

    // A declarator that declares a pointer type
    struct PointerDeclarator : Declarator
    {
        // location of the `*` token
        SourceLoc starLoc;

        RefPtr<Declarator>				inner;
    };

    // A declarator that declares an array type
    struct ArrayDeclarator : Declarator
    {
        RefPtr<Declarator>				inner;

        // location of the `[` token
        SourceLoc openBracketLoc;

        // The expression that yields the element count, or NULL
        Expr*	elementCountExpr = nullptr;
    };

    // "Unwrapped" information about a declarator
    struct DeclaratorInfo
    {
        Expr*	    typeSpec = nullptr;
        NameLoc             nameAndLoc;
        Modifier*	semantics = nullptr;
        Expr*	    initializer = nullptr;
    };

    // Add a member declaration to its container, and ensure that its
    // parent link is set up correctly.
    static void AddMember(ContainerDecl* container, Decl* member)
    {
        if (container)
        {
            member->parentDecl = container;
            container->members.add(member);
        }
    }

    static void AddMember(RefPtr<Scope> scope, Decl* member)
    {
        if (scope)
        {
            AddMember(scope->containerDecl, member);
        }
    }

    static Decl* ParseGenericParamDecl(
        Parser*             parser,
        GenericDecl* genericDecl)
    {
        // simple syntax to introduce a value parameter
        if (AdvanceIf(parser, "let"))
        {
            // default case is a type parameter
            auto paramDecl = parser->astBuilder->create<GenericValueParamDecl>();
            paramDecl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));
            if (AdvanceIf(parser, TokenType::Colon))
            {
                paramDecl->type = parser->ParseTypeExp();
            }
            if (AdvanceIf(parser, TokenType::OpAssign))
            {
                paramDecl->initExpr = parser->ParseInitExpr();
            }
            return paramDecl;
        }
        else
        {
            // default case is a type parameter
            GenericTypeParamDecl* paramDecl = parser->astBuilder->create<GenericTypeParamDecl>();
            parser->FillPosition(paramDecl);
            paramDecl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));
            if (AdvanceIf(parser, TokenType::Colon))
            {
                // The user is apply a constraint to this type parameter...

                auto paramConstraint = parser->astBuilder->create<GenericTypeConstraintDecl>();
                parser->FillPosition(paramConstraint);

                auto paramType = DeclRefType::create(
                    parser->astBuilder,
                    DeclRef<Decl>(paramDecl, nullptr));

                auto paramTypeExpr = parser->astBuilder->create<SharedTypeExpr>();
                paramTypeExpr->loc = paramDecl->loc;
                paramTypeExpr->base.type = paramType;
                paramTypeExpr->type = QualType(parser->astBuilder->getTypeType(paramType));

                paramConstraint->sub = TypeExp(paramTypeExpr);
                paramConstraint->sup = parser->ParseTypeExp();

                AddMember(genericDecl, paramConstraint);


            }
            if (AdvanceIf(parser, TokenType::OpAssign))
            {
                paramDecl->initType = parser->ParseTypeExp();
            }
            return paramDecl;
        }
    }

    template<typename TFunc>
    static void ParseGenericDeclImpl(
        Parser* parser, GenericDecl* decl, const TFunc & parseInnerFunc)
    {
        parser->ReadToken(TokenType::OpLess);
        parser->genericDepth++;
        while (!parser->LookAheadToken(TokenType::OpGreater))
        {
            AddMember(decl, ParseGenericParamDecl(parser, decl));

            if (parser->LookAheadToken(TokenType::OpGreater))
                break;

            parser->ReadToken(TokenType::Comma);
        }
        parser->genericDepth--;
        parser->ReadToken(TokenType::OpGreater);
        decl->inner = parseInnerFunc(decl);
        decl->inner->parentDecl = decl;

        // A generic decl hijacks the name of the declaration
        // it wraps, so that lookup can find it.
        if (decl->inner)
        {
            decl->nameAndLoc = decl->inner->nameAndLoc;
            decl->loc = decl->inner->loc;
        }
    }

    template<typename ParseFunc>
    static Decl* parseOptGenericDecl(
        Parser* parser, const ParseFunc& parseInner)
    {
        // TODO: may want more advanced disambiguation than this...
        if (parser->LookAheadToken(TokenType::OpLess))
        {
            GenericDecl* genericDecl = parser->astBuilder->create<GenericDecl>();
            parser->FillPosition(genericDecl);
            parser->PushScope(genericDecl);
            ParseGenericDeclImpl(parser, genericDecl, parseInner);
            parser->PopScope();
            return genericDecl;
        }
        else
        {
            return parseInner(nullptr);
        }
    }

    static NodeBase* ParseGenericDecl(Parser* parser, void*)
    {
        GenericDecl* decl = parser->astBuilder->create<GenericDecl>();
        parser->FillPosition(decl);
        parser->PushScope(decl);
        ParseGenericDeclImpl(parser, decl, [=](GenericDecl* genDecl) {return ParseSingleDecl(parser, genDecl); });
        parser->PopScope();
        return decl;
    }

    static void parseParameterList(
        Parser*         parser,
        CallableDecl*    decl)
    {
        parser->ReadToken(TokenType::LParent);

        // Allow a declaration to use the keyword `void` for a parameter list,
        // since that was required in ancient C, and continues to be supported
        // in a bunc hof its derivatives even if it is a Bad Design Choice
        //
        // TODO: conditionalize this so we don't keep this around for "pure"
        // Slang code
        if( parser->LookAheadToken("void") && parser->LookAheadToken(TokenType::RParent, 1) )
        {
            parser->ReadToken("void");
            parser->ReadToken(TokenType::RParent);
            return;
        }

        while (!AdvanceIfMatch(parser, TokenType::RParent))
        {
            AddMember(decl, parser->ParseParameter());
            if (AdvanceIf(parser, TokenType::RParent))
                break;
            parser->ReadToken(TokenType::Comma);
        }
    }

    // systematically replace all scopes in an expression tree
    class ReplaceScopeVisitor : public ExprVisitor<ReplaceScopeVisitor>
    {
    public:
        RefPtr<Scope> scope;
        void visitDeclRefExpr(DeclRefExpr* expr)
        {
            expr->scope = scope;
        }
        void visitGenericAppExpr(GenericAppExpr * expr)
        {
            expr->functionExpr->accept(this, nullptr);
            for (auto arg : expr->arguments)
                arg->accept(this, nullptr);
        }
        void visitIndexExpr(IndexExpr * expr)
        {
            expr->baseExpression->accept(this, nullptr);
            expr->indexExpression->accept(this, nullptr);
        }
        void visitMemberExpr(MemberExpr * expr)
        {
            expr->baseExpression->accept(this, nullptr);
            expr->scope = scope;
        }
        void visitStaticMemberExpr(StaticMemberExpr * expr)
        {
            expr->baseExpression->accept(this, nullptr);
            expr->scope = scope;
        }
        void visitExpr(Expr* /*expr*/)
        {}
    };

        /// Parse an optional body statement for a declaration that can have a body.
    static Stmt* parseOptBody(Parser* parser)
    {
        if (AdvanceIf(parser, TokenType::Semicolon))
        {
            // empty body
            return nullptr;
        }
        else
        {
            return parser->parseBlockStatement();
        }
    }

        /// Complete parsing of a function using traditional (C-like) declarator syntax
    static Decl* parseTraditionalFuncDecl(
        Parser*                 parser,
        DeclaratorInfo const&   declaratorInfo)
    {
        FuncDecl* decl = parser->astBuilder->create<FuncDecl>();
        parser->FillPosition(decl);
        decl->loc = declaratorInfo.nameAndLoc.loc;
        decl->nameAndLoc = declaratorInfo.nameAndLoc;

        return parseOptGenericDecl(parser, [&](GenericDecl*)
        {
            // HACK: The return type of the function will already have been
            // parsed in a scope that didn't include the function's generic
            // parameters.
            //
            // We will use a visitor here to try and replace the scope associated
            // with any name expressiosn in the reuslt type.
            //
            // TODO: This should be fixed by not associating scopes with
            // such expressions at parse time, and instead pushing down scopes
            // as part of the state during semantic checking.
            //
            ReplaceScopeVisitor replaceScopeVisitor;
            replaceScopeVisitor.scope = parser->currentScope;
            declaratorInfo.typeSpec->accept(&replaceScopeVisitor, nullptr);

            decl->returnType = TypeExp(declaratorInfo.typeSpec);

            parser->PushScope(decl);

            parseParameterList(parser, decl);
            ParseOptSemantics(parser, decl);
            decl->body = parseOptBody(parser);

            parser->PopScope();

            return decl;
        });
    }

    static VarDeclBase* CreateVarDeclForContext(
        ASTBuilder* astBuilder, 
        ContainerDecl*  containerDecl )
    {
        if (as<CallableDecl>(containerDecl))
        {
            // Function parameters always use their dedicated syntax class.
            //
            return astBuilder->create<ParamDecl>();
        }
        else
        {
            // Globals, locals, and member variables all use the same syntax class.
            //
            return astBuilder->create<VarDecl>();
        }
    }

    // Add modifiers to the end of the modifier list for a declaration
    void AddModifiers(Decl* decl, Modifier* modifiers)
    {
        if (!modifiers)
            return;

        Modifier** link = &decl->modifiers.first;
        while (*link)
        {
            link = &(*link)->next;
        }
        *link = modifiers;
    }

    static Name* generateName(Parser* parser, String const& base)
    {
        // TODO: somehow mangle the name to avoid clashes
        return getName(parser, "SLANG_" + base);
    }

    static Name* generateName(Parser* parser)
    {
        return generateName(parser, "anonymous_" + String(parser->anonymousCounter++));
    }


    // Set up a variable declaration based on what we saw in its declarator...
    static void CompleteVarDecl(
        Parser*					parser,
        VarDeclBase*		decl,
        DeclaratorInfo const&	declaratorInfo)
    {
        parser->FillPosition(decl);

        if( !declaratorInfo.nameAndLoc.name )
        {
            // HACK(tfoley): we always give a name, even if the declarator didn't include one... :(
            decl->nameAndLoc = NameLoc(generateName(parser));
        }
        else
        {
            decl->loc = declaratorInfo.nameAndLoc.loc;
            decl->nameAndLoc = declaratorInfo.nameAndLoc;
        }
        decl->type = TypeExp(declaratorInfo.typeSpec);

        AddModifiers(decl, declaratorInfo.semantics);

        decl->initExpr = declaratorInfo.initializer;
    }

    typedef unsigned int DeclaratorParseOptions;
    enum
    {
        kDeclaratorParseOptions_None = 0,
        kDeclaratorParseOption_AllowEmpty = 1 << 0,
    };

    static RefPtr<Declarator> parseDeclarator(
        Parser*                 parser,
        DeclaratorParseOptions  options);

    static RefPtr<Declarator> parseDirectAbstractDeclarator(
        Parser*                 parser,
        DeclaratorParseOptions  options)
    {
        RefPtr<Declarator> declarator;
        switch( parser->tokenReader.peekTokenType() )
        {
        case TokenType::Identifier:
            {
                auto nameDeclarator = new NameDeclarator();
                nameDeclarator->flavor = Declarator::Flavor::name;
                nameDeclarator->nameAndLoc = ParseDeclName(parser);
                declarator = nameDeclarator;
            }
            break;

        case TokenType::LParent:
            {
                // Note(tfoley): This is a point where disambiguation is required.
                // We could be looking at an abstract declarator for a function-type
                // parameter:
                //
                //     void F( int(int) );
                //
                // Or we could be looking at the use of parenthesese in an ordinary
                // declarator:
                //
                //     void (*f)(int);
                //
                // The difference really doesn't matter right now, but we err in
                // the direction of assuming the second case.
                //
                // TODO: We should consider just not supporting this case at all,
                // since it can't come up in current Slang (no pointer or function-type
                // support), and we might be able to introduce alternative syntax
                // to get around these issues when those features come online.
                //
                parser->ReadToken(TokenType::LParent);
                declarator = parseDeclarator(parser, options);
                parser->ReadToken(TokenType::RParent);
            }
            break;

        default:
            if(options & kDeclaratorParseOption_AllowEmpty)
            {
                // an empty declarator is allowed
            }
            else
            {
                // If an empty declarator is now allowed, then we
                // will give the user an error message saying that
                // an identifier was expected.
                //
                expectIdentifier(parser);
            }
            return nullptr;
        }

        // postifx additions
        for( ;;)
        {
            switch( parser->tokenReader.peekTokenType() )
            {
            case TokenType::LBracket:
                {
                    auto arrayDeclarator = new ArrayDeclarator();
                    arrayDeclarator->openBracketLoc = parser->tokenReader.peekLoc();
                    arrayDeclarator->flavor = Declarator::Flavor::Array;
                    arrayDeclarator->inner = declarator;

                    parser->ReadToken(TokenType::LBracket);
                    if( parser->tokenReader.peekTokenType() != TokenType::RBracket )
                    {
                        arrayDeclarator->elementCountExpr = parser->ParseExpression();
                    }
                    parser->ReadToken(TokenType::RBracket);

                    declarator = arrayDeclarator;
                    continue;
                }

            case TokenType::LParent:
                break;

            default:
                break;
            }

            break;
        }

        return declarator;
    }

    // Parse a declarator (or at least as much of one as we support)
    static RefPtr<Declarator> parseDeclarator(
        Parser*                 parser,
        DeclaratorParseOptions  options)
    {
        if( parser->tokenReader.peekTokenType() == TokenType::OpMul )
        {
            auto ptrDeclarator = new PointerDeclarator();
            ptrDeclarator->starLoc = parser->tokenReader.peekLoc();
            ptrDeclarator->flavor = Declarator::Flavor::Pointer;

            parser->ReadToken(TokenType::OpMul);

            // TODO(tfoley): allow qualifiers like `const` here?

            ptrDeclarator->inner = parseDeclarator(parser, options);
            return ptrDeclarator;
        }
        else
        {
            return parseDirectAbstractDeclarator(parser, options);
        }
    }

    // A declarator plus optional semantics and initializer
    struct InitDeclarator
    {
        RefPtr<Declarator>  declarator;
        Modifier*    semantics = nullptr;
        Expr*        initializer = nullptr;
    };

    // Parse a declarator plus optional semantics
    static InitDeclarator parseSemanticDeclarator(
        Parser*                 parser,
        DeclaratorParseOptions  options)
    {
        InitDeclarator result;
        result.declarator = parseDeclarator(parser, options);
        result.semantics = ParseOptSemantics(parser);
        return result;
    }

    // Parse a declarator plus optional semantics and initializer
    static InitDeclarator parseInitDeclarator(
        Parser*                 parser,
        DeclaratorParseOptions  options)
    {
        InitDeclarator result = parseSemanticDeclarator(parser, options);
        if (AdvanceIf(parser, TokenType::OpAssign))
        {
            result.initializer = parser->ParseInitExpr();
        }
        return result;
    }

    static void UnwrapDeclarator(
        ASTBuilder*         astBuilder, 
        RefPtr<Declarator>	declarator,
        DeclaratorInfo*		ioInfo)
    {
        while( declarator )
        {
            switch(declarator->flavor)
            {
            case Declarator::Flavor::name:
                {
                    auto nameDeclarator = (NameDeclarator*) declarator.Ptr();
                    ioInfo->nameAndLoc = nameDeclarator->nameAndLoc;
                    return;
                }
                break;

            case Declarator::Flavor::Pointer:
                {
                    auto ptrDeclarator = (PointerDeclarator*) declarator.Ptr();

                    // TODO(tfoley): we don't support pointers for now
                    // ioInfo->typeSpec = new PointerTypeExpr(ioInfo->typeSpec);

                    declarator = ptrDeclarator->inner;
                }
                break;

            case Declarator::Flavor::Array:
                {
                    // TODO(tfoley): we don't support pointers for now
                    auto arrayDeclarator = (ArrayDeclarator*) declarator.Ptr();

                    auto arrayTypeExpr = astBuilder->create<IndexExpr>();
                    arrayTypeExpr->loc = arrayDeclarator->openBracketLoc;
                    arrayTypeExpr->baseExpression = ioInfo->typeSpec;
                    arrayTypeExpr->indexExpression = arrayDeclarator->elementCountExpr;
                    ioInfo->typeSpec = arrayTypeExpr;

                    declarator = arrayDeclarator->inner;
                }
                break;

            default:
                SLANG_UNREACHABLE("all cases handled");
                break;
            }
        }
    }

    static void UnwrapDeclarator(
        ASTBuilder*             astBuilder, 
        InitDeclarator const&	initDeclarator,
        DeclaratorInfo*			ioInfo)
    {
        UnwrapDeclarator(astBuilder, initDeclarator.declarator, ioInfo);
        ioInfo->semantics = initDeclarator.semantics;
        ioInfo->initializer = initDeclarator.initializer;
    }

    // Either a single declaration, or a group of them
    struct DeclGroupBuilder
    {
        SourceLoc        startPosition;
        Decl*        decl = nullptr;
        DeclGroup*   group = nullptr;
        ASTBuilder*      astBuilder = nullptr;

        // Add a new declaration to the potential group
        void addDecl(
            Decl*    newDecl)
        {
            SLANG_ASSERT(newDecl);

            if( decl )
            {
                group = astBuilder->create<DeclGroup>();
                group->loc = startPosition;
                group->decls.add(decl);
                decl = nullptr;
            }

            if( group )
            {
                group->decls.add(newDecl);
            }
            else
            {
                decl = newDecl;
            }
        }

        DeclBase* getResult()
        {
            if(group) return group;
            return decl;
        }
    };

    // Pares an argument to an application of a generic
    Expr* ParseGenericArg(Parser* parser)
    {
        return parser->ParseArgExpr();
    }

    // Create a type expression that will refer to the given declaration
    static Expr*
    createDeclRefType(Parser* parser, Decl* decl)
    {
        // For now we just construct an expression that
        // will look up the given declaration by name.
        //
        // TODO: do this better, e.g. by filling in the `declRef` field directly

        auto expr = parser->astBuilder->create<VarExpr>();
        expr->scope = parser->currentScope.Ptr();
        expr->loc = decl->getNameLoc();
        expr->name = decl->getName();
        return expr;
    }

    // Representation for a parsed type specifier, which might
    // include a declaration (e.g., of a `struct` type)
    struct TypeSpec
    {
        // If the type-spec declared something, then put it here
        Decl*                    decl = nullptr;

        // Put the resulting expression (which should evaluate to a type) here
        Expr*    expr = nullptr;
    };

    static Expr* parseGenericApp(
        Parser*                         parser,
        Expr*    base)
    {
        GenericAppExpr* genericApp = parser->astBuilder->create<GenericAppExpr>();

        parser->FillPosition(genericApp); // set up scope for lookup
        genericApp->functionExpr = base;
        parser->ReadToken(TokenType::OpLess);
        parser->genericDepth++;
        // For now assume all generics have at least one argument
        genericApp->arguments.add(ParseGenericArg(parser));
        while (AdvanceIf(parser, TokenType::Comma))
        {
            genericApp->arguments.add(ParseGenericArg(parser));
        }
        parser->genericDepth--;

        if (parser->tokenReader.peekToken().type == TokenType::OpRsh)
        {
            parser->tokenReader.peekToken().type = TokenType::OpGreater;
            parser->tokenReader.peekToken().loc.setRaw(parser->tokenReader.peekToken().loc.getRaw() + 1);
        }
        else if (parser->LookAheadToken(TokenType::OpGreater))
            parser->ReadToken(TokenType::OpGreater);
        else
            parser->sink->diagnose(parser->tokenReader.peekToken(), Diagnostics::tokenTypeExpected, "'>'");
        return genericApp;
    }

    static bool isGenericName(Parser* parser, Name* name)
    {
        auto lookupResult = lookUp(
            parser->astBuilder,
            nullptr, // no semantics visitor available yet
            name,
            parser->currentScope);
        if (!lookupResult.isValid() || lookupResult.isOverloaded())
            return false;

        return lookupResult.item.declRef.is<GenericDecl>();
    }

    static Expr* tryParseGenericApp(
        Parser*                         parser,
        Expr*    base)
    {
        Name * baseName = nullptr;
        if (auto varExpr = as<VarExpr>(base))
            baseName = varExpr->name;
        // if base is a known generics, parse as generics
        if (baseName && isGenericName(parser, baseName))
            return parseGenericApp(parser, base);

        // otherwise, we speculate as generics, and fallback to comparison when parsing failed
        TokenSpan tokenSpan;
        tokenSpan.m_begin = parser->tokenReader.m_cursor;
        tokenSpan.m_end = parser->tokenReader.m_end;
        DiagnosticSink newSink(parser->sink->getSourceManager());
        Parser newParser(*parser);
        newParser.sink = &newSink;

        /* auto speculateParseRs = */parseGenericApp(&newParser, base);

        if (newSink.getErrorCount() == 0)
        {
            // disambiguate based on FOLLOW set
            switch (peekTokenType(&newParser))
            {
            case TokenType::Dot:
            case TokenType::LParent:
            case TokenType::RParent:
            case TokenType::RBracket:
            case TokenType::Colon:
            case TokenType::Comma:
            case TokenType::QuestionMark:
            case TokenType::Semicolon:
            case TokenType::OpEql:
            case TokenType::OpNeq:
            {
                return parseGenericApp(parser, base);
            }
            }
        }
        return base;
    }
    static Expr* parseMemberType(Parser * parser, Expr* base)
    {
        // When called the :: or . have been consumed, so don't need to consume here.

        MemberExpr* memberExpr = parser->astBuilder->create<MemberExpr>();

        parser->FillPosition(memberExpr);
        memberExpr->baseExpression = base;
        memberExpr->name = expectIdentifier(parser).name;
        return memberExpr;
    }

    // Parse option `[]` braces after a type expression, that indicate an array type
    static Expr* parsePostfixTypeSuffix(
        Parser* parser,
        Expr* inTypeExpr)
    {
        auto typeExpr = inTypeExpr;
        while (parser->LookAheadToken(TokenType::LBracket))
        {
            IndexExpr* arrType = parser->astBuilder->create<IndexExpr>();
            arrType->loc = typeExpr->loc;
            arrType->baseExpression = typeExpr;
            parser->ReadToken(TokenType::LBracket);
            if (!parser->LookAheadToken(TokenType::RBracket))
            {
                arrType->indexExpression = parser->ParseExpression();
            }
            parser->ReadToken(TokenType::RBracket);
            typeExpr = arrType;
        }
        return typeExpr;
    }

    static Expr* parseTaggedUnionType(Parser* parser)
    {
        TaggedUnionTypeExpr* taggedUnionType = parser->astBuilder->create<TaggedUnionTypeExpr>();

        parser->ReadToken(TokenType::LParent);
        while(!AdvanceIfMatch(parser, TokenType::RParent))
        {
            auto caseType = parser->ParseTypeExp();
            taggedUnionType->caseTypes.add(caseType);

            if(AdvanceIf(parser, TokenType::RParent))
                break;

            parser->ReadToken(TokenType::Comma);
        }

        return taggedUnionType;
    }

    static NodeBase* parseTaggedUnionType(Parser* parser, void* /*unused*/)
    {
        return parseTaggedUnionType(parser);
    }

        /// Parse a `This` type expression
    static Expr* parseThisTypeExpr(Parser* parser)
    {
        ThisTypeExpr* expr = parser->astBuilder->create<ThisTypeExpr>();
        expr->scope = parser->currentScope;
        return expr;
    }

    static NodeBase* parseThisTypeExpr(Parser* parser, void* /*userData*/)
    {
        return parseThisTypeExpr(parser);
    }

    static TypeSpec parseTypeSpec(Parser* parser)
    {
        TypeSpec typeSpec;

        // We may see a `struct` (or `enum` or `class`) tag specified here, and need to act accordingly
        //
        // TODO(tfoley): Handle the case where the user is just using `struct`
        // as a way to name an existing struct "tag" (e.g., `struct Foo foo;`)
        //
        // TODO: We should really make these keywords be registered like any other
        // syntax category, rather than be special-cased here. The main issue here
        // is that we need to allow them to be used as type specifiers, as in:
        //
        //      struct Foo { int x } foo;
        //
        // The ideal answer would be to register certain keywords as being able
        // to parse a type specifier, and look for those keywords here.
        // We should ideally add special case logic that bails out of declarator
        // parsing iff we have one of these kinds of type specifiers and the
        // closing `}` is at the end of its line, as a bit of a special case
        // to allow the common idiom.
        //
        if( parser->LookAheadToken("struct") )
        {
            auto decl = parser->ParseStruct();
            typeSpec.decl = decl;
            typeSpec.expr = createDeclRefType(parser, decl);
            return typeSpec;
        }
        else if( parser->LookAheadToken("class") )
        {
            auto decl = parser->ParseClass();
            typeSpec.decl = decl;
            typeSpec.expr = createDeclRefType(parser, decl);
            return typeSpec;
        }
        else if(parser->LookAheadToken("enum"))
        {
            auto decl = parseEnumDecl(parser);
            typeSpec.decl = decl;
            typeSpec.expr = createDeclRefType(parser, decl);
            return typeSpec;
        }
        // TODO: This case would not be needed if we had the
        // code below dispatch into `parseAtomicExpr`, which
        // already includes logic for keyword lookup.
        //
        // Leaving this case here for now to avoid breaking anything.
        //
        else if(AdvanceIf(parser, "__TaggedUnion"))
        {
            typeSpec.expr = parseTaggedUnionType(parser);
            return typeSpec;
        }
        else if(AdvanceIf(parser, "This"))
        {
            typeSpec.expr = parseThisTypeExpr(parser);
            return typeSpec;
        }

        Token typeName = parser->ReadToken(TokenType::Identifier);

        auto basicType = parser->astBuilder->create<VarExpr>();
        basicType->scope = parser->currentScope.Ptr();
        basicType->loc = typeName.loc;
        basicType->name = typeName.getNameOrNull();

        Expr* typeExpr = basicType;

        bool shouldLoop = true;
        while (shouldLoop)
        {
            switch (peekTokenType(parser))
            {
            case TokenType::OpLess:
                typeExpr = parseGenericApp(parser, typeExpr);
                break;
            case TokenType::Scope:
                parser->ReadToken(TokenType::Scope);
                typeExpr = parseMemberType(parser, typeExpr);
                break;
            case TokenType::Dot:
                parser->ReadToken(TokenType::Dot);
                typeExpr = parseMemberType(parser, typeExpr);
                break;
            default:
                shouldLoop = false;
            }
        }

        typeSpec.expr = typeExpr;
        return typeSpec;
    }

    static DeclBase* ParseDeclaratorDecl(
        Parser*         parser,
        ContainerDecl*  containerDecl)
    {
        SourceLoc startPosition = parser->tokenReader.peekLoc();

        auto typeSpec = parseTypeSpec(parser);

        // We may need to build up multiple declarations in a group,
        // but the common case will be when we have just a single
        // declaration
        DeclGroupBuilder declGroupBuilder;
        declGroupBuilder.startPosition = startPosition;
        declGroupBuilder.astBuilder = parser->astBuilder;

        // The type specifier may include a declaration. E.g.,
        // it might declare a `struct` type.
        if(typeSpec.decl)
            declGroupBuilder.addDecl(typeSpec.decl);

        if( AdvanceIf(parser, TokenType::Semicolon) )
        {
            // No actual variable is being declared here, but
            // that might not be an error.

            auto result = declGroupBuilder.getResult();
            if( !result )
            {
                parser->sink->diagnose(startPosition, Diagnostics::declarationDidntDeclareAnything);
            }
            return result;
        }

        // It is possible that we have a plain `struct`, `enum`,
        // or similar declaration that isn't being used to declare
        // any variable, and the user didn't put a trailing
        // semicolon on it:
        //
        //      struct Batman
        //      {
        //          int cape;
        //      }
        //
        // We want to allow this syntax (rather than give an
        // inscrutable error), but also support the less common
        // idiom where that declaration is used as part of
        // a variable declaration:
        //
        //      struct Robin
        //      {
        //          float tights;
        //      } boyWonder;
        //
        // As a bit of a hack (insofar as it means we aren't
        // *really* compatible with arbitrary HLSL code), we
        // will check if there are any more tokens on the
        // same line as the closing `}`, and if not, we
        // will treat it like the end of the declaration.
        //
        // Just as a safety net, only apply this logic for
        // a file that is being passed in as "true" Slang code.
        //
        if(parser->getSourceLanguage() == SourceLanguage::Slang)
        {
            if(typeSpec.decl)
            {
                if(peekToken(parser).flags & TokenFlag::AtStartOfLine)
                {
                    // The token after the `}` is at the start of its
                    // own line, which means it can't be on the same line.
                    //
                    // This means the programmer probably wants to
                    // just treat this as a declaration.
                    return declGroupBuilder.getResult();
                }
            }
        }


        InitDeclarator initDeclarator = parseInitDeclarator(parser, kDeclaratorParseOptions_None);

        DeclaratorInfo declaratorInfo;
        declaratorInfo.typeSpec = typeSpec.expr;


        // Rather than parse function declarators properly for now,
        // we'll just do a quick disambiguation here. This won't
        // matter unless we actually decide to support function-type parameters,
        // using C syntax.
        //
        if ((parser->tokenReader.peekTokenType() == TokenType::LParent ||
            parser->tokenReader.peekTokenType() == TokenType::OpLess)

            // Only parse as a function if we didn't already see mutually-exclusive
            // constructs when parsing the declarator.
            && !initDeclarator.initializer
            && !initDeclarator.semantics)
        {
            // Looks like a function, so parse it like one.
            UnwrapDeclarator(parser->astBuilder, initDeclarator, &declaratorInfo);
            return parseTraditionalFuncDecl(parser, declaratorInfo);
        }

        // Otherwise we are looking at a variable declaration, which could be one in a sequence...

        if( AdvanceIf(parser, TokenType::Semicolon) )
        {
            // easy case: we only had a single declaration!
            UnwrapDeclarator(parser->astBuilder, initDeclarator, &declaratorInfo);
            VarDeclBase* firstDecl = CreateVarDeclForContext(parser->astBuilder, containerDecl);
            CompleteVarDecl(parser, firstDecl, declaratorInfo);

            declGroupBuilder.addDecl(firstDecl);
            return declGroupBuilder.getResult();
        }

        // Otherwise we have multiple declarations in a sequence, and these
        // declarations need to somehow share both the type spec and modifiers.
        //
        // If there are any errors in the type specifier, we only want to hear
        // about it once, so we need to share structure rather than just
        // clone syntax.

        auto sharedTypeSpec = parser->astBuilder->create<SharedTypeExpr>();
        sharedTypeSpec->loc = typeSpec.expr->loc;
        sharedTypeSpec->base = TypeExp(typeSpec.expr);

        for(;;)
        {
            declaratorInfo.typeSpec = sharedTypeSpec;
            UnwrapDeclarator(parser->astBuilder, initDeclarator, &declaratorInfo);

            VarDeclBase* varDecl = CreateVarDeclForContext(parser->astBuilder, containerDecl);
            CompleteVarDecl(parser, varDecl, declaratorInfo);

            declGroupBuilder.addDecl(varDecl);

            // end of the sequence?
            if(AdvanceIf(parser, TokenType::Semicolon))
                return declGroupBuilder.getResult();

            // ad-hoc recovery, to avoid infinite loops
            if( parser->isRecovering )
            {
                parser->ReadToken(TokenType::Semicolon);
                return declGroupBuilder.getResult();
            }

            // Let's default to assuming that a missing `,`
            // indicates the end of a declaration,
            // where a `;` would be expected, and not
            // a continuation of this declaration, where
            // a `,` would be expected (this is tailoring
            // the diagnostic message a bit).
            //
            // TODO: a more advanced heuristic here might
            // look at whether the next token is on the
            // same line, to predict whether `,` or `;`
            // would be more likely...

            if (!AdvanceIf(parser, TokenType::Comma))
            {
                parser->ReadToken(TokenType::Semicolon);
                return declGroupBuilder.getResult();
            }

            // expect another variable declaration...
            initDeclarator = parseInitDeclarator(parser, kDeclaratorParseOptions_None);
        }
    }

    /// Parse the "register name" part of a `register` or `packoffset` semantic.
    ///
    /// The syntax matched is:
    ///
    ///     register-name-and-component-mask ::= register-name component-mask?
    ///     register-name ::= identifier
    ///     component-mask ::= '.' identifier
    ///
    static void parseHLSLRegisterNameAndOptionalComponentMask(
        Parser*             parser,
        HLSLLayoutSemantic* semantic)
    {
        semantic->registerName = parser->ReadToken(TokenType::Identifier);
        if (AdvanceIf(parser, TokenType::Dot))
        {
            semantic->componentMask = parser->ReadToken(TokenType::Identifier);
        }
    }

    /// Parse an HLSL `register` semantic.
    ///
    /// The syntax matched is:
    ///
    ///     register-semantic ::= 'register' '(' register-name-and-component-mask register-space? ')'
    ///     register-space ::= ',' identifier
    ///
    static void parseHLSLRegisterSemantic(
        Parser*                 parser,
        HLSLRegisterSemantic*   semantic)
    {
        // Read the `register` keyword
        semantic->name = parser->ReadToken(TokenType::Identifier);

        // Expect a parenthized list of additional arguments
        parser->ReadToken(TokenType::LParent);

        // First argument is a required register name and optional component mask
        parseHLSLRegisterNameAndOptionalComponentMask(parser, semantic);

        // Second argument is an optional register space
        if(AdvanceIf(parser, TokenType::Comma))
        {
            semantic->spaceName = parser->ReadToken(TokenType::Identifier);
        }

        parser->ReadToken(TokenType::RParent);
    }

    /// Parse an HLSL `packoffset` semantic.
    ///
    /// The syntax matched is:
    ///
    ///     packoffset-semantic ::= 'packoffset' '(' register-name-and-component-mask ')'
    ///
    static void parseHLSLPackOffsetSemantic(
        Parser*                 parser,
        HLSLPackOffsetSemantic* semantic)
    {
        // Read the `packoffset` keyword
        semantic->name = parser->ReadToken(TokenType::Identifier);

        // Expect a parenthized list of additional arguments
        parser->ReadToken(TokenType::LParent);

        // First and only argument is a required register name and optional component mask
        parseHLSLRegisterNameAndOptionalComponentMask(parser, semantic);

        parser->ReadToken(TokenType::RParent);

        parser->sink->diagnose(semantic, Diagnostics::packOffsetNotSupported);
    }

    //
    // semantic ::= identifier ( '(' args ')' )?
    //
    static Modifier* ParseSemantic(
        Parser* parser)
    {
        if (parser->LookAheadToken("register"))
        {
            HLSLRegisterSemantic* semantic = parser->astBuilder->create<HLSLRegisterSemantic>();
            parser->FillPosition(semantic);
            parseHLSLRegisterSemantic(parser, semantic);
            return semantic;
        }
        else if (parser->LookAheadToken("packoffset"))
        {
            HLSLPackOffsetSemantic* semantic = parser->astBuilder->create<HLSLPackOffsetSemantic>();
            parser->FillPosition(semantic);
            parseHLSLPackOffsetSemantic(parser, semantic);
            return semantic;
        }
        else if (parser->LookAheadToken(TokenType::Identifier))
        {
            HLSLSimpleSemantic* semantic = parser->astBuilder->create<HLSLSimpleSemantic>();
            parser->FillPosition(semantic);
            semantic->name = parser->ReadToken(TokenType::Identifier);
            return semantic;
        }
        else
        {
            // expect an identifier, just to produce an error message
            parser->ReadToken(TokenType::Identifier);
            return nullptr;
        }
    }

    //
    // opt-semantics ::= (':' semantic)*
    //
    static Modifier* ParseOptSemantics(
        Parser* parser)
    {
        if (!AdvanceIf(parser, TokenType::Colon))
            return nullptr;

        Modifier* result = nullptr;
        Modifier** link = &result;
        SLANG_ASSERT(!*link);

        for (;;)
        {
            Modifier* semantic = ParseSemantic(parser);
            if (semantic)
            {
                *link = semantic;
                link = &semantic->next;
            }

            // If we see another `:`, then that means there
            // is yet another semantic to be processed.
            // Otherwise we assume we are at the end of the list.
            //
            // TODO: This could produce sub-optimal diagnostics
            // when the user *meant* to apply multiple semantics
            // to a single declaration:
            //
            //     Foo foo : register(t0)   register(s0);
            //                            ^
            //         missing ':' here   |
            //
            // However, that is an uncommon occurence, and trying
            // to continue parsing semantics here even if we didn't
            // see a colon forces us to be careful about
            // avoiding an infinite loop here.
            if (!AdvanceIf(parser, TokenType::Colon))
            {
                return result;
            }
        }

    }


    static void ParseOptSemantics(
        Parser* parser,
        Decl*	decl)
    {
        AddModifiers(decl, ParseOptSemantics(parser));
    }

    static Decl* ParseHLSLBufferDecl(
        Parser*	parser,
        String  bufferWrapperTypeName)
    {
        // An HLSL declaration of a constant buffer like this:
        //
        //     cbuffer Foo : register(b0) { int a; float b; };
        //
        // is treated as syntax sugar for a type declaration
        // and then a global variable declaration using that type:
        //
        //     struct $anonymous { int a; float b; };
        //     ConstantBuffer<$anonymous> Foo;
        //
        // where `$anonymous` is a fresh name, and the variable
        // declaration is made to be "transparent" so that lookup
        // will see through it to the members inside.

        auto bufferWrapperTypeNamePos = parser->tokenReader.peekLoc();

        // We are going to represent each buffer as a pair of declarations.
        // The first is a type declaration that holds all the members, while
        // the second is a variable declaration that uses the buffer type.
        StructDecl* bufferDataTypeDecl = parser->astBuilder->create<StructDecl>();
        VarDecl* bufferVarDecl = parser->astBuilder->create<VarDecl>();

        // Both declarations will have a location that points to the name
        parser->FillPosition(bufferDataTypeDecl);
        parser->FillPosition(bufferVarDecl);

        auto reflectionNameToken = parser->ReadToken(TokenType::Identifier);

        // Attach the reflection name to the block so we can use it
        auto reflectionNameModifier = parser->astBuilder->create<ParameterGroupReflectionName>();
        reflectionNameModifier->nameAndLoc = NameLoc(reflectionNameToken);
        addModifier(bufferVarDecl, reflectionNameModifier);

        // Both the buffer variable and its type need to have names generated
        bufferVarDecl->nameAndLoc.name = generateName(parser, "parameterGroup_" + String(reflectionNameToken.getContent()));
        bufferDataTypeDecl->nameAndLoc.name = generateName(parser, "ParameterGroup_" + String(reflectionNameToken.getContent()));

        addModifier(bufferDataTypeDecl, parser->astBuilder->create<ImplicitParameterGroupElementTypeModifier>());
        addModifier(bufferVarDecl, parser->astBuilder->create<ImplicitParameterGroupVariableModifier>());

        // TODO(tfoley): We end up constructing unchecked syntax here that
        // is expected to type check into the right form, but it might be
        // cleaner to have a more explicit desugaring pass where we parse
        // these constructs directly into the AST and *then* desugar them.

        // Construct a type expression to reference the buffer data type
        auto bufferDataTypeExpr = parser->astBuilder->create<VarExpr>();
        bufferDataTypeExpr->loc = bufferDataTypeDecl->loc;
        bufferDataTypeExpr->name = bufferDataTypeDecl->nameAndLoc.name;
        bufferDataTypeExpr->scope = parser->currentScope.Ptr();

        // Construct a type expression to reference the type constructor
        auto bufferWrapperTypeExpr = parser->astBuilder->create<VarExpr>();
        bufferWrapperTypeExpr->loc = bufferWrapperTypeNamePos;
        bufferWrapperTypeExpr->name = getName(parser, bufferWrapperTypeName);

        // Always need to look this up in the outer scope,
        // so that it won't collide with, e.g., a local variable called `ConstantBuffer`
        bufferWrapperTypeExpr->scope = parser->outerScope;

        // Construct a type expression that represents the type for the variable,
        // which is the wrapper type applied to the data type
        auto bufferVarTypeExpr = parser->astBuilder->create<GenericAppExpr>();
        bufferVarTypeExpr->loc = bufferVarDecl->loc;
        bufferVarTypeExpr->functionExpr = bufferWrapperTypeExpr;
        bufferVarTypeExpr->arguments.add(bufferDataTypeExpr);

        bufferVarDecl->type.exp = bufferVarTypeExpr;

        // Any semantics applied to the buffer declaration are taken as applying
        // to the variable instead.
        ParseOptSemantics(parser, bufferVarDecl);

        // The declarations in the body belong to the data type.
        parseDeclBody(parser, bufferDataTypeDecl);

        // All HLSL buffer declarations are "transparent" in that their
        // members are implicitly made visible in the parent scope.
        // We achieve this by applying the transparent modifier to the variable.
        auto transparentModifier = parser->astBuilder->create<TransparentModifier>();
        transparentModifier->next = bufferVarDecl->modifiers.first;
        bufferVarDecl->modifiers.first = transparentModifier;

        // Because we are constructing two declarations, we have a thorny
        // issue that were are only supposed to return one.
        // For now we handle this by adding the type declaration to
        // the current scope manually, and then returning the variable
        // declaration.
        //
        // Note: this means that any modifiers that have already been parsed
        // will get attached to the variable declaration, not the type.
        // There might be cases where we need to shuffle things around.

        AddMember(parser->currentScope, bufferDataTypeDecl);

        return bufferVarDecl;
    }

    static NodeBase* parseHLSLCBufferDecl(
        Parser*	parser, void* /*userData*/)
    {
        return ParseHLSLBufferDecl(parser, "ConstantBuffer");
    }

    static NodeBase* parseHLSLTBufferDecl(
        Parser*	parser, void* /*userData*/)
    {
        return ParseHLSLBufferDecl(parser, "TextureBuffer");
    }

    static void parseOptionalInheritanceClause(Parser* parser, AggTypeDeclBase* decl)
    {
        if (AdvanceIf(parser, TokenType::Colon))
        {
            do
            {
                auto base = parser->ParseTypeExp();

                auto inheritanceDecl = parser->astBuilder->create<InheritanceDecl>();
                inheritanceDecl->loc = base.exp->loc;
                inheritanceDecl->nameAndLoc.name = getName(parser, "$inheritance");
                inheritanceDecl->base = base;

                AddMember(decl, inheritanceDecl);

            } while (AdvanceIf(parser, TokenType::Comma));
        }
    }

    static NodeBase* ParseExtensionDecl(Parser* parser, void* /*userData*/)
    {
        ExtensionDecl* decl = parser->astBuilder->create<ExtensionDecl>();
        parser->FillPosition(decl);
        decl->targetType = parser->ParseTypeExp();
        parseOptionalInheritanceClause(parser, decl);
        parseDeclBody(parser, decl);

        return decl;
    }


    static void parseOptionalGenericConstraints(Parser* parser, ContainerDecl* decl)
    {
        if (AdvanceIf(parser, TokenType::Colon))
        {
            do
            {
                GenericTypeConstraintDecl* paramConstraint = parser->astBuilder->create<GenericTypeConstraintDecl>();
                parser->FillPosition(paramConstraint);

                // substitution needs to be filled during check
                DeclRefType* paramType = DeclRefType::create(parser->astBuilder, DeclRef<Decl>(decl, nullptr));

                SharedTypeExpr* paramTypeExpr = parser->astBuilder->create<SharedTypeExpr>();
                paramTypeExpr->loc = decl->loc;
                paramTypeExpr->base.type = paramType;
                paramTypeExpr->type = QualType(parser->astBuilder->getTypeType(paramType));

                paramConstraint->sub = TypeExp(paramTypeExpr);
                paramConstraint->sup = parser->ParseTypeExp();

                AddMember(decl, paramConstraint);
            } while (AdvanceIf(parser, TokenType::Comma));
        }
    }

    static NodeBase* parseAssocType(Parser* parser, void *)
    {
        AssocTypeDecl* assocTypeDecl = parser->astBuilder->create<AssocTypeDecl>();

        auto nameToken = parser->ReadToken(TokenType::Identifier);
        assocTypeDecl->nameAndLoc = NameLoc(nameToken);
        assocTypeDecl->loc = nameToken.loc;
        parseOptionalGenericConstraints(parser, assocTypeDecl);
        parser->ReadToken(TokenType::Semicolon);
        return assocTypeDecl;
    }

    static NodeBase* parseGlobalGenericTypeParamDecl(Parser * parser, void *)
    {
        GlobalGenericParamDecl* genParamDecl = parser->astBuilder->create<GlobalGenericParamDecl>();
        auto nameToken = parser->ReadToken(TokenType::Identifier);
        genParamDecl->nameAndLoc = NameLoc(nameToken);
        genParamDecl->loc = nameToken.loc;
        parseOptionalGenericConstraints(parser, genParamDecl);
        parser->ReadToken(TokenType::Semicolon);
        return genParamDecl;
    }

    static NodeBase* parseGlobalGenericValueParamDecl(Parser * parser, void *)
    {
        GlobalGenericValueParamDecl* genericParamDecl = parser->astBuilder->create<GlobalGenericValueParamDecl>();
        auto nameToken = parser->ReadToken(TokenType::Identifier);
        genericParamDecl->nameAndLoc = NameLoc(nameToken);
        genericParamDecl->loc = nameToken.loc;

        if(AdvanceIf(parser, TokenType::Colon))
        {
            genericParamDecl->type = parser->ParseTypeExp();
        }

        if(AdvanceIf(parser, TokenType::OpAssign))
        {
            genericParamDecl->initExpr = parser->ParseInitExpr();
        }

        parser->ReadToken(TokenType::Semicolon);
        return genericParamDecl;
    }

    static NodeBase* parseInterfaceDecl(Parser* parser, void* /*userData*/)
    {
        InterfaceDecl* decl = parser->astBuilder->create<InterfaceDecl>();
        parser->FillPosition(decl);
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        parseOptionalInheritanceClause(parser, decl);

        parseDeclBody(parser, decl);

        return decl;
    }

    static NodeBase* parseNamespaceDecl(Parser* parser, void* /*userData*/)
    {
        // We start by parsing the name of the namespace that is being opened.
        //
        // TODO: We should eventually support a qualified name for
        // a namespace declaration:
        //
        //      namespace A.B { ... }
        //
        // which should expand as if the user had written nested
        // namespace declarations:
        //
        //      namespace A { namespace B { ... } }
        //
        // TODO: Support we also support the degenerate case of
        // a namesapce without a name? Should that be treated as
        // an anonymous (and implicitly imported) namespace, or
        // something else?
        //
        // TODO: Should we support a shorthand syntax for putting
        // the rest of the current scope/file into a namespace:
        //
        //      namespace A.B;
        //
        //      ...
        //
        NameLoc nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        // Once we have the name for the namespace, we face a challenge:
        // either the namespace hasn't been seen before (in which case
        // we need to create it and start filling it in), or we've seen
        // the same namespace before inside the same module, such that
        // we should be adding the declarations we parse to the existing
        // declarations (so that they share a common scope/parent).
        //
        // In each case we will find a namespace that we want to fill in,
        // but depending on the case we may or may not want to return
        // a declaration to the caller (since they will try to add
        // any non-null pointer we return to the AST).
        //
        NamespaceDecl* namespaceDecl = nullptr;
        NodeBase* result = nullptr;
        //
        // In order to find out what case we are in, we start by looking
        // for a namespace declaration of the same name in the parent
        // declaration.
        //
        {
            auto parentDecl = parser->currentScope->containerDecl;
            SLANG_ASSERT(parentDecl);

            // We meed to make sure that the member dictionary of
            // the parent declaration has been built/rebuilt so that
            // lookup by name will work.
            //
            // TODO: The current way we rebuild the member dictionary
            // would make for O(N^2) parsing time in a file that
            // consisted of N back-to-back `namespace`s, since each
            // would trigger a rebuild of the member dictionary that
            // would take O(N) time.
            //
            // Eventually we should make `builtMemberDictionary()`
            // incremental, so that it only has to process members
            // added since the last time it was invoked.
            //
            buildMemberDictionary(parentDecl);

            // There might be multiple members of the same name
            // (if we define a namespace `foo` after an overloaded
            // function `foo` has been defined), and direct member
            // lookup will only give us the first.
            //
            Decl* firstDecl = nullptr;
            parentDecl->memberDictionary.TryGetValue(nameAndLoc.name, firstDecl);
            //
            // We will search through the declarations of the name
            // and find the first that is a namespace (if any).
            //
            // Note: we do not issue diagnostics here based on
            // the potential conflicts between these declarations,
            // because we want to do as little semantic analysis
            // as possible in the parser, and we'd rather be
            // as permissive as possible right now.
            //
            for(Decl* d = firstDecl; d; d = d->nextInContainerWithSameName)
            {
                namespaceDecl = as<NamespaceDecl>(d);
                if(namespaceDecl)
                    break;
            }

            // If we didn't find a pre-existing namespace, then
            // we will go ahead and create one now.
            //
            if( !namespaceDecl )
            {
                namespaceDecl = parser->astBuilder->create<NamespaceDecl>();
                namespaceDecl->nameAndLoc = nameAndLoc;

                // In the case where we are creating the first
                // declaration of the given namesapce, we need
                // to use it as the return value of the parsing
                // callback, so that it is appropriately added
                // to the parent declaration.
                //
                result = namespaceDecl;
            }
        }

        // Now that we have a namespace declaration to fill in
        // (whether a new or existing one), we can parse the
        // `{}`-enclosed body to add declarations as children
        // of the namespace.
        //
        parseDeclBody(parser, namespaceDecl);

        return result;
    }

    static NodeBase* parseConstructorDecl(Parser* parser, void* /*userData*/)
    {
        ConstructorDecl* decl = parser->astBuilder->create<ConstructorDecl>();
        parser->FillPosition(decl);
        parser->PushScope(decl);

        // TODO: we need to make sure that all initializers have
        // the same name, but that this name doesn't conflict
        // with any user-defined names.
        // Giving them a name (rather than leaving it null)
        // ensures that we can use name-based lookup to find
        // all of the initializers on a type (and has
        // the potential to unify initializer lookup with
        // ordinary member lookup).
        decl->nameAndLoc.name = getName(parser, "$init");

        parseParameterList(parser, decl);

        decl->body = parseOptBody(parser);

        parser->PopScope();
        return decl;
    }

    static AccessorDecl* parseAccessorDecl(Parser* parser)
    {
        Modifiers modifiers = ParseModifiers(parser);

        AccessorDecl* decl = nullptr;
        auto loc = peekToken(parser).loc;
        if( AdvanceIf(parser, "get") )
        {
            decl = parser->astBuilder->create<GetterDecl>();
        }
        else if( AdvanceIf(parser, "set") )
        {
            decl = parser->astBuilder->create<SetterDecl>();
        }
        else if( AdvanceIf(parser, "ref") )
        {
            decl = parser->astBuilder->create<RefAccessorDecl>();
        }
        else
        {
            Unexpected(parser);
            return nullptr;
        }
        decl->loc = loc;

        AddModifiers(decl, modifiers.first);

        parser->PushScope(decl);

        // A `set` declaration should support declaring an explicit
        // name for the parameter representing the new value.
        //
        // We handle this by supporting an arbitrary parameter list
        // on any accessor, and then assume that semantic checking
        // will diagnose any cases that aren't allowed.
        //
        if(parser->tokenReader.peekTokenType() == TokenType::LParent)
        {
            parseModernParamList(parser, decl);
        }

        if( parser->tokenReader.peekTokenType() == TokenType::LBrace )
        {
            decl->body = parser->parseBlockStatement();
        }
        else
        {
            parser->ReadToken(TokenType::Semicolon);
        }

        parser->PopScope();


        return decl;
    }

    static void parseStorageDeclBody(Parser* parser, ContainerDecl* decl)
    {
        if( AdvanceIf(parser, TokenType::LBrace) )
        {
            // We want to parse nested "accessor" declarations
            while( !AdvanceIfMatch(parser, TokenType::RBrace) )
            {
                auto accessor = parseAccessorDecl(parser);
                AddMember(decl, accessor);
            }
        }
        else
        {
            parser->ReadToken(TokenType::Semicolon);

            // empty body should be treated like `{ get; }`
        }
    }

    static NodeBase* ParseSubscriptDecl(Parser* parser, void* /*userData*/)
    {
        SubscriptDecl* decl = parser->astBuilder->create<SubscriptDecl>();
        parser->FillPosition(decl);
        parser->PushScope(decl);

        // TODO: the use of this name here is a bit magical...
        decl->nameAndLoc.name = getName(parser, "operator[]");

        parseParameterList(parser, decl);

        if( AdvanceIf(parser, TokenType::RightArrow) )
        {
            decl->returnType = parser->ParseTypeExp();
        }

        parseStorageDeclBody(parser, decl);

        parser->PopScope();
        return decl;
    }

    static bool expect(Parser* parser, TokenType tokenType)
    {
        return parser->ReadToken(tokenType).type == tokenType;
    }

    static NodeBase* ParsePropertyDecl(Parser* parser, void* /*userData*/)
    {
        PropertyDecl* decl = parser->astBuilder->create<PropertyDecl>();
        parser->FillPosition(decl);
        parser->PushScope(decl);

        decl->nameAndLoc = expectIdentifier(parser);

        if( expect(parser, TokenType::Colon) )
        {
            decl->type = parser->ParseTypeExp();
        }

        parseStorageDeclBody(parser, decl);

        parser->PopScope();
        return decl;
    }

    static void parseModernVarDeclBaseCommon(
        Parser*             parser,
        VarDeclBase* decl)
    {
        parser->FillPosition(decl);
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        if(AdvanceIf(parser, TokenType::Colon))
        {
            decl->type = parser->ParseTypeExp();
        }

        if(AdvanceIf(parser, TokenType::OpAssign))
        {
            decl->initExpr = parser->ParseInitExpr();
        }
    }

    static void parseModernVarDeclCommon(
        Parser*         parser,
        VarDecl* decl)
    {
        parseModernVarDeclBaseCommon(parser, decl);
        expect(parser, TokenType::Semicolon);
    }

    static NodeBase* parseLetDecl(
        Parser* parser, void* /*userData*/)
    {
        LetDecl* decl = parser->astBuilder->create<LetDecl>();
        parseModernVarDeclCommon(parser, decl);
        return decl;
    }

    static NodeBase* parseVarDecl(
        Parser* parser, void* /*userData*/)
    {
        VarDecl* decl = parser->astBuilder->create<VarDecl>();
        parseModernVarDeclCommon(parser, decl);
        return decl;
    }

    static ParamDecl* parseModernParamDecl(
        Parser* parser)
    {
        ParamDecl* decl = parser->astBuilder->create<ParamDecl>();

        // TODO: "modern" parameters should not accept keyword-based
        // modifiers and should only accept `[attribute]` syntax for
        // modifiers to keep the grammar as simple as possible.
        //
        // Further, they should accept `out` and `in out`/`inout`
        // before the type (e.g., `a: inout float4`).
        //
        decl->modifiers = ParseModifiers(parser);
        parseModernVarDeclBaseCommon(parser, decl);
        return decl;
    }

    static void parseModernParamList(
        Parser*                 parser,
        CallableDecl*    decl)
    {
        parser->ReadToken(TokenType::LParent);

        while (!AdvanceIfMatch(parser, TokenType::RParent))
        {
            AddMember(decl, parseModernParamDecl(parser));
            if (AdvanceIf(parser, TokenType::RParent))
                break;
            parser->ReadToken(TokenType::Comma);
        }
    }

    static NodeBase* parseFuncDecl(
        Parser* parser, void* /*userData*/)
    {
        FuncDecl* decl = parser->astBuilder->create<FuncDecl>();

        parser->FillPosition(decl);
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        return parseOptGenericDecl(parser, [&](GenericDecl*)
        {
            parser->PushScope(decl);
            parseModernParamList(parser, decl);
            if(AdvanceIf(parser, TokenType::RightArrow))
            {
                decl->returnType = parser->ParseTypeExp();
            }
            decl->body = parseOptBody(parser);
            parser->PopScope();
            return decl;
        });
    }

    static NodeBase* parseTypeAliasDecl(
        Parser* parser, void* /*userData*/)
    {
        TypeAliasDecl* decl = parser->astBuilder->create<TypeAliasDecl>();

        parser->FillPosition(decl);
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        return parseOptGenericDecl(parser, [&](GenericDecl*)
        {
            if( expect(parser, TokenType::OpAssign) )
            {
                decl->type = parser->ParseTypeExp();
            }
            expect(parser, TokenType::Semicolon);
            return decl;
        });
    }

    // This is a catch-all syntax-construction callback to handle cases where
    // a piece of syntax is fully defined by the keyword to use, along with
    // the class of AST node to construct.
    static NodeBase* parseSimpleSyntax(Parser* parser, void* userData)
    {
        SyntaxClassBase syntaxClass((ReflectClassInfo*) userData);
        return (NodeBase*)syntaxClass.createInstanceImpl(parser->astBuilder);
    }

    // Parse a declaration of a keyword that can be used to define further syntax.
    static NodeBase* parseSyntaxDecl(Parser* parser, void* /*userData*/)
    {
        // Right now the basic form is:
        //
        // syntax <name:id> [: <syntaxClass:id>] [= <existingKeyword:id>];
        //
        // - `name` gives the name of the keyword to define.
        // - `syntaxClass` is the name of an AST node class that we expect
        //   this syntax to construct when parsed.
        // - `existingKeyword` is the name of an existing keyword that
        //   the new syntax should be an alias for.

        // First we parse the keyword name.
        auto nameAndLoc = expectIdentifier(parser);

        // Next we look for a clause that specified the AST node class.
        SyntaxClass<NodeBase> syntaxClass;
        if (AdvanceIf(parser, TokenType::Colon))
        {
            // User is specifying the class that should be construted
            auto classNameAndLoc = expectIdentifier(parser);

            syntaxClass = parser->astBuilder->findSyntaxClass(classNameAndLoc.name);
        }

        // If the user specified a syntax class, then we will default
        // to the `parseSimpleSyntax` callback that will just construct
        // an instance of that type to represent the keyword in the AST.
        SyntaxParseCallback parseCallback = &parseSimpleSyntax;
        void* parseUserData = (void*) syntaxClass.classInfo;

        // Next we look for an initializer that will make this keyword
        // an alias for some existing keyword.
        if (AdvanceIf(parser, TokenType::OpAssign))
        {
            auto existingKeywordNameAndLoc = expectIdentifier(parser);

            auto existingSyntax = tryLookUpSyntaxDecl(parser, existingKeywordNameAndLoc.name);
            if (!existingSyntax)
            {
                // TODO: diagnose: keyword did not name syntax
            }
            else
            {
                // The user is expecting us to parse our new syntax like
                // the existing syntax given, so we need to override
                // the callback.
                parseCallback = existingSyntax->parseCallback;
                parseUserData = existingSyntax->parseUserData;

                // If we don't already have a syntax class specified, then
                // we will crib the one from the existing syntax, to ensure
                // that we are creating a drop-in alias.
                if (!syntaxClass.classInfo)
                    syntaxClass = existingSyntax->syntaxClass;
            }
        }

        // It is an error if the user didn't give us either an existing keyword
        // to use to the define the callback, or a valid AST node class to construct.
        //
        // TODO: down the line this should be expanded so that the user can reference
        // an existing *function* to use to parse the chosen syntax.
        if (!syntaxClass.classInfo)
        {
            // TODO: diagnose: either a type or an existing keyword needs to be specified
        }

        expect(parser, TokenType::Semicolon);

        // TODO: skip creating the declaration if anything failed, just to not screw things
        // up for downstream code?

        SyntaxDecl* syntaxDecl = parser->astBuilder->create<SyntaxDecl>();
        syntaxDecl->nameAndLoc = nameAndLoc;
        syntaxDecl->loc = nameAndLoc.loc;
        syntaxDecl->syntaxClass = syntaxClass;
        syntaxDecl->parseCallback = parseCallback;
        syntaxDecl->parseUserData = parseUserData;
        return syntaxDecl;
    }

    // A parameter declaration in an attribute declaration.
    //
    // We are going to use `name: type` syntax just for simplicty, and let the type
    // be optional, because we don't actually need it in all cases.
    //
    static ParamDecl* parseAttributeParamDecl(Parser* parser)
    {
        auto nameAndLoc = expectIdentifier(parser);

        ParamDecl* paramDecl = parser->astBuilder->create<ParamDecl>();
        paramDecl->nameAndLoc = nameAndLoc;

        if(AdvanceIf(parser, TokenType::Colon))
        {
            paramDecl->type = parser->ParseTypeExp();
        }

        if(AdvanceIf(parser, TokenType::OpAssign))
        {
            paramDecl->initExpr = parser->ParseInitExpr();
        }

        return paramDecl;
    }

    // Parse declaration of a name to be used for resolving `[attribute(...)]` style modifiers.
    //
    // These are distinct from `syntax` declarations, because their names don't get added
    // to the current scope using their default name.
    //
    // Also, attribute-specific code doesn't get invokved during parsing. We always parse
    // using the default attribute-parsing logic and then all specialized behavior takes
    // place during semantic checking.
    //
    static NodeBase* parseAttributeSyntaxDecl(Parser* parser, void* /*userData*/)
    {
        // Right now the basic form is:
        //
        // attribute_syntax <name:id> : <syntaxClass:id>;
        //
        // - `name` gives the name of the attribute to define.
        // - `syntaxClass` is the name of an AST node class that we expect
        //   this attribute to create when checked.
        // - `existingKeyword` is the name of an existing keyword that
        //   the new syntax should be an alias for.

        expect(parser, TokenType::LBracket);

        // First we parse the attribute name.
        auto nameAndLoc = expectIdentifier(parser);

        AttributeDecl* attrDecl = parser->astBuilder->create<AttributeDecl>();
        if(AdvanceIf(parser, TokenType::LParent))
        {
            while(!AdvanceIfMatch(parser, TokenType::RParent))
            {
                auto param = parseAttributeParamDecl(parser);

                AddMember(attrDecl, param);

                if(AdvanceIfMatch(parser, TokenType::RParent))
                    break;

                expect(parser, TokenType::Comma);
            }
        }

        expect(parser, TokenType::RBracket);

        // TODO: we should allow parameters to be specified here, to cut down
        // on the amount of per-attribute-type logic that has to occur later.

        // Next we look for a clause that specified the AST node class.
        SyntaxClass<NodeBase> syntaxClass;
        if (AdvanceIf(parser, TokenType::Colon))
        {
            // User is specifying the class that should be construted
            auto classNameAndLoc = expectIdentifier(parser);

            syntaxClass = parser->astBuilder->findSyntaxClass(classNameAndLoc.name);
        }
        else
        {
            // For now we don't support the alternative approach where
            // an existing piece of syntax is named to provide the parsing
            // support.

            // TODO: diagnose: a syntax class must be specified.
        }

        expect(parser, TokenType::Semicolon);

        // TODO: skip creating the declaration if anything failed, just to not screw things
        // up for downstream code?

        attrDecl->nameAndLoc = nameAndLoc;
        attrDecl->loc = nameAndLoc.loc;
        attrDecl->syntaxClass = syntaxClass;
        return attrDecl;
    }

    // Finish up work on a declaration that was parsed
    static void CompleteDecl(
        Parser*				/*parser*/,
        Decl*		decl,
        ContainerDecl*		containerDecl,
        Modifiers			modifiers)
    {
        // Add any modifiers we parsed before the declaration to the list
        // of modifiers on the declaration itself.
        //
        // We need to be careful, because if `decl` is a generic declaration,
        // then we really want the modifiers to apply to the inner declaration.
        //
        Decl* declToModify = decl;
        if(auto genericDecl = as<GenericDecl>(decl))
            declToModify = genericDecl->inner;
        AddModifiers(declToModify, modifiers.first);

        // Make sure the decl is properly nested inside its lexical parent
        if (containerDecl)
        {
            AddMember(containerDecl, decl);
        }
    }

    static DeclBase* ParseDeclWithModifiers(
        Parser*             parser,
        ContainerDecl*      containerDecl,
        Modifiers			modifiers )
    {
        DeclBase* decl = nullptr;

        auto loc = parser->tokenReader.peekLoc();

        switch (peekTokenType(parser))
        {
        case TokenType::Identifier:
            {
                // A declaration that starts with an identifier might be:
                //
                // - A keyword-based declaration (e.g., `cbuffer ...`)
                // - The beginning of a type in a declarator-based declaration (e.g., `int ...`)

                // First we will check whether we can use the identifier token
                // as a declaration keyword and parse a declaration using
                // its associated callback:
                Decl* parsedDecl = nullptr;
                if (tryParseUsingSyntaxDecl<Decl>(parser, &parsedDecl))
                {
                    decl = parsedDecl;
                    break;
                }

                // Our final fallback case is to assume that the user is
                // probably writing a C-style declarator-based declaration.
                decl = ParseDeclaratorDecl(parser, containerDecl);
                break;
            }
            break;

        // It is valid in HLSL/GLSL to have an "empty" declaration
        // that consists of just a semicolon. In particular, this
        // gets used a lot in GLSL to attach custom semantics to
        // shader input or output.
        //
        case TokenType::Semicolon:
            {
                advanceToken(parser);

                decl = parser->astBuilder->create<EmptyDecl>();
                decl->loc = loc;
            }
            break;

        // If nothing else matched, we try to parse an "ordinary" declarator-based declaration
        default:
            decl = ParseDeclaratorDecl(parser, containerDecl);
            break;
        }

        if (decl)
        {
            if( auto dd = as<Decl>(decl) )
            {
                CompleteDecl(parser, dd, containerDecl, modifiers);
            }
            else if(auto declGroup = as<DeclGroup>(decl))
            {
                // We are going to add the same modifiers to *all* of these declarations,
                // so we want to give later passes a way to detect which modifiers
                // were shared, vs. which ones are specific to a single declaration.

                auto sharedModifiers = parser->astBuilder->create<SharedModifiers>();
                sharedModifiers->next = modifiers.first;
                modifiers.first = sharedModifiers;

                for( auto subDecl : declGroup->decls )
                {
                    CompleteDecl(parser, subDecl, containerDecl, modifiers);
                }
            }
        }
        return decl;
    }

    static DeclBase* ParseDecl(
        Parser*         parser,
        ContainerDecl*  containerDecl)
    {
        Modifiers modifiers = ParseModifiers(parser);
        return ParseDeclWithModifiers(parser, containerDecl, modifiers);
    }

    static Decl* ParseSingleDecl(
        Parser*			parser,
        ContainerDecl*	containerDecl)
    {
        auto declBase = ParseDecl(parser, containerDecl);
        if(!declBase)
            return nullptr;
        if( auto decl = as<Decl>(declBase) )
        {
            return decl;
        }
        else if( auto declGroup = as<DeclGroup>(declBase) )
        {
            if( declGroup->decls.getCount() == 1 )
            {
                return declGroup->decls[0];
            }
        }

        parser->sink->diagnose(declBase->loc, Diagnostics::unimplemented, "didn't expect multiple declarations here");
        return nullptr;
    }


    static void parseDecls(
        Parser*         parser,
        ContainerDecl*  containerDecl,
        TokenType       closingToken)
    {
        while(!AdvanceIfMatch(parser, closingToken))
        {
            ParseDecl(parser, containerDecl);
        }
    }

    static void parseDeclBody(
        Parser*         parser,
        ContainerDecl*  parent)
    {
        parser->PushScope(parent);

        parser->ReadToken(TokenType::LBrace);
        parseDecls(parser, parent, TokenType::RBrace);

        parser->PopScope();
    }


    void Parser::parseSourceFile(ModuleDecl* program)
    {
        if (outerScope)
        {
            currentScope = outerScope;
        }

        PushScope(program);

        // A single `ModuleDecl` might span multiple source files, so it
        // is possible that we are parsing a new source file into a module
        // that has already been created and filled in for a previous
        // source file.
        //
        // If this is the first source file for the module then we expect
        // its location information to be invalid, and we will set it to
        // refer to the start of the first source file.
        //
        // This convention is reasonable for any single-source-file module,
        // and about as good as possible for multiple-file modules.
        //
        if(!program->loc.isValid())
        {
            program->loc = tokenReader.peekLoc();
        }

        parseDecls(this, program, TokenType::EndOfFile);
        PopScope();

        SLANG_RELEASE_ASSERT(currentScope == outerScope);
        currentScope = nullptr;
    }

    Decl* Parser::ParseStruct()
    {
        StructDecl* rs = astBuilder->create<StructDecl>();
        FillPosition(rs);
        ReadToken("struct");

        // TODO: support `struct` declaration without tag
        rs->nameAndLoc = expectIdentifier(this);

        return parseOptGenericDecl(this, [&](GenericDecl*)
        {
            // We allow for an inheritance clause on a `struct`
            // so that it can conform to interfaces.
            parseOptionalInheritanceClause(this, rs);
            parseDeclBody(this, rs);
            return rs;
        });
    }

    ClassDecl* Parser::ParseClass()
    {
        ClassDecl* rs = astBuilder->create<ClassDecl>();
        FillPosition(rs);
        ReadToken("class");
        rs->nameAndLoc = expectIdentifier(this);

        parseOptionalInheritanceClause(this, rs);

        parseDeclBody(this, rs);
        return rs;
    }

    static EnumCaseDecl* parseEnumCaseDecl(Parser* parser)
    {
        EnumCaseDecl* decl = parser->astBuilder->create<EnumCaseDecl>();
        decl->nameAndLoc = expectIdentifier(parser);

        if(AdvanceIf(parser, TokenType::OpAssign))
        {
            decl->tagExpr = parser->ParseArgExpr();
        }

        return decl;
    }

    static Decl* parseEnumDecl(Parser* parser)
    {
        EnumDecl* decl = parser->astBuilder->create<EnumDecl>();
        parser->FillPosition(decl);

        parser->ReadToken("enum");

        // HACK: allow the user to write `enum class` in case
        // they are trying to share a header between C++ and Slang.
        //
        // TODO: diagnose this with a warning some day, and move
        // toward deprecating it.
        //
        AdvanceIf(parser, "class");

        decl->nameAndLoc = expectIdentifier(parser);


        return parseOptGenericDecl(parser, [&](GenericDecl*)
        {
            parseOptionalInheritanceClause(parser, decl);
            parser->ReadToken(TokenType::LBrace);

            while(!AdvanceIfMatch(parser, TokenType::RBrace))
            {
                EnumCaseDecl* caseDecl = parseEnumCaseDecl(parser);
                AddMember(decl, caseDecl);

                if(AdvanceIf(parser, TokenType::RBrace))
                    break;

                parser->ReadToken(TokenType::Comma);
            }
            return decl;
        });
    }

    static Stmt* ParseSwitchStmt(Parser* parser)
    {
        SwitchStmt* stmt = parser->astBuilder->create<SwitchStmt>();
        parser->FillPosition(stmt);
        parser->ReadToken("switch");
        parser->ReadToken(TokenType::LParent);
        stmt->condition = parser->ParseExpression();
        parser->ReadToken(TokenType::RParent);
        stmt->body = parser->parseBlockStatement();
        return stmt;
    }

    static Stmt* ParseCaseStmt(Parser* parser)
    {
        CaseStmt* stmt = parser->astBuilder->create<CaseStmt>();
        parser->FillPosition(stmt);
        parser->ReadToken("case");
        stmt->expr = parser->ParseExpression();
        parser->ReadToken(TokenType::Colon);
        return stmt;
    }

    static Stmt* ParseDefaultStmt(Parser* parser)
    {
        DefaultStmt* stmt = parser->astBuilder->create<DefaultStmt>();
        parser->FillPosition(stmt);
        parser->ReadToken("default");
        parser->ReadToken(TokenType::Colon);
        return stmt;
    }

    GpuForeachStmt* ParseGpuForeachStmt(Parser* parser)
    {
        // Hard-coding parsing of the following:
        // __GPU_FOREACH(renderer, gridDims, LAMBDA(uint3 dispatchThreadID) {
        //  kernelCall(args, ...); });

        // Setup the scope so that dispatchThreadID is in scope for kernelCall
        ScopeDecl* scopeDecl = parser->astBuilder->create<ScopeDecl>();
        GpuForeachStmt* stmt = parser->astBuilder->create<GpuForeachStmt>();
        stmt->scopeDecl = scopeDecl;

        parser->FillPosition(stmt);
        parser->ReadToken("__GPU_FOREACH");
        parser->ReadToken(TokenType::LParent);
        stmt->renderer = parser->ParseArgExpr();
        parser->ReadToken(TokenType::Comma);
        stmt->gridDims = parser->ParseArgExpr();

        parser->ReadToken(TokenType::Comma);
        parser->ReadToken("LAMBDA");
        parser->ReadToken(TokenType::LParent);

        auto idType = parser->ParseTypeExp();
        NameLoc varNameAndLoc = expectIdentifier(parser);
        VarDecl* varDecl = parser->astBuilder->create<VarDecl>();
        varDecl->nameAndLoc = varNameAndLoc;
        varDecl->loc = varNameAndLoc.loc;
        varDecl->type = idType;
        stmt->dispatchThreadID = varDecl;

        parser->ReadToken(TokenType::RParent);
        parser->ReadToken(TokenType::LBrace);

        parser->pushScopeAndSetParent(scopeDecl);
        AddMember(parser->currentScope, varDecl);

        stmt->kernelCall = parser->ParseExpression();

        parser->PopScope();

        parser->ReadToken(TokenType::Semicolon);
        parser->ReadToken(TokenType::RBrace);

        parser->ReadToken(TokenType::RParent);

        parser->ReadToken(TokenType::Semicolon);

        return stmt;
    }

    static bool _isType(Decl* decl)
    {
        return decl && (as<AggTypeDecl>(decl) || as<SimpleTypeDecl>(decl));
    }

    // TODO(JS):
    // This only handles StaticMemberExpr, and VarExpr lookup scenarios!
    static Decl* _tryResolveDecl(Parser* parser, Expr* expr)
    {
        if (auto staticMemberExpr = as<StaticMemberExpr>(expr))
        {
            Decl* baseTypeDecl = _tryResolveDecl(parser, staticMemberExpr->baseExpression);
            if (!baseTypeDecl)
            {
                return nullptr;
            }
            if (AggTypeDecl* aggTypeDecl = as<AggTypeDecl>(baseTypeDecl))
            {
                // TODO(JS):
                // Is it valid to always have empty substitution set here?
                DeclRef<ContainerDecl> declRef(aggTypeDecl, SubstitutionSet());

                auto lookupResult = lookUpDirectAndTransparentMembers(
                    parser->astBuilder,
                    nullptr, // no semantics visitor available yet
                    staticMemberExpr->name,
                    declRef);

                if (!lookupResult.isValid() || lookupResult.isOverloaded())
                    return nullptr;

                return lookupResult.item.declRef.getDecl();
            }

            // Didn't find it
            return nullptr;
        }

        if (auto varExpr = as<VarExpr>(expr))
        {
            // Do the lookup in the current scope
            auto lookupResult = lookUp(
                parser->astBuilder,
                nullptr, // no semantics visitor available yet
                varExpr->name,
                parser->currentScope);
            if (!lookupResult.isValid() || lookupResult.isOverloaded())
                return nullptr;

            return lookupResult.item.declRef.getDecl();
        }

        return nullptr;
    }

    static bool isTypeName(Parser* parser, Name* name)
    {
        auto lookupResult = lookUp(
            parser->astBuilder,
            nullptr, // no semantics visitor available yet
            name,
            parser->currentScope);
        if(!lookupResult.isValid() || lookupResult.isOverloaded())
            return false;

        return _isType(lookupResult.item.declRef.getDecl());
    }

    static bool peekTypeName(Parser* parser)
    {
        if(!parser->LookAheadToken(TokenType::Identifier))
            return false;

        auto name = parser->tokenReader.peekToken().getName();
        return isTypeName(parser, name);
    }

    Stmt* parseCompileTimeForStmt(
        Parser* parser)
    {
        ScopeDecl* scopeDecl = parser->astBuilder->create<ScopeDecl>();
        CompileTimeForStmt* stmt = parser->astBuilder->create<CompileTimeForStmt>();
        stmt->scopeDecl = scopeDecl;


        parser->ReadToken("for");
        parser->ReadToken(TokenType::LParent);

        NameLoc varNameAndLoc = expectIdentifier(parser);
        VarDecl* varDecl = parser->astBuilder->create<VarDecl>();
        varDecl->nameAndLoc = varNameAndLoc;
        varDecl->loc = varNameAndLoc.loc;

        stmt->varDecl = varDecl;

        parser->ReadToken("in");
        parser->ReadToken("Range");
        parser->ReadToken(TokenType::LParent);

        Expr* rangeBeginExpr = nullptr;
        Expr* rangeEndExpr = parser->ParseArgExpr();
        if (AdvanceIf(parser, TokenType::Comma))
        {
            rangeBeginExpr = rangeEndExpr;
            rangeEndExpr = parser->ParseArgExpr();
        }

        stmt->rangeBeginExpr = rangeBeginExpr;
        stmt->rangeEndExpr = rangeEndExpr;

        parser->ReadToken(TokenType::RParent);
        parser->ReadToken(TokenType::RParent);

        parser->pushScopeAndSetParent(scopeDecl);
        AddMember(parser->currentScope, varDecl);

        stmt->body = parser->ParseStatement();

        parser->PopScope();

        return stmt;
    }

    Stmt* parseCompileTimeStmt(
        Parser* parser)
    {
        parser->ReadToken(TokenType::Dollar);
        if (parser->LookAheadToken("for"))
        {
            return parseCompileTimeForStmt(parser);
        }
        else
        {
            Unexpected(parser);
            return nullptr;
        }
    }

    Stmt* Parser::ParseStatement()
    {
        auto modifiers = ParseModifiers(this);

        Stmt* statement = nullptr;
        if (LookAheadToken(TokenType::LBrace))
            statement = parseBlockStatement();
        else if (LookAheadToken("if"))
            statement = parseIfStatement();
        else if (LookAheadToken("for"))
            statement = ParseForStatement();
        else if (LookAheadToken("while"))
            statement = ParseWhileStatement();
        else if (LookAheadToken("do"))
            statement = ParseDoWhileStatement();
        else if (LookAheadToken("break"))
            statement = ParseBreakStatement();
        else if (LookAheadToken("continue"))
            statement = ParseContinueStatement();
        else if (LookAheadToken("return"))
            statement = ParseReturnStatement();
        else if (LookAheadToken("discard"))
        {
            statement = astBuilder->create<DiscardStmt>();
            FillPosition(statement);
            ReadToken("discard");
            ReadToken(TokenType::Semicolon);
        }
        else if (LookAheadToken("switch"))
            statement = ParseSwitchStmt(this);
        else if (LookAheadToken("case"))
            statement = ParseCaseStmt(this);
        else if (LookAheadToken("default"))
            statement = ParseDefaultStmt(this);
        else if (LookAheadToken("__GPU_FOREACH"))
            statement = ParseGpuForeachStmt(this);
        else if (LookAheadToken(TokenType::Dollar))
        {
            statement = parseCompileTimeStmt(this);
        }
        else if (LookAheadToken(TokenType::Identifier))
        {
            // We might be looking at a local declaration, or an
            // expression statement, and we need to figure out which.
            //
            // We'll solve this with backtracking for now.
            //
            // TODO: This should not require backtracking at all.

            TokenReader::ParsingCursor startPos = tokenReader.getCursor();

            // Try to parse a type (knowing that the type grammar is
            // a subset of the expression grammar, and so this should
            // always succeed).
            //
            // HACK: The type grammar that `ParseType` supports is *not*
            // a subset of the expression grammar because it includes
            // type specififers like `struct` and `enum` declarations
            // which should always be the start of a declaration.
            //
            // TODO: Before launching into this attempt to parse a type,
            // this logic should really be looking up the `SyntaxDecl`,
            // if any, assocaited with the identifier. If a piece of
            // syntax is discovered, then it should dictate the next
            // steps of parsing, and only in the case where the lookahead
            // isn't a keyword should we fall back to the approach
            // here.
            //
            Expr* type = ParseType();

            // We don't actually care about the type, though, so
            // don't retain it
            //
            // TODO: There is no reason to throw away the work we
            // did parsing the `type` expression. Once we disambiguate
            // what to do, we should be able to use the expression
            // we already parsed as a starting point for whatever we
            // parse next. E.g., if we have `A.b` and the lookahead is `+`
            // then we can use `A.b` as the left-hand-side expression
            // when starting to parse an infix expression.
            //
            type = nullptr;

            // TODO: If we decide to intermix parsing of statement bodies
            // with semantic checking (by delaying the parsing of bodies
            // until semantic context is available), then we could look
            // at the *type* of `type` to disambiguate what to do next,
            // which might result in a nice simplification (at the cost
            // of definitely making the grammar context-dependent).

            // If the next token after we parsed a type looks like
            // we are going to declare a variable, then lets guess
            // that this is a declaration.
            //
            // TODO(tfoley): this wouldn't be robust for more
            // general kinds of declarators (notably pointer declarators),
            // so we'll need to be careful about this.
            //
            // If the lookahead token is `*`, then we have to decide
            // whether to parse a pointer declarator or a multiply
            // expression. In this context it makes sense to disambiguate
            // in favor of a pointer over a multiply, since a multiply
            // expression can't appear at the start of a statement
            // with any side effects.
            //
            //
            if (LookAheadToken(TokenType::Identifier))
            {
                // Reset the cursor and try to parse a declaration now.
                // Note: the declaration will consume any modifiers
                // that had been in place on the statement.
                tokenReader.setCursor(startPos);
                statement = parseVarDeclrStatement(modifiers);
                return statement;
            }

            // Fallback: reset and parse an expression
            tokenReader.setCursor(startPos);
            statement = ParseExpressionStatement();
        }
        else if (LookAheadToken(TokenType::Semicolon))
        {
            statement = astBuilder->create<EmptyStmt>();
            FillPosition(statement);
            ReadToken(TokenType::Semicolon);
        }
        else
        {
            // Default case should always fall back to parsing an expression,
            // and then let that detect any errors
            statement = ParseExpressionStatement();
        }

        if (statement && !as<DeclStmt>(statement))
        {
            // Install any modifiers onto the statement.
            // Note: this path is bypassed in the case of a
            // declaration statement, so we don't end up
            // doubling up the modifiers.
            statement->modifiers = modifiers;
        }

        return statement;
    }

    Stmt* Parser::parseBlockStatement()
    {
        ScopeDecl* scopeDecl = astBuilder->create<ScopeDecl>();
        BlockStmt* blockStatement = astBuilder->create<BlockStmt>();
        blockStatement->scopeDecl = scopeDecl;
        pushScopeAndSetParent(scopeDecl);
        ReadToken(TokenType::LBrace);

        Stmt* body = nullptr;

        if(!tokenReader.isAtEnd())
        {
            FillPosition(blockStatement);
        }
        while (!AdvanceIfMatch(this, TokenType::RBrace))
        {
            auto stmt = ParseStatement();
            if(stmt)
            {
                if (!body)
                {
                    body = stmt;
                }
                else if (auto seqStmt = as<SeqStmt>(body))
                {
                    seqStmt->stmts.add(stmt);
                }
                else
                {
                    SeqStmt* newBody = astBuilder->create<SeqStmt>();
                    newBody->loc = blockStatement->loc;
                    newBody->stmts.add(body);
                    newBody->stmts.add(stmt);

                    body = newBody;
                }
            }
            TryRecover(this);
        }
        PopScope();

        if(!body)
        {
            body = astBuilder->create<EmptyStmt>();
            body->loc = blockStatement->loc;
        }

        blockStatement->body = body;
        return blockStatement;
    }

    DeclStmt* Parser::parseVarDeclrStatement(
        Modifiers modifiers)
    {
        DeclStmt*varDeclrStatement = astBuilder->create<DeclStmt>();

        FillPosition(varDeclrStatement);
        auto decl = ParseDeclWithModifiers(this, currentScope->containerDecl, modifiers);
        varDeclrStatement->decl = decl;
        return varDeclrStatement;
    }

    IfStmt* Parser::parseIfStatement()
    {
        IfStmt* ifStatement = astBuilder->create<IfStmt>();
        FillPosition(ifStatement);
        ReadToken("if");
        ReadToken(TokenType::LParent);
        ifStatement->predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        ifStatement->positiveStatement = ParseStatement();
        if (LookAheadToken("else"))
        {
            ReadToken("else");
            ifStatement->negativeStatement = ParseStatement();
        }
        return ifStatement;
    }

    ForStmt* Parser::ParseForStatement()
    {
        ScopeDecl* scopeDecl = astBuilder->create<ScopeDecl>();

        // HLSL implements the bad approach to scoping a `for` loop
        // variable, and we want to respect that, but *only* when
        // parsing HLSL code.
        //

        bool brokenScoping = getSourceLanguage() == SourceLanguage::HLSL;

        // We will create a distinct syntax node class for the unscoped
        // case, just so that we can correctly handle it in downstream
        // logic.
        //
        ForStmt* stmt = nullptr;
        if (brokenScoping)
        {
            stmt = astBuilder->create<UnscopedForStmt>();
        }
        else
        {
            stmt = astBuilder->create<ForStmt>();
        }

        stmt->scopeDecl = scopeDecl;

        if(!brokenScoping)
            pushScopeAndSetParent(scopeDecl);
        FillPosition(stmt);
        ReadToken("for");
        ReadToken(TokenType::LParent);
        if (peekTypeName(this))
        {
            stmt->initialStatement = parseVarDeclrStatement(Modifiers());
        }
        else
        {
            if (!LookAheadToken(TokenType::Semicolon))
            {
                stmt->initialStatement = ParseExpressionStatement();
            }
            else
            {
                ReadToken(TokenType::Semicolon);
            }
        }
        if (!LookAheadToken(TokenType::Semicolon))
            stmt->predicateExpression = ParseExpression();
        ReadToken(TokenType::Semicolon);
        if (!LookAheadToken(TokenType::RParent))
            stmt->sideEffectExpression = ParseExpression();
        ReadToken(TokenType::RParent);
        stmt->statement = ParseStatement();

        if (!brokenScoping)
            PopScope();

        return stmt;
    }

    WhileStmt* Parser::ParseWhileStatement()
    {
        WhileStmt* whileStatement = astBuilder->create<WhileStmt>();
        FillPosition(whileStatement);
        ReadToken("while");
        ReadToken(TokenType::LParent);
        whileStatement->predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        whileStatement->statement = ParseStatement();
        return whileStatement;
    }

    DoWhileStmt* Parser::ParseDoWhileStatement()
    {
        DoWhileStmt* doWhileStatement = astBuilder->create<DoWhileStmt>();
        FillPosition(doWhileStatement);
        ReadToken("do");
        doWhileStatement->statement = ParseStatement();
        ReadToken("while");
        ReadToken(TokenType::LParent);
        doWhileStatement->predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        ReadToken(TokenType::Semicolon);
        return doWhileStatement;
    }

    BreakStmt* Parser::ParseBreakStatement()
    {
        BreakStmt* breakStatement = astBuilder->create<BreakStmt>();
        FillPosition(breakStatement);
        ReadToken("break");
        ReadToken(TokenType::Semicolon);
        return breakStatement;
    }

    ContinueStmt* Parser::ParseContinueStatement()
    {
        ContinueStmt* continueStatement = astBuilder->create<ContinueStmt>();
        FillPosition(continueStatement);
        ReadToken("continue");
        ReadToken(TokenType::Semicolon);
        return continueStatement;
    }

    ReturnStmt* Parser::ParseReturnStatement()
    {
        ReturnStmt* returnStatement = astBuilder->create<ReturnStmt>();
        FillPosition(returnStatement);
        ReadToken("return");
        if (!LookAheadToken(TokenType::Semicolon))
            returnStatement->expression = ParseExpression();
        ReadToken(TokenType::Semicolon);
        return returnStatement;
    }

    ExpressionStmt* Parser::ParseExpressionStatement()
    {
        ExpressionStmt* statement = astBuilder->create<ExpressionStmt>();

        FillPosition(statement);
        statement->expression = ParseExpression();

        ReadToken(TokenType::Semicolon);
        return statement;
    }

    ParamDecl* Parser::ParseParameter()
    {
        ParamDecl* parameter = astBuilder->create<ParamDecl>();
        parameter->modifiers = ParseModifiers(this);

        DeclaratorInfo declaratorInfo;
        declaratorInfo.typeSpec = ParseType();

        InitDeclarator initDeclarator = parseInitDeclarator(this, kDeclaratorParseOption_AllowEmpty);
        UnwrapDeclarator(astBuilder, initDeclarator, &declaratorInfo);

        // Assume it is a variable-like declarator
        CompleteVarDecl(this, parameter, declaratorInfo);
        return parameter;
    }

    Expr* Parser::ParseType()
    {
        auto typeSpec = parseTypeSpec(this);
        if( typeSpec.decl )
        {
            AddMember(currentScope, typeSpec.decl);
        }
        auto typeExpr = typeSpec.expr;

        typeExpr = parsePostfixTypeSuffix(this, typeExpr);

        return typeExpr;
    }



    TypeExp Parser::ParseTypeExp()
    {
        return TypeExp(ParseType());
    }

    enum class Associativity
    {
        Left, Right
    };



    Associativity GetAssociativityFromLevel(Precedence level)
    {
        if (level == Precedence::Assignment)
            return Associativity::Right;
        else
            return Associativity::Left;
    }




    Precedence GetOpLevel(Parser* parser, TokenType type)
    {
        switch(type)
        {
        case TokenType::QuestionMark:
            return Precedence::TernaryConditional;
        case TokenType::Comma:
            return Precedence::Comma;
        case TokenType::OpAssign:
        case TokenType::OpMulAssign:
        case TokenType::OpDivAssign:
        case TokenType::OpAddAssign:
        case TokenType::OpSubAssign:
        case TokenType::OpModAssign:
        case TokenType::OpShlAssign:
        case TokenType::OpShrAssign:
        case TokenType::OpOrAssign:
        case TokenType::OpAndAssign:
        case TokenType::OpXorAssign:
            return Precedence::Assignment;
        case TokenType::OpOr:
            return Precedence::LogicalOr;
        case TokenType::OpAnd:
            return Precedence::LogicalAnd;
        case TokenType::OpBitOr:
            return Precedence::BitOr;
        case TokenType::OpBitXor:
            return Precedence::BitXor;
        case TokenType::OpBitAnd:
            return Precedence::BitAnd;
        case TokenType::OpEql:
        case TokenType::OpNeq:
            return Precedence::EqualityComparison;
        case TokenType::OpGreater:
        case TokenType::OpGeq:
            // Don't allow these ops inside a generic argument
            if (parser->genericDepth > 0) return Precedence::Invalid;
            ; // fall-thru
        case TokenType::OpLeq:
        case TokenType::OpLess:
            return Precedence::RelationalComparison;
        case TokenType::OpRsh:
            // Don't allow this op inside a generic argument
            if (parser->genericDepth > 0) return Precedence::Invalid;
            ; // fall-thru
        case TokenType::OpLsh:
            return Precedence::BitShift;
        case TokenType::OpAdd:
        case TokenType::OpSub:
            return Precedence::Additive;
        case TokenType::OpMul:
        case TokenType::OpDiv:
        case TokenType::OpMod:
            return Precedence::Multiplicative;
        default:
            return Precedence::Invalid;
        }
    }

    static Expr* parseOperator(Parser* parser)
    {
        Token opToken;
        switch(parser->tokenReader.peekTokenType())
        {
        case TokenType::QuestionMark:
            opToken = parser->ReadToken();
            opToken.setContent(UnownedStringSlice::fromLiteral("?:"));
            break;

        default:
            opToken = parser->ReadToken();
            break;
        }

        auto opExpr = parser->astBuilder->create<VarExpr>();
        opExpr->name = getName(parser, opToken.getContent());
        opExpr->scope = parser->currentScope;
        opExpr->loc = opToken.loc;

        return opExpr;

    }

    static Expr* createInfixExpr(
        Parser*         parser,
        Expr*    left,
        Expr*    op,
        Expr*    right)
    {
        InfixExpr* expr = parser->astBuilder->create<InfixExpr>();
        expr->loc = op->loc;
        expr->functionExpr = op;
        expr->arguments.add(left);
        expr->arguments.add(right);
        return expr;
    }

    static Expr* parseInfixExprWithPrecedence(
        Parser*                         parser,
        Expr*    inExpr,
        Precedence                      prec)
    {
        auto expr = inExpr;
        for(;;)
        {
            auto opTokenType = parser->tokenReader.peekTokenType();
            auto opPrec = GetOpLevel(parser, opTokenType);
            if(opPrec < prec)
                break;

            auto op = parseOperator(parser);

            // Special case the `?:` operator since it is the
            // one non-binary case we need to deal with.
            if(opTokenType == TokenType::QuestionMark)
            {
                SelectExpr* select = parser->astBuilder->create<SelectExpr>();
                select->loc = op->loc;
                select->functionExpr = op;

                select->arguments.add(expr);

                select->arguments.add(parser->ParseExpression(opPrec));
                parser->ReadToken(TokenType::Colon);
                select->arguments.add(parser->ParseExpression(opPrec));

                expr = select;
                continue;
            }

            auto right = parser->ParseLeafExpression();

            for(;;)
            {
                auto nextOpPrec = GetOpLevel(parser, parser->tokenReader.peekTokenType());

                if((GetAssociativityFromLevel(nextOpPrec) == Associativity::Right) ? (nextOpPrec < opPrec) : (nextOpPrec <= opPrec))
                    break;

                right = parseInfixExprWithPrecedence(parser, right, nextOpPrec);
            }

            if (opTokenType == TokenType::OpAssign)
            {
                AssignExpr* assignExpr = parser->astBuilder->create<AssignExpr>();
                assignExpr->loc = op->loc;
                assignExpr->left = expr;
                assignExpr->right = right;

                expr = assignExpr;
            }
            else
            {
                expr = createInfixExpr(parser, expr, op, right);
            }
        }
        return expr;
    }

    Expr* Parser::ParseExpression(Precedence level)
    {
        auto expr = ParseLeafExpression();
        return parseInfixExprWithPrecedence(this, expr, level);

#if 0

        if (level == Precedence::Prefix)
            return ParseLeafExpression();
        if (level == Precedence::TernaryConditional)
        {
            // parse select clause
            auto condition = ParseExpression(Precedence(level + 1));
            if (LookAheadToken(TokenType::QuestionMark))
            {
                SelectExpr* select = new SelectExpr();
                FillPosition(select.Ptr());

                select->Arguments.Add(condition);

                select->FunctionExpr = parseOperator(this);

                select->Arguments.Add(ParseExpression(level));
                ReadToken(TokenType::Colon);
                select->Arguments.Add(ParseExpression(level));
                return select;
            }
            else
                return condition;
        }
        else
        {
            if (GetAssociativityFromLevel(level) == Associativity::Left)
            {
                auto left = ParseExpression(Precedence(level + 1));
                while (GetOpLevel(this, tokenReader.PeekTokenType()) == level)
                {
                    OperatorExpr* tmp = new InfixExpr();
                    tmp->FunctionExpr = parseOperator(this);

                    tmp->Arguments.Add(left);
                    FillPosition(tmp.Ptr());
                    tmp->Arguments.Add(ParseExpression(Precedence(level + 1)));
                    left = tmp;
                }
                return left;
            }
            else
            {
                auto left = ParseExpression(Precedence(level + 1));
                if (GetOpLevel(this, tokenReader.PeekTokenType()) == level)
                {
                    OperatorExpr* tmp = new InfixExpr();
                    tmp->Arguments.Add(left);
                    FillPosition(tmp.Ptr());
                    tmp->FunctionExpr = parseOperator(this);
                    tmp->Arguments.Add(ParseExpression(level));
                    left = tmp;
                }
                return left;
            }
        }
#endif
    }

    // We *might* be looking at an application of a generic to arguments,
    // but we need to disambiguate to make sure.
    static Expr* maybeParseGenericApp(
        Parser*                             parser,

        // TODO: need to support more general expressions here
        Expr*     base)
    {
        if(peekTokenType(parser) != TokenType::OpLess)
            return base;
        return tryParseGenericApp(parser, base);
    }

    static Expr* parsePrefixExpr(Parser* parser);

    // Parse OOP `this` expression syntax
    static NodeBase* parseThisExpr(Parser* parser, void* /*userData*/)
    {
        ThisExpr* expr = parser->astBuilder->create<ThisExpr>();
        expr->scope = parser->currentScope;
        return expr;
    }

    static Expr* parseBoolLitExpr(Parser* parser, bool value)
    {
        BoolLiteralExpr* expr = parser->astBuilder->create<BoolLiteralExpr>();
        expr->value = value;
        return expr;
    }

    static NodeBase* parseTrueExpr(Parser* parser, void* /*userData*/)
    {
        return parseBoolLitExpr(parser, true);
    }

    static NodeBase* parseFalseExpr(Parser* parser, void* /*userData*/)
    {
        return parseBoolLitExpr(parser, false);
    }

    static bool _isFinite(double value)
    {
        // Lets type pun double to uint64_t, so we can detect special double values
        union
        {
            double d;
            uint64_t i;
        } u = { value };
        // Detects nan and +-inf
        const uint64_t i = u.i;
        int e = int(i >> 52) & 0x7ff;
        return (e != 0x7ff);
    }

    enum class FloatFixKind
    {
        None,               ///< No modification was made
        Unrepresentable,    ///< Unrepresentable
        Zeroed,             ///< Too close to 0
        Truncated,          ///< Truncated to a non zero value
    };

    static FloatFixKind _fixFloatLiteralValue(BaseType type, IRFloatingPointValue value, IRFloatingPointValue& outValue)
    {
        IRFloatingPointValue epsilon = 1e-10f;

        // Check the value is finite for checking narrowing to literal type losing information
        if (_isFinite(value))
        {
            switch (type)
            {
                case BaseType::Float:
                {
                    // Fix out of range
                    if (value > FLT_MAX)
                    {
                        if (Math::AreNearlyEqual(value, FLT_MAX, epsilon))
                        {
                            outValue = FLT_MAX;
                            return FloatFixKind::Truncated;
                        }
                        else
                        {
                            outValue = float(INFINITY);
                            return FloatFixKind::Unrepresentable;
                        }
                    }
                    else if (value < -FLT_MAX)
                    {
                        if (Math::AreNearlyEqual(-value, FLT_MAX, epsilon))
                        {
                            outValue = -FLT_MAX;
                            return FloatFixKind::Truncated;
                        }
                        else
                        {
                            outValue = -float(INFINITY);
                            return FloatFixKind::Unrepresentable;
                        }
                    }
                    else if (value && float(value) == 0.0f)
                    {
                        outValue = 0.0f;
                        return FloatFixKind::Zeroed;
                    }
                    break;
                }
                case BaseType::Double:
                {
                    // All representable
                    break;
                }
                case BaseType::Half:
                {
                    // Fix out of range
                    if (value > SLANG_HALF_MAX)
                    {
                        if (Math::AreNearlyEqual(value, FLT_MAX, epsilon))
                        {
                            outValue = SLANG_HALF_MAX;
                            return FloatFixKind::Truncated;
                        }
                        else
                        {
                            outValue = float(INFINITY);
                            return FloatFixKind::Unrepresentable;
                        }
                    }
                    else if (value < -SLANG_HALF_MAX)
                    {
                        if (Math::AreNearlyEqual(-value, FLT_MAX, epsilon))
                        {
                            outValue = -SLANG_HALF_MAX;
                            return FloatFixKind::Truncated;
                        }
                        else
                        {
                            outValue = -float(INFINITY);
                            return FloatFixKind::Unrepresentable;
                        }
                    }
                    else if (value && Math::Abs(value) < SLANG_HALF_SUB_NORMAL_MIN)
                    {
                        outValue = 0.0f;
                        return FloatFixKind::Zeroed;
                    }
                    break;
                }
                default: break;
            }
        }

        outValue = value;
        return FloatFixKind::None;
    }

    static IntegerLiteralValue _fixIntegerLiteral(BaseType baseType, IntegerLiteralValue value, Token* token, DiagnosticSink* sink)
    {
        // TODO(JS):
        // It is worth noting here that because of the way that the lexer works, that literals
        // are always handled as if they are positive (a preceding - is taken as a negate on a
        // positive value).
        // The code here is designed to work with positive and negative values, as this behavior
        // might change in the future, and is arguably more 'correct'.

        const BaseTypeInfo& info = BaseTypeInfo::getInfo(baseType);
        SLANG_ASSERT(info.flags & BaseTypeInfo::Flag::Integer);
        SLANG_COMPILE_TIME_ASSERT(sizeof(value) == sizeof(uint64_t));

        // If the type is 64 bits, do nothing, we'll assume all is good
        if (baseType != BaseType::Void && info.sizeInBytes != sizeof(value))
        {
            const IntegerLiteralValue signBit = IntegerLiteralValue(1) << (8 * info.sizeInBytes - 1);
            // Same as (~IntegerLiteralValue(0)) << (8 * info.sizeInBytes);, without the need for variable shift
            const IntegerLiteralValue mask = -(signBit + signBit);

            IntegerLiteralValue truncatedValue = value;
            // If it's signed, and top bit is set, sign extend it negative
            if (info.flags & BaseTypeInfo::Flag::Signed)
            {
                // Sign extend
                truncatedValue = (value & signBit) ? (value | mask) : (value & ~mask);
            }
            else
            {
                // 0 top bits
                truncatedValue = value & ~mask;
            }

            const IntegerLiteralValue maskedValue = value & mask;

            // If the masked value is 0 or equal to the mask, we 'assume' no information is
            // lost
            // This allows for example -1u, to give 0xffffffff
            // It also means 0xfffffffffffffffffu will give 0xffffffff, without a warning.
            if ((!(maskedValue == 0 || maskedValue == mask)) && sink && token)
            {
                // Output a warning that number has been altered
                sink->diagnose(*token, Diagnostics::integerLiteralTruncated, token->getContent(), BaseTypeInfo::asText(baseType), truncatedValue);
            }

            value = truncatedValue;
        }

        return value;
    }

    static bool _isCast(Parser* parser, Expr* expr)
    {

        // We can't just look at expr and look up if it's a type, because we allow
        // out-of-order declarations. So to a first approximation we'll try and
        // determine if it is a cast via a heuristic based on what comes next

        TokenType tokenType = peekTokenType(parser);

        // Expression
        // ==========
        //
        // Misc: ; ) [ ] , . = ? (ternary) { } ++ -- -> 
        // Binary ops: * / | & ^ % << >> 
        // Logical ops: || &&  
        // Comparisons: != == < > <= =>
        //
        // Any assign op
        // 
        // If we don't have pointers then
        // & : (Thing::Another) &v 
        // * : (Thing::Another)*ptr is a cast.
        //
        // Cast
        // ====
        //
        // Misc: (
        // Identifier, Literal
        // Unary ops: !, ~
        // 
        // Ambiguous
        // =========
        //
        // - : Can be unary and therefore a cast or a binary subtract, and therefore an expression
        // + : Can be unary and therefore could be a cast, or a binary add and therefore an expression
        //
        // Arbitrary
        // =========
        //
        // End of file, End of directive, Invalid, :, ::

        switch (tokenType)
        {
            case TokenType::FloatingPointLiteral:
            case TokenType::CharLiteral:
            case TokenType::IntegerLiteral:
            case TokenType::Identifier:
            case TokenType::OpNot:
            case TokenType::OpBitNot:
            {
                // If followed by one of these, must be a cast
                return true;
            }
            case TokenType::LParent:
            {
                // If we are followed by ( it might not be a cast - it could be a call invocation.
                // BUT we can always *assume* it is a call, because such a 'call' will be correctly
                // handled as a cast if necessary later.
                return false;
            }
            case TokenType::OpAdd:
            case TokenType::OpSub:
            {
                // + - are ambiguous, it could be a binary + or - so -> expression, or unary -> cast
                //
                // (Some::Stuff) + 3
                // (Some::Stuff) - 3
                // Strictly I can only tell if this is an expression or a cast if I know Some::Stuff is a type or not
                // but we can't know here in general because we allow out-of-order declarations.

                // If we can determine it's a type, then it must be a cast, and we are done.
                // 
                // NOTE! This test can only determine if it's a type *iff* it has already been defined. A future out
                // of order declaration, will not be correctly found here.
                //
                // This means the semantics change depending on the order of definition (!)
                Decl* decl = _tryResolveDecl(parser, expr);
                // If we can find the decl-> we can resolve unambiguously
                if (decl)
                {
                    return _isType(decl);
                }

                // Now we use a heuristic.
                //
                // Whitespace before, whitespace after->binary
                // No whitespace before, no whitespace after->binary
                // Otherwise->unary
                //
                // Unary -> cast, binary -> expression.
                // 
                // Ie:
                // (Some::Stuff) +3  - must be a cast
                // (Some::Stuff)+ 3  - must be a cast (?) This is a bit odd.
                // (Some::Stuff) + 3 - must be an expression.
                // (Some::Stuff)+3 - must be an expression.

                // TODO(JS): This covers the (SomeScope::Identifier) case
                //
                // But perhaps there other ways of referring to types, that this now misses? With associated types/generics perhaps.
                // 
                // For now we'll assume it's not a cast if it's not a StaticMemberExpr
                // The reason for the restriction (which perhaps can be broadened), is we don't
                // want the interpretation of something in parentheses to be determined by something as common as + or - whitespace.

                if (auto staticMemberExpr = dynamicCast<StaticMemberExpr>(expr))
                {
                    // Apply the heuristic:
                    TokenReader::ParsingCursor cursor = parser->tokenReader.getCursor();
                    // Skip the + or -
                    const Token opToken = advanceToken(parser);
                    // Peek the next token to see if it was preceded by white space
                    const Token nextToken = peekToken(parser);

                    // Rewind
                    parser->tokenReader.setCursor(cursor);

                    const bool isBinary = (nextToken.flags & TokenFlag::AfterWhitespace) == (opToken.flags & TokenFlag::AfterWhitespace);

                    // If it's binary it's not a cast
                    return !isBinary;
                }
                break;
            }
            default: break;
        }

        // We'll assume it's not a cast
        return false;
    }

    static Expr* parseAtomicExpr(Parser* parser)
    {
        switch( peekTokenType(parser) )
        {
        default:
            // TODO: should this return an error expression instead of NULL?
            parser->sink->diagnose(parser->tokenReader.peekLoc(), Diagnostics::syntaxError);
            return nullptr;

        // Either:
        // - parenthesized expression `(exp)`
        // - cast `(type) exp`
        //
        // Proper disambiguation requires mixing up parsing
        // and semantic checking (which we should do eventually)
        // but for now we will follow some heuristics.
        case TokenType::LParent:
            {
                Token openParen = parser->ReadToken(TokenType::LParent);

                if (peekTypeName(parser) && parser->LookAheadToken(TokenType::RParent, 1))
                {
                    TypeCastExpr* tcexpr = parser->astBuilder->create<ExplicitCastExpr>();
                    parser->FillPosition(tcexpr);
                    tcexpr->functionExpr = parser->ParseType();
                    parser->ReadToken(TokenType::RParent);

                    auto arg = parsePrefixExpr(parser);
                    tcexpr->arguments.add(arg);

                    return tcexpr;
                }
                else
                {
                    // The above branch catches the case where we have a cast like (Thing), but with
                    // the scoping operator it will not handle (SomeScope::Thing). In that case this
                    // branch will be taken. This is okay in so far as SomeScope::Thing will parse
                    // as an expression.
                    
                    Expr* base = parser->ParseExpression();

                    parser->ReadToken(TokenType::RParent);

                    // We now try and determine by what base is, if this is actually a cast or an expression in parentheses
                    if (_isCast(parser, base))
                    {
                        // Parse as a cast

                        TypeCastExpr* tcexpr = parser->astBuilder->create<ExplicitCastExpr>();
                        tcexpr->loc = openParen.loc;

                        tcexpr->functionExpr = base;

                        auto arg = parsePrefixExpr(parser);
                        tcexpr->arguments.add(arg);

                        return tcexpr;
                    }
                    else
                    {
                        // Pass as an expression in parentheses

                        ParenExpr* parenExpr = parser->astBuilder->create<ParenExpr>();
                        parenExpr->loc = openParen.loc;
                        parenExpr->base = base;
                        return parenExpr;
                    }
                }
            }

        // An initializer list `{ expr, ... }`
        case TokenType::LBrace:
            {
                InitializerListExpr* initExpr = parser->astBuilder->create<InitializerListExpr>();
                parser->FillPosition(initExpr);

                // Initializer list
                parser->ReadToken(TokenType::LBrace);

                List<Expr*> exprs;

                for(;;)
                {
                    if(AdvanceIfMatch(parser, TokenType::RBrace))
                        break;

                    auto expr = parser->ParseArgExpr();
                    if( expr )
                    {
                        initExpr->args.add(expr);
                    }

                    if(AdvanceIfMatch(parser, TokenType::RBrace))
                        break;

                    parser->ReadToken(TokenType::Comma);
                }

                return initExpr;
            }

        case TokenType::IntegerLiteral:
            {
                IntegerLiteralExpr* constExpr = parser->astBuilder->create<IntegerLiteralExpr>();
                parser->FillPosition(constExpr);

                auto token = parser->tokenReader.advanceToken();
                constExpr->token = token;

                UnownedStringSlice suffix;
                IntegerLiteralValue value = getIntegerLiteralValue(token, &suffix);

                // Look at any suffix on the value
                char const* suffixCursor = suffix.begin();
                const char*const suffixEnd = suffix.end();

                // If no suffix is defined go with the default
                BaseType suffixBaseType = BaseType::Int;

                if( suffixCursor < suffixEnd )
                {
                    // Mark as void, taken as an error
                    suffixBaseType = BaseType::Void;

                    int lCount = 0;
                    int uCount = 0;
                    int unknownCount = 0;
                    while(suffixCursor < suffixEnd)
                    {
                        switch( *suffixCursor++ )
                        {
                        case 'l': case 'L':
                            lCount++;
                            break;

                        case 'u': case 'U':
                            uCount++;
                            break;

                        default:
                            unknownCount++;
                            break;
                        }
                    }

                    if(unknownCount)
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidIntegerLiteralSuffix, suffix);
                        suffixBaseType = BaseType::Void;
                    }
                    // `u` or `ul` suffix -> `uint`
                    else if(uCount == 1 && (lCount <= 1))
                    {
                        suffixBaseType = BaseType::UInt;
                    }
                    // `l` suffix on integer -> `int` (== `long`)
                    else if(lCount == 1 && !uCount)
                    {
                        suffixBaseType = BaseType::Int; 
                    }
                    // `ull` suffix -> `uint64_t`
                    else if(uCount == 1 && lCount == 2)
                    {
                        suffixBaseType = BaseType::UInt64;
                    }
                    // `ll` suffix -> `int64_t`
                    else if(uCount == 0 && lCount == 2)
                    {
                        suffixBaseType = BaseType::Int64;
                    }
                    // TODO: do we need suffixes for smaller integer types?
                    else
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidIntegerLiteralSuffix, suffix);
                        suffixBaseType = BaseType::Void;
                    }
                }

                value = _fixIntegerLiteral(suffixBaseType, value, &token, parser->sink);

                ASTBuilder* astBuilder = parser->astBuilder;
                Type* suffixType = (suffixBaseType == BaseType::Void) ? astBuilder->getErrorType() : astBuilder->getBuiltinType(suffixBaseType);

                constExpr->value = value;
                constExpr->type = QualType(suffixType);

                return constExpr;
            }


        case TokenType::FloatingPointLiteral:
            {
                FloatingPointLiteralExpr* constExpr = parser->astBuilder->create<FloatingPointLiteralExpr>();
                parser->FillPosition(constExpr);

                auto token = parser->tokenReader.advanceToken();
                constExpr->token = token;

                UnownedStringSlice suffix;
                FloatingPointLiteralValue value = getFloatingPointLiteralValue(token, &suffix);

                // Look at any suffix on the value
                char const* suffixCursor = suffix.begin();
                const char*const suffixEnd = suffix.end();

                // Default is Float
                BaseType suffixBaseType = BaseType::Float;
                if( suffixCursor < suffixEnd )
                {
                    int fCount = 0;
                    int lCount = 0;
                    int hCount = 0;
                    int unknownCount = 0;
                    while(suffixCursor < suffixEnd)
                    {
                        switch( *suffixCursor++ )
                        {
                        case 'f': case 'F':
                            fCount++;
                            break;

                        case 'l': case 'L':
                            lCount++;
                            break;

                        case 'h': case 'H':
                            hCount++;
                            break;

                        default:
                            unknownCount++;
                            break;
                        }
                    }

                    if (unknownCount)
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidFloatingPointLiteralSuffix, suffix);
                        suffixBaseType = BaseType::Void;
                    }
                    // `f` suffix -> `float`
                    if(fCount == 1 && !lCount && !hCount)
                    {
                        suffixBaseType = BaseType::Float;
                    }
                    // `l` or `lf` suffix on floating-point literal -> `double`
                    else if(lCount == 1 && (fCount <= 1))
                    {
                        suffixBaseType = BaseType::Double;
                    }
                    // `h` or `hf` suffix on floating-point literal -> `half`
                    else if(hCount == 1 && (fCount <= 1))
                    {
                        suffixBaseType = BaseType::Half;
                    }
                    // TODO: are there other suffixes we need to handle?
                    else
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidFloatingPointLiteralSuffix, suffix);
                        suffixBaseType = BaseType::Void;
                    }
                }

                // TODO(JS):
                // It is worth noting here that because of the way that the lexer works, that literals
                // are always handled as if they are positive (a preceding - is taken as a negate on a
                // positive value).
                // The code here is designed to work with positive and negative values, as this behavior
                // might change in the future, and is arguably more 'correct'.

                FloatingPointLiteralValue fixedValue = value;
                auto fixType = _fixFloatLiteralValue(suffixBaseType, value, fixedValue);

                switch (fixType)
                {
                    case FloatFixKind::Truncated: 
                    case FloatFixKind::None:
                    {
                        // No warning.
                        // The truncation allowed must be very small. When Truncated the value *is* changed though.
                        break;
                    }
                    case FloatFixKind::Zeroed:
                    {
                        parser->sink->diagnose(token, Diagnostics::floatLiteralTooSmall, BaseTypeInfo::asText(suffixBaseType), token.getContent(), fixedValue);
                        break;
                    }
                    case FloatFixKind::Unrepresentable:
                    {
                        parser->sink->diagnose(token, Diagnostics::floatLiteralUnrepresentable, BaseTypeInfo::asText(suffixBaseType), token.getContent(), fixedValue);
                        break;
                    }
                }

                ASTBuilder* astBuilder = parser->astBuilder;

                Type* suffixType = (suffixBaseType == BaseType::Void) ? astBuilder->getErrorType() : astBuilder->getBuiltinType(suffixBaseType);

                constExpr->value = fixedValue;
                constExpr->type = QualType(suffixType);

                return constExpr;
            }

        case TokenType::StringLiteral:
            {
                StringLiteralExpr* constExpr = parser->astBuilder->create<StringLiteralExpr>();
                auto token = parser->tokenReader.advanceToken();
                constExpr->token = token;
                parser->FillPosition(constExpr);

                if (!parser->LookAheadToken(TokenType::StringLiteral))
                {
                    // Easy/common case: a single string
                    constExpr->value = getStringLiteralTokenValue(token);
                }
                else
                {
                    StringBuilder sb;
                    sb << getStringLiteralTokenValue(token);
                    while (parser->LookAheadToken(TokenType::StringLiteral))
                    {
                        token = parser->tokenReader.advanceToken();
                        sb << getStringLiteralTokenValue(token);
                    }
                    constExpr->value = sb.ProduceString();
                }

                return constExpr;
            }

        case TokenType::Identifier:
            {
                // We will perform name lookup here so that we can find syntax
                // keywords registered for use as expressions.
                Token nameToken = peekToken(parser);

                Expr* parsedExpr = nullptr;
                if (tryParseUsingSyntaxDecl<Expr>(parser, &parsedExpr))
                {
                    if (!parsedExpr->loc.isValid())
                    {
                        parsedExpr->loc = nameToken.loc;
                    }
                    return parsedExpr;
                }

                // Default behavior is just to create a name expression
                VarExpr* varExpr = parser->astBuilder->create<VarExpr>();
                varExpr->scope = parser->currentScope;
                parser->FillPosition(varExpr);

                auto nameAndLoc = expectIdentifier(parser);
                varExpr->name = nameAndLoc.name;

                if(peekTokenType(parser) == TokenType::OpLess)
                {
                    return maybeParseGenericApp(parser, varExpr);
                }

                return varExpr;
            }
        }
    }

    static Expr* parsePostfixExpr(Parser* parser)
    {
        auto expr = parseAtomicExpr(parser);
        for(;;)
        {
            switch( peekTokenType(parser) )
            {
            default:
                return expr;

            // Postfix increment/decrement
            case TokenType::OpInc:
            case TokenType::OpDec:
                {
                    OperatorExpr* postfixExpr = parser->astBuilder->create<PostfixExpr>();
                    parser->FillPosition(postfixExpr);
                    postfixExpr->functionExpr = parseOperator(parser);
                    postfixExpr->arguments.add(expr);

                    expr = postfixExpr;
                }
                break;

            // Subscript operation `a[i]`
            case TokenType::LBracket:
                {
                    IndexExpr* indexExpr = parser->astBuilder->create<IndexExpr>();
                    indexExpr->baseExpression = expr;
                    parser->FillPosition(indexExpr);
                    parser->ReadToken(TokenType::LBracket);
                    // TODO: eventually we may want to support multiple arguments inside the `[]`
                    if (!parser->LookAheadToken(TokenType::RBracket))
                    {
                        indexExpr->indexExpression = parser->ParseExpression();
                    }
                    parser->ReadToken(TokenType::RBracket);

                    expr = indexExpr;
                }
                break;

            // Call oepration `f(x)`
            case TokenType::LParent:
                {
                    InvokeExpr* invokeExpr = parser->astBuilder->create<InvokeExpr>();
                    invokeExpr->functionExpr = expr;
                    parser->FillPosition(invokeExpr);
                    parser->ReadToken(TokenType::LParent);
                    while (!parser->tokenReader.isAtEnd())
                    {
                        if (!parser->LookAheadToken(TokenType::RParent))
                            invokeExpr->arguments.add(parser->ParseArgExpr());
                        else
                        {
                            break;
                        }
                        if (!parser->LookAheadToken(TokenType::Comma))
                            break;
                        parser->ReadToken(TokenType::Comma);
                    }
                    parser->ReadToken(TokenType::RParent);

                    expr = invokeExpr;
                }
                break;

            // Scope access `x::m`
            case TokenType::Scope:
            {
                StaticMemberExpr* staticMemberExpr = parser->astBuilder->create<StaticMemberExpr>();

                // TODO(tfoley): why would a member expression need this?
                staticMemberExpr->scope = parser->currentScope;

                parser->FillPosition(staticMemberExpr);
                staticMemberExpr->baseExpression = expr;
                parser->ReadToken(TokenType::Scope);
                staticMemberExpr->name = expectIdentifier(parser).name;

                if (peekTokenType(parser) == TokenType::OpLess)
                    expr = maybeParseGenericApp(parser, staticMemberExpr);
                else
                    expr = staticMemberExpr;

                break;
            }
            // Member access `x.m`
            case TokenType::Dot:
                {
                    MemberExpr* memberExpr = parser->astBuilder->create<MemberExpr>();

                    // TODO(tfoley): why would a member expression need this?
                    memberExpr->scope = parser->currentScope.Ptr();

                    parser->FillPosition(memberExpr);
                    memberExpr->baseExpression = expr;
                    parser->ReadToken(TokenType::Dot);
                    memberExpr->name = expectIdentifier(parser).name;

                    if (peekTokenType(parser) == TokenType::OpLess)
                        expr = maybeParseGenericApp(parser, memberExpr);
                    else
                        expr = memberExpr;
                }
                break;
            }
        }
    }

    static IRIntegerValue _foldIntegerPrefixOp(TokenType tokenType, IRIntegerValue value)
    {
        switch (tokenType)
        {
            case TokenType::OpBitNot:   return ~value;
            case TokenType::OpAdd:      return value;
            case TokenType::OpSub:      return -value;
            default:
            {
                SLANG_ASSERT(!"Unexpected op");
                return value;
            }
        }
    }

    static IRFloatingPointValue _foldFloatPrefixOp(TokenType tokenType, IRFloatingPointValue value)
    {
        switch (tokenType)
        {
            case TokenType::OpAdd:      return value;
            case TokenType::OpSub:      return -value;
            default:
            {
                SLANG_ASSERT(!"Unexpected op");
                return value;
            }
        }
    }

    static Expr* parsePrefixExpr(Parser* parser)
    {
        auto tokenType = peekTokenType(parser);
        switch( tokenType )
        {
        default:
            return parsePostfixExpr(parser);

        case TokenType::OpNot:
        case TokenType::OpInc:
        case TokenType::OpDec:
        {
            PrefixExpr* prefixExpr = parser->astBuilder->create<PrefixExpr>();
            parser->FillPosition(prefixExpr);
            prefixExpr->functionExpr = parseOperator(parser);

            auto arg = parsePrefixExpr(parser);

            prefixExpr->arguments.add(arg);
            return prefixExpr;
        }
        case TokenType::OpBitNot:
        case TokenType::OpAdd:
        case TokenType::OpSub:
            {
                PrefixExpr* prefixExpr = parser->astBuilder->create<PrefixExpr>();
                parser->FillPosition(prefixExpr);
                prefixExpr->functionExpr = parseOperator(parser);

                auto arg = parsePrefixExpr(parser);

                if (auto intLit = as<IntegerLiteralExpr>(arg))
                {
                    IntegerLiteralExpr* newLiteral = parser->astBuilder->create<IntegerLiteralExpr>(*intLit);

                    IRIntegerValue value = _foldIntegerPrefixOp(tokenType, newLiteral->value);

                    // Need to get the basic type, so we can fit to underlying type
                    if (auto basicExprType = as<BasicExpressionType>(intLit->type.type))
                    {
                        value = _fixIntegerLiteral(basicExprType->baseType, value, nullptr, nullptr);
                    }

                    newLiteral->value = value;
                    return newLiteral;
                }
                else if (auto floatLit = as<FloatingPointLiteralExpr>(arg))
                {
                    FloatingPointLiteralExpr* newLiteral = parser->astBuilder->create<FloatingPointLiteralExpr>(*floatLit);
                    newLiteral->value = _foldFloatPrefixOp(tokenType, floatLit->value);
                    return newLiteral;
                }

                prefixExpr->arguments.add(arg);
                return prefixExpr;
            }
        
            break;
        }
    }

    Expr* Parser::ParseLeafExpression()
    {
        return parsePrefixExpr(this);
    }

    Expr* parseTermFromSourceFile(
        ASTBuilder*                     astBuilder,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope,
        NamePool*                       namePool,
        SourceLanguage                  sourceLanguage)
    {
        Parser parser(astBuilder, tokens, sink, outerScope);
        parser.currentScope = outerScope;
        parser.namePool = namePool;
        parser.sourceLanguage = sourceLanguage;
        return parser.ParseExpression();
    }

    // Parse a source file into an existing translation unit
    void parseSourceFile(
        ASTBuilder*                     astBuilder,
        TranslationUnitRequest*         translationUnit,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope)
    {
        Parser parser(astBuilder, tokens, sink, outerScope);
        parser.namePool = translationUnit->getNamePool();
        parser.sourceLanguage = translationUnit->sourceLanguage;

        return parser.parseSourceFile(translationUnit->getModuleDecl());
    }

    static void addBuiltinSyntaxImpl(
        Session*                    session,
        Scope*                      scope,
        char const*                 nameText,
        SyntaxParseCallback         callback,
        void*                       userData,
        SyntaxClass<NodeBase>      syntaxClass)
    {
        Name* name = session->getNamePool()->getName(nameText);

        ASTBuilder* globalASTBuilder = session->getGlobalASTBuilder();

        SyntaxDecl* syntaxDecl = globalASTBuilder->create<SyntaxDecl>();
        syntaxDecl->nameAndLoc = NameLoc(name);
        syntaxDecl->syntaxClass = syntaxClass;
        syntaxDecl->parseCallback = callback;
        syntaxDecl->parseUserData = userData;

        AddMember(scope, syntaxDecl);
    }

    template<typename T>
    static void addBuiltinSyntax(
        Session*            session,
        Scope*              scope,
        char const*         name,
        SyntaxParseCallback callback,
        void*               userData = nullptr)
    {
        addBuiltinSyntaxImpl(session, scope, name, callback, userData, getClass<T>());
    }

    template<typename T>
    static void addSimpleModifierSyntax(
        Session*        session,
        Scope*          scope,
        char const*     name)
    {
        auto syntaxClass = getClass<T>();
        addBuiltinSyntaxImpl(session, scope, name, &parseSimpleSyntax, (void*) syntaxClass.classInfo, getClass<T>());
    }

    static NodeBase* parseIntrinsicOpModifier(Parser* parser, void* /*userData*/)
    {
        IntrinsicOpModifier* modifier = parser->astBuilder->create<IntrinsicOpModifier>();

        // We allow a few difference forms here:
        //
        // First, we can specify the intrinsic op `enum` value directly:
        //
        //     __intrinsic_op(<integer literal>)
        //
        // Second, we can specify the operation by name:
        //
        //     __intrinsic_op(<identifier>)
        //
        // Finally, we can leave off the specification, so that the
        // op name will be derived from the function name:
        //
        //     __intrinsic_op
        //
        if (AdvanceIf(parser, TokenType::LParent))
        {
            if (AdvanceIf(parser, TokenType::OpSub))
            {
                modifier->op = IROp(-StringToInt(parser->ReadToken().getContent()));
            }
            else if (parser->LookAheadToken(TokenType::IntegerLiteral))
            {
                modifier->op = IROp(StringToInt(parser->ReadToken().getContent()));
            }
            else
            {
                modifier->opToken = parser->ReadToken(TokenType::Identifier);

                modifier->op = findIROp(modifier->opToken.getContent());

                if (modifier->op == kIROp_Invalid)
                {
                    parser->sink->diagnose(modifier->opToken, Diagnostics::unimplemented, "unknown intrinsic op");
                }
            }

            parser->ReadToken(TokenType::RParent);
        }


        return modifier;
    }

    static NodeBase* parseTargetIntrinsicModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = parser->astBuilder->create<TargetIntrinsicModifier>();

        if (AdvanceIf(parser, TokenType::LParent))
        {
            modifier->targetToken = parser->ReadToken(TokenType::Identifier);

            if( AdvanceIf(parser, TokenType::Comma) )
            {
                if( parser->LookAheadToken(TokenType::StringLiteral) )
                {
                    modifier->definitionToken = parser->ReadToken();
                }
                else
                {
                    modifier->definitionToken = parser->ReadToken(TokenType::Identifier);
                }
            }

            parser->ReadToken(TokenType::RParent);
        }

        return modifier;
    }

    static NodeBase* parseSpecializedForTargetModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = parser->astBuilder->create<SpecializedForTargetModifier>();
        if (AdvanceIf(parser, TokenType::LParent))
        {
            modifier->targetToken = parser->ReadToken(TokenType::Identifier);
            parser->ReadToken(TokenType::RParent);
        }
        return modifier;
    }

    static NodeBase* parseGLSLExtensionModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = parser->astBuilder->create<RequiredGLSLExtensionModifier>();

        parser->ReadToken(TokenType::LParent);
        modifier->extensionNameToken = parser->ReadToken(TokenType::Identifier);
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static NodeBase* parseGLSLVersionModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = parser->astBuilder->create<RequiredGLSLVersionModifier>();

        parser->ReadToken(TokenType::LParent);
        modifier->versionNumberToken = parser->ReadToken(TokenType::IntegerLiteral);
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }


    static SlangResult parseSemanticVersion(Parser* parser, Token& outToken, SemanticVersion& outVersion)
    {
        parser->ReadToken(TokenType::LParent);
        outToken = parser->ReadToken();
        parser->ReadToken(TokenType::RParent);

        UnownedStringSlice content = outToken.getContent();
        // We allow specified as major.minor or as a string (in quotes)
        switch (outToken.type)
        {
            case TokenType::FloatingPointLiteral:
            {
                break;
            }
            case TokenType::StringLiteral:
            {
                // We need to trim quotes if needed
                SLANG_ASSERT(content.getLength() >= 2 && content[0] == '"' && content[content.getLength() - 1] == '"');
                content = UnownedStringSlice(content.begin() + 1, content.end() - 1);
                break;
            }
            default:
            {
                return SLANG_FAIL;
            }
        }
        return SemanticVersion::parse(content, outVersion);
    }

    static NodeBase* parseSPIRVVersionModifier(Parser* parser, void* /*userData*/)
    {
        Token token;
        SemanticVersion version;
        if (SLANG_SUCCEEDED(parseSemanticVersion(parser, token, version)))
        {
            auto modifier = parser->astBuilder->create<RequiredSPIRVVersionModifier>();
            modifier->version = version;
            return modifier;
        }
        parser->sink->diagnose(token, Diagnostics::invalidSPIRVVersion);
        return nullptr;
    }

    static NodeBase* parseCUDASMVersionModifier(Parser* parser, void* /*userData*/)
    {
        Token token;
        SemanticVersion version;
        if (SLANG_SUCCEEDED(parseSemanticVersion(parser, token, version)))
        {
            auto modifier = parser->astBuilder->create<RequiredCUDASMVersionModifier>();
            modifier->version = version;
            return modifier;
        }
        parser->sink->diagnose(token, Diagnostics::invalidCUDASMVersion);
        return nullptr;
    }

    static NodeBase* parseLayoutModifier(Parser* parser, void* /*userData*/)
    {
        ModifierListBuilder listBuilder;

        UncheckedAttribute* numThreadsAttrib = nullptr;

        listBuilder.add(parser->astBuilder->create<GLSLLayoutModifierGroupBegin>());
        
        parser->ReadToken(TokenType::LParent);
        while (!AdvanceIfMatch(parser, TokenType::RParent))
        {
            auto nameAndLoc = expectIdentifier(parser);
            const String& nameText = nameAndLoc.name->text;

            const char localSizePrefix[] = "local_size_";

            int localSizeIndex = -1;
            if (nameText.startsWith(localSizePrefix) && nameText.getLength() == SLANG_COUNT_OF(localSizePrefix) - 1 + 1)
            {
                char lastChar = nameText[SLANG_COUNT_OF(localSizePrefix) - 1];
                localSizeIndex = (lastChar >= 'x' && lastChar <= 'z') ? (lastChar - 'x') : -1;
            }

            if (localSizeIndex >= 0)
            {
                if (!numThreadsAttrib)
                {
                    numThreadsAttrib = parser->astBuilder->create<UncheckedAttribute>();
                    numThreadsAttrib->args.setCount(3);

                    // Just mark the loc and name from the first in the list
                    numThreadsAttrib->name = getName(parser, "numthreads");
                    numThreadsAttrib->loc = nameAndLoc.loc;
                    numThreadsAttrib->scope = parser->currentScope;
                }

                if (AdvanceIf(parser, TokenType::OpAssign))
                {
                    auto expr = parseAtomicExpr(parser);
                    //SLANG_ASSERT(expr);
                    if (!expr)
                    {
                        return nullptr;
                    }

                    numThreadsAttrib->args[localSizeIndex] = expr;
                }
            }
            else if (nameText == "binding" ||
                nameText == "set")
            {
                GLSLBindingAttribute* attr = listBuilder.find<GLSLBindingAttribute>();
                if (!attr)
                {
                    attr = parser->astBuilder->create<GLSLBindingAttribute>();
                    listBuilder.add(attr);
                }

                parser->ReadToken(TokenType::OpAssign);

                // If the token asked for is not returned found will put in recovering state, and return token found
                Token valToken = parser->ReadToken(TokenType::IntegerLiteral);
                // If wasn't the desired IntegerLiteral return that couldn't parse
                if (valToken.type == TokenType::IntegerLiteral)
                {
                    // Work out the value
                    auto value = getIntegerLiteralValue(valToken);

                    if (nameText == "binding")
                    {
                        attr->binding = int32_t(value);
                    }
                    else
                    {
                        attr->set = int32_t(value);
                    }
                }
            }
            else
            {
                Modifier* modifier = nullptr;

#define CASE(key, type) if (nameText == #key) { modifier = parser->astBuilder->create<type>(); } else
                CASE(push_constant, PushConstantAttribute) 
                CASE(shaderRecordNV, ShaderRecordAttribute)
                CASE(constant_id,   GLSLConstantIDLayoutModifier) 
                CASE(location, GLSLLocationLayoutModifier) 
                {
                    modifier = parser->astBuilder->create<GLSLUnparsedLayoutModifier>();
                }
                SLANG_ASSERT(modifier);
#undef CASE

                modifier->name = nameAndLoc.name;
                modifier->loc = nameAndLoc.loc;

                // Special handling for GLSLLayoutModifier
                if (auto glslModifier = as<GLSLLayoutModifier>(modifier))
                {
                    if (AdvanceIf(parser, TokenType::OpAssign))
                    {
                        glslModifier->valToken = parser->ReadToken(TokenType::IntegerLiteral);
                    }
                }

                listBuilder.add(modifier);
            }

            if (AdvanceIf(parser, TokenType::RParent))
                break;
            parser->ReadToken(TokenType::Comma);
        }

        if (numThreadsAttrib)
        {
            listBuilder.add(numThreadsAttrib);
        }

        listBuilder.add(parser->astBuilder->create<GLSLLayoutModifierGroupEnd>());

        return listBuilder.getFirst();
    }

    static NodeBase* parseBuiltinTypeModifier(Parser* parser, void* /*userData*/)
    {
        BuiltinTypeModifier* modifier = parser->astBuilder->create<BuiltinTypeModifier>();
        parser->ReadToken(TokenType::LParent);
        modifier->tag = BaseType(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).getContent()));
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static NodeBase* parseMagicTypeModifier(Parser* parser, void* /*userData*/)
    {
        MagicTypeModifier* modifier = parser->astBuilder->create<MagicTypeModifier>();
        parser->ReadToken(TokenType::LParent);
        modifier->name = parser->ReadToken(TokenType::Identifier).getContent();
        if (AdvanceIf(parser, TokenType::Comma))
        {
            modifier->tag = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).getContent()));
        }
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static NodeBase* parseIntrinsicTypeModifier(Parser* parser, void* /*userData*/)
    {
        IntrinsicTypeModifier* modifier = parser->astBuilder->create<IntrinsicTypeModifier>();
        parser->ReadToken(TokenType::LParent);
        modifier->irOp = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).getContent()));
        while( AdvanceIf(parser, TokenType::Comma) )
        {
            auto operand = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).getContent()));
            modifier->irOperands.add(operand);
        }
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }
    static NodeBase* parseImplicitConversionModifier(Parser* parser, void* /*userData*/)
    {
        ImplicitConversionModifier* modifier = parser->astBuilder->create<ImplicitConversionModifier>();

        ConversionCost cost = kConversionCost_Default;
        if( AdvanceIf(parser, TokenType::LParent) )
        {
            cost = ConversionCost(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).getContent()));
            parser->ReadToken(TokenType::RParent);
        }
        modifier->cost = cost;
        return modifier;
    }

    static NodeBase* parseAttributeTargetModifier(Parser* parser, void* /*userData*/)
    {
        expect(parser, TokenType::LParent);
        auto syntaxClassNameAndLoc = expectIdentifier(parser);
        expect(parser, TokenType::RParent);

        auto syntaxClass = parser->astBuilder->findSyntaxClass(syntaxClassNameAndLoc.name);

        AttributeTargetModifier* modifier = parser->astBuilder->create<AttributeTargetModifier>();
        modifier->syntaxClass = syntaxClass;

        return modifier;
    }

    ModuleDecl* populateBaseLanguageModule(
        ASTBuilder*     astBuilder,
        RefPtr<Scope>   scope)
    {
        Session* session = astBuilder->getGlobalSession();

        ModuleDecl* moduleDecl = astBuilder->create<ModuleDecl>();
        scope->containerDecl = moduleDecl;

        // Add syntax for declaration keywords
    #define DECL(KEYWORD, CALLBACK) \
        addBuiltinSyntax<Decl>(session, scope, #KEYWORD, &CALLBACK)
        DECL(typedef,         ParseTypeDef);
        DECL(associatedtype,  parseAssocType);
        DECL(type_param,    parseGlobalGenericTypeParamDecl);
        DECL(cbuffer,         parseHLSLCBufferDecl);
        DECL(tbuffer,         parseHLSLTBufferDecl);
        DECL(__generic,       ParseGenericDecl);
        DECL(__extension,     ParseExtensionDecl);
        DECL(extension,       ParseExtensionDecl);
        DECL(__init,          parseConstructorDecl);
        DECL(__subscript,     ParseSubscriptDecl);
        DECL(property,        ParsePropertyDecl);
        DECL(interface,       parseInterfaceDecl);
        DECL(syntax,          parseSyntaxDecl);
        DECL(attribute_syntax,parseAttributeSyntaxDecl);
        DECL(__import,        parseImportDecl);
        DECL(import,          parseImportDecl);
        DECL(let,             parseLetDecl);
        DECL(var,             parseVarDecl);
        DECL(func,            parseFuncDecl);
        DECL(typealias,       parseTypeAliasDecl);
        DECL(__generic_value_param, parseGlobalGenericValueParamDecl);
        DECL(namespace,       parseNamespaceDecl);

    #undef DECL

        // Add syntax for "simple" modifier keywords.
        // These are the ones that just appear as a single
        // keyword (no further tokens expected/allowed),
        // and which can be represented just by creating
        // a new AST node of the corresponding type.
    #define MODIFIER(KEYWORD, CLASS) \
        addSimpleModifierSyntax<CLASS>(session, scope, #KEYWORD)

        MODIFIER(in,        InModifier);
        MODIFIER(input,     InputModifier);
        MODIFIER(out,       OutModifier);
        MODIFIER(inout,     InOutModifier);
        MODIFIER(__ref,     RefModifier);
        MODIFIER(const,     ConstModifier);
        MODIFIER(instance,  InstanceModifier);
        MODIFIER(__builtin, BuiltinModifier);

        MODIFIER(inline,    InlineModifier);
        MODIFIER(public,    PublicModifier);
        MODIFIER(require,   RequireModifier);
        MODIFIER(param,     ParamModifier);
        MODIFIER(extern,    ExternModifier);

        MODIFIER(row_major,     HLSLRowMajorLayoutModifier);
        MODIFIER(column_major,  HLSLColumnMajorLayoutModifier);

        MODIFIER(nointerpolation,   HLSLNoInterpolationModifier);
        MODIFIER(noperspective,     HLSLNoPerspectiveModifier);
        MODIFIER(linear,            HLSLLinearModifier);
        MODIFIER(sample,            HLSLSampleModifier);
        MODIFIER(centroid,          HLSLCentroidModifier);
        MODIFIER(precise,           PreciseModifier);
        MODIFIER(shared,            HLSLEffectSharedModifier);
        MODIFIER(groupshared,       HLSLGroupSharedModifier);
        MODIFIER(static,            HLSLStaticModifier);
        MODIFIER(uniform,           HLSLUniformModifier);
        MODIFIER(volatile,          HLSLVolatileModifier);

        // Modifiers for geometry shader input
        MODIFIER(point,         HLSLPointModifier);
        MODIFIER(line,          HLSLLineModifier);
        MODIFIER(triangle,      HLSLTriangleModifier);
        MODIFIER(lineadj,       HLSLLineAdjModifier);
        MODIFIER(triangleadj,   HLSLTriangleAdjModifier);

        // Modifiers for unary operator declarations
        MODIFIER(__prefix,   PrefixModifier);
        MODIFIER(__postfix,  PostfixModifier);

        // Modifier to apply to `import` that should be re-exported
        MODIFIER(__exported,  ExportedModifier);

    #undef MODIFIER

        // Add syntax for more complex modifiers, which allow
        // or expect more tokens after the initial keyword.
    #define MODIFIER(KEYWORD, CALLBACK) \
        addBuiltinSyntax<Modifier>(session, scope, #KEYWORD, &CALLBACK)

        MODIFIER(layout,            parseLayoutModifier);

        MODIFIER(__intrinsic_op,        parseIntrinsicOpModifier);
        MODIFIER(__target_intrinsic,    parseTargetIntrinsicModifier);
        MODIFIER(__specialized_for_target,    parseSpecializedForTargetModifier);
        MODIFIER(__glsl_extension,  parseGLSLExtensionModifier);
        MODIFIER(__glsl_version,    parseGLSLVersionModifier);
        MODIFIER(__spirv_version,   parseSPIRVVersionModifier);
        MODIFIER(__cuda_sm_version, parseCUDASMVersionModifier);

        MODIFIER(__builtin_type,    parseBuiltinTypeModifier);
        MODIFIER(__magic_type,      parseMagicTypeModifier);
        MODIFIER(__intrinsic_type,    parseIntrinsicTypeModifier);
        MODIFIER(__implicit_conversion,     parseImplicitConversionModifier);

        MODIFIER(__attributeTarget, parseAttributeTargetModifier);


#undef MODIFIER

        // Add syntax for expression keywords
    #define EXPR(KEYWORD, CALLBACK) \
        addBuiltinSyntax<Expr>(session, scope, #KEYWORD, &CALLBACK)

        EXPR(this,  parseThisExpr);
        EXPR(This,  parseThisTypeExpr);
        EXPR(true,  parseTrueExpr);
        EXPR(false, parseFalseExpr);
        EXPR(__TaggedUnion, parseTaggedUnionType);

    #undef EXPR

        return moduleDecl;
    }

}
