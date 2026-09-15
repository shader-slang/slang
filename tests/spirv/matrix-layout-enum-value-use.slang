//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// `MatrixLayoutMode` is a public enum, so `MatrixLayoutMode.Unknown` is also an ordinary value.
// A `switch` case label on it is lowered while its type is still the enum, which makes it the
// same deduplicated constant as the unspecified layout operand of `float4x4`. Resolving that
// layout to the target default must leave the case label at 0; it used to become the default
// layout's value, so the `Unknown` case silently matched the wrong enumerator.

StructuredBuffer<float4x4> a;
RWStructuredBuffer<int> outBuf;
uniform int selector;

int classify(MatrixLayoutMode m)
{
    switch (m)
    {
    case MatrixLayoutMode.Unknown:
        return 100;
    case MatrixLayoutMode.RowMajor:
        return 200;
    default:
        return 300;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = classify(MatrixLayoutMode(selector)) + int(a[0][0][0]);
}

// The case labels must be the enumerators as written, in source order.
// CHECK: OpSwitch %{{[A-Za-z0-9_]+}} %{{[0-9]+}} 0 %{{[0-9]+}} 1 %{{[0-9]+}}
