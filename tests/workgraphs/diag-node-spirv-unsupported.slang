//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv-asm -stage node -entry main -experimental-feature
import experimental.workgraph;

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(DispatchNodeInputRecord<Record> input, RWBuffer<int> output)
//CHECK: entrypoint 'main' uses features that are not available in 'node' stage for 'spirv' compilation target.
{
    output[0] = input.Get().value;
}
