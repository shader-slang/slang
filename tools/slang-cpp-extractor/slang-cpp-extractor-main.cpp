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
#include "../../source/core/slang-name-convention-util.h"

#include "../../source/slang/slang-source-loc.h"
#include "../../source/slang/slang-lexer.h"
#include "../../source/slang/slang-diagnostics.h"
#include "../../source/slang/slang-file-system.h"
#include "../../source/slang/slang-name.h"

#include "slang-cpp-extractor-diagnostics.h"

/*
Some command lines:

-d source/slang slang-ast-support-types.h slang-ast-base.h slang-ast-decl.h slang-ast-expr.h slang-ast-modifier.h slang-ast-stmt.h slang-ast-type.h slang-ast-val.h -strip-prefix slang- -o slang-generated -output-fields -mark-suffix _CLASS
*/

namespace SlangExperimental
{

using namespace Slang;

enum class IdentifierStyle
{
    None,               ///< It's not an identifier

    Identifier,         ///< Just an identifier

    PreDeclare,        ///< Declare a type (not visible in C++ code)
    TypeSet,            ///< TypeSet

    TypeModifier,       ///< const, volatile etc
    Keyword,            ///< A keyword C/C++ keyword that is not another type
    Class,              ///< class
    Struct,             ///< struct
    Namespace,          ///< namespace
    Access,             ///< public, protected, private

    Reflected,
    Unreflected,

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
        Reflection  = 0x8,
    };
};

