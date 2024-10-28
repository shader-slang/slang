#pragma once

#include <slang.h>
#include <unordered_map>
#include <emscripten/val.h>

namespace Slang
{
    class LanguageServerCore;
}

namespace slang
{
namespace wgsl
{

class Error
{
public:
    // Can be
    // "USER": User did not call the API correctly
    // "INTERNAL": Slang failed due to a bug
    std::string type;
    std::string message;
    SlangResult result;
};

Error getLastError();

class CompileTargets
{
public:
    CompileTargets();
    int findCompileTarget(const std::string& name);
private:
    std::unordered_map<std::string, SlangCompileTarget> m_compileTargetMap;
};

CompileTargets* getCompileTargets();

class ComponentType
{
public:

    ComponentType(slang::IComponentType* interface) :
        m_interface(interface) {}

    ComponentType* link();

    std::string getEntryPointCode(int entryPointIndex, int targetIndex);
    emscripten::val getEntryPointCodeBlob(int entryPointIndex, int targetIndex);
    std::string getTargetCode(int targetIndex);
    emscripten::val getTargetCodeBlob(int targetIndex);

    slang::IComponentType* interface() const {return m_interface;}

    virtual ~ComponentType() = default;

private:

    Slang::ComPtr<slang::IComponentType> m_interface;
};

class EntryPoint : public ComponentType
{
public:
    EntryPoint(slang::IEntryPoint* interface) : ComponentType(interface) {}
    std::string getName() const
    {
        return entryPointInterface()->getFunctionReflection()->getName();
    }
private:

    slang::IEntryPoint* entryPointInterface() const {
        return static_cast<slang::IEntryPoint*>(interface());
    }
};

class Module : public ComponentType
{
public:

    Module(slang::IModule* interface) : ComponentType(interface) {}

    EntryPoint* findEntryPointByName(const std::string& name);
    EntryPoint* findAndCheckEntryPoint(const std::string& name, int stage);
    EntryPoint* getDefinedEntryPoint(int index);
    int getDefinedEntryPointCount();

    slang::IModule* moduleInterface() const {
        return static_cast<slang::IModule*>(interface());
    }
};

class Session
{
public:

    Session(slang::ISession* interface)
        : m_interface(interface) {}

    Module* loadModuleFromSource(
        const std::string& slangCode, const std::string& name, const std::string& path);

    ComponentType* createCompositeComponentType(
        const std::vector<ComponentType*>& components);

    slang::ISession* interface() const {return m_interface;}

private:

    Slang::ComPtr<slang::ISession> m_interface;
};

class GlobalSession
{
public:

    GlobalSession(slang::IGlobalSession* interface)
        : m_interface(interface) {}

    Session* createSession(int compileTarget);

    slang::IGlobalSession* interface() const {return m_interface;}

private:

    Slang::ComPtr<slang::IGlobalSession> m_interface;
};

GlobalSession* createGlobalSession();

namespace lsp
{
    struct Position
    {
        int line = -1;
        int character = -1;
    };

    struct Range
    {
        Position start;
        Position end;
    };

    struct Location
    {
        std::string uri;
        Range range;
    };

    struct TextEdit
    {
        Range range;
        std::string text;
    };

    struct MarkupContent
    {
        std::string kind;
        std::string value;
    };

    struct Hover
    {
        MarkupContent contents;
        Range range;
    };

    struct CompletionItem
    {
        std::string label;
        int kind;
        std::string detail;
        std::string data;
        std::optional<MarkupContent> documentation;
        std::optional<TextEdit> textEdit;
        std::optional<std::vector<std::string>> commitCharacters;
    };

    struct CompletionContext
    {
        int triggerKind = 1;
        std::string triggerCharacter;
    };

    struct ParameterInformation
    {
        uint32_t label[2] = { 0, 0 };
        MarkupContent documentation;
    };

    struct SignatureInformation
    {
        std::string label;
        MarkupContent documentation;
        std::vector<ParameterInformation> parameters;
    };

    struct SignatureHelp
    {
        std::vector<SignatureInformation> signatures;
        uint32_t activeSignature = 0;
        uint32_t activeParameter = 0;
    };

    struct DocumentSymbol
    {
        std::string name;
        std::string detail;
        int kind = 0;
        Range range;
        Range selectionRange;
        std::vector<DocumentSymbol> children;
    };

    struct Diagnostics
    {
        std::string code;
        Range range;
        std::string message;
        int severity;
    };

    class LanguageServer
    {
    private:
        Slang::LanguageServerCore* m_core = nullptr;
        void init();
    public:
        LanguageServer();
        ~LanguageServer();
        void didOpenTextDocument(std::string uri, std::string text);
        void didCloseTextDocument(std::string uri);
        void didChangeTextDocument(std::string uri, const std::vector<lsp::TextEdit>& changes);
        std::optional<lsp::Hover> hover(std::string uri, lsp::Position position);
        std::optional<std::vector<lsp::Location>> gotoDefinition(std::string uri, lsp::Position position);
        std::optional<std::vector<lsp::CompletionItem>> completion(
            std::string uri, lsp::Position position, CompletionContext context);
        std::optional<lsp::CompletionItem> completionResolve(lsp::CompletionItem args);
        std::optional<std::vector<uint32_t>> semanticTokens(std::string uri);
        std::optional<lsp::SignatureHelp> signatureHelp(std::string uri, lsp::Position position);
        std::optional<std::vector<lsp::DocumentSymbol>> documentSymbol(std::string uri);
        std::optional<std::vector<lsp::Diagnostics>> getDiagnostics(std::string uri);
    };

    LanguageServer* createLanguageServer();
}

} // namespace wgsl
} // namespace slang
