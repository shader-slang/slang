//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// The `ref` accessor returns a pointer into `computeMain()`'s thread-local `storage`. Once the
// accessor's result is specialized, Metal address-space specialization must revisit
// `computeMain()` so the call result acquires the same address space. Since `computeMain()`'s
// `void` result does not change, that iteration must not queue any caller for another iteration.

RWStructuredBuffer<int> outputBuffer;

struct RefAccessorStorage
{
    int value;

    property int storedValue
    {
        [mutating]
        ref { return value; }
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    RefAccessorStorage storage;
    storage.storedValue = int(dispatchThreadID.x);
    outputBuffer[dispatchThreadID.x] = storage.value;
}

// CHECK-LABEL: int thread* RefAccessorStorage_storedValue_ref_0(
// CHECK-SAME: RefAccessorStorage_0 thread* [[THIS:[A-Za-z_][A-Za-z0-9_]*]])
// CHECK: return &[[THIS]]->value_0;
// CHECK: {{\[\[}}kernel{{\]\]}} void computeMain(
// CHECK: int thread* [[ADDRESS:[A-Za-z_][A-Za-z0-9_]*]]
// CHECK-SAME: = RefAccessorStorage_storedValue_ref_0(&storage_0);
// CHECK: *[[ADDRESS]] =
