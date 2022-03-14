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

    void setKindEnabled(Node::Kind kind, bool isEnabled = true);
    bool isTypeEnabled(Node::Kind kind) { return (m_nodeTypeEnabled & (NodeTypeBitType(1) << int(kind))) != 0; }

        /// If set classes require a 'marker' to be reflected
    void setRequireMarker(bool requireMarker) { m_requireMarker = requireMarker;  }
    bool getRequireMarker() const { return m_requireMarker; }

    void setKindsEnabled(const Node::Kind* kinds, Index kindsCount, bool isEnabled = true);

    Parser(NodeTree* nodeTree, DiagnosticSink* sink);

protected:
    static Node::Kind _toNodeKind(IdentifierStyle style);

    bool _isMarker(const UnownedStringSlice& name);

    SlangResult _maybeConsumeScope();

    SlangResult _parsePreDeclare();
    SlangResult _parseTypeSet();

    SlangResult _maybeParseNode(Node::Kind kind);
    SlangResult _maybeParseContained(Node** outNode);

    SlangResult _parseTypeDef();
    SlangResult _parseEnum();
    SlangResult _parseMarker();

    SlangResult _maybeParseType(List<Token>& outToks);
    SlangResult _maybeParseType(UnownedStringSlice& outType);

    SlangResult _parseExpression(List<Token>& outExprTokens);

    SlangResult _maybeParseType(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArgs(Index& ioTemplateDepth);
    SlangResult _maybeParseTemplateArg(Index& ioTemplateDepth);

        /// Parse balanced - if a sink is set will report to that sink
    SlangResult _parseBalanced(DiagnosticSink* sink);

        /// Concatenate all tokens from start to the current position
    UnownedStringSlice _concatTokens(TokenReader::ParsingCursor start);

        /// Consume what looks like a template definition
    SlangResult _consumeTemplate();

    void _consumeTypeModifiers();

    SlangResult _consumeToSync();
        /// Consumes balanced parens. Will return an error if not matched. Assumes starts on opening (
    SlangResult _consumeBalancedParens();

    NodeTypeBitType m_nodeTypeEnabled;

    TokenList m_tokenList;
    TokenReader m_reader;

    ScopeNode* m_currentScope;          ///< The current scope being processed
    SourceOrigin* m_sourceOrigin;       ///< The source origin that all tokens are in

    DiagnosticSink* m_sink;             ///< Diagnostic sink 

    NodeTree* m_nodeTree;    ///< Shared state between parses. Nodes will be added to this

    bool m_requireMarker = true;

    const Options* m_options;
};

} // CppExtract

#endif
