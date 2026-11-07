//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL_EACH): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL_EACH): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type

// `Load<bool>` / `Store<bool>` (and bool vectors) on a byte-address buffer were lowered to a
// bit cast between `bool` and `uint`, which Metal (`as_type<bool>(uint)`) and WGSL
// (`bitcast<bool>(u32)`) both reject. The word is converted instead.

// The `bool2` load reads two different values, so swapped components show up, and every
// store changes the word it writes (4: 1 -> 0, 5: 1 -> 0, 6: 0 -> 1, 7: 1 -> 0).
//TEST_INPUT:ubuffer(data=[1 1 0 1 1 1 0 1], stride=4):out,name=buffer
RWByteAddressBuffer buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // METAL-NOT: as_type<bool
    // METAL-NOT: as_type<uint32_t>(!
    // WGSL-NOT: bitcast<bool
    // WGSL-NOT: bitcast<vec2<bool
    bool b = buffer.Load<bool>(0);
    bool2 v = buffer.Load<bool2>(4);
    buffer.Store<bool>(16, !b);
    buffer.Store<bool2>(20, !v);
    buffer.Store<bool>(28, b && v.y);
    // METALLIB: @computeMain
}

// Every load and store, keyed by byte offset, converts the word instead of bit casting it. The
// `bool2` load and store are scalarized, so each of their words has its own check.
// METAL_EACH-DAG: bool({{.*}}[(0U)>>2])
// METAL_EACH-DAG: bool({{.*}}[(4U)>>2])
// METAL_EACH-DAG: bool({{.*}}[(8U)>>2])
// METAL_EACH-DAG: [(16U)>>2] = uint32_t(
// METAL_EACH-DAG: [(20U)>>2] = uint32_t(
// METAL_EACH-DAG: [(24U)>>2] = uint32_t(
// METAL_EACH-DAG: [(28U)>>2] = uint32_t(
// WGSL_EACH-DAG: bool({{.*}}[(u32(0))/4])
// WGSL_EACH-DAG: bool({{.*}}[(u32(4))/4])
// WGSL_EACH-DAG: bool({{.*}}[(u32(8))/4])
// WGSL_EACH-DAG: [(u32(16))/4] = select(u32(0), u32(1),
// WGSL_EACH-DAG: [(u32(20))/4] = select(u32(0), u32(1),
// WGSL_EACH-DAG: [(u32(24))/4] = select(u32(0), u32(1),
// WGSL_EACH-DAG: [(u32(28))/4] = select(u32(0), u32(1),

// BUF: {{^}}1{{$}}
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}0{{$}}
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}0{{$}}
// BUF-NEXT: {{^}}0{{$}}
// BUF-NEXT: {{^}}1{{$}}
// BUF-NEXT: {{^}}0{{$}}
