// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-slice-pool.h"
#include "../../source/core/slang-writer.h"

#include "../../source/slang/slang-source-loc.h"
#include "../../source/slang/slang-lexer.h"
#include "../../source/slang/slang-diagnostics.h"
#include "../../source/slang/slang-file-system.h"
#include "../../source/slang/slang-name.h"

#include "slang-cpp-extractor-diagnostics.h"

namespace SlangExperimental
{

using namespace Slang;

enum class IdentifierStyle
{
    None,               ///< It's not an identifier

    Identifier,         ///< Just an identifier
    Marker,             ///< Marker
    Root,
    BaseClass,          ///< Has the name of a base class defined elsewhere

    TypeModifier,       ///< const, volatile etc
    Keyword,            ///< A keyword C/C++ keyword that is not another type
    Class,              ///< class
    Struct,             ///< struct
    Namespace,          ///< namespace
    Access,             ///< public, protected, private

    CountOf,
};

typedef uint32_t IdentifierFlags;
struct IdentifierFlag
{
    enum Enum : IdentifierFlags
    {
        StartScope  = 0x1,          ///< namespace, struct or class
        Class       = 0x2,          ///< Struct or class
        Keyword     = 0x4,
    };
};

static const IdentifierFlags kIdentifierFlags[Index(IdentifierStyle::CountOf)] =
{
    0,              /// None
    0,              /// Identifier 
    0,              /// Marker
    0,              /// Root
    0,              /// BaseClass
    IdentifierFlag::Keyword,              /// TypeModifier
    IdentifierFlag::Keyword,              /// Keyword
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::Class, /// Class
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::Class, /// Struct
    IdentifierFlag::Keyword | IdentifierFlag::StartScope, /// Namespace
    IdentifierFlag::Keyword,
};

SLANG_FORCE_INLINE IdentifierFlags getFlags(IdentifierStyle style)
{
    return kIdentifierFlags[Index(style)];
}

SLANG_FORCE_INLINE bool hasFlag(IdentifierStyle style, IdentifierFlag::Enum flag)
{
    return (getFlags(style) & flag) != 0;
}

class IdentifierLookup
{
public:

    IdentifierStyle get(const UnownedStringSlice& slice) const
    {
        Index index = m_pool.findIndex(slice);
        return (index >= 0) ? m_styles[index] : IdentifierStyle::None;
    }

    void set(const char* name, IdentifierStyle style)
    {
        set(UnownedStringSlice(name), style);
    }

    void set(const UnownedStringSlice& name, IdentifierStyle style)
    {
        StringSlicePool::Handle handle;
        if (m_pool.findOrAdd(name, handle))
        {
            // Add the extra flags
            m_styles[Index(handle)] = style;
        }
        else
        {
            Index index = Index(handle);
            SLANG_ASSERT(index == m_styles.getCount());
            m_styles.add(style);
        }
    }

    void set(const char*const* names, size_t namesCount, IdentifierStyle style)
    {
        for (size_t i = 0; i < namesCount; ++i)
        {
            set(UnownedStringSlice(names[i]), style);
        }
    }
    IdentifierLookup():
        m_pool(StringSlicePool::Style::Empty)
    {
        SLANG_ASSERT(m_pool.getSlicesCount() == 0);
    }
protected:
    List<IdentifierStyle> m_styles;
    StringSlicePool m_pool;
};

class Node : public RefObject
{
public:
    enum class Type
    {
        Invalid,
        StructType,
        ClassType,
        Namespace,
        AnonymousNamespace,
    };

    struct Field
    {
        UnownedStringSlice type;
        Token name;
    };

    bool isClassLike() const { return m_type == Type::StructType || m_type == Type::ClassType; }

