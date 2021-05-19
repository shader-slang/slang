#include "parser.h"

#include "options.h"
#include "identifier-lookup.h"

#include "../../source/compiler-core/slang-name-convention-util.h"

#include "../../source/core/slang-io.h"

namespace CppExtract {
using namespace Slang;

// If fails then we need more bits to identify types
SLANG_COMPILE_TIME_ASSERT(int(Node::Type::CountOf) <= 8 * sizeof(uint32_t));

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Parser !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Parser::Parser(NodeTree* nodeTree, DiagnosticSink* sink) :
    m_sink(sink),
    m_nodeTree(nodeTree),
    m_nodeTypeEnabled(0)
{
    // Enable types by default
    const Node::Type defaultEnabled[] =
    {
        Node::Type::ClassType,
        Node::Type::StructType,
        Node::Type::Namespace,
        Node::Type::AnonymousNamespace,
        Node::Type::Field,
    };
    setTypesEnabled(defaultEnabled, SLANG_COUNT_OF(defaultEnabled));
}

void Parser::setTypeEnabled(Node::Type type, bool isEnabled )
{
    if (isEnabled)
    {
        m_nodeTypeEnabled |= (NodeTypeBitType(1) << int(type));
    }
    else
    {
        m_nodeTypeEnabled &= ~(NodeTypeBitType(1) << int(type));
    }
}

void Parser::setTypesEnabled(const Node::Type* types, Index typesCount, bool isEnabled)
{
    for (Index i = 0; i < typesCount; ++i)
    {
        setTypeEnabled(types[i], isEnabled);
    }
}

bool Parser::_isMarker(const UnownedStringSlice& name)
{
    return name.startsWith(m_options->m_markPrefix.getUnownedSlice()) && name.endsWith(m_options->m_markSuffix.getUnownedSlice());
}

SlangResult Parser::expect(TokenType type, Token* outToken)
{
    if (m_reader.peekTokenType() != type)
    {
        m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::expectingToken, type);
        return SLANG_FAIL;
    }

    if (outToken)
    {
        *outToken = m_reader.advanceToken();
    }
    else
    {
        m_reader.advanceToken();
    }
    return SLANG_OK;
}

bool Parser::advanceIfToken(TokenType type, Token* outToken)
{
    if (m_reader.peekTokenType() == type)
    {
        Token token = m_reader.advanceToken();
        if (outToken)
        {
            *outToken = token;
        }
        return true;
    }
    return false;
}

bool Parser::advanceIfMarker(Token* outToken)
{
    const Token peekToken = m_reader.peekToken();
    if (peekToken.type == TokenType::Identifier && _isMarker(peekToken.getContent()))
    {
        m_reader.advanceToken();
        if (outToken)
        {
            *outToken = peekToken;
        }
        return true;
    }
    return false;
}

bool Parser::advanceIfStyle(IdentifierStyle style, Token* outToken)
{
    if (m_reader.peekTokenType() == TokenType::Identifier)
    {
        IdentifierStyle readStyle = m_nodeTree->m_identifierLookup->get(m_reader.peekToken().getContent());
        if (readStyle == style)
        {
            Token token = m_reader.advanceToken();
            if (outToken)
            {
                *outToken = token;
            }
            return true;
        }
    }
    return false;
}


SlangResult Parser::pushAnonymousNamespace()
{
    m_currentScope = m_currentScope->getAnonymousNamespace();

    if (m_sourceOrigin)
    {
        m_sourceOrigin->addNode(m_currentScope);
    }

    return SLANG_OK;
}

SlangResult Parser::pushScope(ScopeNode* scopeNode)
{
    if (m_sourceOrigin)
    {
        m_sourceOrigin->addNode(scopeNode);
    }

    if (scopeNode->m_name.hasContent())
    {
        // For anonymous namespace, we should look if we already have one and just reopen that. Doing so will mean will
        // find anonymous namespace clashes

        if (Node* foundNode = m_currentScope->findChild(scopeNode->m_name.getContent()))
        {
            if (scopeNode->isClassLike())
            {
                m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::typeAlreadyDeclared, scopeNode->m_name.getContent());
                m_sink->diagnose(foundNode->m_name, CPPDiagnostics::seeDeclarationOf, scopeNode->m_name.getContent());
                return SLANG_FAIL;
            }

            if (foundNode->m_type == Node::Type::Namespace)
            {
                if (foundNode->m_type != scopeNode->m_type)
                {
                    // Different types can't work
                    m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::typeAlreadyDeclared, scopeNode->m_name.getContent());
                    return SLANG_FAIL;
                }

                ScopeNode* foundScopeNode = as<ScopeNode>(foundNode);
                SLANG_ASSERT(foundScopeNode);

                // Make sure the node is empty, as we are *not* going to add it, we are just going to use
                // the pre-existing namespace
                SLANG_ASSERT(scopeNode->m_children.getCount() == 0);

                // We can just use the pre-existing namespace
                m_currentScope = foundScopeNode;
                return SLANG_OK;
            }
        }
    }

    m_currentScope->addChild(scopeNode);
    m_currentScope = scopeNode;
    return SLANG_OK;
}

