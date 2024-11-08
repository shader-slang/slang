#include "slang-wasm.h"

#include <emscripten/bind.h>
#include <slang-com-ptr.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(slang)
{
    constant("SLANG_OK", SLANG_OK);

    function(
        "createGlobalSession",
        &slang::wgsl::createGlobalSession,
        return_value_policy::take_ownership());

    function("getLastError", &slang::wgsl::getLastError);

    function(
        "getCompileTargets",
        &slang::wgsl::getCompileTargets,
        return_value_policy::take_ownership());

    class_<slang::wgsl::GlobalSession>("GlobalSession")
        .function(
            "createSession",
            &slang::wgsl::GlobalSession::createSession,
            return_value_policy::take_ownership());

    class_<slang::wgsl::Session>("Session")
        .function(
            "loadModuleFromSource",
            &slang::wgsl::Session::loadModuleFromSource,
            return_value_policy::take_ownership())
        .function(
            "createCompositeComponentType",
            &slang::wgsl::Session::createCompositeComponentType,
            return_value_policy::take_ownership());

    class_<slang::wgsl::ComponentType>("ComponentType")
        .function("link", &slang::wgsl::ComponentType::link, return_value_policy::take_ownership())
        .function("getEntryPointCode", &slang::wgsl::ComponentType::getEntryPointCode)
        .function("getEntryPointCodeBlob", &slang::wgsl::ComponentType::getEntryPointCodeBlob)
        .function("getTargetCodeBlob", &slang::wgsl::ComponentType::getTargetCodeBlob)
        .function("getTargetCode", &slang::wgsl::ComponentType::getTargetCode)
        .function(
            "getLayout",
            &slang::wgsl::ComponentType::getLayout,
            allow_raw_pointers())
        .function(
            "loadStrings",
            &slang::wgsl::ComponentType::loadStrings,
            return_value_policy::take_ownership());

    class_<slang::wgsl::TypeLayoutReflection>("TypeLayoutReflection")
        .function(
            "getDescriptorSetDescriptorRangeType",
            &slang::wgsl::TypeLayoutReflection::getDescriptorSetDescriptorRangeType);

    class_<slang::wgsl::VariableLayoutReflection>("VariableLayoutReflection")
        .function("getName", &slang::wgsl::VariableLayoutReflection::getName)
        .function(
            "getTypeLayout",
            &slang::wgsl::VariableLayoutReflection::getTypeLayout,
            allow_raw_pointers())
        .function("getBindingIndex", &slang::wgsl::VariableLayoutReflection::getBindingIndex);

    class_<slang::wgsl::ProgramLayout>("ProgramLayout")
        .function("getParameterCount", &slang::wgsl::ProgramLayout::getParameterCount)
        .function(
            "getParameterByIndex",
            &slang::wgsl::ProgramLayout::getParameterByIndex,
            allow_raw_pointers())
        .function(
            "getGlobalParamsTypeLayout",
            &slang::wgsl::ProgramLayout::getGlobalParamsTypeLayout,
            allow_raw_pointers());

    enum_<slang::BindingType>("BindingType")
        .value("Unknown", slang::BindingType::Unknown)
        .value("Texture", slang::BindingType::Texture)
        .value("ConstantBuffer", slang::BindingType::ConstantBuffer)
        .value("MutableRawBuffer", slang::BindingType::MutableRawBuffer)
        .value("MutableTypedBuffer", slang::BindingType::MutableTypedBuffer)
        .value("MutableTexture", slang::BindingType::MutableTexture);

    class_<slang::wgsl::Module, base<slang::wgsl::ComponentType>>("Module")
        .function(
            "findEntryPointByName",
            &slang::wgsl::Module::findEntryPointByName,
            return_value_policy::take_ownership())
        .function(
            "findAndCheckEntryPoint",
            &slang::wgsl::Module::findAndCheckEntryPoint,
            return_value_policy::take_ownership())
        .function(
            "getDefinedEntryPoint",
            &slang::wgsl::Module::getDefinedEntryPoint,
            return_value_policy::take_ownership())
        .function("getDefinedEntryPointCount", &slang::wgsl::Module::getDefinedEntryPointCount);

    value_object<slang::wgsl::Error>("Error")
        .field("type", &slang::wgsl::Error::type)
        .field("result", &slang::wgsl::Error::result)
        .field("message", &slang::wgsl::Error::message);

    class_<slang::wgsl::EntryPoint, base<slang::wgsl::ComponentType>>("EntryPoint")
        .function("getName", &slang::wgsl::EntryPoint::getName, allow_raw_pointers());

    class_<slang::wgsl::CompileTargets>("CompileTargets")
        .function(
            "findCompileTarget",
            &slang::wgsl::CompileTargets::findCompileTarget,
            return_value_policy::take_ownership());

    register_vector<slang::wgsl::ComponentType*>("ComponentTypeList");

    register_vector<std::string>("StringList");
    register_optional<std::vector<std::string>>();

    value_object<slang::wgsl::lsp::Position>("Position")
        .field("line", &slang::wgsl::lsp::Position::line)
        .field("character", &slang::wgsl::lsp::Position::character);

    value_object<slang::wgsl::lsp::Range>("Range")
        .field("start", &slang::wgsl::lsp::Range::start)
        .field("end", &slang::wgsl::lsp::Range::end);

    value_object<slang::wgsl::lsp::Location>("Location")
        .field("uri", &slang::wgsl::lsp::Location::uri)
        .field("range", &slang::wgsl::lsp::Location::range);
    register_vector<slang::wgsl::lsp::Location>("LocationList");
    register_optional<std::vector<slang::wgsl::lsp::Location>>();

    value_object<slang::wgsl::lsp::TextEdit>("TextEdit")
        .field("range", &slang::wgsl::lsp::TextEdit::range)
        .field("text", &slang::wgsl::lsp::TextEdit::text);
    register_optional<slang::wgsl::lsp::TextEdit>();
    register_vector<slang::wgsl::lsp::TextEdit>("TextEditList");
    register_optional<std::vector<slang::wgsl::lsp::TextEdit>>();

    value_object<slang::wgsl::lsp::MarkupContent>("MarkupContent")
        .field("kind", &slang::wgsl::lsp::MarkupContent::kind)
        .field("value", &slang::wgsl::lsp::MarkupContent::value);
    register_optional<slang::wgsl::lsp::MarkupContent>();

    value_object<slang::wgsl::lsp::Hover>("Hover")
        .field("contents", &slang::wgsl::lsp::Hover::contents)
        .field("range", &slang::wgsl::lsp::Hover::range);
    register_optional<slang::wgsl::lsp::Hover>();

    value_object<slang::wgsl::lsp::CompletionItem>("CompletionItem")
        .field("label", &slang::wgsl::lsp::CompletionItem::label)
        .field("kind", &slang::wgsl::lsp::CompletionItem::kind)
        .field("detail", &slang::wgsl::lsp::CompletionItem::detail)
        .field("documentation", &slang::wgsl::lsp::CompletionItem::documentation)
        .field("textEdit", &slang::wgsl::lsp::CompletionItem::textEdit)
        .field("data", &slang::wgsl::lsp::CompletionItem::data)
        .field("commitCharacters", &slang::wgsl::lsp::CompletionItem::commitCharacters);
    register_optional<slang::wgsl::lsp::CompletionItem>();
    register_vector<slang::wgsl::lsp::CompletionItem>("CompletionItemList");
    register_optional<std::vector<slang::wgsl::lsp::CompletionItem>>();

    value_object<slang::wgsl::lsp::CompletionContext>("CompletionContext")
        .field("triggerKind", &slang::wgsl::lsp::CompletionContext::triggerKind)
        .field("triggerCharacter", &slang::wgsl::lsp::CompletionContext::triggerCharacter);

    value_array<std::array<uint32_t, 2>>("array_uint_2")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>());

    value_object<slang::wgsl::lsp::ParameterInformation>("ParameterInformation")
        .field("label", &slang::wgsl::lsp::ParameterInformation::label)
        .field("documentation", &slang::wgsl::lsp::ParameterInformation::documentation);

    register_vector<slang::wgsl::lsp::ParameterInformation>("ParameterInformationList");

    value_object<slang::wgsl::lsp::SignatureInformation>("SignatureInformation")
        .field("label", &slang::wgsl::lsp::SignatureInformation::label)
        .field("documentation", &slang::wgsl::lsp::SignatureInformation::documentation)
        .field("parameters", &slang::wgsl::lsp::SignatureInformation::parameters);

    register_vector<slang::wgsl::lsp::SignatureInformation>("SignatureInformationList");

    value_object<slang::wgsl::lsp::SignatureHelp>("SignatureHelp")
        .field("signatures", &slang::wgsl::lsp::SignatureHelp::signatures)
        .field("activeSignature", &slang::wgsl::lsp::SignatureHelp::activeSignature)
        .field("activeParameter", &slang::wgsl::lsp::SignatureHelp::activeParameter);
    register_optional<slang::wgsl::lsp::SignatureHelp>();

    value_object<slang::wgsl::lsp::DocumentSymbol>("DocumentSymbol")
        .field("name", &slang::wgsl::lsp::DocumentSymbol::name)
        .field("detail", &slang::wgsl::lsp::DocumentSymbol::detail)
        .field("kind", &slang::wgsl::lsp::DocumentSymbol::kind)
        .field("range", &slang::wgsl::lsp::DocumentSymbol::range)
        .field("selectionRange", &slang::wgsl::lsp::DocumentSymbol::selectionRange)
        .field("children", &slang::wgsl::lsp::DocumentSymbol::children);

    register_vector<slang::wgsl::lsp::DocumentSymbol>("DocumentSymbolList");
    register_optional<std::vector<slang::wgsl::lsp::DocumentSymbol>>();

    value_object<slang::wgsl::lsp::Diagnostics>("Diagnostics")
        .field("code", &slang::wgsl::lsp::Diagnostics::code)
        .field("range", &slang::wgsl::lsp::Diagnostics::range)
        .field("severity", &slang::wgsl::lsp::Diagnostics::severity)
        .field("message", &slang::wgsl::lsp::Diagnostics::message);
    register_vector<slang::wgsl::lsp::Diagnostics>("DiagnosticsList");
    register_optional<std::vector<slang::wgsl::lsp::Diagnostics>>();

    register_vector<uint32_t>("Uint32List");
    register_optional<std::vector<uint32_t>>();

    class_<slang::wgsl::lsp::LanguageServer>("LanguageServer")
        .function(
            "didOpenTextDocument",
            &slang::wgsl::lsp::LanguageServer::didOpenTextDocument,
            allow_raw_pointers())
        .function(
            "didCloseTextDocument",
            &slang::wgsl::lsp::LanguageServer::didCloseTextDocument,
            allow_raw_pointers())
        .function(
            "didChangeTextDocument",
            &slang::wgsl::lsp::LanguageServer::didChangeTextDocument,
            allow_raw_pointers())
        .function("hover", &slang::wgsl::lsp::LanguageServer::hover, allow_raw_pointers())
        .function(
            "gotoDefinition",
            &slang::wgsl::lsp::LanguageServer::gotoDefinition,
            allow_raw_pointers())
        .function("completion", &slang::wgsl::lsp::LanguageServer::completion, allow_raw_pointers())
        .function(
            "completionResolve",
            &slang::wgsl::lsp::LanguageServer::completionResolve,
            allow_raw_pointers())
        .function(
            "semanticTokens",
            &slang::wgsl::lsp::LanguageServer::semanticTokens,
            allow_raw_pointers())
        .function(
            "signatureHelp",
            &slang::wgsl::lsp::LanguageServer::signatureHelp,
            allow_raw_pointers())
        .function(
            "documentSymbol",
            &slang::wgsl::lsp::LanguageServer::documentSymbol,
            allow_raw_pointers())
        .function(
            "getDiagnostics",
            &slang::wgsl::lsp::LanguageServer::getDiagnostics,
            allow_raw_pointers());

    function(
        "createLanguageServer",
        &slang::wgsl::lsp::createLanguageServer,
        return_value_policy::take_ownership());

    class_<slang::wgsl::HashedString>("HashedString")
        .function("getString", &slang::wgsl::HashedString::getString);
};
