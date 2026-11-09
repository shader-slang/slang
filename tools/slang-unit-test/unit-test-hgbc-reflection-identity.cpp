#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang-hgbc-container.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Validate the names actually emitted in HGBC, not a second handwritten type-name formatter.
// Source and reloaded modules must resolve each name to the exact same specialization as the
// expected expression. Merely checking for non-null would allow omitted arguments to select
// defaults (for example RWTexture2D instead of RWTexture2D<uint>).
SLANG_UNIT_TEST(hgbcReflectionIdentityRoundTrip)
{
    const char* source = R"(
        public struct Box<T> { public T value; }
        public extension<T : ITexelElement> RWTexture2D<T>
        {
            public struct Marker { public T value; }
        }
        [shader("compute")] [numthreads(1, 1, 1)] void probe<T>() {}
        void forward<T>() { dispatch(probe<T>, uint3(1, 1, 1))(); }
        [host] void main()
        {
            forward<StructuredBuffer<Box<float>>>();
            forward<StructuredBuffer<Box<float>, ScalarDataLayout>>();
            forward<RWTexture2D<uint>>();
            forward<RWTexture2D<float4>>();
            forward<Texture2D<float4, 0, 3>>();
            forward<matrix<float, 2, 3, MatrixLayoutMode(1)>>();
            forward<Array<Box<float>, 3>>();
            forward<DescriptorHandle<StructuredBuffer<Box<float>>>>();
            forward<RWTexture2D<uint>.Marker>();
        }
    )";
    const char* expectedNames[] = {
        "StructuredBuffer<Box<float>, DefaultDataLayout>",
        "StructuredBuffer<Box<float>, ScalarDataLayout>",
        "_Texture<uint, __Shape2D, 0, 0, 0, 1, 0, 0, 0>",
        "_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 1, 0, 0, 0>",
        "_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 0, 0, 0, 3>",
        "matrix<float, 2, 3, MatrixLayoutMode(1)>",
        "Array<Box<float>, 3>",
        "DescriptorHandle<StructuredBuffer<Box<float>, DefaultDataLayout>>",
        "_Texture<uint, __Shape2D, 0, 0, 0, 1, 0, 0, 0>.Marker",
    };
    const char* sourceExpressions[] = {
        "StructuredBuffer<Box<float>>",
        "StructuredBuffer<Box<float>, ScalarDataLayout>",
        "RWTexture2D<uint>",
        "RWTexture2D<float4>",
        "Texture2D<float4, 0, 3>",
        "matrix<float, 2, 3, MatrixLayoutMode(1)>",
        "Array<Box<float>, 3>",
        "DescriptorHandle<StructuredBuffer<Box<float>>>",
        "RWTexture2D<uint>.Marker",
    };

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(globalSession.writeRef())));
    for (int obfuscate = 0; obfuscate < 2; ++obfuscate)
    {
        slang::TargetDesc target = {};
        target.format = SLANG_HGBC;
        slang::CompilerOptionEntry options[2] = {};
        options[0].name = slang::CompilerOptionName::ExperimentalFeature;
        options[0].value.kind = slang::CompilerOptionValueKind::Int;
        options[0].value.intValue0 = 1;
        options[1].name = slang::CompilerOptionName::Obfuscate;
        options[1].value.kind = slang::CompilerOptionValueKind::Int;
        options[1].value.intValue0 = obfuscate;
        slang::SessionDesc desc = {};
        desc.targets = &target;
        desc.targetCount = 1;
        desc.compilerOptionEntries = options;
        desc.compilerOptionEntryCount = 2;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));
        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "reflection_identity",
            "reflection_identity.slang",
            source,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module);
        ComPtr<slang::IEntryPoint> entry;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entry.writeRef())));
        slang::IComponentType* components[] = {module, entry};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            composite->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
        HGBC::Program program;
        SLANG_CHECK_ABORT(
            HGBC::readProgram(code->getBufferPointer(), code->getBufferSize(), program));

        ComPtr<slang::IBlob> serialized;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(serialized.writeRef())));
        ComPtr<slang::ISession> reloadSession;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(globalSession->createSession(desc, reloadSession.writeRef())));
        auto reloaded = reloadSession->loadModuleFromIRBlob(
            "reflection_identity",
            "reflection_identity.slang",
            serialized,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(reloaded);

        bool seen[SLANG_COUNT_OF(expectedNames)] = {};
        for (const auto& record : program.records)
        {
            if (record.opcode != HGBCOpcode::SpecializeSlangEntryPoint)
                continue;
            SLANG_CHECK_ABORT(record.operands.size() == 4);
            SLANG_CHECK_ABORT(record.operands[2] == uint32_t(HGBCSpecializationArgKind::Type));
            const auto& emittedName = program.basicIndex.strings.at(record.operands[3]);
            Index expectedIndex = -1;
            for (Index i = 0; i < SLANG_COUNT_OF(expectedNames); ++i)
                if (emittedName == expectedNames[i])
                    expectedIndex = i;
            SLANG_CHECK_ABORT(expectedIndex >= 0);
            seen[expectedIndex] = true;
            String sourceFullName;
            for (auto lookupModule : {module, reloaded})
            {
                auto layout = lookupModule->getLayout();
                SLANG_CHECK_ABORT(layout);
                auto actualType = layout->findTypeByName(emittedName.c_str());
                auto expectedType = layout->findTypeByName(sourceExpressions[expectedIndex]);
                SLANG_CHECK_ABORT(actualType && expectedType);
                ComPtr<slang::IBlob> actualName, expectedName;
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(actualType->getFullName(actualName.writeRef())));
                SLANG_CHECK_ABORT(
                    SLANG_SUCCEEDED(expectedType->getFullName(expectedName.writeRef())));
                String fullName((const char*)actualName->getBufferPointer());
                SLANG_CHECK(fullName == String((const char*)expectedName->getBufferPointer()));
                if (lookupModule == module)
                    sourceFullName = fullName;
                else
                    SLANG_CHECK(fullName == sourceFullName);
            }
        }
        for (bool wasSeen : seen)
            SLANG_CHECK(wasSeen);
    }
}