SlangResult Parser::popScope()
{
    if (m_currentScope->m_parentScope == nullptr)
    {
        m_sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::scopeNotClosed);
        return SLANG_FAIL;
    }

    m_currentScope = m_currentScope->m_parentScope;
    return SLANG_OK;
}

SlangResult Parser::_maybeConsumeScope()
{
    // Look for either ; or { to open scope
    while (true)
    {
        const TokenType type = m_reader.peekTokenType();
        if (type == TokenType::Semicolon)
        {
            m_reader.advanceToken();
            return SLANG_OK;
        }
        else if (type == TokenType::LBrace)
        {
            m_reader.advanceToken();
            return consumeToClosingBrace();
        }
        else if (type == TokenType::EndOfFile)
        {
            return SLANG_OK;
        }

        m_reader.advanceToken();
    }
}

SlangResult Parser::consumeToClosingBrace(const Token* inOpenBraceToken)
{
    Token openToken;
    if (inOpenBraceToken)
    {
        openToken = *inOpenBraceToken;
    }
    else
    {
        openToken = m_reader.advanceToken();
    }

    while (true)
    {
        switch (m_reader.peekTokenType())
        {
            case TokenType::EndOfFile:
            {
                m_sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::didntFindMatchingBrace);
                m_sink->diagnose(openToken, CPPDiagnostics::seeOpen);
                return SLANG_FAIL;
            }
            case TokenType::LBrace:
            {
                SLANG_RETURN_ON_FAIL(consumeToClosingBrace());
                break;
            }
            case TokenType::RBrace:
            {
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                m_reader.advanceToken();
                break;
            }
        }
    }
}


