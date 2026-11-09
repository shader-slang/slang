// Included after the ordinary shader binder so ray records use the same conversion rules.

/// Resolves HGBC's canonical source type and compares it with the shader's declared type.
/// Consider DescriptorHandle<RaytracingAccelerationStructure>: its storage layout may be uint2,
/// but its variable reflection retains the descriptor type. Compare those semantic types using
/// Slang's existing subtype relation in both directions, never their size or physical fields.
static void requireRayShaderType(
    InterpreterState& interpreter,
    HGBCId sourceTypeId,
    slang::TypeReflection* shaderType,
    const char* role)
{
    auto sourceName = interpreter.program->sourceProgram->getSlangTypeName(sourceTypeId);
    auto* component = interpreter.getDispatchTypeLookupComponent();
    auto* layout = component ? component->getLayout() : nullptr;
    auto* sourceType = sourceName && layout ? layout->findTypeByName(sourceName->c_str()) : nullptr;
    if (!sourceType || !shaderType)
        fail(std::string("HGVM cannot resolve the declared ray ") + role + " type");
    if (!layout->isSubType(sourceType, shaderType) || !layout->isSubType(shaderType, sourceType))
        fail(std::string("HGVM ray ") + role + " type does not match shader reflection");
}

/// Checks the native payload/attribute ABI after specialization has resolved each stage's types.
/// Wire type IDs alone cannot establish this: malformed records could attach the same declared
/// payload ID to two shaders whose actual payload declarations differ.
static void validateRayRegistrationTypes(
    InterpreterState& interpreter,
    RayPipelineObject* pipeline,
    RayShaderRegistration* registration,
    uint32_t entryPointIndex)
{
    auto* entry = pipeline->linkedComponent->getLayout()->getEntryPointByIndex(entryPointIndex);
    uint32_t payloadCount = 0;
    uint32_t attributeCount = 0;
    for (uint32_t index = 0; index < entry->getParameterCount(); ++index)
    {
        auto* parameter = entry->getParameterByIndex(index);
        switch (parameter->getCategory())
        {
        case slang::ParameterCategory::RayPayload:
            ++payloadCount;
            requireRayShaderType(
                interpreter,
                registration->payloadType,
                parameter->getType(),
                "payload");
            break;
        case slang::ParameterCategory::HitAttributes:
            ++attributeCount;
            requireRayShaderType(
                interpreter,
                registration->attributeType,
                parameter->getType(),
                "attributes");
            break;
        default:
            break;
        }
    }
    SlangStage stage = pipeline->stages[entryPointIndex];
    bool hasPayload = stage == SLANG_STAGE_MISS || stage == SLANG_STAGE_CLOSEST_HIT ||
                      stage == SLANG_STAGE_ANY_HIT;
    bool hasAttributes = stage == SLANG_STAGE_CLOSEST_HIT || stage == SLANG_STAGE_ANY_HIT;
    if (payloadCount != uint32_t(hasPayload) || attributeCount != uint32_t(hasAttributes))
        fail("HGVM ray shader has an incompatible native payload/attribute signature");
}

/// Resolves one canonical staged shader identity and deduplicates it in the native program.
static uint32_t addRayEntryPoint(
    InterpreterState& interpreter,
    RayPipelineObject* pipeline,
    Address address,
    SlangStage stage)
{
    auto* entryPoint = interpreter.readConstObject<SlangEntryPointObject>(address);
    for (Index i = 0; i < pipeline->entryPoints.getCount(); ++i)
        if (pipeline->entryPoints[i] == entryPoint && pipeline->stages[i] == stage)
            return uint32_t(i);
    pipeline->entryPoints.add(entryPoint);
    pipeline->stages.add(stage);
    return uint32_t(pipeline->entryPoints.getCount() - 1);
}

/// Copies argument references from owned registration storage without copying resource contents.
static List<StagedRecord::RayShaderArgument> getRayRegistrationArguments(
    InterpreterState& interpreter,
    RayShaderRegistration* registration)
{
    if (registration->programInstance != interpreter.programInstance)
        fail("HGVM ray dispatch cannot combine registrations from different program instances");
    List<StagedRecord::RayShaderArgument> arguments;
    for (const auto& value : registration->arguments)
        arguments.add(
            {Address::fromHeapPointer(value->bytes),
             value->typeInfoAddress,
             value->parameterIndex,
             value->stage,
             value->typeId});
    return arguments;
}

