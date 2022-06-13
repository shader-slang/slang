#pragma once

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"
#include "../../slang.h"

#include "../../source/core/slang-rtti-info.h"
#include "../../source/compiler-core/slang-json-value.h"

namespace Slang
{
namespace LanguageServerProtocol
{
struct ServerInfo
{
    String name;
    String version;

    static const StructRttiInfo g_rttiInfo;
};

enum class TextDocumentSyncKind
{
    None = 0,
    Full = 1,
    Incremental = 2
};

struct TextDocumentSyncOptions
{
    bool openClose;
    int32_t change; // TextDocumentSyncKind
    static const StructRttiInfo g_rttiInfo;
};

struct WorkDoneProgressParams
{
    /**
     * An optional token that a server can use to report work done progress.
     */
    String workDoneToken; // optional

    static const StructRttiInfo g_rttiInfo;
};

struct CompletionOptions : public WorkDoneProgressParams
{
    /**
     * Most tools trigger completion request automatically without explicitly
     * requesting it using a keyboard shortcut (e.g. Ctrl+Space). Typically they
     * do so when the user starts to type an identifier. For example if the user
     * types `c` in a JavaScript file code complete will automatically pop up
     * present `console` besides others as a completion item. Characters that
     * make up identifiers don't need to be listed here.
     *
     * If code complete should automatically be trigger on characters not being
     * valid inside an identifier (for example `.` in JavaScript) list them in
     * `triggerCharacters`.
     */
    List<String> triggerCharacters;

    /**
     * The list of all possible characters that commit a completion. This field
     * can be used if clients don't support individual commit characters per
     * completion item. See client capability
     * `completion.completionItem.commitCharactersSupport`.
     *
     * If a server provides both `allCommitCharacters` and commit characters on
     * an individual completion item the ones on the completion item win.
     *
     * @since 3.2.0
     */
    List<String> allCommitCharacters;

    /**
     * The server provides support to resolve additional
     * information for a completion item.
     */
    bool resolveProvider;

    static const StructRttiInfo g_rttiInfo;
};

struct SemanticTokensLegend
{
    /**
     * The token types a server uses.
     */
    List<String> tokenTypes;

    /**
     * The token modifiers a server uses.
     */
    List<String> tokenModifiers;

    static const StructRttiInfo g_rttiInfo;
};


struct SemanticTokensOptions
{
    /**
     * The legend used by the server
     */
    SemanticTokensLegend legend;

    /**
     * Server supports providing semantic tokens for a specific range
     * of a document.
     */
    bool range;

    /**
     * Server supports providing semantic tokens for a full document.
     */
    bool full;

    static const StructRttiInfo g_rttiInfo;
};

struct SignatureHelpOptions
{
    /**
     * The characters that trigger signature help
     * automatically.
     */
    List<String> triggerCharacters;

    /**
     * List of characters that re-trigger signature help.
     *
     * These trigger characters are only active when signature help is already
     * showing. All trigger characters are also counted as re-trigger
     * characters.
     *
     * @since 3.15.0
     */
    List<String> retriggerCharacters;

    static const StructRttiInfo g_rttiInfo;
};

struct TextDocumentItem
{
    String uri;
    String languageId;
    int version;
    String text;
    static const StructRttiInfo g_rttiInfo;
};

struct TextDocumentIdentifier
{
    String uri;
    static const StructRttiInfo g_rttiInfo;
};

struct VersionedTextDocumentIdentifier
{
    String uri;
    int version;
    static const StructRttiInfo g_rttiInfo;
};

struct Position
{
    int line = -1;
    int character = -1;
    static const StructRttiInfo g_rttiInfo;
};

struct Range
{
    Position start;
    Position end;
    static const StructRttiInfo g_rttiInfo;
};

struct DidOpenTextDocumentParams
{
    TextDocumentItem textDocument;
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

struct TextDocumentContentChangeEvent
{
    Range range; // optional
    String text;
    static const StructRttiInfo g_rttiInfo;
};

struct DidChangeTextDocumentParams
{
    VersionedTextDocumentIdentifier textDocument;
    List<TextDocumentContentChangeEvent> contentChanges;
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

struct DidCloseTextDocumentParams
{
    TextDocumentIdentifier textDocument;
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

struct ServerCapabilities
{
    String positionEncoding;
    TextDocumentSyncOptions textDocumentSync;
    bool hoverProvider;
    bool definitionProvider;
    CompletionOptions completionProvider;
    SemanticTokensOptions semanticTokensProvider;
    SignatureHelpOptions signatureHelpProvider;
    static const StructRttiInfo g_rttiInfo;
};

struct WorkspaceFolder
{
    String uri;
    String name;
    static const StructRttiInfo g_rttiInfo;
};

struct InitializeParams
{
    List<WorkspaceFolder> workspaceFolders;
    static const UnownedStringSlice methodName;
    static const StructRttiInfo g_rttiInfo;
};

struct NullResponse
{
    static const StructRttiInfo g_rttiInfo;
    static NullResponse* get();
};

struct InitializeResult
{
    ServerCapabilities capabilities;
    ServerInfo serverInfo;

