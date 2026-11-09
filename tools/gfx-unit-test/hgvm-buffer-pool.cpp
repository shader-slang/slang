#include "slang-hgvm/slang-hgvm-buffer-pool.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
struct PoolOutput : HGVM::IHGVMOutputSink
{
    String text;
    void writeLine(std::string_view line) override
    {
        text.append(line.data(), Index(line.size()));
        text.append("\n");
    }
};

void testBufferPool(UnitTestContext* context, rhi::DeviceType api)
{
    auto apiFlag = api == rhi::DeviceType::CUDA ? RenderApiFlag::CUDA : RenderApiFlag::Vulkan;
    if (!(context->enabledApis & apiFlag))
    {
        SLANG_IGNORE_TEST;
    }
    slang::CompilerOptionEntry option = {};
    option.name = slang::CompilerOptionName::ExperimentalFeature;
    option.value.kind = slang::CompilerOptionValueKind::Int;
    option.value.intValue0 = 1;
    rhi::DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = api;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
    deviceDesc.debugCallback = context->debugCallback;
    deviceDesc.slang.compilerOptionEntries = &option;
    deviceDesc.slang.compilerOptionEntryCount = 1;
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);

    // Exercise the real cache with actual resources, including every compatibility field.
    HGVM::BufferPool pool(128);
    rhi::BufferDesc desc = {};
    desc.size = 64;
    desc.elementSize = 4;
    desc.usage = rhi::BufferUsage::ShaderResource | rhi::BufferUsage::UnorderedAccess |
                 rhi::BufferUsage::CopySource | rhi::BufferUsage::CopyDestination;
    desc.defaultState = rhi::ResourceState::UnorderedAccess;
    ComPtr<rhi::IBuffer> buffer = device->createBuffer(desc);
    SLANG_CHECK_ABORT(buffer);
    auto identity = buffer.get();
    pool.beginLease(desc.size);
    pool.recycle(desc, std::move(buffer));
    for (int field = 0; field < 6; ++field)
    {
        auto incompatible = desc;
        switch (field)
        {
        case 0:
            incompatible.size = 128;
            break;
        case 1:
            incompatible.elementSize = 8;
            break;
        case 2:
            incompatible.format = rhi::Format::R32Uint;
            break;
        case 3:
            incompatible.memoryType = rhi::MemoryType::Upload;
            break;
        case 4:
            incompatible.usage = rhi::BufferUsage::CopySource;
            break;
        case 5:
            incompatible.defaultState = rhi::ResourceState::CopySource;
            break;
        }
        SLANG_CHECK(!pool.acquire(incompatible));
    }
    desc.label = "Labels do not affect compatibility";
    buffer = pool.acquire(desc);
    SLANG_CHECK(buffer.get() == identity);
    SLANG_CHECK(pool.getStats().hits == 1);
    pool.beginLease(desc.size);
    pool.recycle(desc, std::move(buffer));
    // Three differently sized idle allocations exceed the budget: oldest must be evicted.
    auto second = desc;
    second.size = 32;
    buffer = device->createBuffer(second);
    SLANG_CHECK_ABORT(buffer);
    pool.beginLease(second.size);
    pool.recycle(second, std::move(buffer));
    auto third = desc;
    third.size = 48;
    buffer = device->createBuffer(third);
    SLANG_CHECK_ABORT(buffer);
    pool.beginLease(third.size);
    pool.recycle(third, std::move(buffer));
    SLANG_CHECK(pool.getStats().cachedBytes == 80);
    SLANG_CHECK(pool.getStats().evictions == 1);
    SLANG_CHECK(!pool.acquire(desc));
    pool.trim();
    SLANG_CHECK(pool.getStats().cachedBytes == 0);
    SLANG_CHECK(pool.getStats().leasedBytes == 0);

    const char* source = R"(
        [shader("compute")] [numthreads(1,1,1)]
        void store(uniform RWStructuredBuffer<uint> dst, uniform uint value) { dst[0] = value; }
        [shader("compute")] [numthreads(1,1,1)]
        void transfer(uniform RWStructuredBuffer<uint> dst,
            uniform RWStructuredBuffer<uint>.Handle src, uniform uint i)
        { dst[i] = src[0]; }
        void enqueue(RWStructuredBuffer<uint> dst, uint i)
        {
            var scratch = RWStructuredBuffer<uint>::alloc(1);
            dispatch(store, uint3(1, 1, 1))(scratch, i + 100);
            var handle = scratch.getDescriptorHandle();
            dispatch(transfer, uint3(1, 1, 1))(dst, handle, i);
        }
        void step(uint value)
        {
            var scratch = RWStructuredBuffer<uint>::alloc(1);
            dispatch(store, uint3(1, 1, 1))(scratch, value);
            print(readBack(scratch, 0));
        }
        [host] void main()
        {
            // All scratch wrappers go out of scope before this batch is submitted.
            var output = RWStructuredBuffer<uint>::alloc(16);
            for (uint i = 0; i < 16; ++i) enqueue(output, i);
            for (uint i = 0; i < 16; ++i) print(readBack(output, i));
            // Each step's readback makes its scratch eligible for the next step.
            for (uint i = 0; i < 8; ++i) step(i);
        }
    )";
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(global.writeRef())));
    slang::TargetDesc target = {};
    target.format = SLANG_HGBC;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &target;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = &option;
    sessionDesc.compilerOptionEntryCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(global->createSession(sessionDesc, session.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "buffer_pool",
        "buffer_pool.slang",
        source,
        diagnostics.writeRef());
    if (diagnostics)
        fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
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
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef())));
    RefPtr<HGVM::Program> program;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(HGVM::Program::load(
        code->getBufferPointer(),
        code->getBufferSize(),
        "buffer_pool",
        program,
        error)));
    RefPtr<PoolOutput> sink = new PoolOutput;
    String expected;
    for (int i = 0; i < 16; ++i)
    {
        expected.append(i + 100);
        expected.append("\n");
    }
    for (int i = 0; i < 8; ++i)
    {
        expected.append(i);
        expected.append("\n");
    }

    for (uint64_t limit : {uint64_t(0), uint64_t(1024)})
    {
        HGVM::Desc vmDesc = {};
        vmDesc.device = device;
        vmDesc.outputSink = sink;
        vmDesc.bufferPoolByteLimit = limit;
        HGVM::Interpreter vm(vmDesc);
        uint64_t warmMisses = 0;
        for (int repeat = 0; repeat < 3; ++repeat)
        {
            sink->text = "";
            SlangResult result = vm.call(program, "main", error);
            if (SLANG_FAILED(result))
                fprintf(stderr, "%s\n", error.getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK(sink->text == expected);
            auto stats = vm.getBufferPoolStats();
            SLANG_CHECK(stats.leasedBytes == 0);
            if (limit)
            {
                SLANG_CHECK(stats.hits >= 8);
                if (!repeat)
                {
                    warmMisses = stats.misses;
                    // One output plus sixteen scratch buffers that overlap before submission.
                    SLANG_CHECK(warmMisses == 17);
                }
                else
                    SLANG_CHECK(stats.misses == warmMisses);
            }
            else
            {
                SLANG_CHECK(stats.hits == 0);
                SLANG_CHECK(stats.cachedBytes == 0);
                SLANG_CHECK(stats.misses == uint64_t(25 * (repeat + 1)));
            }
        }
        SLANG_CHECK(SLANG_SUCCEEDED(vm.trimBufferPool(error)));
        SLANG_CHECK(vm.getBufferPoolStats().cachedBytes == 0);
        SLANG_CHECK(vm.getBufferPoolStats().leasedBytes == 0);
    }

    // Physical reclamation must not stop at CUDA's secondary page cache.
    SLANG_CHECK(SLANG_SUCCEEDED(device->trimMemory()));
    if (api == rhi::DeviceType::CUDA)
    {
        uint32_t count = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(device->reportHeaps(nullptr, &count)));
        List<rhi::HeapReport> reports;
        reports.setCount(count);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(device->reportHeaps(reports.getBuffer(), &count)));
        for (const auto& report : reports)
            SLANG_CHECK(report.numAllocations == 0);
    }
}
} // namespace

SLANG_UNIT_TEST(hgvmBufferPoolVulkan)
{
    testBufferPool(unitTestContext, rhi::DeviceType::Vulkan);
}
SLANG_UNIT_TEST(hgvmBufferPoolCUDA)
{
    testBufferPool(unitTestContext, rhi::DeviceType::CUDA);
}
