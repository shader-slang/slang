#ifndef CPP_EXTRACT_PARSER_H
#define CPP_EXTRACT_PARSER_H

#include "diagnostics.h"
#include "node.h"
#include "identifier-lookup.h"
#include "node-tree.h"

#include "../../source/compiler-core/slang-lexer.h"

namespace CppExtract {
using namespace Slang;

class Parser
{
public:

    typedef uint32_t NodeTypeBitType;

    SlangResult expect(TokenType type, Token* outToken = nullptr);

    bool advanceIfMarker(Token* outToken = nullptr);
    bool advanceIfToken(TokenType type, Token* outToken = nullptr);
    bool advanceIfStyle(IdentifierStyle style, Token* outToken = nullptr);

    SlangResult pushAnonymousNamespace();
    SlangResult pushScope(ScopeNode* node);
    SlangResult consumeToClosingBrace(const Token* openBraceToken = nullptr);
    SlangResult popScope();

        /// Parse the contents of the source file
    SlangResult parse(SourceOrigin* sourceOrigin, const Options* options);

    void setTypeEnabled(Node::Type type, bool isEnabled = true);
    bool isTypeEnabled(Node::Type type) { return (m_nodeTypeEnabled & (NodeTypeBitType(1) << int(type))) != 0; }
    void setTypesEnabled(const Node::Type* types, Index typesCount, bool isEnabled = true);

    Parser(NodeTree* nodeTree, DiagnosticSink* sink);

protected:
    static Node::Type _toNodeType(IdentifierStyle style);

    bool _isMarker(const UnownedStringSlice& name);

    SlangResult _maybeConsumeScope();

    SlangResult _parsePreDeclare();
    SlangResult _parseTypeSet();

    SlangResult _maybeParseNode(Node::Type type);
    SlangResult _maybeParseField();

    SlangResult _parseTypeDef();
    SlangResult _parseEnum();

    SlangResult _maybeParseType(List<Token>& outToks);
    SlangResult _maybeParseType(UnownedStringSlice& outType);

    SlangResult _maybeParseType(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArgs(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArg(Index& ioTemplateDepth);

        /// Parse balanced - if a sink is set will report to that sink
    SlangResult _parseBalanced(DiagnosticSink* sink);

        /// Concatenate all tokens from start to the current position
    UnownedStringSlice _concatTokens(TokenReader::ParsingCursor start);

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();

    NodeTypeBitType m_nodeTypeEnabled;

    TokenList m_tokenList;
    TokenReader m_reader;

    ScopeNode* m_currentScope;          ///< The current scope being processed
    SourceOrigin* m_sourceOrigin;       ///< The source origin that all tokens are in

    DiagnosticSink* m_sink;             ///< Diagnostic sink 

    NodeTree* m_nodeTree;    ///< Shared state between parses. Nodes will be added to this

    const Options* m_options;
};

} // CppExtract

#endif
