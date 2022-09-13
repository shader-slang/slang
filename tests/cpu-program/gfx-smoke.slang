//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

import gfx;
import slang;
extension Ptr<void>
{
    __intrinsic_op(0)
    __implicit_conversion(200)
    __init(NativeString nativeStr);

    __generic<T>
    __intrinsic_op(0)
    __implicit_conversion(200)
    __init(T* ptr);

    __generic<T>
    __intrinsic_op(0)
    __implicit_conversion(200)
    __init(NativeRef<T> ptr);
}
public __extern_cpp int main()
{
    gfx.DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = gfx.DeviceType.CPU;
    Optional<gfx.IDevice> device;
    gfx.gfxCreateDevice(&deviceDesc, device);
    if (device == none)
    {
        writeln("fail");
        return -1;
    }
    
    gfx.CommandQueueDesc queueDesc = {};
    queueDesc.type = gfx.QueueType.Graphics;
    Optional<gfx.ICommandQueue> queue;
    device.value.createCommandQueue(&queueDesc, queue);

    gfx.ShaderProgramDesc2 programDesc = {};
    NativeString s =
        "[shader(\"compute\")]"
        "[numthreads(4, 1, 1)]"
        "void computeMain("
        "    uint3 sv_dispatchThreadID: SV_DispatchThreadID,"
        "    uniform RWStructuredBuffer<float> buffer"
        "    )"
        "{"
        "    var input = buffer[sv_dispatchThreadID.x];"
        "    buffer[sv_dispatchThreadID.x] = sv_dispatchThreadID.x;"
        "}";
    programDesc.sourceData = s;
    programDesc.sourceType = gfx.ShaderModuleSourceType.SlangSource;
    programDesc.sourceDataSize = s.length;
    programDesc.entryPointCount = 1;
    NativeString entryPointName = "computeMain";
    programDesc.entryPointNames = &entryPointName;
    Optional<gfx.IShaderProgram> program;
    Optional<slang.ISlangBlob> diagBlob;
    device.value.createProgram2(&programDesc, program, diagBlob);

    Optional<gfx.IPipelineState> pipeline;
    gfx.ComputePipelineStateDesc pipelineDesc = {};
    pipelineDesc.program = NativeRef<gfx.IShaderProgram>(program.value);
    device.value.createComputePipelineState(&pipelineDesc, pipeline);

    Optional<gfx.ITransientResourceHeap> transientHeap;
    gfx.TransientResourceHeapDesc transientHeapDesc = {};
    transientHeapDesc.constantBufferDescriptorCount = 64;
    transientHeapDesc.constantBufferSize = 1024;
    transientHeapDesc.srvDescriptorCount = 1024;
    transientHeapDesc.uavDescriptorCount = 1024;
    transientHeapDesc.samplerDescriptorCount = 256;
    transientHeapDesc.accelerationStructureDescriptorCount = 32;
    device.value.createTransientResourceHeap(&transientHeapDesc, transientHeap);

    Optional<gfx.IBufferResource> buffer;
    gfx.BufferResourceDesc bufferDesc = {};
    bufferDesc.memoryType = gfx.MemoryType.DeviceLocal;
    bufferDesc.allowedStates.add(gfx.ResourceState.UnorderedAccess);
    bufferDesc.defaultState = gfx.ResourceState.UnorderedAccess;
    bufferDesc.elementSize = 4;
    bufferDesc.sizeInBytes = 256;
    bufferDesc.type = gfx.ResourceType.Buffer;
    device.value.createBufferResource(&bufferDesc, nullptr, buffer);

    Optional<gfx.IResourceView> bufferView;
    gfx.ResourceViewDesc viewDesc = {};
    viewDesc.type = gfx.ResourceViewType.UnorderedAccess;
    device.value.createBufferView(buffer.value, none, &viewDesc, bufferView);

    Optional<gfx.ICommandBuffer> commandBuffer;
    transientHeap.value.createCommandBuffer(commandBuffer);
    Optional<gfx.IComputeCommandEncoder> encoder;
    commandBuffer.value.encodeComputeCommands(encoder);
    Optional<gfx.IShaderObject> rootObject;
    encoder.value.bindPipeline(pipeline.value, rootObject);
    Optional<gfx.IShaderObject> entryPointObject;
    rootObject.value.getEntryPoint(0, entryPointObject);
    gfx.ShaderOffset offset = {};
    entryPointObject.value.setResource(&offset, bufferView.value);
    encoder.value.dispatchCompute(1, 1, 1);
    encoder.value.endEncoding();

    NativeRef<gfx.ICommandBuffer> commandBufferRef = NativeRef<gfx.ICommandBuffer>(commandBuffer.value);
    queue.value.executeCommandBuffers(1, &commandBufferRef, none, 0);
    queue.value.waitOnHost();

    Optional<slang.ISlangBlob> blob;
    device.value.readBufferResource(buffer.value, 0, 16, blob);

    for (int i = 0; i < 4; i++)
    {
        float val = ((float *)blob.value.getBufferPointer())[i];
        writeln(String(val));
    }
    return 0;
}