/// Finds managed tracker and scene leaves using the canonical ownership map, never byte scanning.
static void discoverRayArguments(
    InterpreterState& interpreter,
    ConstArrayView<StagedRecord::RayShaderArgument> arguments,
    RaytracingPipelineObject*& tracker,
    List<DeviceState::RayAccelerationStructureObject*>& scenes)
{
    for (const auto& argument : arguments)
    {
        const auto& type =
            interpreter.readInstructionRecord<StagedRecord::TypeInfo>(argument.typeInfoAddress);
        if (!type.managedLeafCount)
            continue;
        auto offsets = interpreter.resolveAddress<const uint32_t>(
            type.managedLeafOffsetsAddress,
            type.managedLeafCount * sizeof(uint32_t));
        for (uint32_t i = 0; i < type.managedLeafCount; ++i)
        {
            Address handle =
                interpreter.readValueAt<Address>(argument.valueAddress, offsets.get()[i]);
            if (!handle)
                continue;
            auto* object = handle.getObject();
            if (object->kind == HeapObjectKind::RaytracingPipeline)
            {
                // Multiple trackers will require graph traversal and a column prefix per tracker.
                if (tracker && tracker != object)
                    fail("HGVM ray dispatch supports exactly one captured tracker");
                tracker = static_cast<RaytracingPipelineObject*>(object);
            }
            else if (object->kind == HeapObjectKind::RayAccelerationStructure)
            {
                auto* scene = static_cast<DeviceState::RayAccelerationStructureObject*>(object);
                if (!scene->topLevel || !scene->version)
                    fail("HGVM ray dispatch requires a built top-level acceleration structure");
                if (scenes.indexOf(scene) < 0)
                    scenes.add(scene);
            }
        }
    }
}

/// Compares native pipeline keys by canonical code identity. Captured values are absent.
static bool hasSameRayPipelineKey(const RayPipelineObject& a, const RayPipelineObject& b)
{
    if (a.programInstance != b.programInstance || a.maxRecursion != b.maxRecursion ||
        a.entryPoints.getCount() != b.entryPoints.getCount() ||
        a.hitGroups.getCount() != b.hitGroups.getCount())
        return false;
    for (Index i = 0; i < a.entryPoints.getCount(); ++i)
        if (a.entryPoints[i] != b.entryPoints[i] || a.stages[i] != b.stages[i])
            return false;
    for (Index i = 0; i < a.hitGroups.getCount(); ++i)
    {
        if (a.hitGroups[i].geometryKind != b.hitGroups[i].geometryKind ||
            a.hitGroups[i].attributeType != b.hitGroups[i].attributeType)
            return false;
        for (uint32_t role = 0; role < 3; ++role)
            if (a.hitGroups[i].entryPoints[role] != b.hitGroups[i].entryPoints[role])
                return false;
    }
    return true;
}

