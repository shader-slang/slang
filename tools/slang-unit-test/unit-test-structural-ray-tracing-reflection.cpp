#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <cstring>
#include <stdio.h>
#include <thread>

using namespace Slang;

SLANG_UNIT_TEST(structuralRayTracingReflection)
{
    const char* source = R"(
        import slang.raytracing;

        struct PayloadA { float value; }
        struct PayloadB { uint value; }
        struct HitRecordA { float value; }
        struct HitRecordB { uint4 value; uint tail; }
        struct MissRecordA { float value; }
        struct MissRecordB { uint value; }
        struct CallableRecord { float4 value; float tail; }
        struct CallableData { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        typealias CommonTraceContext = TraceContext;
        typealias CommonCallableData = CallableData;

        struct HitContextA : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadA;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordA;
        }

        struct HitContextB : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadB;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordB;
        }

        struct MissContextA : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadA;
            typealias Record = MissRecordA;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = PayloadB;
            typealias Record = MissRecordB;
        }

        struct CallableContext : rt::ICallableContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias CallableData = CommonCallableData;
            typealias Record = CallableRecord;
        }

        struct ClosestHitA0 : rt::IClosestHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::ClosestHitInput<HitContextA> input) {}
        }

        struct ClosestHitB : rt::IClosestHitShader
        {
            typealias Context = HitContextB;
            void invoke(rt::ClosestHitInput<HitContextB> input) {}
        }

        struct ClosestHitA1 : rt::IClosestHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::ClosestHitInput<HitContextA> input) {}
        }

        struct AnyHitA0 : rt::IAnyHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::AnyHitInput<HitContextA> input) {}
        }

        struct HitGroupA0 : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = ClosestHitA0;
            typealias AnyHit = AnyHitA0;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct HitGroupB : rt::IHitGroup
        {
            typealias Context = HitContextB;
            typealias ClosestHit = ClosestHitB;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct HitGroupA1 : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = ClosestHitA1;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MissA0 : rt::IMissShader
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<MissContextA> input) {}
        }

        struct MissB : rt::IMissShader
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<MissContextB> input) {}
        }

        struct MissA1 : rt::IMissShader
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<MissContextA> input) {}
        }

        struct Callable0 : rt::ICallableShader
        {
            typealias Context = CallableContext;
            void invoke(rt::CallableInput<CallableContext> input) {}
        }

        struct Callable1 : rt::ICallableShader
        {
            typealias Context = CallableContext;
            void invoke(rt::CallableInput<CallableContext> input) {}
        }

        struct ReflectedSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = CommonTraceContext;
            typealias HitGroups = rt::HitGroupList<HitGroupA0, HitGroupB, HitGroupA1>;
            typealias MissShaders = rt::MissShaderList<MissA0, MissB, MissA1>;
            typealias CallableShaders = rt::CallableShaderList<Callable0, Callable1>;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "structuralReflection",
        "structural-reflection.slang",
        source,
        diagnostics.writeRef());
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK(module != nullptr);

    auto program = module->getLayout();
    auto schema = program->findTraceProgramSchema("ReflectedSchema");
    SLANG_CHECK(schema != nullptr);
    SLANG_CHECK(UnownedStringSlice(schema->getName()) == "ReflectedSchema");
    SLANG_CHECK(UnownedStringSlice(schema->getType()->getName()) == "ReflectedSchema");
    SLANG_CHECK(UnownedStringSlice(schema->getTraceContextType()->getName()) == "TraceContext");
    SLANG_CHECK(!schema->isHitGroupSectionOpen());
    SLANG_CHECK(!schema->isMissShaderSectionOpen());
    SLANG_CHECK(!schema->isCallableShaderSectionOpen());

    SLANG_CHECK(schema->getPayloadCount() == 2);
    auto payloadA = schema->getPayload(0);
    SLANG_CHECK(payloadA != nullptr);
    SLANG_CHECK(UnownedStringSlice(payloadA->getType()->getName()) == "PayloadA");
    SLANG_CHECK(payloadA->getTypeLayout() != nullptr);
    SLANG_CHECK(payloadA->getNativePayloadSize() == 0);
    SLANG_CHECK(payloadA->getHitGroupCount() == 2);
    auto hitGroupA0 = payloadA->getHitGroup(0);
    auto hitGroupA1 = payloadA->getHitGroup(1);
    SLANG_CHECK(hitGroupA0 != nullptr);
    SLANG_CHECK(hitGroupA1 != nullptr);
    SLANG_CHECK(hitGroupA0->getFunctionIndex() == 0);
    SLANG_CHECK(hitGroupA1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(hitGroupA0->getType()->getName()) == "HitGroupA0");
    SLANG_CHECK(UnownedStringSlice(hitGroupA0->getRecordType()->getName()) == "HitRecordA");
    SLANG_CHECK(hitGroupA0->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(!hitGroupA0->isLinked());
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getPrimitiveType()->getName()) == "TrianglePrimitive");
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getIntersectionAttributesType()->getName()) ==
        "TriangleData");
    SLANG_CHECK(hitGroupA0->getClosestHit()->getStage() == SLANG_STAGE_CLOSEST_HIT);
    SLANG_CHECK(hitGroupA0->getClosestHit()->getEntryPointName() != nullptr);
    SLANG_CHECK(
        UnownedStringSlice(hitGroupA0->getClosestHit()->getEntryPointName()) != "ClosestHitA0");
    SLANG_CHECK(hitGroupA0->getAnyHit()->getStage() == SLANG_STAGE_ANY_HIT);
    SLANG_CHECK(hitGroupA0->getAnyHit()->getEntryPointName() == nullptr);
    SLANG_CHECK(hitGroupA0->getIntersection() == nullptr);

    // Once a payload needs candidate logic, Metal traversal can encounter either native triangle
    // or bounding-box geometry regardless of which primitive kinds this schema declares. The
    // compiler therefore reflects both fixed IFT entries; an absent kind uses a reject-all stub.
    SLANG_CHECK(payloadA->getIntersectionFunctionTableSize() == 2);
    SLANG_CHECK(payloadA->getIntersectionFunctionCount() == 2);
    auto triangleIntersection = payloadA->getIntersectionFunction(0);
    SLANG_CHECK(triangleIntersection != nullptr);
    SLANG_CHECK(triangleIntersection->getIntersectionFunctionTableIndex() == 0);
    SLANG_CHECK(
        triangleIntersection->getGeometryKind() == SLANG_STRUCTURAL_RAY_TRACING_GEOMETRY_TRIANGLE);
    SLANG_CHECK(
        triangleIntersection->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(triangleIntersection->getEntryPointName() != nullptr);
    auto boundingBoxIntersection = payloadA->getIntersectionFunction(1);
    SLANG_CHECK(boundingBoxIntersection != nullptr);
    SLANG_CHECK(boundingBoxIntersection->getIntersectionFunctionTableIndex() == 1);
    SLANG_CHECK(
        boundingBoxIntersection->getGeometryKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_GEOMETRY_BOUNDING_BOX);
    SLANG_CHECK(
        boundingBoxIntersection->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(boundingBoxIntersection->getEntryPointName() != nullptr);

    SLANG_CHECK(payloadA->getMissShaderCount() == 2);
    auto missA0 = payloadA->getMissShader(0);
    auto missA1 = payloadA->getMissShader(1);
    SLANG_CHECK(missA0 != nullptr);
    SLANG_CHECK(missA1 != nullptr);
    SLANG_CHECK(missA0->getFunctionIndex() == 0);
    SLANG_CHECK(missA1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(missA0->getType()->getName()) == "MissA0");
    SLANG_CHECK(UnownedStringSlice(missA0->getRecordType()->getName()) == "MissRecordA");
    SLANG_CHECK(missA0->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(!missA0->isLinked());
    SLANG_CHECK(missA0->getMiss()->getStage() == SLANG_STAGE_MISS);
    SLANG_CHECK(missA0->getMiss()->getEntryPointName() != nullptr);
    SLANG_CHECK(UnownedStringSlice(missA0->getMiss()->getEntryPointName()) != "MissA0");

    auto payloadB = schema->getPayload(1);
    SLANG_CHECK(payloadB != nullptr);
    SLANG_CHECK(UnownedStringSlice(payloadB->getType()->getName()) == "PayloadB");
    SLANG_CHECK(payloadB->getTypeLayout() != nullptr);
    SLANG_CHECK(payloadB->getNativePayloadSize() == 0);
    SLANG_CHECK(payloadB->getHitGroupCount() == 1);
    SLANG_CHECK(payloadB->getHitGroup(0)->getFunctionIndex() == 0);
    SLANG_CHECK(payloadB->getMissShaderCount() == 1);
    SLANG_CHECK(payloadB->getMissShader(0)->getFunctionIndex() == 0);
    SLANG_CHECK(payloadB->getIntersectionFunctionTableSize() == 0);
    SLANG_CHECK(payloadB->getIntersectionFunctionCount() == 0);

    SLANG_CHECK(schema->getCallableShaderCount() == 2);
    auto callable0 = schema->getCallableShader(0);
    auto callable1 = schema->getCallableShader(1);
    SLANG_CHECK(callable0 != nullptr);
    SLANG_CHECK(callable1 != nullptr);
    SLANG_CHECK(callable0->getFunctionIndex() == 0);
    SLANG_CHECK(callable1->getFunctionIndex() == 1);
    SLANG_CHECK(UnownedStringSlice(callable0->getType()->getName()) == "Callable0");
    SLANG_CHECK(UnownedStringSlice(callable0->getRecordType()->getName()) == "CallableRecord");
    SLANG_CHECK(callable0->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(!callable0->isLinked());
    SLANG_CHECK(UnownedStringSlice(callable0->getDataType()->getName()) == "CallableData");
    SLANG_CHECK(callable0->getCallable()->getStage() == SLANG_STAGE_CALLABLE);
    SLANG_CHECK(callable0->getCallable()->getEntryPointName() != nullptr);
    SLANG_CHECK(UnownedStringSlice(callable0->getCallable()->getEntryPointName()) != "Callable0");
    // The physical record sections are shared by both payload partitions. HitRecordB therefore
    // raises the one schema-wide hit stride instead of giving payload B a private interpretation
    // of the same record index.
    SLANG_CHECK(schema->getHitRecordStride() == 48);
    SLANG_CHECK(schema->getMissRecordStride() == 32);
    SLANG_CHECK(schema->getCallableRecordStride() == 48);
    // Metal does not ask the host for native payload or attribute maxima. Its independent
    // structural record ABI starts application data after one fixed 16-byte header.
    SLANG_CHECK(schema->getMaxNativeHitAttributeSize() == 0);
    SLANG_CHECK(schema->getMetalRecordHeaderSize() == 16);

    SLANG_CHECK(schema->getDescriptorResourceCount() == 8);
    bool populatedMetalBindings[8] = {};
    for (SlangUInt resourceIndex = 0; resourceIndex < 8; ++resourceIndex)
    {
        // The lowering and reflection producers share the semantic resource-to-argument-index
        // mapping. Verify the public result is a complete one-to-one binding map rather than an
        // undocumented invitation for hosts to use `resourceIndex` as the Metal argument ID.
        const SlangInt bindingIndex =
            schema->getDescriptorResourceMetalArgumentBufferIndex(resourceIndex);
        SLANG_CHECK(bindingIndex >= 0 && bindingIndex < 8);
        if (bindingIndex >= 0 && bindingIndex < 8)
        {
            SLANG_CHECK(!populatedMetalBindings[bindingIndex]);
            populatedMetalBindings[bindingIndex] = true;
        }
    }
    for (bool populated : populatedMetalBindings)
        SLANG_CHECK(populated);
    SLANG_CHECK(schema->getDescriptorResourceMetalArgumentBufferIndex(8) == -1);
    SLANG_CHECK(
        schema->getDescriptorResourceKind(0) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_INTERSECTION_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(0) == 0);
    SLANG_CHECK(schema->getDescriptorResourceMetalArgumentBufferIndex(0) == 0);
    SLANG_CHECK(
        UnownedStringSlice(schema->getDescriptorResourceName(0)) == "intersectionFunctions0");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(3) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_INTERSECTION_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(3) == 1);
    SLANG_CHECK(schema->getDescriptorResourceMetalArgumentBufferIndex(3) == 3);
    SLANG_CHECK(
        UnownedStringSlice(schema->getDescriptorResourceName(3)) == "intersectionFunctions1");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(6) ==
        SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_CALLABLE_VISIBLE_FUNCTION_TABLE);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(6) == -1);
    SLANG_CHECK(schema->getDescriptorResourceMetalArgumentBufferIndex(6) == 6);
    SLANG_CHECK(UnownedStringSlice(schema->getDescriptorResourceName(6)) == "callableFunctions");
    SLANG_CHECK(
        schema->getDescriptorResourceKind(7) == SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_RECORDS);
    SLANG_CHECK(schema->getDescriptorResourcePayloadIndex(7) == -1);
    SLANG_CHECK(schema->getDescriptorResourceMetalArgumentBufferIndex(7) == 7);
    SLANG_CHECK(UnownedStringSlice(schema->getDescriptorResourceName(7)) == "records");

    SLANG_CHECK(program->findTraceProgramSchema("HitContextA") == nullptr);
}