    static const StructRttiInfo g_rttiInfo;
};

struct ShutdownParams {
    static const UnownedStringSlice methodName;
};

struct ExitParams {
    static const UnownedStringSlice methodName;
};

typedef uint32_t DiagnosticSeverity;
/**
 * Reports an error.
 */
const DiagnosticSeverity kDiagnosticsSeverityError = 1;
/**
 * Reports a warning.
 */
const DiagnosticSeverity kDiagnosticsSeverityWarning = 2;
/**
 * Reports an information.
 */
const DiagnosticSeverity kDiagnosticsSeverityInformation = 3;
/**
 * Reports a hint.
 */
const DiagnosticSeverity kDiagnosticsSeverityHint = 4;


struct Location
{
    String uri;
    Range range;
    static const StructRttiInfo g_rttiInfo;
};

struct DiagnosticRelatedInformation
{
    /**
     * The location of this related diagnostic information.
     */
    Location location;

    /**
     * The message of this related diagnostic information.
     */
    String message;

    static const StructRttiInfo g_rttiInfo;
};

struct Diagnostic
{
    /**
     * The range at which the message applies.
     */
    Range range;

    /**
     * The diagnostic's severity. Can be omitted. If omitted it is up to the
     * client to interpret diagnostics as error, warning, info or hint.
     */
    DiagnosticSeverity severity;

    /**
     * The diagnostic's code, which might appear in the user interface.
     */
    int32_t code;

    /**
     * A human-readable string describing the source of this
     * diagnostic, e.g. 'typescript' or 'super lint'.
     */
    String source;

    /**
     * The diagnostic's message.
     */
    String message;

    /**
     * An array of related diagnostic information, e.g. when symbol-names within
     * a scope collide all definitions can be marked via this property.
     */
    List<DiagnosticRelatedInformation> relatedInformation;

    bool operator==(const Diagnostic& other) const
    {
        return code == other.code && range.start.line == other.range.start.line &&
               message == other.message;
    }

    HashCode getHashCode() const
    {
        return combineHash(
            code, combineHash(range.start.line, message.getHashCode()));
    }

    static const StructRttiInfo g_rttiInfo;
};

struct PublishDiagnosticsParams
{
    /**
     * The URI for which diagnostic information is reported.
     */
    String uri;

    /**
     * An array of diagnostic information items.
     */
    List<Diagnostic> diagnostics;

    static const StructRttiInfo g_rttiInfo;
};

struct TextDocumentPositionParams
{
    /**
     * The text document.
     */
    TextDocumentIdentifier textDocument;

    /**
     * The position inside the text document.
     */
    Position position;

