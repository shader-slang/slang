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
        ClassLike   = 0x2,          ///< Struct or class
        Keyword     = 0x4,
    };
};

static const IdentifierFlags kIdentifierFlags[Index(IdentifierStyle::CountOf)] =
{
    0,              /// None
    0,              /// Identifier 
    0,              /// Root
    0,              /// BaseClass
    IdentifierFlag::Keyword,              /// TypeModifier
    IdentifierFlag::Keyword,              /// Keyword
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Class
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Struct
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

class SourceOrigin;

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

    enum class BaseType
    {
        None,           ///< Neither a base or marked base
        Marked,         ///< It's a marked base
        Unmarked,       ///< It's a base, but it's not marked 
    };

    bool isClassLike() const { return m_type == Type::StructType || m_type == Type::ClassType; }

        /// Add a child node to this nodes scope
    void addChild(Node* child);

        /// Find a child node in this scope with the specified name. Return nullptr if not found
    Node* findChild(const UnownedStringSlice& name) const;

        /// Add a node that is derived from this
    void addDerived(Node* derived);

        /// True if can accept fields (class like types can)
    bool acceptsFields() const { return isClassLike(); }

    void dump(int indent, StringBuilder& out);
    void dumpDerived(int indentCount, StringBuilder& out);

        /// Calculate the absolute name for this namespace/type
    void calcAbsoluteName(StringBuilder& outName) const;

        /// Do depth first traversal of nodes
    void calcScopeDepthFirst(List<Node*>& outNodes);

        /// Traverse the hierarchy of derived nodes, in depth first order
    void calcDerivedDepthFirst(List<Node*>& outNodes);

        /// Calculate the scope path to this node, from the root 
    void calcScopePath(List<Node*>& outPath) { calcScopePath(this, outPath); }

        /// Calculates the derived depth 
    Index calcDerivedDepth() const;

        /// Gets the anonymous namespace associated with this scope
    Node* getAnonymousNamespace();

        /// Find the last (reflected) derived type
    Node* findLastDerived();

        /// True if has a derived type that is reflected
    bool hasReflectedDerivedType() const;
        /// Stores in out any reflected derived types
    void getReflectedDerivedTypes(List<Node*>& out) const;

    static void filterReflectedClassLike(List<Node*>& io);

    static void calcScopePath(Node* node, List<Node*>& outPath);

    Node(Type type):
        m_type(type),
        m_parentScope(nullptr),
        m_isReflected(false),
        m_superNode(nullptr),
        m_baseType(BaseType::None)
    {
        m_anonymousNamespace = nullptr;
    }

        /// The type of node this is
    Type m_type;

        /// All of the types and namespaces in this *scope*
    List<RefPtr<Node>> m_children;

        /// All of the types derived from this type
    List<RefPtr<Node>> m_derivedTypes;

        /// Map from a name (in this scope) to the Node
    Dictionary<UnownedStringSlice, Node*> m_childMap;

        /// All of the fields within a *type*
    List<Field> m_fields;

        /// There can only be one anonymousNamespace for a scope. If there is one it's held here
    Node* m_anonymousNamespace;

        /// Defines where this was uniquely defined. For namespaces if it straddles multiple source files will be the first instance.
    SourceOrigin* m_origin;

        /// Classes can be traversed, but not reflected. To be reflected they have to contain the marker
    bool m_isReflected;
        /// The base type of this
    BaseType m_baseType;

    Token m_name;           ///< The name of this scope/type
    Token m_super;          ///< Super class name
    Token m_marker;         ///< The marker associated with this scope (typically the marker is SLANG_CLASS etc, that is used to identify reflectedType)

    Node* m_parentScope;    ///< The scope this type/scope is defined in
    Node* m_superNode;    ///< If this is a class/struct, the type it is derived from (or nullptr if base)
};

class SourceOrigin : public RefObject
{
public:

    void addNode(Node* node)
    {
        if (node->isClassLike())
        {
            SLANG_ASSERT(node->m_origin == nullptr);
            node->m_origin = this;
        }
        else
        {
            if (node->m_origin == nullptr)
            {
                node->m_origin = this;
            }
        }
        m_nodes.add(node);
    }

    SourceOrigin(SourceFile* sourceFile) :
        m_sourceFile(sourceFile)
    {}

    String m_macroOriginText;                 ///< The macro text is inserted into the macro to identify the origin. It is based on the filename

