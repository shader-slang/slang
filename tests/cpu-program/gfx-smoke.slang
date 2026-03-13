//TEST:EXECUTABLE:
import gfx;
import slang;

gfx.IPipelineState createPipeline(gfx.IDevice device, NativeString entryPointName)
{
    gfx.ShaderProgramDesc2 programDesc = {};
    NativeString filename = "tests/cpu-program/saxpy.slang";
    programDesc.sourceData = filename;
    programDesc.sourceType = gfx.ShaderModuleSourceType.SlangSourceFile;
    programDesc.sourceDataSize = filename.length;
    programDesc.entryPointCount = 1;
    programDesc.entryPointNames = &entryPointName;
    Optional<gfx.IShaderProgram> program;
    Optional<slang.ISlangBlob> diagBlob;
    device.createProgram2(&programDesc, program, diagBlob);

    Optional<gfx.IPipelineState> pipeline;
    gfx.ComputePipelineStateDesc pipelineDesc;
    pipelineDesc.program = NativeRef<gfx.IShaderProgram>(program.value);
    device.createComputePipelineState(&pipelineDesc, pipeline);
    return pipeline.value;
}

gfx.IBufferResource createBuffer(gfx.IDevice device, Ptr<void> data)
{
    Optional<gfx.IBufferResource> buffer;
    gfx.BufferResourceDesc bufferDesc = {};
    bufferDesc.memoryType = gfx.MemoryType.DeviceLocal;
    bufferDesc.allowedStates.add(gfx.ResourceState.UnorderedAccess);
    bufferDesc.defaultState = gfx.ResourceState.UnorderedAccess;
    bufferDesc.elementSize = 4;
    bufferDesc.sizeInBytes = 256;
    bufferDesc.type = gfx.ResourceType.Buffer;
    device.createBufferResource(&bufferDesc, data, buffer);
    return buffer.value;
}

gfx.IResourceView createView(gfx.IDevice device, gfx.IBufferResource buffer)
{
    Optional<gfx.IResourceView> bufferView;
    gfx.ResourceViewDesc viewDesc;
    viewDesc.type = gfx.ResourceViewType.UnorderedAccess;
    device.createBufferView(buffer, none, &viewDesc, bufferView);
    return bufferView.value;
}

export __extern_cpp int main()
{
    gfx.DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = gfx.DeviceType.CPU;
    Optional<gfx.IDevice> device;
    gfx.gfxCreateDevice(&deviceDesc, device);
    if (device == none)
    {
        printf("fail\n");
        return -1;
    }
    
    gfx.CommandQueueDesc queueDesc = {gfx::QueueType::Graphics};
    queueDesc.type = gfx.QueueType.Graphics;
    Optional<gfx.ICommandQueue> queue;
    device.value.createCommandQueue(&queueDesc, queue);

    gfx.IPipelineState pipelineTimes = createPipeline(device.value, "times");
    gfx.IPipelineState pipelinePlus = createPipeline(device.value, "plus");

    Optional<gfx.ITransientResourceHeap> transientHeap;
    gfx.TransientResourceHeapDesc transientHeapDesc;
    transientHeapDesc.constantBufferDescriptorCount = 64;
    transientHeapDesc.constantBufferSize = 1024;
    transientHeapDesc.srvDescriptorCount = 1024;
    transientHeapDesc.uavDescriptorCount = 1024;
    transientHeapDesc.samplerDescriptorCount = 256;
    transientHeapDesc.accelerationStructureDescriptorCount = 32;
    device.value.createTransientResourceHeap(&transientHeapDesc, transientHeap);

    float4 dataX = { 2, 3, 5, 7 };
    gfx.IBufferResource bufferX = createBuffer(device.value, (void*) &dataX);
    gfx.IResourceView bufferViewX = createView(device.value, bufferX);

    float4 dataY = { 11, 13, 17, 19 };
    gfx.IBufferResource bufferY = createBuffer(device.value, (void*) &dataY);
    gfx.IResourceView bufferViewY = createView(device.value, bufferY);

    gfx.IBufferResource bufferAX = createBuffer(device.value, nullptr);
    gfx.IResourceView bufferViewAX = createView(device.value, bufferAX);

    gfx.IBufferResource bufferAXPY = createBuffer(device.value, nullptr);
    gfx.IResourceView bufferViewAXPY = createView(device.value, bufferAXPY);

    Optional<gfx.ICommandBuffer> commandBuffer;
    transientHeap.value.createCommandBuffer(commandBuffer);
    Optional<gfx.IComputeCommandEncoder> encoder;
    commandBuffer.value.encodeComputeCommands(encoder);

    Optional<gfx.IShaderObject> rootTimes;
    encoder.value.bindPipeline(pipelineTimes, rootTimes);
    Optional<gfx.IShaderObject> entryPointTimes;
    rootTimes.value.getEntryPoint(0, entryPointTimes);
    gfx.ShaderOffset offsetTimesX = {};
    offsetTimesX.uniformOffset = 1;
    gfx.ShaderOffset offsetTimesAX = {};
    offsetTimesAX.uniformOffset = 2;
    entryPointTimes.value.setResource(&offsetTimesX, bufferViewX);
    entryPointTimes.value.setResource(&offsetTimesAX, bufferViewAX);
    encoder.value.dispatchCompute(1, 1, 1);

    Optional<gfx.IShaderObject> rootPlus;
    encoder.value.bindPipeline(pipelinePlus, rootPlus);
    Optional<gfx.IShaderObject> entryPointPlus;
    rootPlus.value.getEntryPoint(0, entryPointPlus);
    gfx.ShaderOffset offsetPlusAX = {};
    gfx.ShaderOffset offsetPlusY = {};
    offsetPlusY.uniformOffset = 1;
    gfx.ShaderOffset offsetPlusAXPY = {};
    offsetPlusAXPY.uniformOffset = 2;
    entryPointPlus.value.setResource(&offsetPlusAX, bufferViewAX);
    entryPointPlus.value.setResource(&offsetPlusY, bufferViewY);
    entryPointPlus.value.setResource(&offsetPlusAXPY, bufferViewAXPY);
    encoder.value.dispatchCompute(1, 1, 1);

    encoder.value.endEncoding();
    commandBuffer.value.close();
    
    NativeRef<gfx.ICommandBuffer> commandBufferRef = NativeRef<gfx.ICommandBuffer>(commandBuffer.value);
    queue.value.executeCommandBuffers(1, &commandBufferRef, none, 0);
    queue.value.waitOnHost();

    Optional<slang.ISlangBlob> blob;
    device.value.readBufferResource(bufferAXPY, 0, 16, blob);

    for (int i = 0; i < 4; i++)
    {
        float val = ((float *)blob.value.getBufferPointer())[i];
        printf("%.1f\n", val);
    }
    return 0;
}
