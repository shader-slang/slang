// Included after InterpreterState so acceleration-structure execution can use its staged reads.

/// Allocates native storage from the backend's size query and records one complete build.
/// Scratch storage and the destination remain alive until the containing batch completes.
/// For example, scene.build(instances) only snapshots a TLAS; binding that scene first calls
/// this method to realize it. Tracking here counts the native build once, while later bindings
/// that reuse the same realization produce no build event.
ComPtr<rhi::IAccelerationStructure> DeviceState::buildNativeRayAccelerationStructure(
    rhi::ICommandEncoder* encoder,
    const rhi::AccelerationStructureBuildDesc& build,
    rhi::AccelerationStructureKind kind)
{
    rhi::AccelerationStructureSizes sizes;
    if (SLANG_FAILED(device->getAccelerationStructureSizes(build, &sizes)))
        fail("HGVM failed to query ray acceleration-structure build sizes");

    rhi::BufferDesc scratchDesc;
    scratchDesc.size = sizes.scratchSize;
    scratchDesc.usage = rhi::BufferUsage::UnorderedAccess;
    scratchDesc.defaultState = rhi::ResourceState::UnorderedAccess;
    auto scratch = device->createBuffer(scratchDesc);
    if (!scratch)
        fail("HGVM failed to allocate ray acceleration-structure scratch storage");

    rhi::AccelerationStructureDesc desc;
    desc.kind = kind;
    desc.size = sizes.accelerationStructureSize;
    ComPtr<rhi::IAccelerationStructure> result;
    if (SLANG_FAILED(device->createAccelerationStructure(desc, result.writeRef())))
        fail("HGVM failed to allocate a ray acceleration structure");
    pendingBatch.holds.buffers.add(scratch);
    pendingBatch.holds.accelerationStructures.add(result);
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        beginAccelerationStructureBuild(
            pendingBatch.executions,
            encoder,
            pendingBatch.trackingId,
            result.get(),
            kind));
    encoder->buildAccelerationStructure(build, result, nullptr, scratch, 0, nullptr);
    SLANG_HGVM_TRACK(eventTracker.get(), endDeviceExecution(pendingBatch.executions, encoder));
    return result;
}

/// Checks ranges against the actual allocation rather than assuming a host float3 stride.
/// Device layouts may pad vertices, while indices must remain packed 32-bit unsigned integers.
static void validateRayTriangleBuffer(
    DeviceState::StructuredBufferObject* buffer,
    uint32_t offset,
    uint32_t count,
    bool vertices)
{
    const auto& desc = buffer->resource->getDesc();
    if (!rhi::is_set(desc.usage, rhi::BufferUsage::AccelerationStructureBuildInput))
        fail("HGVM ray geometry buffers require AccelerationStructureBuildInput usage");
    if (vertices ? (desc.elementSize < 12 || desc.elementSize % 4 != 0) : desc.elementSize != 4)
        fail("HGVM ray triangles require float3 vertices and uint32 indices");
    SLANG_RELEASE_ASSERT(desc.elementSize == buffer->elementStride);
    uint64_t capacity = desc.size / desc.elementSize;
    if (offset > capacity || count > capacity - offset)
        fail("HGVM ray geometry buffer range is out of bounds");
}

/// Converts a validated element offset into the native RHI offset representation.
/// Buffer sizes are 64-bit even on WASM32, while RHI offsets use the host's size_t width.
static rhi::Offset getRayGeometryByteOffset(uint32_t elementOffset, uint32_t elementStride)
{
    uint64_t byteOffset = uint64_t(elementOffset) * elementStride;
    if (byteOffset > std::numeric_limits<rhi::Offset>::max())
        fail("HGVM ray geometry buffer offset exceeds the native RHI address range");
    return rhi::Offset(byteOffset);
}