SlangResult Parser::_parseEnum()
{
    // We are looking for
    // enum ([class name] | [name]) [: base] ( { | ; )

    Token enumToken;

    // consume enum
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &enumToken));

    if (!m_currentScope->acceptsTypes())
    {
        m_sink->diagnose(enumToken.loc, CPPDiagnostics::cannotDeclareTypeInScope);
        return SLANG_FAIL;
    }

    Node::Type type = Node::Type::Enum;

    Token nameToken;
    if (advanceIfToken(TokenType::Identifier, &nameToken))
    {
        const IdentifierStyle style = m_nodeTree->m_identifierLookup->get(nameToken.getContent());

        if (style == IdentifierStyle::Class)
        {
            type = Node::Type::EnumClass;
            SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &nameToken));
        }
        else if (style == IdentifierStyle::None)
        {
            // It holds the name then
        }
        else
        {
            m_sink->diagnose(nameToken.loc, CPPDiagnostics::expectingIdentifier, nameToken.getContent());
            return SLANG_FAIL;
        }
    }

    RefPtr<EnumNode> node = new EnumNode(type);
    node->m_name = nameToken;

    if (advanceIfToken(TokenType::Colon))
    {
        // We may have tokens up to { or ;
        List<Token> backingTokens;

        while (true)
        {
            TokenType tokenType = m_reader.peekTokenType();
            if (tokenType == TokenType::Semicolon ||
                tokenType == TokenType::LBrace ||
                tokenType == TokenType::EndOfFile)
            {
                break;
            }

            backingTokens.add(m_reader.advanceToken());
        }

        // TODO - Look up the backing type. It can only be an integral. We can assume it must be defined before lookup
        // for our uses here.
        // If we can't find the type, we could assume it's size is undefined

        if (backingTokens.getCount() == 1)
        {
            node->m_backingToken = backingTokens[0];
        }
    }

    pushScope(node);

    if (advanceIfToken(TokenType::Semicolon))
    {
        if (nameToken.type != TokenType::Invalid)
        {
            Node* node = m_currentScope->findChild(nameToken.getContent());
            if (node)
            {
                // Strictly speaking we should check the backing type etc, match, but for now ignore and assume it's ok

                if (node->m_type == type)
                {
                    return SLANG_OK;
                }
                m_sink->diagnose(nameToken.loc, CPPDiagnostics::typeAlreadyDeclared, nameToken.getContent());
                return SLANG_FAIL;
            }
            return popScope();
        }
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::LBrace));

    while (true)
    {
        TokenType tokenType = m_reader.peekTokenType();
        if (tokenType == TokenType::RBrace)
        {
            break;
        }

        RefPtr<EnumCaseNode> caseNode(new EnumCaseNode);

        // We could also check if the name is a valid identifier for name, for now just assume.
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &caseNode->m_name));

        if (node->findChild(caseNode->m_name.getContent()))
        {
            m_sink->diagnose(caseNode->m_name.loc, CPPDiagnostics::identifierAlreadyDefined, caseNode->m_name.getContent());
            return SLANG_FAIL;
        }

        // Add the value
        node->addChild(caseNode);

        // TODO(JS):
        // This could be better. We could lookup the value etc. For now just assume only one token is valid.
        
        if (advanceIfToken(TokenType::OpAssign))
        {
            List<Token> valueTokens;
            // Consume up to } or ,
            while (true)
            {
                TokenType assignType = m_reader.peekTokenType();

                if (assignType == TokenType::Comma ||
                    assignType == TokenType::RBrace ||
                    assignType == TokenType::EndOfFile)
                {
                    break;
                }
                valueTokens.add(m_reader.advanceToken());
            }

            if (valueTokens.getCount() == 1)
            {
                caseNode->m_value = valueTokens[0];
            }
        }

        tokenType = m_reader.peekTokenType();
        if (tokenType == TokenType::Comma)
        {
            m_reader.advanceToken();
            continue;
        }
    
        break;
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::RBrace));
    SLANG_RETURN_ON_FAIL(expect(TokenType::Semicolon));

    return popScope();
}