    void addChild(Node* child)
    {
        SLANG_ASSERT(child->m_parentScope == nullptr);
        child->m_parentScope = this;
        m_children.add(child);

        if (child->m_type == Type::AnonymousNamespace)
        {
            SLANG_ASSERT(m_anonymousNamespace == nullptr);
            m_anonymousNamespace = child;
        
        }
        else if (child->m_name.Content.getLength())
        {
            m_childMap.Add(child->m_name.Content, child);
        }
    }
    Node* findChild(const UnownedStringSlice& name) const
    {
        Node** nodePtr = m_childMap.TryGetValue(name);
        return (nodePtr) ? *nodePtr : nullptr;
    }

    void addDerived(Node* derived)
    {
        SLANG_ASSERT(derived->m_derivedFrom == nullptr);
        derived->m_derivedFrom = this;
        m_derivedTypes.add(derived);
    }

    bool acceptsFields() const { return isClassLike(); }

    void dump(int indent, StringBuilder& out);
    void dumpDerived(int indentCount, StringBuilder& out);

    void calcName(StringBuilder& outName) const;


    Node(Type type, Node* parent = nullptr):
        m_type(type),
        m_parentScope(parent),
        m_isReflected(false),
        m_derivedFrom(nullptr),
        m_isBaseType(false)
    {
        m_anonymousNamespace = nullptr;
    }

    Type m_type;

    List<RefPtr<Node>> m_children;

    List<RefPtr<Node>> m_derivedTypes;

    Dictionary<UnownedStringSlice, Node*> m_childMap;

    List<Field> m_fields;

    Node* m_anonymousNamespace;

    bool m_isReflected;
    bool m_isBaseType;          ///< The Super type is bothered to be looked up

    Token m_name;
    Token m_super;
    Token m_marker;        

