//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

__target_intrinsic(hgbc, "OpWindowType")
struct Window {}

typealias TestSurface = Surface<43>;
typealias TestSurfaceTarget = RenderTarget2D<float4, 43>;

extension<T, let format : int> RenderTarget2D<T, format>
{
    void GetDimensions(out uint width, out uint height)
    {
        uint2 dimensions = hgbc_asm {
            result : $$uint2 = OpTextureGetDimensions $this;
        };
        width = dimensions.x;
        height = dimensions.y;
    }
}

Window createWindow(String title, uint width, uint height)
{
    return hgbc_asm {
        result : $$Window = OpCreateWindow $title $width $height;
    };
}

bool processWindowEvents(Window window)
{
    return hgbc_asm {
        result : $$bool = OpProcessWindowEvents $window;
    };
}

enum InputEventType : uint
{
    none = 0,
    keyPress = 1,
    keyRelease = 2,
    mouseButtonDown = 3,
    mouseButtonUp = 4,
    mouseMove = 5,
    mouseScroll = 6,
    windowResize = 7,
}

enum KeyCode : uint
{
    unknown = 0,
    r = 82,
    s = 83,
    w = 87,
    escape = 256,
}

enum MouseButton : uint
{
    left = 0,
    unknown = 3,
}

struct InputEvent
{
    InputEventType type;
    KeyCode key;
    MouseButton button;
    float2 pos;
    float2 scroll;
    uint2 size;
}

InputEvent pollInputEvent(Window window)
{
    return hgbc_asm {
        result : $$InputEvent = OpPollInputEvent $window;
    };
}

void closeWindow(Window window)
{
    hgbc_asm {
        OpCloseWindow $window;
    };
}

TestSurface createSurface(Window window, uint imageCount)
{
    return hgbc_asm {
        result : $$TestSurface = OpCreateSurface $window $imageCount;
    };
}

TestSurfaceTarget acquireSurfaceTexture(TestSurface surface)
{
    return hgbc_asm {
        result : $$TestSurfaceTarget = OpAcquireSurfaceTexture $surface;
    };
}

void presentSurface(TestSurface surface)
{
    hgbc_asm {
        OpPresentSurface $surface;
    };
}

RWTexture2D<T> allocateTexture2D<T : ITexelElement>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm {
        result : $$RWTexture2D<T> = OpAllocateResource
            Texture Texture2D $__getHGBCNaturalTextureFormat<T>() DeviceLocal UnorderedAccess $$T
            $width $height $one 1 1 0
            ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
    };
}

void blitTexture2D<TDestination : ITexelElement, TSource : ITexelElement>(
    RWTexture2D<TDestination> destination,
    RWTexture2D<TSource> source)
{
    hgbc_asm {
        OpBlitTexture2D $$RWTexture2D<TDestination> $$RWTexture2D<TSource>
            $destination $source;
    };
}

void blitTexture2D<TSource : ITexelElement>(TestSurfaceTarget destination,
                                            RWTexture2D<TSource> source)
{
    hgbc_asm {
        OpBlitTexture2D $$TestSurfaceTarget $$RWTexture2D<TSource> $destination $source;
    };
}

__target_intrinsic(hgbc, "OpByteStreamType")
struct ByteStream {}

ByteStream openByteStream(String path, uint mode)
{
    if (mode == 0)
    {
        return hgbc_asm {
            result : $$ByteStream = OpByteStreamFromFile Read $path;
        };
    }
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Write $path;
    };
}

void closeByteStream(ByteStream stream)
{
    hgbc_asm {
        OpCloseByteStream $stream;
    };
}

uint readByteStreamTextureEXR(ByteStream stream, RWTexture2D<float4> destination)
{
    return hgbc_asm {
        result : $$uint = OpImageDecode
            EXR ByteStream DeviceTexture2D $stream $destination;
    };
}

uint writeByteStreamTextureEXR(ByteStream stream, RWTexture2D<float4> source)
{
    return hgbc_asm {
        result : $$uint = OpImageEncode
            EXR DeviceTexture2D ByteStream $source $stream;
    };
}

uint readValue<T>(ByteStream stream, out T destination)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            HostAddress $stream &destination $byteOffset $byteCount;
    };
}

uint writeValue<T>(ByteStream stream, __constref T source)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            HostAddress $stream &source $byteOffset $byteCount;
    };
}

[host]
void main()
{
    Window window = createWindow("HGBC window", 640, 480);
    bool open = processWindowEvents(window);
    InputEvent inputEvent = pollInputEvent(window);
    if (inputEvent.type == InputEventType.keyPress && inputEvent.key == KeyCode.escape)
        closeWindow(window);
    TestSurface surface = createSurface(window, 2);
    TestSurfaceTarget target = acquireSurfaceTexture(surface);
    RWTexture2D<float4> intermediate = allocateTexture2D<float4>(640, 480);
    RWTexture2D<float4> loaded = allocateTexture2D<float4>(640, 480);
    ByteStream imageInput = openByteStream("reference.exr", 0);
    uint imageBytesRead = readByteStreamTextureEXR(imageInput, loaded);
    closeByteStream(imageInput);
    blitTexture2D(target, intermediate);
    ByteStream imageOutput = openByteStream("actual.exr", 1);
    uint imageBytesWritten = writeByteStreamTextureEXR(imageOutput, loaded);
    closeByteStream(imageOutput);
    uint width;
    uint height;
    target.GetDimensions(width, height);
    presentSurface(surface);

    ByteStream input = openByteStream("data.bin", 0);
    uint count;
    uint countBytes = readValue(input, count);
    RWStructuredBuffer<float2> buffer = RWStructuredBuffer<float2>::alloc(count);
    uint byteOffset = 0;
    uint byteCount = count * sizeof(float2);
    uint bytesRead = hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceBuffer $input $buffer $byteOffset $byteCount;
    };
    closeByteStream(input);
    ByteStream output = openByteStream("copy.bin", 1);
    uint bytesWritten = writeValue(output, count);
    uint payloadBytes = hgbc_asm {
        result : $$uint = OpByteStreamWrite
            DeviceBuffer $output $buffer $byteOffset $byteCount;
    };
    closeByteStream(output);
    if (open)
    {
        print(width);
        print(height);
    }
}

// CHECK-DAG: [[WINDOW:%[A-Za-z0-9_]+]] = OpWindowType
// CHECK-DAG: [[SURFACE:%[A-Za-z0-9_]+]] = OpSurfaceType RGBA8UnormSrgb
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpRenderTarget2DType {{%[A-Za-z0-9_]+}} RGBA8UnormSrgb
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[WINDOW]] = OpCreateWindow
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpProcessWindowEvents {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpPollInputEvent {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpCloseWindow {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[SURFACE]] = OpCreateSurface
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpAcquireSurfaceTexture
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpAllocateResource Texture Texture2D RGBA32Float DeviceLocal UnorderedAccess
// CHECK-DAG: OpBlitTexture2D
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpImageDecode EXR ByteStream DeviceTexture2D
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpImageEncode EXR DeviceTexture2D ByteStream
// CHECK-DAG: OpPresentSurface
// CHECK-DAG: OpByteStreamType
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpByteStreamFromFile
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpByteStreamRead HostAddress
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpByteStreamRead DeviceBuffer
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpByteStreamWrite
// CHECK-DAG: OpCloseByteStream
