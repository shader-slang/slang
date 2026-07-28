//TEST:SIMPLE(filecheck=PLAIN): -stage compute -entry plainMain -target wgsl
//TEST:SIMPLE(filecheck=EARLY): -stage compute -entry earlyMain -target wgsl

// Regression test for https://github.com/shader-slang/slang/issues/11946.
//
// WGSL switch cases never fall through, so the `break;` Slang places at the tail of each
// case only exits the switch — which reaching the case's end already does. Older `naga`
// validators reject `break` outside a loop, so that trailing break must be omitted for WGSL.
// A genuine early break inside a case is semantically required and must still be emitted.

RWStructuredBuffer<int> outputBuffer;

// (a) Plain switch whose only breaks are the redundant terminating ones. Each case body
// must end immediately after its assignment (`}` on the next line), with no `break` anywhere.
[shader("compute")]
[numthreads(1, 1, 1)]
void plainMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    switch (tid.x)
    {
    case 0u:
        r = 10;
        break;
    case 1u:
        r = 20;
        break;
    default:
        r = 30;
        break;
    }
    outputBuffer[0] = r;
}

//PLAIN: switch(
//PLAIN: i32(10)
//PLAIN-NEXT: }
//PLAIN: i32(20)
//PLAIN-NEXT: }
//PLAIN: i32(30)
//PLAIN-NEXT: }
//PLAIN-NOT: break

// (b) A case with a genuine early break followed by more code. The early break (inside the
// `if`) is kept; the terminating break of case 0 (after `r = 2`) and of the default case are
// both omitted.
[shader("compute")]
[numthreads(1, 1, 1)]
void earlyMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    switch (tid.x)
    {
    case 0u:
        if (tid.y > 0u)
        {
            r = 1;
            break;
        }
        r = 2;
        break;
    default:
        r = 3;
        break;
    }
    outputBuffer[0] = r;
}

//EARLY: switch(
//EARLY: i32(1)
//EARLY-NEXT: break;
//EARLY: i32(2)
//EARLY-NEXT: }
//EARLY: i32(3)
//EARLY-NEXT: }