void DeviceState::buildRayAccelerationStructure(
    InterpreterState& interpreter,
    const StagedRecord::BuildRayAccelerationStructure& record)
{
    if (!device || !device->hasFeature(rhi::Feature::RayTracing) ||
        (device->getDeviceType() != rhi::DeviceType::Vulkan &&
         device->getDeviceType() != rhi::DeviceType::D3D12))
        fail("HGVM ray acceleration structures currently require Vulkan or D3D12 ray tracing");
    if (!interpreter.readValue<Address>(record.structureAddress))
        fail("HGVM cannot build a null ray acceleration structure");
    auto object =
        interpreter.readHeapObject<RayAccelerationStructureObject>(record.structureAddress);
    SLANG_RELEASE_ASSERT(object->topLevel == record.topLevel);
    uint32_t mode = interpreter.readValue<uint32_t>(record.buildModeAddress);
    if (mode != uint32_t(rhi::AccelerationStructureBuildMode::Build))
        fail("HGVM ray acceleration-structure updates are not supported; use Build mode");
    uint32_t flags = interpreter.readValue<uint32_t>(record.buildFlagsAddress);
    constexpr uint32_t supportedFlags =
        uint32_t(rhi::AccelerationStructureBuildFlags::AllowUpdate) |
        uint32_t(rhi::AccelerationStructureBuildFlags::AllowCompaction) |
        uint32_t(rhi::AccelerationStructureBuildFlags::PreferFastTrace) |
        uint32_t(rhi::AccelerationStructureBuildFlags::PreferFastBuild) |
        uint32_t(rhi::AccelerationStructureBuildFlags::MinimizeMemory);
    uint32_t preferences = uint32_t(rhi::AccelerationStructureBuildFlags::PreferFastTrace) |
                           uint32_t(rhi::AccelerationStructureBuildFlags::PreferFastBuild);
    if ((flags & ~supportedFlags) || (flags & preferences) == preferences)
        fail("HGVM ray acceleration-structure build flags are unsupported or conflicting");
    auto inputs = interpreter.getRuntimeArrayObject(
        interpreter.readValue<Address>(record.inputsAddress),
        record.inputsTypeAddress);
    if (!inputs->elementCount)
        fail("HGVM ray acceleration-structure builds require at least one input");

    if (record.topLevel)
    {
        List<rhi::AccelerationStructureInstanceDescGeneric> instances;
        List<ComPtr<rhi::IAccelerationStructure>> bottomLevels;
        uint64_t geometryCount = 0;
        List<HGBCRayGeometryKind> geometryKinds;
        for (uint32_t i = 0; i < inputs->elementCount; ++i)
        {
            Address input =
                Address::fromHeapPointer(inputs->payload + i * inputs->elementByteStride);
            if (!interpreter.readValueAt<Address>(input, record.fieldOffsets[0]))
                fail(
                    "HGVM ray scene instances require a built bottom-level acceleration structure");
            auto blas = interpreter.readHeapObjectAt<RayAccelerationStructureObject>(
                input,
                record.fieldOffsets[0]);
            if (blas->topLevel || !blas->resource)
                fail(
                    "HGVM ray scene instances require a built bottom-level acceleration structure");
            uint32_t instanceID = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[2]);
            uint32_t mask = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[3]);
            uint32_t instanceFlags =
                interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[4]);
            uint32_t opaqueFlags = uint32_t(rhi::AccelerationStructureInstanceFlags::ForceOpaque) |
                                   uint32_t(rhi::AccelerationStructureInstanceFlags::NoOpaque);
            if (instanceID > 0xffffff || mask > 0xff || (instanceFlags & ~0xfu) ||
                (instanceFlags & opaqueFlags) == opaqueFlags)
                fail("HGVM ray instance ID, visibility mask, or flags are out of range");
            if (geometryCount > 0xffffff)
                fail("HGVM ray scene geometry prefix exceeds the 24-bit shader-table limit");
            rhi::AccelerationStructureInstanceDescGeneric instance = {};
            // HGVM's canonical host matrix layout is twelve packed scalars in row-major
            // order. Read that staged representation explicitly; shader matrix packing and
            // a host C++ matrix type do not determine the native instance descriptor layout.
            for (uint32_t row = 0; row < 3; ++row)
            {
                for (uint32_t column = 0; column < 4; ++column)
                {
                    float value = interpreter.readValueAt<float>(
                        input,
                        record.fieldOffsets[1] + (row * 4 + column) * sizeof(float));
                    if (!std::isfinite(value))
                        fail("HGVM ray instance transforms must contain finite values");
                    instance.transform[row][column] = value;
                }
            }
            instance.instanceID = instanceID;
            instance.instanceMask = mask;
            instance.flags = rhi::AccelerationStructureInstanceFlags(instanceFlags);
            // Store a logical geometry prefix. The selected pipeline's total ray-type width
            // is applied only when realizing a native TLAS, so future multiple trackers can
            // share this snapshot by choosing their combined width and individual offsets.
            instance.instanceContributionToHitGroupIndex = uint32_t(geometryCount);
            instance.accelerationStructure = blas->resource->getHandle();
            geometryCount += blas->geometryCount;
            geometryKinds.addRange(blas->geometryKinds);
            if (geometryCount > std::numeric_limits<uint32_t>::max())
                fail("HGVM ray scene has too many geometries");
            instances.add(instance);
            bottomLevels.add(blas->resource);
        }
        // Replace the logical snapshot only after all inputs validate. Existing submissions
        // independently retain their old native TLAS and BLAS objects in the batch holds.
        object->instances.swapWith(instances);
        object->bottomLevels.swapWith(bottomLevels);
        object->realizations.clear();
        object->geometryCount = uint32_t(geometryCount);
        object->geometryKinds.swapWith(geometryKinds);
    }
    else
    {
        // The RHI input is a tagged union without a default constructor. Construct each
        // initialized triangle explicitly instead of asking List to default-create slots.
        std::vector<rhi::AccelerationStructureBuildInput> geometries;
        List<RefPtr<StructuredBufferObject>> leases;
        for (uint32_t i = 0; i < inputs->elementCount; ++i)
        {
            Address input =
                Address::fromHeapPointer(inputs->payload + i * inputs->elementByteStride);
            if (record.inputKind == HGBCAccelerationStructureInputKind::AABBs)
            {
                if (!interpreter.readValueAt<Address>(input, record.fieldOffsets[0]))
                    fail("HGVM procedural geometry requires an AABB buffer");
                auto bounds = interpreter.readHeapObjectAt<StructuredBufferObject>(
                    input,
                    record.fieldOffsets[0]);
                uint32_t offset = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[1]);
                uint32_t count = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[2]);
                uint32_t geometryFlags =
                    interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[3]);
                const auto& desc = bounds->resource->getDesc();
                if (!count || (geometryFlags & ~3u))
                    fail("HGVM procedural geometry requires a nonempty range and valid flags");
                if (!rhi::is_set(desc.usage, rhi::BufferUsage::AccelerationStructureBuildInput))
                    fail("HGVM ray geometry buffers require AccelerationStructureBuildInput usage");
                if (desc.elementSize != sizeof(rhi::AccelerationStructureAABB))
                    fail("HGVM procedural bounds require packed 24-byte AABBs");
                uint64_t capacity = desc.size / desc.elementSize;
                if (offset > capacity || count > capacity - offset)
                    fail("HGVM procedural bounds range is out of bounds");
                rhi::AccelerationStructureBuildInput geometry = {};
                geometry.type = rhi::AccelerationStructureBuildInputType::ProceduralPrimitives;
                geometry.proceduralPrimitives.aabbBuffers[0] = {
                    bounds->resource,
                    getRayGeometryByteOffset(offset, desc.elementSize)};
                geometry.proceduralPrimitives.aabbBufferCount = 1;
                geometry.proceduralPrimitives.aabbStride = desc.elementSize;
                geometry.proceduralPrimitives.primitiveCount = count;
                geometry.proceduralPrimitives.flags =
                    rhi::AccelerationStructureGeometryFlags(geometryFlags);
                geometries.push_back(geometry);
                leases.add(bounds);
                continue;
            }
            if (!interpreter.readValueAt<Address>(input, record.fieldOffsets[0]) ||
                !interpreter.readValueAt<Address>(input, record.fieldOffsets[1]))
                fail("HGVM ray triangle geometry requires vertex and index buffers");
            auto vertices =
                interpreter.readHeapObjectAt<StructuredBufferObject>(input, record.fieldOffsets[0]);
            auto indices =
                interpreter.readHeapObjectAt<StructuredBufferObject>(input, record.fieldOffsets[1]);
            uint32_t vertexOffset =
                interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[2]);
            uint32_t vertexCount = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[3]);
            uint32_t indexOffset = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[4]);
            uint32_t indexCount = interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[5]);
            uint32_t geometryFlags =
                interpreter.readValueAt<uint32_t>(input, record.fieldOffsets[6]);
            if (!vertexCount || !indexCount || indexCount % 3 || (geometryFlags & ~3u))
                fail("HGVM ray triangles require nonempty vertex/index ranges and valid geometry "
                     "flags");
            validateRayTriangleBuffer(vertices, vertexOffset, vertexCount, true);
            validateRayTriangleBuffer(indices, indexOffset, indexCount, false);
            rhi::AccelerationStructureBuildInput geometry = {};
            geometry.type = rhi::AccelerationStructureBuildInputType::Triangles;
            geometry.triangles.vertexBuffers[0] = {
                vertices->resource,
                getRayGeometryByteOffset(vertexOffset, vertices->resource->getDesc().elementSize)};
            geometry.triangles.vertexBufferCount = 1;
            geometry.triangles.vertexFormat = rhi::Format::RGB32Float;
            geometry.triangles.vertexCount = vertexCount;
            geometry.triangles.vertexStride = vertices->resource->getDesc().elementSize;
            geometry.triangles.indexBuffer = {
                indices->resource,
                getRayGeometryByteOffset(indexOffset, indices->resource->getDesc().elementSize)};
            geometry.triangles.indexFormat = rhi::IndexFormat::Uint32;
            geometry.triangles.indexCount = indexCount;
            geometry.triangles.flags = rhi::AccelerationStructureGeometryFlags(geometryFlags);
            geometries.push_back(geometry);
            leases.add(vertices);
            leases.add(indices);
        }
        auto encoder = getPendingCommandEncoder();
        for (const auto& lease : leases)
        {
            pendingBatch.holds.bufferLeases.add(lease);
            encoder->setBufferState(
                lease->resource,
                rhi::ResourceState::AccelerationStructureBuildInput);
        }
        rhi::AccelerationStructureBuildDesc build;
        build.inputs = geometries.data();
        build.inputCount = uint32_t(geometries.size());
        build.flags = rhi::AccelerationStructureBuildFlags(flags);
        object->resource = buildNativeRayAccelerationStructure(
            encoder,
            build,
            rhi::AccelerationStructureKind::BottomLevel);
        object->geometryCount = inputs->elementCount;
        object->geometryKinds.setCount(inputs->elementCount);
        for (auto& kind : object->geometryKinds)
            kind = record.inputKind == HGBCAccelerationStructureInputKind::AABBs
                       ? HGBCRayGeometryKind::Procedural
                       : HGBCRayGeometryKind::Triangles;
        finishDeviceOperation();
    }
    object->buildFlags = flags;
    ++object->version;
}

