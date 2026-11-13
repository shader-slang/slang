#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <string.h>

using namespace Slang;

namespace
{
const char* payloadContract = R"(
    public interface IPayload { [mutating] void add(float value); }
    public interface IRayTracer
    {
        associatedtype Payload;
        static void hit(inout Payload payload, float2 uv);
    }
    public interface IDerivedTracer : IRayTracer {}
    public interface IOuter { associatedtype Inner : IRayTracer; }
    public extern struct RayTracer : IRayTracer;
    public extern struct DerivedTracer : IDerivedTracer;
    public extern struct DefinedTracer : IRayTracer;
    public extern struct GenericTracer<T, int N> : IRayTracer;
    public extern struct Outer : IOuter;
    public extern static const int count;
    public struct Box<T> { T value; }

    [shader("closesthit")]
    void direct(inout RayTracer.Payload p, BuiltInTriangleIntersectionAttributes a)
    { RayTracer.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void explicitSemantics(inout RayTracer.Payload p : SV_RayPayload,
        BuiltInTriangleIntersectionAttributes a : SV_IntersectionAttributes)
    { RayTracer.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void defined(inout DefinedTracer.Payload p, BuiltInTriangleIntersectionAttributes a)
    { DefinedTracer.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void inherited(inout DerivedTracer.Payload p, BuiltInTriangleIntersectionAttributes a)
    { DerivedTracer.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void generic(inout GenericTracer<float, 2>.Payload p, BuiltInTriangleIntersectionAttributes a)
    { GenericTracer<float, 2>.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void chained(inout Outer.Inner.Payload p, BuiltInTriangleIntersectionAttributes a)
    { Outer.Inner.hit(p, a.barycentrics); }
    [shader("closesthit")]
    void nested(inout Box<RayTracer.Payload> p, BuiltInTriangleIntersectionAttributes a)
    { RayTracer.hit(p.value, a.barycentrics); }
    [shader("closesthit")]
    void valueArgument(inout GenericTracer<float, count>.Payload p,
        BuiltInTriangleIntersectionAttributes a)
    { GenericTracer<float, count>.hit(p, a.barycentrics); }
)";

// Both modules bind exactly the same extern declarations, but only the selected component
// belongs to a given composition. Their payloads have different sizes to expose cache leakage.
String getPayloadImplementation(int elementCount)
{
    StringBuilder source;
    source << "import payload_contract;\n";
    source << "public struct TestPayload : IPayload { public float values[" << elementCount << "];";
    source << R"(
        public [mutating] void add(float value) { values[0] += value; }
    }
    public struct Tracer : IDerivedTracer
    {
        public typealias Payload = TestPayload;
        public static void hit(inout Payload p, float2 uv) { p.add(uv.x); }
    }
    public struct GenericImpl<T, int N> : IRayTracer
    {
        public struct Payload : IPayload
        {
            public float values[N];
            public [mutating] void add(float value) { values[0] += value; }
        }
        public static void hit(inout Payload p, float2 uv) { p.add(uv.x); }
    }
    public struct OuterImpl : IOuter { public typealias Inner = Tracer; }
    export struct RayTracer : IRayTracer = Tracer;
    export struct DerivedTracer : IDerivedTracer = Tracer;
    export struct DefinedTracer : IRayTracer
    {
        public typealias Payload = TestPayload;
        public static void hit(inout TestPayload p, float2 uv) { p.add(uv.x); }
    }
    export struct GenericTracer<T, int N> : IRayTracer = GenericImpl<T, N>;
    export struct Outer : IOuter = OuterImpl;
    export static const int count = 3;
    [shader("closesthit")]
    void concrete(inout TestPayload p, BuiltInTriangleIntersectionAttributes a)
    { p.add(a.barycentrics.x); }
    export struct LocalTracer : IRayTracer = Tracer;
    [shader("closesthit")]
    void exportLookup(inout LocalTracer.Payload p, BuiltInTriangleIntersectionAttributes a)
    { LocalTracer.hit(p, a.barycentrics); }
    )";
    return source;
}

void checkPayloadCode(slang::IComponentType* program, SlangCompileTarget target)
{
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    auto result = program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result) && code);
    String text((const char*)code->getBufferPointer());
    if (target == SLANG_SPIRV_ASM)
    {
        SLANG_CHECK(text.contains("IncomingRayPayloadKHR"));
        SLANG_CHECK(text.contains("HitAttributeKHR"));
        SLANG_CHECK(text.contains("OpFAdd"));
        SLANG_CHECK(text.contains("OpStore"));
        SLANG_CHECK(!text.contains(" Private"));
    }
    else if (target == SLANG_CUDA_SOURCE)
    {
        SLANG_CHECK(text.contains("optixGetPayload_0"));
        SLANG_CHECK(text.contains("optixSetPayload_0"));
    }
    else
    {
        SLANG_CHECK(text.contains("fadd"));
        SLANG_CHECK(text.contains("store float"));
    }
}

void checkLinkTimePayload(SlangCompileTarget target)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    if (target == SLANG_DXIL_ASM &&
        SLANG_FAILED(globalSession->checkCompileTargetSupport(SLANG_DXIL)))
        SLANG_IGNORE_TEST;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = target;
    targetDesc.profile = globalSession->findProfile("sm_6_6");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    ComPtr<slang::ISession> sourceSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, sourceSession.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    auto sourceModule = sourceSession->loadModuleFromSourceString(
        "payload_contract",
        "payload_contract.slang",
        payloadContract,
        diagnostics.writeRef());
    if (!sourceModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(sourceModule);
    ComPtr<slang::IBlob> serialized;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(sourceModule->serialize(serialized.writeRef())));

