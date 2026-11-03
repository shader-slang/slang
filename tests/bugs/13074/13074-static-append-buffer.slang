// Append- and consume-buffer lowering can split one declared variable into separate buffer and
// counter values, with details that depend on the target. Because resource-global legalization
// cannot yet replace one file- or namespace-scope `static` resource with several values, semantic
// checking rejects these declarations before lowering.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

AppendStructuredBuffer<float> outputBuffer;
static AppendStructuredBuffer<float> cachedOutputBuffer = outputBuffer;
//CHECK:                             ^^^^^^^^^^^^^^^^^^ opaque type is not supported for mutable static storage
//CHECK:                             ^^^^^^^^^^^^^^^^^^ mutable file- or namespace-scope `static` variable 'cachedOutputBuffer' has the opaque type 'AppendStructuredBuffer<float, DefaultDataLayout>', which cannot be represented by mutable storage

ConsumeStructuredBuffer<float> inputBuffer;
static ConsumeStructuredBuffer<float> cachedInputBuffer = inputBuffer;
//CHECK:                              ^^^^^^^^^^^^^^^^^ opaque type is not supported for mutable static storage
//CHECK:                              ^^^^^^^^^^^^^^^^^ mutable file- or namespace-scope `static` variable 'cachedInputBuffer' has the opaque type 'ConsumeStructuredBuffer<float, DefaultDataLayout>', which cannot be represented by mutable storage

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    cachedOutputBuffer.Append(cachedInputBuffer.Consume());
}
