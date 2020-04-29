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

namespace SlangExperimental
{

using namespace Slang;

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

    void addChild(Node* child)
    {
        SLANG_ASSERT(child->m_parent == nullptr);
        child->m_parent = this;
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

    bool acceptsFields() const
    {
        return m_braceStack.getCount() == 0 && (m_type == Type::StructType || m_type == Type::ClassType);
    }

    void dump(int indent, StringBuilder& out);
    
    Node(Type type, Node* parent = nullptr):
        m_type(type),
        m_parent(parent),
        m_isReflected(false)
    {
        m_anonymousNamespace = nullptr;
    }

    Type m_type;

    List<Token> m_braceStack;

    List<RefPtr<Node>> m_children;

    Dictionary<UnownedStringSlice, Node*> m_childMap;

    List<Field> m_fields;

    Node* m_anonymousNamespace;

    bool m_isReflected;

    Token m_name;
    Token m_super;
    Token m_marker;        

    Node* m_parent;
};

static void _indent(int indentCount, StringBuilder& out)
{
    for (int i = 0; i < indentCount; ++i)
    {
        out << "  ";
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CPPExtractor
{
public:
    
    SlangResult expect(TokenType type, Token* outToken = nullptr);

    bool advanceIfToken(TokenType type, Token* outToken = nullptr);

    SlangResult pushAnonymousNamespace();
    SlangResult pushNode(Node* node);
    void pushBrace(const Token& token);
    SlangResult popBrace();

    SlangResult parse(SourceFile* sourceFile, NamePool* namePool, DiagnosticSink* sink, Node* rootNode);

    CPPExtractor();

    static Node::Type _textToNodeType(const UnownedStringSlice& in);
    SlangResult _maybeParseNode(Node::Type type);
    SlangResult _maybeParseField();

    SlangResult _maybeParseType(UnownedStringSlice& outType);
    SlangResult _maybeParseTemplateArgs();
    SlangResult _maybeParseTemplateArg();

    static bool _isVisibilityKeyword(const UnownedStringSlice& slice)
    {
        return slice == UnownedStringSlice::fromLiteral("public") || slice == UnownedStringSlice::fromLiteral("protected") || slice == UnownedStringSlice::fromLiteral("private");
    }
    SlangResult _consumeToSync();

    TokenList m_tokenList;
    TokenReader m_reader;

    List<Node*> m_nodeStack;
    Node* m_currentNode;

    Node* m_rootNode;

    DiagnosticSink* m_sink;

    StringSlicePool m_stringPool;
};

CPPExtractor::CPPExtractor() :
    m_stringPool(StringSlicePool::Style::Default)
{
    // Strings to look for...
    m_stringPool.add("SLANG_ABSTRACT_CLASS");
    m_stringPool.add("SLANG_CLASS");
}

SlangResult CPPExtractor::expect(TokenType type, Token* outToken)
{
    if (m_reader.peekTokenType() != type)
    {
        StringBuilder buf;

        buf << "Expecting " << TokenTypeToString(type) << " found '" << TokenTypeToString(m_reader.peekTokenType());
        m_sink->diagnoseRaw(Severity::Error, buf.getUnownedSlice());

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
            if (node->m_type == Node::Type::StructType ||
                node->m_type == Node::Type::ClassType)
            {
                StringBuilder buf;
                buf << "Type " << foundNode->m_name.Content << " already found";

                m_sink->diagnoseRaw(Severity::Error, buf.getUnownedSlice());
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

void CPPExtractor::pushBrace(const Token& brace)
{
    SLANG_ASSERT(brace.type == TokenType::LBrace);
    m_currentNode->m_braceStack.add(brace);
}

SlangResult CPPExtractor::popBrace()
{
    if (m_currentNode->m_braceStack.getCount() > 0)
    {
        m_currentNode->m_braceStack.removeLast();
        return SLANG_OK;
    }

    if (m_currentNode->m_parent == nullptr)
    {
        m_sink->diagnoseRaw(Severity::Error, "Leaving root scope");
        return SLANG_FAIL;
    }

    m_currentNode = m_currentNode->m_parent;
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
        Token accessToken = m_reader.peekToken();
        if (accessToken.type == TokenType::Identifier && _isVisibilityKeyword(accessToken.Content))
        {
            m_reader.advanceToken();
        }

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
                m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("Expecting { "));
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
        if (m_reader.peekTokenType() == TokenType::Identifier && _isVisibilityKeyword(m_reader.peekToken().Content))
        {
            m_reader.advanceToken();
            // Consume it and a colon
            if (SLANG_FAILED(expect(TokenType::Colon)))
            {
                pushBrace(braceToken);
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
        Index markerIndex = m_stringPool.findIndex(lexeme);
        if (markerIndex < 0)
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
        StringBuilder msg;
        msg << "Class name '" << node->m_name.Content << "' doesn't match '" << typeNameToken.Content << "'";
        m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    node->m_isReflected = true;

    return pushNode(node);
}

static const UnownedStringSlice kConst = UnownedStringSlice::fromLiteral("const");

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

SlangResult CPPExtractor::_maybeParseTemplateArg()
{
    switch (m_reader.peekTokenType())
    {
        case TokenType::Identifier:
        {
            UnownedStringSlice name;
            SLANG_RETURN_ON_FAIL(_maybeParseType(name));
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

SlangResult CPPExtractor::_maybeParseTemplateArgs()
{
    if (!advanceIfToken(TokenType::OpLess))
    {
        return SLANG_FAIL;
    }

    while (true)
    {
        switch (m_reader.peekTokenType())
        {
            case TokenType::OpGreater:
            {
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                while (true)
                {
                    SLANG_RETURN_ON_FAIL(_maybeParseTemplateArg());

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

static bool _isNotTypeKeyword(const UnownedStringSlice& name)
{
    if (name == "virtual" || name == "typedef")
    {
        return true;
    }
    return false;
}


SlangResult CPPExtractor::_maybeParseType(UnownedStringSlice& outType)
{
    Token startToken = m_reader.peekToken();

    if (m_reader.peekTokenType() == TokenType::Identifier)
    {
        if (_isNotTypeKeyword(m_reader.peekToken().Content))
        {
            return SLANG_FAIL;
        }
        if (m_reader.peekToken().Content == kConst)
        {
            m_reader.advanceToken();
        }
    }

    if (m_reader.peekTokenType() == TokenType::Scope)
    {
        m_reader.advanceToken();
    }

    while (true)
    {
        // Strip all the consts
        while (m_reader.peekTokenType() == TokenType::Identifier && m_reader.peekToken().Content == kConst)
        {
            m_reader.advanceToken();
        }

        if (m_reader.peekTokenType() != TokenType::Identifier)
        {
            return SLANG_FAIL;
        }

        m_reader.advanceToken();
        if (m_reader.peekTokenType() == TokenType::Scope)
        {
            m_reader.advanceToken();
            continue;
        }
        break;
    }

    if (m_reader.peekTokenType() == TokenType::OpLess)
    {
        SLANG_RETURN_ON_FAIL(_maybeParseTemplateArgs());
    }

    // Strip all the consts
    while (m_reader.peekTokenType() == TokenType::Identifier && m_reader.peekToken().Content == kConst)
    {
        m_reader.advanceToken();
    }

    // It's a reference and we are done
    if (m_reader.peekTokenType() == TokenType::OpBitAnd)
    {
        m_reader.advanceToken();
        return SLANG_OK;
    }

    while (true)
    {
        if (m_reader.peekTokenType() == TokenType::OpMul)
        {
            m_reader.advanceToken();
            // Strip all the consts
            while (m_reader.peekTokenType() == TokenType::Identifier && m_reader.peekToken().Content == kConst)
            {
                m_reader.advanceToken();
            }
            continue;
        }
        break;
    }

    // This is a bit of a hack -> I don't store the previous token, and Lexer doesn't have an easy interface
    // So I just pull out the content to the begining of the peek token (which might have trailing whitespace (or even comments)
    outType = UnownedStringSlice(startToken.Content.begin(), m_reader.peekToken().Content.begin());

    return SLANG_OK;
}

SlangResult CPPExtractor::_maybeParseField()
{
    Node::Field field;

    UnownedStringSlice typeName;
    if (SLANG_FAILED(_maybeParseType(typeName)))
    {
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

Node::Type CPPExtractor::_textToNodeType(const UnownedStringSlice& in)
{
    if (in == UnownedStringSlice::fromLiteral("struct"))
    {
        return Node::Type::StructType;
    }
    else if (in == UnownedStringSlice::fromLiteral("class"))
    {
        return Node::Type::ClassType;
    }
    else if (in == UnownedStringSlice::fromLiteral("namespace"))
    {
        return Node::Type::Namespace;
    }
    return Node::Type::Invalid;
}

SlangResult CPPExtractor::parse(SourceFile* sourceFile, NamePool* namePool, DiagnosticSink* sink, Node* rootNode)
{
    m_sink = sink;
    m_rootNode = rootNode;
    m_currentNode = rootNode;

    SLANG_ASSERT(rootNode && rootNode->m_braceStack.getCount() == 0);
   
    SourceManager* manager = sourceFile->getSourceManager();

    SourceView* sourceView = manager->createSourceView(sourceFile, nullptr);

    Lexer lexer;

    lexer.initialize(sourceView, m_sink, namePool, manager->getMemoryArena());
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
                Node::Type type = _textToNodeType(m_reader.peekToken().Content);
                if (type != Node::Type::Invalid)
                {
                    SLANG_RETURN_ON_FAIL(_maybeParseNode(type));
                }
                else
                {
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
            case TokenType::LBrace:
            {
                pushBrace(m_reader.advanceToken());
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
                if (m_currentNode != m_rootNode || m_rootNode->m_braceStack.getCount() > 0)
                {
                    if (m_currentNode->m_braceStack.getCount() > 0)
                    {
                        m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("Didn't find matching brace"));
                    }
                    else
                    {
                        m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("Didn't find matching braces at end of file"));
                    }
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractorApp !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CPPExtractorApp
{
public:
    struct Options
    {
        void reset()
        {
            m_inputPaths.clear();
            m_outputPath = String();
        }

        List<String> m_inputPaths;
        String m_outputPath;
    };

    SlangResult readAllText(const Slang::String& fileName, String& outRead);

    SlangResult parseContents(SourceFile* sourceFile, NamePool* namePool, Node* rootNode);

    SlangResult execute(const Options& options);

    /// Parse the parameters. NOTE! Must have the program path removed
    SlangResult parseArgs(int argc, const char*const* argv, Options& outOptions);

        /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv);

    CPPExtractorApp(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager)
    {
        m_namePool.setRootNamePool(rootNamePool);
    }

    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;
};

SlangResult CPPExtractorApp::readAllText(const Slang::String& fileName, String& outRead)
{
    try
    {
        StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
        outRead = reader.ReadToEnd();

    }
    catch (const IOException& except)
    {
        m_sink->diagnoseRaw(Severity::Error, except.Message.getUnownedSlice());
        return SLANG_FAIL;
    }
    catch (...)
    {
        StringBuilder msg;
        msg << "Unable to read '" << fileName << "'";
        m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::parseContents(SourceFile* sourceFile, NamePool* namePool, Node* rootNode)
{
    CPPExtractor parser;
    SLANG_RETURN_ON_FAIL(parser.parse(sourceFile, namePool, m_sink, rootNode));
    return SLANG_OK;
}

SlangResult CPPExtractorApp::execute(const Options& options)
{
    m_options = options;

    // Create the root node
    RefPtr<Node> rootNode(new Node(Node::Type::Namespace));

    // Read in each of the input files
    for (Index i = 0; i < m_options.m_inputPaths.getCount(); ++i)
    {
        const String& inputPath = m_options.m_inputPaths[i];

        // Read the input file
        String contents;
        SLANG_RETURN_ON_FAIL(readAllText(inputPath, contents));

        PathInfo pathInfo = PathInfo::makeFromString(inputPath);

        SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(pathInfo, contents);

        // Okay we now need to parse contents. We know this is 0 terminated (as all strings are).
        SLANG_RETURN_ON_FAIL(parseContents(sourceFile, &m_namePool, rootNode));
    }

    // Dump out the tree
    {
        StringBuilder buf;
        rootNode->dump(0, buf);

        m_sink->writer->write(buf.getBuffer(), buf.getLength());
    }

    return SLANG_OK;
}

/// Parse the parameters. NOTE! Must have the program path removed
SlangResult CPPExtractorApp::parseArgs(int argc, const char*const* argv, Options& outOptions)
{
    outOptions.reset();

    Index i = 0;
    while (i < argc)
    {
        const UnownedStringSlice arg = UnownedStringSlice(argv[i]);

        if (arg.getLength() > 0 && arg[0] == '-')
        {
            if (arg == "-o")
            {
                if (i + 1 < argc)
                {
                    // Next parameter is the output path, there can only be one

                    if (outOptions.m_outputPath.getLength())
                    {
                        // There already is output
                        StringBuilder msg;
                        msg << "Output has already been defined '" << outOptions.m_outputPath << "'";
                        m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                        return SLANG_FAIL;
                    }
                }
                else
                {
                    StringBuilder msg;
                    msg << "Require a value after -o option";
                    m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                    return SLANG_FAIL;
                }

                outOptions.m_outputPath = argv[i + 1];
                i += 2;
                continue;
            }


            {
                StringBuilder msg;
                msg << "Unknown option '" << arg << "'";
                m_sink->diagnoseRaw(Severity::Error, msg.getUnownedSlice());
                return SLANG_FAIL;
            }
        }

        // If it starts with - then it an unknown option
        outOptions.m_inputPaths.add(arg);
        i++;
    }

    if (outOptions.m_inputPaths.getCount() < 0)
    {
        m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("No input paths specified"));
        return SLANG_FAIL;
    }
    if (outOptions.m_outputPath.getLength() == 0)
    {
        m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("No -o output path specified"));
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/// Execute
SlangResult CPPExtractorApp::executeWithArgs(int argc, const char*const* argv)
{
    Options options;
    SLANG_RETURN_ON_FAIL(parseArgs(argc, argv, options));
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

