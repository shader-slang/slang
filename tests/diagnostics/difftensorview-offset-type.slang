//TEST:SIMPLE(filecheck=CHECK):

void mismatchedGradient(DiffTensorView<float, uint64_t, AtomicAdd> view)
{
}

// CHECK: type argument 'AtomicAdd<uint>' does not conform
// CHECK: IDiffTensorWrapper<uint64_t>
