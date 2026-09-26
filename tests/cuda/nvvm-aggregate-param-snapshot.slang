//TEST:SIMPLE(filecheck=IR):-target ptx -stage compute -entry computeMain -O3 -capability cuda_sm_8_0 -emit-cuda-via-nvvm -dump-ir-after deferBufferLoad -validate-ir -o -
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[1 2], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 7
// CHECK-NEXT: 99
// CHECK-NEXT: 18
// CHECK-NEXT: 88
// CHECK-NEXT: 20
// CHECK-NEXT: 40
// CHECK-NEXT: 1307
// CHECK-NEXT: 7
// CHECK-NEXT: 55
// CHECK-NEXT: 23

struct Snapshot
{
    uint first;
    uint middle;
    uint last;
};
struct NestedSnapshot
{
    Snapshot data;
    uint sibling;
};

// A separately captured value must survive a callee mutation of its original source.
[noinline]
uint readDuringMutation(Snapshot saved, inout Snapshot live)
{
    live.first = 99;
    return saved.first;
}
[noinline]
uint readNestedDuringMutation(NestedSnapshot saved, inout NestedSnapshot live)
{
    live.data.middle = 88;
    return saved.data.first + saved.data.middle;
}
[noinline]
uint readEnds(Snapshot saved)
{
    return saved.first + saved.last;
}
[noinline]
uint readReordered(Snapshot saved)
{
    return saved.last * 100 + saved.first;
}
[noinline]
uint readFirst(Snapshot saved)
{
    return saved.first;
}

// A whole-value use must keep the original aggregate parameter contract.
[noinline]
Snapshot returnWhole(Snapshot saved)
{
    return saved;
}

StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;
[numthreads(1, 1, 1)]
void computeMain()
{
    Snapshot live = {7, 11, 13};
    Snapshot beforeMutation = live;
    outputs[0] = readDuringMutation(beforeMutation, live);
    outputs[1] = live.first;

    NestedSnapshot nested = {{7, 11, 13}, 31};
    NestedSnapshot beforeNestedMutation = nested;
    outputs[2] = readNestedDuringMutation(beforeNestedMutation, nested);
    outputs[3] = nested.data.middle;

    for (uint i = 0; i < 2; ++i)
    {
        Snapshot left = {7, 11, 13};
        Snapshot right = {17, 19, 23};
        Snapshot selected = inputs[i] == 1 ? left : right;
        left.first = 101;
        right.last = 103;
        outputs[4 + i] = readEnds(selected);
    }

    Snapshot original = {7, 11, 13};
    outputs[6] = readReordered(original);
    Snapshot saved = original;
    original.first = 55;
    outputs[7] = readFirst(saved);
    outputs[8] = original.first;
    Snapshot whole = returnWhole(Snapshot(17, 19, 23));
    outputs[9] = whole.last;
}

// The pass changes only partially consumed value parameters. Whole-value uses retain their ABI.
// IR-LABEL: func %readDuringMutation{{.*}}: Func(UInt, UInt, BorrowInOutParam(%Snapshot))
// IR-LABEL: func %readNestedDuringMutation{{.*}}: Func(UInt, %Snapshot, BorrowInOutParam(%NestedSnapshot))
// IR-LABEL: func %readEnds{{.*}}: Func(UInt, UInt, UInt)
// IR-LABEL: func %readReordered{{.*}}: Func(UInt, UInt, UInt)
// IR-LABEL: func %returnWhole{{.*}}: Func(%Snapshot, %Snapshot)