    // Reload the contract in a fresh session so the test exercises the serialization boundary.
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
    auto contract = session->loadModuleFromIRBlob(
        "payload_contract",
        "payload_contract.slang-module",
        serialized,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(contract);
    // Compile implementations against the serialized contract too. Reusing the source contract
    // here would test a different checked witness graph from separate slangc invocations.
    ComPtr<slang::ISession> rendererSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, rendererSession.writeRef())));
    SLANG_CHECK_ABORT(rendererSession->loadModuleFromIRBlob(
        "payload_contract",
        "payload_contract.slang-module",
        serialized,
        diagnostics.writeRef()));
    slang::IModule* implementations[2] = {};
    for (int i = 0; i < 2; ++i)
    {
        auto source = getPayloadImplementation(i + 1);
        auto implementation = rendererSession->loadModuleFromSourceString(
            i ? "payload_b" : "payload_a",
            i ? "payload_b.slang" : "payload_a.slang",
            source.getBuffer(),
            diagnostics.writeRef());
        if (!implementation && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(implementation);
        ComPtr<slang::IBlob> implementationIR;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(implementation->serialize(implementationIR.writeRef())));
        implementations[i] = session->loadModuleFromIRBlob(
            i ? "payload_b" : "payload_a",
            i ? "payload_b.slang-module" : "payload_a.slang-module",
            implementationIR,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(implementations[i]);
    }

    const char* entryNames[] = {
        "direct",
        "explicitSemantics",
        "inherited",
        "defined",
        "generic",
        "chained",
        "nested",
        "valueArgument",
        "concrete",
        "exportLookup"};
    // A, B, A must each use its own binding, even after both implementations have been loaded.
    for (int selection : {0, 1, 0})
    {
        for (auto entryName : entryNames)
        {
            auto owner =
                strcmp(entryName, "concrete") == 0 || strcmp(entryName, "exportLookup") == 0
                    ? implementations[selection]
                    : contract;
            ComPtr<slang::IEntryPoint> entry;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(owner->findEntryPointByName(entryName, entry.writeRef())));
            slang::IComponentType* parts[] = {contract, implementations[selection], entry};
            ComPtr<slang::IComponentType> composite;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
                parts,
                3,
                composite.writeRef(),
                diagnostics.writeRef())));
            ComPtr<slang::IComponentType> linked;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));
            auto layout = linked->getLayout(0, diagnostics.writeRef());
            SLANG_CHECK_ABORT(layout);
            auto payload = layout->getEntryPointByIndex(0)->getParameterByIndex(0);
            SLANG_CHECK_ABORT(payload);
            auto payloadLayout = payload->getTypeLayout();
            SLANG_CHECK(payloadLayout->getKind() == slang::TypeReflection::Kind::Struct);
            SLANG_CHECK(payload->getCategory() == slang::ParameterCategory::RayPayload);
            SLANG_CHECK(payload->getOffset(slang::ParameterCategory::RayPayload) == 0);
            if (strcmp(entryName, "nested") == 0)
                payloadLayout = payloadLayout->getFieldByIndex(0)->getTypeLayout();
            SLANG_CHECK_ABORT(payloadLayout->getFieldCount() == 1);
            auto count = payloadLayout->getFieldByIndex(0)->getTypeLayout()->getElementCount();
            auto expectedCount = strcmp(entryName, "generic") == 0         ? 2
                                 : strcmp(entryName, "valueArgument") == 0 ? 3
                                                                           : selection + 1;
            SLANG_CHECK(count == expectedCount);
            checkPayloadCode(linked, target);
        }
    }
}
} // namespace