    Node* m_parentScope;
    Node* m_derivedFrom;
};

static void _indent(int indentCount, StringBuilder& out)
{
    for (int i = 0; i < indentCount; ++i)
    {
        out << "  ";
    }
}

void Node::dumpDerived(int indentCount, StringBuilder& out)
{
    if (isClassLike() && m_isReflected && m_name.Content.getLength() > 0)
    {
        _indent(indentCount, out);
        out << m_name.Content << "\n";
    }

    for (Node* derivedType : m_derivedTypes)
    {
        derivedType->dumpDerived(indentCount + 1, out);
    }
}

void Node::dump(int indentCount, StringBuilder& out)
{
    _indent(indentCount, out);

    switch (m_type)
    {
        case Type::AnonymousNamespace:
        {
            out << "namespace {\n";
        }
        case Type::Namespace:
        {
            if (m_name.Content.getLength())
            {
                out << "namespace " << m_name.Content << " {\n";
            }
            else
            {
                out << "{\n";
            }
            break;
        }
        case Type::StructType:
        case Type::ClassType:
        {
            const char* typeName = (m_type == Type::StructType) ? "struct" : "class";
            
            out << typeName << " ";

            if (!m_isReflected)
            {
                out << " (";
            }
            out << m_name.Content;
            if (!m_isReflected)
            {
                out << ") ";
            }

            if (m_super.Content.getLength())
            {
                out << " : " << m_super.Content; 
            }

            out << " {\n";
            break;
        }
    }

    for (Node* child : m_children)
    {
        child->dump(indentCount + 1, out);
    }

    for (const Field& field : m_fields)
    {
        _indent(indentCount + 1, out);
        out << field.type << " " << field.name.Content << "\n";
    }

    _indent(indentCount, out);
    out << "}\n";
}

void Node::calcName(StringBuilder& outName) const
{
    if (m_parentScope == nullptr)
    {
        if (m_name.Content.getLength() == 0)
        {
            return;
        }
        outName << m_name.Content;
    }
    else
    {
        outName << "::";
        if (m_type == Type::AnonymousNamespace)
        {
            outName << "::{Anonymous}";
        }
        else
        {
            outName << m_name.Content;
        }
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CPPExtractor
{
public:
    
    SlangResult expect(TokenType type, Token* outToken = nullptr);

    bool advanceIfToken(TokenType type, Token* outToken = nullptr);
    bool advanceIfStyle(IdentifierStyle style, Token* outToken = nullptr);

    SlangResult pushAnonymousNamespace();
    SlangResult pushNode(Node* node);
    SlangResult consumeToClosingBrace(const Token* openBraceToken = nullptr);
    SlangResult popBrace();

    SlangResult parse(SourceFile* sourceFile);

    SlangResult calcDerivedTypes();

        /// Only valid after calcDerivedTypes has been executed
    const List<Node*>& getBaseTypes() const { return m_baseTypes; }

        /// Get the root node
    Node* getRootNode() const { return m_rootNode; }

    CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup);

protected:
    static Node::Type _toNodeType(IdentifierStyle style);

    SlangResult _maybeParseNode(Node::Type type);
    SlangResult _maybeParseField();

    SlangResult _maybeParseType(UnownedStringSlice& outType);

    SlangResult _maybeParseType(UnownedStringSlice& outType, Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArgs(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArg(Index& ioTemplateDepth);

    SlangResult _calcDerivedTypesRec(Node* node);

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();

    TokenList m_tokenList;
    TokenReader m_reader;

    List<Node*> m_nodeStack;
    Node* m_currentNode;

    RefPtr<Node> m_rootNode;

    List<Node*> m_baseTypes;

    DiagnosticSink* m_sink;

    NamePool* m_namePool;

    IdentifierLookup* m_identifierLookup;
    StringSlicePool* m_typePool;
};

CPPExtractor::CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup):
    m_typePool(typePool),
    m_sink(sink),
    m_namePool(namePool),
    m_identifierLookup(identifierLookup)
{
    m_rootNode = new Node(Node::Type::Namespace);
}

SlangResult CPPExtractor::expect(TokenType type, Token* outToken)
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

bool CPPExtractor::advanceIfToken(TokenType type, Token* outToken)
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

bool CPPExtractor::advanceIfStyle(IdentifierStyle style, Token* outToken)
{
    if (m_reader.peekTokenType() == TokenType::Identifier)
    {
        IdentifierStyle readStyle = m_identifierLookup->get(m_reader.peekToken().Content);
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


SlangResult CPPExtractor::pushAnonymousNamespace()
{
    // See if we find a anonymous namespace node
    if (!m_currentNode->m_anonymousNamespace)
    {
        Node* node = new Node(Node::Type::AnonymousNamespace);
        m_currentNode->addChild(node);
    }
    m_currentNode = m_currentNode->m_anonymousNamespace;

    return SLANG_OK;
}

SlangResult CPPExtractor::pushNode(Node* node)
{
    if (node->m_name.Content.getLength())
    {
        // For anonymous namespace, we should look if we already have one and just reopen that. Doing so will mean will
        // find anonymous namespace clashes

        if (Node* foundNode = m_currentNode->findChild(node->m_name.Content))
        {
            if (node->isClassLike())
            {
                m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::typeAlreadyDeclared, node->m_name.Content);
                m_sink->diagnose(foundNode->m_name, CPPDiagnostics::seeDeclarationOf, node->m_name.Content);
                return SLANG_FAIL;
            }

            if (node->m_type == Node::Type::Namespace)
            {
                // We can just use the pre-existing namespace
                m_currentNode = foundNode;
                return SLANG_OK;
            }
        }
    }

    m_currentNode->addChild(node);
    m_currentNode = node;
    return SLANG_OK;
}

SlangResult CPPExtractor::consumeToClosingBrace(const Token* inOpenBraceToken)
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
                m_sink->diagnose(openToken, CPPDiagnostics::seeOpenBrace);
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

SlangResult CPPExtractor::popBrace()
{
    if (m_currentNode->m_parentScope == nullptr)
    {
        m_sink->diagnose(m_reader.peekLoc(), CPPDiagnostics::scopeNotClosed);
        return SLANG_FAIL;
    }

    m_currentNode = m_currentNode->m_parentScope;
    return SLANG_OK;
}

SlangResult CPPExtractor::_maybeParseNode(Node::Type type)
{
    // We are looking for
    // struct/class identifier [: [public|private|protected] Identifier ] { [public|private|proctected:]* marker ( identifier );

    if (type == Node::Type::Namespace)
    {
        // consume namespace
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier));

        if (m_reader.peekTokenType() == TokenType::LBrace)
        {
            m_reader.advanceToken();
            return pushAnonymousNamespace();
        }
        else if (m_reader.peekTokenType() == TokenType::Identifier)
        {
            Token token = m_reader.advanceToken();

            if (m_reader.peekTokenType() == TokenType::LBrace)
            {
                // Okay looks like we are opening a namespace

                RefPtr<Node> node(new Node(Node::Type::Namespace));
                node->m_name = token;

                // Skip the brace
                m_reader.advanceToken();

                // Push the node
                return pushNode(node);
            }
        }

        // Just ignore it then
        return SLANG_OK;
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

    RefPtr<Node> node(new Node(type, nullptr));
    node->m_name = name;

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

        // Node does define a class, but it's not reflected
        node->m_isReflected = false;
        return pushNode(node);
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
                node->m_isReflected = false;
                SLANG_RETURN_ON_FAIL(pushNode(node));
                SLANG_RETURN_ON_FAIL(popBrace());
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                node->m_isReflected = false;
                SLANG_RETURN_ON_FAIL(pushNode(node));
                return SLANG_OK;
            }
        }
        
        // If it's one of the markers, then we add it
        UnownedStringSlice lexeme = m_reader.peekToken().Content;

        const IdentifierStyle style = m_identifierLookup->get(lexeme);
        if (style != IdentifierStyle::Marker)
        {
            // Looks like a class, but looks like non-reflected
            node->m_isReflected = false;

            // We still need to add the node,
            SLANG_RETURN_ON_FAIL(pushNode(node));
            return SLANG_OK;
        }
        node->m_marker = m_reader.advanceToken();
        break;
    }