    String m_filePath;
    SourceFile* m_sourceFile;
    
    // All of the nodes defined in this file in the order they were defined
    // Note that the same namespace may be listed multiple times.
    List<RefPtr<Node> > m_nodes;
};

struct Options;

class CPPExtractor
{
public:

    SlangResult expect(TokenType type, Token* outToken = nullptr);

    bool advanceIfMarker(Token* outToken = nullptr);
    bool advanceIfToken(TokenType type, Token* outToken = nullptr);
    bool advanceIfStyle(IdentifierStyle style, Token* outToken = nullptr);

    SlangResult pushAnonymousNamespace();
    SlangResult pushNode(Node* node);
    SlangResult consumeToClosingBrace(const Token* openBraceToken = nullptr);
    SlangResult popBrace();

        /// Parse the contents of the source file
    SlangResult parse(SourceFile* sourceFile, const Options* options);

    /// When parsing we don't lookup all up super types/add derived types. This is because
    /// we allow files to be processed in any order, so we have to do the type lookup as a separate operation
    SlangResult calcDerivedTypes();

    /// Only valid after calcDerivedTypes has been executed
    const List<Node*>& getBaseTypes() const { return m_baseTypes; }

        /// Get all of the parsed source origins
    const List<RefPtr<SourceOrigin> >& getSourceOrigins() const { return m_origins; }

    /// Get the root node
    Node* getRootNode() const { return m_rootNode; }

    CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup);

protected:
    static Node::Type _toNodeType(IdentifierStyle style);

    bool _isMarker(const UnownedStringSlice& name);

    SlangResult _maybeParseNode(Node::Type type);
    SlangResult _maybeParseField();

    SlangResult _maybeParseType(UnownedStringSlice& outType);

    SlangResult _maybeParseType(UnownedStringSlice& outType, Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArgs(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArg(Index& ioTemplateDepth);

    SlangResult _calcDerivedTypesRec(Node* node);
    String _calcMacroOriginText(const String& filePath);

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();

    TokenList m_tokenList;
    TokenReader m_reader;

    Node* m_currentNode;            ///< The current scope being processed

    RefPtr<Node> m_rootNode;        ///< The root scope 

    List<Node*> m_baseTypes;        ///< All of the types which are base. Only set after calcDerivedTypes

    SourceOrigin* m_origin;

    DiagnosticSink* m_sink;

    NamePool* m_namePool;

    List<RefPtr<SourceOrigin>> m_origins;

    const Options* m_options;

    IdentifierLookup* m_identifierLookup;
    StringSlicePool* m_typePool;
};


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Node Impl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Node* Node::getAnonymousNamespace()
{
    
    if (!m_anonymousNamespace)
    {
        m_anonymousNamespace = new Node(Type::AnonymousNamespace);
        m_anonymousNamespace->m_parentScope = this;
        m_children.add(m_anonymousNamespace);
    }

    return m_anonymousNamespace;
}

void Node::addChild(Node* child)
{
    SLANG_ASSERT(child->m_parentScope == nullptr);
    // Can't add anonymous namespace this way - should be added via getAnonymousNamespace
    SLANG_ASSERT(child->m_type != Type::AnonymousNamespace);

    child->m_parentScope = this;
    m_children.add(child);

    if (child->m_name.Content.getLength())
    {
        m_childMap.Add(child->m_name.Content, child);
    }
}

Node* Node::findChild(const UnownedStringSlice& name) const
{
    Node** nodePtr = m_childMap.TryGetValue(name);
    return (nodePtr) ? *nodePtr : nullptr;
}

/// Add a node that is derived from this
void Node::addDerived(Node* derived)
{
    SLANG_ASSERT(derived->m_superNode == nullptr);
    derived->m_superNode = this;
    m_derivedTypes.add(derived);
}

void Node::calcScopeDepthFirst(List<Node*>& outNodes)
{
    outNodes.add(this);
    for (Node* child : m_children)
    {
        child->calcScopeDepthFirst(outNodes);
    }
}

void Node::calcDerivedDepthFirst(List<Node*>& outNodes)
{
    outNodes.add(this);
    for (Node* derivedType : m_derivedTypes)
    {
        derivedType->calcDerivedDepthFirst(outNodes);
    }
}

static void _indent(Index indentCount, StringBuilder& out)
{
    for (Index i = 0; i < indentCount; ++i)
    {
        out << "    ";
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

void Node::calcAbsoluteName(StringBuilder& outName) const
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
            outName << "{Anonymous}";
        }
        else
        {
            outName << m_name.Content;
        }
    }
}

