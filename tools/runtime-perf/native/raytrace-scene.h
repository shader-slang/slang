// Fixed scene and rays for compiler comparisons. Builds finish before calibration.
#include <slang-rhi/acceleration-structure-utils.h>

struct RayScene
{
    static constexpr uint32_t side = 32;
    static constexpr uint32_t cells = side * side;
    ComPtr<IAccelerationStructure> blas, tlas;
    double setupMs = 0;

    static void origin(uint32_t i, float* out)
    {
        uint32_t cell = (i / 4) % cells;
        bool miss = i % 4 == 3;
        out[0] =
            float(cell % side) + (miss ? 0.75f : 0.125f + float((i / (cells * 4)) % 3) * 0.0625f);
        out[1] = float(cell / side) + (miss ? 0.75f : 0.25f);
        out[2] = 0;
        out[3] = 4;
    }

    // Analytic intersection against the near plane and cell triangle. Rays avoid edges.
    static void expected(uint32_t i, bool visibility, float* out)
    {
        float ray[4];
        origin(i, ray);
        uint32_t x = uint32_t(std::floor(ray[0])), y = uint32_t(std::floor(ray[1]));
        float u = ray[0] - float(x), v = ray[1] - float(y);
        bool hit = x < side && y < side && u + v < 1.0f;
        if (visibility)
        {
            out[0] = hit ? 1.0f : 0.0f;
            out[1] = out[2] = out[3] = 0;
        }
        else if (hit)
        {
            out[0] = 1;
            out[1] = float(cells + y * side + x);
            out[2] = u;
            out[3] = v;
        }
        else
            out[0] = out[1] = out[2] = out[3] = -1;
    }

    bool build(
        IDevice* device,
        ICommandQueue* queue,
        const AccelerationStructureBuildInput& input,
        AccelerationStructureKind kind,
        ComPtr<IAccelerationStructure>& result)
    {
        AccelerationStructureBuildDesc desc = {};
        desc.inputs = &input;
        desc.inputCount = 1;
        desc.flags = AccelerationStructureBuildFlags::PreferFastTrace;
        AccelerationStructureSizes sizes = {};
        CHECK(device->getAccelerationStructureSizes(desc, &sizes));
        AccelerationStructureDesc asDesc = {};
        asDesc.kind = kind;
        asDesc.size = sizes.accelerationStructureSize;
        CHECK(device->createAccelerationStructure(asDesc, result.writeRef()));
        BufferDesc scratchDesc = {};
        scratchDesc.size = sizes.scratchSize;
        scratchDesc.usage = BufferUsage::UnorderedAccess;
        scratchDesc.defaultState = ResourceState::UnorderedAccess;
        ComPtr<IBuffer> scratch;
        CHECK(device->createBuffer(scratchDesc, nullptr, scratch.writeRef()));
        auto encoder = queue->createCommandEncoder();
        if (!encoder)
            return false;
        encoder->buildAccelerationStructure(desc, result, nullptr, scratch, 0, nullptr);
        CHECK(queue->submit(encoder->finish()));
        CHECK(queue->waitOnHost());
        return true;
    }

    bool initialize(IDevice* device, ICommandQueue* queue)
    {
        auto start = Clock::now();
        std::vector<float> vertices(cells * 2 * 9);
        size_t offset = 0;
        // Far triangles precede near triangles so closest-hit validation detects wrong selection.
        for (float z : {2.0f, 1.0f})
            for (uint32_t y = 0; y < side; ++y)
                for (uint32_t x = 0; x < side; ++x)
                {
                    const float triangle[] = {
                        float(x),
                        float(y),
                        z,
                        float(x + 1),
                        float(y),
                        z,
                        float(x),
                        float(y + 1),
                        z};
                    for (float component : triangle)
                        vertices[offset++] = component;
                }
        BufferDesc vertexDesc = {};
        vertexDesc.size = vertices.size() * sizeof(float);
        vertexDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        vertexDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        ComPtr<IBuffer> vertexBuffer;
        CHECK(device->createBuffer(vertexDesc, vertices.data(), vertexBuffer.writeRef()));
        AccelerationStructureBuildInput triangles = {};
        triangles.type = AccelerationStructureBuildInputType::Triangles;
        triangles.triangles.vertexBuffers[0] = vertexBuffer;
        triangles.triangles.vertexBufferCount = 1;
        triangles.triangles.vertexFormat = Format::RGB32Float;
        triangles.triangles.vertexCount = uint32_t(vertices.size() / 3);
        triangles.triangles.vertexStride = 3 * sizeof(float);
        triangles.triangles.flags = AccelerationStructureGeometryFlags::Opaque;
        if (!build(device, queue, triangles, AccelerationStructureKind::BottomLevel, blas))
            return false;

        AccelerationStructureInstanceDescGeneric instance = {};
        instance.transform[0][0] = instance.transform[1][1] = instance.transform[2][2] = 1;
        instance.instanceMask = 255;
        instance.accelerationStructure = blas->getHandle();
        auto type = getAccelerationStructureInstanceDescType(device);
        size_t stride = getAccelerationStructureInstanceDescSize(type);
        std::vector<uint8_t> bytes(stride);
        convertAccelerationStructureInstanceDescs(
            1,
            type,
            bytes.data(),
            stride,
            &instance,
            sizeof(instance));
        BufferDesc instanceDesc = {};
        instanceDesc.size = bytes.size();
        instanceDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        instanceDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        ComPtr<IBuffer> instanceBuffer;
        CHECK(device->createBuffer(instanceDesc, bytes.data(), instanceBuffer.writeRef()));
        AccelerationStructureBuildInput instances = {};
        instances.type = AccelerationStructureBuildInputType::Instances;
        instances.instances.instanceBuffer = instanceBuffer;
        instances.instances.instanceCount = 1;
        instances.instances.instanceStride = uint32_t(stride);
        if (!build(device, queue, instances, AccelerationStructureKind::TopLevel, tlas))
            return false;
        setupMs = elapsed(start);
        return true;
    }
};
