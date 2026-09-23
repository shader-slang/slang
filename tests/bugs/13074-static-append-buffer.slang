// Append-buffer lowering represents one source variable with separate buffer and counter values.
// Resource-global legalization cannot yet replace those values as one file-scope `static` variable,
// so it rejects this declaration before lowering.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

AppendStructuredBuffer<float> outputBuffer;
static AppendStructuredBuffer<float> cachedOutputBuffer = outputBuffer;
//CHECK:                             ^^^^^^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                             ^^^^^^^^^^^^^^^^^^ global variable cannot have opaque type.
//CHECK:                             ^^^^^^^^^^^^^^^^^^ do you intend to define a `static const` instead?

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    cachedOutputBuffer.Append(1.0);
}