SlangResult Parser::_maybeParseNode(Node::Type type)
{
    // We are looking for
    // struct/class identifier [: [public|private|protected] Identifier ] { [public|private|proctected:]* marker ( identifier );

    if (type == Node::Type::Namespace)
    {
        // consume namespace
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier));

        Token name;
        if (advanceIfToken(TokenType::LBrace))
        {
            return pushAnonymousNamespace();
        }
        else if (advanceIfToken(TokenType::Identifier, &name))
        {
            if (advanceIfToken(TokenType::LBrace))
            {
                // Okay looks like we are opening a namespace
                RefPtr<ScopeNode> node(new ScopeNode(Node::Type::Namespace));
                node->m_name = name;
                // Push the node
                return pushScope(node);
            }
        }

        // Just ignore it then
        return SLANG_OK;
    }
    else if (Node::isEnumLikeType(type))
    {
        return _parseEnum();
    }

    // Must be class | struct

    SLANG_ASSERT(type == Node::Type::ClassType || type == Node::Type::StructType);

    Token name;

    // consume class | struct
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier));
    // Next is the class name
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &name));

    if (m_reader.peekTokenType() == TokenType::Semicolon)
    {
        // pre declaration;
        return SLANG_OK;
    }

    RefPtr<ClassLikeNode> node(new ClassLikeNode(type));
    node->m_name = name;

    // Defaults to not reflected
    SLANG_ASSERT(!node->isReflected());

    if (advanceIfToken(TokenType::Colon))
    {
        // Could have public
        advanceIfStyle(IdentifierStyle::Access);

        if (!advanceIfToken(TokenType::Identifier, &node->m_super))
        {
            return SLANG_OK;
        }
    }

    if (m_reader.peekTokenType() != TokenType::LBrace)
    {
        // Consume up until we see a brace else it's an error
        while (true)
        {
            const TokenType peekTokenType = m_reader.peekTokenType();
            if (peekTokenType == TokenType::EndOfFile)
            {
                // Expecting brace
                m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::expectingToken, TokenType::LBrace);
                return SLANG_FAIL;
            }
            else if (peekTokenType == TokenType::LBrace)
            {
                break;
            }
            m_reader.advanceToken();
        }

        return pushScope(node);
    }

    Token braceToken = m_reader.advanceToken();

    while (true)
    {
        // Okay now we are looking for the markers, or visibility qualifiers
        if (advanceIfStyle(IdentifierStyle::Access))
        {
            // Consume it and a colon
            if (SLANG_FAILED(expect(TokenType::Colon)))
            {
                consumeToClosingBrace(&braceToken);
                return SLANG_OK;
            }
            continue;
        }

        switch (m_reader.peekTokenType())
        {
            case TokenType::Identifier:  break;
            case TokenType::RBrace:
            {
                SLANG_RETURN_ON_FAIL(pushScope(node));
                SLANG_RETURN_ON_FAIL(popScope());
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                SLANG_RETURN_ON_FAIL(pushScope(node));
                return SLANG_OK;
            }
        }

        // If it's one of the markers, then we continue to extract parameter
        if (advanceIfMarker(&node->m_marker))
        {
            break;
        }

        // We still need to add the node,
        SLANG_RETURN_ON_FAIL(pushScope(node));
        return SLANG_OK;
    }

    // Let's extract the type set
    {
        UnownedStringSlice slice(node->m_marker.getContent());

        SLANG_ASSERT(_isMarker(slice));

        // Strip the prefix and suffix
        slice = UnownedStringSlice(slice.begin() + m_options->m_markPrefix.getLength(), slice.end() - m_options->m_markSuffix.getLength());

        // Strip ABSTRACT_ if it's there
        UnownedStringSlice abstractSlice("ABSTRACT_");
        if (slice.startsWith(abstractSlice))
        {
            slice = UnownedStringSlice(slice.begin() + abstractSlice.getLength(), slice.end());
        }

        // TODO: We could strip other stuff or have other heuristics there, but this is
        // probably okay for now

        // Set the typeSet 
        node->m_typeSet = m_nodeTree->getOrAddTypeSet(slice);
    }

    // Okay now looking for ( identifier)
    Token typeNameToken;

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeNameToken));
    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    if (typeNameToken.getContent() != node->m_name.getContent())
    {
        m_sink->diagnose(typeNameToken, CPPDiagnostics::typeNameDoesntMatch, node->m_name.getContent());
        return SLANG_FAIL;
    }

    node->m_reflectionType = ReflectionType::Reflected;
    return pushScope(node);
}

SlangResult Parser::_consumeToSync()
{
    while (true)
    {
        TokenType type = m_reader.peekTokenType();

        switch (type)
        {
            case TokenType::Semicolon:
            {
                m_reader.advanceToken();
                return SLANG_OK;
            }
            case TokenType::Pound:
            case TokenType::EndOfFile:
            case TokenType::LBrace:
            case TokenType::RBrace:
            {
                return SLANG_OK;
            }
        }

        m_reader.advanceToken();
    }
}

