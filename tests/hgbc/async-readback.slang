//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
uint main(RWStructuredBuffer<uint> values)
{
    Defer<uint> pending = asyncReadBack(values, 0);
    return pending.wait();
}

// CHECK: OpDeferType
// CHECK: OpAsyncReadBackBufferElement
// CHECK: OpWaitDefer
