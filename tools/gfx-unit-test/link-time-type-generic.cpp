#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

// Test that generic link time types conforming to a generic interface with generic
// methods/subscript members work correctly.
// Also test that global generic link-time functions works correctly.

namespace gfx_test
{
static Slang::Result loadProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    slang::ProgramLayout*& slangReflection,
    bool linkSpecialization = false)
{
    const char* moduleInterfaceSrc = R"(
            interface ISimple { float getVal(); }
            interface IHasProperty { property float val2{get;set;} }
            interface IFoo<T:__BuiltinFloatingPointType> : IHasProperty
            {
                static const int offset;
                [mutating] void setValue(float v);

                T getValue<U:ISimple>(U u);

                __subscript<U:__BuiltinIntegerType>(U index) -> T { get; }
            }
            struct FooImpl<T:__BuiltinFloatingPointType, int x> : IFoo<T>
            {
                T val;
                static const int offset = x;
                [mutating] void setValue(float v) { val = T(v); }
                T getValue<U:ISimple>(U u){ return val + T(u.getVal()); }
                property float val2 {
                    get { return __real_cast<float>(val) + 2.0; }
                    set { val = T(newValue); }
                }
                __subscript<U:__BuiltinIntegerType>(U index) -> T { get {return T(1.0); } }
            };
            struct BarImpl<T:__BuiltinFloatingPointType, int x> : IFoo<T>
            {
                T val;
                static const int offset = -x;
                [mutating] void setValue(float v) { val = T(v); }
                T getValue<U:ISimple>(U u){ return val - T(1.0); }
                property float val2 {
                    get { return __real_cast<float>(val) + 2.0; }
                    set { val = T(newValue); }
                }
                __subscript<U:__BuiltinIntegerType>(U index) -> T { get {return T(2.0); } }
            };
        )";
    const char* module0Src = R"(
            import ifoo;
            extern struct Foo<T:__BuiltinFloatingPointType, int i> : IFoo<T> = FooImpl<T, i+1>;
            extern static const float c = 0.0;
            extern int linkTimeFunc<int x>() { return x; }
            struct SimpleImpl : ISimple
            {
                float getVal() { return 100.0; }
            };

            // Use an indirect generic function to retrieve val2, to make sure intermediate witness tables
            // can be obtained correctly from link-time witnesses.
            float getVal2<T:IHasProperty>(T t) { return t.val2; }

            [numthreads(1,1,1)]
            void computeMain(uniform RWStructuredBuffer<float> buffer)
            {
                Foo<float, 0> foo;
                foo.setValue(3.0);
                buffer[0] = foo.getValue(SimpleImpl()) + getVal2(foo) + Foo<float, 0>.offset + c + foo[0] + linkTimeFunc<0>();
            }
        )";
    const char* module1Src = R"(
            import ifoo;
            export struct Foo<T1:__BuiltinFloatingPointType, int i> : IFoo<T1> = BarImpl<T1, i+1>;
            export static const float c = 1.0;
            export int linkTimeFunc<int x>() { return x + 1; }
        )";
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    auto moduleInterfaceBlob =
        Slang::UnownedRawBlob::create(moduleInterfaceSrc, strlen(moduleInterfaceSrc));
    auto module0Blob = Slang::UnownedRawBlob::create(module0Src, strlen(module0Src));
    auto module1Blob = Slang::UnownedRawBlob::create(module1Src, strlen(module1Src));
    slang::IModule* moduleInterface =
        slangSession->loadModuleFromSource("ifoo", "ifoo.slang", moduleInterfaceBlob);
    slang::IModule* module0 = slangSession->loadModuleFromSource("module0", "path0", module0Blob);
    slang::IModule* module1 = slangSession->loadModuleFromSource("module1", "path1", module1Blob);
    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module0->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));

    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(moduleInterface);
    componentTypes.add(module0);
    if (linkSpecialization)
        componentTypes.add(module1);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    composedProgram = linkedProgram;
    slangReflection = composedProgram->getLayout();

    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createShaderProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

void linkTimeTypeGenericTestImpl(IDevice* device, UnitTestContext* context)
{
    // Create pipeline without linking a specialization override module, so we should
    // see the default value of `extern Foo`.
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadProgram(device, shaderProgram, slangReflection, false));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    // Create pipeline with a specialization override module linked in, so we should
    // see the result of using `BarImpl<T>` for `extern Foo<T>`.
    ComPtr<IShaderProgram> shaderProgram1;
    GFX_CHECK_CALL_ABORT(loadProgram(device, shaderProgram1, slangReflection, true));

    ComputePipelineDesc pipelineDesc1 = {};
    pipelineDesc1.program = shaderProgram1.get();
    ComPtr<IComputePipeline> pipelineState1;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc1, pipelineState1.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 0.0f, 0.0f, 0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    auto queue = device->getQueue(QueueType::Graphics);

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto commandEncoder = queue->createCommandEncoder();
        auto computePassEncoder = commandEncoder->beginComputePass();

        auto rootObject = computePassEncoder->bindPipeline(pipelineState);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        computePassEncoder->dispatchCompute(1, 1, 1);
        computePassEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{110.0f});

    // Now run again with the overrided program.
    {
        auto commandEncoder = queue->createCommandEncoder();
        auto computePassEncoder = commandEncoder->beginComputePass();

        auto rootObject = computePassEncoder->bindPipeline(pipelineState1);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        computePassEncoder->dispatchCompute(1, 1, 1);
        computePassEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{10.0f});
}

SLANG_UNIT_TEST(linkTimeTypeGenericD3D12)
{
    runTestImpl(linkTimeTypeGenericTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeTypeGenerictVulkan)
{
    runTestImpl(linkTimeTypeGenericTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
