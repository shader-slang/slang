#include <emscripten/bind.h>
#include <slang-com-ptr.h>
#include "slang-wasm.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(slang)
{
    constant("SLANG_OK", SLANG_OK);

    function(
        "createGlobalSession",
        &slang::wgsl::createGlobalSession,
        return_value_policy::take_ownership());

    function(
        "getLastError",
        &slang::wgsl::getLastError);

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
        .function(
            "link",
            &slang::wgsl::ComponentType::link,
            return_value_policy::take_ownership())
        .function(
            "getEntryPointCode",
            &slang::wgsl::ComponentType::getEntryPointCode);

    class_<slang::wgsl::Module, base<slang::wgsl::ComponentType>>("Module")
        .function(
            "findEntryPointByName",
            &slang::wgsl::Module::findEntryPointByName,
            return_value_policy::take_ownership());

    value_object<slang::wgsl::Error>("Error")
        .field("type", &slang::wgsl::Error::type)
        .field("result", &slang::wgsl::Error::result)
        .field("message", &slang::wgsl::Error::message);

    class_<slang::wgsl::EntryPoint, base<slang::wgsl::ComponentType>>("EntryPoint");

    register_vector<slang::wgsl::ComponentType*>("ComponentTypeList");
}
