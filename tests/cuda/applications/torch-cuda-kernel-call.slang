// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Reduced from SlangPy's test_tensorview and test_difftensorview CUDAKernel wrappers.
// Consider a generated compute entry that calls a user's [CUDAKernel] function as a helper.
// The caller supplies the dispatch; the function reads that dispatch's thread coordinates.
// NVVM rejects the decorated callee before emission. Removing the annotation compiles, so the
// ordinary helper below is a control for the same body and TensorView signature.
// Desired execution: with count active elements, output[i] becomes i for 0 <= i < count.
// This manifest-driven reproducer is not a negative test accepting that rejection as correct.

[CUDAKernel]
void fillThreadIDs(uint count, TensorView<int> output)
{
    uint tid = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    if (tid < count)
        output.store(tid, int(tid));
}

void fillThreadIDsHelper(uint count, TensorView<int> output)
{
    uint tid = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    if (tid < count)
        output.store(tid, int(tid));
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uniform uint count, uniform TensorView<int> output)
{
    fillThreadIDs(count, output);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void helperControl(uniform uint count, uniform TensorView<int> output)
{
    fillThreadIDsHelper(count, output);
}