Index Node::calcDerivedDepth() const
{
    const Node* node = this;
    Index count = 0;

    while (node)
    {
        count++;
        node = node->m_superNode;
    }

    return count;
}

Node* Node::findLastDerived()
{
    if (!m_isReflected)
    {
        return nullptr;
    }

    for (Index i = m_derivedTypes.getCount() - 1; i >= 0; --i)
    {
        Node* derivedType = m_derivedTypes[i];
        Node* found = derivedType->findLastDerived();
        if (found)
        {
            return found;
        }
    }
    return this;
}

/* static */void Node::calcScopePath(Node* node, List<Node*>& outPath)
{
    outPath.clear();

    while (node)
    {
        outPath.add(node);
        node = node->m_parentScope;
    }

    // reverse the order, so we go from root to the node
    outPath.reverse();
}

bool Node::hasReflectedDerivedType() const
{
    for (Node* type : m_derivedTypes)
    {
        if (type->m_isReflected)
        {
            return true;
        }
    }
    return false;
}

void Node::getReflectedDerivedTypes(List<Node*>& out) const
{
    out.clear();
    for (Node* type : m_derivedTypes)
    {
        if (type->m_isReflected)
        {
            out.add(type);
        }
    }
}

/* static */void Node::filterReflectedClassLike(List<Node*>& ioNodes)
{
    // Filter out all the unreflected nodes
    const Index count = ioNodes.getCount();
    for (Index j = 0; j < count; )
    {
        Node* node = ioNodes[j];
        if (!node->isClassLike() || !node->m_isReflected)
        {
            ioNodes.removeAt(j);
        }
        else
        {
            j++;
        }
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Options !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

struct Options
{
    void reset()
    {
        *this = Options();
    }

    Options()
    {
        m_prefixMark = "SLANG_";
        m_postfixMark = "_CLASS";
    }

    bool m_defs = false;            ///< If set will output a '-defs.h' file for each of the input files, that corresponds to previous defs files (although doesn't have fields/RAW)
    bool m_dump = false;            ///< If true will dump to stderr the types/fields and hierarchy it extracted

    List<String> m_inputPaths;      ///< The input paths to the files to be processed
        
    String m_outputPath;            ///< The ouput path. Note that the extractor can generate multiple output files, and this will actually be the 'stem' of several files
    String m_inputDirectory;        ///< The input directory that is by default used for reading m_inputPaths from. 
    String m_reflectType;           ///< The typename used for output
    String m_prefixMark;            ///< The prefix of the 'marker' used to identify a reflected type
    String m_postfixMark;           ///< The postfix of the 'marker' used to identify a reflected type
    String m_stripFilePrefix;       ///< Used for the 'origin' information, this is stripped from the source filename, and the remainder of the filename (without extension) is 'macroized'
};

struct OptionsParser
{
    /// Parse the parameters. NOTE! Must have the program path removed
    SlangResult parse(int argc, const char*const* argv, DiagnosticSink* sink, Options& outOptions);

    SlangResult _parseArgWithValue(const char* option, String& outValue);
    SlangResult _parseArgReplaceValue(const char* option, String& outValue);

    String m_reflectType;

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

SlangResult OptionsParser::_parseArgReplaceValue(const char* option, String& ioValue)
{
    SLANG_ASSERT(UnownedStringSlice(m_args[m_index]) == option);
    if (m_index + 1 >= m_argCount)
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
            else if (arg == "-reflect-type")
            {
                SLANG_RETURN_ON_FAIL(_parseArgWithValue("-reflect-type", outOptions.m_reflectType));
                continue;
            }
            else if (arg == "-prefix-mark")
            {
                SLANG_RETURN_ON_FAIL(_parseArgReplaceValue("-prefix-mark", outOptions.m_prefixMark));
                continue;
            }
            else if (arg == "-postfix-mark")
            {
                SLANG_RETURN_ON_FAIL(_parseArgReplaceValue("-postfix-mark", outOptions.m_postfixMark));
                continue;
            }
            else if (arg == "-defs")
            {
                outOptions.m_defs = true;
                continue;
            }
            else if (arg == "-strip-prefix")
            {
                SLANG_RETURN_ON_FAIL(_parseArgWithValue("-strip-prefix", outOptions.m_stripFilePrefix));
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

    // Set default name
    if (outOptions.m_reflectType.getLength() == 0)
    {
        outOptions.m_reflectType = "ASTNode";
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

CPPExtractor::CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup):
    m_typePool(typePool),
    m_sink(sink),
    m_namePool(namePool),
    m_identifierLookup(identifierLookup)
{
    m_rootNode = new Node(Node::Type::Namespace);
}

bool CPPExtractor::_isMarker(const UnownedStringSlice& name)
{
    return name.startsWith(m_options->m_prefixMark.getUnownedSlice()) && name.endsWith(m_options->m_postfixMark.getUnownedSlice());
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

bool CPPExtractor::advanceIfMarker(Token* outToken)
{
    const Token peekToken = m_reader.peekToken();
    if (peekToken.type == TokenType::Identifier && _isMarker(peekToken.Content))
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
    m_currentNode = m_currentNode->getAnonymousNamespace();

    if (m_origin)
    {
        m_origin->addNode(m_currentNode);
    }

    return SLANG_OK;
}

SlangResult CPPExtractor::pushNode(Node* node)
{
    if (m_origin)
    {
        m_origin->addNode(node);
    }

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
                // Make sure the node is empty, as we are *not* going to add it, we are just going to use
                // the pre-existing namespace
                SLANG_ASSERT(node->m_children.getCount() == 0);

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
                RefPtr<Node> node(new Node(Node::Type::Namespace));
                node->m_name = name;
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

    RefPtr<Node> node(new Node(type));
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

        // If it's one of the markers, then we continue to extract parameter
        if (advanceIfMarker(&node->m_marker))
        {
            break;
        }

        // Looks like a class, but looks like non-reflected
        node->m_isReflected = false;
        // We still need to add the node,
        SLANG_RETURN_ON_FAIL(pushNode(node));
        return SLANG_OK;
    }

    // Okay now looking for ( identifier)
    Token typeNameToken;

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeNameToken));
    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    if (typeNameToken.Content != node->m_name.Content)
    {
        m_sink->diagnose(typeNameToken, CPPDiagnostics::typeNameDoesntMatch, node->m_name.Content);
        return SLANG_FAIL;
    }
    
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
    while (advanceIfStyle(IdentifierStyle::TypeModifier));
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

SlangResult CPPExtractor::parse(SourceFile* sourceFile, const Options* options)
{
    m_options = options;

    RefPtr<SourceOrigin> origin = new SourceOrigin(sourceFile);
    origin->m_macroOriginText = _calcMacroOriginText(sourceFile->getPathInfo().foundPath);
    m_origins.add(origin);

    // Set the current origin
    m_origin = origin;

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
                        node->m_baseType = Node::BaseType::Marked;

                        SLANG_RETURN_ON_FAIL(pushNode(node));
                        popBrace();
                        break;
                    }
                    case IdentifierStyle::Root:
                    {
                        if (m_currentNode && m_currentNode->isClassLike())
                        {
                            m_currentNode->m_baseType = Node::BaseType::Marked;
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
    if (node->isClassLike() && node->m_baseType == Node::BaseType::None)
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
            node->m_baseType = Node::BaseType::Unmarked;
        }
    }

    if (node->m_baseType != Node::BaseType::None)
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


String CPPExtractor::_calcMacroOriginText(const String& filePath)
{
    String fileName = Path::getFileNameWithoutExt(filePath);

    if (m_options->m_stripFilePrefix.getLength() && fileName.startsWith(m_options->m_stripFilePrefix))
    {
        const Index len = m_options->m_stripFilePrefix.getLength();
        fileName = UnownedStringSlice(fileName.begin() + len, fileName.end());
    }

    const char* start = fileName.begin();
    const char* end = fileName.end();

    // Trim any - 
    while (start < end && *start == '-') ++start;
    while (end - 1 > start && end[-1] == '-') --end;

    StringBuilder out;

    // Make into macro like name
    for (; start < end; ++start)
    {
        char c = *start;

        if (c == '-')
        {
            c = '_';
        }
        else if (c >= 'a' && c <= 'z')
        {
            c = c - 'a' + 'A';
        }
        out.append(c);
    }

    return out;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractorApp !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CPPExtractorApp
{
public:
    
    SlangResult readAllText(const Slang::String& fileName, String& outRead);
    SlangResult writeAllText(const Slang::String& fileName, const UnownedStringSlice& text);

    SlangResult execute(const Options& options);

        /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv);

        /// Write output
    SlangResult writeOutput(CPPExtractor& extractor);

        /// Write def files
    SlangResult writeDefs(CPPExtractor& extractor);

        /// Calculate the header 
    SlangResult calcHeader(CPPExtractor& extractor, StringBuilder& out);
    SlangResult calcChildrenHeader(CPPExtractor& exctractor, StringBuilder& out);

    SlangResult calcDef(CPPExtractor& extractor, SourceOrigin* origin, StringBuilder& out);

    CPPExtractorApp(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager),
        m_slicePool(StringSlicePool::Style::Default)
    {
        m_namePool.setRootNamePool(rootNamePool);

        // Some keywords
        {
            const char* names[] = { "virtual", "typedef", "continue", "if", "case", "break", "catch", "default", "delete", "do", "else", "for", "new", "goto", "return", "switch", "throw", "using", "while" };
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Keyword);
        }

        // Type modifier keywords
        {
            const char* names[] = { "const", "volatile" };
            m_identifierLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::TypeModifier);
        }

        // Special markers
        {
            m_identifierLookup.set("SLANG_CLASS_ROOT", IdentifierStyle::Root);
            m_identifierLookup.set("SLANG_REFLECT_BASE_CLASS", IdentifierStyle::BaseClass);
        }

        // Keywords which introduce types/scopes
        {
            m_identifierLookup.set("struct", IdentifierStyle::Struct);
            m_identifierLookup.set("class", IdentifierStyle::Class);
            m_identifierLookup.set("namespace", IdentifierStyle::Namespace);
        }

        // Keywords that control access
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

SlangResult CPPExtractorApp::writeAllText(const Slang::String& fileName, const UnownedStringSlice& text)
{
    try
    {
        StreamWriter writer(new FileStream(fileName, FileMode::Create));
        writer.Write(text);
    }
    catch (const IOException&)
    {
        m_sink->diagnose(SourceLoc(), CPPDiagnostics::cannotOpenFile, fileName);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::calcDef(CPPExtractor& extractor, SourceOrigin* origin, StringBuilder& out)
{
    Node* currentScope = nullptr;

    for (Node* node : origin->m_nodes)
    {
        if (node->isClassLike() && node->m_isReflected)
        {
            if (node->m_marker.Content.indexOf(UnownedStringSlice::fromLiteral("ABSTRACT")) >= 0)
            {
                out << "ABSTRACT_";
            }

            out << "SYNTAX_CLASS(" << node->m_name.Content << ", " << node->m_super.Content << ")\n";
            out << "END_SYNTAX_CLASS()\n\n";
        }
    }
    return SLANG_OK;
}



SlangResult CPPExtractorApp::calcChildrenHeader(CPPExtractor& extractor, StringBuilder& out)
{
    const List<Node*>& baseTypes = extractor.getBaseTypes();

    const String& reflectTypeName = m_options.m_reflectType;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        Node* baseType = baseTypes[i];
        if (baseType->m_baseType != Node::BaseType::Marked)
        {
            continue;
        }

        List<Node*> nodes;
        baseType->calcDerivedDepthFirst(nodes);
        Node::filterReflectedClassLike(nodes);

        List<Node*> derivedTypes;

        out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHILDREN !!!!!!!!!!!!!!!!!!!!!!!!!!!! */ \n\n";

        // Now the children 
        for (Node* node : nodes)
        {
            node->getReflectedDerivedTypes(derivedTypes);

            // Define the derived types
            out << "#define " << m_options.m_prefixMark << "CHILDREN_" << reflectTypeName << "_"  << node->m_name.Content << "(x, param)";

            if (derivedTypes.getCount())
            {
                out << " \\\n";
                for (Index j = 0; j < derivedTypes.getCount(); ++j)
                {
                    Node* derivedType = derivedTypes[j];
                    _indent(1, out);
                    out << m_options.m_prefixMark << "ALL_" << reflectTypeName << "_" << derivedType->m_name.Content << "(x, param)";
                    if (j < derivedTypes.getCount() - 1)
                    {
                        out << "\\\n";
                    }
                }    
            }
            out << "\n\n";
        }

        out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ALL !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

        for (Node* node : nodes)
        {
            // Define the derived types
            out << "#define " << m_options.m_prefixMark << "ALL_" << reflectTypeName << "_" << node->m_name.Content << "(x, param) \\\n";
            _indent(1, out);
            out << m_options.m_prefixMark << reflectTypeName << "_"  << node->m_name.Content << "(x, param)";

            // If has derived types output them
            if (node->hasReflectedDerivedType())
            {
                out << " \\\n";
                _indent(1, out);
                out << m_options.m_prefixMark << "CHILDREN_" << reflectTypeName << "_" << node->m_name.Content << "(x, param)";
            }
            out << "\n\n";
        }
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::calcHeader(CPPExtractor& extractor, StringBuilder& out)
{
    const List<Node*>& baseTypes = extractor.getBaseTypes();

    const String& reflectTypeName = m_options.m_reflectType;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        Node* baseType = baseTypes[i];
        if (baseType->m_baseType != Node::BaseType::Marked)
        {
            continue;
        }

        List<Node*> baseScopePath;
        baseType->calcScopePath(baseScopePath);

        // Remove the global scope
        baseScopePath.removeAt(0);
        // Remove the type itself
        baseScopePath.removeLast();

        for (Node* scopeNode : baseScopePath)
        {
            SLANG_ASSERT(scopeNode->m_type == Node::Type::Namespace);
            out << "namespace " << scopeNode->m_name.Content << " {\n";
        }

        List<Node*> nodes;
        baseType->calcDerivedDepthFirst(nodes);
        Node::filterReflectedClassLike(nodes);

        // Write out the types
        {
            out << "\n";
            out << "enum class " << reflectTypeName << "Type\n";
            out << "{\n";

            Index typeIndex = 0;
            for (Node* node : nodes)
            {
                // Okay first we are going to output the enum values
                const Index depth = node->calcDerivedDepth() - 1;
                _indent(depth, out);
                out << node->m_name.Content << " = " << typeIndex << ",\n";
                typeIndex++;
            }

            out << "};\n\n";
        }

#if 0
        out << "\n";
        out << "enum class " << reflectTypeName << "Last\n";
        out << "{\n";

        for (Node* node : nodes)
        {
            SLANG_ASSERT(node->isClassLike());
            // If it's not reflected we don't output, in the enum list
            if (!node->m_isReflected)
            {
                continue;
            }

            Node* lastDerived = node->findLastDerived();
            if (lastDerived)
            {
                // Okay first we are going to output the enum values
                const Index depth = node->calcDerivedDepth() - 1;
                _indent(depth, out);
                out << node->m_name.Content << " = int(" << reflectTypeName << "Type::" << lastDerived->m_name.Content << "),\n";
            }
        }

        out << "};\n\n";
#endif

        // Predeclare the classes
        {
            out << "// Predeclare\n\n";
            for (Node* node : nodes)
            {
                SLANG_ASSERT(node->isClassLike());
                // If it's not reflected we don't output, in the enum list
                if (node->m_isReflected)
                {
                    const char* type = (node->m_type == Node::Type::ClassType) ? "class" : "struct";
                    out << type << " " << node->m_name.Content << ";\n";
                }
            }
        }

#if 0
        out << "struct " << reflectTypeName << "Super\n";
        out << "{\n";

        for (Node* node : nodes)
        {
            // If it's not reflected we don't output, in the enum list
            if (node->m_isReflected && node->m_superNode)
            {
                _indent(1, out);
                // We concat _Super so the typedef are distinct from the ones being referenced
                out << "typedef " << node->m_superNode->m_name.Content << " " << node->m_name.Content << "_Super;\n";
            }
        }

        out << "};\n";
#endif

        // Do the macros for each of the types

        {
            out << "// Type macros\n\n";

            out << "// Order is (NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \n";
            out << "// NAME - is the class name\n";
            out << "// SUPER - is the super class name (or NO_SUPER)\n";
            out << "// LAST - is the class name for the last in the range (or NO_LAST)\n";
            out << "// MARKER - is the text inbetween in the prefix/postix (like ABSTRACT). If no inbetween text is is 'NONE'\n";
            out << "// TYPE - Can be BASE, INNER or LEAF for the overall base class, an INNER class, or a LEAF class\n";
            out << "// param is a user defined parameter that can be parsed to the invoked x macro\n\n";

            // Output all of the definitions for each type
            for (Node* node : nodes)
            {
                out << "#define SLANG_" << reflectTypeName << "_" << node->m_name.Content << "(x, param) ";

                // Output the X macro part
                _indent(1, out);
                out << "x(" << node->m_name.Content << ", ";

                if (node->m_superNode)
                {
                    out << node->m_superNode->m_name.Content << ", ";
                }
                else
                {
                    out << "NO_SUPER, ";
                }

                // Output the (file origin)
                out << node->m_origin->m_macroOriginText;
                out << ", ";

                // The last type
                Node* lastDerived = node->findLastDerived();
                if (lastDerived)
                {
                    out << lastDerived->m_name.Content << ", ";
                }
                else
                {
                    out << "NO_LAST, ";
                }

                // Output any specifics of the markup
                UnownedStringSlice marker = node->m_marker.Content;
                // Need to extract the name
                if (marker.getLength() > m_options.m_prefixMark.getLength() + m_options.m_postfixMark.getLength())
                {
                    marker = UnownedStringSlice(marker.begin() + m_options.m_prefixMark.getLength(), marker.end() - m_options.m_postfixMark.getLength());
                }
                else
                {
                    marker = UnownedStringSlice::fromLiteral("NONE");
                }
                out << marker << ", ";

                if (node->m_baseType != Node::BaseType::None || node->m_superNode && node->m_superNode->m_isReflected == false)
                {
                    out << "BASE, ";
                }
                else if (node->hasReflectedDerivedType())
                {
                    out << "INNER, ";
                }
                else
                {
                    out << "LEAF, ";
                }
                out << "param)\n";
            }
        }

        // Now pop the scope in revers
        for (Index j = baseScopePath.getCount() - 1; j >= 0; j--)
        {
            Node* scopeNode = baseScopePath[j];
            out << "} // namespace " << scopeNode->m_name.Content << "\n";
        }
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::writeDefs(CPPExtractor& extractor)
{
    const auto& origins = extractor.getSourceOrigins();

    for (SourceOrigin* origin : origins)
    {
        const String path = origin->m_sourceFile->getPathInfo().foundPath;

        // We need to work out the name of the def file

        String ext = Path::getPathExt(path);
        String pathWithoutExt = Path::getPathWithoutExt(path);

        // The output path

        StringBuilder outPath;
        outPath << pathWithoutExt << "-defs." << ext;

        StringBuilder content;
        SLANG_RETURN_ON_FAIL(calcDef(extractor, origin, content));

        // Write the defs file
        SLANG_RETURN_ON_FAIL(writeAllText(outPath, content.getUnownedSlice()));
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::writeOutput(CPPExtractor& extractor)
{
    
    String path;
    if (m_options.m_inputDirectory.getLength())
    {
        path = Path::combine(m_options.m_inputDirectory, m_options.m_outputPath);
    }
    else
    {
        path = m_options.m_outputPath;
    }

    // Get the ext
    String ext = Path::getPathExt(path);
    if (ext.getLength() == 0)
    {
        // Default to .h if not specified
        ext = "h";
    }

    // Strip the extension if set
    path = Path::getPathWithoutExt(path);

    {
        /// Calculate the header
        StringBuilder header;
        SLANG_RETURN_ON_FAIL(calcHeader(extractor, header));

        // Write it out

        StringBuilder headerPath;
        headerPath << path << "." << ext;
        SLANG_RETURN_ON_FAIL(writeAllText(headerPath, header.getUnownedSlice()));
    }

    {
        StringBuilder childrenHeader;
        SLANG_RETURN_ON_FAIL(calcChildrenHeader(extractor, childrenHeader));

        StringBuilder headerPath;
        headerPath << path << "-macro." + ext;
        SLANG_RETURN_ON_FAIL(writeAllText(headerPath, childrenHeader.getUnownedSlice()));
    }

    // Write to output
    // m_sink->writer->write(header.getBuffer(), header.getLength());

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

        SLANG_RETURN_ON_FAIL(extractor.parse(sourceFile, &m_options));
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

    if (options.m_defs)
    {
        SLANG_RETURN_ON_FAIL(writeDefs(extractor));
    }

    if (options.m_outputPath.getLength())
    {
        SLANG_RETURN_ON_FAIL(writeOutput(extractor));
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