static const IdentifierFlags kIdentifierFlags[Index(IdentifierStyle::CountOf)] =
{
    0,              /// None
    0,              /// Identifier
    0,              /// Declare type
    0,              /// Type set
    IdentifierFlag::Keyword,              /// TypeModifier
    IdentifierFlag::Keyword,              /// Keyword
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Class
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Struct
    IdentifierFlag::Keyword | IdentifierFlag::StartScope, /// Namespace
    IdentifierFlag::Keyword,                              /// Access
    IdentifierFlag::Reflection,                           /// Reflected
    IdentifierFlag::Reflection,                           /// Unreflected
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
    void reset()
    {
        m_styles.clear();
        m_pool.clear();
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

enum class ReflectionType
{
    NotReflected,
    Reflected,
};

// Pre-declare
class TypeSet;
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
        bool isReflected() const { return reflectionType == ReflectionType::Reflected; }

        UnownedStringSlice type;
        Token name;
        ReflectionType reflectionType;
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

        /// Get the absolute name
    String getAbsoluteName() const { StringBuilder buf; calcAbsoluteName(buf); return buf.ProduceString(); }

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

        /// True if reflected
    bool isReflected() const { return m_reflectionType == ReflectionType::Reflected; }

        /// Gets the reflection for any contained types
    ReflectionType getContainedReflectionType() const { return m_reflectionType == ReflectionType::NotReflected ? ReflectionType::NotReflected : m_reflectionOverride; }

        /// True if has a derived type that is reflected
    bool hasReflectedDerivedType() const;
        /// Stores in out any reflected derived types
    void getReflectedDerivedTypes(List<Node*>& out) const;

    typedef bool (*Filter)(Node* node);

    static bool isClassLikeAndReflected(Node* node)
    {
        return node->isClassLike() && node->isReflected();
    }
    static bool isClassLike(Node* node)
    {
        return node->isClassLike();
    }

    static void filter(Filter filter, List<Node*>& io);

    static void calcScopePath(Node* node, List<Node*>& outPath);

    Node(Type type):
        m_type(type),
        m_parentScope(nullptr),
        m_reflectionType(ReflectionType::NotReflected),
        m_reflectionOverride(ReflectionType::Reflected),
        m_superNode(nullptr),
        m_origin(nullptr),
        m_typeSet(nullptr)
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
    ReflectionType m_reflectionType;

        /// For child types, fields, how reflection is handled. If this type is not reflected
    ReflectionType m_reflectionOverride;    

    Token m_name;           ///< The name of this scope/type
    Token m_super;          ///< Super class name
    Token m_marker;         ///< The marker associated with this scope (typically the marker is SLANG_CLASS etc, that is used to identify reflectedType)

    TypeSet* m_typeSet;     ///< The typeset this type belongs to. 

    Node* m_parentScope;    ///< The scope this type/scope is defined in
    Node* m_superNode;      ///< If this is a class/struct, the type it is derived from (or nullptr if base)
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

    SourceOrigin(SourceFile* sourceFile, const String& macroOrigin) :
        m_sourceFile(sourceFile),
        m_macroOrigin(macroOrigin)
    {}

         ///< The macro text is inserted into the macro to identify the origin. It is based on the filename
    String m_macroOrigin;
        /// The source file - also holds the path information
    SourceFile* m_sourceFile;
    
        /// All of the nodes defined in this file in the order they were defined
        /// Note that the same namespace may be listed multiple times.
    List<RefPtr<Node> > m_nodes;
};

class TypeSet : public RefObject
{
public:

        /// This is the looked up name.
    UnownedStringSlice m_macroName;    ///< The name extracted from the macro SLANG_ABSTRACT_AST_CLASS -> AST

    String m_typeName;           ///< The enum type name associated with this type for AST it is ASTNode
    String m_fileMark;           ///< This 'mark' becomes of the output filename

    List<Node*> m_baseTypes;    ///< The base types for this type set
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

        /// Find the name starting in specified scope
    Node* findNode(Node* scope, const UnownedStringSlice& name);

        /// Get all of the parsed source origins
    const List<RefPtr<SourceOrigin> >& getSourceOrigins() const { return m_origins; }

    TypeSet* getTypeSet(const UnownedStringSlice& slice);
    TypeSet* getOrAddTypeSet(const UnownedStringSlice& slice);

        /// Get all of the type sets
    const List<RefPtr<TypeSet>>& getTypeSets() const { return m_typeSets; }

        /// Get the root node
    Node* getRootNode() const { return m_rootNode; }

    CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup);

protected:
    static Node::Type _toNodeType(IdentifierStyle style);

    bool _isMarker(const UnownedStringSlice& name);

    SlangResult _parsePreDeclare();
    SlangResult _parseTypeSet();

    SlangResult _maybeParseNode(Node::Type type);
    SlangResult _maybeParseField();

    SlangResult _maybeParseType(UnownedStringSlice& outType);

    SlangResult _maybeParseType(UnownedStringSlice& outType, Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArgs(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArg(Index& ioTemplateDepth);

        /// Parse balanced - if a sink is set will report to that sink
    SlangResult _parseBalanced(DiagnosticSink* sink);

    SlangResult _calcDerivedTypesRec(Node* node);
    static String _calcMacroOrigin(const String& filePath, const Options& options);

        /// Concatenate all tokens from start to the current position
    UnownedStringSlice _concatTokens(TokenReader::ParsingCursor start);

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();

    TokenList m_tokenList;
    TokenReader m_reader;

    Node* m_currentNode;            ///< The current scope being processed

    RefPtr<Node> m_rootNode;        ///< The root scope 

    SourceOrigin* m_origin;

    DiagnosticSink* m_sink;

    NamePool* m_namePool;

    List<RefPtr<SourceOrigin>> m_origins;

    const Options* m_options;

    StringSlicePool m_typeSetPool;              ///< Pool for type set names
    List<RefPtr<TypeSet> > m_typeSets;          ///< The type sets

    IdentifierLookup* m_identifierLookup;
    StringSlicePool* m_typePool;                ///< Pool for just types
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

    if (child->m_name.hasContent())
    {
        m_childMap.Add(child->m_name.getContent(), child);
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
    if (isClassLike() && isReflected() && m_name.hasContent())
    {
        _indent(indentCount, out);
        out << m_name.getContent() << "\n";
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
            if (m_name.hasContent())
            {
                out << "namespace " << m_name.getContent() << " {\n";
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

            if (!isReflected())
            {
                out << " (";
            }
            out << m_name.getContent();
            if (!isReflected())
            {
                out << ") ";
            }

            if (m_super.hasContent())
            {
                out << " : " << m_super.getContent(); 
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
        if (field.isReflected())
        {
            _indent(indentCount + 1, out);
            out << field.type << " " << field.name.getContent() << "\n";
        }
    }

    _indent(indentCount, out);
    out << "}\n";
}

void Node::calcAbsoluteName(StringBuilder& outName) const
{
    List<Node*> path;
    calcScopePath(const_cast<Node*>(this), path);

    // 1 so we skip the global scope
    for (Index i = 1; i < path.getCount(); ++i)
    {
        Node* node = path[i];

        if (i > 1)
        {
            outName << "::";
        }

        if (node->m_type == Type::AnonymousNamespace)
        {
            outName << "{Anonymous}";
        }
        else
        {
            outName << node->m_name.getContent();
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
        if (type->isReflected())
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
        if (type->isReflected())
        {
            out.add(type);
        }
    }
}

/* static */void Node::filter(Filter inFilter, List<Node*>& ioNodes)
{
    // Filter out all the unreflected nodes
    Index count = ioNodes.getCount();
    for (Index j = 0; j < count; )
    {
        Node* node = ioNodes[j];

        if (!inFilter(node))
        {
            ioNodes.removeAt(j);
            count--;
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
        m_markPrefix = "SLANG_";
        m_markSuffix = "_CLASS";
    }

    bool m_defs = false;            ///< If set will output a '-defs.h' file for each of the input files, that corresponds to previous defs files (although doesn't have fields/RAW)
    bool m_dump = false;            ///< If true will dump to stderr the types/fields and hierarchy it extracted

    bool m_outputFields = false;     ///< When dumping macros also dump field definitions

    List<String> m_inputPaths;      ///< The input paths to the files to be processed
        
    String m_outputPath;            ///< The output path. Note that the extractor can generate multiple output files, and this will actually be the 'stem' of several files

    String m_inputDirectory;        ///< The input directory that is by default used for reading m_inputPaths from. 
    String m_markPrefix;            ///< The prefix of the 'marker' used to identify a reflected type
    String m_markSuffix;           ///< The postfix of the 'marker' used to identify a reflected type
    String m_stripFilePrefix;       ///< Used for the 'origin' information, this is stripped from the source filename, and the remainder of the filename (without extension) is 'macroized'
};

struct OptionsParser
{
    /// Parse the parameters. NOTE! Must have the program path removed
    SlangResult parse(int argc, const char*const* argv, DiagnosticSink* sink, Options& outOptions);

    SlangResult _parseArgWithValue(const char* option, String& outValue);
    SlangResult _parseArgReplaceValue(const char* option, String& outValue);
    SlangResult _parseArgFlag(const char* option, bool& outFlag);

    String m_reflectType;

    Index m_index;
    Int m_argCount;
    const char*const* m_args;
    DiagnosticSink* m_sink;
};

SlangResult OptionsParser::_parseArgFlag(const char* option, bool& outFlag)
{
     SLANG_ASSERT(UnownedStringSlice(m_args[m_index]) == option);
     SLANG_ASSERT(m_index < m_argCount);

     m_index ++;
     outFlag = true;
     return SLANG_OK;
}

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
            else if (arg == "-mark-prefix")
            {
                SLANG_RETURN_ON_FAIL(_parseArgReplaceValue("-mark-prefix", outOptions.m_markPrefix));
                continue;
            }
            else if (arg == "-mark-suffix")
            {
                SLANG_RETURN_ON_FAIL(_parseArgReplaceValue("-mark-suffix", outOptions.m_markSuffix));
                continue;
            }
            else if (arg == "-defs")
            {
                SLANG_RETURN_ON_FAIL(_parseArgFlag("-defs", outOptions.m_defs));
                continue; 
            }
            else if (arg == "-output-fields")
            {
                SLANG_RETURN_ON_FAIL(_parseArgFlag("-output-fields", outOptions.m_outputFields));
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

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CPPExtractor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

CPPExtractor::CPPExtractor(StringSlicePool* typePool, NamePool* namePool, DiagnosticSink* sink, IdentifierLookup* identifierLookup):
    m_typePool(typePool),
    m_sink(sink),
    m_namePool(namePool),
    m_identifierLookup(identifierLookup),
    m_typeSetPool(StringSlicePool::Style::Empty)
{
    m_rootNode = new Node(Node::Type::Namespace);
    m_rootNode->m_reflectionType = ReflectionType::Reflected;
}

TypeSet* CPPExtractor::getTypeSet(const UnownedStringSlice& slice)
{
    Index index = m_typeSetPool.findIndex(slice);
    if (index < 0)
    {
        return nullptr;
    }
    return m_typeSets[index];
}

TypeSet* CPPExtractor::getOrAddTypeSet(const UnownedStringSlice& slice)
{
    const Index index = Index(m_typeSetPool.add(slice));
    if (index >= m_typeSets.getCount())
    {
        SLANG_ASSERT(m_typeSets.getCount() == index);
        TypeSet* typeSet = new TypeSet;

        m_typeSets.add(typeSet);
        typeSet->m_macroName = m_typeSetPool.getSlice(StringSlicePool::Handle(index));
        return typeSet;
    }
    else
    {
        return m_typeSets[index];
    }
}

bool CPPExtractor::_isMarker(const UnownedStringSlice& name)
{
    return name.startsWith(m_options->m_markPrefix.getUnownedSlice()) && name.endsWith(m_options->m_markSuffix.getUnownedSlice());
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

bool CPPExtractor::advanceIfStyle(IdentifierStyle style, Token* outToken)
{
    if (m_reader.peekTokenType() == TokenType::Identifier)
    {
        IdentifierStyle readStyle = m_identifierLookup->get(m_reader.peekToken().getContent());
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

    if (node->m_name.hasContent())
    {
        // For anonymous namespace, we should look if we already have one and just reopen that. Doing so will mean will
        // find anonymous namespace clashes

        if (Node* foundNode = m_currentNode->findChild(node->m_name.getContent()))
        {
            if (node->isClassLike())
            {
                m_sink->diagnose(m_reader.peekToken(), CPPDiagnostics::typeAlreadyDeclared, node->m_name.getContent());
                m_sink->diagnose(foundNode->m_name, CPPDiagnostics::seeDeclarationOf, node->m_name.getContent());
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
                SLANG_RETURN_ON_FAIL(pushNode(node));
                SLANG_RETURN_ON_FAIL(popBrace());
                m_reader.advanceToken();
                return SLANG_OK;
            }
            default:
            {
                SLANG_RETURN_ON_FAIL(pushNode(node));
                return SLANG_OK;
            }
        }

        // If it's one of the markers, then we continue to extract parameter
        if (advanceIfMarker(&node->m_marker))
        {
            break;
        }

        // We still need to add the node,
        SLANG_RETURN_ON_FAIL(pushNode(node));
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
        node->m_typeSet = getOrAddTypeSet(slice);
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

// Returns true if there needs to be a spave between the previous token type, and the current token
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

UnownedStringSlice CPPExtractor::_concatTokens(TokenReader::ParsingCursor start)
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

    return m_typePool->getSlice(m_typePool->add(buf));
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

        const IdentifierStyle style = m_identifierLookup->get(identifierToken.getContent());
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
    outType = _concatTokens(startCursor);
    return SLANG_OK;
}

SlangResult CPPExtractor::_maybeParseType(UnownedStringSlice& outType)
{
    Index templateDepth = 0;
    SlangResult res = _maybeParseType(outType, templateDepth);
    if (SLANG_FAILED(res) && m_sink->getErrorCount())
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

SlangResult CPPExtractor::_parseBalanced(DiagnosticSink* sink)
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

SlangResult CPPExtractor::_maybeParseField()
{
    Node::Field field;

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

        typeName = m_typePool->getSlice( m_typePool->add(buf));
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
            Node::Field field;
            field.type = typeName;
            field.name = fieldName;
            field.reflectionType = m_currentNode->getContainedReflectionType();

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


SlangResult CPPExtractor::_parsePreDeclare()
{
    // Skip the declare type token
    m_reader.advanceToken();

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

    // Get the typeSet
    Token typeSetToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeSetToken));
    TypeSet* typeSet = getOrAddTypeSet(typeSetToken.getContent());

    SLANG_RETURN_ON_FAIL(expect(TokenType::Comma));

    // Get the type of type
    Node::Type nodeType;
    {
        Token typeToken;
        SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeToken));

        const IdentifierStyle style = m_identifierLookup->get(typeToken.getContent());

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

    RefPtr<Node> node(new Node(nodeType));

    node->m_name = name;
    node->m_super = super;
    node->m_typeSet = typeSet;

    // Assume it is reflected
    node->m_reflectionType = ReflectionType::Reflected;

    SLANG_RETURN_ON_FAIL(pushNode(node));
    // Pop out of the node
    popBrace();
    
    return SLANG_OK;
}

SlangResult CPPExtractor::_parseTypeSet()
{
    // Skip the declare type token
    m_reader.advanceToken();

    SLANG_RETURN_ON_FAIL(expect(TokenType::LParent));

    Token typeSetToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeSetToken));

    TypeSet* typeSet = getOrAddTypeSet(typeSetToken.getContent());

    SLANG_RETURN_ON_FAIL(expect(TokenType::Comma));

    // Get the type of type
    Token typeToken;
    SLANG_RETURN_ON_FAIL(expect(TokenType::Identifier, &typeToken));

    SLANG_RETURN_ON_FAIL(expect(TokenType::RParent));

    // Set the typename
    typeSet->m_typeName = typeToken.getContent();

    return SLANG_OK;
}

SlangResult CPPExtractor::parse(SourceFile* sourceFile, const Options* options)
{
    SLANG_ASSERT(options);
    m_options = options;

    // Calculate from the path, a 'macro origin' name. 
    const String macroOrigin = _calcMacroOrigin(sourceFile->getPathInfo().foundPath, *options);

    RefPtr<SourceOrigin> origin = new SourceOrigin(sourceFile, macroOrigin);
    m_origins.add(origin);

    // Set the current origin
    m_origin = origin;

    SourceManager* manager = sourceFile->getSourceManager();

    SourceView* sourceView = manager->createSourceView(sourceFile, nullptr, SourceLoc::fromRaw(0));

    Lexer lexer;

    m_currentNode = m_rootNode;

    lexer.initialize(sourceView, m_sink, m_namePool, manager->getMemoryArena());
    m_tokenList = lexer.lexAllTokens();
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
                const IdentifierStyle style = m_identifierLookup->get(m_reader.peekToken().getContent());
                
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
                        if (m_currentNode)
                        {
                            m_currentNode->m_reflectionOverride = ReflectionType::Reflected;
                        }
                        break;
                    }
                    case IdentifierStyle::Unreflected:
                    {
                        m_reader.advanceToken();
                        if (m_currentNode)
                        {
                            m_currentNode->m_reflectionOverride = ReflectionType::NotReflected;
                        }
                        break;
                    }
                    case IdentifierStyle::Access:
                    {
                        m_reader.advanceToken();
                        SLANG_RETURN_ON_FAIL(expect(TokenType::Colon));
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

Node* CPPExtractor::findNode(Node* scope, const UnownedStringSlice& name)
{
    // TODO(JS): We may want to lookup based on the path. 
    // If the name is qualified, we give up for not
    if (String(name).indexOf("::") >= 0)
    {
        return nullptr;
    }

    // Okay try in all scopes up to the root
    while (scope)
    {
        if (Node* node = scope->findChild(name))
        {
            return node;
        }

        scope = scope->m_parentScope;
    }

    return nullptr;
}

SlangResult CPPExtractor::_calcDerivedTypesRec(Node* node)
{
    if (node->isClassLike())
    {
        if (node->m_super.hasContent())
        {
            Node* parentScope = node->m_parentScope;
            if (parentScope == nullptr)
            {
                m_sink->diagnoseRaw(Severity::Error, UnownedStringSlice::fromLiteral("Can't lookup in scope if there is none!"));
                return SLANG_FAIL;
            }

            Node* superType = findNode(parentScope, node->m_super.getContent());

            if (!superType)
            {
                if (node->isReflected())
                {
                    m_sink->diagnose(node->m_name, CPPDiagnostics::superTypeNotFound, node->getAbsoluteName());
                    return SLANG_FAIL;
                }
            }
            else
            {
                if (!superType->isClassLike())
                {
                    m_sink->diagnose(node->m_name, CPPDiagnostics::superTypeNotAType, node->getAbsoluteName());
                    return SLANG_FAIL;
                }

                if (superType->m_typeSet != node->m_typeSet)
                {
                    m_sink->diagnose(node->m_name, CPPDiagnostics::typeInDifferentTypeSet, node->m_name.getContent(), node->m_typeSet->m_macroName, superType->m_typeSet->m_macroName);
                    return SLANG_FAIL;
                }

                // The base class must be defined in same scope (as we didn't allow different scopes for base classes)
                superType->addDerived(node);
            }
        }
        else
        {
            // Add the root nodes
            if (node->isReflected())
            {
                node->m_typeSet->m_baseTypes.add(node);
            }
        }
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

/* static */String CPPExtractor::_calcMacroOrigin(const String& filePath, const Options& options)
{
    // Get the filename without extension
    String fileName = Path::getFileNameWithoutExt(filePath);

    // We can work on just the slice
    UnownedStringSlice slice = fileName.getUnownedSlice();

    // Filename prefix
    if (options.m_stripFilePrefix.getLength() && slice.startsWith(options.m_stripFilePrefix.getUnownedSlice()))
    {
        const Index len = options.m_stripFilePrefix.getLength();
        slice = UnownedStringSlice(slice.begin() + len, slice.end());
    }

    // Trim -
    slice = slice.trim('-');

    StringBuilder out;
    NameConventionUtil::convert(slice, CharCase::Upper, NameConvention::Snake, out);
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
    SlangResult calcTypeHeader(CPPExtractor& extractor, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcChildrenHeader(CPPExtractor& exctractor, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcOriginHeader(CPPExtractor& extractor, StringBuilder& out);

    SlangResult calcDef(CPPExtractor& extractor, SourceOrigin* origin, StringBuilder& out);

    const Options& getOptions() const { return m_options; }

    CPPExtractorApp(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager),
        m_slicePool(StringSlicePool::Style::Default)
    {
        m_namePool.setRootNamePool(rootNamePool);
    }

protected:

        /// Called to set up identifier lookup. Must be performed after options are initials
    static void _initIdentifierLookup(const Options& options, IdentifierLookup& outLookup);

    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;
    
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
        if (File::exists(fileName))
        {
            String existingText;
            if (readAllText(fileName, existingText) == SLANG_OK)
            {
                if (existingText == text)
                    return SLANG_OK;
            }
        }
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
        if (node->isClassLike() && node->isReflected())
        {
            if (node->m_marker.getContent().indexOf(UnownedStringSlice::fromLiteral("ABSTRACT")) >= 0)
            {
                out << "ABSTRACT_";
            }

            out << "SYNTAX_CLASS(" << node->m_name.getContent() << ", " << node->m_super.getContent() << ")\n";
            out << "END_SYNTAX_CLASS()\n\n";
        }
    }
    return SLANG_OK;
}

SlangResult CPPExtractorApp::calcChildrenHeader(CPPExtractor& extractor, TypeSet* typeSet, StringBuilder& out)
{
    const List<Node*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    List<Node*> nodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        Node* baseType = baseTypes[i];        
        baseType->calcDerivedDepthFirst(nodes);
    }

    Node::filter(Node::isClassLike, nodes);

    List<Node*> derivedTypes;

    out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHILDREN !!!!!!!!!!!!!!!!!!!!!!!!!!!! */ \n\n";

    // Now the children 
    for (Node* node : nodes)
    {
        node->getReflectedDerivedTypes(derivedTypes);

        // Define the derived types
        out << "#define " << m_options.m_markPrefix << "CHILDREN_" << reflectTypeName << "_"  << node->m_name.getContent() << "(x, param)";

        if (derivedTypes.getCount())
        {
            out << " \\\n";
            for (Index j = 0; j < derivedTypes.getCount(); ++j)
            {
                Node* derivedType = derivedTypes[j];
                _indent(1, out);
                out << m_options.m_markPrefix << "ALL_" << reflectTypeName << "_" << derivedType->m_name.getContent() << "(x, param)";
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
        out << "#define " << m_options.m_markPrefix << "ALL_" << reflectTypeName << "_" << node->m_name.getContent() << "(x, param) \\\n";
        _indent(1, out);
        out << m_options.m_markPrefix << reflectTypeName << "_"  << node->m_name.getContent() << "(x, param)";

        // If has derived types output them
        if (node->hasReflectedDerivedType())
        {
            out << " \\\n";
            _indent(1, out);
            out << m_options.m_markPrefix << "CHILDREN_" << reflectTypeName << "_" << node->m_name.getContent() << "(x, param)";
        }
        out << "\n\n";
    }

    if (m_options.m_outputFields)
    {
        out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

        for (Node* node : nodes)
        {
            // Define the derived types
            out << "#define " << m_options.m_markPrefix << "FIELDS_" << reflectTypeName << "_" << node->m_name.getContent() << "(_x_, _param_)";

            if (node->m_fields.getCount() > 0)
            {
                out << "\\\n";

                const Index fieldsCount = node->m_fields.getCount();
                bool previousField = false;
                for (Index j = 0; j < fieldsCount; ++j) 
                {
                    const auto& field = node->m_fields[j];
                        
                    if (field.isReflected())
                    {
                        if (previousField)
                        {
                            out << "\\\n";
                        }

                        _indent(1, out);

                        // NOTE! We put the type field in brackets, such that there is no issue with templates containing a comma.
                        // If stringified
                        out << "_x_(" << field.name.getContent() << ", (" << field.type << "), _param_)";
                        previousField = true;
                    }
                }
            }
                
            out << "\n\n";
        }
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::calcOriginHeader(CPPExtractor& extractor, StringBuilder& out)
{
    // Do macros by origin

    out << "// Origin macros\n\n";

    for (SourceOrigin* origin : extractor.getSourceOrigins())
    {   
        out << "#define " << m_options.m_markPrefix << "ORIGIN_" << origin->m_macroOrigin << "(x, param) \\\n";

        for (Node* node : origin->m_nodes)
        {
            if (!(node->isReflected() && node->isClassLike()))
            {
                continue;
            }

            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", param) \\\n";
        }
        out << "/* */\n\n";
    }

    return SLANG_OK;
}

SlangResult CPPExtractorApp::calcTypeHeader(CPPExtractor& extractor, TypeSet* typeSet, StringBuilder& out)
{
    const List<Node*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    if (baseTypes.getCount() == 0)
    {
        return SLANG_OK;
    }

    // Set up the scope
    List<Node*> baseScopePath;
    baseTypes[0]->calcScopePath(baseScopePath);

    // Remove the global scope
    baseScopePath.removeAt(0);
    // Remove the type itself
    baseScopePath.removeLast();

    for (Node* scopeNode : baseScopePath)
    {
        SLANG_ASSERT(scopeNode->m_type == Node::Type::Namespace);
        out << "namespace " << scopeNode->m_name.getContent() << " {\n";
    }

    // Add all the base types, with in order traversals
    List<Node*> nodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        Node* baseType = baseTypes[i];
        baseType->calcDerivedDepthFirst(nodes);
    }

    Node::filter(Node::isClassLikeAndReflected, nodes);

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
            out << node->m_name.getContent() << " = " << typeIndex << ",\n";
            typeIndex++;
        }

        _indent(1, out);
        out << "CountOf\n";

        out << "};\n\n";
    }

    // TODO(JS):
    // Strictly speaking if we wanted the types to be in different scopes, we would have to
    // change the namespaces here

    // Predeclare the classes
    {
        out << "// Predeclare\n\n";
        for (Node* node : nodes)
        {
            SLANG_ASSERT(node->isClassLike());
            // If it's not reflected we don't output, in the enum list
            if (node->isReflected())
            {
                const char* type = (node->m_type == Node::Type::ClassType) ? "class" : "struct";
                out << type << " " << node->m_name.getContent() << ";\n";
            }
        }
    }

    // Do the macros for each of the types

    {
        out << "// Type macros\n\n";

        out << "// Order is (NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \n";
        out << "// NAME - is the class name\n";
        out << "// SUPER - is the super class name (or NO_SUPER)\n";
        out << "// ORIGIN - where the definition was found\n";
        out << "// LAST - is the class name for the last in the range (or NO_LAST)\n";
        out << "// MARKER - is the text inbetween in the prefix/postix (like ABSTRACT). If no inbetween text is is 'NONE'\n";
        out << "// TYPE - Can be BASE, INNER or LEAF for the overall base class, an INNER class, or a LEAF class\n";
        out << "// param is a user defined parameter that can be parsed to the invoked x macro\n\n";

        // Output all of the definitions for each type
        for (Node* node : nodes)
        {
            out << "#define " << m_options.m_markPrefix <<  reflectTypeName << "_" << node->m_name.getContent() << "(x, param) ";

            // Output the X macro part
            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", ";

            if (node->m_superNode)
            {
                out << node->m_superNode->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_SUPER, ";
            }

            // Output the (file origin)
            out << node->m_origin->m_macroOrigin;
            out << ", ";

            // The last type
            Node* lastDerived = node->findLastDerived();
            if (lastDerived)
            {
                out << lastDerived->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_LAST, ";
            }

            // Output any specifics of the markup
            UnownedStringSlice marker = node->m_marker.getContent();
            // Need to extract the name
            if (marker.getLength() > m_options.m_markPrefix.getLength() + m_options.m_markSuffix.getLength())
            {
                marker = UnownedStringSlice(marker.begin() + m_options.m_markPrefix.getLength(), marker.end() - m_options.m_markSuffix.getLength());
            }
            else
            {
                marker = UnownedStringSlice::fromLiteral("NONE");
            }
            out << marker << ", ";

            if (node->m_superNode == nullptr)
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
        out << "} // namespace " << scopeNode->m_name.getContent() << "\n";
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

    for (TypeSet* typeSet : extractor.getTypeSets())
    {
        {
            /// Calculate the header
            StringBuilder header;
            SLANG_RETURN_ON_FAIL(calcTypeHeader(extractor, typeSet, header));

            // Write it out

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "." << ext;
            SLANG_RETURN_ON_FAIL(writeAllText(headerPath, header.getUnownedSlice()));
        }

        {
            StringBuilder childrenHeader;
            SLANG_RETURN_ON_FAIL(calcChildrenHeader(extractor, typeSet, childrenHeader));

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "-macro." + ext;
            SLANG_RETURN_ON_FAIL(writeAllText(headerPath, childrenHeader.getUnownedSlice()));
        }
    }

    return SLANG_OK;
}

/* static */void CPPExtractorApp::_initIdentifierLookup(const Options& options, IdentifierLookup& outLookup)
{
    outLookup.reset();

    // Some keywords
    {
        const char* names[] = { "virtual", "typedef", "continue", "if", "case", "break", "catch", "default", "delete", "do", "else", "for", "new", "goto", "return", "switch", "throw", "using", "while", "operator" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Keyword);
    }

    // Type modifier keywords
    {
        const char* names[] = { "const", "volatile" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::TypeModifier);
    }

    // Special markers
    {
        const char* names[] = {"PRE_DECLARE", "TYPE_SET", "REFLECTED", "UNREFLECTED"};
        const IdentifierStyle styles[] = { IdentifierStyle::PreDeclare, IdentifierStyle::TypeSet, IdentifierStyle::Reflected, IdentifierStyle::Unreflected };
        SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(names) == SLANG_COUNT_OF(styles));

        StringBuilder buf;        
        for (Index i = 0; i < SLANG_COUNT_OF(names); ++i)
        {
            buf.Clear();
            buf << options.m_markPrefix << names[i];
            outLookup.set(buf.getUnownedSlice(), styles[i]);
        }
    }

    // Keywords which introduce types/scopes
    {
        outLookup.set("struct", IdentifierStyle::Struct);
        outLookup.set("class", IdentifierStyle::Class);
        outLookup.set("namespace", IdentifierStyle::Namespace);
    }

    // Keywords that control access
    {
        const char* names[] = { "private", "protected", "public" };
        outLookup.set(names, SLANG_COUNT_OF(names), IdentifierStyle::Access);
    }
}

SlangResult CPPExtractorApp::execute(const Options& options)
{
    m_options = options;

    IdentifierLookup identifierLookup;
    _initIdentifierLookup(options, identifierLookup);

    CPPExtractor extractor(&m_slicePool, &m_namePool, m_sink, &identifierLookup);

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

    // Okay let's check out the typeSets
    {
        for (TypeSet* typeSet : extractor.getTypeSets())
        {
            // The macro name is in upper snake, so split it 
            List<UnownedStringSlice> slices;
            NameConventionUtil::split(typeSet->m_macroName, slices);

            if (typeSet->m_fileMark.getLength() == 0)
            {
                StringBuilder buf;
                // Let's guess a 'fileMark' (it becomes part of the filename) based on the macro name. Use lower kabab.
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Lower, NameConvention::Kabab, buf);
                typeSet->m_fileMark = buf.ProduceString();
            }

            if (typeSet->m_typeName.getLength() == 0)
            {
                // Let's guess a typename if not set -> go with upper camel
                StringBuilder buf;
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Upper, NameConvention::Camel, buf);
                typeSet->m_typeName = buf.ProduceString();
            }
        }
    }

    // Dump out the tree
    if (options.m_dump)
    {
        {
            StringBuilder buf;
            extractor.getRootNode()->dump(0, buf);
            m_sink->writer->write(buf.getBuffer(), buf.getLength());
        }

        for (TypeSet* typeSet : extractor.getTypeSets())
        {
            const List<Node*>& baseTypes = typeSet->m_baseTypes;

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
        ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

        RootNamePool rootNamePool;

        SourceManager sourceManager;
        sourceManager.initialize(nullptr, nullptr);

        DiagnosticSink sink(&sourceManager);
        sink.writer = writer;

        CPPExtractorApp app(&sink, &sourceManager, &rootNamePool);

        try
        {
            if (SLANG_FAILED(app.executeWithArgs(argc - 1, argv + 1)))
            {
                sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
                return 1;
            }
            if (sink.getErrorCount())
            {
                sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
                return 1;
            }
        }
        catch (...)
        {
            sink.diagnose(SourceLoc(), CPPDiagnostics::internalError);
            return 1;
        }
    }
    return 0;
}