    // Okay now looking for ( identifier)
    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

    Token typeNameToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeNameToken));

    if (typeNameToken.Content != node->m_name.Content)
    {
        m_sink->diagnose(typeNameToken, CPPDiagnostics::typeNameDoesntMatch, node->m_name.Content);
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    node->m_isReflected = true;

    return pushNode(node);
}

SlangResult CPPExtractor::_consumeToSync()
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

SlangResult CPPExtractor::_maybeParseTemplateArg(Index& ioTemplateDepth)
{
    switch (m_reader.peekTokenType())
    {
        case TokenType::Identifier:
        {
            UnownedStringSlice name;
            SLANG_RETURN_ON_FAIL(_maybeParseType(name, ioTemplateDepth));
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

SlangResult CPPExtractor::_maybeParseTemplateArgs(Index& ioTemplateDepth)
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

void CPPExtractor::_consumeTypeModifiers()
{
    while (true)
    {
        if (m_reader.peekTokenType() == TokenType::Identifier)
        {
            IdentifierStyle style = m_identifierLookup->get(m_reader.peekToken().Content);
            if (style == IdentifierStyle::TypeModifier)
            {
                m_reader.advanceToken();
                continue;
            }
        }
        break;
    }
}

SlangResult CPPExtractor::_maybeParseType(UnownedStringSlice& outType, Index& ioTemplateDepth)
{
    auto startCursor = m_reader.getCursor();

    _consumeTypeModifiers();

    advanceIfToken(TokenType::Scope);
    while (true)
    {
        Token identifierToken;
        if (!advanceIfToken(TokenType::Identifier, &identifierToken))
        {
            return SLANG_FAIL;
        }

        const IdentifierStyle style = m_identifierLookup->get(identifierToken.Content);
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

    // We can build up the out type, from the tokens we found
    auto endCursor = m_reader.getCursor();

    m_reader.setCursor(startCursor);

    StringBuilder buf;
    while (!m_reader.isAtCursor(endCursor))
    {
        Token token = m_reader.advanceToken();
        // Concat the type. 
        buf << token.Content;
    }

    auto handle = m_typePool->add(buf);

    outType = m_typePool->getSlice(handle);
    return SLANG_OK;
}

SlangResult CPPExtractor::_maybeParseType(UnownedStringSlice& outType)
{
    Index templateDepth = 0;
    SlangResult res = _maybeParseType(outType, templateDepth);
    if (SLANG_FAILED(res) && m_sink->errorCount)
    {
        return res;
    }

    if (templateDepth != 0)
    {
        m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::unexpectedTemplateClose);
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult CPPExtractor::_maybeParseField()
{
    Node::Field field;

    UnownedStringSlice typeName;
    if (SLANG_FAILED(_maybeParseType(typeName)))
    {
        if (m_sink->errorCount)
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

    switch (m_reader.peekTokenType())
    {
        case TokenType::OpAssign:
        case TokenType::Semicolon:
        {
            Node::Field field;
            field.type = typeName;
            field.name = fieldName;

            m_currentNode->m_fields.add(field);

            break;
        }
        default: break;
    }

    _consumeToSync();
    return SLANG_OK;
}

/* static */Node::Type CPPExtractor::_toNodeType(IdentifierStyle style)
{
    switch (style)
    {
        case IdentifierStyle::Class: return Node::Type::ClassType;
        case IdentifierStyle::Struct: return Node::Type::StructType;
        case IdentifierStyle::Namespace: return Node::Type::Namespace;
        default: return Node::Type::Invalid;
    }
}

SlangResult CPPExtractor::parse(SourceFile* sourceFile)
{  
    SourceManager* manager = sourceFile->getSourceManager();

    SourceView* sourceView = manager->createSourceView(sourceFile, nullptr);

    Lexer lexer;

    m_currentNode = m_rootNode;

    lexer.initialize(sourceView, m_sink, m_namePool, manager->getMemoryArena());
    m_tokenList = lexer.lexAllTokens();
    // See if there were any errors
    if (m_sink->errorCount)
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
                IdentifierStyle style = m_identifierLookup->get(m_reader.peekToken().Content);

                switch (style)
                {
                    case IdentifierStyle::BaseClass:
                    {
                        m_reader.advanceToken();

                        Token nameToken;
                        SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));
                        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &nameToken));
                        SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

                        RefPtr<Node> node(new Node(Node::Type::ClassType));
                        node->m_name = nameToken;
                        node->m_isBaseType = true;

                        SLANG_RETURN_ON_FAIL(pushNode(node));
                        popBrace();
                        break;
                    }
                    case IdentifierStyle::Root:
                    {
                        if (m_currentNode && m_currentNode->isClassLike())
                        {
                            m_currentNode->m_isBaseType = true;
                        }
                        m_reader.advanceToken();
                        break;
                    }
                    default:
                    {
                        IdentifierFlags flags = getFlags(style);

                        if (flags & IdentifierFlag::StartScope)
                        {
                            Node::Type type = _toNodeType(style);
                            SLANG_RETURN_ON_FAIL(_maybeParseNode(type));
                        }
                        else
                        {
                            // Special case the node that's the root of the hierarchy (as far as reflection is concerned)
                            // This could be a field
                            if (m_currentNode->acceptsFields())
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
                SLANG_RETURN_ON_FAIL(popBrace());
                m_reader.advanceToken();
                break;
            }
            case TokenType::EndOfFile:
            {
                // Okay we need to confirm that we are in the root node, and with no open braces
                if (m_currentNode != m_rootNode)
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
                    while (m_reader.peekTokenType() != TokenType::EndOfDirective && m_reader.peekTokenType() != TokenType::EndOfFile)
                    {
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

SlangResult CPPExtractor::_calcDerivedTypesRec(Node* node)
{
    if (node->isClassLike() && !node->m_isBaseType)
    {
        if (node->m_super.Content.getLength())
        {
            Node* parentScope = node->m_parentScope;
            if (parentScope == nullptr)
            {
                m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("Can't lookup in scope if there is none!"));
                return SLANG_FAIL;
            }

            Node* superType = parentScope->findChild(node->m_super.Content);
            if (!superType)
            {
                if (node->m_isReflected)
                {
                    m_sink->diagnose(node->m_name, CPPDiagnostics::superTypeNotFound, node->m_name.Content);
                    return SLANG_FAIL;
                }
            }
            else
            {
                if (!superType->isClassLike())
                {
                    m_sink->diagnose(node->m_name, CPPDiagnostics::superTypeNotAType, node->m_name.Content);
                    return SLANG_FAIL;
                }

                // The base class must be defined in same scope (as we didn't allow different scopes for base classes)

                superType->addDerived(node);
            }
        }
        else
        {
            // If it has no super class defined, then we can just make it a root without being set
            node->m_isBaseType = true;
        }
    }

    if (node->m_isBaseType)
    {
        m_baseTypes.add(node);
    }

    for (Node* child : node->m_children)
    {
        SLANG_RETURN_ON_FAIL(_calcDerivedTypesRec(child));
    }

    return SLANG_OK;
}

SlangResult CPPExtractor::calcDerivedTypes()
{
    return _calcDerivedTypesRec(m_rootNode);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractorApp !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

struct Options
{
    void reset()
    {
        m_inputPaths.clear();
        m_outputPath = String();
        m_dump = false;
    }

    bool m_dump = false;
    List<String> m_inputPaths;
    String m_outputPath;
    String m_inputDirectory;
};

struct OptionsParser
{

    /// Parse the parameters. NOTE! Must have the program path removed
    SlangResult parse(int argc, const char*const* argv, DiagnosticSink* sink, Options& outOptions);

    SlangResult _parseArgWithValue(const char* option, String& outValue);
    
    Index m_index;
    Int m_argCount;
    const char*const* m_args;
    DiagnosticSink* m_sink;
};

SlangResult OptionsParser::_parseArgWithValue(const char* option, String& ioValue)
{
    SLANG_ASSERT(UnownedStringSlice(m_args[m_index]) == option);
    if (m_index + 1 < m_argCount)
    {
        // Next parameter is the output path, there can only be one
        if (ioValue.getLength())
        {
            // There already is output
            m_sink->diagnose(SourceLoc(), CPPDiagnostics::optionAlreadyDefined, option, ioValue);
            return SLANG_FAIL;
        }
    }
    else
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::requireValueAfterOption, option);
        return SLANG_FAIL;
    }

    ioValue = m_args[m_index + 1];
    m_index += 2;
    return SLANG_OK;
}

SlangResult OptionsParser::parse(int argc, const char*const* argv, DiagnosticSink* sink, Options& outOptions)
{
    outOptions.reset();

    m_index = 0;
    m_argCount = argc;
    m_args = argv;
    m_sink = sink;

    outOptions.reset();

    while (m_index < m_argCount)
    {
        const UnownedStringSlice arg = UnownedStringSlice(argv[m_index]);

        if (arg.getLength() > 0 && arg[0] == '-')
        {
            if (arg == "-d")
            {
                SLANG_RETURN_ON_FAIL(_parseArgWithValue("-d", outOptions.m_inputDirectory));
                continue;
            }
            else if (arg == "-o")
            {
                SLANG_RETURN_ON_FAIL(_parseArgWithValue("-o", outOptions.m_outputPath));
                continue;
            }
            else if (arg == "-dump")
            {
                outOptions.m_dump = true;
                m_index++;
                continue;
            }

            m_sink->diagnose(SourceLoc(), CPPDiagnostics::unknownOption, arg);
            return SLANG_FAIL;
        }
        else
        {
            // If it starts with - then it an unknown option
            outOptions.m_inputPaths.add(arg);
            m_index++;
        }
    }

    if (outOptions.m_inputPaths.getCount() < 0)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::noInputPathsSpecified);
        return SLANG_FAIL;
    }
    if (outOptions.m_outputPath.getLength() == 0)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::noOutputPathSpecified);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractorApp !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CPPExtractorApp
{
public:
    
    SlangResult readAllText(const Slang::String& fileName, String& outRead);

    SlangResult execute(const Options& options);

        /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv);

    CPPExtractorApp(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager),
        m_slicePool(StringSlicePool::Style::Default)
    {
        m_namePool.setRootNamePool(rootNamePool);

        // Some keywords
        {
            const char* names[] = { "virtual", "typedef" };
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Keyword);
        }
        
        // Modifiers
        {
            const char* names[] = { "const", "volatile" };
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::TypeModifier);
        }

        // Strings to look for...
        {
            const char* names[] = {"SLANG_ABSTRACT_CLASS", "SLANG_CLASS"};
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Marker);
        }

        {
            m_identifierLookup.set("SLANG_CLASS_ROOT", IdentifierStyle::Root);
            m_identifierLookup.set("SLANG_REFLECT_BASE_CLASS", IdentifierStyle::BaseClass);
        }

        // Scope
        {
            m_identifierLookup.set("struct", IdentifierStyle::Struct);
            m_identifierLookup.set("class", IdentifierStyle::Class);
            m_identifierLookup.set("namespace", IdentifierStyle::Namespace);
        }

        // Access
        {
            const char* names[] = { "private", "protected", "public" };
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Access);
        }
    }

    
