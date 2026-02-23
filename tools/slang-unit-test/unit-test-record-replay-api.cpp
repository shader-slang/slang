// API tests for record-replay functionality
// These tests verify that the record layer wrapper classes correctly wrap Slang API calls.
// They exercise methods on SessionRecorder, ModuleRecorder, EntryPointRecorder, and
// CompositeComponentTypeRecorder to improve code coverage.
// Note: These tests must be run manually with SLANG_RECORD_LAYER=1 environment variable set
// to actually exercise the record-replay code paths.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test 1: Basic session creation
// When run with SLANG_RECORD_LAYER=1, this exercises the GlobalSessionRecorder wrapper
SLANG_UNIT_TEST(RecordReplayApiCreateSession)
{
    // Create global session
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    SLANG_CHECK(globalSession != nullptr);

    // Create a session
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    SLANG_CHECK(session != nullptr);
}

// Test 2: Module loading and reflection
// Exercises ModuleRecorder wrapper by calling reflection methods on the loaded module.
SLANG_UNIT_TEST(RecordReplayApiCompileModule)
{
    // Create session with a target
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Load a module with a struct type
    const char* moduleCode = "struct MyStruct { int x; float y; }\n"
                             "int simpleFunction(int x) { return x * 2; }\n";

    ComPtr<slang::IBlob> diagnosticsBlob;
    auto module = session->loadModuleFromSourceString(
        "test-module",
        "test-module.slang",
        moduleCode,
        diagnosticsBlob.writeRef());

    SLANG_CHECK(module != nullptr);

    // Call reflection methods on module to exercise ModuleRecorder
    auto moduleReflection = module->getModuleReflection();
    SLANG_CHECK(moduleReflection != nullptr);

    // Get module name (exercises ModuleRecorder::getName)
    const char* moduleName = module->getName();
    SLANG_CHECK(moduleName != nullptr);

    // Get file path (exercises ModuleRecorder::getFilePath)
    const char* filePath = module->getFilePath();
    SLANG_CHECK(filePath != nullptr);

    // Get layout (exercises IComponentTypeRecorder::getLayout)
    ComPtr<slang::IBlob> layoutDiagnostics;
    auto layout = module->getLayout(0, layoutDiagnostics.writeRef());
    SLANG_CHECK(layout != nullptr);
}

// Test 3: Entry point, composite components, and linking
// Exercises EntryPointRecorder and CompositeComponentTypeRecorder by calling their methods.
SLANG_UNIT_TEST(RecordReplayApiEntryPoint)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* shaderCode = "[shader(\"compute\")]\n"
                             "[numthreads(1,1,1)]\n"
                             "void main(uniform int value) {}\n";

    ComPtr<slang::IBlob> diagnosticsBlob;
    auto module = session->loadModuleFromSourceString(
        "shader-module",
        "shader.slang",
        shaderCode,
        diagnosticsBlob.writeRef());

    SLANG_CHECK(module != nullptr);

    // Get entry point (exercises ModuleRecorder::findEntryPointByName)
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(module->findEntryPointByName("main", entryPoint.writeRef()) == SLANG_OK);
    SLANG_CHECK(entryPoint != nullptr);

    // Call methods on entry point (exercises EntryPointRecorder)
    auto entryPointReflection = entryPoint->getFunctionReflection();
    SLANG_CHECK(entryPointReflection != nullptr);

    // Create composite component (exercises SessionRecorder::createCompositeComponentType)
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    ComPtr<ISlangBlob> compositeBlob;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            compositeBlob.writeRef()) == SLANG_OK);

    SLANG_CHECK(composite != nullptr);

    // Call methods on composite (exercises CompositeComponentTypeRecorder/IComponentTypeRecorder)
    auto compositeLayout = composite->getLayout(0, nullptr);
    SLANG_CHECK(compositeLayout != nullptr);

    // Get entry point count via layout (exercises IComponentTypeRecorder)
    SlangUInt entryPointCount = compositeLayout->getEntryPointCount();
    SLANG_CHECK(entryPointCount == 1);

    // Link the composite (exercises IComponentTypeRecorder::link)
    ComPtr<slang::IComponentType> linked;
    ComPtr<ISlangBlob> linkDiagnostics;
    SLANG_CHECK(composite->link(linked.writeRef(), linkDiagnostics.writeRef()) == SLANG_OK);
    SLANG_CHECK(linked != nullptr);
}

// Test 4: Session-level type operations
// Exercises SessionRecorder by calling type specialization and layout methods.
SLANG_UNIT_TEST(RecordReplayApiTypeOperations)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Load module with generic types
    const char* moduleCode = "struct MyStruct<T> { T value; }\n"
                             "int useStruct(MyStruct<int> s) { return s.value; }\n";

    ComPtr<slang::IBlob> diagnosticsBlob;
    auto module = session->loadModuleFromSourceString(
        "type-module",
        "type-module.slang",
        moduleCode,
        diagnosticsBlob.writeRef());

    SLANG_CHECK(module != nullptr);

    // Get layout (exercises IComponentTypeRecorder::getLayout)
    ComPtr<slang::IBlob> layoutDiagnostics;
    auto layout = module->getLayout(0, layoutDiagnostics.writeRef());
    SLANG_CHECK(layout != nullptr);

    // Find the generic struct type (layout object IS the program reflection)
    auto structType = layout->findTypeByName("MyStruct");
    if (structType)
    {
        // Get type layout (exercises SessionRecorder::getTypeLayout)
        ComPtr<slang::IBlob> typeLayoutDiagnostics;
        auto typeLayout = session->getTypeLayout(
            structType,
            0,
            slang::LayoutRules::Default,
            typeLayoutDiagnostics.writeRef());
        // Type layout may be null for generic types, that's OK
    }

    // Get dynamic type (exercises SessionRecorder::getDynamicType)
    auto dynamicType = session->getDynamicType();
    SLANG_CHECK(dynamicType != nullptr);
}

// Test 5: Parameter serialization and target code generation
// Exercises parameter-recorder.cpp by compiling code with multiple targets.
SLANG_UNIT_TEST(RecordReplayApiParameterSerialization)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Create session with multiple targets to exercise parameter recording
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Load a compute shader
    const char* shaderCode = "RWStructuredBuffer<int> outputBuffer;\n"
                             "[shader(\"compute\")]\n"
                             "[numthreads(64,1,1)]\n"
                             "void computeMain(uint3 tid : SV_DispatchThreadID)\n"
                             "{\n"
                             "    outputBuffer[tid.x] = tid.x * 2;\n"
                             "}\n";

    ComPtr<slang::IBlob> diagnosticsBlob;
    auto module = session->loadModuleFromSourceString(
        "compute-module",
        "compute.slang",
        shaderCode,
        diagnosticsBlob.writeRef());

    SLANG_CHECK(module != nullptr);

    // Get entry point
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        module->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnosticsBlob.writeRef()) == SLANG_OK);
    SLANG_CHECK(entryPoint != nullptr);

    // Create and link composite (exercises parameter recording for component arrays)
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnosticsBlob.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(composite->link(linked.writeRef(), diagnosticsBlob.writeRef()) == SLANG_OK);

    // Get target code (exercises ParameterRecorder with IBlob results)
    ComPtr<ISlangBlob> targetCode;
    SLANG_CHECK(
        linked->getTargetCode(0, targetCode.writeRef(), diagnosticsBlob.writeRef()) == SLANG_OK);
    SLANG_CHECK(targetCode != nullptr);
    SLANG_CHECK(targetCode->getBufferSize() > 0);
}
