#include "slang-wasm.h"

#include "../core/slang-blob.h"
#include "../core/slang-exception.h"
#include "../slang/slang-language-server.h"

#include <slang.h>
#include <string>
#include <vector>

using namespace slang;

namespace slang
{
namespace wgsl
{

Error g_error;
CompileTargets g_compileTargets;

Error getLastError()
{
    Error currentError = g_error;
    g_error = {};
    return currentError;
}

CompileTargets* getCompileTargets()
{
    return &g_compileTargets;
}

GlobalSession* createGlobalSession()
{
    IGlobalSession* globalSession = nullptr;
    {
        SlangResult result = slang::createGlobalSession(&globalSession);
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new GlobalSession(globalSession);
}

CompileTargets::CompileTargets()
{
#define MAKE_PAIR(x) {#x, SLANG_##x}

    m_compileTargetMap = {
        MAKE_PAIR(GLSL),
        MAKE_PAIR(HLSL),
        MAKE_PAIR(WGSL),
        MAKE_PAIR(SPIRV),
        MAKE_PAIR(METAL),
    };
}

int CompileTargets::findCompileTarget(const std::string& name)
{
    auto res = m_compileTargetMap.find(name);
    if (res != m_compileTargetMap.end())
    {
        return res->second;
    }
    else
    {
        return SLANG_TARGET_UNKNOWN;
    }
}

Session* GlobalSession::createSession(int compileTarget)
{
    ISession* session = nullptr;
    {
        SessionDesc sessionDesc = {};
        sessionDesc.structureSize = sizeof(sessionDesc);
        constexpr SlangInt targetCount = 1;
        TargetDesc target = {};
        target.format = (SlangCompileTarget)compileTarget;
        sessionDesc.targets = &target;
        sessionDesc.targetCount = targetCount;
        SlangResult result = m_interface->createSession(sessionDesc, &session);
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new Session(session);
}

Module* Session::loadModuleFromSource(
    const std::string& slangCode,
    const std::string& name,
    const std::string& path)
{
    Slang::ComPtr<IModule> module;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::ComPtr<ISlangBlob> slangCodeBlob =
            Slang::RawBlob::create(slangCode.c_str(), slangCode.size());
        module = m_interface->loadModuleFromSource(
            name.c_str(),
            path.c_str(),
            slangCodeBlob,
            diagnosticsBlob.writeRef());
        if (!module)
        {
            g_error.type = std::string("USER");
            g_error.message = std::string(
                (char*)diagnosticsBlob->getBufferPointer(),
                (char*)diagnosticsBlob->getBufferPointer() + diagnosticsBlob->getBufferSize());
            return nullptr;
        }
    }

    return new Module(module);
}

EntryPoint* Module::findEntryPointByName(const std::string& name)
{
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        SlangResult result =
            moduleInterface()->findEntryPointByName(name.c_str(), entryPoint.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new EntryPoint(entryPoint);
}


EntryPoint* Module::findAndCheckEntryPoint(const std::string& name, int stage)
{
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = moduleInterface()->findAndCheckEntryPoint(
            name.c_str(),
            (SlangStage)stage,
            entryPoint.writeRef(),
            diagnosticsBlob.writeRef());
        if (!SLANG_SUCCEEDED(result))
        {
            g_error.type = std::string("USER");
            g_error.result = result;

            if (diagnosticsBlob->getBufferSize())
            {
                char* diagnostics = (char*)diagnosticsBlob->getBufferPointer();
                g_error.message = std::string(diagnostics);
            }
            return nullptr;
        }
    }

    return new EntryPoint(entryPoint);
}

int Module::getDefinedEntryPointCount()
{
    return moduleInterface()->getDefinedEntryPointCount();
}

EntryPoint* Module::getDefinedEntryPoint(int index)
{
    if (moduleInterface()->getDefinedEntryPointCount() <= index)
        return nullptr;

    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = moduleInterface()->getDefinedEntryPoint(index, entryPoint.writeRef());
        if (!SLANG_SUCCEEDED(result))
        {
            g_error.type = std::string("USER");
            g_error.result = result;

            if (diagnosticsBlob->getBufferSize())
            {
                char* diagnostics = (char*)diagnosticsBlob->getBufferPointer();
                g_error.message = std::string(diagnostics);
            }
            return nullptr;
        }
    }

    return new EntryPoint(entryPoint);
}


ComponentType* Session::createCompositeComponentType(const std::vector<ComponentType*>& components)
{
    Slang::ComPtr<IComponentType> composite;
    {
        std::vector<IComponentType*> nativeComponents(components.size());
        for (size_t i = 0U; i < components.size(); i++)
            nativeComponents[i] = components[i]->interface();
        SlangResult result = m_interface->createCompositeComponentType(
            nativeComponents.data(),
            (SlangInt)nativeComponents.size(),
            composite.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new ComponentType(composite);
}

ComponentType* ComponentType::link()
{
    Slang::ComPtr<IComponentType> linkedProgram;
    {
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return nullptr;
        }
    }

    return new ComponentType(linkedProgram);
}

std::string ComponentType::getEntryPointCode(int entryPointIndex, int targetIndex)
{
    {
        Slang::ComPtr<IBlob> kernelBlob;
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->getEntryPointCode(
            entryPointIndex,
            targetIndex,
            kernelBlob.writeRef(),
            diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return "";
        }
        std::string wgslCode = std::string(
            (char*)kernelBlob->getBufferPointer(),
            (char*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize());
        return wgslCode;
    }

    return {};
}

// Since result code is binary, we can't return it as a string, we will need to use emscripten::val
// to wrap it and return it to the javascript side.
emscripten::val ComponentType::getEntryPointCodeBlob(int entryPointIndex, int targetIndex)
{
    Slang::ComPtr<IBlob> kernelBlob;
    Slang::ComPtr<ISlangBlob> diagnosticBlob;
    SlangResult result = interface()->getEntryPointCode(
        entryPointIndex,
        targetIndex,
        kernelBlob.writeRef(),
        diagnosticBlob.writeRef());
    if (result != SLANG_OK)
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        g_error.message = std::string(
            (char*)diagnosticBlob->getBufferPointer(),
            (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
        return {};
    }

    const uint8_t* ptr = (uint8_t*)kernelBlob->getBufferPointer();
    return emscripten::val(emscripten::typed_memory_view(kernelBlob->getBufferSize(), ptr));
}

std::string ComponentType::getTargetCode(int targetIndex)
{
    {
        Slang::ComPtr<IBlob> kernelBlob;
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->getTargetCode(
            targetIndex,
            kernelBlob.writeRef(),
            diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
            return "";
        }
        std::string targetCode = std::string(
            (char*)kernelBlob->getBufferPointer(),
            (char*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize());
        return targetCode;
    }

    return {};
}

// Since result code is binary, we can't return it as a string, we will need to use emscripten::val
// to wrap it and return it to the javascript side.
emscripten::val ComponentType::getTargetCodeBlob(int targetIndex)
{
    Slang::ComPtr<IBlob> kernelBlob;
    Slang::ComPtr<ISlangBlob> diagnosticBlob;
    SlangResult result =
        interface()->getTargetCode(targetIndex, kernelBlob.writeRef(), diagnosticBlob.writeRef());
    if (result != SLANG_OK)
    {
        g_error.type = std::string("USER");
        g_error.result = result;
        g_error.message = std::string(
            (char*)diagnosticBlob->getBufferPointer(),
            (char*)diagnosticBlob->getBufferPointer() + diagnosticBlob->getBufferSize());
        return {};
    }

    const uint8_t* ptr = (uint8_t*)kernelBlob->getBufferPointer();
    return emscripten::val(emscripten::typed_memory_view(kernelBlob->getBufferSize(), ptr));
}

namespace lsp
{
Position translate(Slang::LanguageServerProtocol::Position p)
{
    Position result;
    result.line = p.line;
    result.character = p.character;
    return result;
}
Range translate(Slang::LanguageServerProtocol::Range r)
{
    Range result;
    result.start = translate(r.start);
    result.end = translate(r.end);
    return result;
}
Location translate(Slang::LanguageServerProtocol::Location l)
{
    Location result;
    result.uri = l.uri.getBuffer();
    result.range = translate(l.range);
    return result;
}
Slang::LanguageServerProtocol::Position translate(Position p)
{
    Slang::LanguageServerProtocol::Position result;
    result.line = p.line;
    result.character = p.character;
    return result;
}
Slang::LanguageServerProtocol::Range translate(Range r)
{
    Slang::LanguageServerProtocol::Range result;
    result.start = translate(r.start);
    result.end = translate(r.end);
    return result;
}
Slang::LanguageServerProtocol::Location translate(Location l)
{
    Slang::LanguageServerProtocol::Location result;
    result.uri = l.uri.c_str();
    result.range = translate(l.range);
    return result;
}

LanguageServer::LanguageServer()
{
    Slang::LanguageServerStartupOptions options = {};
    m_core = new Slang::LanguageServerCore(options);
    init();
}

LanguageServer::~LanguageServer()
{
    delete m_core;
}

void LanguageServer::init()
{
    Slang::LanguageServerProtocol::InitializeParams args = {};
    Slang::LanguageServerProtocol::WorkspaceFolder folder = {};
    folder.uri = "file:///";
    folder.name = "/";
    args.workspaceFolders.add(folder);
    m_core->init(args);
}

void LanguageServer::didOpenTextDocument(std::string uri, std::string text)
{
    Slang::LanguageServerProtocol::DidOpenTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    args.textDocument.languageId = "slang";
    args.textDocument.text = text.c_str();
    m_core->didOpenTextDocument(args);
}

void LanguageServer::didCloseTextDocument(std::string uri)
{
    Slang::LanguageServerProtocol::DidCloseTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    m_core->didCloseTextDocument(args);
}

void LanguageServer::didChangeTextDocument(
    std::string uri,
    const std::vector<lsp::TextEdit>& changes)
{
    Slang::LanguageServerProtocol::DidChangeTextDocumentParams args = {};
    args.textDocument.uri = uri.c_str();
    for (auto change : changes)
    {
        Slang::LanguageServerProtocol::TextDocumentContentChangeEvent lspChange;
        lspChange.text = change.text.c_str();
        lspChange.range = translate(change.range);
        args.contentChanges.add(lspChange);
    }
    m_core->didChangeTextDocument(args);
}

std::optional<lsp::Hover> LanguageServer::hover(std::string uri, lsp::Position position)
{
    Slang::LanguageServerProtocol::HoverParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->hover(args);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::Hover result;
    result.contents.kind = coreResult.result.contents.kind.getBuffer();
    result.contents.value = coreResult.result.contents.value.getBuffer();
    result.range = translate(coreResult.result.range);
    return result;
}

std::optional<std::vector<lsp::Location>> LanguageServer::gotoDefinition(
    std::string uri,
    lsp::Position position)
{
    Slang::LanguageServerProtocol::DefinitionParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->gotoDefinition(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::Location> result;
    for (auto location : coreResult.result)
        result.push_back(translate(location));
    return result;
}

std::optional<std::vector<lsp::CompletionItem>> LanguageServer::completion(
    std::string uri,
    lsp::Position position,
    CompletionContext context)
{
    Slang::LanguageServerProtocol::CompletionParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    args.context.triggerKind = context.triggerKind;
    args.context.triggerCharacter = context.triggerCharacter.c_str();
    auto coreResult = m_core->completion(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::CompletionItem> result;
    for (auto item : coreResult.result.items)
    {
        lsp::CompletionItem completionItem;
        completionItem.label = item.label.getBuffer();
        completionItem.kind = item.kind;
        completionItem.detail = item.detail.getBuffer();
        MarkupContent documentation;
        documentation.kind = item.documentation.kind.getBuffer();
        documentation.value = item.documentation.value.getBuffer();
        completionItem.documentation = documentation;
        completionItem.textEdit = std::nullopt;
        completionItem.data = item.data.getBuffer();
        std::vector<std::string> commitCharacters;
        for (auto character : item.commitCharacters)
            commitCharacters.push_back(character.getBuffer());
        completionItem.commitCharacters = commitCharacters;
        result.push_back(completionItem);
    }
    return result;
}

std::optional<lsp::CompletionItem> LanguageServer::completionResolve(lsp::CompletionItem args)
{
    Slang::LanguageServerProtocol::CompletionItem coreArgs = {};
    coreArgs.label = args.label.c_str();
    coreArgs.kind = args.kind;
    coreArgs.detail = args.detail.c_str();
    if (args.documentation.has_value())
    {
        coreArgs.documentation.kind = args.documentation.value().kind.c_str();
        coreArgs.documentation.value = args.documentation.value().value.c_str();
    }
    coreArgs.data = args.data.c_str();
    if (args.commitCharacters.has_value())
    {
        for (auto character : args.commitCharacters.value())
            coreArgs.commitCharacters.add(character.c_str());
    }
    Slang::LanguageServerProtocol::TextEditCompletionItem editItem;
    editItem.label = coreArgs.label;
    editItem.kind = coreArgs.kind;
    editItem.detail = coreArgs.detail;
    editItem.documentation.kind = coreArgs.documentation.kind;
    editItem.documentation.value = coreArgs.documentation.value;
    editItem.data = coreArgs.data;
    for (auto character : coreArgs.commitCharacters)
        editItem.commitCharacters.add(character);
    auto coreResult = m_core->completionResolve(coreArgs, editItem);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::CompletionItem result;
    result.label = coreResult.result.label.getBuffer();
    result.kind = coreResult.result.kind;
    result.detail = coreResult.result.detail.getBuffer();
    MarkupContent documentation;
    documentation.kind = coreResult.result.documentation.kind.getBuffer();
    documentation.value = coreResult.result.documentation.value.getBuffer();
    result.documentation = documentation;
    result.textEdit = std::nullopt;
    result.data = coreResult.result.data.getBuffer();
    std::vector<std::string> commitCharacters;
    for (auto character : coreResult.result.commitCharacters)
        commitCharacters.push_back(character.getBuffer());
    result.commitCharacters = commitCharacters;
    return result;
}

std::optional<std::vector<uint32_t>> LanguageServer::semanticTokens(std::string uri)
{
    Slang::LanguageServerProtocol::SemanticTokensParams args = {};
    args.textDocument.uri = uri.c_str();
    auto coreResult = m_core->semanticTokens(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<uint32_t> result;
    result.reserve((size_t)coreResult.result.data.getCount());
    for (auto token : coreResult.result.data)
        result.push_back(token);
    return result;
}

std::optional<lsp::SignatureHelp> LanguageServer::signatureHelp(
    std::string uri,
    lsp::Position position)
{
    Slang::LanguageServerProtocol::SignatureHelpParams args = {};
    args.textDocument.uri = uri.c_str();
    args.position = translate(position);
    auto coreResult = m_core->signatureHelp(args);
    if (coreResult.isNull)
        return std::nullopt;
    lsp::SignatureHelp result;
    for (auto signature : coreResult.result.signatures)
    {
        lsp::SignatureInformation signatureInfo;
        signatureInfo.label = signature.label.getBuffer();
        signatureInfo.documentation.kind = signature.documentation.kind.getBuffer();
        signatureInfo.documentation.value = signature.documentation.value.getBuffer();
        for (auto parameter : signature.parameters)
        {
            lsp::ParameterInformation parameterInfo;
            parameterInfo.label[0] = parameter.label[0];
            parameterInfo.label[1] = parameter.label[1];
            parameterInfo.documentation.kind = parameter.documentation.kind.getBuffer();
            parameterInfo.documentation.value = parameter.documentation.value.getBuffer();
            signatureInfo.parameters.push_back(parameterInfo);
        }
        result.signatures.push_back(signatureInfo);
    }
    result.activeSignature = coreResult.result.activeSignature;
    result.activeParameter = coreResult.result.activeParameter;
    return result;
}

lsp::DocumentSymbol translate(Slang::LanguageServerProtocol::DocumentSymbol symbol)
{
    lsp::DocumentSymbol result;
    result.name = symbol.name.getBuffer();
    result.detail = symbol.detail.getBuffer();
    result.kind = symbol.kind;
    result.range = translate(symbol.range);
    result.selectionRange = translate(symbol.selectionRange);
    for (auto child : symbol.children)
        result.children.push_back(translate(child));
    return result;
}

std::optional<std::vector<lsp::DocumentSymbol>> LanguageServer::documentSymbol(std::string uri)
{
    Slang::LanguageServerProtocol::DocumentSymbolParams args = {};
    args.textDocument.uri = uri.c_str();
    auto coreResult = m_core->documentSymbol(args);
    if (coreResult.isNull)
        return std::nullopt;
    std::vector<lsp::DocumentSymbol> result;
    for (auto symbol : coreResult.result)
    {
        auto documentSymbol = translate(symbol);
        result.push_back(documentSymbol);
    }
    return result;
}

std::optional<std::vector<lsp::Diagnostics>> LanguageServer::getDiagnostics(std::string uri)
{
    std::vector<lsp::Diagnostics> result;
    auto module = m_core->m_workspace->getCurrentVersion()->getOrLoadModule(
        Slang::URI::fromString(Slang::UnownedStringSlice(uri.c_str())).getPath());
    if (!module)
        return std::nullopt;
    for (auto& docDiag : m_core->m_workspace->getCurrentVersion()->diagnostics)
    {
        for (auto& message : docDiag.second.messages)
        {
            lsp::Diagnostics diag;
            diag.code = Slang::String(message.code).getBuffer();
            diag.range = translate(message.range);
            diag.severity = (int)message.severity;
            diag.message = message.message.getBuffer();
            result.push_back(diag);
        }
    }
    return result;
}


LanguageServer* createLanguageServer()
{
    return new LanguageServer();
}

} // namespace lsp

} // namespace wgsl
} // namespace slang
