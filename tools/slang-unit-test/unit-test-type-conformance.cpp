// unit-test-type-conformance.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test the compilation API for adding type conformances.

SLANG_UNIT_TEST(typeConformance)
{
    const char* userSourceBody = R"(
        struct SurfaceInteraction {
        };

        __generic<T>
        struct InterfacePtr {
            T *dptr;
        };

        struct BsdfSample {
            float3 wo;
            float pdf;
            bool delta;
            float3 spectrum;
        };
        interface IBsdf {

            BsdfSample sample(SurfaceInteraction si, float2 uv);
        };
        struct Diffuse : IBsdf {
            float3 _reflectance;

            BsdfSample sample(SurfaceInteraction si, float2 uv) {
                BsdfSample sample;
                sample.wo = float3(uv, 1.0f);
                sample.pdf = uv.x;
                sample.delta = false;
                sample.spectrum = _reflectance;
                return sample;
            }
        };

        interface IShape {
            property InterfacePtr<IBsdf> bsdf;
        };
        struct Mesh : IShape {
            InterfacePtr<IBsdf> bsdf;
        };
        struct Sphere : IShape {
            InterfacePtr<IBsdf> bsdf;
        };

        [[vk::push_constant]] IShape *shapes;
        struct Path {
            float3 sample(IShape *shapes) {
                float3 spectrum = { 0.0f, 0.0f, 0.0f };
                float3 throughput = { 1.0f, 1.0f, 1.0f };

                while (true) {
                    SurfaceInteraction si = {};
           
                    if (true) {
                        const float p = min(max(throughput.r, max(throughput.g, throughput.b)), 0.95f);
                        if (1.0f >= p) return spectrum;
                    }

                    BsdfSample sample = shapes[0].bsdf.dptr.sample(si, float2(1.0f));
                    throughput *= sample.spectrum;
                }
                return spectrum;
            }
        };

        [[vk::binding(0, 0)]] RWTexture2D<float4> output;

        [shader("compute"), numthreads(1, 1, 1)]
        void computeMain() {
            Path path = Path();
            float3 spectrum = path.sample(nullptr);
            output[uint2(0,0)] += float4(spectrum, 1.0f);
        }
        )";
    ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.enableGLSL = true;
    SLANG_CHECK(slang_createGlobalSession2(&globalDesc, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    targetDesc.compilerOptionEntryCount = 1;
    slang::CompilerOptionEntry entry;
    entry.name = slang::CompilerOptionName::Optimization;
    entry.value.kind = slang::CompilerOptionValueKind::Int;
    entry.value.intValue0 = 0;
    targetDesc.compilerOptionEntries = &entry;

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
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());

    auto layout = module->getLayout();

    auto diffuse = layout->findTypeByName("Diffuse");
    auto ibsdf = layout->findTypeByName("IBsdf");
    auto ishape = layout->findTypeByName("IShape");
    auto mesh = layout->findTypeByName("Mesh");
    auto sphere = layout->findTypeByName("Sphere");

    ComPtr<slang::ITypeConformance> diffuseIBsdf;
    ComPtr<slang::ITypeConformance> meshIShape;
    ComPtr<slang::ITypeConformance> sphereIShape;
    session->createTypeConformanceComponentType(
        diffuse,
        ibsdf,
        diffuseIBsdf.writeRef(),
        0,
        diagnosticBlob.writeRef());
    session->createTypeConformanceComponentType(
        mesh,
        ishape,
        meshIShape.writeRef(),
        0,
        diagnosticBlob.writeRef());
    session->createTypeConformanceComponentType(
        sphere,
        ishape,
        sphereIShape.writeRef(),
        0,
        diagnosticBlob.writeRef());

    slang::IComponentType* componentTypes[5] =
        {module, entryPoint.get(), diffuseIBsdf, meshIShape, sphereIShape};
    ComPtr<slang::IComponentType> composedProgram;
    session->createCompositeComponentType(
        componentTypes,
        5,
        composedProgram.writeRef(),
        diagnosticBlob.writeRef());

    ComPtr<slang::IComponentType> linkedProgram;
    composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

    ComPtr<slang::IBlob> code;
    linkedProgram->getTargetCode(0, code.writeRef(), diagnosticBlob.writeRef());

    SLANG_CHECK(code != nullptr);
}

// `createTypeConformanceComponentType`'s interface argument may be a reflection
// obtained from a value/field position, where an interface reflects as its
// existential box `dyn IFoo` (still `Kind::Interface`). The conformance query
// must still find `S : IFoo`, so the API normalizes such a box to the interface.
// This guards that normalization: the interface here is taken from a struct field
// (a box), not from `findTypeByName` (which returns the interface directly).
SLANG_UNIT_TEST(typeConformanceFromValuePositionInterface)
{
    const char* userSourceBody = R"(
        interface IFoo { int f(); }
        struct S : IFoo { int f() { return 1; } }
        struct Holder { IFoo item; }

        RWStructuredBuffer<int> buf;
        [shader("compute"), numthreads(1, 1, 1)]
        void computeMain() {
            S s;
            Holder h;
            h.item = s;
            buf[0] = s.f();
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
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
    if (!module && diagnosticBlob)
        fprintf(stderr, "%s\n", (const char*)diagnosticBlob->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto layout = module->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);

    auto sType = layout->findTypeByName("S");
    SLANG_CHECK_ABORT(sType != nullptr);

    // The interface argument comes from a value position (a field), so it is the
    // existential box `dyn IFoo` rather than the bare interface.
    auto holderType = layout->findTypeByName("Holder");
    SLANG_CHECK_ABORT(holderType != nullptr && holderType->getFieldCount() == 1);
    auto fieldInterfaceType = holderType->getFieldByIndex(0)->getType();
    SLANG_CHECK_ABORT(fieldInterfaceType != nullptr);
    SLANG_CHECK(fieldInterfaceType->getKind() == slang::TypeReflection::Kind::Interface);

    ComPtr<slang::ITypeConformance> conformance;
    auto result = session->createTypeConformanceComponentType(
        sType,
        fieldInterfaceType,
        conformance.writeRef(),
        0,
        diagnosticBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(conformance != nullptr);
}