SLANG_UNIT_TEST(structuralRayTracingNativeABISizeReflection)
{
    // None of these closed schemas is referenced by a trace call. Reflection must retain each
    // exact schema as a private target-manifest root; adding a trace use here would hide a producer
    // regression by making its stages reachable through executable code instead.
    // The nested aggregate intentionally distinguishes D3D allocation from Vulkan scalar-block
    // layout. D3D gives `NestedInner` its 16-byte allocation before placing `tail`, producing 24
    // bytes. Vulkan may reuse the inner aggregate's tail and produces a 16-byte block.
    const char* source = R"(
        import slang.raytracing;

        struct NestedInner { double wide; float narrow; }
        struct NestedValue { NestedInner inner; float tail; }
        struct ThreeWordPayload { uint a; uint b; uint c; }
        struct LargePayload { uint words[33]; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }
        typealias CommonTraceContext = TraceContext;

        struct CustomHitContext : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = NestedValue;
            typealias Primitive = rt::BoundingBoxPrimitive<NestedValue>;
            typealias Record = void;
        }

        struct CustomIntersection : rt::IIntersectionShader
        {
            typealias Context = CustomHitContext;
            void invoke(rt::IntersectionInput<Context> input) {}
        }

        struct CustomHitGroup : rt::IHitGroup
        {
            typealias Context = CustomHitContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = CustomIntersection;
        }

        struct TriangleHitContext : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = ThreeWordPayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = void;
        }

        struct TriangleClosestHit : rt::IClosestHitShader
        {
            typealias Context = TriangleHitContext;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct TriangleHitGroup : rt::IHitGroup
        {
            typealias Context = TriangleHitContext;
            typealias ClosestHit = TriangleClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct LargeMissContext : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = LargePayload;
            typealias Record = void;
        }

        struct LargeMiss : rt::IMissShader
        {
            typealias Context = LargeMissContext;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct CustomSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = CommonTraceContext;
            typealias HitGroups = rt::HitGroupList<CustomHitGroup>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        struct TriangleSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = CommonTraceContext;
            typealias HitGroups = rt::HitGroupList<TriangleHitGroup>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        struct LargeSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = CommonTraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::MissShaderList<LargeMiss>;
            typealias CallableShaders = rt::NoCallableShaders;
        }
    )";

    struct TargetExpectation
    {
        SlangCompileTarget target;
        const char* profile;
        const char* moduleName;
        size_t nestedPayloadSize;
        size_t customAttributeSize;
        size_t threeWordPayloadSize;
        size_t largePayloadSize;
        size_t emptySchemaAttributeSize;
    };
    static const TargetExpectation kTargets[] = {
        {SLANG_HLSL, "sm_6_5", "structuralNativeABID3D", 24, 24, 12, 132, 0},
        {SLANG_SPIRV, "spirv_1_5", "structuralNativeABIVulkan", 16, 16, 12, 132, 0},
        // OptiX transports the nested value in six 32-bit payload registers. Its custom-attribute
        // ABI flattens the same fields to four registers: two for the double and one for each
        // float. A 33-word payload switches to the compiler's two-register pointer representation.
        {SLANG_PTX, nullptr, "structuralNativeABIOptiX", 24, 16, 12, 8, 8},
    };

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    for (const auto& expectation : kTargets)
    {
        slang::TargetDesc target = {};
        target.format = expectation.target;
        if (expectation.profile)
            target.profile = globalSession->findProfile(expectation.profile);
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &target;
        sessionDesc.compilerOptionEntryCount = 1;
        sessionDesc.compilerOptionEntries = &experimentalOption;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            expectation.moduleName,
            expectation.moduleName,
            source,
            diagnostics.writeRef());
        if (!module && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(module != nullptr);

        auto layout = module->getLayout();
        auto customSchema = layout->findTraceProgramSchema("CustomSchema");
        auto triangleSchema = layout->findTraceProgramSchema("TriangleSchema");
        auto largeSchema = layout->findTraceProgramSchema("LargeSchema");
        SLANG_CHECK_ABORT(customSchema && triangleSchema && largeSchema);
        SLANG_CHECK(customSchema->getPayloadCount() == 1);
        SLANG_CHECK(triangleSchema->getPayloadCount() == 1);
        SLANG_CHECK(largeSchema->getPayloadCount() == 1);
        SLANG_CHECK(
            customSchema->getPayload(0)->getNativePayloadSize() == expectation.nestedPayloadSize);
        SLANG_CHECK(
            customSchema->getMaxNativeHitAttributeSize() == expectation.customAttributeSize);
        SLANG_CHECK(
            triangleSchema->getPayload(0)->getNativePayloadSize() ==
            expectation.threeWordPayloadSize);
        SLANG_CHECK(triangleSchema->getMaxNativeHitAttributeSize() == 8);
        SLANG_CHECK(
            largeSchema->getPayload(0)->getNativePayloadSize() == expectation.largePayloadSize);
        SLANG_CHECK(
            largeSchema->getMaxNativeHitAttributeSize() == expectation.emptySchemaAttributeSize);
        SLANG_CHECK(customSchema->getMetalRecordHeaderSize() == 0);
        SLANG_CHECK(triangleSchema->getMetalRecordHeaderSize() == 0);
        SLANG_CHECK(largeSchema->getMetalRecordHeaderSize() == 0);
    }
}