/// Produces distinct native export names for distinct specializations of a source entry point.
static String getRayExportName(const char* prefix, Index index)
{
    StringBuilder name;
    name << prefix << index;
    return name.produceString();
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
/// Describes the native shader inventory using staged identities, before export renaming.
/// A shader shared by two hit groups appears in each group, while repeated SBT records do not
/// duplicate entries. This keeps group membership visible without depending on record contents.
static List<TrackedPipelineShader> getRayPipelineShaders(const RayPipelineObject* pipeline)
{
    List<TrackedPipelineShader> shaders;
    uint64_t programId = pipeline->programInstance->program->debugMap->getProgramId();
    for (Index i = 0; i < pipeline->entryPoints.getCount(); ++i)
    {
        auto stage = pipeline->stages[i];
        if (stage != SLANG_STAGE_CLOSEST_HIT && stage != SLANG_STAGE_ANY_HIT &&
            stage != SLANG_STAGE_INTERSECTION)
            shaders.add({stage, {programId, pipeline->entryPoints[i]->descriptorWordOffset}});
    }
    for (Index group = 0; group < pipeline->hitGroups.getCount(); ++group)
        for (auto index : pipeline->hitGroups[group].entryPoints)
            if (index >= 0)
                shaders.add(
                    {pipeline->stages[index],
                     {programId, pipeline->entryPoints[index]->descriptorWordOffset},
                     uint32_t(group)});
    return shaders;
}
#endif

/// Builds the immutable program once; per-dispatch record bytes never participate in this step.
static void createRayPipeline(
    DeviceState& device,
    InterpreterState& interpreter,
    RayPipelineObject* pipeline)
{
    List<ComPtr<slang::IComponentType>> specialized;
    List<slang::IComponentType*> components;
    List<slang::IModule*> modules;
    for (Index i = 0; i < pipeline->entryPoints.getCount(); ++i)
    {
        auto* entryPoint = pipeline->entryPoints[i];
        if (modules.indexOf(entryPoint->module->module.get()) < 0)
        {
            modules.add(entryPoint->module->module);
            components.add(entryPoint->module->module);
        }
        auto entry = device.specializeEntryPoint(entryPoint, pipeline->stages[i]);
        pipeline->exportNames.add(getRayExportName("hgvm_ray_", i));
        ComPtr<slang::IComponentType> renamed;
        if (SLANG_FAILED(
                entry->renameEntryPoint(pipeline->exportNames[i].getBuffer(), renamed.writeRef())))
            fail("HGVM failed to assign a unique ray shader export name");
        specialized.add(renamed);
    }
    for (const auto& entry : specialized)
        components.add(entry);
    ComPtr<ISlangBlob> diagnostics;
    ComPtr<slang::IComponentType> composite;
    auto session = pipeline->programInstance->slangSession;
    if (SLANG_FAILED(session->createCompositeComponentType(
            components.getBuffer(),
            components.getCount(),
            composite.writeRef(),
            diagnostics.writeRef())))
        fail("HGVM failed to compose ray shaders");
    List<List<SlangInt>> groupIndices;
    for (const auto& group : pipeline->hitGroups)
    {
        List<SlangInt> indices;
        for (auto entry : group.entryPoints)
            if (entry >= 0)
                indices.add(entry);
        if (indices.getCount())
            groupIndices.add(std::move(indices));
    }
    if (groupIndices.getCount())
    {
        List<slang::EntryPointUniformLayoutGroup> groups;
        for (const auto& indices : groupIndices)
            groups.add({indices.getBuffer(), indices.getCount()});
        ComPtr<slang::IEntryPointUniformLayoutService_Experimental> service;
        ComPtr<slang::IComponentType> constraints, constrained;
        if (SLANG_FAILED(composite->queryInterface(SLANG_IID_PPV_ARGS(service.writeRef()))) ||
            SLANG_FAILED(service->createEntryPointUniformLayoutConstraints(
                groups.getBuffer(),
                groups.getCount(),
                constraints.writeRef(),
                diagnostics.writeRef())))
            fail("HGVM failed to declare hit-group shader-record layout constraints");
        slang::IComponentType* parts[] = {composite, constraints};
        if (SLANG_FAILED(session->createCompositeComponentType(
                parts,
                2,
                constrained.writeRef(),
                diagnostics.writeRef())))
            fail("HGVM failed to compose shader-record layout constraints");
        composite = constrained;
    }
    if (SLANG_FAILED(composite->link(pipeline->linkedComponent.writeRef(), diagnostics.writeRef())))
        fail(
            std::string("HGVM failed to link a ray program: ") +
            (diagnostics ? static_cast<const char*>(diagnostics->getBufferPointer()) : ""));

    auto* layout = pipeline->linkedComponent->getLayout();
    if (layout->getParameterCount())
        fail("HGVM ray shaders must capture parameters; user global shader parameters are "
             "unsupported");
    uint32_t payloadSize = 0;
    uint32_t attributeSize = 0;
    for (SlangUInt i = 0; i < layout->getEntryPointCount(); ++i)
    {
        auto* entry = layout->getEntryPointByIndex(i);
        for (uint32_t p = 0; p < entry->getParameterCount(); ++p)
        {
            auto* parameter = entry->getParameterByIndex(p);
            if (parameter->getTypeLayout()->getBindingRangeCount())
                fail(
                    std::string("HGVM ray shader parameter '") + parameter->getName() +
                    "' requires resource slots; use getDescriptorHandle() for captured resources");
            auto category = parameter->getCategory();
            if (category == slang::ParameterCategory::RayPayload ||
                category == slang::ParameterCategory::HitAttributes)
            {
                auto* valueLayout = session->getTypeLayout(
                    parameter->getTypeLayout()->getType(),
                    0,
                    slang::LayoutRules::DefaultStructuredBuffer);
                uint32_t size = getReflectedByteSize(valueLayout->getSize());
                if (category == slang::ParameterCategory::RayPayload)
                    payloadSize = Math::Max(payloadSize, size);
                else
                    attributeSize = Math::Max(attributeSize, size);
            }
        }
    }
    // Intersection-only groups have no native attribute input parameter to reflect. Their
    // declared attribute type still participates in pipeline configuration and cache identity.
    for (const auto& group : pipeline->hitGroups)
    {
        auto sourceName = interpreter.program->sourceProgram->getSlangTypeName(group.attributeType);
        auto lookup = interpreter.getDispatchTypeLookupComponent()->getLayout();
        auto type = sourceName ? lookup->findTypeByName(sourceName->c_str()) : nullptr;
        if (!type)
            fail("HGVM cannot resolve hit-group attributes");
        auto attributeLayout =
            session->getTypeLayout(type, 0, slang::LayoutRules::DefaultStructuredBuffer);
        if (!attributeLayout || attributeLayout->getBindingRangeCount())
            fail("HGVM hit attributes must contain ordinary data only");
        attributeSize = Math::Max(attributeSize, getReflectedByteSize(attributeLayout->getSize()));
    }
    rhi::ShaderProgramDesc programDesc;
    programDesc.slangGlobalScope = pipeline->linkedComponent;
    programDesc.useRayTracingShaderRecordParameters = true;
    if (SLANG_FAILED(device.device->createShaderProgram(
            programDesc,
            pipeline->program.writeRef(),
            diagnostics.writeRef())))
        fail(
            std::string("HGVM failed to create a ray shader program: ") +
            (diagnostics ? static_cast<const char*>(diagnostics->getBufferPointer()) : ""));

    List<rhi::HitGroupDesc> groups;
    for (Index i = 0; i < pipeline->hitGroups.getCount(); ++i)
        pipeline->groupNames.add(getRayExportName("hgvm_hit_", i));
    for (Index i = 0; i < pipeline->hitGroups.getCount(); ++i)
    {
        rhi::HitGroupDesc group;
        group.hitGroupName = pipeline->groupNames[i].getBuffer();
        auto& source = pipeline->hitGroups[i];
        if (source.entryPoints[0] >= 0)
            group.closestHitEntryPoint = pipeline->exportNames[source.entryPoints[0]].getBuffer();
        if (source.entryPoints[1] >= 0)
            group.anyHitEntryPoint = pipeline->exportNames[source.entryPoints[1]].getBuffer();
        if (source.entryPoints[2] >= 0)
            group.intersectionEntryPoint = pipeline->exportNames[source.entryPoints[2]].getBuffer();
        groups.add(group);
    }
    rhi::RayTracingPipelineDesc desc;
    desc.program = pipeline->program;
    desc.hitGroups = groups.getBuffer();
    desc.hitGroupCount = uint32_t(groups.getCount());
    desc.maxRecursion = pipeline->maxRecursion;
    desc.maxRayPayloadSize = payloadSize;
    desc.maxAttributeSizeInBytes = attributeSize;
    SLANG_HGVM_BEGIN_TIMING(device.eventTracker.get(), creationStart);
    pipeline->pipeline = device.device->createRayTracingPipeline(desc);
    SLANG_HGVM_TRACK(
        device.eventTracker.get(),
        trackPipelineCreation(
            pipeline->pipeline.get(),
            TrackedPipelineKind::Raytracing,
            pipeline->pipeline ? SLANG_OK : SLANG_FAIL,
            creationStart,
            {},
            getRayPipelineShaders(pipeline)));
    if (!pipeline->pipeline)
        fail("HGVM failed to create a native ray-tracing pipeline");
}

/// Packs a record through the ordinary shader binder and returns an independent byte snapshot.
List<uint8_t> DeviceState::bindRayRecord(
    InterpreterState& interpreter,
    RayPipelineObject* pipeline,
    uint32_t entryPointIndex,
    ConstArrayView<StagedRecord::RayShaderArgument> arguments,
    HGBCRayStageRole role,
    Dictionary<HeapObject*, DescriptorAccess>& resources)
{
    auto root = device->createRootShaderObject(pipeline->program);
    auto entry = root ? root->getEntryPoint(entryPointIndex) : nullptr;
    if (!entry)
        fail("HGVM could not create a shader object for a ray record");
    rhi::ShaderCursor entryCursor(entry);
    auto* reflectedEntry =
        pipeline->linkedComponent->getLayout()->getEntryPointByIndex(entryPointIndex);
    HashSet<uint32_t> boundParameters;
    for (const auto& argument : arguments)
    {
        if (argument.stage != role)
            continue;
        if (argument.parameterIndex >= reflectedEntry->getParameterCount())
            fail("HGVM ray argument names an invalid shader parameter");
        auto* parameter = reflectedEntry->getParameterByIndex(argument.parameterIndex);
        requireRayShaderType(
            interpreter,
            argument.typeId,
            parameter->getType(),
            "uniform argument");
        if (!shouldBindParameter(parameter))
        {
            // Specialization can erase an empty closure's uniform storage.
            if (parameter->getCategory() == slang::ParameterCategory::None &&
                !parameter->getTypeLayout()->getSize())
                continue;
            fail("HGVM ray argument names a native varying parameter");
        }
        if (!boundParameters.add(argument.parameterIndex))
            fail("HGVM ray record repeats a shader argument");
        auto cursor = entryCursor.getElement(argument.parameterIndex);
        TypeLayoutObject hostLayout(0), deviceLayout(0);
        hostLayout.domain = HGBCLayoutDomain::Host;
        deviceLayout.domain = HGBCLayoutDomain::ShaderObject;
        deviceLayout.reflectedLayout = cursor.getTypeLayout();
        LayoutConverterObject converter(0);
        converter.sourceLayout = &hostLayout;
        converter.destinationLayout = &deviceLayout;
        std::vector<uint32_t> dereferences, path;
        appendShaderBindingOperations(
            interpreter,
            cursor.getTypeLayout(),
            argument.typeInfoAddress,
            0,
            dereferences,
            path,
            &converter);
        for (const auto& operation : converter.shaderBindingOperations)
        {
            if (operation.mode == ShaderBindingMode::Resource)
                fail("HGVM ray resources require explicit bindless descriptor handles");
            Address source = resolveShaderBindingSourceAddress(
                interpreter,
                argument.valueAddress,
                operation,
                &converter);
            prepareDescriptorResource(
                interpreter,
                source,
                operation,
                pendingBatch.encoder,
                resources);
        }
        bindShaderArgument(interpreter, cursor, {argument.valueAddress, {}}, &converter);
    }
    for (uint32_t p = 0; p < reflectedEntry->getParameterCount(); ++p)
        if (shouldBindParameter(reflectedEntry->getParameterByIndex(p)) &&
            reflectedEntry->getParameterByIndex(p)->getTypeLayout()->getSize() &&
            !boundParameters.contains(p))
            fail("HGVM ray record is missing a required uniform shader argument");
    List<uint8_t> bytes;
    bytes.setCount(entry->getSize());
    if (bytes.getCount())
        std::memcpy(bytes.getBuffer(), entry->getRawData(), size_t(bytes.getCount()));
    return bytes;
}

/// Assembles one tracker, realizes its scenes, and emits a snapshot SBT into the pending batch.
void DeviceState::executeRayDispatch(
    InterpreterState& interpreter,
    const StagedRecord::Header& header)
{
    const auto& record =
        interpreter.readRecord<StagedRecord::DispatchRays>(interpreter.programCounter);
    auto extent = interpreter.resolveAddress<const uint32_t>(record.extentAddress, 12);
    if (!extent.get()[0] || !extent.get()[1] || !extent.get()[2])
    {
        interpreter.advance(header);
        return;
    }
    if (!device || !device->hasFeature(rhi::Feature::RayTracing) ||
        !device->hasFeature(rhi::Feature::Bindless) ||
        (device->getDeviceType() != rhi::DeviceType::Vulkan &&
         device->getDeviceType() != rhi::DeviceType::D3D12))
        fail("HGVM ray dispatch requires Vulkan or D3D12 ray tracing and bindless descriptors");
    RaytracingPipelineObject* tracker = nullptr;
    List<RayAccelerationStructureObject*> scenes;
    discoverRayArguments(interpreter, record.getArguments(), tracker, scenes);
    // A ray-generation shader may do work without tracing; it needs no tracker or SBT rows.
    List<RefPtr<RayShaderRegistration>> misses;
    if (tracker)
        misses = tracker->misses;
    const uint32_t rayTypeCount = uint32_t(misses.getCount());
    for (const auto& miss : misses)
    {
        if (!miss)
            fail("HGVM ray dispatch is missing a miss registration");
        auto arguments = getRayRegistrationArguments(interpreter, miss);
        discoverRayArguments(interpreter, arguments.getArrayView(), tracker, scenes);
    }
    uint32_t geometryCount = 0;
    if (tracker)
        for (const auto& hit : tracker->hits)
        {
            uint64_t end = (hit.first >> 32) + 1;
            if (end > 0x1000000u)
                fail("HGVM ray geometry slots exceed the 24-bit instance contribution domain");
            geometryCount = Math::Max(geometryCount, uint32_t(end));
            auto arguments = getRayRegistrationArguments(interpreter, hit.second);
            discoverRayArguments(interpreter, arguments.getArrayView(), tracker, scenes);
        }
    if (tracker && scenes.getCount())
    {
        uint32_t sceneGeometryCount = scenes[0]->geometryCount;
        for (auto* scene : scenes)
            if (scene->geometryCount != sceneGeometryCount ||
                scene->geometryKinds != scenes[0]->geometryKinds)
                fail("HGVM captured scenes must cover the same geometry slots and kinds");
        if (geometryCount > sceneGeometryCount)
            fail("HGVM ray registration exceeds the captured scene's geometry slots");
        geometryCount = sceneGeometryCount;
    }
    uint64_t hitCount = uint64_t(geometryCount) * rayTypeCount;
    if (hitCount > 0x1000000u)
        fail("HGVM ray table exceeds the 24-bit instance contribution domain");

    RefPtr<RayPipelineObject> candidate = new RayPipelineObject;
    candidate->programInstance = interpreter.programInstance;
    candidate->maxRecursion = record.maxRecursion;
    addRayEntryPoint(interpreter, candidate, record.entryPointAddress, SLANG_STAGE_RAY_GENERATION);
    List<uint32_t> missEntries, hitGroups;
    List<RefPtr<RayShaderRegistration>> orderedHits;
    for (const auto& miss : misses)
        missEntries.add(
            addRayEntryPoint(interpreter, candidate, miss->entryPoints[0], SLANG_STAGE_MISS));
    const SlangStage stages[] = {
        SLANG_STAGE_CLOSEST_HIT,
        SLANG_STAGE_ANY_HIT,
        SLANG_STAGE_INTERSECTION};
    for (uint32_t geometry = 0; geometry < geometryCount; ++geometry)
        for (uint32_t ray = 0; ray < rayTypeCount; ++ray)
        {
            auto* found = tracker->hits.tryGetValue((uint64_t(geometry) << 32) | ray);
            if (!found)
                fail("HGVM ray dispatch is missing a geometry/ray hit registration");
            auto* hit = found->get();
            if (scenes.getCount() && hit->geometryKind != scenes[0]->geometryKinds[geometry])
                fail("HGVM hit-group geometry kind does not match the captured scene");
            if (hit->payloadType != misses[ray]->payloadType)
                fail("HGVM hit and miss registrations disagree on the ray payload type");
            SlangEntryPointObject* sourceEntries[3] = {};
            for (uint32_t role = 0; role < 3; ++role)
                if (hit->entryPoints[role])
                    sourceEntries[role] =
                        interpreter.readConstObject<SlangEntryPointObject>(hit->entryPoints[role]);
            Index groupIndex = 0;
            for (; groupIndex < candidate->hitGroups.getCount(); ++groupIndex)
            {
                const auto& existing = candidate->hitGroups[groupIndex];
                bool matches = existing.geometryKind == hit->geometryKind &&
                               existing.attributeType == hit->attributeType;
                for (uint32_t role = 0; role < 3; ++role)
                {
                    auto entry = existing.entryPoints[role];
                    matches &= (entry < 0 ? nullptr : candidate->entryPoints[entry]) ==
                               sourceEntries[role];
                }
                if (matches)
                    break;
            }
            if (groupIndex == candidate->hitGroups.getCount())
            {
                RayPipelineObject::HitGroup group;
                group.geometryKind = hit->geometryKind;
                group.attributeType = hit->attributeType;
                // A native export belongs to one group layout. Sharing the source shader is
                // fine, but two different groups can require different offsets/local roots.
                // Duplicate records reuse the complete group, including all its exports.
                for (uint32_t role = 0; role < 3; ++role)
                    if (sourceEntries[role])
                    {
                        group.entryPoints[role] = int32_t(candidate->entryPoints.getCount());
                        candidate->entryPoints.add(sourceEntries[role]);
                        candidate->stages.add(stages[role]);
                    }
                candidate->hitGroups.add(group);
            }
            hitGroups.add(uint32_t(groupIndex));
            orderedHits.add(hit);
        }
    RefPtr<RayPipelineObject> pipeline;
    for (const auto& existing : rayPipelines)
        if (hasSameRayPipelineKey(*existing, *candidate))
        {
            pipeline = existing;
            break;
        }
    if (!pipeline)
    {
        createRayPipeline(*this, interpreter, candidate);
        pipeline = candidate;
        rayPipelines.add(pipeline);
    }
    for (uint32_t i = 0; i < rayTypeCount; ++i)
        validateRayRegistrationTypes(interpreter, pipeline, misses[i], missEntries[i]);
    for (Index i = 0; i < orderedHits.getCount(); ++i)
    {
        const auto& group = pipeline->hitGroups[hitGroups[i]];
        for (uint32_t role = 0; role < 3; ++role)
            if (group.entryPoints[role] >= 0)
                validateRayRegistrationTypes(
                    interpreter,
                    pipeline,
                    orderedHits[i],
                    uint32_t(group.entryPoints[role]));
    }
    auto* encoder = getPendingCommandEncoder();
    struct BindingScope
    {
        DeviceState& device;
        ~BindingScope()
        {
            device.rayBindingStride = 1;
            device.rayBindingTracker = nullptr;
        }
    } bindingScope{*this};
    rayBindingTracker = tracker;
    rayBindingStride = rayTypeCount ? rayTypeCount : 1;
    Dictionary<HeapObject*, DescriptorAccess> resources;
    List<List<uint8_t>> recordBytes;
    recordBytes.add(bindRayRecord(
        interpreter,
        pipeline,
        0,
        record.getArguments(),
        HGBCRayStageRole::ClosestHit,
        resources));
    for (uint32_t i = 0; i < rayTypeCount; ++i)
    {
        auto arguments = getRayRegistrationArguments(interpreter, misses[i]);
        recordBytes.add(bindRayRecord(
            interpreter,
            pipeline,
            missEntries[i],
            arguments.getArrayView(),
            HGBCRayStageRole::ClosestHit,
            resources));
    }
    for (Index i = 0; i < orderedHits.getCount(); ++i)
    {
        auto arguments = getRayRegistrationArguments(interpreter, orderedHits[i]);
        rhi::RayTracingShaderRecordLayout layout;
        if (SLANG_FAILED(pipeline->pipeline->getShaderRecordLayout(
                pipeline->groupNames[hitGroups[i]].getBuffer(),
                &layout)))
            fail("HGVM could not query the hit-group shader-record layout");
        List<uint8_t> bytes;
        bytes.setCount(layout.size);
        if (layout.size)
            memset(bytes.getBuffer(), 0, layout.size);
        auto& group = pipeline->hitGroups[hitGroups[i]];
        for (uint32_t role = 0; role < 3; ++role)
            if (group.entryPoints[role] >= 0)
            {
                auto stageBytes = bindRayRecord(
                    interpreter,
                    pipeline,
                    group.entryPoints[role],
                    arguments.getArrayView(),
                    HGBCRayStageRole(role),
                    resources);
                bool found = false;
                for (uint32_t entry = 0; entry < layout.entryPointCount; ++entry)
                {
                    const auto& region = layout.entryPoints[entry];
                    if (region.entryPointIndex != uint32_t(group.entryPoints[role]))
                        continue;
                    SLANG_RELEASE_ASSERT(stageBytes.getCount() == region.size);
                    SLANG_RELEASE_ASSERT(
                        uint64_t(region.offset) + region.size <= layout.size || !region.size);
                    if (region.size)
                        memcpy(
                            bytes.getBuffer() + region.offset,
                            stageBytes.getBuffer(),
                            region.size);
                    found = true;
                    break;
                }
                SLANG_RELEASE_ASSERT(found);
            }
        recordBytes.add(std::move(bytes));
    }
    List<rhi::ShaderRecordData> data;
    for (const auto& bytes : recordBytes)
        data.add({bytes.getBuffer(), uint32_t(bytes.getCount()), 0});
    List<const char*> missNames, hitNames;
    for (uint32_t entry : missEntries)
        missNames.add(pipeline->exportNames[entry].getBuffer());
    for (uint32_t group : hitGroups)
        hitNames.add(pipeline->groupNames[group].getBuffer());
    const char* rayGen = pipeline->exportNames[0].getBuffer();
    rhi::ShaderTableDesc tableDesc;
    tableDesc.program = pipeline->program;
    tableDesc.rayGenShaderCount = 1;
    tableDesc.rayGenShaderEntryPointNames = &rayGen;
    tableDesc.rayGenShaderRecordData = data.getBuffer();
    tableDesc.missShaderCount = uint32_t(missNames.getCount());
    tableDesc.missShaderEntryPointNames = missNames.getBuffer();
    tableDesc.missShaderRecordData = missNames.getCount() ? data.getBuffer() + 1 : nullptr;
    tableDesc.hitGroupCount = uint32_t(hitNames.getCount());
    tableDesc.hitGroupNames = hitNames.getBuffer();
    tableDesc.hitGroupRecordData =
        hitNames.getCount() ? data.getBuffer() + 1 + missNames.getCount() : nullptr;
    ComPtr<rhi::IShaderTable> table;
    if (SLANG_FAILED(device->createShaderTable(tableDesc, table.writeRef())))
        fail("HGVM failed to create a shader binding table");
    // AS realization may have added build operations to the same pending batch.
    encoder = getPendingCommandEncoder();
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        beginPipelineExecution(
            pendingBatch.executions,
            encoder,
            pendingBatch.trackingId,
            pipeline->pipeline.get(),
            TrackedPipelineKind::Raytracing));
    auto* pass = encoder->beginRayTracingPass();
    if (!pass)
        fail("HGVM failed to begin a ray-tracing pass");
    if (!pass->bindPipeline(pipeline->pipeline, table))
    {
        pass->end();
        fail("HGVM failed to bind a ray-tracing pipeline");
    }
    pendingBatch.holds.rayPipelines.add(pipeline->pipeline);
    pendingBatch.holds.shaderTables.add(table);
    pass->dispatchRays(0, extent.get()[0], extent.get()[1], extent.get()[2]);
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        trackDispatch(
            pendingBatch.executions,
            pendingBatch.trackingId,
            pipeline->pipeline.get(),
            extent.get()[0],
            extent.get()[1],
            extent.get()[2]));
    pass->end();
    SLANG_HGVM_TRACK(eventTracker.get(), endDeviceExecution(pendingBatch.executions, encoder));
    finishDeviceOperation();
    interpreter.advance(header);
}