SlangResult Parser::_maybeParseTemplateArg(Index& ioTemplateDepth)
{
    switch (m_reader.peekTokenType())
    {
        case TokenType::Identifier:
        {
            SLANG_RETURN_ON_FAIL(_maybeParseType(ioTemplateDepth));
            return SLANG_OK;
        }
        case TokenType::IntegerLiteral:
        {
            m_reader.advanceToken();
            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

SlangResult Parser::_maybeParseTemplateArgs(Index& ioTemplateDepth)
{
    if (!advanceIfToken(TokenType::OpLess))
    {
        return SLANG_FAIL;
    }

    ioTemplateDepth++;

    while (true)
    {
        if (ioTemplateDepth == 0)
        {
            return SLANG_OK;
        }

        switch (m_reader.peekTokenType())
        {
            case TokenType::OpGreater:
            {
                if (ioTemplateDepth <= 0)
                {
                    m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::unexpectedTemplateClose);
                    return SLANG_FAIL;
                }
                ioTemplateDepth--;
                m_reader.advanceToken();
                return SLANG_OK;
            }
            case TokenType::OpRsh:
            {
                if (ioTemplateDepth <= 1)
                {
                    m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::unexpectedTemplateClose);
                    return SLANG_FAIL;
                }
                ioTemplateDepth -= 2;
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                while (true)
                {
                    SLANG_RETURN_ON_FAIL(_maybeParseTemplateArg(ioTemplateDepth));

                    if (m_reader.peekTokenType() == TokenType::Comma)
                    {
                        m_reader.advanceToken();
                        // If there is a comma parse another arg
                        continue;
                    }
                    break;
                }
                break;
            }
        }
    }
}

void Parser::_consumeTypeModifiers()
{
    while (advanceIfStyle(IdentifierStyle::TypeModifier));
}

// True if two of these token types of the same type placed immediately after one another 
// produce a different token. Can be conservative, as if not strictly required
// it will just mean more spacing in the output
static bool _canRepeatTokenType(TokenType type)
{
    switch (type)
    {
        case TokenType::OpAdd:
        case TokenType::OpSub:
        case TokenType::OpAnd:
        case TokenType::OpOr:
        case TokenType::OpGreater:
        case TokenType::OpLess:
        case TokenType::Identifier:
        case TokenType::OpAssign:
        case TokenType::Colon:
        {
            return false;
        }
        default: break;
    }
    return true;
}

// Returns true if there needs to be a space between the previous token type, and the current token
// type for correct output. It is assumed that the token stream is appropriate.
// The implementation might need more sophistication, but this at least avoids Blah const *  -> Blahconst* 
static bool _tokenConcatNeedsSpace(TokenType prev, TokenType cur)
{
    if ((cur == TokenType::OpAssign) ||
        (prev == cur && !_canRepeatTokenType(cur)))
    {
        return true;
    }
    return false;
}

UnownedStringSlice Parser::_concatTokens(TokenReader::ParsingCursor start)
{
    auto endCursor = m_reader.getCursor();
    m_reader.setCursor(start);

    TokenType prevTokenType = TokenType::Unknown;

    StringBuilder buf;
    while (!m_reader.isAtCursor(endCursor))
    {
        const Token token = m_reader.advanceToken();
        // Check if we need a space between tokens
        if (_tokenConcatNeedsSpace(prevTokenType, token.type))
        {
            buf << " ";
        }
        buf << token.getContent();

        prevTokenType = token.type;
    }

    StringSlicePool* typePool = m_nodeTree->m_typePool;
    return typePool->getSlice(typePool->add(buf));
}

SlangResult Parser::_maybeParseType(Index& ioTemplateDepth)
{
    _consumeTypeModifiers();

    advanceIfToken(TokenType::Scope);
    while (true)
    {
        Token identifierToken;
        if (!advanceIfToken(TokenType::Identifier, &identifierToken))
        {
            return SLANG_FAIL;
        }

        const IdentifierStyle style = m_nodeTree->m_identifierLookup->get(identifierToken.getContent());
        if (hasFlag(style, IdentifierFlag::Keyword))
        {
            return SLANG_FAIL;
        }

        if (advanceIfToken(TokenType::Scope))
        {
            continue;
        }
        break;
    }

    if (m_reader.peekTokenType() == TokenType::OpLess)
    {
        SLANG_RETURN_ON_FAIL(_maybeParseTemplateArgs(ioTemplateDepth));
    }

    // Strip all the consts etc modifiers
    _consumeTypeModifiers();

    // It's a reference and we are done
    if (advanceIfToken(TokenType::OpBitAnd))
    {
        return SLANG_OK;
    }

    while (true)
    {
        if (advanceIfToken(TokenType::OpMul))
        {
            // Strip all the consts
            _consumeTypeModifiers();
            continue;
        }
        break;
    }

    return SLANG_OK;
}

SlangResult Parser::_maybeParseType(List<Token>& outToks)
{
    auto startCursor = m_reader.getCursor();

    Index templateDepth = 0;
    SlangResult res = _maybeParseType(templateDepth);
    if (SLANG_FAILED(res) && m_sink->getErrorCount())
    {
        return res;
    }

    if (templateDepth != 0)
    {
        m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::unexpectedTemplateClose);
        return SLANG_FAIL;
    }

    auto endCursor = m_reader.getCursor();
    m_reader.setCursor(startCursor);

    while (!m_reader.isAtCursor(endCursor))
    {
        outToks.add(m_reader.advanceToken());
    }

    return SLANG_OK;
}

SlangResult Parser::_maybeParseType(UnownedStringSlice& outType)
{
    auto startCursor = m_reader.getCursor();

    Index templateDepth = 0;
    SlangResult res = _maybeParseType(templateDepth);
    if (SLANG_FAILED(res) && m_sink->getErrorCount())
    {
        return res;
    }

    if (templateDepth != 0)
    {
        m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::unexpectedTemplateClose);
        return SLANG_FAIL;
    }

    // We can build up the out type, from the tokens we found
    outType = _concatTokens(startCursor);
    return SLANG_OK;
}