ComPtr<rhi::IAccelerationStructure> DeviceState::realizeRayAccelerationStructure(
    RayAccelerationStructureObject* object,
    uint32_t rayTypeCount)
{
    if (!object->topLevel || !object->instances.getCount())
        fail("HGVM ray tracing requires a built top-level acceleration structure");
    if (!rayTypeCount || rayTypeCount > 15)
        fail("HGVM ray tracing requires between one and fifteen ray types");
    for (const auto& blas : object->bottomLevels)
        pendingBatch.holds.accelerationStructures.add(blas);
    if (auto cached = object->realizations.tryGetValue(rayTypeCount))
    {
        pendingBatch.holds.accelerationStructures.add(*cached);
        return *cached;
    }

    auto instances = object->instances;
    for (auto& instance : instances)
    {
        uint64_t contribution =
            uint64_t(instance.instanceContributionToHitGroupIndex) * rayTypeCount;
        if (contribution > 0xffffff)
            fail("HGVM ray scene contribution exceeds the 24-bit shader-table limit");
        instance.instanceContributionToHitGroupIndex = uint32_t(contribution);
    }
    auto nativeType = rhi::getAccelerationStructureInstanceDescType(device);
    size_t stride = rhi::getAccelerationStructureInstanceDescSize(nativeType);
    std::vector<uint8_t> nativeInstances(size_t(instances.getCount()) * stride);
    rhi::convertAccelerationStructureInstanceDescs(
        size_t(instances.getCount()),
        nativeType,
        nativeInstances.data(),
        stride,
        instances.getBuffer(),
        sizeof(rhi::AccelerationStructureInstanceDescGeneric));
    rhi::BufferDesc bufferDesc;
    bufferDesc.size = nativeInstances.size();
    bufferDesc.usage = rhi::BufferUsage::AccelerationStructureBuildInput;
    bufferDesc.defaultState = rhi::ResourceState::AccelerationStructureBuildInput;
    auto buffer = device->createBuffer(bufferDesc, nativeInstances.data());
    if (!buffer)
        fail("HGVM failed to allocate native ray scene instances");
    pendingBatch.holds.buffers.add(buffer);
    rhi::AccelerationStructureBuildInput input = {};
    input.type = rhi::AccelerationStructureBuildInputType::Instances;
    input.instances.instanceBuffer = buffer;
    input.instances.instanceStride = uint32_t(stride);
    input.instances.instanceCount = uint32_t(instances.getCount());
    rhi::AccelerationStructureBuildDesc build;
    build.inputs = &input;
    build.inputCount = 1;
    build.flags = rhi::AccelerationStructureBuildFlags(object->buildFlags);
    auto result = buildNativeRayAccelerationStructure(
        getPendingCommandEncoder(),
        build,
        rhi::AccelerationStructureKind::TopLevel);
    object->realizations.add(rayTypeCount, result);
    finishDeviceOperation();
    return result;
}
