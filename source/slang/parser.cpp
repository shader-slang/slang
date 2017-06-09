#include "Parser.h"

#include <assert.h>

#include "lookup.h"

namespace Slang
{
    namespace Compiler
    {
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
            CompileOptions& options;
            int anonymousCounter = 0;

            RefPtr<Scope> outerScope;
            RefPtr<Scope> currentScope;

            TokenReader tokenReader;
            DiagnosticSink * sink;
            String fileName;
            int genericDepth = 0;

            // Is the parser in a "recovering" state?
            // During recovery we don't emit additional errors, until we find
            // a token that we expected, when we exit recovery.
            bool isRecovering = false;

            void FillPosition(SyntaxNode * node)
            {
                node->Position = tokenReader.PeekLoc();
            }
            void PushScope(ContainerDecl* containerDecl)
            {
                RefPtr<Scope> newScope = new Scope();
                newScope->containerDecl = containerDecl;
                newScope->parent = currentScope;

                currentScope = newScope;
            }
            void PopScope()
            {
                currentScope = currentScope->parent;
            }
            Parser(
                CompileOptions& options,
                TokenSpan const& _tokens,
                DiagnosticSink * sink,
                String _fileName,
                RefPtr<Scope> const& outerScope)
                : options(options)
                , tokenReader(_tokens)
                , sink(sink)
                , fileName(_fileName)
                , outerScope(outerScope)
            {}
            RefPtr<ProgramSyntaxNode> Parse();

            Token ReadToken();
            Token ReadToken(TokenType type);
            Token ReadToken(const char * string);
            bool LookAheadToken(TokenType type, int offset = 0);
            bool LookAheadToken(const char * string, int offset = 0);
            void                                        parseSourceFile(ProgramSyntaxNode* program);
            RefPtr<ProgramSyntaxNode>					ParseProgram();
            RefPtr<StructSyntaxNode>					ParseStruct();
            RefPtr<ClassSyntaxNode>					    ParseClass();
            RefPtr<StatementSyntaxNode>					ParseStatement();
            RefPtr<StatementSyntaxNode>			        ParseBlockStatement();
            RefPtr<VarDeclrStatementSyntaxNode>			ParseVarDeclrStatement(Modifiers modifiers);
            RefPtr<IfStatementSyntaxNode>				ParseIfStatement();
            RefPtr<ForStatementSyntaxNode>				ParseForStatement();
            RefPtr<WhileStatementSyntaxNode>			ParseWhileStatement();
            RefPtr<DoWhileStatementSyntaxNode>			ParseDoWhileStatement();
            RefPtr<BreakStatementSyntaxNode>			ParseBreakStatement();
            RefPtr<ContinueStatementSyntaxNode>			ParseContinueStatement();
            RefPtr<ReturnStatementSyntaxNode>			ParseReturnStatement();
            RefPtr<ExpressionStatementSyntaxNode>		ParseExpressionStatement();
            RefPtr<ExpressionSyntaxNode>				ParseExpression(Precedence level = Precedence::Comma);

            // Parse an expression that might be used in an initializer or argument context, so we should avoid operator-comma
            inline RefPtr<ExpressionSyntaxNode>			ParseInitExpr() { return ParseExpression(Precedence::Assignment); }
            inline RefPtr<ExpressionSyntaxNode>			ParseArgExpr()  { return ParseExpression(Precedence::Assignment); }

            RefPtr<ExpressionSyntaxNode>				ParseLeafExpression();
            RefPtr<ParameterSyntaxNode>					ParseParameter();
            RefPtr<ExpressionSyntaxNode>				ParseType();
            TypeExp										ParseTypeExp();

            Parser & operator = (const Parser &) = delete;
        };

        // Forward Declarations

        static void ParseDeclBody(
            Parser*						parser,
            ContainerDecl*				containerDecl,
            TokenType	                closingToken);

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
            TokenType tokenType = reader->AdvanceToken().Type;
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