static bool _isBalancedOpen(TokenType tokenType)
{
    return tokenType == TokenType::LBrace ||
        tokenType == TokenType::LParent ||
        tokenType == TokenType::LBracket;
}

static bool _isBalancedClose(TokenType tokenType)
{
    return tokenType == TokenType::RBrace ||
        tokenType == TokenType::RParent ||
        tokenType == TokenType::RBracket;
}

static TokenType _getBalancedClose(TokenType tokenType)
{
    SLANG_ASSERT(_isBalancedOpen(tokenType));
    switch (tokenType)
    {
        case TokenType::LBrace:         return TokenType::RBrace;
        case TokenType::LParent:        return TokenType::RParent;
        case TokenType::LBracket:       return TokenType::RBracket;
        default:                        return TokenType::Unknown;
    }
}

SlangResult Parser::_parseBalanced(DiagnosticSink* sink)
{
    const TokenType openTokenType = m_reader.peekTokenType();
    if (!_isBalancedOpen(openTokenType))
    {
        return SLANG_FAIL;
    }

    // Save the start token
    const Token startToken = m_reader.advanceToken();
    // Get the token type that would close the open
    const TokenType closeTokenType = _getBalancedClose(openTokenType);

    while (true)
    {
        const TokenType tokenType = m_reader.peekTokenType();

        // If we hit the closing token, we are done
        if (tokenType == closeTokenType)
        {
            m_reader.advanceToken();
            return SLANG_OK;
        }

        // If we hit a balanced open, recurse 
        if (_isBalancedOpen(tokenType))
        {
            SLANG_RETURN_ON_FAIL(_parseBalanced(sink));
            continue;
        }

        // If we hit a close token that doesn't match, then the balancing has gone wrong
        if (_isBalancedClose(tokenType))
        {
            // Only diagnose if required
            if (sink)
            {
                sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::unexpectedUnbalancedToken);
                sink->diagnose(startToken, CPPDiagnostics::seeOpen);
            }
            return SLANG_FAIL;
        }

        // If we hit the end of the file and have not hit the closing token, then
        // somethings gone wrong
        if (tokenType == TokenType::EndOfFile)
        {
            if (sink)
            {
                sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::unexpectedEndOfFile);
                sink->diagnose(startToken, CPPDiagnostics::seeOpen);
            }

            return SLANG_FAIL;
        }

        // Skip the token
        m_reader.advanceToken();
    }
}

SlangResult Parser::_parseTypeDef()
{
    if (!m_currentScope->acceptsTypes())
    {
        m_sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::cannotDeclareTypeInScope);
        return SLANG_FAIL;
    }

    // Consume the typedef
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier));

    // Parse the type
    List<Token> toks;
    SLANG_RETURN_ON_FAIL(_maybeParseType(toks));

    Token name;

    // Get the name
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &name));

    if (Node::lookupNameInScope(m_currentScope, name.getContent()))
    {
        m_sink->diagnose(name.loc, CPPDiagnostics::identifierAlreadyDefined, name.getContent());
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::Semicolon));

    RefPtr<TypeDefNode> node = new TypeDefNode;
    node->m_name = name;

    // Set what aliases too
    node->m_targetTypeTokens.swapWith(toks);

    m_currentScope->addChild(node);

    return SLANG_OK;
}

