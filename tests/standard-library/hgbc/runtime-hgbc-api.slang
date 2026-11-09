// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.io.byte_stream;
import slang.runtime.io.image;
import slang.runtime.core.simple_assert;
import slang.runtime.io.simple_file;
import slang.runtime.window.simple_input;
import slang.runtime.core.simple_string;
import slang.runtime.resource.simple_texture;
import slang.runtime.core.simple_time;
import slang.runtime.window.simple_window;
import slang.runtime.resource.upload_buffer;

/// Instantiates every Slang wrapper around the HGBC runtime opcodes.
///
/// This test only compiles the graph. For example, the file and window names below are never
/// opened by `slang-test`; they make changes to an opcode's operands fail in this demo-local test
/// instead of leaving an uninstantiated generic wrapper stale.
[host]
void main()
{
    String title = format("HGVM runtime API {}", 1);
    Timer timer = createTimer();
    float elapsed = elapsedSeconds(timer);
    resetTimer(timer);

    Window window = createWindow(title, 16, 16);
    setTitle(window, title);
    bool isOpen = processWindowEvents(window);
    InputEvent inputEvent = pollInputEvent(window);
    DefaultSurface surface = createSurface(window);
    DefaultSurfaceTarget recoveredSurfaceTexture;
    if (resizeSurface(surface, 32, 32) &&
        tryAcquireSurfaceTexture(surface, recoveredSurfaceTexture))
        print(tryPresentSurface(surface));
    DefaultSurfaceTarget surfaceTexture = acquireSurfaceTexture(surface);

    RWTexture2D<float4> image = allocateTexture2D<float4>(16, 16);
    RWTexture2D<float4> copy = RWTexture2D<float4>::alloc(16, 16);
    DepthTarget2D depth = allocateDepthTarget2D(16, 16);
    blitTexture2D(surfaceTexture, image);

    UploadBuffer<uint> upload = UploadBuffer<uint>::allocate(4);
    upload[0] = 42;
    RWStructuredBuffer<uint> readWriteBuffer = RWStructuredBuffer<uint>::alloc(4);
    StructuredBuffer<uint> readOnlyBuffer = StructuredBuffer<uint>::alloc(4);
    upload.uploadTo(readWriteBuffer);
    upload.uploadTo(readOnlyBuffer);

    ByteStream input = openByteStream("input.bin", ByteStreamMode::read);
    uint streamSize = getByteStreamSize(input);
    uint firstByte = readByte(input);
    uint integerValue = input.read<uint>();
    float3 floatValue = input.read<float3>();
    uint bytesRead = readByteStreamBuffer(input, readWriteBuffer);
    bytesRead += readByteStreamBuffer(input, readOnlyBuffer);
    bytesRead += decodeImageEXR(input, image);
    closeByteStream(input);

    ByteStream output = openByteStream("output.bin", ByteStreamMode::write);
    uint bytesWritten = writeByteStream(output, integerValue);
    bytesWritten += writeByteStreamBuffer(output, readWriteBuffer);
    bytesWritten += writeByteStreamBuffer(output, readOnlyBuffer);
    bytesWritten += encodeImageEXR(image, output);
    bytesWritten += encodeImagePNG(copy, output);
    closeByteStream(output);

    RWStructuredBuffer<uint> loadedReadWrite = loadBuffer<uint>("read-write.bin");
    StructuredBuffer<uint> loadedReadOnly = StructuredBuffer<uint>::loadBuffer("read-only.bin");
    bytesWritten += storeBuffer("stored-read-write.bin", loadedReadWrite);
    bytesWritten += storeBuffer("stored-read-only.bin", loadedReadOnly);

    uint titleByteCount = getStringByteCount(title);
    uint titleByte = readStringByte(title, 0);
    if (!isOpen || inputEvent.type != InputEventType::none || elapsed < 0.0 ||
        streamSize + firstByte + bytesRead + bytesWritten + titleByteCount + titleByte == 0)
    {
        fail("HGVM runtime API smoke test failed");
    }

    presentSurface(surface);
    closeWindow(window);
}

// CHECK-DAG: OpAllocateResource
// CHECK-DAG: OpUploadBufferStore
// CHECK-DAG: OpUploadBufferUploadTo
// CHECK-DAG: OpByteStreamFromFile
// CHECK-DAG: OpByteStreamGetSize
// CHECK-DAG: OpByteStreamRead HostAddress
// CHECK-DAG: OpByteStreamRead DeviceBuffer
// CHECK-DAG: OpByteStreamWrite HostAddress
// CHECK-DAG: OpByteStreamWrite DeviceBuffer
// CHECK-DAG: OpCloseByteStream
// CHECK-DAG: OpImageDecode EXR ByteStream DeviceTexture2D
// CHECK-DAG: OpImageEncode EXR DeviceTexture2D ByteStream
// CHECK-DAG: OpImageEncode PNG DeviceTexture2D ByteStream
// CHECK-DAG: OpBlitTexture2D
// CHECK-DAG: OpFormatString
// CHECK-DAG: OpStringGetByteCount
// CHECK-DAG: OpStringReadByte
// CHECK-DAG: OpCreateTimer
// CHECK-DAG: OpTimerElapsed
// CHECK-DAG: OpResetTimer
// CHECK-DAG: OpCreateWindow
// CHECK-DAG: OpProcessWindowEvents
// CHECK-DAG: OpSetWindowTitle
// CHECK-DAG: OpPollInputEvent
// CHECK-DAG: OpCreateSurface
// CHECK-DAG: OpAcquireSurfaceTexture
// CHECK-DAG: OpPresentSurface
// CHECK-DAG: OpResizeSurface
// CHECK-DAG: OpTryAcquireSurfaceTexture
// CHECK-DAG: OpTryPresentSurface
// CHECK-DAG: OpCloseWindow
// CHECK-DAG: OpFail