SLANG_UNIT_TEST(linkTimePayloadSPIRV)
{
    checkLinkTimePayload(SLANG_SPIRV_ASM);
}

SLANG_UNIT_TEST(linkTimePayloadCUDA)
{
    checkLinkTimePayload(SLANG_CUDA_SOURCE);
}
SLANG_UNIT_TEST(linkTimePayloadDXIL)
{
    checkLinkTimePayload(SLANG_DXIL_ASM);
}

SLANG_UNIT_TEST(linkTimeAssociatedVaryings)
{
    const char* contractSource = R"(
        public interface IMode
        {
            associatedtype VertexOut;
            associatedtype FragmentOut;
            static VertexOut vertex();
            static FragmentOut fragment();
        }
        public extern struct Mode : IMode;
        [shader("vertex")]
        Mode.VertexOut vertexMain() { return Mode.vertex(); }
        [shader("fragment")]
        Mode.FragmentOut fragmentMain() { return Mode.fragment(); }
    )";
    const char* implementationSource = R"(
        import varying_contract;
        public struct VertexOutput { public float4 position : SV_Position; }
        public struct FragmentOutput { public float4 color : SV_Target0; }
        public struct Implementation : IMode
        {
            public typealias VertexOut = VertexOutput;
            public typealias FragmentOut = FragmentOutput;
            public static VertexOut vertex()
            { VertexOut result; result.position = float4(1, 2, 3, 4); return result; }
            public static FragmentOut fragment()
            { FragmentOut result; result.color = float4(1, 2, 3, 4); return result; }
        }
        export struct Mode : IMode = Implementation;
    )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    for (auto target : {SLANG_SPIRV_ASM, SLANG_GLSL, SLANG_HLSL})
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = target;
        targetDesc.profile = globalSession->findProfile("sm_6_6");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
        ComPtr<slang::IBlob> diagnostics;
        auto contract = session->loadModuleFromSourceString(
            "varying_contract",
            "varying_contract.slang",
            contractSource,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(contract);
        auto implementation = session->loadModuleFromSourceString(
            "varying_implementation",
            "varying_implementation.slang",
            implementationSource,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(implementation);
        for (auto entryName : {"vertexMain", "fragmentMain"})
        {
            ComPtr<slang::IEntryPoint> entry;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(contract->findEntryPointByName(entryName, entry.writeRef())));
            slang::IComponentType* parts[] = {contract, implementation, entry};
            ComPtr<slang::IComponentType> composite;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
                parts,
                3,
                composite.writeRef(),
                diagnostics.writeRef())));
            ComPtr<slang::IComponentType> linked;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));
            auto layout = linked->getLayout(0, diagnostics.writeRef());
            SLANG_CHECK_ABORT(layout);
            auto result = layout->getEntryPointByIndex(0)->getResultVarLayout()->getTypeLayout();
            SLANG_CHECK_ABORT(result->getKind() == slang::TypeReflection::Kind::Struct);
            SLANG_CHECK(result->getFieldCount() == 1);
            ComPtr<slang::IBlob> code;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
            SLANG_CHECK_ABORT(code);
            if (target == SLANG_SPIRV_ASM)
                SLANG_CHECK(String((const char*)code->getBufferPointer()).contains("OpStore"));
        }
    }
}