SlangResult Parser::_maybeParseField()
{
    // Can only add a field if we are in a class
    SLANG_ASSERT(m_currentScope->isClassLike());

    UnownedStringSlice typeName;
    if (SLANG_FAILED(_maybeParseType(typeName)))
    {
        if (m_sink->getErrorCount())
        {
            return SLANG_FAIL;
        }

        _consumeToSync();
        return SLANG_OK;
    }

    if (m_reader.peekTokenType() != TokenType::Identifier)
    {
        _consumeToSync();
        return SLANG_OK;
    }

    Token fieldName = m_reader.advanceToken();

    if (m_reader.peekTokenType() == TokenType::LBracket)
    {
        auto startCursor = m_reader.getCursor();

        // If it's not balanced we just assume it's not correct - and ignore
        if (SLANG_FAILED(_parseBalanced(nullptr)))
        {
            _consumeToSync();
            return SLANG_OK;
        }

        UnownedStringSlice arraySuffix = _concatTokens(startCursor);

        // The overall type is the typename concated with the arraySuffix
        StringBuilder buf;
        buf << typeName << arraySuffix;

        StringSlicePool* typePool = m_nodeTree->m_typePool;

        typeName = typePool->getSlice(typePool->add(buf));
    }

    switch (m_reader.peekTokenType())
    {
        case TokenType::OpAssign:
        {
            // Special case to handle
            // Type operator=(...

            m_reader.advanceToken();
            if (m_reader.peekTokenType() == TokenType::LParent)
            {
                // Not a field
                break;
            }
        }
        case TokenType::Semicolon:
        {
            FieldNode* fieldNode = new FieldNode;

            fieldNode->m_fieldType = typeName;
            fieldNode->m_name = fieldName;
            fieldNode->m_reflectionType = m_currentScope->getContainedReflectionType();

            m_currentScope->addChild(fieldNode);
            break;
        }
        default: break;
    }

    _consumeToSync();
    return SLANG_OK;
}

/* static */Node::Type Parser::_toNodeType(IdentifierStyle style)
{
    switch (style)
    {
        case IdentifierStyle::Class:        return Node::Type::ClassType;
        case IdentifierStyle::Struct:       return Node::Type::StructType;
        case IdentifierStyle::Namespace:    return Node::Type::Namespace;
        case IdentifierStyle::Enum:         return Node::Type::Enum;
        case IdentifierStyle::TypeDef:      return Node::Type::TypeDef;
        default: return Node::Type::Invalid;
    }
}

static UnownedStringSlice _trimUnderscorePrefix(const UnownedStringSlice& slice)
{
    if (slice.getLength() && slice[0] == '_')
    {
        return UnownedStringSlice(slice.begin() + 1, slice.end());
    }
    else
    {
        return slice;
    }
}

SlangResult Parser::_parsePreDeclare()
{
    // Skip the declare type token
    m_reader.advanceToken();

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

    // Get the typeSet
    Token typeSetToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeSetToken));
    TypeSet* typeSet = m_nodeTree->getOrAddTypeSet(typeSetToken.getContent());

    SLANG_RETURN_ON_FAIL(expect(TokenType::Comma));

    // Get the type of type
    Node::Type nodeType;
    {
        Token typeToken;
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeToken));

        const IdentifierStyle style = m_nodeTree->m_identifierLookup->get(typeToken.getContent());

        if (style != IdentifierStyle::Struct && style != IdentifierStyle::Class)
        {
            m_sink->diagnose(typeToken, CPPDiagnostics::expectingTypeKeyword, typeToken.getContent());
            return SLANG_FAIL;
        }
        nodeType = _toNodeType(style);
    }

    Token name;
    Token super;

    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &name));

    if (advanceIfToken(TokenType::Colon))
    {
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &super));
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    switch (nodeType)
    {
        case Node::Type::ClassType:
        case Node::Type::StructType:
        {
            RefPtr<ClassLikeNode> node(new ClassLikeNode(nodeType));

            node->m_name = name;
            node->m_super = super;
            node->m_typeSet = typeSet;

            // Assume it is reflected
            node->m_reflectionType = ReflectionType::Reflected;

            SLANG_RETURN_ON_FAIL(pushScope(node));
            // Pop out of the node
            popScope();
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }


    return SLANG_OK;
}

SlangResult Parser::_parseTypeSet()
{
    // Skip the declare type token
    m_reader.advanceToken();

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

    Token typeSetToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeSetToken));

    TypeSet* typeSet = m_nodeTree->getOrAddTypeSet(typeSetToken.getContent());

    SLANG_RETURN_ON_FAIL(expect(TokenType::Comma));

    // Get the type of type
    Token typeToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeToken));

    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    // Set the typename
    typeSet->m_typeName = typeToken.getContent();

    return SLANG_OK;
}