SLANG_UNIT_TEST(structuralRayTracingEntryPointRename)
{
    const char* expectedClosestHitName =
        "__slang_structural_rt_"
        "53746167654e616d6573706163652e54657374436c6f73657374486974";
    const char* expectedMissName =
        "__slang_structural_rt_53746167654e616d6573706163652e546573744d697373";
    const char* unspecializedGenericMissName =
        "__slang_structural_rt_"
        "53746167654e616d6573706163652e47656e657269634d697373";
    const char* expectedMainName = "__slang_structural_rt_6d61696e";

    const char* source = R"(
        import slang.raytracing;

        namespace StageNamespace
        {
            struct Payload { float value; }
            struct Record {}

            struct TraceContext : rt::ITraceContext
            {
                typealias AccelerationStructure = rt::AccelerationStructure;
                typealias Motion = rt::NoMotion;
            }

            struct MissContext : rt::IPayloadContext
            {
                typealias TraceContext = StageNamespace.TraceContext;
                typealias Payload = StageNamespace.Payload;
                typealias Record = StageNamespace.Record;
            }

            struct HitContext : rt::IHitContext
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias Payload = StageNamespace::Payload;
                typealias Primitive = rt::TrianglePrimitive;
                typealias Record = StageNamespace::Record;
            }

            struct TestClosestHit : rt::IClosestHitShader
            {
                typealias Context = HitContext;
                void invoke(rt::ClosestHitInput<HitContext> input)
                {
                    input.payload.value = input.triangle.barycentricCoord.x;
                }
            }

            struct HitGroup : rt::IHitGroup
            {
                typealias Context = HitContext;
                typealias ClosestHit = TestClosestHit;
                typealias AnyHit = rt::NoAnyHit<HitContext>;
                typealias Intersection = rt::NoIntersection<HitContext>;
            }

            struct TestMiss : rt::IMissShader
            {
                typealias Context = MissContext;
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 1.0f;
                }
            }

            struct Schema : rt::ITraceProgramSchema
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
                typealias MissShaders = rt::MissShaderList<StageNamespace::TestMiss>;
                typealias CallableShaders = rt::NoCallableShaders;
            }

            struct GenericMiss<T> : rt::IMissShader
            {
                typealias Context = MissContext;
                void invoke(rt::MissInput<MissContext> input)
                {
                    input.payload.value = 4.0f;
                }
            }

            struct GenericSchema<T> : rt::ITraceProgramSchema
            {
                typealias TraceContext = StageNamespace::TraceContext;
                typealias HitGroups = rt::NoHitGroups;
                typealias MissShaders = rt::MissShaderList<GenericMiss<T>>;
                typealias CallableShaders = rt::NoCallableShaders;
            }
        }

        struct main : rt::IMissShader
        {
            typealias Context = StageNamespace::MissContext;
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 2.0f;
            }
        }

        struct MainSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
            typealias MissShaders = rt::MissShaderList<main>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        // This legal source identifier is exactly the encoded spelling of
        // `StageNamespace.TestMiss`. It must itself be encoded to keep the mapping injective.
        struct __slang_structural_rt_53746167654e616d6573706163652e546573744d697373
            : rt::IMissShader
        {
            typealias Context = StageNamespace::MissContext;
            void invoke(rt::MissInput<StageNamespace::MissContext> input)
            {
                input.payload.value = 3.0f;
            }
        }

        struct ReservedPrefixSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = StageNamespace::TraceContext;
            typealias HitGroups = rt::HitGroupList<StageNamespace::HitGroup>;
            typealias MissShaders = rt::MissShaderList<
                __slang_structural_rt_53746167654e616d6573706163652e546573744d697373>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<StageNamespace::Schema> traceProgram;
        rt::TraceProgramDescriptor<StageNamespace::GenericSchema<uint>> genericTraceProgram;
        rt::TraceProgramDescriptor<StageNamespace::GenericSchema<float>> floatGenericTraceProgram;

        void traceHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::Schema> tracer;
            tracer.trace(desc, scene, traceProgram, payload);
        }

        void traceGenericHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::GenericSchema<uint>> tracer;
            tracer.trace(desc, scene, genericTraceProgram, payload);
        }

        void traceFloatGenericHelper(inout StageNamespace::Payload payload)
        {
            rt::RayTraversalDesc desc = {};
            desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
            desc.ray.tMax = 1.0f;
            desc.instanceMask = 0xff;
            rt::RayTracer<StageNamespace::GenericSchema<float>> tracer;
            tracer.trace(desc, scene, floatGenericTraceProgram, payload);
        }
    )";

    const char* raygenSource = R"(
        import structuralRename;

        [shader("raygeneration")]
        void raygenMain()
        {
            StageNamespace::Payload payload = {};
            traceHelper(payload);
        }

        [shader("raygeneration")]
        void genericRaygenMain()
        {
            StageNamespace::Payload payload = {};
            traceGenericHelper(payload);
            traceFloatGenericHelper(payload);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_CUDA_SOURCE;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralRename",
        "structural-rename.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    // Cache the ordinary module IR before selecting a structural stage below. The selected target
    // program must put the stage metadata on its layout-IR declaration, and linking must preserve
    // it when the cached module supplies the chosen body.
    auto reflectedProgram = module->getLayout();
    auto reflectedSchema = reflectedProgram->findTraceProgramSchema("StageNamespace.Schema");
    SLANG_CHECK_ABORT(reflectedSchema != nullptr);
    auto reflectedPayload = reflectedSchema->getPayload(0);
    SLANG_CHECK_ABORT(reflectedPayload != nullptr);
    auto reflectedClosestHit = reflectedPayload->getHitGroup(0)->getClosestHit();
    auto reflectedMiss = reflectedPayload->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedClosestHit != nullptr);
    SLANG_CHECK_ABORT(reflectedMiss != nullptr);
    String closestHitEntryPointName(reflectedClosestHit->getEntryPointName());
    String missEntryPointName(reflectedMiss->getEntryPointName());
    SLANG_CHECK(closestHitEntryPointName == expectedClosestHitName);
    SLANG_CHECK(missEntryPointName == expectedMissName);
    String expectedMissSymbol = String("__miss__") + missEntryPointName;
    String expectedClosestHitSymbol = String("__closesthit__") + closestHitEntryPointName;

    auto reflectedMainSchema = reflectedProgram->findTraceProgramSchema("MainSchema");
    SLANG_CHECK_ABORT(reflectedMainSchema != nullptr);
    auto reflectedMain = reflectedMainSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedMain != nullptr);
    String mainEntryPointName(reflectedMain->getEntryPointName());
    SLANG_CHECK(mainEntryPointName == expectedMainName);

    auto reflectedReservedSchema = reflectedProgram->findTraceProgramSchema("ReservedPrefixSchema");
    SLANG_CHECK_ABORT(reflectedReservedSchema != nullptr);
    auto reflectedReserved = reflectedReservedSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedReserved != nullptr);
    String reservedEntryPointName(reflectedReserved->getEntryPointName());
    SLANG_CHECK(reservedEntryPointName != expectedMissName);
    SLANG_CHECK(reservedEntryPointName.getUnownedSlice().startsWith("__slang_structural_rt_"));

    auto reflectedGenericSchema =
        reflectedProgram->findTraceProgramSchema("StageNamespace.GenericSchema<uint>");
    SLANG_CHECK_ABORT(reflectedGenericSchema != nullptr);
    SLANG_CHECK(
        UnownedStringSlice(reflectedGenericSchema->getName()) !=
        "StageNamespace.GenericSchema<uint>");
    auto reflectedGenericMiss = reflectedGenericSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedGenericMiss != nullptr);
    String genericMissEntryPointName(reflectedGenericMiss->getEntryPointName());
    SLANG_CHECK(genericMissEntryPointName != unspecializedGenericMissName);

    // Both types instantiate the same source declaration, but they are different semantic stage
    // types and must therefore advertise different physical entry-point names.
    auto reflectedFloatGenericSchema =
        reflectedProgram->findTraceProgramSchema("StageNamespace.GenericSchema<float>");
    SLANG_CHECK_ABORT(reflectedFloatGenericSchema != nullptr);
    auto reflectedFloatGenericMiss =
        reflectedFloatGenericSchema->getPayload(0)->getMissShader(0)->getMiss();
    SLANG_CHECK_ABORT(reflectedFloatGenericMiss != nullptr);
    String floatGenericMissEntryPointName(reflectedFloatGenericMiss->getEntryPointName());
    SLANG_CHECK(floatGenericMissEntryPointName != unspecializedGenericMissName);
    SLANG_CHECK(floatGenericMissEntryPointName != genericMissEntryPointName);

    ComPtr<slang::IModule> raygenModule(session->loadModuleFromSourceString(
        "structuralRenameRaygen",
        "structural-rename-raygen.slang",
        raygenSource,
        diagnostics.writeRef()));
    if (!raygenModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(raygenModule != nullptr);

    // Deliberately select this stage after `getLayout()` has cached the undecorated module body.
    ComPtr<slang::IEntryPoint> sourceEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "StageNamespace.TestMiss",
        SLANG_STAGE_MISS,
        sourceEntryPoint.writeRef(),
        diagnostics.writeRef())));

    // Materializing the selected structural stage without a client rename must use the same
    // target-safe default that reflection advertises.
    ComPtr<slang::IComponentType> linkedUnrenamedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        sourceEntryPoint->link(linkedUnrenamedEntryPoint.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> unrenamedCode;
    auto unrenamedResult = linkedUnrenamedEntryPoint->getEntryPointCode(
        0,
        0,
        unrenamedCode.writeRef(),
        diagnostics.writeRef());
    if (SLANG_FAILED(unrenamedResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unrenamedResult));
    SLANG_CHECK_ABORT(unrenamedCode != nullptr);
    UnownedStringSlice unrenamedGeneratedCode(
        (const char*)unrenamedCode->getBufferPointer(),
        (const char*)unrenamedCode->getBufferPointer() + unrenamedCode->getBufferSize());
    SLANG_CHECK(unrenamedGeneratedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(unrenamedGeneratedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);

    ComPtr<slang::IEntryPoint> mainEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_MISS,
        mainEntryPoint.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedMainEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        mainEntryPoint->link(linkedMainEntryPoint.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> mainCode;
    auto mainResult =
        linkedMainEntryPoint->getEntryPointCode(0, 0, mainCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(mainResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(mainResult));
    SLANG_CHECK_ABORT(mainCode != nullptr);
    UnownedStringSlice mainGeneratedCode(
        (const char*)mainCode->getBufferPointer(),
        (const char*)mainCode->getBufferPointer() + mainCode->getBufferSize());
    String expectedMainSymbol = String("__miss__") + mainEntryPointName;
    SLANG_CHECK(mainGeneratedCode.indexOf(expectedMainSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(mainGeneratedCode.indexOf(toSlice("__miss__main_")) == -1);

    ComPtr<slang::IComponentType> renamedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        sourceEntryPoint->renameEntryPoint("renamedStructuralMiss", renamedEntryPoint.writeRef())));

    ComPtr<slang::IEntryPoint> sourceClosestHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "StageNamespace.TestClosestHit",
        SLANG_STAGE_CLOSEST_HIT,
        sourceClosestHit.writeRef(),
        diagnostics.writeRef())));

    ComPtr<slang::IComponentType> renamedClosestHit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(sourceClosestHit->renameEntryPoint(
        "renamedStructuralClosestHit",
        renamedClosestHit.writeRef())));

    ComPtr<slang::IEntryPoint> raygenEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        raygenModule->findEntryPointByName("raygenMain", raygenEntryPoint.writeRef())));

    // This composite contains no selected structural stage components. The raygen trace operation
    // must synthesize the same names that the program-schema reflection above advertised.
    slang::IComponentType* autoComponents[] = {raygenModule, module, raygenEntryPoint};
    ComPtr<slang::IComponentType> autoProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        autoComponents,
        SLANG_COUNT_OF(autoComponents),
        autoProgram.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedAutoProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(autoProgram->link(linkedAutoProgram.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> autoCode;
    auto autoResult =
        linkedAutoProgram->getEntryPointCode(0, 0, autoCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(autoResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(autoResult));
    SLANG_CHECK_ABORT(autoCode != nullptr);
    UnownedStringSlice autoGeneratedCode(
        (const char*)autoCode->getBufferPointer(),
        (const char*)autoCode->getBufferPointer() + autoCode->getBufferSize());
    SLANG_CHECK(autoGeneratedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(autoGeneratedCode.indexOf(expectedClosestHitSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(autoGeneratedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);
    SLANG_CHECK(
        autoGeneratedCode.indexOf(toSlice("__closesthit__StageNamespace.TestClosestHit")) == -1);

    ComPtr<slang::IEntryPoint> genericRaygenEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(raygenModule->findEntryPointByName(
        "genericRaygenMain",
        genericRaygenEntryPoint.writeRef())));
    slang::IComponentType* genericComponents[] = {
        raygenModule,
        module,
        genericRaygenEntryPoint,
    };
    ComPtr<slang::IComponentType> genericProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        genericComponents,
        SLANG_COUNT_OF(genericComponents),
        genericProgram.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedGenericProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        genericProgram->link(linkedGenericProgram.writeRef(), diagnostics.writeRef())));
    ComPtr<slang::IBlob> genericCode;
    auto genericResult = linkedGenericProgram->getEntryPointCode(
        0,
        0,
        genericCode.writeRef(),
        diagnostics.writeRef());
    if (SLANG_FAILED(genericResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(genericResult));
    SLANG_CHECK_ABORT(genericCode != nullptr);
    UnownedStringSlice genericGeneratedCode(
        (const char*)genericCode->getBufferPointer(),
        (const char*)genericCode->getBufferPointer() + genericCode->getBufferSize());
    String expectedGenericMissSymbol = String("__miss__") + genericMissEntryPointName;
    String expectedFloatGenericMissSymbol = String("__miss__") + floatGenericMissEntryPointName;
    SLANG_CHECK(genericGeneratedCode.indexOf(expectedGenericMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(
        genericGeneratedCode.indexOf(expectedFloatGenericMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(genericGeneratedCode.indexOf(toSlice("GenericMiss<uint>")) == -1);

    slang::IComponentType* components[] = {
        raygenModule,
        module,
        raygenEntryPoint,
        renamedClosestHit,
        renamedEntryPoint,
    };
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IBlob> code;
    auto result = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);

    UnownedStringSlice generatedCode(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(expectedMissSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(generatedCode.indexOf(expectedClosestHitSymbol.getUnownedSlice()) != -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__renamedStructuralMiss")) == -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__closesthit__renamedStructuralClosestHit")) == -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__StageNamespace.TestMiss")) == -1);
    SLANG_CHECK(
        generatedCode.indexOf(toSlice("__closesthit__StageNamespace.TestClosestHit")) == -1);

    code.setNull();
    result = linkedProgram->getEntryPointCode(1, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);
    generatedCode = UnownedStringSlice(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(toSlice("__closesthit__renamedStructuralClosestHit")) != -1);
    // Per-entry code generation must not promote structural metadata retained for another selected
    // component. This product owns only the renamed closest-hit adapter and has no trace operation
    // that would request a schema-stable adapter.
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__renamedStructuralMiss")) == -1);
    SLANG_CHECK(generatedCode.indexOf(expectedClosestHitSymbol.getUnownedSlice()) == -1);

    code.setNull();
    result = linkedProgram->getEntryPointCode(2, 0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(code != nullptr);
    generatedCode = UnownedStringSlice(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
    SLANG_CHECK(generatedCode.indexOf(toSlice("__miss__renamedStructuralMiss")) != -1);
    SLANG_CHECK(generatedCode.indexOf(toSlice("__closesthit__renamedStructuralClosestHit")) == -1);
    SLANG_CHECK(generatedCode.indexOf(expectedMissSymbol.getUnownedSlice()) == -1);

    // The castable interface is available on every compiled target, but target-only Metal ABI
    // records must not leak into CUDA (or any other portable target).
    ComPtr<slang::IMetadata> targetMetadata;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        linkedProgram->getTargetMetadata(0, targetMetadata.writeRef(), diagnostics.writeRef())));
    auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
        targetMetadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
    SLANG_CHECK_ABORT(structuralMetadata != nullptr);
    SLANG_CHECK(structuralMetadata->getMetalPayloadInfoCount() == 0);
}

SLANG_UNIT_TEST(structuralRayTracingMetalTargetMetadata)
{
    const char* source = R"(
        import slang.raytracing;

        struct Payload { float value; }
        struct CustomAttributes { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct TriangleContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = void;
        }

        struct BoundingBoxContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
            typealias Record = void;
        }

        struct CurveContext : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Primitive = rt::CurvePrimitive;
            typealias Record = void;
        }

        struct BoundingBoxIntersection : rt::IIntersectionShader
        {
            typealias Context = BoundingBoxContext;
            void invoke(rt::IntersectionInput<Context> input)
            {
                CustomAttributes attributes = { input.worldSpaceOrigin.x };
                input.reportHit(1.0, attributes);
            }
        }

        struct TriangleGroup : rt::IHitGroup
        {
            typealias Context = TriangleContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct BoundingBoxGroup : rt::IHitGroup
        {
            typealias Context = BoundingBoxContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = BoundingBoxIntersection;
        }

        struct CurveGroup : rt::IHitGroup
        {
            typealias Context = CurveContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MetadataSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups =
                rt::HitGroupList<TriangleGroup, BoundingBoxGroup, CurveGroup>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<MetadataSchema> program;
        RWStructuredBuffer<float> output;

        [shader("raygeneration")]
        void main()
        {
            rt::RayTraversalDesc desc = {};
            Payload payload = {};
            rt::RayTracer<MetadataSchema> tracer;
            tracer.trace(desc, scene, program, payload);
            output[0] = payload.value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralMetalMetadata",
        "structural-metal-metadata.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composite.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(composite->link(linkedProgram.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IBlob> generatedCode;
    auto codeResult =
        linkedProgram->getEntryPointCode(0, 0, generatedCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(codeResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult));

    auto schema = linkedProgram->getLayout()->findTraceProgramSchema("MetadataSchema");
    SLANG_CHECK_ABORT(schema != nullptr);
    SLANG_CHECK_ABORT(schema->getName() != nullptr);
    auto payload = schema->getPayload(0);
    SLANG_CHECK_ABORT(payload != nullptr);

    // Activating bounding-box candidate logic generates fixed-index dispatchers for every geometry
    // kind this payload may encounter. Triangle and curve have explicit accepting arms for the
    // schema's fixed-function groups; no host-authored opaque function bypasses record selection.
    SLANG_CHECK(payload->getIntersectionFunctionTableSize() == 3);
    SLANG_CHECK(payload->getIntersectionFunctionCount() == 3);
    auto triangleFunction = payload->getIntersectionFunction(0);
    auto boundingBoxFunction = payload->getIntersectionFunction(1);
    auto curveFunction = payload->getIntersectionFunction(2);
    SLANG_CHECK_ABORT(triangleFunction && boundingBoxFunction && curveFunction);
    SLANG_CHECK(triangleFunction->getIntersectionFunctionTableIndex() == 0);
    SLANG_CHECK(
        triangleFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(triangleFunction->getEntryPointName() != nullptr);
    SLANG_CHECK(boundingBoxFunction->getIntersectionFunctionTableIndex() == 1);
    SLANG_CHECK(
        boundingBoxFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(boundingBoxFunction->getEntryPointName() != nullptr);
    SLANG_CHECK(curveFunction->getIntersectionFunctionTableIndex() == 2);
    SLANG_CHECK(
        curveFunction->getImplementationKind() ==
        SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION);
    SLANG_CHECK(curveFunction->getEntryPointName() != nullptr);

    // Source reflection keeps `NoClosestHit` absent. This schema has no real closest-hit stage, but
    // every group still reflects the same signature-compatible no-op so its dense VFT has no hole.
    auto triangleGroup = payload->getHitGroup(0);
    auto boundingBoxGroup = payload->getHitGroup(1);
    auto curveGroup = payload->getHitGroup(2);
    SLANG_CHECK_ABORT(triangleGroup && boundingBoxGroup && curveGroup);
    SLANG_CHECK(triangleGroup->getClosestHit() == nullptr);
    SLANG_CHECK(boundingBoxGroup->getClosestHit() == nullptr);
    SLANG_CHECK(curveGroup->getClosestHit() == nullptr);
    SLANG_CHECK(triangleGroup->getClosestHitEntryPointName() != nullptr);
    SLANG_CHECK(boundingBoxGroup->getClosestHitEntryPointName() != nullptr);
    SLANG_CHECK(curveGroup->getClosestHitEntryPointName() != nullptr);
    SLANG_CHECK(
        UnownedStringSlice(triangleGroup->getClosestHitEntryPointName()) ==
        boundingBoxGroup->getClosestHitEntryPointName());
    SLANG_CHECK(
        UnownedStringSlice(triangleGroup->getClosestHitEntryPointName()) ==
        curveGroup->getClosestHitEntryPointName());
    SLANG_CHECK(UnownedStringSlice(triangleGroup->getClosestHitEntryPointName())
                    .startsWith("__slang_structural_rt_"));
    UnownedStringSlice code(
        (const char*)generatedCode->getBufferPointer(),
        (const char*)generatedCode->getBufferPointer() + generatedCode->getBufferSize());
    // Include the opening parenthesis so an emitted `reflectedName_0` does not pass as a prefix.
    // Hosts look up these functions by the reflected spelling, so the complete identifier is ABI.
    auto containsExactFunctionName = [&](const char* name)
    {
        StringBuilder token;
        token << name << "(";
        return code.indexOf(token.getUnownedSlice()) != -1;
    };
    SLANG_CHECK(containsExactFunctionName(triangleGroup->getClosestHitEntryPointName()));
    SLANG_CHECK(containsExactFunctionName(triangleFunction->getEntryPointName()));
    SLANG_CHECK(containsExactFunctionName(boundingBoxFunction->getEntryPointName()));
    SLANG_CHECK(containsExactFunctionName(curveFunction->getEntryPointName()));

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef())));
    auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
        metadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
    SLANG_CHECK_ABORT(structuralMetadata != nullptr);
    SLANG_CHECK(structuralMetadata->getMetalPayloadInfoCount() == 1);
    slang::StructuralRayTracingMetalPayloadInfo info = {};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(structuralMetadata->getMetalPayloadInfo(0, &info)));
    SLANG_CHECK(UnownedStringSlice(info.schemaName) == schema->getName());
    SLANG_CHECK(info.payloadIndex == 0);
    const auto signature = uint32_t(info.intersectionFunctionSignature);
    SLANG_CHECK((signature & uint32_t(slang::MetalIntersectionFunctionSignature::Instancing)) != 0);
    SLANG_CHECK(
        (signature & uint32_t(slang::MetalIntersectionFunctionSignature::WorldSpaceData)) != 0);
}

SLANG_UNIT_TEST(structuralRayTracingMetalTargetMetadataOperationOrder)
{
    const char* source = R"(
        import slang.raytracing;

        struct PayloadA { uint value; }
        struct PayloadB { float value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct HitContextA : rt::IHitContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadA;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = void;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadB;
            typealias Record = void;
        }

        struct AnyHitA : rt::IAnyHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::AnyHitInput<Context> input)
            {
                input.payload.value = input.triangle.frontFacing ? 1 : 0;
            }
        }

        struct HitGroupA : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = AnyHitA;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MissB : rt::IMissShader
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<Context> input) { input.payload.value = 2.0; }
        }

        struct Schema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::HitGroupList<HitGroupA>;
            typealias MissShaders = rt::MissShaderList<MissB>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<Schema> program;
        RWStructuredBuffer<float2> output;

        [shader("raygeneration")]
        void main()
        {
            rt::RayTraversalDesc desc = {};
            rt::RayTracer<Schema> tracer;
            PayloadA a = {};
            PayloadB b = {};
        #if REVERSE
            tracer.trace(desc, scene, program, b);
            tracer.trace(desc, scene, program, a);
        #else
            tracer.trace(desc, scene, program, a);
            tracer.trace(desc, scene, program, b);
        #endif
            output[0] = float2(a.value, b.value);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto compileSignatures = [&](bool reverse, uint32_t(&outSignatures)[2])
    {
        slang::CompilerOptionEntry options[2] = {};
        options[0].name = slang::CompilerOptionName::ExperimentalFeature;
        options[0].value.kind = slang::CompilerOptionValueKind::Int;
        options[0].value.intValue0 = 1;
        options[1].name = slang::CompilerOptionName::MacroDefine;
        options[1].value.kind = slang::CompilerOptionValueKind::String;
        options[1].value.stringValue0 = "REVERSE";
        options[1].value.stringValue1 = reverse ? "1" : "0";

        slang::TargetDesc target = {};
        target.format = SLANG_METAL;
        target.profile = globalSession->findProfile("metal_3_1");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &target;
        sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(options);
        sessionDesc.compilerOptionEntries = options;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            reverse ? "structuralMetadataReverse" : "structuralMetadataForward",
            reverse ? "structural-metadata-reverse.slang" : "structural-metadata-forward.slang",
            source,
            diagnostics.writeRef()));
        if (!module && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
        slang::IComponentType* components[] = {module, entryPoint};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            composite.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(composite->link(linkedProgram.writeRef(), diagnostics.writeRef())));

        ComPtr<slang::IBlob> generatedCode;
        auto codeResult = linkedProgram->getEntryPointCode(
            0,
            0,
            generatedCode.writeRef(),
            diagnostics.writeRef());
        if (SLANG_FAILED(codeResult) && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult));

        auto schema = linkedProgram->getLayout()->findTraceProgramSchema("Schema");
        SLANG_CHECK_ABORT(schema != nullptr);
        SLANG_CHECK(schema->getPayloadCount() == 2);

        ComPtr<slang::IMetadata> metadata;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef())));
        auto structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
            metadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
        SLANG_CHECK_ABORT(structuralMetadata != nullptr);
        SLANG_CHECK_ABORT(structuralMetadata->getMetalPayloadInfoCount() == 2);
        for (uint32_t i = 0; i < 2; ++i)
        {
            slang::StructuralRayTracingMetalPayloadInfo info = {};
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(structuralMetadata->getMetalPayloadInfo(i, &info)));
            SLANG_CHECK_ABORT(UnownedStringSlice(info.schemaName) == schema->getName());
            SLANG_CHECK_ABORT(info.payloadIndex < 2);
            outSignatures[info.payloadIndex] = uint32_t(info.intersectionFunctionSignature);
        }
    };

    uint32_t forwardSignatures[2] = {};
    uint32_t reverseSignatures[2] = {};
    compileSignatures(false, forwardSignatures);
    compileSignatures(true, reverseSignatures);
    SLANG_CHECK(forwardSignatures[0] == reverseSignatures[0]);
    SLANG_CHECK(forwardSignatures[1] == reverseSignatures[1]);
    SLANG_CHECK(
        (forwardSignatures[0] &
         uint32_t(slang::MetalIntersectionFunctionSignature::TriangleData)) != 0);
    SLANG_CHECK(
        (forwardSignatures[1] &
         uint32_t(slang::MetalIntersectionFunctionSignature::TriangleData)) == 0);
}

