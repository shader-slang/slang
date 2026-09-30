//TEST:SIMPLE(filecheck=CHECK):

void invalidOffsets(TensorView<float, float> floating, TensorView<float, int> signedOffset)
{
}

// CHECK: type argument 'float' does not conform to the required interface '__TensorViewOffsetType'
// CHECK: type argument 'int' does not conform to the required interface '__TensorViewOffsetType'
