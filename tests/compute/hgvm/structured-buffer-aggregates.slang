//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct AggregateItem
{
    Array<bool, 3> flags;
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void exerciseAggregateBuffers(
    RWStructuredBuffer<AggregateItem> items,
    RWStructuredBuffer<Array<AggregateItem, 2>> batches,
    RWStructuredBuffer<uint> output)
{
    AggregateItem first;
    first.flags[0] = true;
    first.flags[1] = false;
    first.flags[2] = true;
    first.value = 41;
    items[0] = first;

    AggregateItem loadedItem = items[0];
    output[0] = loadedItem.flags[0] ? loadedItem.value : 0;
    output[1] = loadedItem.flags[1] ? 1 : 0;
    output[2] = loadedItem.flags[2] ? 1 : 0;

    AggregateItem second;
    second.flags[0] = false;
    second.flags[1] = true;
    second.flags[2] = false;
    second.value = 73;

    Array<AggregateItem, 2> batch;
    batch[0] = first;
    batch[1] = second;
    batches[0] = batch;

    Array<AggregateItem, 2> loadedBatch = batches[0];
    output[3] = loadedBatch[0].value;
    output[4] = loadedBatch[1].flags[1] ? loadedBatch[1].value : 0;

    uint itemCount;
    uint itemStride;
    items.GetDimensions(itemCount, itemStride);
    output[5] = itemCount;
    output[6] = itemStride;

    uint batchCount;
    uint batchStride;
    batches.GetDimensions(batchCount, batchStride);
    output[7] = batchCount;
    output[8] = batchStride;
}

[host]
void main()
{
    RWStructuredBuffer<AggregateItem> items = RWStructuredBuffer<AggregateItem>::alloc(1);
    RWStructuredBuffer<Array<AggregateItem, 2>> batches =
        RWStructuredBuffer<Array<AggregateItem, 2>>::alloc(1);
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(9);

    dispatch(exerciseAggregateBuffers, uint3(1, 1, 1))(items, batches, output);
    for (uint index = 0; index < 9; ++index)
        print(readBack(output, index));

    // Aggregate readback uses the inverse of the reflected device layout. In particular, the
    // three four-byte device booleans are compacted back into the VM's three one-byte slots before
    // the following ordinary struct and array accesses execute.
    AggregateItem loadedItem = readBack(items, 0);
    print(loadedItem.flags[0]);
    print(loadedItem.flags[1]);
    print(loadedItem.flags[2]);
    print(loadedItem.value);

    Array<AggregateItem, 2> loadedBatch = readBack(batches, 0);
    print(loadedBatch[0].value);
    print(loadedBatch[1].flags[1]);
    print(loadedBatch[1].value);
}

// CHECK: 41
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 41
// CHECK-NEXT: 73
// CHECK-NEXT: 1
// CHECK-NEXT: 16
// CHECK-NEXT: 1
// CHECK-NEXT: 32
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: 41
// CHECK-NEXT: 41
// CHECK-NEXT: true
// CHECK-NEXT: 73