            // Determine if we are looking for a closing token at all...
            bool lookingForClose = false;
            for (int ii = 0; ii < recoverBeforeCount; ++ii)
            {
                if (IsClosingToken(recoverBefore[ii]))
                    lookingForClose = true;
            }
            for (int ii = 0; ii < recoverAfterCount; ++ii)
            {
                if (IsClosingToken(recoverAfter[ii]))
                    lookingForClose = true;
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
                    if (!lookingForClose)
                        return false;
                    break;

                // never skip a `}`, to avoid spurious errors
                case TokenType::RBrace:
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

        RefPtr<ProgramSyntaxNode> Parser::Parse()
        {
            return ParseProgram();
        }

        RefPtr<TypeDefDecl> ParseTypeDef(Parser* parser)
        {
            // Consume the `typedef` keyword
            parser->ReadToken("typedef");

            // TODO(tfoley): parse an actual declarator
            auto type = parser->ParseTypeExp();

            auto nameToken = parser->ReadToken(TokenType::Identifier);

            RefPtr<TypeDefDecl> typeDefDecl = new TypeDefDecl();
            typeDefDecl->Name = nameToken;
            typeDefDecl->Type = type;

            return typeDefDecl;
        }

        // Add a modifier to a list of modifiers being built
        static void AddModifier(RefPtr<Modifier>** ioModifierLink, RefPtr<Modifier> modifier)
        {
            RefPtr<Modifier>*& modifierLink = *ioModifierLink;

            while(*modifierLink)
                modifierLink = &(*modifierLink)->next;

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

        // Parse HLSL-style `[name(arg, ...)]` style "attribute" modifiers
        static void ParseSquareBracketAttributes(Parser* parser, RefPtr<Modifier>** ioModifierLink)
        {
            parser->ReadToken(TokenType::LBracket);
            for(;;)
            {
                auto nameToken = parser->ReadToken(TokenType::Identifier);
                RefPtr<HLSLUncheckedAttribute> modifier = new HLSLUncheckedAttribute();
                modifier->nameToken = nameToken;

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
        }

        static Modifiers ParseModifiers(Parser* parser)
        {
            Modifiers modifiers;
            RefPtr<Modifier>* modifierLink = &modifiers.first;
            for (;;)
            {
                CodePosition loc = parser->tokenReader.PeekLoc();

                if (0) {}

            #define CASE(KEYWORD, TYPE)						\
                else if(AdvanceIf(parser, #KEYWORD)) do {		\
                    RefPtr<TYPE> modifier = new TYPE();		\
                    modifier->Position = loc;				\
                    AddModifier(&modifierLink, modifier);	\
                } while(0)

                CASE(in, InModifier);
                CASE(input, InputModifier);
                CASE(out, OutModifier);
                CASE(inout, InOutModifier);
                CASE(const, ConstModifier);
                CASE(instance, InstanceModifier);
                CASE(__builtin, BuiltinModifier);

                CASE(inline, InlineModifier);
                CASE(public, PublicModifier);
                CASE(require, RequireModifier);
                CASE(param, ParamModifier);
                CASE(extern, ExternModifier);

                CASE(row_major, HLSLRowMajorLayoutModifier);
                CASE(column_major, HLSLColumnMajorLayoutModifier);

                CASE(nointerpolation, HLSLNoInterpolationModifier);
                CASE(linear, HLSLLinearModifier);
                CASE(sample, HLSLSampleModifier);
                CASE(centroid, HLSLCentroidModifier);
                CASE(precise, HLSLPreciseModifier);
                CASE(shared, HLSLEffectSharedModifier);
                CASE(groupshared, HLSLGroupSharedModifier);
                CASE(static, HLSLStaticModifier);
                CASE(uniform, HLSLUniformModifier);
                CASE(volatile, HLSLVolatileModifier);

                // Modifiers for geometry shader input
                CASE(point,         HLSLPointModifier);
                CASE(line,          HLSLLineModifier);
                CASE(triangle,      HLSLTriangleModifier);
                CASE(lineadj,       HLSLLineAdjModifier);
                CASE(triangleadj,   HLSLTriangleAdjModifier);

                // Modifiers for unary operator declarations
                CASE(__prefix,   PrefixModifier);
                CASE(__postfix,  PostfixModifier);

                #undef CASE

                else if (AdvanceIf(parser, "__intrinsic"))
                {
                    auto modifier = new IntrinsicModifier();
                    modifier->Position = loc;

                    if (AdvanceIf(parser, TokenType::LParent))
                    {
                        if (parser->LookAheadToken(TokenType::IntLiterial))
                        {
                            modifier->op = (IntrinsicOp)StringToInt(parser->ReadToken().Content);
                        }
                        else
                        {
                            modifier->opToken = parser->ReadToken(TokenType::Identifier);

                            modifier->op = findIntrinsicOp(modifier->opToken.Content.Buffer());

                            if (modifier->op == IntrinsicOp::Unknown)
                            {
                                parser->sink->diagnose(loc, Diagnostics::unimplemented, "unknown intrinsic op");
                            }
                        }

                        parser->ReadToken(TokenType::RParent);
                    }

                    AddModifier(&modifierLink, modifier);
                }


                else if (AdvanceIf(parser, "layout"))
                {
                    parser->ReadToken(TokenType::LParent);
                    while (!AdvanceIfMatch(parser, TokenType::RParent))
                    {
                        auto nameToken = parser->ReadToken(TokenType::Identifier);

                        RefPtr<GLSLLayoutModifier> modifier;

                        // TODO: better handling of this choise (e.g., lookup in scope)
                        if(0) {}
                    #define CASE(KEYWORD, CLASS) \
                        else if(nameToken.Content == #KEYWORD) modifier = new CLASS()

                        CASE(constant_id,   GLSLConstantIDLayoutModifier);
                        CASE(binding,       GLSLBindingLayoutModifier);
                        CASE(set,           GLSLSetLayoutModifier);
                        CASE(location,      GLSLLocationLayoutModifier);

                    #undef CASE
                        else
                        {
                            modifier = new GLSLUnparsedLayoutModifier();
                        }

                        modifier->nameToken = nameToken;

                        if(AdvanceIf(parser, TokenType::OpAssign))
                        {
                            modifier->valToken = parser->ReadToken(TokenType::IntLiterial);
                        }

                        AddModifier(&modifierLink, modifier);

                        if (AdvanceIf(parser, TokenType::RParent))
                            break;
                        parser->ReadToken(TokenType::Comma);
                    }
                }
                else if (parser->tokenReader.PeekTokenType() == TokenType::LBracket)
                {
                    ParseSquareBracketAttributes(parser, &modifierLink);
                }
                else if (AdvanceIf(parser,"__builtin_type"))
                {
                    RefPtr<BuiltinTypeModifier> modifier = new BuiltinTypeModifier();
                    parser->ReadToken(TokenType::LParent);
                    modifier->tag = BaseType(StringToInt(parser->ReadToken(TokenType::IntLiterial).Content));
                    parser->ReadToken(TokenType::RParent);

                    AddModifier(&modifierLink, modifier);
                }
                else if (AdvanceIf(parser,"__magic_type"))
                {
                    RefPtr<MagicTypeModifier> modifier = new MagicTypeModifier();
                    parser->ReadToken(TokenType::LParent);
                    modifier->name = parser->ReadToken(TokenType::Identifier).Content;
                    if (AdvanceIf(parser, TokenType::Comma))
                    {
                        modifier->tag = uint32_t(StringToInt(parser->ReadToken(TokenType::IntLiterial).Content));
                    }
                    parser->ReadToken(TokenType::RParent);

                    AddModifier(&modifierLink, modifier);
                }
                else
                {
                    // Fallback case if none of the above explicit cases matched.

                    // If we are looking at an identifier, then it may map to a
                    // modifier declaration visible in the current scope
                    if( parser->LookAheadToken(TokenType::Identifier) )
                    {
                        LookupResult lookupResult = LookUp(
                            parser->tokenReader.PeekToken().Content,
                            parser->currentScope);

                        if( lookupResult.isValid() && !lookupResult.isOverloaded() )
                        {
                            auto& item = lookupResult.item;
                            auto decl = item.declRef.GetDecl();

                            if( auto modifierDecl = dynamic_cast<ModifierDecl*>(decl) )
                            {
                                // We found a declaration for some modifier syntax,
                                // so lets create an instance of the type it names
                                // here.

                                auto syntax = createInstanceOfSyntaxClassByName(modifierDecl->classNameToken.Content);
                                auto modifier = dynamic_cast<Modifier*>(syntax);

                                if( modifier )
                                {
                                    modifier->Position = parser->tokenReader.PeekLoc();
                                    modifier->nameToken = parser->ReadToken(TokenType::Identifier);

                                    AddModifier(&modifierLink, modifier);
                                    continue;
                                }
                                else
                                {
                                    parser->ReadToken(TokenType::Identifier);
                                    assert(!"unexpected");
                                }
                            }
                        }
                    }

                    // Done with modifier list
                    return modifiers;
                }
            }
        }

        static RefPtr<Decl> ParseUsing(
            Parser* parser)
        {
            parser->ReadToken("using");
            if (parser->tokenReader.PeekTokenType() == TokenType::StringLiterial)
            {
                auto usingDecl = new UsingFileDecl();
                usingDecl->fileName = parser->ReadToken(TokenType::StringLiterial);
                parser->ReadToken(TokenType::Semicolon);
                return usingDecl;
            }
            else
            {
                unexpected();
            }
        }

        static Token ParseDeclName(
            Parser* parser)
        {
            Token name;
            if (AdvanceIf(parser, "operator"))
            {
                name = parser->ReadToken();
                switch (name.Type)
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
                        name.Content = name.Content + ":";
                        break;
                    }

                default:
                    parser->sink->diagnose(name.Position, Diagnostics::invalidOperator, name.Content);
                    break;
                }
            }
            else
            {
                name = parser->ReadToken(TokenType::Identifier);
            }
            return name;
        }

        // A "declarator" as used in C-style languages
        struct Declarator : RefObject
        {
            // Different cases of declarator appear as "flavors" here
            enum class Flavor
            {
                Name,
                Pointer,
                Array,
            };
            Flavor flavor;
        };

        // The most common case of declarator uses a simple name
        struct NameDeclarator : Declarator
        {
            Token nameToken;
        };

        // A declarator that declares a pointer type
        struct PointerDeclarator : Declarator
        {
            // location of the `*` token
            CodePosition starLoc;

            RefPtr<Declarator>				inner;
        };

        // A declarator that declares an array type
        struct ArrayDeclarator : Declarator
        {
            RefPtr<Declarator>				inner;

            // location of the `[` token
            CodePosition openBracketLoc;

            // The expression that yields the element count, or NULL
            RefPtr<ExpressionSyntaxNode>	elementCountExpr;
        };

        // "Unwrapped" information about a declarator
        struct DeclaratorInfo
        {
            RefPtr<ExpressionSyntaxNode>	typeSpec;
            Token							nameToken;
            RefPtr<Modifier>				semantics;
            RefPtr<ExpressionSyntaxNode>	initializer;
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

        static void ParseFuncDeclHeader(
            Parser*                     parser,
            DeclaratorInfo const&       declaratorInfo,
            RefPtr<FunctionSyntaxNode>  decl)
        {
            parser->PushScope(decl.Ptr());

            parser->FillPosition(decl.Ptr());
            decl->Position = declaratorInfo.nameToken.Position;

            decl->Name = declaratorInfo.nameToken;
            decl->ReturnType = TypeExp(declaratorInfo.typeSpec);
            parseParameterList(parser, decl);
            ParseOptSemantics(parser, decl.Ptr());
        }

        static RefPtr<Decl> ParseFuncDecl(
            Parser*                 parser,
            ContainerDecl*          /*containerDecl*/,
            DeclaratorInfo const&   declaratorInfo)
        {
            RefPtr<FunctionSyntaxNode> decl = new FunctionSyntaxNode();
            ParseFuncDeclHeader(parser, declaratorInfo, decl);

            if (AdvanceIf(parser, TokenType::Semicolon))
            {
                // empty body
            }
            else
            {
                decl->Body = parser->ParseBlockStatement();
            }

            parser->PopScope();
            return decl;
        }

        static RefPtr<VarDeclBase> CreateVarDeclForContext(
            ContainerDecl*  containerDecl )
        {
            if (dynamic_cast<StructSyntaxNode*>(containerDecl) || dynamic_cast<ClassSyntaxNode*>(containerDecl))
            {
                return new StructField();
            }
            else if (dynamic_cast<CallableDecl*>(containerDecl))
            {
                return new ParameterSyntaxNode();
            }
            else
            {
                return new Variable();
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


        static String GenerateName(Parser* parser, String const& base)
        {
            // TODO: somehow mangle the name to avoid clashes
            return base;
        }

        static String GenerateName(Parser* parser)
        {
            return GenerateName(parser, "_anonymous_" + String(parser->anonymousCounter++));
        }


        // Set up a variable declaration based on what we saw in its declarator...
        static void CompleteVarDecl(
            Parser*					parser,
            RefPtr<VarDeclBase>		decl,
            DeclaratorInfo const&	declaratorInfo)
        {
            parser->FillPosition(decl.Ptr());

            if( declaratorInfo.nameToken.Type == TokenType::Unknown )
            {
                // HACK(tfoley): we always give a name, even if the declarator didn't include one... :(
                decl->Name.Content = GenerateName(parser);
            }
            else
            {
                decl->Position = declaratorInfo.nameToken.Position;
                decl->Name = declaratorInfo.nameToken;
            }
            decl->Type = TypeExp(declaratorInfo.typeSpec);

            AddModifiers(decl.Ptr(), declaratorInfo.semantics);

            decl->Expr = declaratorInfo.initializer;
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
                    nameDeclarator->flavor = Declarator::Flavor::Name;
                    nameDeclarator->nameToken = ParseDeclName(parser);
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
            RefPtr<ExpressionSyntaxNode>	initializer;
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
                case Declarator::Flavor::Name:
                    {
                        auto nameDeclarator = (NameDeclarator*) declarator.Ptr();
                        ioInfo->nameToken = nameDeclarator->nameToken;
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

                        auto arrayTypeExpr = new IndexExpressionSyntaxNode();
                        arrayTypeExpr->Position = arrayDeclarator->openBracketLoc;
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
            CodePosition        startPosition;
            RefPtr<Decl>        decl;
            RefPtr<DeclGroup>   group;

            // Add a new declaration to the potential group
            void addDecl(
                RefPtr<Decl>    newDecl)
            {
                assert(newDecl);
            
                if( decl )
                {
                    group = new DeclGroup();
                    group->Position = startPosition;
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
        RefPtr<ExpressionSyntaxNode> ParseGenericArg(Parser* parser)
        {
            return parser->ParseArgExpr();
        }

        // Create a type expression that will refer to the given declaration
        static RefPtr<ExpressionSyntaxNode>
        createDeclRefType(Parser* parser, RefPtr<Decl> decl)
        {
            // For now we just construct an expression that
            // will look up the given declaration by name.
            //
            // TODO: do this better, e.g. by filling in the `declRef` field directly

            auto expr = new VarExpressionSyntaxNode();
            expr->scope = parser->currentScope.Ptr();
            expr->Position = decl->getNameToken().Position;
            expr->Variable = decl->getName();
            return expr;
        }

        // Representation for a parsed type specifier, which might
        // include a declaration (e.g., of a `struct` type)
        struct TypeSpec
        {
            // If the type-spec declared something, then put it here
            RefPtr<Decl>                    decl;

            // Put the resulting expression (which should evaluate to a type) here
            RefPtr<ExpressionSyntaxNode>    expr;
        };

        static TypeSpec
        parseTypeSpec(Parser* parser)
        {
            TypeSpec typeSpec;

            // We may see a `struct` type specified here, and need to act accordingly
            //
            // TODO(tfoley): Handle the case where the user is just using `struct`
            // as a way to name an existing struct "tag" (e.g., `struct Foo foo;`)
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

            Token typeName = parser->ReadToken(TokenType::Identifier);

            auto basicType = new VarExpressionSyntaxNode();
            basicType->scope = parser->currentScope.Ptr();
            basicType->Position = typeName.Position;
            basicType->Variable = typeName.Content;

            RefPtr<ExpressionSyntaxNode> typeExpr = basicType;

            if (parser->LookAheadToken(TokenType::OpLess))
            {
                RefPtr<GenericAppExpr> gtype = new GenericAppExpr();
                parser->FillPosition(gtype.Ptr()); // set up scope for lookup
                gtype->Position = typeName.Position;
                gtype->FunctionExpr = typeExpr;
                parser->ReadToken(TokenType::OpLess);
                parser->genericDepth++;
                // For now assume all generics have at least one argument
                gtype->Arguments.Add(ParseGenericArg(parser));
                while (AdvanceIf(parser, TokenType::Comma))
                {
                    gtype->Arguments.Add(ParseGenericArg(parser));
                }
                parser->genericDepth--;
                parser->ReadToken(TokenType::OpGreater);
                typeExpr = gtype;
            }

            typeSpec.expr = typeExpr;
            return typeSpec;
        }


        static RefPtr<DeclBase> ParseDeclaratorDecl(
            Parser*         parser,
            ContainerDecl*  containerDecl)
        {
            CodePosition startPosition = parser->tokenReader.PeekLoc();

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


            InitDeclarator initDeclarator = ParseInitDeclarator(parser);

            DeclaratorInfo declaratorInfo;
            declaratorInfo.typeSpec = typeSpec.expr;


            // Rather than parse function declarators properly for now,
            // we'll just do a quick disambiguation here. This won't
            // matter unless we actually decide to support function-type parameters,
            // using C syntax.
            //
            if( parser->tokenReader.PeekTokenType() == TokenType::LParent

                // Only parse as a function if we didn't already see mutually-exclusive
                // constructs when parsing the declarator.
                && !initDeclarator.initializer
                && !initDeclarator.semantics)
            {
                // Looks like a function, so parse it like one.
                UnwrapDeclarator(initDeclarator, &declaratorInfo);
                return ParseFuncDecl(parser, containerDecl, declaratorInfo);
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

                return firstDecl;
            }

            // Otherwise we have multiple declarations in a sequence, and these
            // declarations need to somehow share both the type spec and modifiers.
            //
            // If there are any errors in the type specifier, we only want to hear
            // about it once, so we need to share structure rather than just
            // clone syntax.

            auto sharedTypeSpec = new SharedTypeExpr();
            sharedTypeSpec->Position = typeSpec.expr->Position;
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

        //
        // layout-semantic ::= (register | packoffset) '(' register-name component-mask? ')'
        // register-name ::= identifier
        // component-mask ::= '.' identifier
        //
        static void ParseHLSLLayoutSemantic(
            Parser*				parser,
            HLSLLayoutSemantic*	semantic)
        {
            semantic->name = parser->ReadToken(TokenType::Identifier);

            parser->ReadToken(TokenType::LParent);
            semantic->registerName = parser->ReadToken(TokenType::Identifier);
            if (AdvanceIf(parser, TokenType::Dot))
            {
                semantic->componentMask = parser->ReadToken(TokenType::Identifier);
            }
            parser->ReadToken(TokenType::RParent);
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
                ParseHLSLLayoutSemantic(parser, semantic.Ptr());
                return semantic;
            }
            else if (parser->LookAheadToken("packoffset"))
            {
                RefPtr<HLSLPackOffsetSemantic> semantic = new HLSLPackOffsetSemantic();
                ParseHLSLLayoutSemantic(parser, semantic.Ptr());
                return semantic;
            }
            else
            {
                RefPtr<HLSLSimpleSemantic> semantic = new HLSLSimpleSemantic();
                semantic->name = parser->ReadToken(TokenType::Identifier);
                return semantic;
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
            assert(!*link);

            for (;;)
            {
                RefPtr<Modifier> semantic = ParseSemantic(parser);
                if (semantic)
                {
                    *link = semantic;
                    link = &semantic->next;
                }

                switch (parser->tokenReader.PeekTokenType())
                {
                case TokenType::LBrace:
                case TokenType::Semicolon:
                case TokenType::Comma:
                case TokenType::RParent:
                case TokenType::EndOfFile:
                    return result;

                default:
                    break;
                }

                parser->ReadToken(TokenType::Colon);
            }

        }


        static void ParseOptSemantics(
            Parser* parser,
            Decl*	decl)
        {
            AddModifiers(decl, ParseOptSemantics(parser));
        }

        static RefPtr<Decl> ParseHLSLBufferDecl(
            Parser*	parser)
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

            // We first look at the declaration keywrod to determine
            // the type of buffer to declare:
            String bufferWrapperTypeName;
            CodePosition bufferWrapperTypeNamePos = parser->tokenReader.PeekLoc();
            if (AdvanceIf(parser, "cbuffer"))
            {
                bufferWrapperTypeName = "ConstantBuffer";
            }
            else if (AdvanceIf(parser, "tbuffer"))
            {
                bufferWrapperTypeName = "TextureBuffer";
            }
            else
            {
                Unexpected(parser);
            }

            // We are going to represent each buffer as a pair of declarations.
            // The first is a type declaration that holds all the members, while
            // the second is a variable declaration that uses the buffer type.
            RefPtr<StructSyntaxNode> bufferDataTypeDecl = new StructSyntaxNode();
            RefPtr<Variable> bufferVarDecl = new Variable();

            // Both declarations will have a location that points to the name
            parser->FillPosition(bufferDataTypeDecl.Ptr());
            parser->FillPosition(bufferVarDecl.Ptr());

            auto reflectionNameToken = parser->ReadToken(TokenType::Identifier);

            // Attach the reflection name to the block so we can use it
            auto reflectionNameModifier = new ParameterBlockReflectionName();
            reflectionNameModifier->nameToken = reflectionNameToken;
            addModifier(bufferVarDecl, reflectionNameModifier);

            // Both the buffer variable and its type need to have names generated
            bufferVarDecl->Name.Content = GenerateName(parser, "SLANG_constantBuffer_" + reflectionNameToken.Content);
            bufferDataTypeDecl->Name.Content = GenerateName(parser, "SLANG_ConstantBuffer_" + reflectionNameToken.Content);

            addModifier(bufferDataTypeDecl, new ImplicitParameterBlockElementTypeModifier());
            addModifier(bufferVarDecl, new ImplicitParameterBlockVariableModifier());

            // TODO(tfoley): We end up constructing unchecked syntax here that
            // is expected to type check into the right form, but it might be
            // cleaner to have a more explicit desugaring pass where we parse
            // these constructs directly into the AST and *then* desugar them.

            // Construct a type expression to reference the buffer data type
            auto bufferDataTypeExpr = new VarExpressionSyntaxNode();
            bufferDataTypeExpr->Position = bufferDataTypeDecl->Position;
            bufferDataTypeExpr->Variable = bufferDataTypeDecl->Name.Content;
            bufferDataTypeExpr->scope = parser->currentScope.Ptr();

            // Construct a type exrpession to reference the type constructor
            auto bufferWrapperTypeExpr = new VarExpressionSyntaxNode();
            bufferWrapperTypeExpr->Position = bufferWrapperTypeNamePos;
            bufferWrapperTypeExpr->Variable = bufferWrapperTypeName;

            // Always need to look this up in the outer scope,
            // so that it won't collide with, e.g., a local variable called `ConstantBuffer`
            bufferWrapperTypeExpr->scope = parser->outerScope;

            // Construct a type expression that represents the type for the variable,
            // which is the wrapper type applied to the data type
            auto bufferVarTypeExpr = new GenericAppExpr();
            bufferVarTypeExpr->Position = bufferVarDecl->Position;
            bufferVarTypeExpr->FunctionExpr = bufferWrapperTypeExpr;
            bufferVarTypeExpr->Arguments.Add(bufferDataTypeExpr);

            bufferVarDecl->Type.exp = bufferVarTypeExpr;

            // Any semantics applied to the bufer declaration are taken as applying
            // to the variable instead.
            ParseOptSemantics(parser, bufferVarDecl.Ptr());

            // The declarations in the body belong to the data type.
            parser->ReadToken(TokenType::LBrace);
            ParseDeclBody(parser, bufferDataTypeDecl.Ptr(), TokenType::RBrace);

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
        
        static void removeModifier(
            Modifiers&          modifiers,
            RefPtr<Modifier>    modifier)
        {
            RefPtr<Modifier>* link = &modifiers.first;
            while (*link)
            {
                if (*link == modifier)
                {
                    *link = (*link)->next;
                    return;
                }

                link = &(*link)->next;
            }
        }

        static RefPtr<Decl> parseGLSLBlockDecl(
            Parser*	    parser,
            Modifiers&   modifiers)
        {
            // An GLSL block like this:
            //
            //     uniform Foo { int a; float b; } foo;
            //
            // is treated as syntax sugar for a type declaration
            // and then a global variable declaration using that type:
            //
            //     struct $anonymous { int a; float b; };
            //     Block<$anonymous> foo;
            //
            // where `$anonymous` is a fresh name.
            //
            // If a "local name" like `foo` is not given, then
            // we make the declaration "transparent" so that lookup
            // will see through it to the members inside.


            CodePosition pos = parser->tokenReader.PeekLoc();

            // The initial name before the `{` is only supposed
            // to be made visible to reflection
            auto reflectionNameToken = parser->ReadToken(TokenType::Identifier);

            // Look at the qualifiers present on the block to decide what kind
            // of block we are looking at. Also *remove* those qualifiers so
            // that they don't interfere with downstream work.
            String blockWrapperTypeName;
            if( auto uniformMod = modifiers.findModifier<HLSLUniformModifier>() )
            {
                removeModifier(modifiers, uniformMod);
                blockWrapperTypeName = "ConstantBuffer";
            }
            else if( auto inMod = modifiers.findModifier<InModifier>() )
            {
                removeModifier(modifiers, inMod);
                blockWrapperTypeName = "__GLSLInputParameterBlock";
            }
            else if( auto outMod = modifiers.findModifier<OutModifier>() )
            {
                removeModifier(modifiers, outMod);
                blockWrapperTypeName = "__GLSLOutputParameterBlock";
            }
            else if( auto bufferMod = modifiers.findModifier<GLSLBufferModifier>() )
            {
                removeModifier(modifiers, bufferMod);
                blockWrapperTypeName = "__GLSLShaderStorageBuffer";
            }
            else
            {
                // Unknown case: just map to a constant buffer and hope for the best
                blockWrapperTypeName = "ConstantBuffer";
            }

            // We are going to represent each buffer as a pair of declarations.
            // The first is a type declaration that holds all the members, while
            // the second is a variable declaration that uses the buffer type.
            RefPtr<StructSyntaxNode> blockDataTypeDecl = new StructSyntaxNode();
            RefPtr<Variable> blockVarDecl = new Variable();

            addModifier(blockDataTypeDecl, new ImplicitParameterBlockElementTypeModifier());
            addModifier(blockVarDecl, new ImplicitParameterBlockVariableModifier());

            // Attach the reflection name to the block so we can use it
            auto reflectionNameModifier = new ParameterBlockReflectionName();
            reflectionNameModifier->nameToken = reflectionNameToken;
            addModifier(blockVarDecl, reflectionNameModifier);

            // Both declarations will have a location that points to the name
            parser->FillPosition(blockDataTypeDecl.Ptr());
            parser->FillPosition(blockVarDecl.Ptr());

            // Generate a unique name for the data type
            blockDataTypeDecl->Name.Content = GenerateName(parser, "SLANG_ParameterBlock_" + reflectionNameToken.Content);

            // TODO(tfoley): We end up constructing unchecked syntax here that
            // is expected to type check into the right form, but it might be
            // cleaner to have a more explicit desugaring pass where we parse
            // these constructs directly into the AST and *then* desugar them.

            // Construct a type expression to reference the buffer data type
            auto blockDataTypeExpr = new VarExpressionSyntaxNode();
            blockDataTypeExpr->Position = blockDataTypeDecl->Position;
            blockDataTypeExpr->Variable = blockDataTypeDecl->Name.Content;
            blockDataTypeExpr->scope = parser->currentScope.Ptr();

            // Construct a type exrpession to reference the type constructor
            auto blockWrapperTypeExpr = new VarExpressionSyntaxNode();
            blockWrapperTypeExpr->Position = pos;
            blockWrapperTypeExpr->Variable = blockWrapperTypeName;
            // Always need to look this up in the outer scope,
            // so that it won't collide with, e.g., a local variable called `ConstantBuffer`
            blockWrapperTypeExpr->scope = parser->outerScope;

            // Construct a type expression that represents the type for the variable,
            // which is the wrapper type applied to the data type
            auto blockVarTypeExpr = new GenericAppExpr();
            blockVarTypeExpr->Position = blockVarDecl->Position;
            blockVarTypeExpr->FunctionExpr = blockWrapperTypeExpr;
            blockVarTypeExpr->Arguments.Add(blockDataTypeExpr);

            blockVarDecl->Type.exp = blockVarTypeExpr;

            // The declarations in the body belong to the data type.
            parser->ReadToken(TokenType::LBrace);
            ParseDeclBody(parser, blockDataTypeDecl.Ptr(), TokenType::RBrace);

            if( parser->LookAheadToken(TokenType::Identifier) )
            {
                // The user gave an explicit name to the block,
                // so we need to use that as our variable name
                blockVarDecl->Name = parser->ReadToken(TokenType::Identifier);

                // TODO: in this case we make actually have a more complex
                // declarator, including `[]` brackets.
            }
            else
            {
                // synthesize a dummy name
                blockVarDecl->Name.Content = GenerateName(parser, "SLANG_parameterBlock_" + reflectionNameToken.Content);

                // Otherwise we have a transparent declaration, similar
                // to an HLSL `cbuffer`
                auto transparentModifier = new TransparentModifier();
                transparentModifier->Position = pos;
                addModifier(blockVarDecl, transparentModifier);
            }

            // Expect a trailing `;`
            parser->ReadToken(TokenType::Semicolon);

            // Because we are constructing two declarations, we have a thorny
            // issue that were are only supposed to return one.
            // For now we handle this by adding the type declaration to
            // the current scope manually, and then returning the variable
            // declaration.
            //
            // Note: this means that any modifiers that have already been parsed
            // will get attached to the variable declaration, not the type.
            // There might be cases where we need to shuffle things around.

            AddMember(parser->currentScope, blockDataTypeDecl);

            return blockVarDecl;
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
                paramDecl->Name = parser->ReadToken(TokenType::Identifier);
                if (AdvanceIf(parser, TokenType::Colon))
                {
                    paramDecl->Type = parser->ParseTypeExp();
                }
                if (AdvanceIf(parser, TokenType::OpAssign))
                {
                    paramDecl->Expr = parser->ParseInitExpr();
                }
                return paramDecl;
            }
            else
            {
                // default case is a type parameter
                auto paramDecl = new GenericTypeParamDecl();
                parser->FillPosition(paramDecl);
                paramDecl->Name = parser->ReadToken(TokenType::Identifier);
                if (AdvanceIf(parser, TokenType::Colon))
                {
                    // The user is apply a constraint to this type parameter...

                    auto paramConstraint = new GenericTypeConstraintDecl();
                    parser->FillPosition(paramConstraint);

                    auto paramType = DeclRefType::Create(DeclRef(paramDecl, nullptr));

                    auto paramTypeExpr = new SharedTypeExpr();
                    paramTypeExpr->Position = paramDecl->Position;
                    paramTypeExpr->base.type = paramType;
                    paramTypeExpr->Type = new TypeType(paramType);

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

        static RefPtr<Decl> ParseGenericDecl(
            Parser* parser)
        {
            RefPtr<GenericDecl> decl = new GenericDecl();
            parser->FillPosition(decl.Ptr());
            parser->PushScope(decl.Ptr());
            parser->ReadToken("__generic");
            parser->ReadToken(TokenType::OpLess);
            parser->genericDepth++;
            while (!parser->LookAheadToken(TokenType::OpGreater))
            {
                AddMember(decl, ParseGenericParamDecl(parser, decl));

if( parser->LookAheadToken(TokenType::OpGreater) )
break;

parser->ReadToken(TokenType::Comma);
            }
            parser->genericDepth--;
            parser->ReadToken(TokenType::OpGreater);

            decl->inner = ParseSingleDecl(parser, decl.Ptr());

            // A generic decl hijacks the name of the declaration
            // it wraps, so that lookup can find it.
            decl->Name = decl->inner->Name;

            parser->PopScope();
            return decl;
        }

        static RefPtr<Decl> ParseTraitConformanceDecl(
            Parser* parser)
        {
            RefPtr<TraitConformanceDecl> decl = new TraitConformanceDecl();
            parser->FillPosition(decl.Ptr());
            parser->ReadToken("__conforms");

            decl->base = parser->ParseTypeExp();

            return decl;
        }


        static RefPtr<ExtensionDecl> ParseExtensionDecl(Parser* parser)
        {
            RefPtr<ExtensionDecl> decl = new ExtensionDecl();
            parser->FillPosition(decl.Ptr());
            parser->ReadToken("__extension");
            decl->targetType = parser->ParseTypeExp();
            parser->ReadToken(TokenType::LBrace);
            ParseDeclBody(parser, decl.Ptr(), TokenType::RBrace);
            return decl;
        }

        static RefPtr<TraitDecl> ParseTraitDecl(Parser* parser)
        {
            RefPtr<TraitDecl> decl = new TraitDecl();
            parser->FillPosition(decl.Ptr());
            parser->ReadToken("__trait");
            decl->Name = parser->ReadToken(TokenType::Identifier);

            if( AdvanceIf(parser, TokenType::Colon) )
            {
                do
                {
                    auto base = parser->ParseTypeExp();
                    decl->bases.Add(base);
                } while( AdvanceIf(parser, TokenType::Comma) );
            }

            parser->ReadToken(TokenType::LBrace);
            ParseDeclBody(parser, decl.Ptr(), TokenType::RBrace);
            return decl;
        }

        static RefPtr<ConstructorDecl> ParseConstructorDecl(Parser* parser)
        {
            RefPtr<ConstructorDecl> decl = new ConstructorDecl();
            parser->FillPosition(decl.Ptr());
            parser->ReadToken("__init");

            parseParameterList(parser, decl);

            if( AdvanceIf(parser, TokenType::Semicolon) )
            {
                // empty body
            }
            else
            {
                decl->Body = parser->ParseBlockStatement();
            }
            return decl;
        }

        static RefPtr<AccessorDecl> parseAccessorDecl(Parser* parser)
        {
            RefPtr<AccessorDecl> decl;
            if( AdvanceIf(parser, "get") )
            {
                decl = new GetterDecl();
            }
            else if( AdvanceIf(parser, "set") )
            {
                decl = new SetterDecl();
            }
            else
            {
                Unexpected(parser);
                return nullptr;
            }

            if( parser->tokenReader.PeekTokenType() == TokenType::LBrace )
            {
                decl->Body = parser->ParseBlockStatement();
            }
            else
            {
                parser->ReadToken(TokenType::Semicolon);
            }

            return decl;
        }

        static RefPtr<SubscriptDecl> ParseSubscriptDecl(Parser* parser)
        {
            RefPtr<SubscriptDecl> decl = new SubscriptDecl();
            parser->FillPosition(decl.Ptr());
            parser->ReadToken("__subscript");

            // TODO: the use of this name here is a bit magical...
            decl->Name.Content = "operator[]";

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

        // Parse a declaration of a new modifier keyword
        static RefPtr<ModifierDecl> parseModifierDecl(Parser* parser)
        {
            RefPtr<ModifierDecl> decl = new ModifierDecl();

            // read the `__modifier` keyword
            parser->ReadToken(TokenType::Identifier);

            parser->ReadToken(TokenType::LParent);
            decl->classNameToken = parser->ReadToken(TokenType::Identifier);
            parser->ReadToken(TokenType::RParent);

            parser->FillPosition(decl.Ptr());
            decl->Name = parser->ReadToken(TokenType::Identifier);

            parser->ReadToken(TokenType::Semicolon);
            return decl;
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
            AddModifiers(decl.Ptr(), modifiers.first);

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

            // TODO: actual dispatch!
            if (parser->LookAheadToken("struct"))
                decl = ParseDeclaratorDecl(parser, containerDecl);
            else if (parser->LookAheadToken("class"))
                decl = ParseDeclaratorDecl(parser, containerDecl);
            else if (parser->LookAheadToken("typedef"))
                decl = ParseTypeDef(parser);
            else if (parser->LookAheadToken("using"))
                decl = ParseUsing(parser);
            else if (parser->LookAheadToken("cbuffer") || parser->LookAheadToken("tbuffer"))
                decl = ParseHLSLBufferDecl(parser);
            else if (parser->LookAheadToken("__generic"))
                decl = ParseGenericDecl(parser);
            else if (parser->LookAheadToken("__conforms"))
                decl = ParseTraitConformanceDecl(parser);
            else if (parser->LookAheadToken("__extension"))
                decl = ParseExtensionDecl(parser);
            else if (parser->LookAheadToken("__init"))
                decl = ParseConstructorDecl(parser);
            else if (parser->LookAheadToken("__subscript"))
                decl = ParseSubscriptDecl(parser);
            else if (parser->LookAheadToken("__trait"))
                decl = ParseTraitDecl(parser);
            else if(parser->LookAheadToken("__modifier"))
                decl = parseModifierDecl(parser);
            else if (AdvanceIf(parser, TokenType::Semicolon))
            {
                decl = new EmptyDecl();
                decl->Position = loc;
            }
            // GLSL requires that we be able to parse "block" declarations,
            // which look superficially similar to declarator declarations
            else if( parser->LookAheadToken(TokenType::Identifier)
                && parser->LookAheadToken(TokenType::LBrace, 1) )
            {
                decl = parseGLSLBlockDecl(parser, modifiers);
            }
            else
            {
                // Default case: just parse a declarator-based declaration
                decl = ParseDeclaratorDecl(parser, containerDecl);
            }

            if (decl)
            {
                if( auto dd = decl.As<Decl>() )
                {
                    CompleteDecl(parser, dd, containerDecl, modifiers);
                }
                else if(auto declGroup = decl.As<DeclGroup>())
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
            if( auto decl = declBase.As<Decl>() )
            {
                return decl;
            }
            else if( auto declGroup = declBase.As<DeclGroup>() )
            {
                if( declGroup->decls.Count() == 1 )
                {
                    return declGroup->decls[0];
                }
            }

            parser->sink->diagnose(declBase->Position, Diagnostics::unimplemented, "didn't expect multiple declarations here");
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
                TryRecover(parser);
            }
        }

        void Parser::parseSourceFile(ProgramSyntaxNode* program)
        {
            if (outerScope)
            {
                currentScope = outerScope;
            }

            PushScope(program);
            program->Position = CodePosition(0, 0, 0, fileName);
            ParseDeclBody(this, program, TokenType::EndOfFile);
            PopScope();

            assert(currentScope == outerScope);
            currentScope = nullptr;
        }

        RefPtr<ProgramSyntaxNode> Parser::ParseProgram()
        {
            RefPtr<ProgramSyntaxNode> program = new ProgramSyntaxNode();

            parseSourceFile(program.Ptr());

            return program;
        }

        RefPtr<StructSyntaxNode> Parser::ParseStruct()
        {
            RefPtr<StructSyntaxNode> rs = new StructSyntaxNode();
            FillPosition(rs.Ptr());
            ReadToken("struct");
            rs->Name = ReadToken(TokenType::Identifier);
            ReadToken(TokenType::LBrace);
            ParseDeclBody(this, rs.Ptr(), TokenType::RBrace);

            return rs;
        }

        RefPtr<ClassSyntaxNode> Parser::ParseClass()
        {
            RefPtr<ClassSyntaxNode> rs = new ClassSyntaxNode();
            FillPosition(rs.Ptr());
            ReadToken("class");
            rs->Name = ReadToken(TokenType::Identifier);
            ReadToken(TokenType::LBrace);
            ParseDeclBody(this, rs.Ptr(), TokenType::RBrace);
            return rs;
        }

        static RefPtr<StatementSyntaxNode> ParseSwitchStmt(Parser* parser)
        {
            RefPtr<SwitchStmt> stmt = new SwitchStmt();
            parser->FillPosition(stmt.Ptr());
            parser->ReadToken("switch");
            parser->ReadToken(TokenType::LParent);
            stmt->condition = parser->ParseExpression();
            parser->ReadToken(TokenType::RParent);
            stmt->body = parser->ParseBlockStatement();
            return stmt;
        }

        static RefPtr<StatementSyntaxNode> ParseCaseStmt(Parser* parser)
        {
            RefPtr<CaseStmt> stmt = new CaseStmt();
            parser->FillPosition(stmt.Ptr());
            parser->ReadToken("case");
            stmt->expr = parser->ParseExpression();
            parser->ReadToken(TokenType::Colon);
            return stmt;
        }

        static RefPtr<StatementSyntaxNode> ParseDefaultStmt(Parser* parser)
        {
            RefPtr<DefaultStmt> stmt = new DefaultStmt();
            parser->FillPosition(stmt.Ptr());
            parser->ReadToken("default");
            parser->ReadToken(TokenType::Colon);
            return stmt;
        }

        static bool peekTypeName(Parser* parser)
        {
            if(!parser->LookAheadToken(TokenType::Identifier))
                return false;

            auto name = parser->tokenReader.PeekToken().Content;

            auto lookupResult = LookUp(name, parser->currentScope);
            if(!lookupResult.isValid() || lookupResult.isOverloaded())
                return false;

            auto decl = lookupResult.item.declRef.GetDecl();
            if( auto typeDecl = dynamic_cast<AggTypeDecl*>(decl) )
            {
                return true;
            }
            else if( auto typeVarDecl = dynamic_cast<SimpleTypeDecl*>(decl) )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        RefPtr<StatementSyntaxNode> Parser::ParseStatement()
        {
            auto modifiers = ParseModifiers(this);

            RefPtr<StatementSyntaxNode> statement;
            if (LookAheadToken(TokenType::LBrace))
                statement = ParseBlockStatement();
            else if (peekTypeName(this))
                statement = ParseVarDeclrStatement(modifiers);
            else if (LookAheadToken("if"))
                statement = ParseIfStatement();
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
                statement = new DiscardStatementSyntaxNode();
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
            else if (LookAheadToken(TokenType::Identifier))
            {
                // We might be looking at a local declaration, or an
                // expression statement, and we need to figure out which.
                //
                // We'll solve this with backtracking for now.

                Token* startPos = tokenReader.mCursor;

                // Try to parse a type (knowing that the type grammar is
                // a subset of the expression grammar, and so this should
                // always succeed).
                RefPtr<ExpressionSyntaxNode> type = ParseType();
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
                    tokenReader.mCursor = startPos;
                    statement = ParseVarDeclrStatement(modifiers);
                    return statement;
                }

                // Fallback: reset and parse an expression
                tokenReader.mCursor = startPos;
                statement = ParseExpressionStatement();
            }
            else if (LookAheadToken(TokenType::Semicolon))
            {
                statement = new EmptyStatementSyntaxNode();
                FillPosition(statement.Ptr());
                ReadToken(TokenType::Semicolon);
            }
            else
            {
                // Default case should always fall back to parsing an expression,
                // and then let that detect any errors
                statement = ParseExpressionStatement();
            }

            if (statement)
            {
                // Install any modifiers onto the statement.
                // Note: this path is bypassed in the case of a
                // declaration statement, so we don't end up
                // doubling up the modifiers.
                statement->modifiers = modifiers;
            }

            return statement;
        }

        RefPtr<StatementSyntaxNode> Parser::ParseBlockStatement()
        {
            if( options.flags & SLANG_COMPILE_FLAG_NO_CHECKING )
            {
                // We have been asked to parse the input, but not attempt to understand it.

                // TODO: record start/end locations...

                List<Token> tokens;

                ReadToken(TokenType::LBrace);

                int depth = 1;
                for( ;;)
                {
                    switch( tokenReader.PeekTokenType() )
                    {
                    case TokenType::EndOfFile:
                        goto done;

                    case TokenType::RBrace:
                        depth--;
                        if(depth == 0)
                            goto done;
                        break;

                    case TokenType::LBrace:
                        depth++;
                        break;

                    default:
                        break;
                    }

                    auto token = tokenReader.AdvanceToken();
                    tokens.Add(token);
                }
            done:
                ReadToken(TokenType::RBrace);

                RefPtr<UnparsedStmt> unparsedStmt = new UnparsedStmt();
                unparsedStmt->tokens = tokens;
                return unparsedStmt;
            }


            RefPtr<ScopeDecl> scopeDecl = new ScopeDecl();
            RefPtr<BlockStatementSyntaxNode> blockStatement = new BlockStatementSyntaxNode();
            blockStatement->scopeDecl = scopeDecl;
            PushScope(scopeDecl.Ptr());
            ReadToken(TokenType::LBrace);
            if(!tokenReader.IsAtEnd())
            {
                FillPosition(blockStatement.Ptr());
            }
            while (!AdvanceIfMatch(this, TokenType::RBrace))
            {
                auto stmt = ParseStatement();
                if(stmt)
                {
                    blockStatement->Statements.Add(stmt);
                }
                TryRecover(this);
            }
            PopScope();
            return blockStatement;
        }

        RefPtr<VarDeclrStatementSyntaxNode> Parser::ParseVarDeclrStatement(
            Modifiers modifiers)
        {
            RefPtr<VarDeclrStatementSyntaxNode>varDeclrStatement = new VarDeclrStatementSyntaxNode();
        
            FillPosition(varDeclrStatement.Ptr());
            auto decl = ParseDeclWithModifiers(this, currentScope->containerDecl, modifiers);
            varDeclrStatement->decl = decl;
            return varDeclrStatement;
        }

        RefPtr<IfStatementSyntaxNode> Parser::ParseIfStatement()
        {
            RefPtr<IfStatementSyntaxNode> ifStatement = new IfStatementSyntaxNode();
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

        RefPtr<ForStatementSyntaxNode> Parser::ParseForStatement()
        {
            RefPtr<ScopeDecl> scopeDecl = new ScopeDecl();
            RefPtr<ForStatementSyntaxNode> stmt = new ForStatementSyntaxNode();
            stmt->scopeDecl = scopeDecl;

            // Note(tfoley): HLSL implements `for` with incorrect scoping.
            // We need an option to turn on this behavior in a kind of "legacy" mode
//			PushScope(scopeDecl.Ptr());
            FillPosition(stmt.Ptr());
            ReadToken("for");
            ReadToken(TokenType::LParent);
            if (peekTypeName(this))
            {
                stmt->InitialStatement = ParseVarDeclrStatement(Modifiers());
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
//			PopScope();
            return stmt;
        }

        RefPtr<WhileStatementSyntaxNode> Parser::ParseWhileStatement()
        {
            RefPtr<WhileStatementSyntaxNode> whileStatement = new WhileStatementSyntaxNode();
            FillPosition(whileStatement.Ptr());
            ReadToken("while");
            ReadToken(TokenType::LParent);
            whileStatement->Predicate = ParseExpression();
            ReadToken(TokenType::RParent);
            whileStatement->Statement = ParseStatement();
            return whileStatement;
        }

        RefPtr<DoWhileStatementSyntaxNode> Parser::ParseDoWhileStatement()
        {
            RefPtr<DoWhileStatementSyntaxNode> doWhileStatement = new DoWhileStatementSyntaxNode();
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

        RefPtr<BreakStatementSyntaxNode> Parser::ParseBreakStatement()
        {
            RefPtr<BreakStatementSyntaxNode> breakStatement = new BreakStatementSyntaxNode();
            FillPosition(breakStatement.Ptr());
            ReadToken("break");
            ReadToken(TokenType::Semicolon);
            return breakStatement;
        }

        RefPtr<ContinueStatementSyntaxNode>	Parser::ParseContinueStatement()
        {
            RefPtr<ContinueStatementSyntaxNode> continueStatement = new ContinueStatementSyntaxNode();
            FillPosition(continueStatement.Ptr());
            ReadToken("continue");
            ReadToken(TokenType::Semicolon);
            return continueStatement;
        }

        RefPtr<ReturnStatementSyntaxNode> Parser::ParseReturnStatement()
        {
            RefPtr<ReturnStatementSyntaxNode> returnStatement = new ReturnStatementSyntaxNode();
            FillPosition(returnStatement.Ptr());
            ReadToken("return");
            if (!LookAheadToken(TokenType::Semicolon))
                returnStatement->Expression = ParseExpression();
            ReadToken(TokenType::Semicolon);
            return returnStatement;
        }

        RefPtr<ExpressionStatementSyntaxNode> Parser::ParseExpressionStatement()
        {
            RefPtr<ExpressionStatementSyntaxNode> statement = new ExpressionStatementSyntaxNode();
            
            FillPosition(statement.Ptr());
            statement->Expression = ParseExpression();
            
            ReadToken(TokenType::Semicolon);
            return statement;
        }

        RefPtr<ParameterSyntaxNode> Parser::ParseParameter()
        {
            RefPtr<ParameterSyntaxNode> parameter = new ParameterSyntaxNode();
            parameter->modifiers = ParseModifiers(this);

            DeclaratorInfo declaratorInfo;
            declaratorInfo.typeSpec = ParseType();

            InitDeclarator initDeclarator = ParseInitDeclarator(this);
            UnwrapDeclarator(initDeclarator, &declaratorInfo);

            // Assume it is a variable-like declarator
            CompleteVarDecl(this, parameter, declaratorInfo);
            return parameter;
        }

        RefPtr<ExpressionSyntaxNode> Parser::ParseType()
        {
            auto typeSpec = parseTypeSpec(this);
            if( typeSpec.decl )
            {
                AddMember(currentScope, typeSpec.decl);
            }
            auto typeExpr = typeSpec.expr;

            while (LookAheadToken(TokenType::LBracket))
            {
                RefPtr<IndexExpressionSyntaxNode> arrType = new IndexExpressionSyntaxNode();
                arrType->Position = typeExpr->Position;
                arrType->BaseExpression = typeExpr;
                ReadToken(TokenType::LBracket);
                if (!LookAheadToken(TokenType::RBracket))
                {
                    arrType->IndexExpression = ParseExpression();
                }
                ReadToken(TokenType::RBracket);
                typeExpr = arrType;
            }

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
            case TokenType::OpLeq:
            case TokenType::OpLess:
                return Precedence::RelationalComparison;
            case TokenType::OpRsh:
                // Don't allow this op inside a generic argument
                if (parser->genericDepth > 0) return Precedence::Invalid;
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

        Operator GetOpFromToken(Token & token)
        {
            switch(token.Type)
            {
            case TokenType::Comma:
                return Operator::Sequence;
            case TokenType::OpAssign:
                return Operator::Assign;
            case TokenType::OpAddAssign:
                return Operator::AddAssign;
            case TokenType::OpSubAssign:
                return Operator::SubAssign;
            case TokenType::OpMulAssign:
                return Operator::MulAssign;
            case TokenType::OpDivAssign:
                return Operator::DivAssign;
            case TokenType::OpModAssign:
                return Operator::ModAssign;
            case TokenType::OpShlAssign:
                return Operator::LshAssign;
            case TokenType::OpShrAssign:
                return Operator::RshAssign;
            case TokenType::OpOrAssign:
                return Operator::OrAssign;
            case TokenType::OpAndAssign:
                return Operator::AddAssign;
            case TokenType::OpXorAssign:
                return Operator::XorAssign;
            case TokenType::OpOr:
                return Operator::Or;
            case TokenType::OpAnd:
                return Operator::And;
            case TokenType::OpBitOr:
                return Operator::BitOr;
            case TokenType::OpBitXor:
                return Operator::BitXor;
            case TokenType::OpBitAnd:
                return Operator::BitAnd;
            case TokenType::OpEql:
                return Operator::Eql;
            case TokenType::OpNeq:
                return Operator::Neq;
            case TokenType::OpGeq:
                return Operator::Geq;
            case TokenType::OpLeq:
                return Operator::Leq;
            case TokenType::OpGreater:
                return Operator::Greater;
            case TokenType::OpLess:
                return Operator::Less;
            case TokenType::OpLsh:
                return Operator::Lsh;
            case TokenType::OpRsh:
                return Operator::Rsh;
            case TokenType::OpAdd:
                return Operator::Add;
            case TokenType::OpSub:
                return Operator::Sub;
            case TokenType::OpMul:
                return Operator::Mul;
            case TokenType::OpDiv:
                return Operator::Div;
            case TokenType::OpMod:
                return Operator::Mod;
            case TokenType::OpInc:
                return Operator::PostInc;
            case TokenType::OpDec:
                return Operator::PostDec;
            case TokenType::OpNot:
                return Operator::Not;
            case TokenType::OpBitNot:
                return Operator::BitNot;
            default:
                throw "Illegal TokenType.";
            }
        }

        static RefPtr<ExpressionSyntaxNode> parseOperator(Parser* parser)
        {
            Token opToken;
            switch(parser->tokenReader.PeekTokenType())
            {
            case TokenType::QuestionMark:
                opToken = parser->ReadToken();
                opToken.Content = "?:";
                break;

            default:
                opToken = parser->ReadToken();
                break;
            }

            auto opExpr = new VarExpressionSyntaxNode();
            opExpr->Variable = opToken.Content;
            opExpr->scope = parser->currentScope;
            opExpr->Position = opToken.Position;

            return opExpr;

        }

        RefPtr<ExpressionSyntaxNode> Parser::ParseExpression(Precedence level)
        {
            if (level == Precedence::Prefix)
                return ParseLeafExpression();
            if (level == Precedence::TernaryConditional)
            {
                // parse select clause
                auto condition = ParseExpression(Precedence(level + 1));
                if (LookAheadToken(TokenType::QuestionMark))
                {
                    RefPtr<SelectExpressionSyntaxNode> select = new SelectExpressionSyntaxNode();
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
                        RefPtr<OperatorExpressionSyntaxNode> tmp = new InfixExpr();
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
                        RefPtr<OperatorExpressionSyntaxNode> tmp = new InfixExpr();
                        tmp->Arguments.Add(left);
                        FillPosition(tmp.Ptr());
                        tmp->FunctionExpr = parseOperator(this);
                        tmp->Arguments.Add(ParseExpression(level));
                        left = tmp;
                    }
                    return left;
                }
            }
        }

        RefPtr<ExpressionSyntaxNode> Parser::ParseLeafExpression()
        {
            RefPtr<ExpressionSyntaxNode> rs;
            if (LookAheadToken(TokenType::OpInc) ||
                LookAheadToken(TokenType::OpDec) ||
                LookAheadToken(TokenType::OpNot) ||
                LookAheadToken(TokenType::OpBitNot) ||
                LookAheadToken(TokenType::OpSub))
            {
                RefPtr<OperatorExpressionSyntaxNode> unaryExpr = new PrefixExpr();
                FillPosition(unaryExpr.Ptr());
                unaryExpr->FunctionExpr = parseOperator(this);
                unaryExpr->Arguments.Add(ParseLeafExpression());
                rs = unaryExpr;
                return rs;
            }

            if (LookAheadToken(TokenType::LParent))
            {
                ReadToken(TokenType::LParent);
                RefPtr<ExpressionSyntaxNode> expr;
                if (peekTypeName(this) && LookAheadToken(TokenType::RParent, 1))
                {
                    RefPtr<TypeCastExpressionSyntaxNode> tcexpr = new TypeCastExpressionSyntaxNode();
                    FillPosition(tcexpr.Ptr());
                    tcexpr->TargetType = ParseTypeExp();
                    ReadToken(TokenType::RParent);
                    tcexpr->Expression = ParseExpression(Precedence::Multiplicative); // Note(tfoley): need to double-check this
                    expr = tcexpr;
                }
                else
                {
                    expr = ParseExpression();
                    ReadToken(TokenType::RParent);
                }
                rs = expr;
            }
            else if( LookAheadToken(TokenType::LBrace) )
            {
                RefPtr<InitializerListExpr> initExpr = new InitializerListExpr();
                FillPosition(initExpr.Ptr());

                // Initializer list
                ReadToken(TokenType::LBrace);

                List<RefPtr<ExpressionSyntaxNode>> exprs;

                for(;;)
                {
                    if(AdvanceIfMatch(this, TokenType::RBrace))
                        break;

                    auto expr = ParseArgExpr();
                    if( expr )
                    {
                        initExpr->args.Add(expr);
                    }

                    if(AdvanceIfMatch(this, TokenType::RBrace))
                        break;

                    ReadToken(TokenType::Comma);
                }
                rs = initExpr;
            }

            else if (LookAheadToken(TokenType::IntLiterial) ||
                LookAheadToken(TokenType::DoubleLiterial))
            {
                RefPtr<ConstantExpressionSyntaxNode> constExpr = new ConstantExpressionSyntaxNode();
                auto token = tokenReader.AdvanceToken();
                FillPosition(constExpr.Ptr());
                if (token.Type == TokenType::IntLiterial)
                {
                    constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Int;
                    constExpr->IntValue = StringToInt(token.Content);
                }
                else if (token.Type == TokenType::DoubleLiterial)
                {
                    constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Float;
                    constExpr->FloatValue = (FloatingPointLiteralValue) StringToDouble(token.Content);
                }
                rs = constExpr;
            }
            else if (LookAheadToken("true") || LookAheadToken("false"))
            {
                RefPtr<ConstantExpressionSyntaxNode> constExpr = new ConstantExpressionSyntaxNode();
                auto token = tokenReader.AdvanceToken();
                FillPosition(constExpr.Ptr());
                constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Bool;
                constExpr->IntValue = token.Content == "true" ? 1 : 0;
                rs = constExpr;
            }
            else if (LookAheadToken(TokenType::Identifier))
            {
                RefPtr<VarExpressionSyntaxNode> varExpr = new VarExpressionSyntaxNode();
                varExpr->scope = currentScope.Ptr();
                FillPosition(varExpr.Ptr());
                auto token = ReadToken(TokenType::Identifier);
                varExpr->Variable = token.Content;
                rs = varExpr;
            }

            while (!tokenReader.IsAtEnd() &&
                (LookAheadToken(TokenType::OpInc) ||
                LookAheadToken(TokenType::OpDec) ||
                LookAheadToken(TokenType::Dot) ||
                LookAheadToken(TokenType::LBracket) ||
                LookAheadToken(TokenType::LParent)))
            {
                if (LookAheadToken(TokenType::OpInc))
                {
                    RefPtr<OperatorExpressionSyntaxNode> unaryExpr = new PostfixExpr();
                    FillPosition(unaryExpr.Ptr());
                    unaryExpr->FunctionExpr = parseOperator(this);
                    unaryExpr->Arguments.Add(rs);
                    rs = unaryExpr;
                }
                else if (LookAheadToken(TokenType::OpDec))
                {
                    RefPtr<OperatorExpressionSyntaxNode> unaryExpr = new PostfixExpr();
                    FillPosition(unaryExpr.Ptr());
                    unaryExpr->FunctionExpr = parseOperator(this);
                    unaryExpr->Arguments.Add(rs);
                    rs = unaryExpr;
                }
                else if (LookAheadToken(TokenType::LBracket))
                {
                    RefPtr<IndexExpressionSyntaxNode> indexExpr = new IndexExpressionSyntaxNode();
                    indexExpr->BaseExpression = rs;
                    FillPosition(indexExpr.Ptr());
                    ReadToken(TokenType::LBracket);
                    indexExpr->IndexExpression = ParseExpression();
                    ReadToken(TokenType::RBracket);
                    rs = indexExpr;
                }
                else if (LookAheadToken(TokenType::LParent))
                {
                    RefPtr<InvokeExpressionSyntaxNode> invokeExpr = new InvokeExpressionSyntaxNode();
                    invokeExpr->FunctionExpr = rs;
                    FillPosition(invokeExpr.Ptr());
                    ReadToken(TokenType::LParent);
                    while (!tokenReader.IsAtEnd())
                    {
                        if (!LookAheadToken(TokenType::RParent))
                            invokeExpr->Arguments.Add(ParseArgExpr());
                        else
                        {
                            break;
                        }
                        if (!LookAheadToken(TokenType::Comma))
                            break;
                        ReadToken(TokenType::Comma);
                    }
                    ReadToken(TokenType::RParent);
                    rs = invokeExpr;
                }
                else if (LookAheadToken(TokenType::Dot))
                {
                    RefPtr<MemberExpressionSyntaxNode> memberExpr = new MemberExpressionSyntaxNode();
                    memberExpr->scope = currentScope.Ptr();
                    FillPosition(memberExpr.Ptr());
                    memberExpr->BaseExpression = rs;
                    ReadToken(TokenType::Dot); 
                    memberExpr->MemberName = ReadToken(TokenType::Identifier).Content;
                    rs = memberExpr;
                }
            }
            if (!rs)
            {
                sink->diagnose(tokenReader.PeekLoc(), Diagnostics::syntaxError);
            }
            return rs;
        }

                // Parse a source file into an existing translation unit
        void parseSourceFile(
            ProgramSyntaxNode*  translationUnitSyntax,
            CompileOptions&     options,
            TokenSpan const&    tokens,
            DiagnosticSink*     sink,
            String const&       fileName,
            RefPtr<Scope> const&outerScope)
        {
            Parser parser(options, tokens, sink, fileName, outerScope);
            return parser.parseSourceFile(translationUnitSyntax);
        }
    }
}