SLANG_UNIT_TEST(structuralRayTracingOpenSchemaReflection)
{
    // This test models the host workflow open schemas are designed for. The schema module names
    // only tag interfaces, while an independently composed module contributes concrete entries.
    // The host asks the ordinary composite program for reflection; it does not construct or add a
    // TypeConformance component for `Schema` itself.
    const char* contextSource = R"(
        module structural_open_reflection_context;

        import slang.raytracing;

        public struct ListedPayload { uint value; }
        public struct LinkedPayload { float2 value; }
        public struct ListedRecord { uint value; }
        public struct LinkedRecord { float4 value; }
        public struct CallableData { uint value; }

        public struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        public struct OtherTraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        public typealias CommonTraceContext = TraceContext;
        public typealias CommonCallableData = CallableData;

        public struct ListedHitContext : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = ListedPayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = ListedRecord;
        }

        public struct LinkedHitContext : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = LinkedPayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = LinkedRecord;
        }

        public struct MismatchedHitContext : rt::IHitContext
        {
            typealias TraceContext = OtherTraceContext;
            typealias Payload = LinkedPayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = LinkedRecord;
        }

        public struct ListedMissContext : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = ListedPayload;
            typealias Record = ListedRecord;
        }

        public struct LinkedMissContext : rt::IPayloadContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = LinkedPayload;
            typealias Record = LinkedRecord;
        }

        public struct ListedCallableContext : rt::ICallableContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias CallableData = CommonCallableData;
            typealias Record = ListedRecord;
        }

        public struct LinkedCallableContext : rt::ICallableContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias CallableData = CommonCallableData;
            typealias Record = LinkedRecord;
        }

        public interface IHitTag : rt::IHitGroup {}
        public interface IMismatchedHitTag : rt::IHitGroup {}
        public interface IMissTag : rt::IMissShader {}
        public interface ICallableTag : rt::ICallableShader {}
    )";

    const char* schemaSource = R"(
        module structural_open_reflection_schema;

        import slang.raytracing;
        import structural_open_reflection_context;

        typealias SchemaTraceContext = TraceContext;

        struct ListedClosestHit : rt::IClosestHitShader
        {
            typealias Context = ListedHitContext;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct ListedHitGroup : IHitTag
        {
            typealias Context = ListedHitContext;
            typealias ClosestHit = ListedClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct ListedMiss : IMissTag
        {
            typealias Context = ListedMissContext;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct ListedCallable : ICallableTag
        {
            typealias Context = ListedCallableContext;
            void invoke(rt::CallableInput<Context> input) {}
        }

        public struct Schema : rt::ITraceProgramSchema
        {
            typealias TraceContext = SchemaTraceContext;
            typealias HitGroups = rt::OpenHitGroups<IHitTag, ListedHitGroup>;
            typealias MissShaders = rt::OpenMissShaders<IMissTag, ListedMiss>;
            typealias CallableShaders = rt::OpenCallableShaders<ICallableTag, ListedCallable>;
        }

        public struct MismatchedSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = SchemaTraceContext;
            typealias HitGroups = rt::OpenHitGroups<IMismatchedHitTag>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }
    )";

    const char* pluginSource = R"(
        module structural_open_reflection_plugin;

        import slang.raytracing;
        import structural_open_reflection_context;

        struct LinkedClosestHit : rt::IClosestHitShader
        {
            typealias Context = LinkedHitContext;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct LinkedHitGroup : IHitTag
        {
            typealias Context = LinkedHitContext;
            typealias ClosestHit = LinkedClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MismatchedHitGroup : IMismatchedHitTag
        {
            typealias Context = MismatchedHitContext;
            typealias ClosestHit = rt::NoClosestHit<Context>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct LinkedMiss : IMissTag
        {
            typealias Context = LinkedMissContext;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct LinkedCallable : ICallableTag
        {
            typealias Context = LinkedCallableContext;
            void invoke(rt::CallableInput<Context> input) {}
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    auto loadModule = [&](const char* moduleName, const char* source) -> ComPtr<slang::IModule>
    {
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            moduleName,
            moduleName,
            source,
            diagnostics.writeRef()));
        if (!module && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        return module;
    };

    auto contextModule = loadModule("structural_open_reflection_context", contextSource);
    SLANG_CHECK_ABORT(contextModule != nullptr);
    auto schemaModule = loadModule("structural_open_reflection_schema", schemaSource);
    SLANG_CHECK_ABORT(schemaModule != nullptr);
    auto pluginModule = loadModule("structural_open_reflection_plugin", pluginSource);
    SLANG_CHECK_ABORT(pluginModule != nullptr);

    slang::IComponentType* components[] = {schemaModule, pluginModule};
    ComPtr<slang::IComponentType> program;
    ComPtr<slang::IBlob> diagnostics;
    auto composeResult = session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef());
    if (SLANG_FAILED(composeResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(composeResult));

    auto layout = program->getLayout(0, diagnostics.writeRef());
    if (!layout && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(layout != nullptr);
    auto schema = layout->findTraceProgramSchema("Schema");
    SLANG_CHECK_ABORT(schema != nullptr);

    // Open flags report the source contract, while all returned collections are already finalized
    // for the composed program. Listed entries retain index zero and linked entries follow them.
    SLANG_CHECK(schema->isHitGroupSectionOpen());
    SLANG_CHECK(schema->isMissShaderSectionOpen());
    SLANG_CHECK(schema->isCallableShaderSectionOpen());
    SLANG_CHECK(schema->getPayloadCount() == 2);
    auto listedPayload = schema->getPayload(0);
    auto linkedPayload = schema->getPayload(1);
    SLANG_CHECK_ABORT(listedPayload && linkedPayload);
    SLANG_CHECK(UnownedStringSlice(listedPayload->getType()->getName()) == "ListedPayload");
    SLANG_CHECK(UnownedStringSlice(linkedPayload->getType()->getName()) == "LinkedPayload");
    SLANG_CHECK(listedPayload->getTypeLayout() != nullptr);
    SLANG_CHECK(linkedPayload->getTypeLayout() != nullptr);

    SLANG_CHECK(listedPayload->getHitGroupCount() == 1);
    SLANG_CHECK(linkedPayload->getHitGroupCount() == 1);
    auto listedHit = listedPayload->getHitGroup(0);
    auto linkedHit = linkedPayload->getHitGroup(0);
    SLANG_CHECK_ABORT(listedHit && linkedHit);
    SLANG_CHECK(!listedHit->isLinked());
    SLANG_CHECK(linkedHit->isLinked());
    SLANG_CHECK(UnownedStringSlice(listedHit->getType()->getName()) == "ListedHitGroup");
    SLANG_CHECK(UnownedStringSlice(linkedHit->getType()->getName()) == "LinkedHitGroup");
    SLANG_CHECK(listedHit->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(linkedHit->getRecordTypeLayout() != nullptr);

    SLANG_CHECK(listedPayload->getMissShaderCount() == 1);
    SLANG_CHECK(linkedPayload->getMissShaderCount() == 1);
    auto listedMiss = listedPayload->getMissShader(0);
    auto linkedMiss = linkedPayload->getMissShader(0);
    SLANG_CHECK_ABORT(listedMiss && linkedMiss);
    SLANG_CHECK(!listedMiss->isLinked());
    SLANG_CHECK(linkedMiss->isLinked());
    SLANG_CHECK(UnownedStringSlice(listedMiss->getType()->getName()) == "ListedMiss");
    SLANG_CHECK(UnownedStringSlice(linkedMiss->getType()->getName()) == "LinkedMiss");
    SLANG_CHECK(listedMiss->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(linkedMiss->getRecordTypeLayout() != nullptr);

    SLANG_CHECK(schema->getCallableShaderCount() == 2);
    auto listedCallable = schema->getCallableShader(0);
    auto linkedCallable = schema->getCallableShader(1);
    SLANG_CHECK_ABORT(listedCallable && linkedCallable);
    SLANG_CHECK(!listedCallable->isLinked());
    SLANG_CHECK(linkedCallable->isLinked());
    SLANG_CHECK(UnownedStringSlice(listedCallable->getType()->getName()) == "ListedCallable");
    SLANG_CHECK(UnownedStringSlice(linkedCallable->getType()->getName()) == "LinkedCallable");
    SLANG_CHECK(listedCallable->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(linkedCallable->getRecordTypeLayout() != nullptr);

    // The completed schema is cached on the ordinary program layout, so repeated host queries do
    // not relink or construct a second reflection object.
    SLANG_CHECK(layout->findTraceProgramSchema("Schema") == schema);

    // This second schema is valid in its defining module, but the composed plugin contributes a
    // hit group with a different trace context. The pointer-only reflection query must not publish
    // that invalid finalized schema. It cannot return the link-time diagnostic; the corresponding
    // code-generation diagnostic is covered by open-schema-trace-context-mismatch.slang.
    SLANG_CHECK(layout->findTraceProgramSchema("MismatchedSchema") == nullptr);
}

SLANG_UNIT_TEST(structuralRayTracingEmptyPayloadInvariantAppliesToReflectionOnlySchemas)
{
    const char* source = R"(
        import slang.raytracing;

        struct EmptyPayloadA {}
        struct EmptyPayloadB {}

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        interface IMissTag : rt::IMissShader {}
        interface IValidMissTag : rt::IMissShader {}

        struct MissContext<PayloadType> : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadType;
            typealias Record = void;
        }

        struct MissA : IMissTag
        {
            typealias Context = MissContext<EmptyPayloadA>;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct MissB : IMissTag
        {
            typealias Context = MissContext<EmptyPayloadB>;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct ValidMiss : IValidMissTag
        {
            typealias Context = MissContext<EmptyPayloadA>;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct ClosedSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::MissShaderList<MissA, MissB>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        struct OpenSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::OpenMissShaders<IMissTag>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        struct ValidClosedSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::MissShaderList<ValidMiss>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        struct ValidOpenSchema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::OpenMissShaders<IValidMissTag>;
            typealias CallableShaders = rt::NoCallableShaders;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralReflectionOnlyEmptyPayloads",
        "structural-reflection-only-empty-payloads.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto layout = module->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout != nullptr);

    // None of these schemas is activated by an entry point or trace. Positive controls prove that
    // ordinary reflection succeeds through both paths; the invalid closed list and linked open
    // section must fail specifically because each contains two empty payload identities.
    SLANG_CHECK(layout->findTraceProgramSchema("ValidClosedSchema") != nullptr);
    SLANG_CHECK(layout->findTraceProgramSchema("ValidOpenSchema") != nullptr);
    SLANG_CHECK(layout->findTraceProgramSchema("ClosedSchema") == nullptr);
    SLANG_CHECK(layout->findTraceProgramSchema("OpenSchema") == nullptr);
}

SLANG_UNIT_TEST(structuralRayTracingInvalidOpenSchemaManifestIsNotCached)
{
    const char* source = R"(
        import slang.raytracing;

        struct EmptyPayloadA {}
        struct EmptyPayloadB {}
        struct PayloadWithData { uint value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        interface IMissTag : rt::IMissShader {}

        struct MissContextA : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = EmptyPayloadA;
            typealias Record = void;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = EmptyPayloadB;
            typealias Record = void;
        }

        struct MissA : IMissTag
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct MissB : IMissTag
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct DataMissContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = PayloadWithData;
            typealias Record = void;
        }

        struct DataMiss : rt::IMissShader
        {
            typealias Context = DataMissContext;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct Schema : rt::ITraceProgramSchema
        {
            typealias TraceContext = ::TraceContext;
            typealias HitGroups = rt::NoHitGroups;
            typealias MissShaders = rt::OpenMissShaders<IMissTag, DataMiss>;
            typealias CallableShaders = rt::NoCallableShaders;
        }

        rt::AccelerationStructure scene;
        rt::TraceProgramDescriptor<Schema> program;

        [shader("raygeneration")]
        void main()
        {
            rt::RayTracer<Schema> tracer;
            PayloadWithData payload = {};
            tracer.trace<PayloadWithData>({}, scene, program, payload);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralInvalidOpenManifest",
        "structural-invalid-open-manifest.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())));

    // Reflection requests a finalized schema even when no payload-less trace is present. The two
    // linked empty payload identities therefore invalidate the same schema before reflection can
    // publish it; choosing `PayloadWithData` in `main` cannot hide that schema-wide conflict.
    auto layout = program->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout != nullptr);
    SLANG_CHECK(layout->findTraceProgramSchema("Schema") == nullptr);

    auto expectAmbiguousPayloadDiagnostic = [&]()
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> requestDiagnostics;
        auto result =
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), requestDiagnostics.writeRef());
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK_ABORT(requestDiagnostics != nullptr);
        UnownedStringSlice text(
            (const char*)requestDiagnostics->getBufferPointer(),
            (const char*)requestDiagnostics->getBufferPointer() +
                requestDiagnostics->getBufferSize());
        SLANG_CHECK(
            text.indexOf(
                toSlice("linked structural ray-tracing schema has multiple empty payloads")) != -1);
    };

    // Both requests share one TargetProgram. The first invalid manifest must not enter that
    // program's cache; otherwise the second request would reuse a trace-less module and lose the
    // link-time ambiguity diagnostic.
    expectAmbiguousPayloadDiagnostic();
    expectAmbiguousPayloadDiagnostic();
}

SLANG_UNIT_TEST(structuralRayTracingSerializedOpenSchemaReflection)
{
    // A standard or plugin module is normally shipped as serialized AST and IR. Compile the three
    // independently composable pieces first, then discard that session so no transient compiler
    // registry can make reflection pass accidentally.
    const char* contextSource = R"(
        module structural_serialized_reflection_context;

        import slang.raytracing;

        public struct Payload { uint value; }
        public struct Record { uint4 value; }

        public struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        public typealias CommonTraceContext = TraceContext;
        public typealias CommonPayload = Payload;
        public typealias CommonRecord = Record;

        public struct HitContext : rt::IHitContext
        {
            typealias TraceContext = CommonTraceContext;
            typealias Payload = CommonPayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = CommonRecord;
        }

        public interface IHitTag : rt::IHitGroup {}
    )";

    const char* schemaSource = R"(
        module structural_serialized_reflection_schema;

        import slang.raytracing;
        import structural_serialized_reflection_context;

        typealias SchemaTraceContext = TraceContext;

        public struct Schema : rt::ITraceProgramSchema
        {
            typealias TraceContext = SchemaTraceContext;
            typealias HitGroups = rt::OpenHitGroups<IHitTag>;
            typealias MissShaders = rt::NoMissShaders;
            typealias CallableShaders = rt::NoCallableShaders;
        }
    )";

    const char* pluginSource = R"(
        module structural_serialized_reflection_plugin;

        import slang.raytracing;
        import structural_serialized_reflection_context;

        struct LinkedClosestHit : rt::IClosestHitShader
        {
            typealias Context = HitContext;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct LinkedHitGroup : IHitTag
        {
            typealias Context = HitContext;
            typealias ClosestHit = LinkedClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        public struct GenericClosestHit<T> : rt::IClosestHitShader
        {
            typealias Context = HitContext;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        public struct GenericHitGroup<T> : IHitTag
        {
            typealias Context = HitContext;
            typealias ClosestHit = GenericClosestHit<T>;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry compilerOptions[2] = {};
    compilerOptions[0].name = slang::CompilerOptionName::ExperimentalFeature;
    compilerOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[0].value.intValue0 = 1;
    // Obfuscation ensures that recovery cannot accidentally depend on an IR linkage decoration.
    // The producer persists the original module export-table key in compiler-owned metadata.
    compilerOptions[1].name = slang::CompilerOptionName::Obfuscate;
    compilerOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[1].value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(compilerOptions);
    sessionDesc.compilerOptionEntries = compilerOptions;

    ComPtr<slang::IBlob> contextBlob;
    ComPtr<slang::IBlob> schemaBlob;
    ComPtr<slang::IBlob> pluginBlob;
    {
        ComPtr<slang::ISession> sourceSession;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, sourceSession.writeRef()) == SLANG_OK);

        auto compileAndSerialize =
            [&](const char* moduleName, const char* source, ComPtr<slang::IBlob>& outBlob)
        {
            ComPtr<slang::IBlob> diagnostics;
            ComPtr<slang::IModule> module(sourceSession->loadModuleFromSourceString(
                moduleName,
                moduleName,
                source,
                diagnostics.writeRef()));
            if (!module && diagnostics)
                fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
            SLANG_CHECK_ABORT(module != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(outBlob.writeRef())));
        };

        compileAndSerialize("structural_serialized_reflection_context", contextSource, contextBlob);
        compileAndSerialize("structural_serialized_reflection_schema", schemaSource, schemaBlob);
        compileAndSerialize("structural_serialized_reflection_plugin", pluginSource, pluginBlob);
    }

    ComPtr<slang::ISession> loadedSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, loadedSession.writeRef()) == SLANG_OK);

    auto loadSerialized = [&](const char* moduleName, ISlangBlob* blob) -> ComPtr<slang::IModule>
    {
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(slang_loadModuleFromIRBlob(
            loadedSession,
            moduleName,
            moduleName,
            blob->getBufferPointer(),
            blob->getBufferSize(),
            diagnostics.writeRef()));
        if (!module && diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        return module;
    };

    auto contextModule = loadSerialized("structural_serialized_reflection_context", contextBlob);
    SLANG_CHECK_ABORT(contextModule != nullptr);
    auto schemaModule = loadSerialized("structural_serialized_reflection_schema", schemaBlob);
    SLANG_CHECK_ABORT(schemaModule != nullptr);
    auto pluginModule = loadSerialized("structural_serialized_reflection_plugin", pluginBlob);
    SLANG_CHECK_ABORT(pluginModule != nullptr);

    // A generic declaration is not an automatic member of an open section because it denotes an
    // unbounded family. The host selects one finite specialization with an ordinary semantic
    // TypeConformance component. Creating that component in this fresh session must retain the
    // `uint` argument and register the exact type; declaration-key recovery deliberately does not
    // reconstruct generic arguments from serialized IR.
    auto pluginLayout = pluginModule->getLayout(0, nullptr);
    auto contextLayout = contextModule->getLayout(0, nullptr);
    SLANG_CHECK_ABORT(pluginLayout && contextLayout);
    auto genericHitGroupType = pluginLayout->findTypeByName("GenericHitGroup<uint>");
    auto hitTagType = contextLayout->findTypeByName("IHitTag");
    SLANG_CHECK_ABORT(genericHitGroupType && hitTagType);
    ComPtr<slang::ITypeConformance> genericHitGroupConformance;
    ComPtr<slang::IBlob> conformanceDiagnostics;
    auto conformanceResult = loadedSession->createTypeConformanceComponentType(
        genericHitGroupType,
        hitTagType,
        genericHitGroupConformance.writeRef(),
        -1,
        conformanceDiagnostics.writeRef());
    if (SLANG_FAILED(conformanceResult) && conformanceDiagnostics)
        fprintf(stderr, "%s\n", (const char*)conformanceDiagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(conformanceResult));

    slang::IComponentType* components[] = {schemaModule, pluginModule, genericHitGroupConformance};
    ComPtr<slang::IComponentType> program;
    ComPtr<slang::IBlob> diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadedSession->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef())));
    auto layout = program->getLayout(0, diagnostics.writeRef());
    if (!layout && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(layout != nullptr);

    // Reflection must recover the exact checked type selected by the linked identity. Parsing the
    // source name here would be incorrect because two modules can declare the same qualified name
    // and generic specializations do not have a source spelling that encodes their identity.
    auto schema = layout->findTraceProgramSchema("Schema");
    SLANG_CHECK_ABORT(schema != nullptr);
    SLANG_CHECK(schema->isHitGroupSectionOpen());
    SLANG_CHECK(schema->getPayloadCount() == 1);
    auto payload = schema->getPayload(0);
    SLANG_CHECK_ABORT(payload != nullptr);
    SLANG_CHECK(payload->getHitGroupCount() == 2);
    bool foundLinkedGroup = false;
    bool foundGenericGroup = false;
    for (SlangUInt i = 0; i < payload->getHitGroupCount(); ++i)
    {
        auto group = payload->getHitGroup(i);
        SLANG_CHECK_ABORT(group != nullptr);
        SLANG_CHECK(group->isLinked());
        SLANG_CHECK(group->getRecordTypeLayout() != nullptr);
        foundLinkedGroup |= UnownedStringSlice(group->getType()->getName()) == "LinkedHitGroup";
        foundGenericGroup |= group->getType() == genericHitGroupType;
    }
    SLANG_CHECK(foundLinkedGroup);
    SLANG_CHECK(foundGenericGroup);
}

SLANG_UNIT_TEST(structuralRayTracingEntryCatalogueWithoutSchema)
{
    // This module deliberately declares structural stages without declaring an
    // `ITraceProgramSchema`. `CatalogueHitGroup` also reaches `IHitGroup` through two tag
    // interfaces. Lowering records every matching tag on the one conformance, but the public
    // declaration catalogue must contain the semantic hit-group type exactly once.
    const char* source = R"(
        module structural_entry_catalogue_no_schema;

        import slang.raytracing;

        public struct CataloguePayload { uint value; }
        public struct CatalogueHitRecord { uint hitValue; }
        public struct CatalogueMissRecord { uint missValue; }
        public struct CatalogueCallableRecord { uint callableValue; }
        public struct CatalogueCallableData { uint value; }

        public struct CatalogueTraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        public struct CatalogueHitContext : rt::IHitContext
        {
            typealias TraceContext = CatalogueTraceContext;
            typealias Payload = CataloguePayload;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = CatalogueHitRecord;
        }

        public struct CatalogueMissContext : rt::IPayloadContext
        {
            typealias TraceContext = CatalogueTraceContext;
            typealias Payload = CataloguePayload;
            typealias Record = CatalogueMissRecord;
        }

        public struct CatalogueCallableContext : rt::ICallableContext
        {
            typealias TraceContext = CatalogueTraceContext;
            typealias CallableData = CatalogueCallableData;
            typealias Record = CatalogueCallableRecord;
        }

        [noinline]
        uint catalogueUnusedHitSentinel() { return 0x10203040u; }

        [noinline]
        uint catalogueUnusedMissSentinel() { return 0x50607080u; }

        [noinline]
        uint catalogueUnusedCallableSentinel() { return 0x90a0b0c0u; }

        public struct CatalogueClosestHit : rt::IClosestHitShader
        {
            typealias Context = CatalogueHitContext;
            void invoke(rt::ClosestHitInput<Context> input)
            {
                input.payload.value = catalogueUnusedHitSentinel();
            }
        }

        public interface ICatalogueHitTag : rt::IHitGroup {}
        public interface IDerivedCatalogueHitTag : ICatalogueHitTag {}

        public struct CatalogueHitGroup : IDerivedCatalogueHitTag
        {
            typealias Context = CatalogueHitContext;
            typealias ClosestHit = CatalogueClosestHit;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        public struct CatalogueMiss : rt::IMissShader
        {
            typealias Context = CatalogueMissContext;
            void invoke(rt::MissInput<Context> input)
            {
                input.payload.value = catalogueUnusedMissSentinel();
            }
        }

        public struct CatalogueCallable : rt::ICallableShader
        {
            typealias Context = CatalogueCallableContext;
            void invoke(rt::CallableInput<Context> input)
            {
                input.data.value = catalogueUnusedCallableSentinel();
            }
        }

        RWStructuredBuffer<uint> output;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main() { output[0] = 1; }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structural_entry_catalogue_no_schema",
        "structural-entry-catalogue-no-schema.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composite.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> program;
    auto linkResult = composite->link(program.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(linkResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(linkResult));

    auto layout = program->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout != nullptr);

    // All workers begin their first query together. Before cache publication was serialized, two
    // workers could both construct different cache owners; the last assignment released the first
    // owner while its worker still held raw entry pointers. Every worker must instead observe the
    // one stable catalogue and its three independently owned entry objects.
    static const int kThreadCount = 8;
    std::atomic<int> readyThreadCount(0);
    std::atomic<bool> startQueries(false);
    SlangUInt hitGroupCounts[kThreadCount] = {};
    SlangUInt missShaderCounts[kThreadCount] = {};
    SlangUInt callableShaderCounts[kThreadCount] = {};
    slang::RayTracingHitGroupReflection* hitGroups[kThreadCount] = {};
    slang::RayTracingMissShaderReflection* missShaders[kThreadCount] = {};
    slang::RayTracingCallableShaderReflection* callableShaders[kThreadCount] = {};
    std::thread queryThreads[kThreadCount];
    for (int i = 0; i < kThreadCount; ++i)
    {
        queryThreads[i] = std::thread(
            [&, i]
            {
                readyThreadCount.fetch_add(1, std::memory_order_release);
                while (!startQueries.load(std::memory_order_acquire))
                    std::this_thread::yield();
                // Capture the very first cache owner's raw entry pointer before any subsequent
                // query can converge on whichever unsynchronized publication happened last.
                hitGroups[i] = layout->getStructuralRayTracingHitGroup(0);
                hitGroupCounts[i] = layout->getStructuralRayTracingHitGroupCount();
                missShaderCounts[i] = layout->getStructuralRayTracingMissShaderCount();
                callableShaderCounts[i] = layout->getStructuralRayTracingCallableShaderCount();
                missShaders[i] = layout->getStructuralRayTracingMissShader(0);
                callableShaders[i] = layout->getStructuralRayTracingCallableShader(0);
            });
    }
    while (readyThreadCount.load(std::memory_order_acquire) != kThreadCount)
        std::this_thread::yield();
    startQueries.store(true, std::memory_order_release);
    for (auto& thread : queryThreads)
        thread.join();

    for (int i = 0; i < kThreadCount; ++i)
    {
        SLANG_CHECK(hitGroupCounts[i] == 1);
        SLANG_CHECK(missShaderCounts[i] == 1);
        SLANG_CHECK(callableShaderCounts[i] == 1);
        SLANG_CHECK(hitGroups[i] == hitGroups[0]);
        SLANG_CHECK(missShaders[i] == missShaders[0]);
        SLANG_CHECK(callableShaders[i] == callableShaders[0]);
    }

    auto hitGroup = hitGroups[0];
    auto missShader = missShaders[0];
    auto callableShader = callableShaders[0];
    SLANG_CHECK_ABORT(hitGroup && missShader && callableShader);
    SLANG_CHECK(layout->getStructuralRayTracingHitGroup(1) == nullptr);
    SLANG_CHECK(layout->getStructuralRayTracingMissShader(1) == nullptr);
    SLANG_CHECK(layout->getStructuralRayTracingCallableShader(1) == nullptr);

    SLANG_CHECK(UnownedStringSlice(hitGroup->getType()->getName()) == "CatalogueHitGroup");
    SLANG_CHECK(UnownedStringSlice(hitGroup->getContextType()->getName()) == "CatalogueHitContext");
    SLANG_CHECK(UnownedStringSlice(hitGroup->getRecordType()->getName()) == "CatalogueHitRecord");
    SLANG_CHECK(hitGroup->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(hitGroup->getFunctionIndex() == -1);
    SLANG_CHECK(!hitGroup->isLinked());
    SLANG_CHECK(hitGroup->getClosestHit() != nullptr);
    SLANG_CHECK(hitGroup->getAnyHit() == nullptr);
    SLANG_CHECK(hitGroup->getIntersection() == nullptr);
    SLANG_CHECK(hitGroup->getClosestHitEntryPointName() == nullptr);
    SLANG_CHECK(hitGroup->getClosestHit()->getEntryPointName() == nullptr);

    SLANG_CHECK(UnownedStringSlice(missShader->getType()->getName()) == "CatalogueMiss");
    SLANG_CHECK(
        UnownedStringSlice(missShader->getContextType()->getName()) == "CatalogueMissContext");
    SLANG_CHECK(
        UnownedStringSlice(missShader->getRecordType()->getName()) == "CatalogueMissRecord");
    SLANG_CHECK(missShader->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(missShader->getFunctionIndex() == -1);
    SLANG_CHECK(!missShader->isLinked());
    SLANG_CHECK(missShader->getMiss() != nullptr);
    SLANG_CHECK(missShader->getMiss()->getEntryPointName() == nullptr);

    SLANG_CHECK(UnownedStringSlice(callableShader->getType()->getName()) == "CatalogueCallable");
    SLANG_CHECK(
        UnownedStringSlice(callableShader->getContextType()->getName()) ==
        "CatalogueCallableContext");
    SLANG_CHECK(
        UnownedStringSlice(callableShader->getRecordType()->getName()) ==
        "CatalogueCallableRecord");
    SLANG_CHECK(
        UnownedStringSlice(callableShader->getDataType()->getName()) == "CatalogueCallableData");
    SLANG_CHECK(callableShader->getRecordTypeLayout() != nullptr);
    SLANG_CHECK(callableShader->getFunctionIndex() == -1);
    SLANG_CHECK(!callableShader->isLinked());
    SLANG_CHECK(callableShader->getCallable() != nullptr);
    SLANG_CHECK(callableShader->getCallable()->getEntryPointName() == nullptr);

    // The catalogue owns stable cached objects. Repeating a count or get query must not rebuild
    // the catalogue, and no later schema query can mutate these declaration-only entries.
    SLANG_CHECK(layout->getStructuralRayTracingHitGroup(0) == hitGroup);
    SLANG_CHECK(layout->getStructuralRayTracingMissShader(0) == missShader);
    SLANG_CHECK(layout->getStructuralRayTracingCallableShader(0) == callableShader);

    // Catalogue discovery reads the pre-DCE conformance index but does not add a keep-alive root.
    // Only the unrelated compute entry point should reach target emission.
    ComPtr<slang::IBlob> generatedCode;
    auto codeResult =
        program->getEntryPointCode(0, 0, generatedCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(codeResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult));
    SLANG_CHECK_ABORT(generatedCode != nullptr);
    UnownedStringSlice code(
        (const char*)generatedCode->getBufferPointer(),
        (const char*)generatedCode->getBufferPointer() + generatedCode->getBufferSize());
    SLANG_CHECK(code.indexOf(toSlice("catalogueUnusedHitSentinel")) == -1);
    SLANG_CHECK(code.indexOf(toSlice("catalogueUnusedMissSentinel")) == -1);
    SLANG_CHECK(code.indexOf(toSlice("catalogueUnusedCallableSentinel")) == -1);

    // A fresh session has no source-time reflection-type registry entries for this module. The
    // serialized catalogue must recover exported nominal types through the producer's declaration
    // lookup key and then verify their canonical identities before returning them.
    ComPtr<slang::IBlob> serializedModule;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(serializedModule.writeRef())));
    ComPtr<slang::ISession> loadedSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, loadedSession.writeRef()) == SLANG_OK);
    ComPtr<slang::IModule> loadedModule(slang_loadModuleFromIRBlob(
        loadedSession,
        "structural_entry_catalogue_no_schema",
        "structural-entry-catalogue-no-schema.slang-module",
        serializedModule->getBufferPointer(),
        serializedModule->getBufferSize(),
        diagnostics.writeRef()));
    if (!loadedModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(loadedModule != nullptr);
    auto loadedLayout = loadedModule->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(loadedLayout != nullptr);
    SLANG_CHECK(loadedLayout->getStructuralRayTracingHitGroupCount() == 1);
    SLANG_CHECK(loadedLayout->getStructuralRayTracingMissShaderCount() == 1);
    SLANG_CHECK(loadedLayout->getStructuralRayTracingCallableShaderCount() == 1);
    SLANG_CHECK(
        UnownedStringSlice(
            loadedLayout->getStructuralRayTracingHitGroup(0)->getType()->getName()) ==
        "CatalogueHitGroup");
    SLANG_CHECK(
        UnownedStringSlice(
            loadedLayout->getStructuralRayTracingMissShader(0)->getType()->getName()) ==
        "CatalogueMiss");
    SLANG_CHECK(
        UnownedStringSlice(
            loadedLayout->getStructuralRayTracingCallableShader(0)->getType()->getName()) ==
        "CatalogueCallable");
}

SLANG_UNIT_TEST(structuralRayTracingEntryCatalogueIsSchemaIndependent)
{
    // The two schemas below intentionally select disjoint trace contexts and callable-data types.
    // The catalogue describes both declaration families without applying either schema's
    // cross-entry restrictions. Each schema query then constructs its own indexed objects.
    const char* source = R"(
        import slang.raytracing;

        struct PayloadA { uint value; }
        struct PayloadB { float value; }
        struct HitRecordA { uint value; }
        struct HitRecordB { float2 value; }
        struct MissRecordA { uint value; }
        struct MissRecordB { float value; }
        struct CallableRecordA { uint value; }
        struct CallableRecordB { float4 value; }
        struct CallableDataA { uint value; }
        struct CallableDataB { float2 value; }

        struct TraceContextA : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct TraceContextB : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct HitContextA : rt::IHitContext
        {
            typealias TraceContext = TraceContextA;
            typealias Payload = PayloadA;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordA;
        }

        struct HitContextB : rt::IHitContext
        {
            typealias TraceContext = TraceContextB;
            typealias Payload = PayloadB;
            typealias Primitive = rt::TrianglePrimitive;
            typealias Record = HitRecordB;
        }

        struct MissContextA : rt::IPayloadContext
        {
            typealias TraceContext = TraceContextA;
            typealias Payload = PayloadA;
            typealias Record = MissRecordA;
        }

        struct MissContextB : rt::IPayloadContext
        {
            typealias TraceContext = TraceContextB;
            typealias Payload = PayloadB;
            typealias Record = MissRecordB;
        }

        struct CallableContextA : rt::ICallableContext
        {
            typealias TraceContext = TraceContextA;
            typealias CallableData = CallableDataA;
            typealias Record = CallableRecordA;
        }

        struct CallableContextB : rt::ICallableContext
        {
            typealias TraceContext = TraceContextB;
            typealias CallableData = CallableDataB;
            typealias Record = CallableRecordB;
        }

        struct ClosestHitA : rt::IClosestHitShader
        {
            typealias Context = HitContextA;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct ClosestHitB : rt::IClosestHitShader
        {
            typealias Context = HitContextB;
            void invoke(rt::ClosestHitInput<Context> input) {}
        }

        struct HitGroupB : rt::IHitGroup
        {
            typealias Context = HitContextB;
            typealias ClosestHit = ClosestHitB;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct HitGroupA : rt::IHitGroup
        {
            typealias Context = HitContextA;
            typealias ClosestHit = ClosestHitA;
            typealias AnyHit = rt::NoAnyHit<Context>;
            typealias Intersection = rt::NoIntersection<Context>;
        }

        struct MissB : rt::IMissShader
        {
            typealias Context = MissContextB;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct MissA : rt::IMissShader
        {
            typealias Context = MissContextA;
            void invoke(rt::MissInput<Context> input) {}
        }

        struct CallableB : rt::ICallableShader
        {
            typealias Context = CallableContextB;
            void invoke(rt::CallableInput<Context> input) {}
        }

        struct CallableA : rt::ICallableShader
        {
            typealias Context = CallableContextA;
            void invoke(rt::CallableInput<Context> input) {}
        }

        struct SchemaA : rt::ITraceProgramSchema
        {
            typealias TraceContext = TraceContextA;
            typealias HitGroups = rt::HitGroupList<HitGroupA>;
            typealias MissShaders = rt::MissShaderList<MissA>;
            typealias CallableShaders = rt::CallableShaderList<CallableA>;
        }

        struct SchemaB : rt::ITraceProgramSchema
        {
            typealias TraceContext = TraceContextB;
            typealias HitGroups = rt::HitGroupList<HitGroupB>;
            typealias MissShaders = rt::MissShaderList<MissB>;
            typealias CallableShaders = rt::CallableShaderList<CallableB>;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralEntryCatalogueSchemas",
        "structural-entry-catalogue-schemas.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto layout = module->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout != nullptr);
    SLANG_CHECK(layout->getStructuralRayTracingHitGroupCount() == 2);
    SLANG_CHECK(layout->getStructuralRayTracingMissShaderCount() == 2);
    SLANG_CHECK(layout->getStructuralRayTracingCallableShaderCount() == 2);

    // Canonical type identity gives a deterministic order independent of module and declaration
    // enumeration. Each B declaration appears before A in the source, but these same-length names
    // have canonical identity order A, B.
    auto catalogueHitA = layout->getStructuralRayTracingHitGroup(0);
    auto catalogueHitB = layout->getStructuralRayTracingHitGroup(1);
    auto catalogueMissA = layout->getStructuralRayTracingMissShader(0);
    auto catalogueMissB = layout->getStructuralRayTracingMissShader(1);
    auto catalogueCallableA = layout->getStructuralRayTracingCallableShader(0);
    auto catalogueCallableB = layout->getStructuralRayTracingCallableShader(1);
    SLANG_CHECK_ABORT(
        catalogueHitA && catalogueHitB && catalogueMissA && catalogueMissB && catalogueCallableA &&
        catalogueCallableB);
    SLANG_CHECK(UnownedStringSlice(catalogueHitA->getType()->getName()) == "HitGroupA");
    SLANG_CHECK(UnownedStringSlice(catalogueHitB->getType()->getName()) == "HitGroupB");
    SLANG_CHECK(UnownedStringSlice(catalogueMissA->getType()->getName()) == "MissA");
    SLANG_CHECK(UnownedStringSlice(catalogueMissB->getType()->getName()) == "MissB");
    SLANG_CHECK(UnownedStringSlice(catalogueCallableA->getType()->getName()) == "CallableA");
    SLANG_CHECK(UnownedStringSlice(catalogueCallableB->getType()->getName()) == "CallableB");
    SLANG_CHECK(catalogueHitA->getClosestHit()->getEntryPointName() != nullptr);
    SLANG_CHECK(catalogueMissA->getMiss()->getEntryPointName() != nullptr);
    SLANG_CHECK(catalogueCallableA->getCallable()->getEntryPointName() != nullptr);

    // A declaration catalogue is allowed to contain unrelated contexts and callable signatures.
    // Those compatibility checks belong to whichever concrete schema selects an entry.
    SLANG_CHECK(UnownedStringSlice(catalogueHitA->getContextType()->getName()) == "HitContextA");
    SLANG_CHECK(UnownedStringSlice(catalogueHitB->getContextType()->getName()) == "HitContextB");
    SLANG_CHECK(
        UnownedStringSlice(catalogueCallableA->getDataType()->getName()) == "CallableDataA");
    SLANG_CHECK(
        UnownedStringSlice(catalogueCallableB->getDataType()->getName()) == "CallableDataB");
    SLANG_CHECK(catalogueHitA->getFunctionIndex() == -1);
    SLANG_CHECK(catalogueHitB->getFunctionIndex() == -1);
    SLANG_CHECK(!catalogueHitA->isLinked());
    SLANG_CHECK(!catalogueHitB->isLinked());

    auto schemaA = layout->findTraceProgramSchema("SchemaA");
    auto schemaB = layout->findTraceProgramSchema("SchemaB");
    SLANG_CHECK_ABORT(schemaA && schemaB);
    SLANG_CHECK(schemaA->getPayloadCount() == 1);
    SLANG_CHECK(schemaB->getPayloadCount() == 1);
    auto schemaHitA = schemaA->getPayload(0)->getHitGroup(0);
    auto schemaHitB = schemaB->getPayload(0)->getHitGroup(0);
    auto schemaMissA = schemaA->getPayload(0)->getMissShader(0);
    auto schemaMissB = schemaB->getPayload(0)->getMissShader(0);
    auto schemaCallableA = schemaA->getCallableShader(0);
    auto schemaCallableB = schemaB->getCallableShader(0);
    SLANG_CHECK_ABORT(
        schemaHitA && schemaHitB && schemaMissA && schemaMissB && schemaCallableA &&
        schemaCallableB);

    // Schema entries have independent ownership and can safely acquire dense indices. Querying
    // both schemas must not mutate the previously cached declaration catalogue.
    SLANG_CHECK(schemaHitA != catalogueHitA);
    SLANG_CHECK(schemaHitB != catalogueHitB);
    SLANG_CHECK(schemaMissA != catalogueMissA);
    SLANG_CHECK(schemaMissB != catalogueMissB);
    SLANG_CHECK(schemaCallableA != catalogueCallableA);
    SLANG_CHECK(schemaCallableB != catalogueCallableB);
    SLANG_CHECK(schemaHitA->getFunctionIndex() == 0);
    SLANG_CHECK(schemaHitB->getFunctionIndex() == 0);
    SLANG_CHECK(schemaMissA->getFunctionIndex() == 0);
    SLANG_CHECK(schemaMissB->getFunctionIndex() == 0);
    SLANG_CHECK(schemaCallableA->getFunctionIndex() == 0);
    SLANG_CHECK(schemaCallableB->getFunctionIndex() == 0);
    SLANG_CHECK(catalogueHitA->getFunctionIndex() == -1);
    SLANG_CHECK(catalogueMissA->getFunctionIndex() == -1);
    SLANG_CHECK(catalogueCallableA->getFunctionIndex() == -1);
}

SLANG_UNIT_TEST(structuralRayTracingD3DRecordBindingReflection)
{
    const char* source = R"(
        import slang.raytracing;

        struct Payload { uint value; }
        struct Record
        {
            float3 direction;
            float weight;
            float2 samples[2];
            uint tail;
        }
        struct SecondRecord { uint2 value; }

        // Use the same type in a handwritten cbuffer so reflection can prove the synthesized
        // structural Record follows the ordinary HLSL constant-buffer packing rules.
        ConstantBuffer<Record> existingRecord : register(b0);
        RWStructuredBuffer<uint> output;

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct RecordContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = ::Record;
        }

        struct RecordMiss : rt::IMissShader
        {
            typealias Context = RecordContext;
            void invoke(rt::MissInput<Context> input)
            {
                output[0] = input.record.tail + existingRecord.tail;
            }
        }

        struct SecondRecordContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = ::SecondRecord;
        }

        // Qualified structural stage names have a separate source lookup identity and target-safe
        // physical export name. Keep this stage qualified so the combined-program test below
        // verifies that parameter layout preserves the same physical identity as IR linking.
        namespace Stages
        {
            struct SecondRecordMiss : rt::IMissShader
            {
                typealias Context = ::SecondRecordContext;
                void invoke(rt::MissInput<::SecondRecordContext> input)
                {
                    output[1] = input.record.value.x;
                }
            }
        }

        struct VoidContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = void;
        }

        struct VoidMiss : rt::IMissShader
        {
            typealias Context = VoidContext;
            void invoke(rt::MissInput<Context> input) {}
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_6_6");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "structuralD3DRecordBinding",
        "structural-d3d-record-binding.slang",
        source,
        diagnostics.writeRef()));
    if (!module && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(module != nullptr);

    auto checkLinkedStageLayout = [&](const char* name,
                                      SlangInt expectedBindingIndex,
                                      SlangInt expectedBindingSpace,
                                      bool expectVoidRecord)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
            name,
            SLANG_STAGE_MISS,
            entryPoint.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IComponentType> linkedEntryPoint;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(entryPoint->link(linkedEntryPoint.writeRef(), diagnostics.writeRef())));
        auto layout = linkedEntryPoint->getLayout(0, diagnostics.writeRef());
        SLANG_CHECK_ABORT(layout != nullptr);

        auto reflectedEntryPoint = layout->getEntryPointByIndex(0);
        SLANG_CHECK_ABORT(reflectedEntryPoint != nullptr);
        SLANG_CHECK(
            reflectedEntryPoint->getStructuralRayTracingRecordBindingIndex() ==
            expectedBindingIndex);
        SLANG_CHECK(
            reflectedEntryPoint->getStructuralRayTracingRecordBindingSpace() ==
            expectedBindingSpace);
        auto recordType = reflectedEntryPoint->getStructuralRayTracingRecordType();
        auto recordTypeLayout = reflectedEntryPoint->getStructuralRayTracingRecordTypeLayout();
        SLANG_CHECK_ABORT(recordType != nullptr);
        SLANG_CHECK_ABORT(recordTypeLayout != nullptr);
        SLANG_CHECK(
            (recordType->getScalarType() == slang::TypeReflection::ScalarType::Void) ==
            expectVoidRecord);
        if (!expectVoidRecord)
        {
            slang::VariableLayoutReflection* existingRecord = nullptr;
            for (SlangUInt i = 0; i < layout->getParameterCount(); ++i)
            {
                auto parameter = layout->getParameterByIndex(i);
                if (parameter && parameter->getName() &&
                    strcmp(parameter->getName(), "existingRecord") == 0)
                {
                    existingRecord = parameter;
                    break;
                }
            }
            SLANG_CHECK_ABORT(existingRecord != nullptr);
            auto handwrittenRecordLayout = existingRecord->getTypeLayout()->getElementTypeLayout();
            SLANG_CHECK_ABORT(handwrittenRecordLayout != nullptr);

            // `samples` has a 16-byte element stride in an HLSL cbuffer but an 8-byte stride in a
            // structured buffer, so comparing the aggregate and field layouts detects an
            // accidental use of the wrong Record ABI rules.
            SLANG_CHECK(recordTypeLayout->getSize() == handwrittenRecordLayout->getSize());
            SLANG_CHECK(
                recordTypeLayout->getAlignment() == handwrittenRecordLayout->getAlignment());
            SLANG_CHECK(
                recordTypeLayout->getFieldCount() == handwrittenRecordLayout->getFieldCount());
            for (unsigned i = 0; i < recordTypeLayout->getFieldCount(); ++i)
            {
                auto reflectedField = recordTypeLayout->getFieldByIndex(i);
                auto handwrittenField = handwrittenRecordLayout->getFieldByIndex(i);
                SLANG_CHECK_ABORT(reflectedField != nullptr && handwrittenField != nullptr);
                SLANG_CHECK(reflectedField->getOffset() == handwrittenField->getOffset());
                SLANG_CHECK(
                    reflectedField->getTypeLayout()->getSize() ==
                    handwrittenField->getTypeLayout()->getSize());
                SLANG_CHECK(
                    reflectedField->getTypeLayout()->getStride() ==
                    handwrittenField->getTypeLayout()->getStride());
            }

            auto samplesIndex = recordTypeLayout->findFieldIndexByName("samples");
            SLANG_CHECK_ABORT(samplesIndex >= 0);
            auto samplesLayout =
                recordTypeLayout->getFieldByIndex(unsigned(samplesIndex))->getTypeLayout();
            SLANG_CHECK(samplesLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM) == 16);
        }
    };

    // b0 is explicitly occupied by `existingRecord`. The structural record binding receives b0
    // in a fresh compiler-owned space rather than sharing the user's global resource namespace.
    checkLinkedStageLayout("RecordMiss", 0, 1, false);

    // A combined target library gives every selected structural entry point a distinct register.
    // Rename one component before composition to verify that IR lowering matches the final export
    // name rather than assuming the source declaration name survived component composition.
    ComPtr<slang::IEntryPoint> firstSelectedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "RecordMiss",
        SLANG_STAGE_MISS,
        firstSelectedEntryPoint.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> renamedFirstEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(firstSelectedEntryPoint->renameEntryPoint(
        "RenamedRecordMiss",
        renamedFirstEntryPoint.writeRef())));

    ComPtr<slang::IEntryPoint> secondSelectedEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findAndCheckEntryPoint(
        "Stages.SecondRecordMiss",
        SLANG_STAGE_MISS,
        secondSelectedEntryPoint.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* selectedComponents[] = {
        renamedFirstEntryPoint,
        secondSelectedEntryPoint,
    };
    ComPtr<slang::IComponentType> selectedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        selectedComponents,
        SLANG_COUNT_OF(selectedComponents),
        selectedProgram.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedSelectedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        selectedProgram->link(linkedSelectedProgram.writeRef(), diagnostics.writeRef())));

    auto selectedLayout = linkedSelectedProgram->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(selectedLayout != nullptr);
    SLANG_CHECK(selectedLayout->getEntryPointCount() == 2);
    auto renamedEntryPointLayout = selectedLayout->getEntryPointByIndex(0);
    auto qualifiedEntryPointLayout = selectedLayout->getEntryPointByIndex(1);
    SLANG_CHECK_ABORT(renamedEntryPointLayout != nullptr && qualifiedEntryPointLayout != nullptr);
    SLANG_CHECK(strcmp(renamedEntryPointLayout->getNameOverride(), "RenamedRecordMiss") == 0);
    SLANG_CHECK(renamedEntryPointLayout->getStructuralRayTracingRecordBindingIndex() == 0);
    SLANG_CHECK(renamedEntryPointLayout->getStructuralRayTracingRecordBindingSpace() == 1);
    SLANG_CHECK(strcmp(qualifiedEntryPointLayout->getName(), "Stages.SecondRecordMiss") == 0);
    SLANG_CHECK(
        strcmp(
            qualifiedEntryPointLayout->getNameOverride(),
            qualifiedEntryPointLayout->getName()) != 0);
    SLANG_CHECK(qualifiedEntryPointLayout->getStructuralRayTracingRecordBindingIndex() == 1);
    SLANG_CHECK(qualifiedEntryPointLayout->getStructuralRayTracingRecordBindingSpace() == 1);

    ComPtr<slang::IBlob> selectedCode;
    auto selectedCodeResult =
        linkedSelectedProgram->getTargetCode(0, selectedCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(selectedCodeResult) && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(selectedCodeResult));
    SLANG_CHECK_ABORT(selectedCode != nullptr);
    UnownedStringSlice selectedCodeText(
        (const char*)selectedCode->getBufferPointer(),
        (const char*)selectedCode->getBufferPointer() + selectedCode->getBufferSize());
    auto firstBindingPosition = selectedCodeText.indexOf(toSlice("register(b0, space1)"));
    auto secondBindingPosition = selectedCodeText.indexOf(toSlice("register(b1, space1)"));
    auto firstRecordPosition = selectedCodeText.indexOf(toSlice("Record_0 record_"));
    auto secondRecordPosition = selectedCodeText.indexOf(toSlice("SecondRecord_0 record_"));
    SLANG_CHECK(firstBindingPosition >= 0);
    SLANG_CHECK(secondBindingPosition > firstBindingPosition);
    SLANG_CHECK(firstRecordPosition > firstBindingPosition);
    SLANG_CHECK(firstRecordPosition < secondBindingPosition);
    SLANG_CHECK(secondRecordPosition > secondBindingPosition);

    // A separately linked stage with Record = void keeps its reflected structural contract but
    // does not reserve a local cbuffer binding that it cannot use.
    checkLinkedStageLayout("VoidMiss", -1, -1, true);

    const char* recordOnlySource = R"(
        import slang.raytracing;

        struct Payload { uint value; }
        struct Record { uint value; }

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct RecordOnlyContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = ::Record;
        }

        struct RecordOnlyMiss : rt::IMissShader
        {
            typealias Context = RecordOnlyContext;
            void invoke(rt::MissInput<Context> input)
            {
                uint value = input.record.value;
            }
        }
    )";
    ComPtr<slang::IModule> recordOnlyModule(session->loadModuleFromSourceString(
        "structuralD3DRecordOnly",
        "structural-d3d-record-only.slang",
        recordOnlySource,
        diagnostics.writeRef()));
    if (!recordOnlyModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(recordOnlyModule != nullptr);

    ComPtr<slang::IEntryPoint> recordOnlyEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(recordOnlyModule->findAndCheckEntryPoint(
        "RecordOnlyMiss",
        SLANG_STAGE_MISS,
        recordOnlyEntryPoint.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedRecordOnlyEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        recordOnlyEntryPoint->link(linkedRecordOnlyEntryPoint.writeRef(), diagnostics.writeRef())));
    auto recordOnlyLayout = linkedRecordOnlyEntryPoint->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(recordOnlyLayout != nullptr);
    auto recordOnlyReflection = recordOnlyLayout->getEntryPointByIndex(0);
    SLANG_CHECK_ABORT(recordOnlyReflection != nullptr);
    SLANG_CHECK(recordOnlyReflection->getStructuralRayTracingRecordBindingIndex() == 0);
    SLANG_CHECK(recordOnlyReflection->getStructuralRayTracingRecordBindingSpace() == 0);

    // A record-only SM 6.6 component has no user resource that would otherwise claim space zero.
    // Reserving the hidden cbuffer must still mark that space before the bindless heap is assigned.
    SLANG_CHECK(recordOnlyLayout->getBindlessSpaceIndex() == 1);

    const char* wholeSpaceSource = R"(
        import slang.raytracing;

        struct Payload { uint value; }
        struct Record { uint value; }
        struct Globals { uint value; }

        // A ParameterBlock owns all of space0. The hidden local Record CBV must not be allocated
        // inside that space merely because per-register occupancy is tracked separately.
        ParameterBlock<Globals> globals : register(space0);
        RWStructuredBuffer<uint> output : register(u0, space2);

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct WholeSpaceContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = ::Record;
        }

        struct WholeSpaceMiss : rt::IMissShader
        {
            typealias Context = WholeSpaceContext;
            void invoke(rt::MissInput<Context> input)
            {
                output[0] = input.record.value + globals.value;
            }
        }
    )";
    ComPtr<slang::IModule> wholeSpaceModule(session->loadModuleFromSourceString(
        "structuralD3DWholeSpace",
        "structural-d3d-whole-space.slang",
        wholeSpaceSource,
        diagnostics.writeRef()));
    if (!wholeSpaceModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(wholeSpaceModule != nullptr);

    ComPtr<slang::IEntryPoint> wholeSpaceEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(wholeSpaceModule->findAndCheckEntryPoint(
        "WholeSpaceMiss",
        SLANG_STAGE_MISS,
        wholeSpaceEntryPoint.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linkedWholeSpaceEntryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        wholeSpaceEntryPoint->link(linkedWholeSpaceEntryPoint.writeRef(), diagnostics.writeRef())));
    auto wholeSpaceLayout = linkedWholeSpaceEntryPoint->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(wholeSpaceLayout != nullptr);
    auto wholeSpaceReflection = wholeSpaceLayout->getEntryPointByIndex(0);
    SLANG_CHECK_ABORT(wholeSpaceReflection != nullptr);
    SLANG_CHECK(wholeSpaceReflection->getStructuralRayTracingRecordBindingIndex() == 0);
    SLANG_CHECK(wholeSpaceReflection->getStructuralRayTracingRecordBindingSpace() == 1);

    const char* noRecordSource = R"(
        import slang.raytracing;

        struct Payload { uint value; }
        struct Record { uint value; }
        RWStructuredBuffer<uint> output;

        struct TraceContext : rt::ITraceContext
        {
            typealias AccelerationStructure = rt::AccelerationStructure;
            typealias Motion = rt::NoMotion;
        }

        struct VoidContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = void;
        }

        struct VoidOnlyMiss : rt::IMissShader
        {
            typealias Context = VoidContext;
            void invoke(rt::MissInput<Context> input) {}
        }

        // This declaration deliberately reads a non-void Record but is not selected as an entry
        // point and is unreachable from `ordinaryMain`. It must not reserve a hidden D3D binding
        // for either selected entry point below.
        struct UnusedRecordContext : rt::IPayloadContext
        {
            typealias TraceContext = ::TraceContext;
            typealias Payload = ::Payload;
            typealias Record = ::Record;
        }

        struct UnusedRecordMiss : rt::IMissShader
        {
            typealias Context = UnusedRecordContext;
            void invoke(rt::MissInput<Context> input)
            {
                output[0] = input.record.value;
            }
        }

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void ordinaryMain() {}

        // Merely selecting a ray-generation stage does not make unrelated structural declarations
        // reachable. Only a RayTracer operation may request adapters and their Record binding.
        [shader("raygeneration")]
        void unrelatedRaygen() {}
    )";
    ComPtr<slang::IModule> noRecordModule(session->loadModuleFromSourceString(
        "structuralD3DNoRecord",
        "structural-d3d-no-record.slang",
        noRecordSource,
        diagnostics.writeRef()));
    if (!noRecordModule && diagnostics)
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    SLANG_CHECK_ABORT(noRecordModule != nullptr);

    auto checkNoRecordBinding = [&](const char* name, SlangStage stage, bool expectStructuralVoid)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(noRecordModule->findAndCheckEntryPoint(
            name,
            stage,
            entryPoint.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IComponentType> linkedEntryPoint;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(entryPoint->link(linkedEntryPoint.writeRef(), diagnostics.writeRef())));
        auto layout = linkedEntryPoint->getLayout(0, diagnostics.writeRef());
        SLANG_CHECK_ABORT(layout != nullptr);

        auto reflectedEntryPoint = layout->getEntryPointByIndex(0);
        SLANG_CHECK_ABORT(reflectedEntryPoint != nullptr);
        SLANG_CHECK(reflectedEntryPoint->getStructuralRayTracingRecordBindingIndex() == -1);
        SLANG_CHECK(reflectedEntryPoint->getStructuralRayTracingRecordBindingSpace() == -1);
        auto recordType = reflectedEntryPoint->getStructuralRayTracingRecordType();
        auto recordTypeLayout = reflectedEntryPoint->getStructuralRayTracingRecordTypeLayout();
        if (expectStructuralVoid)
        {
            SLANG_CHECK_ABORT(recordType != nullptr);
            SLANG_CHECK_ABORT(recordTypeLayout != nullptr);
            SLANG_CHECK(recordType->getScalarType() == slang::TypeReflection::ScalarType::Void);
        }
        else
        {
            SLANG_CHECK(recordType == nullptr);
            SLANG_CHECK(recordTypeLayout == nullptr);
        }
    };

    // A structural stage with Record = void keeps its stage contract but needs no hidden cbuffer.
    checkNoRecordBinding("VoidOnlyMiss", SLANG_STAGE_MISS, true);

    // Ordinary entry points expose neither a structural Record contract nor its D3D binding.
    checkNoRecordBinding("ordinaryMain", SLANG_STAGE_COMPUTE, false);
    checkNoRecordBinding("unrelatedRaygen", SLANG_STAGE_RAY_GENERATION, false);
}