protected:
    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;
    IdentifierLookup m_identifierLookup;

    StringSlicePool m_slicePool;
};

SlangResult CPPExtractorApp::readAllText(const Slang::String& fileName, String& outRead)
{
    try
    {
        StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
        outRead = reader.ReadToEnd();

    }
    catch (const IOException&)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }
    catch (...)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::execute(const Options& options)
{
    m_options = options;

    CPPExtractor extractor(&m_slicePool, &m_namePool, m_sink, &m_identifierLookup);

    // Read in each of the input files
    for (Index i = 0; i < m_options.m_inputPaths.getCount(); ++i)
    {
        String inputPath;

        if (m_options.m_inputDirectory.getLength())
        {
            inputPath = Path::combine(m_options.m_inputDirectory, m_options.m_inputPaths[i]);
        }
        else
        {
            inputPath = m_options.m_inputPaths[i];
        }

        // Read the input file
        String contents;
        SLANG_RETURN_ON_FAIL(readAllText(inputPath, contents));

        PathInfo pathInfo = PathInfo::makeFromString(inputPath);

        SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(pathInfo, contents);

        SLANG_RETURN_ON_FAIL(extractor.parse(sourceFile));
    }

    SLANG_RETURN_ON_FAIL(extractor.calcDerivedTypes());

    // Dump out the tree
    if (options.m_dump)
    {
        {
            StringBuilder buf;
            extractor.getRootNode()->dump(0, buf);
            m_sink->writer->write(buf.getBuffer(), buf.getLength());
        }

        {
            const List<Node*>& baseTypes = extractor.getBaseTypes();

            for (Node* baseType : baseTypes)
            {
                StringBuilder buf;

                baseType->dumpDerived(0, buf);

                m_sink->writer->write(buf.getBuffer(), buf.getLength());

            }
        }
    }

    return SLANG_OK;
}

/// Execute
SlangResult CPPExtractorApp::executeWithArgs(int argc, const char*const* argv)
{
    Options options;
    OptionsParser optionsParser;
    SLANG_RETURN_ON_FAIL(optionsParser.parse(argc, argv, m_sink, options));
    SLANG_RETURN_ON_FAIL(execute(options));
    return SLANG_OK;
}

} // namespace SlangExperimental

int main(int argc, const char*const* argv)
{
    using namespace SlangExperimental;
    using namespace Slang;

    {
        RootNamePool rootNamePool;

        SourceManager sourceManager;
        sourceManager.initialize(nullptr, nullptr);

        ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

        DiagnosticSink sink(&sourceManager);
        sink.writer = writer;

        CPPExtractorApp app(&sink, &sourceManager, &rootNamePool);
        if (SLANG_FAILED(app.executeWithArgs(argc - 1, argv + 1)))
        {
            return 1;
        }
        if (sink.errorCount)
        {
            return 1;
        }

    }
    return 0;
}

