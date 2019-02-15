#include "parser.h"

#include <assert.h>

#include "compiler.h"
#include "lookup.h"
#include "visitor.h"

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
        RefPtr<Modifier> getFirst() { return m_result; };
    protected:

        RefPtr<Modifier> m_result;
        RefPtr<Modifier>* m_next;
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

        NamePool* getNamePool() { return namePool; }
        SourceLanguage getSourceLanguage() { return sourceLanguage; }

        int anonymousCounter = 0;

        RefPtr<Scope> outerScope;
        RefPtr<Scope> currentScope;

        TokenReader tokenReader;
        DiagnosticSink * sink;
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
            node->loc = tokenReader.PeekLoc();
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
            containerDecl->ParentDecl = currentScope->containerDecl;
            PushScope(containerDecl);
        }

        void PopScope()
        {
            currentScope = currentScope->parent;
        }
        Parser(
            Session* session,
            TokenSpan const& _tokens,
            DiagnosticSink * sink,
            RefPtr<Scope> const& outerScope)
            : tokenReader(_tokens)
            , sink(sink)
            , outerScope(outerScope)
            , m_session(session)
        {}
        Parser(const Parser & other) = default;

        Session* m_session = nullptr;
        Session* getSession() { return m_session; }

        Token ReadToken();
        Token ReadToken(TokenType type);
        Token ReadToken(const char * string);
        bool LookAheadToken(TokenType type, int offset = 0);
        bool LookAheadToken(const char * string, int offset = 0);
        void                                        parseSourceFile(ModuleDecl* program);
        RefPtr<Decl>					ParseStruct();
        RefPtr<ClassDecl>					    ParseClass();
        RefPtr<Stmt>					ParseStatement();
        RefPtr<Stmt>			        parseBlockStatement();
        RefPtr<DeclStmt>			parseVarDeclrStatement(Modifiers modifiers);
        RefPtr<IfStmt>				parseIfStatement();
        RefPtr<ForStmt>				ParseForStatement();
        RefPtr<WhileStmt>			ParseWhileStatement();
        RefPtr<DoWhileStmt>			ParseDoWhileStatement();
        RefPtr<BreakStmt>			ParseBreakStatement();
        RefPtr<ContinueStmt>			ParseContinueStatement();
        RefPtr<ReturnStmt>			ParseReturnStatement();
        RefPtr<ExpressionStmt>		ParseExpressionStatement();
        RefPtr<Expr>				ParseExpression(Precedence level = Precedence::Comma);

        // Parse an expression that might be used in an initializer or argument context, so we should avoid operator-comma
        inline RefPtr<Expr>			ParseInitExpr() { return ParseExpression(Precedence::Assignment); }
        inline RefPtr<Expr>			ParseArgExpr()  { return ParseExpression(Precedence::Assignment); }

        RefPtr<Expr>				ParseLeafExpression();
        RefPtr<ParamDecl>					ParseParameter();
        RefPtr<Expr>				ParseType();
        TypeExp										ParseTypeExp();

        Parser & operator = (const Parser &) = delete;
    };

    // Forward Declarations

    static void ParseDeclBody(
        Parser*						parser,
        ContainerDecl*				containerDecl,
        TokenType	                closingToken);

    static RefPtr<Decl> parseEnumDecl(Parser* parser);

    // Parse the `{}`-delimeted body of an aggregate type declaration
    static void parseAggTypeDeclBody(
        Parser*             parser,
        AggTypeDeclBase*    decl);

    static RefPtr<Modifier> ParseOptSemantics(
        Parser* parser);

    static void ParseOptSemantics(
        Parser* parser,
        Decl*	decl);

    static RefPtr<DeclBase> ParseDecl(
        Parser*			parser,
        ContainerDecl*	containerDecl);

    static RefPtr<Decl> ParseSingleDecl(
        Parser*			parser,
        ContainerDecl*	containerDecl);

    //

    static void Unexpected(
        Parser*     parser)
    {
        // Don't emit "unexpected token" errors if we are in recovering mode
        if (!parser->isRecovering)
        {
            parser->sink->diagnose(parser->tokenReader.PeekLoc(), Diagnostics::unexpectedToken,
                parser->tokenReader.PeekTokenType());

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
            parser->sink->diagnose(parser->tokenReader.PeekLoc(), Diagnostics::unexpectedTokenExpectedTokenName,
                parser->tokenReader.PeekTokenType(),
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
            parser->sink->diagnose(parser->tokenReader.PeekLoc(), Diagnostics::unexpectedTokenExpectedTokenType,
                parser->tokenReader.PeekTokenType(),
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
        TokenType tokenType = reader->AdvanceToken().type;
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
            if (reader->IsAtEnd()) return TokenType::EndOfFile;
            if (reader->PeekTokenType() == tokenType)
            {
                reader->AdvanceToken();
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
        if (tokenReader.PeekTokenType() == TokenType::Identifier
                && tokenReader.PeekToken().Content == expected)
        {
            isRecovering = false;
            return tokenReader.AdvanceToken();
        }

        if (!isRecovering)
        {
            Unexpected(this, expected);
            return tokenReader.PeekToken();
        }
        else
        {
            // Try to find a place to recover
            for (;;)
            {
                // The token we expected?
                // Then exit recovery mode and pretend like all is well.
                if (tokenReader.PeekTokenType() == TokenType::Identifier
                    && tokenReader.PeekToken().Content == expected)
                {
                    isRecovering = false;
                    return tokenReader.AdvanceToken();
                }


                // Don't skip past any "closing" tokens.
                if (IsClosingToken(tokenReader.PeekTokenType()))
                {
                    return tokenReader.PeekToken();
                }

                // Skip balanced tokens and try again.
                SkipBalancedToken(&tokenReader);
            }
        }
    }

    Token Parser::ReadToken()
    {
        return tokenReader.AdvanceToken();
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

        TokenReader* tokenReader = &parser->tokenReader;
        for (;;)
        {
            TokenType peek = tokenReader->PeekTokenType();

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
                    tokenReader->AdvanceToken();
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

                return false;
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
        if (tokenReader.PeekTokenType() == expected)
        {
            isRecovering = false;
            return tokenReader.AdvanceToken();
        }

        if (!isRecovering)
        {
            Unexpected(this, expected);
            return tokenReader.PeekToken();
        }
        else
        {
            // Try to find a place to recover
            if (TryRecoverBefore(this, expected))
            {
                isRecovering = false;
                return tokenReader.AdvanceToken();
            }

            return tokenReader.PeekToken();
        }
    }

    bool Parser::LookAheadToken(const char * string, int offset)
    {
        TokenReader r = tokenReader;
        for (int ii = 0; ii < offset; ++ii)
            r.AdvanceToken();

        return r.PeekTokenType() == TokenType::Identifier
            && r.PeekToken().Content == string;
}

    bool Parser::LookAheadToken(TokenType type, int offset)
    {
        TokenReader r = tokenReader;
        for (int ii = 0; ii < offset; ++ii)
            r.AdvanceToken();

        return r.PeekTokenType() == type;
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
        if (parser->tokenReader.PeekTokenType() == TokenType::EndOfFile)
        {
            parser->ReadToken(tokenType);
            return true;
        }
        return false;
    }

    RefPtr<RefObject> ParseTypeDef(Parser* parser, void* /*userData*/)
    {
        RefPtr<TypeDefDecl> typeDefDecl = new TypeDefDecl();

        // TODO(tfoley): parse an actual declarator
        auto type = parser->ParseTypeExp();

        auto nameToken = parser->ReadToken(TokenType::Identifier);
        typeDefDecl->loc = nameToken.loc;

        typeDefDecl->nameAndLoc = NameLoc(nameToken);
        typeDefDecl->type = type;

        return typeDefDecl;
    }

    // Add a modifier to a list of modifiers being built
    static void AddModifier(RefPtr<Modifier>** ioModifierLink, RefPtr<Modifier> modifier)
    {
        RefPtr<Modifier>*& modifierLink = *ioModifierLink;

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
            RefPtr<Modifier> linkMod = *modifierLink;
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
        RefPtr<ModifiableSyntaxNode>    syntax,
        RefPtr<Modifier>                modifier)
    {
        auto modifierLink = &syntax->modifiers.first;
        AddModifier(&modifierLink, modifier);
    }

    // 
    // '::'? identifier ('::' identifier)* 
    static Token parseAttributeName(Parser* parser)
    {
        const SourceLoc scopedIdSourceLoc = parser->tokenReader.PeekLoc();

        // Strip initial :: if there is one
        const TokenType initialTokenType = parser->tokenReader.PeekTokenType();
        if (initialTokenType == TokenType::Scope)
        {
            parser->ReadToken(TokenType::Scope); 
        }

        const Token firstIdentifier = parser->ReadToken(TokenType::Identifier);
        if (initialTokenType != TokenType::Scope && parser->tokenReader.PeekTokenType() != TokenType::Scope)
        {
            return firstIdentifier;
        }

        // Build up scoped string
        StringBuilder scopedIdentifierBuilder;
        if (initialTokenType == TokenType::Scope)
        {
            scopedIdentifierBuilder.Append('_'); 
        }
        scopedIdentifierBuilder.Append(firstIdentifier.Content);

        while (parser->tokenReader.PeekTokenType() == TokenType::Scope)
        {
            parser->ReadToken(TokenType::Scope);
            scopedIdentifierBuilder.Append('_'); 
            
            const Token nextIdentifier(parser->ReadToken(TokenType::Identifier));
            scopedIdentifierBuilder.Append(nextIdentifier.Content);
        }

        // Make a 'token'
        SourceManager* sourceManager = parser->sink->sourceManager;
        const UnownedStringSlice scopedIdentifier(sourceManager->allocateStringSlice(scopedIdentifierBuilder.getUnownedSlice()));   
        Token token(TokenType::Identifier, scopedIdentifier, scopedIdSourceLoc);

        // Get the name pool
        auto namePool = parser->getNamePool();

        // Since it's an Identifier have to set the name.
        token.ptrValue = namePool->getName(token.Content);

        return token;
    }

    // Parse HLSL-style `[name(arg, ...)]` style "attribute" modifiers
    static void ParseSquareBracketAttributes(Parser* parser, RefPtr<Modifier>** ioModifierLink)
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

            RefPtr<UncheckedAttribute> modifier = new UncheckedAttribute();
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
                        modifier->args.Add(arg);
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
        return parser->tokenReader.PeekTokenType();
    }

    static Token advanceToken(Parser* parser)
    {
        return parser->ReadToken();
    }

    static Token peekToken(Parser* parser)
    {
        return parser->tokenReader.PeekToken();
    }

    static SyntaxDecl* tryLookUpSyntaxDecl(
        Parser* parser,
        Name*   name)
    {
        // Let's look up the name and see what we find.

        auto lookupResult = lookUp(
            parser->getSession(),
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
        RefPtr<T>*  outSyntax)
    {
        if (!syntaxDecl)
            return false;

        if (!syntaxDecl->syntaxClass.isSubClassOf<T>())
            return false;

        // Consume the token that specified the keyword
        auto keywordToken = advanceToken(parser);

        RefPtr<RefObject> parsedObject = syntaxDecl->parseCallback(parser, syntaxDecl->parseUserData);
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
        RefPtr<T>*  outSyntax)
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
        RefPtr<Modifier>* modifierLink = &modifiers.first;
        for (;;)
        {
            SourceLoc loc = parser->tokenReader.PeekLoc();

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

                    RefPtr<Modifier> parsedModifier;
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


    static RefPtr<RefObject> parseImportDecl(
        Parser* parser, void* /*userData*/)
    {
        parser->haveSeenAnyImportDecls = true;

        auto decl = new ImportDecl();
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
                    sb << parser->ReadToken(TokenType::Identifier).Content;
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
                    nameToken.Content = UnownedStringSlice::fromLiteral("?:"); 
                    break;
                }
                ;       // fall-thru
            default:
                parser->sink->diagnose(nameToken.loc, Diagnostics::invalidOperator, nameToken);
                break;
            }

            return NameLoc(
                getName(parser, nameToken.Content),
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
        RefPtr<Expr>	elementCountExpr;
    };

    // "Unwrapped" information about a declarator
    struct DeclaratorInfo
    {
        RefPtr<Expr>	    typeSpec;
        NameLoc             nameAndLoc;
        RefPtr<Modifier>	semantics;
        RefPtr<Expr>	    initializer;
    };

    // Add a member declaration to its container, and ensure that its
    // parent link is set up correctly.
    static void AddMember(RefPtr<ContainerDecl> container, RefPtr<Decl> member)
    {
        if (container)
        {
            member->ParentDecl = container.Ptr();
            container->Members.Add(member);

            container->memberDictionaryIsValid = false;
        }
    }

    static void AddMember(RefPtr<Scope> scope, RefPtr<Decl> member)
    {
        if (scope)
        {
            AddMember(scope->containerDecl, member);
        }
    }

    static RefPtr<Decl> ParseGenericParamDecl(
        Parser*             parser,
        RefPtr<GenericDecl> genericDecl)
    {
        // simple syntax to introduce a value parameter
        if (AdvanceIf(parser, "let"))
        {
            // default case is a type parameter
            auto paramDecl = new GenericValueParamDecl();
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
            RefPtr<GenericTypeParamDecl> paramDecl = new GenericTypeParamDecl();
            parser->FillPosition(paramDecl);
            paramDecl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));
            if (AdvanceIf(parser, TokenType::Colon))
            {
                // The user is apply a constraint to this type parameter...

                auto paramConstraint = new GenericTypeConstraintDecl();
                parser->FillPosition(paramConstraint);

                auto paramType = DeclRefType::Create(
                    parser->getSession(),
                    DeclRef<Decl>(paramDecl, nullptr));

                auto paramTypeExpr = new SharedTypeExpr();
                paramTypeExpr->loc = paramDecl->loc;
                paramTypeExpr->base.type = paramType;
                paramTypeExpr->type = QualType(getTypeType(paramType));

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
        decl->inner->ParentDecl = decl;

        // A generic decl hijacks the name of the declaration
        // it wraps, so that lookup can find it.
        if (decl->inner)
        {
            decl->nameAndLoc = decl->inner->nameAndLoc;
            decl->loc = decl->inner->loc;
        }
    }

    template<typename ParseFunc>
    static RefPtr<Decl> parseOptGenericDecl(
        Parser* parser, const ParseFunc& parseInner)
    {
        // TODO: may want more advanced disambiguation than this...
        if (parser->LookAheadToken(TokenType::OpLess))
        {
            RefPtr<GenericDecl> genericDecl = new GenericDecl();
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

    static RefPtr<RefObject> ParseGenericDecl(Parser* parser, void*)
    {
        RefPtr<GenericDecl> decl = new GenericDecl();
        parser->FillPosition(decl.Ptr());
        parser->PushScope(decl.Ptr());
        ParseGenericDeclImpl(parser, decl.Ptr(), [=](GenericDecl* genDecl) {return ParseSingleDecl(parser, genDecl); });
        parser->PopScope();
        return decl;
    }

    static void parseParameterList(
        Parser*                 parser,
        RefPtr<CallableDecl>    decl)
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
            expr->FunctionExpr->accept(this, nullptr);
            for (auto arg : expr->Arguments)
                arg->accept(this, nullptr);
        }
        void visitIndexExpr(IndexExpr * expr)
        {
            expr->BaseExpression->accept(this, nullptr);
            expr->IndexExpression->accept(this, nullptr);
        }
        void visitMemberExpr(MemberExpr * expr)
        {
            expr->BaseExpression->accept(this, nullptr);
            expr->scope = scope;
        }
        void visitStaticMemberExpr(StaticMemberExpr * expr)
        {
            expr->BaseExpression->accept(this, nullptr);
            expr->scope = scope;
        }
        void visitExpr(Expr* /*expr*/)
        {}
    };

        /// Parse an optional body statement for a declaration that can have a body.
    static RefPtr<Stmt> parseOptBody(Parser* parser)
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
    static RefPtr<Decl> parseTraditionalFuncDecl(
        Parser*                 parser,
        DeclaratorInfo const&   declaratorInfo)
    {
        RefPtr<FuncDecl> decl = new FuncDecl();
        parser->FillPosition(decl.Ptr());
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

            decl->ReturnType = TypeExp(declaratorInfo.typeSpec);

            parser->PushScope(decl);

            parseParameterList(parser, decl);
            ParseOptSemantics(parser, decl.Ptr());
            decl->Body = parseOptBody(parser);

            parser->PopScope();

            return decl;
        });
    }

    static RefPtr<VarDeclBase> CreateVarDeclForContext(
        ContainerDecl*  containerDecl )
    {
        if (as<CallableDecl>(containerDecl))
        {
            // Function parameters always use their dedicated syntax class.
            //
            return new ParamDecl();
        }
        else
        {
            // Globals, locals, and member variables all use the same syntax class.
            //
            return new VarDecl();
        }
    }

    // Add modifiers to the end of the modifier list for a declaration
    void AddModifiers(Decl* decl, RefPtr<Modifier> modifiers)
    {
        if (!modifiers)
            return;

        RefPtr<Modifier>* link = &decl->modifiers.first;
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
        RefPtr<VarDeclBase>		decl,
        DeclaratorInfo const&	declaratorInfo)
    {
        parser->FillPosition(decl.Ptr());

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

        AddModifiers(decl.Ptr(), declaratorInfo.semantics);

        decl->initExpr = declaratorInfo.initializer;
    }

    static RefPtr<Declarator> ParseDeclarator(Parser* parser);

    static RefPtr<Declarator> ParseDirectAbstractDeclarator(
        Parser* parser)
    {
        RefPtr<Declarator> declarator;
        switch( parser->tokenReader.PeekTokenType() )
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
                parser->ReadToken(TokenType::LParent);
                declarator = ParseDeclarator(parser);
                parser->ReadToken(TokenType::RParent);
            }
            break;

        default:
            // an empty declarator is allowed
            return nullptr;
        }

        // postifx additions
        for( ;;)
        {
            switch( parser->tokenReader.PeekTokenType() )
            {
            case TokenType::LBracket:
                {
                    auto arrayDeclarator = new ArrayDeclarator();
                    arrayDeclarator->openBracketLoc = parser->tokenReader.PeekLoc();
                    arrayDeclarator->flavor = Declarator::Flavor::Array;
                    arrayDeclarator->inner = declarator;

                    parser->ReadToken(TokenType::LBracket);
                    if( parser->tokenReader.PeekTokenType() != TokenType::RBracket )
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
    static RefPtr<Declarator> ParseDeclarator(
        Parser* parser)
    {
        if( parser->tokenReader.PeekTokenType() == TokenType::OpMul )
        {
            auto ptrDeclarator = new PointerDeclarator();
            ptrDeclarator->starLoc = parser->tokenReader.PeekLoc();
            ptrDeclarator->flavor = Declarator::Flavor::Pointer;

            parser->ReadToken(TokenType::OpMul);

            // TODO(tfoley): allow qualifiers like `const` here?

            ptrDeclarator->inner = ParseDeclarator(parser);
            return ptrDeclarator;
        }
        else
        {
            return ParseDirectAbstractDeclarator(parser);
        }
    }

    // A declarator plus optional semantics and initializer
    struct InitDeclarator
    {
        RefPtr<Declarator>				declarator;
        RefPtr<Modifier>				semantics;
        RefPtr<Expr>	initializer;
    };

    // Parse a declarator plus optional semantics
    static InitDeclarator ParseSemanticDeclarator(
        Parser* parser)
    {
        InitDeclarator result;
        result.declarator = ParseDeclarator(parser);
        result.semantics = ParseOptSemantics(parser);
        return result;
    }

    // Parse a declarator plus optional semantics and initializer
    static InitDeclarator ParseInitDeclarator(
        Parser* parser)
    {
        InitDeclarator result = ParseSemanticDeclarator(parser);
        if (AdvanceIf(parser, TokenType::OpAssign))
        {
            result.initializer = parser->ParseInitExpr();
        }
        return result;
    }

    static void UnwrapDeclarator(
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

                    auto arrayTypeExpr = new IndexExpr();
                    arrayTypeExpr->loc = arrayDeclarator->openBracketLoc;
                    arrayTypeExpr->BaseExpression = ioInfo->typeSpec;
                    arrayTypeExpr->IndexExpression = arrayDeclarator->elementCountExpr;
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
        InitDeclarator const&	initDeclarator,
        DeclaratorInfo*			ioInfo)
    {
        UnwrapDeclarator(initDeclarator.declarator, ioInfo);
        ioInfo->semantics = initDeclarator.semantics;
        ioInfo->initializer = initDeclarator.initializer;
    }

    // Either a single declaration, or a group of them
    struct DeclGroupBuilder
    {
        SourceLoc        startPosition;
        RefPtr<Decl>        decl;
        RefPtr<DeclGroup>   group;

        // Add a new declaration to the potential group
        void addDecl(
            RefPtr<Decl>    newDecl)
        {
            SLANG_ASSERT(newDecl);

            if( decl )
            {
                group = new DeclGroup();
                group->loc = startPosition;
                group->decls.Add(decl);
                decl = nullptr;
            }

            if( group )
            {
                group->decls.Add(newDecl);
            }
            else
            {
                decl = newDecl;
            }
        }

        RefPtr<DeclBase> getResult()
        {
            if(group) return group;
            return decl;
        }
    };

    // Pares an argument to an application of a generic
    RefPtr<Expr> ParseGenericArg(Parser* parser)
    {
        return parser->ParseArgExpr();
    }

    // Create a type expression that will refer to the given declaration
    static RefPtr<Expr>
    createDeclRefType(Parser* parser, RefPtr<Decl> decl)
    {
        // For now we just construct an expression that
        // will look up the given declaration by name.
        //
        // TODO: do this better, e.g. by filling in the `declRef` field directly

        auto expr = new VarExpr();
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
        RefPtr<Decl>                    decl;

        // Put the resulting expression (which should evaluate to a type) here
        RefPtr<Expr>    expr;
    };

    static RefPtr<Expr> parseGenericApp(
        Parser*                         parser,
        RefPtr<Expr>    base)
    {
        RefPtr<GenericAppExpr> genericApp = new GenericAppExpr();

        parser->FillPosition(genericApp.Ptr()); // set up scope for lookup
        genericApp->FunctionExpr = base;
        parser->ReadToken(TokenType::OpLess);
        parser->genericDepth++;
        // For now assume all generics have at least one argument
        genericApp->Arguments.Add(ParseGenericArg(parser));
        while (AdvanceIf(parser, TokenType::Comma))
        {
            genericApp->Arguments.Add(ParseGenericArg(parser));
        }
        parser->genericDepth--;

        if (parser->tokenReader.PeekToken().type == TokenType::OpRsh)
        {
            parser->tokenReader.PeekToken().type = TokenType::OpGreater;
            parser->tokenReader.PeekToken().loc.setRaw(parser->tokenReader.PeekToken().loc.getRaw() + 1);
        }
        else if (parser->LookAheadToken(TokenType::OpGreater))
            parser->ReadToken(TokenType::OpGreater);
        else
            parser->sink->diagnose(parser->tokenReader.PeekToken(), Diagnostics::tokenTypeExpected, "'>'");
        return genericApp;
    }

    static bool isGenericName(Parser* parser, Name* name)
    {
        auto lookupResult = lookUp(
            parser->getSession(),
            nullptr, // no semantics visitor available yet
            name,
            parser->currentScope);
        if (!lookupResult.isValid() || lookupResult.isOverloaded())
            return false;

        return lookupResult.item.declRef.is<GenericDecl>();
    }

    static RefPtr<Expr> tryParseGenericApp(
        Parser*                         parser,
        RefPtr<Expr>    base)
    {
        Name * baseName = nullptr;
        if (auto varExpr = as<VarExpr>(base))
            baseName = varExpr->name;
        // if base is a known generics, parse as generics
        if (baseName && isGenericName(parser, baseName))
            return parseGenericApp(parser, base);

        // otherwise, we speculate as generics, and fallback to comparison when parsing failed
        TokenSpan tokenSpan;
        tokenSpan.mBegin = parser->tokenReader.mCursor;
        tokenSpan.mEnd = parser->tokenReader.mEnd;
        DiagnosticSink newSink;
        newSink.sourceManager = parser->sink->sourceManager;
        Parser newParser(*parser);
        newParser.sink = &newSink;
        auto speculateParseRs = parseGenericApp(&newParser, base);
        if (newSink.errorCount == 0)
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
    static RefPtr<Expr> parseMemberType(Parser * parser, RefPtr<Expr> base)
    {
        RefPtr<MemberExpr> memberExpr = new MemberExpr();
        parser->ReadToken(TokenType::Dot);
        parser->FillPosition(memberExpr.Ptr());
        memberExpr->BaseExpression = base;
        memberExpr->name = expectIdentifier(parser).name;
        return memberExpr;
    }

    // Parse option `[]` braces after a type expression, that indicate an array type
    static RefPtr<Expr> parsePostfixTypeSuffix(
        Parser* parser,
        RefPtr<Expr> inTypeExpr)
    {
        auto typeExpr = inTypeExpr;
        while (parser->LookAheadToken(TokenType::LBracket))
        {
            RefPtr<IndexExpr> arrType = new IndexExpr();
            arrType->loc = typeExpr->loc;
            arrType->BaseExpression = typeExpr;
            parser->ReadToken(TokenType::LBracket);
            if (!parser->LookAheadToken(TokenType::RBracket))
            {
                arrType->IndexExpression = parser->ParseExpression();
            }
            parser->ReadToken(TokenType::RBracket);
            typeExpr = arrType;
        }
        return typeExpr;
    }

    static RefPtr<Expr> parseTaggedUnionType(Parser* parser)
    {
        RefPtr<TaggedUnionTypeExpr> taggedUnionType = new TaggedUnionTypeExpr();

        parser->ReadToken(TokenType::LParent);
        while(!AdvanceIfMatch(parser, TokenType::RParent))
        {
            auto caseType = parser->ParseTypeExp();
            taggedUnionType->caseTypes.Add(caseType);

            if(AdvanceIf(parser, TokenType::RParent))
                break;

            parser->ReadToken(TokenType::Comma);
        }

        return taggedUnionType;
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
        else if(AdvanceIf(parser, "__TaggedUnion"))
        {
            typeSpec.expr = parseTaggedUnionType(parser);
            return typeSpec;
        }

        Token typeName = parser->ReadToken(TokenType::Identifier);

        auto basicType = new VarExpr();
        basicType->scope = parser->currentScope.Ptr();
        basicType->loc = typeName.loc;
        basicType->name = typeName.getNameOrNull();

        RefPtr<Expr> typeExpr = basicType;

        bool shouldLoop = true;
        while (shouldLoop)
        {
            switch (peekTokenType(parser))
            {
            case TokenType::OpLess:
                typeExpr = parseGenericApp(parser, typeExpr);
                break;
            case TokenType::Dot:
                typeExpr = parseMemberType(parser, typeExpr);
                break;
            default:
                shouldLoop = false;
            }
        }

        // GLSL allows `[]` directly in a type specifier
        if (parser->getSourceLanguage() == SourceLanguage::GLSL)
        {
            typeExpr = parsePostfixTypeSuffix(parser, typeExpr);
        }

        typeSpec.expr = typeExpr;
        return typeSpec;
    }

    static RefPtr<DeclBase> ParseDeclaratorDecl(
        Parser*         parser,
        ContainerDecl*  containerDecl)
    {
        SourceLoc startPosition = parser->tokenReader.PeekLoc();

        auto typeSpec = parseTypeSpec(parser);

        // We may need to build up multiple declarations in a group,
        // but the common case will be when we have just a single
        // declaration
        DeclGroupBuilder declGroupBuilder;
        declGroupBuilder.startPosition = startPosition;

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


        InitDeclarator initDeclarator = ParseInitDeclarator(parser);

        DeclaratorInfo declaratorInfo;
        declaratorInfo.typeSpec = typeSpec.expr;


        // Rather than parse function declarators properly for now,
        // we'll just do a quick disambiguation here. This won't
        // matter unless we actually decide to support function-type parameters,
        // using C syntax.
        //
        if ((parser->tokenReader.PeekTokenType() == TokenType::LParent ||
            parser->tokenReader.PeekTokenType() == TokenType::OpLess)

            // Only parse as a function if we didn't already see mutually-exclusive
            // constructs when parsing the declarator.
            && !initDeclarator.initializer
            && !initDeclarator.semantics)
        {
            // Looks like a function, so parse it like one.
            UnwrapDeclarator(initDeclarator, &declaratorInfo);
            return parseTraditionalFuncDecl(parser, declaratorInfo);
        }

        // Otherwise we are looking at a variable declaration, which could be one in a sequence...

        if( AdvanceIf(parser, TokenType::Semicolon) )
        {
            // easy case: we only had a single declaration!
            UnwrapDeclarator(initDeclarator, &declaratorInfo);
            RefPtr<VarDeclBase> firstDecl = CreateVarDeclForContext(containerDecl);
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

        auto sharedTypeSpec = new SharedTypeExpr();
        sharedTypeSpec->loc = typeSpec.expr->loc;
        sharedTypeSpec->base = TypeExp(typeSpec.expr);

        for(;;)
        {
            declaratorInfo.typeSpec = sharedTypeSpec;
            UnwrapDeclarator(initDeclarator, &declaratorInfo);

            RefPtr<VarDeclBase> varDecl = CreateVarDeclForContext(containerDecl);
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
            initDeclarator = ParseInitDeclarator(parser);
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
    static RefPtr<Modifier> ParseSemantic(
        Parser* parser)
    {
        if (parser->LookAheadToken("register"))
        {
            RefPtr<HLSLRegisterSemantic> semantic = new HLSLRegisterSemantic();
            parser->FillPosition(semantic);
            parseHLSLRegisterSemantic(parser, semantic.Ptr());
            return semantic;
        }
        else if (parser->LookAheadToken("packoffset"))
        {
            RefPtr<HLSLPackOffsetSemantic> semantic = new HLSLPackOffsetSemantic();
            parser->FillPosition(semantic);
            parseHLSLPackOffsetSemantic(parser, semantic.Ptr());
            return semantic;
        }
        else if (parser->LookAheadToken(TokenType::Identifier))
        {
            RefPtr<HLSLSimpleSemantic> semantic = new HLSLSimpleSemantic();
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
    static RefPtr<Modifier> ParseOptSemantics(
        Parser* parser)
    {
        if (!AdvanceIf(parser, TokenType::Colon))
            return nullptr;

        RefPtr<Modifier> result;
        RefPtr<Modifier>* link = &result;
        SLANG_ASSERT(!*link);

        for (;;)
        {
            RefPtr<Modifier> semantic = ParseSemantic(parser);
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

    static RefPtr<Decl> ParseHLSLBufferDecl(
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

        auto bufferWrapperTypeNamePos = parser->tokenReader.PeekLoc();

        // We are going to represent each buffer as a pair of declarations.
        // The first is a type declaration that holds all the members, while
        // the second is a variable declaration that uses the buffer type.
        RefPtr<StructDecl> bufferDataTypeDecl = new StructDecl();
        RefPtr<VarDecl> bufferVarDecl = new VarDecl();

        // Both declarations will have a location that points to the name
        parser->FillPosition(bufferDataTypeDecl.Ptr());
        parser->FillPosition(bufferVarDecl.Ptr());

        auto reflectionNameToken = parser->ReadToken(TokenType::Identifier);

        // Attach the reflection name to the block so we can use it
        auto reflectionNameModifier = new ParameterGroupReflectionName();
        reflectionNameModifier->nameAndLoc = NameLoc(reflectionNameToken);
        addModifier(bufferVarDecl, reflectionNameModifier);

        // Both the buffer variable and its type need to have names generated
        bufferVarDecl->nameAndLoc.name = generateName(parser, "parameterGroup_" + String(reflectionNameToken.Content));
        bufferDataTypeDecl->nameAndLoc.name = generateName(parser, "ParameterGroup_" + String(reflectionNameToken.Content));

        addModifier(bufferDataTypeDecl, new ImplicitParameterGroupElementTypeModifier());
        addModifier(bufferVarDecl, new ImplicitParameterGroupVariableModifier());

        // TODO(tfoley): We end up constructing unchecked syntax here that
        // is expected to type check into the right form, but it might be
        // cleaner to have a more explicit desugaring pass where we parse
        // these constructs directly into the AST and *then* desugar them.

        // Construct a type expression to reference the buffer data type
        auto bufferDataTypeExpr = new VarExpr();
        bufferDataTypeExpr->loc = bufferDataTypeDecl->loc;
        bufferDataTypeExpr->name = bufferDataTypeDecl->nameAndLoc.name;
        bufferDataTypeExpr->scope = parser->currentScope.Ptr();

        // Construct a type exrpession to reference the type constructor
        auto bufferWrapperTypeExpr = new VarExpr();
        bufferWrapperTypeExpr->loc = bufferWrapperTypeNamePos;
        bufferWrapperTypeExpr->name = getName(parser, bufferWrapperTypeName);

        // Always need to look this up in the outer scope,
        // so that it won't collide with, e.g., a local variable called `ConstantBuffer`
        bufferWrapperTypeExpr->scope = parser->outerScope;

        // Construct a type expression that represents the type for the variable,
        // which is the wrapper type applied to the data type
        auto bufferVarTypeExpr = new GenericAppExpr();
        bufferVarTypeExpr->loc = bufferVarDecl->loc;
        bufferVarTypeExpr->FunctionExpr = bufferWrapperTypeExpr;
        bufferVarTypeExpr->Arguments.Add(bufferDataTypeExpr);

        bufferVarDecl->type.exp = bufferVarTypeExpr;

        // Any semantics applied to the bufer declaration are taken as applying
        // to the variable instead.
        ParseOptSemantics(parser, bufferVarDecl.Ptr());

        // The declarations in the body belong to the data type.
        parseAggTypeDeclBody(parser, bufferDataTypeDecl.Ptr());

        // All HLSL buffer declarations are "transparent" in that their
        // members are implicitly made visible in the parent scope.
        // We achieve this by applying the transparent modifier to the variable.
        auto transparentModifier = new TransparentModifier();
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

    static RefPtr<RefObject> parseHLSLCBufferDecl(
        Parser*	parser, void* /*userData*/)
    {
        return ParseHLSLBufferDecl(parser, "ConstantBuffer");
    }

    static RefPtr<RefObject> parseHLSLTBufferDecl(
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

                auto inheritanceDecl = new InheritanceDecl();
                inheritanceDecl->loc = base.exp->loc;
                inheritanceDecl->nameAndLoc.name = getName(parser, "$inheritance");
                inheritanceDecl->base = base;

                AddMember(decl, inheritanceDecl);

            } while (AdvanceIf(parser, TokenType::Comma));
        }
    }

    static RefPtr<RefObject> ParseExtensionDecl(Parser* parser, void* /*userData*/)
    {
        RefPtr<ExtensionDecl> decl = new ExtensionDecl();
        parser->FillPosition(decl.Ptr());
        decl->targetType = parser->ParseTypeExp();
        parseOptionalInheritanceClause(parser, decl);
        parseAggTypeDeclBody(parser, decl.Ptr());

        return decl;
    }


    void parseOptionalGenericConstraints(Parser * parser, ContainerDecl* decl)
    {
        if (AdvanceIf(parser, TokenType::Colon))
        {
            do
            {
                RefPtr<GenericTypeConstraintDecl> paramConstraint = new GenericTypeConstraintDecl();
                parser->FillPosition(paramConstraint);

                // substitution needs to be filled during check
                RefPtr<DeclRefType> paramType = DeclRefType::Create(
                    parser->getSession(),
                    DeclRef<Decl>(decl, nullptr));

                RefPtr<SharedTypeExpr> paramTypeExpr = new SharedTypeExpr();
                paramTypeExpr->loc = decl->loc;
                paramTypeExpr->base.type = paramType;
                paramTypeExpr->type = QualType(getTypeType(paramType));

                paramConstraint->sub = TypeExp(paramTypeExpr);
                paramConstraint->sup = parser->ParseTypeExp();

                AddMember(decl, paramConstraint);
            } while (AdvanceIf(parser, TokenType::Comma));
        }
    }

    RefPtr<RefObject> parseAssocType(Parser * parser, void *)
    {
        RefPtr<AssocTypeDecl> assocTypeDecl = new AssocTypeDecl();

        auto nameToken = parser->ReadToken(TokenType::Identifier);
        assocTypeDecl->nameAndLoc = NameLoc(nameToken);
        assocTypeDecl->loc = nameToken.loc;
        parseOptionalGenericConstraints(parser, assocTypeDecl);
        parser->ReadToken(TokenType::Semicolon);
        return assocTypeDecl;
    }

    RefPtr<RefObject> parseGlobalGenericParamDecl(Parser * parser, void *)
    {
        RefPtr<GlobalGenericParamDecl> genParamDecl = new GlobalGenericParamDecl();
        auto nameToken = parser->ReadToken(TokenType::Identifier);
        genParamDecl->nameAndLoc = NameLoc(nameToken);
        genParamDecl->loc = nameToken.loc;
        parseOptionalGenericConstraints(parser, genParamDecl);
        parser->ReadToken(TokenType::Semicolon);
        return genParamDecl;
    }

    static RefPtr<RefObject> parseInterfaceDecl(Parser* parser, void* /*userData*/)
    {
        RefPtr<InterfaceDecl> decl = new InterfaceDecl();
        parser->FillPosition(decl.Ptr());
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        parseOptionalInheritanceClause(parser, decl.Ptr());

        parseAggTypeDeclBody(parser, decl.Ptr());

        return decl;
    }

    static RefPtr<RefObject> parseConstructorDecl(Parser* parser, void* /*userData*/)
    {
        RefPtr<ConstructorDecl> decl = new ConstructorDecl();
        parser->FillPosition(decl.Ptr());

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

        decl->Body = parseOptBody(parser);

        return decl;
    }

    static RefPtr<AccessorDecl> parseAccessorDecl(Parser* parser)
    {
        Modifiers modifiers = ParseModifiers(parser);

        RefPtr<AccessorDecl> decl;
        if( AdvanceIf(parser, "get") )
        {
            decl = new GetterDecl();
        }
        else if( AdvanceIf(parser, "set") )
        {
            decl = new SetterDecl();
        }
        else if( AdvanceIf(parser, "ref") )
        {
            decl = new RefAccessorDecl();
        }
        else
        {
            Unexpected(parser);
            return nullptr;
        }

        AddModifiers(decl, modifiers.first);

        if( parser->tokenReader.PeekTokenType() == TokenType::LBrace )
        {
            decl->Body = parser->parseBlockStatement();
        }
        else
        {
            parser->ReadToken(TokenType::Semicolon);
        }

        return decl;
    }

    static RefPtr<RefObject> ParseSubscriptDecl(Parser* parser, void* /*userData*/)
    {
        RefPtr<SubscriptDecl> decl = new SubscriptDecl();
        parser->FillPosition(decl.Ptr());

        // TODO: the use of this name here is a bit magical...
        decl->nameAndLoc.name = getName(parser, "operator[]");

        parseParameterList(parser, decl);

        if( AdvanceIf(parser, TokenType::RightArrow) )
        {
            decl->ReturnType = parser->ParseTypeExp();
        }

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

        return decl;
    }

    static bool expect(Parser* parser, TokenType tokenType)
    {
        return parser->ReadToken(tokenType).type == tokenType;
    }

    static void parseModernVarDeclBaseCommon(
        Parser*             parser,
        RefPtr<VarDeclBase> decl)
    {
        parser->FillPosition(decl.Ptr());
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
        RefPtr<VarDecl> decl)
    {
        parseModernVarDeclBaseCommon(parser, decl);
        expect(parser, TokenType::Semicolon);
    }

    static RefPtr<RefObject> parseLetDecl(
        Parser* parser, void* /*userData*/)
    {
        RefPtr<LetDecl> decl = new LetDecl();
        parseModernVarDeclCommon(parser, decl);
        return decl;
    }

    static RefPtr<RefObject> parseVarDecl(
        Parser* parser, void* /*userData*/)
    {
        RefPtr<VarDecl> decl = new VarDecl();
        parseModernVarDeclCommon(parser, decl);
        return decl;
    }

    static RefPtr<ParamDecl> parseModernParamDecl(
        Parser* parser)
    {
        RefPtr<ParamDecl> decl = new ParamDecl();

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
        RefPtr<CallableDecl>    decl)
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

    static RefPtr<RefObject> parseFuncDecl(
        Parser* parser, void* /*userData*/)
    {
        RefPtr<FuncDecl> decl = new FuncDecl();

        parser->FillPosition(decl.Ptr());
        decl->nameAndLoc = NameLoc(parser->ReadToken(TokenType::Identifier));

        return parseOptGenericDecl(parser, [&](GenericDecl*)
        {
            parser->PushScope(decl.Ptr());
            parseModernParamList(parser, decl);
            if(AdvanceIf(parser, TokenType::RightArrow))
            {
                decl->ReturnType = parser->ParseTypeExp();
            }
            decl->Body = parseOptBody(parser);
            parser->PopScope();
            return decl;
        });
    }

    static RefPtr<RefObject> parseTypeAliasDecl(
        Parser* parser, void* /*userData*/)
    {
        RefPtr<TypeAliasDecl> decl = new TypeAliasDecl();

        parser->FillPosition(decl.Ptr());
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
    static RefPtr<RefObject> parseSimpleSyntax(Parser* /*parser*/, void* userData)
    {
        SyntaxClassBase syntaxClass((SyntaxClassBase::ClassInfo*) userData);
        return (RefObject*) syntaxClass.createInstanceImpl();
    }

    // Parse a declaration of a keyword that can be used to define further syntax.
    static RefPtr<RefObject> parseSyntaxDecl(Parser* parser, void* /*userData*/)
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
        SyntaxClass<RefObject> syntaxClass;
        if (AdvanceIf(parser, TokenType::Colon))
        {
            // User is specifying the class that should be construted
            auto classNameAndLoc = expectIdentifier(parser);

            syntaxClass = parser->getSession()->findSyntaxClass(classNameAndLoc.name);
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

        RefPtr<SyntaxDecl> syntaxDecl = new SyntaxDecl();
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
    static RefPtr<ParamDecl> parseAttributeParamDecl(Parser* parser)
    {
        auto nameAndLoc = expectIdentifier(parser);

        RefPtr<ParamDecl> paramDecl = new ParamDecl();
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
    static RefPtr<RefObject> parseAttributeSyntaxDecl(Parser* parser, void* /*userData*/)
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

        RefPtr<AttributeDecl> attrDecl = new AttributeDecl();
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
        SyntaxClass<RefObject> syntaxClass;
        if (AdvanceIf(parser, TokenType::Colon))
        {
            // User is specifying the class that should be construted
            auto classNameAndLoc = expectIdentifier(parser);

            syntaxClass = parser->getSession()->findSyntaxClass(classNameAndLoc.name);
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
        RefPtr<Decl>		decl,
        ContainerDecl*		containerDecl,
        Modifiers			modifiers)
    {
        // Add any modifiers we parsed before the declaration to the list
        // of modifiers on the declaration itself.
        //
        // We need to be careful, because if `decl` is a generic declaration,
        // then we really want the modifiers to apply to the inner declaration.
        //
        RefPtr<Decl> declToModify = decl;
        if(auto genericDecl = as<GenericDecl>(decl))
            declToModify = genericDecl->inner;
        AddModifiers(declToModify.Ptr(), modifiers.first);

        // Make sure the decl is properly nested inside its lexical parent
        if (containerDecl)
        {
            AddMember(containerDecl, decl);
        }
    }

    static RefPtr<DeclBase> ParseDeclWithModifiers(
        Parser*             parser,
        ContainerDecl*      containerDecl,
        Modifiers			modifiers )
    {
        RefPtr<DeclBase> decl;

        auto loc = parser->tokenReader.PeekLoc();

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
                RefPtr<Decl> parsedDecl;
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

                decl = new EmptyDecl();
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

                auto sharedModifiers = new SharedModifiers();
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

    static RefPtr<DeclBase> ParseDecl(
        Parser*         parser,
        ContainerDecl*  containerDecl)
    {
        Modifiers modifiers = ParseModifiers(parser);
        return ParseDeclWithModifiers(parser, containerDecl, modifiers);
    }

    static RefPtr<Decl> ParseSingleDecl(
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
            if( declGroup->decls.Count() == 1 )
            {
                return declGroup->decls[0];
            }
        }

        parser->sink->diagnose(declBase->loc, Diagnostics::unimplemented, "didn't expect multiple declarations here");
        return nullptr;
    }


    // Parse a body consisting of declarations
    static void ParseDeclBody(
        Parser*         parser,
        ContainerDecl*  containerDecl,
        TokenType       closingToken)
    {
        while(!AdvanceIfMatch(parser, closingToken))
        {
            ParseDecl(parser, containerDecl);
        }
    }

    // Parse the `{}`-delimeted body of an aggregate type declaration
    static void parseAggTypeDeclBody(
        Parser*             parser,
        AggTypeDeclBase*    decl)
    {
        // TODO: the scope used for the body might need to be
        // slightly specialized to deal with the complexity
        // of how `this` works.
        //
        // Alternatively, that complexity can be pushed down
        // to semantic analysis so that it doesn't clutter
        // things here.
        parser->PushScope(decl);

        parser->ReadToken(TokenType::LBrace);
        ParseDeclBody(parser, decl, TokenType::RBrace);

        parser->PopScope();
    }


    void Parser::parseSourceFile(ModuleDecl* program)
    {
        if (outerScope)
        {
            currentScope = outerScope;
        }

        PushScope(program);
        program->loc = tokenReader.PeekLoc();
        program->scope = currentScope;
        ParseDeclBody(this, program, TokenType::EndOfFile);
        PopScope();

        SLANG_RELEASE_ASSERT(currentScope == outerScope);
        currentScope = nullptr;
    }

    RefPtr<Decl> Parser::ParseStruct()
    {
        RefPtr<StructDecl> rs = new StructDecl();
        FillPosition(rs.Ptr());
        ReadToken("struct");

        // TODO: support `struct` declaration without tag
        rs->nameAndLoc = expectIdentifier(this);

        return parseOptGenericDecl(this, [&](GenericDecl*)
        {
            // We allow for an inheritance clause on a `struct`
            // so that it can conform to interfaces.
            parseOptionalInheritanceClause(this, rs.Ptr());
            parseAggTypeDeclBody(this, rs.Ptr());
            return rs;
        });
    }

    RefPtr<ClassDecl> Parser::ParseClass()
    {
        RefPtr<ClassDecl> rs = new ClassDecl();
        FillPosition(rs.Ptr());
        ReadToken("class");
        rs->nameAndLoc = expectIdentifier(this);
        ReadToken(TokenType::LBrace);
        parseOptionalInheritanceClause(this, rs.Ptr());
        parseAggTypeDeclBody(this, rs.Ptr());
        return rs;
    }

    static RefPtr<EnumCaseDecl> parseEnumCaseDecl(Parser* parser)
    {
        RefPtr<EnumCaseDecl> decl = new EnumCaseDecl();
        decl->nameAndLoc = expectIdentifier(parser);

        if(AdvanceIf(parser, TokenType::OpAssign))
        {
            decl->tagExpr = parser->ParseArgExpr();
        }

        return decl;
    }

    static RefPtr<Decl> parseEnumDecl(Parser* parser)
    {
        RefPtr<EnumDecl> decl = new EnumDecl();
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
                RefPtr<EnumCaseDecl> caseDecl = parseEnumCaseDecl(parser);
                AddMember(decl, caseDecl);

                if(AdvanceIf(parser, TokenType::RBrace))
                    break;

                parser->ReadToken(TokenType::Comma);
            }
            return decl;
        });
    }

    static RefPtr<Stmt> ParseSwitchStmt(Parser* parser)
    {
        RefPtr<SwitchStmt> stmt = new SwitchStmt();
        parser->FillPosition(stmt.Ptr());
        parser->ReadToken("switch");
        parser->ReadToken(TokenType::LParent);
        stmt->condition = parser->ParseExpression();
        parser->ReadToken(TokenType::RParent);
        stmt->body = parser->parseBlockStatement();
        return stmt;
    }

    static RefPtr<Stmt> ParseCaseStmt(Parser* parser)
    {
        RefPtr<CaseStmt> stmt = new CaseStmt();
        parser->FillPosition(stmt.Ptr());
        parser->ReadToken("case");
        stmt->expr = parser->ParseExpression();
        parser->ReadToken(TokenType::Colon);
        return stmt;
    }

    static RefPtr<Stmt> ParseDefaultStmt(Parser* parser)
    {
        RefPtr<DefaultStmt> stmt = new DefaultStmt();
        parser->FillPosition(stmt.Ptr());
        parser->ReadToken("default");
        parser->ReadToken(TokenType::Colon);
        return stmt;
    }

    static bool isTypeName(Parser* parser, Name* name)
    {
        auto lookupResult = lookUp(
            parser->getSession(),
            nullptr, // no semantics visitor available yet
            name,
            parser->currentScope);
        if(!lookupResult.isValid() || lookupResult.isOverloaded())
            return false;

        auto decl = lookupResult.item.declRef.getDecl();
        if( auto typeDecl = as<AggTypeDecl>(decl) )
        {
            return true;
        }
        else if( auto typeVarDecl = as<SimpleTypeDecl>(decl) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    static bool peekTypeName(Parser* parser)
    {
        if(!parser->LookAheadToken(TokenType::Identifier))
            return false;

        auto name = parser->tokenReader.PeekToken().getName();
        return isTypeName(parser, name);
    }

    RefPtr<Stmt> parseCompileTimeForStmt(
        Parser* parser)
    {
        RefPtr<ScopeDecl> scopeDecl = new ScopeDecl();
        RefPtr<CompileTimeForStmt> stmt = new CompileTimeForStmt();
        stmt->scopeDecl = scopeDecl;


        parser->ReadToken("for");
        parser->ReadToken(TokenType::LParent);

        NameLoc varNameAndLoc = expectIdentifier(parser);
        RefPtr<VarDecl> varDecl = new VarDecl();
        varDecl->nameAndLoc = varNameAndLoc;
        varDecl->loc = varNameAndLoc.loc;

        stmt->varDecl = varDecl;

        parser->ReadToken("in");
        parser->ReadToken("Range");
        parser->ReadToken(TokenType::LParent);

        RefPtr<Expr> rangeBeginExpr;
        RefPtr<Expr> rangeEndExpr = parser->ParseArgExpr();
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

    RefPtr<Stmt> parseCompileTimeStmt(
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

    RefPtr<Stmt> Parser::ParseStatement()
    {
        auto modifiers = ParseModifiers(this);

        RefPtr<Stmt> statement;
        if (LookAheadToken(TokenType::LBrace))
            statement = parseBlockStatement();
        else if (peekTypeName(this))
            statement = parseVarDeclrStatement(modifiers);
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
            statement = new DiscardStmt();
            FillPosition(statement.Ptr());
            ReadToken("discard");
            ReadToken(TokenType::Semicolon);
        }
        else if (LookAheadToken("switch"))
            statement = ParseSwitchStmt(this);
        else if (LookAheadToken("case"))
            statement = ParseCaseStmt(this);
        else if (LookAheadToken("default"))
            statement = ParseDefaultStmt(this);
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

            TokenReader::ParsingCursor startPos = tokenReader.getCursor();

            // Try to parse a type (knowing that the type grammar is
            // a subset of the expression grammar, and so this should
            // always succeed).
            RefPtr<Expr> type = ParseType();
            // We don't actually care about the type, though, so
            // don't retain it
            type = nullptr;

            // If the next token after we parsed a type looks like
            // we are going to declare a variable, then lets guess
            // that this is a declaration.
            //
            // TODO(tfoley): this wouldn't be robust for more
            // general kinds of declarators (notably pointer declarators),
            // so we'll need to be careful about this.
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
            statement = new EmptyStmt();
            FillPosition(statement.Ptr());
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

    RefPtr<Stmt> Parser::parseBlockStatement()
    {
        RefPtr<ScopeDecl> scopeDecl = new ScopeDecl();
        RefPtr<BlockStmt> blockStatement = new BlockStmt();
        blockStatement->scopeDecl = scopeDecl;
        pushScopeAndSetParent(scopeDecl.Ptr());
        ReadToken(TokenType::LBrace);

        RefPtr<Stmt> body;

        if(!tokenReader.IsAtEnd())
        {
            FillPosition(blockStatement.Ptr());
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
                    seqStmt->stmts.Add(stmt);
                }
                else
                {
                    RefPtr<SeqStmt> newBody = new SeqStmt();
                    newBody->loc = blockStatement->loc;
                    newBody->stmts.Add(body);
                    newBody->stmts.Add(stmt);

                    body = newBody;
                }
            }
            TryRecover(this);
        }
        PopScope();

        if(!body)
        {
            body = new EmptyStmt();
            body->loc = blockStatement->loc;
        }

        blockStatement->body = body;
        return blockStatement;
    }

    RefPtr<DeclStmt> Parser::parseVarDeclrStatement(
        Modifiers modifiers)
    {
        RefPtr<DeclStmt>varDeclrStatement = new DeclStmt();

        FillPosition(varDeclrStatement.Ptr());
        auto decl = ParseDeclWithModifiers(this, currentScope->containerDecl, modifiers);
        varDeclrStatement->decl = decl;
        return varDeclrStatement;
    }

    RefPtr<IfStmt> Parser::parseIfStatement()
    {
        RefPtr<IfStmt> ifStatement = new IfStmt();
        FillPosition(ifStatement.Ptr());
        ReadToken("if");
        ReadToken(TokenType::LParent);
        ifStatement->Predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        ifStatement->PositiveStatement = ParseStatement();
        if (LookAheadToken("else"))
        {
            ReadToken("else");
            ifStatement->NegativeStatement = ParseStatement();
        }
        return ifStatement;
    }

    RefPtr<ForStmt> Parser::ParseForStatement()
    {
        RefPtr<ScopeDecl> scopeDecl = new ScopeDecl();

        // HLSL implements the bad approach to scoping a `for` loop
        // variable, and we want to respect that, but *only* when
        // parsing HLSL code.
        //

        bool brokenScoping = getSourceLanguage() == SourceLanguage::HLSL;

        // We will create a distinct syntax node class for the unscoped
        // case, just so that we can correctly handle it in downstream
        // logic.
        //
        RefPtr<ForStmt> stmt;
        if (brokenScoping)
        {
            stmt = new UnscopedForStmt();
        }
        else
        {
            stmt = new ForStmt();
        }

        stmt->scopeDecl = scopeDecl;

        if(!brokenScoping)
            pushScopeAndSetParent(scopeDecl.Ptr());
        FillPosition(stmt.Ptr());
        ReadToken("for");
        ReadToken(TokenType::LParent);
        if (peekTypeName(this))
        {
            stmt->InitialStatement = parseVarDeclrStatement(Modifiers());
        }
        else
        {
            if (!LookAheadToken(TokenType::Semicolon))
            {
                stmt->InitialStatement = ParseExpressionStatement();
            }
            else
            {
                ReadToken(TokenType::Semicolon);
            }
        }
        if (!LookAheadToken(TokenType::Semicolon))
            stmt->PredicateExpression = ParseExpression();
        ReadToken(TokenType::Semicolon);
        if (!LookAheadToken(TokenType::RParent))
            stmt->SideEffectExpression = ParseExpression();
        ReadToken(TokenType::RParent);
        stmt->Statement = ParseStatement();

        if (!brokenScoping)
            PopScope();

        return stmt;
    }

    RefPtr<WhileStmt> Parser::ParseWhileStatement()
    {
        RefPtr<WhileStmt> whileStatement = new WhileStmt();
        FillPosition(whileStatement.Ptr());
        ReadToken("while");
        ReadToken(TokenType::LParent);
        whileStatement->Predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        whileStatement->Statement = ParseStatement();
        return whileStatement;
    }

    RefPtr<DoWhileStmt> Parser::ParseDoWhileStatement()
    {
        RefPtr<DoWhileStmt> doWhileStatement = new DoWhileStmt();
        FillPosition(doWhileStatement.Ptr());
        ReadToken("do");
        doWhileStatement->Statement = ParseStatement();
        ReadToken("while");
        ReadToken(TokenType::LParent);
        doWhileStatement->Predicate = ParseExpression();
        ReadToken(TokenType::RParent);
        ReadToken(TokenType::Semicolon);
        return doWhileStatement;
    }

    RefPtr<BreakStmt> Parser::ParseBreakStatement()
    {
        RefPtr<BreakStmt> breakStatement = new BreakStmt();
        FillPosition(breakStatement.Ptr());
        ReadToken("break");
        ReadToken(TokenType::Semicolon);
        return breakStatement;
    }

    RefPtr<ContinueStmt> Parser::ParseContinueStatement()
    {
        RefPtr<ContinueStmt> continueStatement = new ContinueStmt();
        FillPosition(continueStatement.Ptr());
        ReadToken("continue");
        ReadToken(TokenType::Semicolon);
        return continueStatement;
    }

    RefPtr<ReturnStmt> Parser::ParseReturnStatement()
    {
        RefPtr<ReturnStmt> returnStatement = new ReturnStmt();
        FillPosition(returnStatement.Ptr());
        ReadToken("return");
        if (!LookAheadToken(TokenType::Semicolon))
            returnStatement->Expression = ParseExpression();
        ReadToken(TokenType::Semicolon);
        return returnStatement;
    }

    RefPtr<ExpressionStmt> Parser::ParseExpressionStatement()
    {
        RefPtr<ExpressionStmt> statement = new ExpressionStmt();

        FillPosition(statement.Ptr());
        statement->Expression = ParseExpression();

        ReadToken(TokenType::Semicolon);
        return statement;
    }

    RefPtr<ParamDecl> Parser::ParseParameter()
    {
        RefPtr<ParamDecl> parameter = new ParamDecl();
        parameter->modifiers = ParseModifiers(this);

        DeclaratorInfo declaratorInfo;
        declaratorInfo.typeSpec = ParseType();

        InitDeclarator initDeclarator = ParseInitDeclarator(this);
        UnwrapDeclarator(initDeclarator, &declaratorInfo);

        // Assume it is a variable-like declarator
        CompleteVarDecl(this, parameter, declaratorInfo);
        return parameter;
    }

    RefPtr<Expr> Parser::ParseType()
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

    static RefPtr<Expr> parseOperator(Parser* parser)
    {
        Token opToken;
        switch(parser->tokenReader.PeekTokenType())
        {
        case TokenType::QuestionMark:
            opToken = parser->ReadToken();
            opToken.Content = UnownedStringSlice::fromLiteral("?:");
            break;

        default:
            opToken = parser->ReadToken();
            break;
        }

        auto opExpr = new VarExpr();
        opExpr->name = getName(parser, opToken.Content);
        opExpr->scope = parser->currentScope;
        opExpr->loc = opToken.loc;

        return opExpr;

    }

    static RefPtr<Expr> createInfixExpr(
        Parser*                         /*parser*/,
        RefPtr<Expr>    left,
        RefPtr<Expr>    op,
        RefPtr<Expr>    right)
    {
        RefPtr<InfixExpr> expr = new InfixExpr();
        expr->loc = op->loc;
        expr->FunctionExpr = op;
        expr->Arguments.Add(left);
        expr->Arguments.Add(right);
        return expr;
    }

    static RefPtr<Expr> parseInfixExprWithPrecedence(
        Parser*                         parser,
        RefPtr<Expr>    inExpr,
        Precedence                      prec)
    {
        auto expr = inExpr;
        for(;;)
        {
            auto opTokenType = parser->tokenReader.PeekTokenType();
            auto opPrec = GetOpLevel(parser, opTokenType);
            if(opPrec < prec)
                break;

            auto op = parseOperator(parser);

            // Special case the `?:` operator since it is the
            // one non-binary case we need to deal with.
            if(opTokenType == TokenType::QuestionMark)
            {
                RefPtr<SelectExpr> select = new SelectExpr();
                select->loc = op->loc;
                select->FunctionExpr = op;

                select->Arguments.Add(expr);

                select->Arguments.Add(parser->ParseExpression(opPrec));
                parser->ReadToken(TokenType::Colon);
                select->Arguments.Add(parser->ParseExpression(opPrec));

                expr = select;
                continue;
            }

            auto right = parser->ParseLeafExpression();

            for(;;)
            {
                auto nextOpPrec = GetOpLevel(parser, parser->tokenReader.PeekTokenType());

                if((GetAssociativityFromLevel(nextOpPrec) == Associativity::Right) ? (nextOpPrec < opPrec) : (nextOpPrec <= opPrec))
                    break;

                right = parseInfixExprWithPrecedence(parser, right, nextOpPrec);
            }

            if (opTokenType == TokenType::OpAssign)
            {
                RefPtr<AssignExpr> assignExpr = new AssignExpr();
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

    RefPtr<Expr> Parser::ParseExpression(Precedence level)
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
                RefPtr<SelectExpr> select = new SelectExpr();
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
                    RefPtr<OperatorExpr> tmp = new InfixExpr();
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
                    RefPtr<OperatorExpr> tmp = new InfixExpr();
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
    static RefPtr<Expr> maybeParseGenericApp(
        Parser*                             parser,

        // TODO: need to support more general expressions here
        RefPtr<Expr>     base)
    {
        if(peekTokenType(parser) != TokenType::OpLess)
            return base;
        return tryParseGenericApp(parser, base);
    }

    static RefPtr<Expr> parsePrefixExpr(Parser* parser);

    // Parse OOP `this` expression syntax
    static RefPtr<RefObject> parseThisExpr(Parser* parser, void* /*userData*/)
    {
        RefPtr<ThisExpr> expr = new ThisExpr();
        expr->scope = parser->currentScope;
        return expr;
    }

    static RefPtr<Expr> parseBoolLitExpr(Parser* /*parser*/, bool value)
    {
        RefPtr<BoolLiteralExpr> expr = new BoolLiteralExpr();
        expr->value = value;
        return expr;
    }

    static RefPtr<RefObject> parseTrueExpr(Parser* parser, void* /*userData*/)
    {
        return parseBoolLitExpr(parser, true);
    }

    static RefPtr<RefObject> parseFalseExpr(Parser* parser, void* /*userData*/)
    {
        return parseBoolLitExpr(parser, false);
    }

    static RefPtr<Expr> parseAtomicExpr(Parser* parser)
    {
        switch( peekTokenType(parser) )
        {
        default:
            // TODO: should this return an error expression instead of NULL?
            parser->sink->diagnose(parser->tokenReader.PeekLoc(), Diagnostics::syntaxError);
            return nullptr;

        // Either:
        // - parenthized expression `(exp)`
        // - cast `(type) exp`
        //
        // Proper disambiguation requires mixing up parsing
        // and semantic checking (which we should do eventually)
        // but for now we will follow some hueristics.
        case TokenType::LParent:
            {
                Token openParen = parser->ReadToken(TokenType::LParent);

                if (peekTypeName(parser) && parser->LookAheadToken(TokenType::RParent, 1))
                {
                    RefPtr<TypeCastExpr> tcexpr = new ExplicitCastExpr();
                    parser->FillPosition(tcexpr.Ptr());
                    tcexpr->FunctionExpr = parser->ParseType();
                    parser->ReadToken(TokenType::RParent);

                    auto arg = parsePrefixExpr(parser);
                    tcexpr->Arguments.Add(arg);

                    return tcexpr;
                }
                else
                {
                    RefPtr<Expr> base = parser->ParseExpression();
                    parser->ReadToken(TokenType::RParent);

                    RefPtr<ParenExpr> parenExpr = new ParenExpr();
                    parenExpr->loc = openParen.loc;
                    parenExpr->base = base;
                    return parenExpr;
                }
            }

        // An initializer list `{ expr, ... }`
        case TokenType::LBrace:
            {
                RefPtr<InitializerListExpr> initExpr = new InitializerListExpr();
                parser->FillPosition(initExpr.Ptr());

                // Initializer list
                parser->ReadToken(TokenType::LBrace);

                List<RefPtr<Expr>> exprs;

                for(;;)
                {
                    if(AdvanceIfMatch(parser, TokenType::RBrace))
                        break;

                    auto expr = parser->ParseArgExpr();
                    if( expr )
                    {
                        initExpr->args.Add(expr);
                    }

                    if(AdvanceIfMatch(parser, TokenType::RBrace))
                        break;

                    parser->ReadToken(TokenType::Comma);
                }

                return initExpr;
            }

        case TokenType::IntegerLiteral:
            {
                RefPtr<IntegerLiteralExpr> constExpr = new IntegerLiteralExpr();
                parser->FillPosition(constExpr.Ptr());

                auto token = parser->tokenReader.AdvanceToken();
                constExpr->token = token;

                UnownedStringSlice suffix;
                IntegerLiteralValue value = getIntegerLiteralValue(token, &suffix);

                // Look at any suffix on the value
                char const* suffixCursor = suffix.begin();
                const char*const suffixEnd = suffix.end();

                RefPtr<Type> suffixType = nullptr;
                if( suffixCursor < suffixEnd )
                {
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
                        suffixType = parser->getSession()->getErrorType();
                    }
                    // `u` or `ul` suffix -> `uint`
                    else if(uCount == 1 && (lCount <= 1))
                    {
                        suffixType = parser->getSession()->getUIntType();
                    }
                    // `l` suffix on integer -> `int` (== `long`)
                    else if(lCount == 1 && !uCount)
                    {
                        suffixType = parser->getSession()->getIntType();
                    }
                    // `ull` suffix -> `uint64_t`
                    else if(uCount == 1 && lCount == 2)
                    {
                        suffixType = parser->getSession()->getUInt64Type();
                    }
                    // `ll` suffix -> `int64_t`
                    else if(uCount == 0 && lCount == 2)
                    {
                        suffixType = parser->getSession()->getInt64Type();
                    }
                    // TODO: do we need suffixes for smaller integer types?
                    else
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidIntegerLiteralSuffix, suffix);
                        suffixType = parser->getSession()->getErrorType();
                    }
                }

                constExpr->value = value;
                constExpr->type = QualType(suffixType);

                return constExpr;
            }


        case TokenType::FloatingPointLiteral:
            {
                RefPtr<FloatingPointLiteralExpr> constExpr = new FloatingPointLiteralExpr();
                parser->FillPosition(constExpr.Ptr());

                auto token = parser->tokenReader.AdvanceToken();
                constExpr->token = token;

                UnownedStringSlice suffix;
                FloatingPointLiteralValue value = getFloatingPointLiteralValue(token, &suffix);

                // Look at any suffix on the value
                char const* suffixCursor = suffix.begin();
                const char*const suffixEnd = suffix.end();

                RefPtr<Type> suffixType = nullptr;
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
                        suffixType = parser->getSession()->getErrorType();
                    }
                    // `f` suffix -> `float`
                    if(fCount == 1 && !lCount)
                    {
                        suffixType = parser->getSession()->getFloatType();
                    }
                    // `l` or `lf` suffix on floating-point literal -> `double`
                    else if(lCount == 1 && (fCount <= 1))
                    {
                        suffixType = parser->getSession()->getDoubleType();
                    }
                    // `h` or `hf` suffix on floating-point literal -> `half`
                    else if(lCount == 1 && (fCount <= 1))
                    {
                        suffixType = parser->getSession()->getHalfType();
                    }
                    // TODO: are there other suffixes we need to handle?
                    else
                    {
                        parser->sink->diagnose(token, Diagnostics::invalidFloatingPointLiteralSuffix, suffix);
                        suffixType = parser->getSession()->getErrorType();
                    }
                }

                constExpr->value = value;
                constExpr->type = QualType(suffixType);

                return constExpr;
            }

        case TokenType::StringLiteral:
            {
                RefPtr<StringLiteralExpr> constExpr = new StringLiteralExpr();
                auto token = parser->tokenReader.AdvanceToken();
                constExpr->token = token;
                parser->FillPosition(constExpr.Ptr());

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
                        token = parser->tokenReader.AdvanceToken();
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

                RefPtr<Expr> parsedExpr;
                if (tryParseUsingSyntaxDecl<Expr>(parser, &parsedExpr))
                {
                    if (!parsedExpr->loc.isValid())
                    {
                        parsedExpr->loc = nameToken.loc;
                    }
                    return parsedExpr;
                }

                // Default behavior is just to create a name expression
                RefPtr<VarExpr> varExpr = new VarExpr();
                varExpr->scope = parser->currentScope.Ptr();
                parser->FillPosition(varExpr.Ptr());

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

    static RefPtr<Expr> parsePostfixExpr(Parser* parser)
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
                    RefPtr<OperatorExpr> postfixExpr = new PostfixExpr();
                    parser->FillPosition(postfixExpr.Ptr());
                    postfixExpr->FunctionExpr = parseOperator(parser);
                    postfixExpr->Arguments.Add(expr);

                    expr = postfixExpr;
                }
                break;

            // Subscript operation `a[i]`
            case TokenType::LBracket:
                {
                    RefPtr<IndexExpr> indexExpr = new IndexExpr();
                    indexExpr->BaseExpression = expr;
                    parser->FillPosition(indexExpr.Ptr());
                    parser->ReadToken(TokenType::LBracket);
                    // TODO: eventually we may want to support multiple arguments inside the `[]`
                    if (!parser->LookAheadToken(TokenType::RBracket))
                    {
                        indexExpr->IndexExpression = parser->ParseExpression();
                    }
                    parser->ReadToken(TokenType::RBracket);

                    expr = indexExpr;
                }
                break;

            // Call oepration `f(x)`
            case TokenType::LParent:
                {
                    RefPtr<InvokeExpr> invokeExpr = new InvokeExpr();
                    invokeExpr->FunctionExpr = expr;
                    parser->FillPosition(invokeExpr.Ptr());
                    parser->ReadToken(TokenType::LParent);
                    while (!parser->tokenReader.IsAtEnd())
                    {
                        if (!parser->LookAheadToken(TokenType::RParent))
                            invokeExpr->Arguments.Add(parser->ParseArgExpr());
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

            // Member access `x.m`
            case TokenType::Dot:
                {
                    RefPtr<MemberExpr> memberExpr = new MemberExpr();

                    // TODO(tfoley): why would a member expression need this?
                    memberExpr->scope = parser->currentScope.Ptr();

                    parser->FillPosition(memberExpr.Ptr());
                    memberExpr->BaseExpression = expr;
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

    static RefPtr<Expr> parsePrefixExpr(Parser* parser)
    {
        switch( peekTokenType(parser) )
        {
        default:
            return parsePostfixExpr(parser);

        case TokenType::OpInc:
        case TokenType::OpDec:
        case TokenType::OpNot:
        case TokenType::OpBitNot:
        case TokenType::OpAdd:
        case TokenType::OpSub:
            {
                RefPtr<PrefixExpr> prefixExpr = new PrefixExpr();
                parser->FillPosition(prefixExpr.Ptr());
                prefixExpr->FunctionExpr = parseOperator(parser);
                prefixExpr->Arguments.Add(parsePrefixExpr(parser));
                return prefixExpr;
            }
            break;
        }
    }

    RefPtr<Expr> Parser::ParseLeafExpression()
    {
        return parsePrefixExpr(this);
    }

    RefPtr<Expr> parseTypeFromSourceFile(
        Session*                        session,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope,
        NamePool*                       namePool,
        SourceLanguage                  sourceLanguage)
    {
        Parser parser(session, tokens, sink, outerScope);
        parser.currentScope = outerScope;
        parser.namePool = namePool;
        parser.sourceLanguage = sourceLanguage;
        return parser.ParseType();
    }

    // Parse a source file into an existing translation unit
    void parseSourceFile(
        TranslationUnitRequest*         translationUnit,
        TokenSpan const&                tokens,
        DiagnosticSink*                 sink,
        RefPtr<Scope> const&            outerScope)
    {
        Parser parser(translationUnit->getSession(), tokens, sink, outerScope);
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
        SyntaxClass<RefObject>      syntaxClass)
    {
        Name* name = session->getNamePool()->getName(nameText);

        RefPtr<SyntaxDecl> syntaxDecl = new SyntaxDecl();
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

    static RefPtr<RefObject> parseIntrinsicOpModifier(Parser* parser, void* /*userData*/)
    {
        RefPtr<IntrinsicOpModifier> modifier = new IntrinsicOpModifier();

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
                modifier->op = IROp(-StringToInt(parser->ReadToken().Content));
            }
            else if (parser->LookAheadToken(TokenType::IntegerLiteral))
            {
                modifier->op = IROp(StringToInt(parser->ReadToken().Content));
            }
            else
            {
                modifier->opToken = parser->ReadToken(TokenType::Identifier);

                modifier->op = findIROp(modifier->opToken.Content);

                if (modifier->op == kIROp_Invalid)
                {
                    parser->sink->diagnose(modifier->opToken, Diagnostics::unimplemented, "unknown intrinsic op");
                }
            }

            parser->ReadToken(TokenType::RParent);
        }


        return modifier;
    }

    static RefPtr<RefObject> parseTargetIntrinsicModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = new TargetIntrinsicModifier();

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

    static RefPtr<RefObject> parseSpecializedForTargetModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = new SpecializedForTargetModifier();
        if (AdvanceIf(parser, TokenType::LParent))
        {
            modifier->targetToken = parser->ReadToken(TokenType::Identifier);
            parser->ReadToken(TokenType::RParent);
        }
        return modifier;
    }

    static RefPtr<RefObject> parseGLSLExtensionModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = new RequiredGLSLExtensionModifier();

        parser->ReadToken(TokenType::LParent);
        modifier->extensionNameToken = parser->ReadToken(TokenType::Identifier);
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static RefPtr<RefObject> parseGLSLVersionModifier(Parser* parser, void* /*userData*/)
    {
        auto modifier = new RequiredGLSLVersionModifier();

        parser->ReadToken(TokenType::LParent);
        modifier->versionNumberToken = parser->ReadToken(TokenType::IntegerLiteral);
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static RefPtr<RefObject> parseLayoutModifier(Parser* parser, void* /*userData*/)
    {
        ModifierListBuilder listBuilder;

        listBuilder.add(new GLSLLayoutModifierGroupBegin());
        
        parser->ReadToken(TokenType::LParent);
        while (!AdvanceIfMatch(parser, TokenType::RParent))
        {
            auto nameAndLoc = expectIdentifier(parser);
            const String& nameText = nameAndLoc.name->text;

            if (nameText == "binding" ||
                nameText == "set")
            {
                GLSLBindingAttribute* attr = listBuilder.find<GLSLBindingAttribute>();
                if (!attr)
                {
                    attr = new GLSLBindingAttribute();
                    listBuilder.add(attr);
                }

                parser->ReadToken(TokenType::OpAssign);

                Token valToken = parser->ReadToken(TokenType::IntegerLiteral);
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
            else
            {
                RefPtr<Modifier> modifier;

#define CASE(key, type) if (nameText == #key) { modifier = new type; } else
                CASE(push_constant, PushConstantAttribute) 
                CASE(shaderRecordNV, ShaderRecordAttribute)
                CASE(constant_id,   GLSLConstantIDLayoutModifier) 
                CASE(location, GLSLLocationLayoutModifier) 
                CASE(local_size_x, GLSLLocalSizeXLayoutModifier) 
                CASE(local_size_y, GLSLLocalSizeYLayoutModifier) 
                CASE(local_size_z, GLSLLocalSizeZLayoutModifier)
                {
                    modifier = new GLSLUnparsedLayoutModifier();
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

        listBuilder.add(new GLSLLayoutModifierGroupEnd());

        return listBuilder.getFirst();
    }

    static RefPtr<RefObject> parseBuiltinTypeModifier(Parser* parser, void* /*userData*/)
    {
        RefPtr<BuiltinTypeModifier> modifier = new BuiltinTypeModifier();
        parser->ReadToken(TokenType::LParent);
        modifier->tag = BaseType(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).Content));
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static RefPtr<RefObject> parseMagicTypeModifier(Parser* parser, void* /*userData*/)
    {
        RefPtr<MagicTypeModifier> modifier = new MagicTypeModifier();
        parser->ReadToken(TokenType::LParent);
        modifier->name = parser->ReadToken(TokenType::Identifier).Content;
        if (AdvanceIf(parser, TokenType::Comma))
        {
            modifier->tag = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).Content));
        }
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }

    static RefPtr<RefObject> parseIntrinsicTypeModifier(Parser* parser, void* /*userData*/)
    {
        RefPtr<IntrinsicTypeModifier> modifier = new IntrinsicTypeModifier();
        parser->ReadToken(TokenType::LParent);
        modifier->irOp = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).Content));
        while( AdvanceIf(parser, TokenType::Comma) )
        {
            auto operand = uint32_t(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).Content));
            modifier->irOperands.Add(operand);
        }
        parser->ReadToken(TokenType::RParent);

        return modifier;
    }
    static RefPtr<RefObject> parseImplicitConversionModifier(Parser* parser, void* /*userData*/)
    {
        RefPtr<ImplicitConversionModifier> modifier = new ImplicitConversionModifier();

        ConversionCost cost = kConversionCost_Default;
        if( AdvanceIf(parser, TokenType::LParent) )
        {
            cost = ConversionCost(StringToInt(parser->ReadToken(TokenType::IntegerLiteral).Content));
            parser->ReadToken(TokenType::RParent);
        }
        modifier->cost = cost;
        return modifier;
    }

    static RefPtr<RefObject> parseAttributeTargetModifier(Parser* parser, void* /*userData*/)
    {
        expect(parser, TokenType::LParent);
        auto syntaxClassNameAndLoc = expectIdentifier(parser);
        expect(parser, TokenType::RParent);

        auto syntaxClass = parser->getSession()->findSyntaxClass(syntaxClassNameAndLoc.name);

        RefPtr<AttributeTargetModifier> modifier = new AttributeTargetModifier();
        modifier->syntaxClass = syntaxClass;

        return modifier;
    }

    RefPtr<ModuleDecl> populateBaseLanguageModule(
        Session*        session,
        RefPtr<Scope>   scope)
    {
        RefPtr<ModuleDecl> moduleDecl = new ModuleDecl();
        scope->containerDecl = moduleDecl;

        // Add syntax for declaration keywords
    #define DECL(KEYWORD, CALLBACK) \
        addBuiltinSyntax<Decl>(session, scope, #KEYWORD, &CALLBACK)
        DECL(typedef,         ParseTypeDef);
        DECL(associatedtype,  parseAssocType);
        DECL(type_param,    parseGlobalGenericParamDecl);
        DECL(cbuffer,         parseHLSLCBufferDecl);
        DECL(tbuffer,         parseHLSLTBufferDecl);
        DECL(__generic,       ParseGenericDecl);
        DECL(__extension,     ParseExtensionDecl);
        DECL(extension,       ParseExtensionDecl);
        DECL(__init,          parseConstructorDecl);
        DECL(__subscript,     ParseSubscriptDecl);
        DECL(interface,       parseInterfaceDecl);
        DECL(syntax,          parseSyntaxDecl);
        DECL(attribute_syntax,parseAttributeSyntaxDecl);
        DECL(__import,        parseImportDecl);
        DECL(import,          parseImportDecl);
        DECL(let,             parseLetDecl);
        DECL(var,             parseVarDecl);
        DECL(func,            parseFuncDecl);
        DECL(typealias,       parseTypeAliasDecl);

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
        MODIFIER(precise,           HLSLPreciseModifier);
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
        EXPR(true,  parseTrueExpr);
        EXPR(false, parseFalseExpr);

    #undef EXPR

        return moduleDecl;
    }

}
