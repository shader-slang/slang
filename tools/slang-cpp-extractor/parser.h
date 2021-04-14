#ifndef CPP_EXTRACT_PARSER_H
#define CPP_EXTRACT_PARSER_H

#include "diagnostics.h"
#include "node.h"
#include "identifier-lookup.h"

#include "../../source/compiler-core/slang-lexer.h"

namespace SlangExperimental {
using namespace Slang;

class TypeSet : public RefObject
{
public:

    /// This is the looked up name.
    UnownedStringSlice m_macroName;    ///< The name extracted from the macro SLANG_ABSTRACT_AST_CLASS -> AST

    String m_typeName;           ///< The enum type name associated with this type for AST it is ASTNode
    String m_fileMark;           ///< This 'mark' becomes of the output filename

    List<ClassLikeNode*> m_baseTypes;    ///< The base types for this type set
};

class SourceOrigin : public RefObject
{
public:

    void addNode(Node* node)
    {
        if (auto classLike = as<ClassLikeNode>(node))
        {
            SLANG_ASSERT(classLike->m_origin == nullptr);
            classLike->m_origin = this;
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

struct Options;
class IdentifierLookup;

class CPPExtractor
{
public:

    SlangResult expect(TokenType type, Token* outToken = nullptr);

    bool advanceIfMarker(Token* outToken = nullptr);
    bool advanceIfToken(TokenType type, Token* outToken = nullptr);
    bool advanceIfStyle(IdentifierStyle style, Token* outToken = nullptr);

    SlangResult pushAnonymousNamespace();
    SlangResult pushScope(ScopeNode* node);
    SlangResult consumeToClosingBrace(const Token* openBraceToken = nullptr);
    SlangResult popScope();

    /// Parse the contents of the source file
    SlangResult parse(SourceFile* sourceFile, const Options* options);

    /// When parsing we don't lookup all up super types/add derived types. This is because
    /// we allow files to be processed in any order, so we have to do the type lookup as a separate operation
    SlangResult calcDerivedTypes();

    /// Find the name starting in specified scope
    Node* findNode(ScopeNode* scope, const UnownedStringSlice& name);

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

    SlangResult _calcDerivedTypesRec(ScopeNode* node);
    static String _calcMacroOrigin(const String& filePath, const Options& options);

    /// Concatenate all tokens from start to the current position
    UnownedStringSlice _concatTokens(TokenReader::ParsingCursor start);

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();

    TokenList m_tokenList;
    TokenReader m_reader;

    ScopeNode* m_currentScope;      ///< The current scope being processed

    RefPtr<ScopeNode> m_rootNode;   ///< The root scope 

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

} // SlangExperimental

#endif
