//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

// An OpSwitch case literal must occupy the same word count as the selector
// type, so a 64-bit selector needs a two-word literal per case. Case values
// exceed 2^32 to distinguish a correct two-word literal from a truncated one.
// See https://github.com/shader-slang/slang/issues/12240.

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint64_t c = tid.x;
    switch (c)
    {
    case 0x123456789ABCDEF: outBuf[0] = 1; break;
    case 0x100000005:       outBuf[0] = 2; break;
    default:                outBuf[0] = 3; break;
    }

    // A negative case value exercises the sign-extended high word.
    int64_t s = tid.z;
    switch (s)
    {
    case -0x100000005:      outBuf[1] = 1; break;
    default:                outBuf[1] = 2; break;
    }

    // 32-bit selector control: a single-word literal.
    uint d = tid.y;
    switch (d)
    {
    case 5:  outBuf[2] = 10; break;
    default: outBuf[2] = 20; break;
    }
}

// spirv-dis prints the two-word case literals as decimal:
// 0x123456789ABCDEF = 81985529216486895, 0x100000005 = 4294967301.
// CHECK: OpSwitch %{{.*}} %{{.*}} 81985529216486895 %{{.*}} 4294967301 %{{.*}}
// CHECK: OpSwitch %{{.*}} %{{.*}} -4294967301 %{{.*}}
// CHECK: OpSwitch %{{.*}} %{{.*}} 5 %{{.*}}