    static const StructRttiInfo g_rttiInfo;
};

struct HoverParams
    : WorkDoneProgressParams
    ,TextDocumentPositionParams 
{
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

struct DefinitionParams
    : WorkDoneProgressParams
    , TextDocumentPositionParams
{
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

struct MarkupContent
{
    /**
     * The type of the Markup
     */
    String kind;

    /**
     * The content itself
     */
    String value;

    static const StructRttiInfo g_rttiInfo;
};

struct Hover
{
    /**
     * The hover's content
     */
    MarkupContent contents;

    /**
     * An optional range is a range inside a text document
     * that is used to visualize a hover, e.g. by changing the background color.
     */
    Range range;

    static const StructRttiInfo g_rttiInfo;
};

struct CompletionParams
    : WorkDoneProgressParams
    , TextDocumentPositionParams
{
    static const StructRttiInfo g_rttiInfo;
    static const UnownedStringSlice methodName;
};

typedef int32_t CompletionItemKind;
const CompletionItemKind kCompletionItemKindText = 1;
const CompletionItemKind kCompletionItemKindMethod = 2;
const CompletionItemKind kCompletionItemKindFunction = 3;
const CompletionItemKind kCompletionItemKindConstructor = 4;
const CompletionItemKind kCompletionItemKindField = 5;
const CompletionItemKind kCompletionItemKindVariable = 6;
const CompletionItemKind kCompletionItemKindClass = 7;
const CompletionItemKind kCompletionItemKindInterface = 8;
const CompletionItemKind kCompletionItemKindModule = 9;
const CompletionItemKind kCompletionItemKindProperty = 10;
const CompletionItemKind kCompletionItemKindUnit = 11;
const CompletionItemKind kCompletionItemKindValue = 12;
const CompletionItemKind kCompletionItemKindEnum = 13;
const CompletionItemKind kCompletionItemKindKeyword = 14;
const CompletionItemKind kCompletionItemKindSnippet = 15;
const CompletionItemKind kCompletionItemKindColor = 16;
const CompletionItemKind kCompletionItemKindFile = 17;
const CompletionItemKind kCompletionItemKindReference = 18;
const CompletionItemKind kCompletionItemKindFolder = 19;
const CompletionItemKind kCompletionItemKindEnumMember = 20;
const CompletionItemKind kCompletionItemKindConstant = 21;
const CompletionItemKind kCompletionItemKindStruct = 22;
const CompletionItemKind kCompletionItemKindEvent = 23;
const CompletionItemKind kCompletionItemKindOperator = 24;
const CompletionItemKind kCompletionItemKindTypeParameter = 25;

struct CompletionItem
{
    /**
     * The label of this completion item.
     *
     * The label property is also by default the text that
     * is inserted when selecting this completion.
     *
     * If label details are provided the label itself should
     * be an unqualified name of the completion item.
     */
    String label;

    /**
     * The kind of this completion item. Based of the kind
     * an icon is chosen by the editor. The standardized set
     * of available values is defined in `CompletionItemKind`.
     */
    CompletionItemKind kind;

    /**
     * A human-readable string with additional information
     * about this item, like type or symbol information.
     */
    String detail;

    /**
     * A human-readable string that represents a doc-comment.
     */
    MarkupContent documentation;

    /**
     * An optional set of characters that when pressed while this completion is
     * active will accept it first and then type that character. *Note* that all
     * commit characters should have `length=1` and that superfluous characters
     * will be ignored.
     */
    List<String> commitCharacters;

    // Additional data.
    String data;

    static const StructRttiInfo g_rttiInfo;
};

struct SemanticTokensParams : WorkDoneProgressParams
{
    TextDocumentIdentifier textDocument;

    static const UnownedStringSlice methodName;

    static const StructRttiInfo g_rttiInfo;
};


struct SemanticTokens
{
    /**
     * An optional result id. If provided and clients support delta updating
     * the client will include the result id in the next semantic token request.
     * A server can then instead of computing all semantic tokens again simply
     * send a delta.
     */
    String resultId;

    /**
     * The actual tokens.
     */
    List<uint32_t> data;

    static const StructRttiInfo g_rttiInfo;
};

struct SignatureHelpParams
    : WorkDoneProgressParams
    , TextDocumentPositionParams
{
    static const UnownedStringSlice methodName;

    static const StructRttiInfo g_rttiInfo;
};

/**
 * Represents a parameter of a callable-signature. A parameter can
 * have a label and a doc-comment.
 */
struct ParameterInformation
{
    /**
     * The label of this parameter information.
     *
     * Either a string or an inclusive start and exclusive end offsets within
     * its containing signature label. (see SignatureInformation.label). The
     * offsets are based on a UTF-16 string representation as `Position` and
     * `Range` does.
     *
     * *Note*: a label of type string should be a substring of its containing
     * signature label. Its intended use case is to highlight the parameter
     * label part in the `SignatureInformation.label`.
     */
    uint32_t label[2];

    /**
     * The human-readable doc-comment of this parameter. Will be shown
     * in the UI but can be omitted.
     */
    MarkupContent documentation;

    static const StructRttiInfo g_rttiInfo;
};

/**
 * Represents the signature of something callable. A signature
 * can have a label, like a function-name, a doc-comment, and
 * a set of parameters.
 */
struct SignatureInformation
{
    /**
     * The label of this signature. Will be shown in
     * the UI.
     */
    String label;

    /**
     * The human-readable doc-comment of this signature. Will be shown
     * in the UI but can be omitted.
     */
    MarkupContent documentation;

    /**
     * The parameters of this signature.
     */
    List<ParameterInformation> parameters;

    static const StructRttiInfo g_rttiInfo;
};

struct SignatureHelp
{
    /**
     * One or more signatures. If no signatures are available the signature help
     * request should return `null`.
     */
    List<SignatureInformation> signatures;

    /**
     * The active signature. If omitted or the value lies outside the
     * range of `signatures` the value defaults to zero or is ignore if
     * the `SignatureHelp` as no signatures.
     *
     * Whenever possible implementors should make an active decision about
     * the active signature and shouldn't rely on a default value.
     *
     * In future version of the protocol this property might become
     * mandatory to better express this.
     */
    uint32_t activeSignature;

    /**
     * The active parameter of the active signature. If omitted or the value
     * lies outside the range of `signatures[activeSignature].parameters`
     * defaults to 0 if the active signature has parameters. If
     * the active signature has no parameters it is ignored.
     * In future version of the protocol this property might become
     * mandatory to better express the active parameter if the
     * active signature does have any.
     */
    uint32_t activeParameter;

    static const StructRttiInfo g_rttiInfo;
};


} // namespace LanguageServerProtocol
} // namespace Slang
