//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

// Exercise the CUDA prelude directly so that every native TensorView overload is covered,
// including the generic array overloads. Rebase each view using integer address arithmetic
// so that its tested coordinate points at one real buffer element. This tests offsets beyond
// 4 GiB without requiring a large allocation. Check addresses before dereferencing them.
uint testTensorViewOffsets(RWStructuredBuffer<uint> buffer)
{
    __requirePrelude(R"(
static_assert(sizeof(TensorView) == 56, "TensorView size must preserve the host ABI");
static_assert(alignof(TensorView) == 8, "TensorView alignment must preserve the host ABI");
static_assert(sizeof(((TensorView*)0)->strides[0]) == 4, "Strides must remain 32-bit");
static_assert(sizeof(((TensorView*)0)->sizes[0]) == 4, "Sizes must remain 32-bit");

template<unsigned int N>
__device__ bool testTensorViewAccess(uint32_t* target, uint32_t stride, uint32_t index)
{
    TensorView view = {};
    uint indices[N];
    for (unsigned int i = 0; i < N; ++i)
    {
        view.strides[i] = stride;
        view.sizes[i] = index + 1;
        indices[i] = index;
    }
    view.dimensionCount = N;
    uint64_t offset = uint64_t(stride) * index * N;
    view.data = reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(target) - offset);

    // The generic overload must widen every product, not just its accumulator.
    if (view.data_ptr_at<uint32_t, N>(indices) != target ||
        &view.load<uint32_t, N>(indices) != target)
        return false;
    *target = 11;
    if (view.load<uint32_t, N>(indices) != 11)
        return false;
    view.store<uint32_t, N>(indices, 13);
    if (*target != 13)
        return false;
    *target = 11;

    if constexpr (N == 1)
    {
        if (view.data_ptr_at<uint32_t>(index) != target ||
            &view.load<uint32_t>(index) != target)
            return false;
        if (view.load<uint32_t>(index) != 11)
            return false;
        view.store<uint32_t>(index, 19);
    }
    else if constexpr (N == 2)
    {
        uint2 coord = make_uint2(index, index);
        if (view.data_ptr_at<uint32_t>(coord) != target ||
            &view.load<uint32_t>(coord) != target ||
            &view.load<uint32_t>(index, index) != target)
            return false;
        if (view.load<uint32_t>(coord) != 11 || view.load<uint32_t>(index, index) != 11)
            return false;
        view.store<uint32_t>(index, index, 17);
        if (*target != 17)
            return false;
        view.store<uint32_t>(coord, 19);
    }
    else if constexpr (N == 3)
    {
        uint3 coord = make_uint3(index, index, index);
        if (view.data_ptr_at<uint32_t>(coord) != target ||
            &view.load<uint32_t>(coord) != target ||
            &view.load<uint32_t>(index, index, index) != target)
            return false;
        if (view.load<uint32_t>(coord) != 11 || view.load<uint32_t>(index, index, index) != 11)
            return false;
        view.store<uint32_t>(index, index, index, 17);
        if (*target != 17)
            return false;
        view.store<uint32_t>(coord, 19);
    }
    else if constexpr (N == 4)
    {
        uint4 coord = make_uint4(index, index, index, index);
        if (view.data_ptr_at<uint32_t>(coord) != target ||
            &view.load<uint32_t>(coord) != target ||
            &view.load<uint32_t>(index, index, index, index) != target)
            return false;
        if (view.load<uint32_t>(coord) != 11 ||
            view.load<uint32_t>(index, index, index, index) != 11)
            return false;
        view.store<uint32_t>(index, index, index, index, 17);
        if (*target != 17)
            return false;
        view.store<uint32_t>(coord, 19);
    }
    else if constexpr (N == 5)
    {
        if (&view.load<uint32_t>(index, index, index, index, index) != target)
            return false;
        if (view.load<uint32_t>(index, index, index, index, index) != 11)
            return false;
        view.store<uint32_t>(index, index, index, index, index, 19);
    }
    if (*target != 19)
        return false;

    // Slang's TensorView atomics use data_ptr_at too.
    return atomicAdd(view.data_ptr_at<uint32_t, N>(indices), 1) == 19 && *target == 20;
}

template<unsigned int N>
__device__ bool testTensorViewDimension(uint32_t* target)
{
    return testTensorViewAccess<N>(target, 4, 1) &&
        testTensorViewAccess<N>(target, 4, (1u << 30) - 1) &&
        testTensorViewAccess<N>(target, 4, 1u << 30) &&
        testTensorViewAccess<N>(target, 4, (1u << 30) + 1) &&
        // Here each product fits 32 bits, but their sum exceeds it for N >= 2.
        testTensorViewAccess<N>(target, 1u << 31, 1);
}

__device__ uint32_t testTensorViewOffsetsImpl(uint32_t* target)
{
    // Pin field offsets as well as the size and alignment checked above.
    TensorView view = {};
    if (reinterpret_cast<uint8_t*>(&view.data) - reinterpret_cast<uint8_t*>(&view) != 0 ||
        reinterpret_cast<uint8_t*>(&view.strides) - reinterpret_cast<uint8_t*>(&view) != 8 ||
        reinterpret_cast<uint8_t*>(&view.sizes) - reinterpret_cast<uint8_t*>(&view) != 28 ||
        reinterpret_cast<uint8_t*>(&view.dimensionCount) - reinterpret_cast<uint8_t*>(&view) != 48)
        return 0;
    return testTensorViewDimension<1>(target) && testTensorViewDimension<2>(target) &&
        testTensorViewDimension<3>(target) && testTensorViewDimension<4>(target) &&
        testTensorViewDimension<5>(target);
}
)");
    __intrinsic_asm "testTensorViewOffsetsImpl($0.data + 1)";
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = testTensorViewOffsets(outputBuffer);
}

// CHECK: 1
// CHECK: 20
