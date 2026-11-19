//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -compute -output-using-type
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -output-using-type
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -DUSE_UINT2=1
//TEST:SIMPLE(filecheck=WGSL-SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -DUSE_UINT2=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -output-using-type -xslang -DUSE_UINT2=1

// Byte-address loads and stores require only four-byte alignment for eight-byte values. Converting to
// an unsized view must preserve this contract when it omits the byte count.
// WGSL uses uint2 instead of the unsupported uint64_t to exercise the same access at offset four.
#ifdef USE_UINT2
typedef uint2 WideValue;
#else
typedef uint64_t WideValue;
#endif
//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=inputBytes
ByteAddressBuffer inputBytes;
//TEST_INPUT:ubuffer(data=[111 0 0 444], stride=4):name=outputBytes
RWByteAddressBuffer outputBytes;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    WideValue sizedValue = inputBytes.Load<WideValue>(4);
    __UnsizedByteAddressBuffer inputView = inputBytes;
    WideValue unsizedValue = inputView.Load<WideValue>(4);
#ifdef USE_UINT2
    uint2 sizedWords = sizedValue;
    uint2 unsizedWords = unsizedValue;
#else
    uint2 sizedWords = uint2(uint(sizedValue), uint(sizedValue >> 32));
    uint2 unsizedWords = uint2(uint(unsizedValue), uint(unsizedValue >> 32));
#endif
    output[0] = sizedWords.x;
    output[1] = sizedWords.y;
    output[2] = unsizedWords.x;
    output[3] = unsizedWords.y;

    __UnsizedRWByteAddressBuffer outputView = outputBytes;
    outputView.Store<WideValue>(4, sizedValue);
    output[4] = outputBytes.Load(4);
    output[5] = outputBytes.Load(8);
    output[6] = outputBytes.Load(0);
    output[7] = outputBytes.Load(12);
}

// CHECK: 2
// CHECK: 3
// CHECK: 2
// CHECK: 3
// CHECK: 2
// CHECK: 3
// CHECK: 111
// CHECK: 444

// Metal must split both sized and unsized loads into words at offsets 4 and 8, and split
// the unsized store the same way instead of forming an unaligned ulong pointer.
// METAL: ulong {{.*}} = ulong(as_type<uint>({{.*}}[(4U)>>2])) | (ulong(as_type<uint>({{.*}}[(8U)>>2])) << 32ULL);
// METAL: ulong {{.*}} = ulong(as_type<uint>({{.*}}[(4U)>>2])) | (ulong(as_type<uint>({{.*}}[(8U)>>2])) << 32ULL);
// METAL: {{.*}}[(4U)>>2] = as_type<uint32_t>(
// METAL: {{.*}}[(8U)>>2] = as_type<uint32_t>(

// WGSL: var<storage, read> [[INPUT:inputBytes_[0-9]+]] : array<u32>;
// WGSL: var<storage, read_write> [[BYTES:outputBytes_[0-9]+]] : array<u32>;
// WGSL: [[INPUT]][(u32(4))/4];
// WGSL-NEXT: {{.*}}[[INPUT]][(u32(8))/4];
// WGSL-NEXT: {{.*}}[[INPUT]][(u32(4))/4];
// WGSL-NEXT: {{.*}}[[INPUT]][(u32(8))/4];
// WGSL: [[BYTES]][(u32(4))/4] =
// WGSL-NEXT: [[BYTES]][(u32(8))/4] =
// WGSL: [[BYTES]][(u32(0))/4];
// WGSL-NEXT: {{.*}}[[BYTES]][(u32(12))/4];

// WGSL-SPIRV: OpEntryPoint GLCompute
// WGSL-SPIRV: OpLoad
// WGSL-SPIRV: OpStore