SlangResult Parser::parse(SourceOrigin* sourceOrigin, const Options* options)
{
    SLANG_ASSERT(options);
    m_options = options;

    // Set the current origin
    m_sourceOrigin = sourceOrigin;

    SourceFile* sourceFile = sourceOrigin->m_sourceFile;

    SourceManager* manager = sourceFile->getSourceManager();

    SourceView* sourceView = manager->createSourceView(sourceFile, nullptr, SourceLoc::fromRaw(0));

    Lexer lexer;

    m_currentScope = m_nodeTree->m_rootNode;

    lexer.initialize(sourceView, m_sink, m_nodeTree->m_namePool, manager->getMemoryArena());
    m_tokenList = lexer.lexAllSemanticTokens();
    // See if there were any errors
    if (m_sink->getErrorCount())
    {
        return SLANG_FAIL;
    }

    m_reader = TokenReader(m_tokenList);

    while (true)
    {
        switch (m_reader.peekTokenType())
        {
            case TokenType::Identifier:
            {
                const IdentifierStyle style = m_nodeTree->m_identifierLookup->get(m_reader.peekToken().getContent());

                switch (style)
                {
                    case IdentifierStyle::PreDeclare:
                    {
                        SLANG_RETURN_ON_FAIL(_parsePreDeclare());
                        break;
                    }
                    case IdentifierStyle::TypeSet:
                    {
                        SLANG_RETURN_ON_FAIL(_parseTypeSet());
                        break;
                    }
                    case IdentifierStyle::Reflected:
                    {
                        m_reader.advanceToken();
                        if (m_currentScope)
                        {
                            m_currentScope->m_reflectionOverride = ReflectionType::Reflected;
                        }
                        break;
                    }
                    case IdentifierStyle::Unreflected:
                    {
                        m_reader.advanceToken();
                        if (m_currentScope)
                        {
                            m_currentScope->m_reflectionOverride = ReflectionType::NotReflected;
                        }
                        break;
                    }
                    case IdentifierStyle::Access:
                    {
                        m_reader.advanceToken();
                        SLANG_RETURN_ON_FAIL(expect(TokenType::Colon));
                        break;
                    }
                    case IdentifierStyle::TypeDef:
                    {
                        if (isTypeEnabled(Node::Type::TypeDef))
                        {
                            SLANG_RETURN_ON_FAIL(_parseTypeDef());
                        }
                        else
                        {
                            m_reader.advanceToken();
                            SLANG_RETURN_ON_FAIL(_consumeToSync());
                        }
                        break;
                    }
                    default:
                    {
                        IdentifierFlags flags = getFlags(style);

                        if (flags & IdentifierFlag::StartScope)
                        {
                            Node::Type type = _toNodeType(style);
                            SLANG_ASSERT(type != Node::Type::Invalid);

                            if (isTypeEnabled(type))
                            {
                                SLANG_RETURN_ON_FAIL(_maybeParseNode(type));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_maybeConsumeScope());
                            }
                        }
                        else
                        {
                            // Special case the node that's the root of the hierarchy (as far as reflection is concerned)
                            // This could be a field
                            if (m_currentScope->acceptsFields())
                            {
                                SLANG_RETURN_ON_FAIL(_maybeParseField());
                            }
                            else
                            {
                                m_reader.advanceToken();
                            }
                        }
                        break;
                    }
                }
                break;
            }
            case TokenType::LBrace:
            {
                SLANG_RETURN_ON_FAIL(consumeToClosingBrace());
                break;
            }
            case TokenType::RBrace:
            {
                SLANG_RETURN_ON_FAIL(popScope());
                m_reader.advanceToken();
                break;
            }
            case TokenType::EndOfFile:
            {
                // Okay we need to confirm that we are in the root node, and with no open braces
                if (m_currentScope != m_nodeTree->getRootNode())
                {
                    m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::braceOpenAtEndOfFile);
                    return SLANG_FAIL;
                }

                return SLANG_OK;
            }
            case TokenType::Pound:
            {
                Token token = m_reader.peekToken();
                if (token.flags & TokenFlag::AtStartOfLine)
                {
                    // We are just going to ignore all of these for now....
                    m_reader.advanceToken();
                    for (;;)
                    {
                        auto t = m_reader.peekToken();
                        if (t.type == TokenType::EndOfFile || (t.flags & TokenFlag::AtStartOfLine))
                        {
                            break;
                        }
                        m_reader.advanceToken();
                    }
                    break;
                }
                // Skip it then
                m_reader.advanceToken();
                break;
            }
            default:
            {
                // Skip it then
                m_reader.advanceToken();
                break;
            }
        }
    }
}

} // namespace CppExtract
