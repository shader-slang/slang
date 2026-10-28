// unit-test-image-format-reflection.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Clients can enumerate the public formats with the original two-argument macro.
static constexpr SlangImageFormat kLegacyImageFormats[] = {
#define SLANG_FORMAT(NAME, DESC) SLANG_IMAGE_FORMAT_##NAME,
#include "slang-image-format-defs.h"
};
static_assert(SLANG_COUNT_OF(kLegacyImageFormats) == SLANG_IMAGE_FORMAT_bgra8 + 1);

// Test that the getBindingRangeImageFormat API works.

SLANG_UNIT_TEST(imageFormatReflection)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        Texture2D<uint4> g_tex : register(t0);
        RWTexture2D<float4> defaultFloat;
        [format("rgba16f")] RWTexture2D<float4, 0, 1> attributedLegacy;
        RWTexture2D<RGBA8Unorm> described;
        float4 fragMain(float4 pos:SV_Position) : SV_Target
        {
            return pos;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    auto layout = compositeProgram->getLayout(0);

    // Descriptor formats resolve a dependent constant through this session's
    // standard library. Creating another session must not redirect those lookups
    // to declarations owned by a different AST builder.
    ComPtr<slang::IGlobalSession> otherGlobalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        slang_createGlobalSession(SLANG_API_VERSION, otherGlobalSession.writeRef())));
    ComPtr<slang::ISession> otherSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(otherGlobalSession->createSession(sessionDesc, otherSession.writeRef())));
    auto otherModule = otherSession->loadModuleFromSourceString(
        "otherImageFormats",
        "otherImageFormats.slang",
        "RWTexture2D<RGBA16F> otherTexture;",
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(otherModule != nullptr);
    SLANG_CHECK_ABORT(otherModule->getLayout() != nullptr);

    const SlangImageFormat expectedFormats[] = {
        SLANG_IMAGE_FORMAT_rgba32ui,
        SLANG_IMAGE_FORMAT_unknown,
        SLANG_IMAGE_FORMAT_rgba16f,
        SLANG_IMAGE_FORMAT_rgba8,
    };
    auto globalLayout = layout->getGlobalParamsTypeLayout();
    SLANG_CHECK_ABORT(globalLayout->getBindingRangeCount() == SLANG_COUNT_OF(expectedFormats));
    SLANG_CHECK_ABORT(layout->getParameterCount() == SLANG_COUNT_OF(expectedFormats));
    for (SlangInt i = 0; i < SLANG_COUNT_OF(expectedFormats); ++i)
    {
        SLANG_CHECK(globalLayout->getBindingRangeImageFormat(i) == expectedFormats[i]);
        SLANG_CHECK(layout->getParameterByIndex(i)->getImageFormat() == expectedFormats[i]);
    }
}

// Texture names retain the public texel argument, while resource results expose
// the loaded payload. SlangPy also uses this API to infer generic texture arguments.
SLANG_UNIT_TEST(textureFormatTypeReflection)
{
    const char* source = R"(
        Texture2D<T> textureIdentity<T : ITexelFormat>(Texture2D<T> texture)
        {
            return texture;
        }
        RWTexture2D<T> rwTextureIdentity<T : ITexelFormat>(RWTexture2D<T> texture)
        {
            return texture;
        }
        Buffer<T> bufferIdentity<T : ITexelFormat>(Buffer<T> buffer)
        {
            return buffer;
        }
        RWBuffer<T> rwBufferIdentity<T : ITexelFormat>(RWBuffer<T> buffer)
        {
            return buffer;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "textureFormats",
        "textureFormats.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    auto layout = module->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);

    struct TestCase
    {
        const char* typeName;
        const char* fullName;
        const char* payloadName;
        const char* functionName;
    };
    const TestCase cases[] = {
        {"Texture2D<float1>", "Texture2D<vector<float,1>>", "vector<float,1>", "textureIdentity"},
        {"Texture2D<float4>", "Texture2D<vector<float,4>>", "vector<float,4>", "textureIdentity"},
        {"RWTexture2D<float4>",
         "RWTexture2D<vector<float,4>>",
         "vector<float,4>",
         "rwTextureIdentity"},
        {"Texture2D<RGBA16F>", "Texture2D<RGBA16F>", "vector<float,4>", "textureIdentity"},
        {"Texture2D<RGBA32F>", "Texture2D<RGBA32F>", "vector<float,4>", "textureIdentity"},
        {"RWTexture2D<RGBA16F>", "RWTexture2D<RGBA16F>", "vector<float,4>", "rwTextureIdentity"},
        {"Buffer<float4>", "Buffer<vector<float,4>>", "vector<float,4>", "bufferIdentity"},
        {"Buffer<RGBA32F>", "Buffer<RGBA32F>", "vector<float,4>", "bufferIdentity"},
        {"RWBuffer<RGBA32F>", "RWBuffer<RGBA32F>", "vector<float,4>", "rwBufferIdentity"},
    };

    for (const auto& testCase : cases)
    {
        auto type = layout->findTypeByName(testCase.typeName);
        SLANG_CHECK_ABORT(type != nullptr);
        ComPtr<slang::IBlob> name;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(type->getFullName(name.writeRef())));
        SLANG_CHECK(String((const char*)name->getBufferPointer()) == testCase.fullName);
        auto roundTrippedType = layout->findTypeByName((const char*)name->getBufferPointer());
        SLANG_CHECK_ABORT(roundTrippedType != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(roundTrippedType->getFullName(name.writeRef())));
        SLANG_CHECK(String((const char*)name->getBufferPointer()) == testCase.fullName);

        auto payload = type->getResourceResultType();
        SLANG_CHECK_ABORT(payload != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(payload->getFullName(name.writeRef())));
        SLANG_CHECK(String((const char*)name->getBufferPointer()) == testCase.payloadName);

        auto function = layout->findFunctionByName(testCase.functionName);
        SLANG_CHECK_ABORT(function != nullptr);
        slang::TypeReflection* arguments[] = {type};
        auto specialized = function->specializeWithArgTypes(1, arguments);
        SLANG_CHECK_ABORT(specialized != nullptr);
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(specialized->getReturnType()->getFullName(name.writeRef())));
        SLANG_CHECK(String((const char*)name->getBufferPointer()) == testCase.fullName);
    }

    // Resolving the payload creates dependent member references. A different
    // global session must not supply its own standard-library declarations.
    auto originalTexture = layout->findTypeByName("RWTexture2D<RGBA16F>");
    SLANG_CHECK_ABORT(originalTexture != nullptr);
    ComPtr<slang::IGlobalSession> otherGlobalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        slang_createGlobalSession(SLANG_API_VERSION, otherGlobalSession.writeRef())));
    ComPtr<slang::ISession> otherSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(otherGlobalSession->createSession(sessionDesc, otherSession.writeRef())));
    auto otherModule = otherSession->loadModuleFromSourceString(
        "otherTextureFormats",
        "otherTextureFormats.slang",
        "RWTexture2D<RGBA16F> texture;",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(otherModule != nullptr);
    auto originalPayload = originalTexture->getResourceResultType();
    SLANG_CHECK_ABORT(originalPayload != nullptr);
    ComPtr<slang::IBlob> name;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(originalPayload->getFullName(name.writeRef())));
    SLANG_CHECK(String((const char*)name->getBufferPointer()) == "vector<float,4>");
}
