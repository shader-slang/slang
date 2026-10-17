//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute

// Loops with a simple counter (initialized just before the loop, used only inside it, and
// updated once by a constant on the back edge) are emitted as WGSL `for` loops with the
// update in the `for` header. Tint can prove such loops finite, so with robustness enabled
// it does not add its iteration counter to them. The update must not be repeated at
// `continue` sites, since a WGSL `for` already runs it on `continue`.
// counted-loop-for-exec.slang runs these loops and checks their results.

// The WGSL output is valid: Tint parses it and translates it to SPIR-V.
// SPIRV: OpEntryPoint

[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> buffer;

// `[unroll]` (as in the shader of #13733) does not prevent the match. WGSL has no loop
// attribute, so nothing is printed for it.
// WGSL-LABEL: fn countUp
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL-NEXT: {
// WGSL-NEXT: if(([[I]] < i32(4)))
// WGSL-NOT: [[I]] = [[I]] + i32(1);
// WGSL: return
uint countUp(uint x)
{
    uint s = 0;
    [unroll]
    for (int i = 0; i < 4; i++)
        s += x << i;
    return s;
}

// WGSL-LABEL: fn countWithContinue
// WGSL: for (var [[I:i_[0-9]+]] : u32 = u32(0); ; [[I]] = [[I]] + u32(1))
// WGSL-NOT: [[I]] = [[I]] + u32(1);
// WGSL: continue;
// WGSL-NOT: [[I]] = [[I]] + u32(1);
// WGSL: return
uint countWithContinue(uint x)
{
    uint s = 0;
    for (uint i = 0; i < 8; i++)
    {
        if (((x >> i) & 1) == 0)
            continue;
        s += i;
    }
    return s;
}

// WGSL-LABEL: fn countDown
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(7); ; [[I]] = [[I]] - i32(1))
// WGSL-NOT: [[I]] - i32(1)
// WGSL: return
uint countDown(uint x)
{
    uint s = 0;
    for (int i = 7; i >= 0; i--)
        s = s * 3 + (x >> i);
    return s;
}

// The update value is also used in the body: the body keeps its temporary, and the header
// recomputes the update from the counter.
// WGSL-LABEL: fn sharedIncrement
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL: var [[T:_S[0-9]+]] : i32 = [[I]] + i32(1);
// WGSL-NOT: [[I]] = [[T]];
// WGSL: return
uint sharedIncrement(uint x)
{
    uint s = 0;
    for (int i = 0; i <= 3; i++)
        s += x * uint(i + 1) + uint(i);
    return s;
}

// WGSL-LABEL: fn nestedLoops
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL: for (var [[J:j_[0-9]+]] : i32 = i32(0); ; [[J]] = [[J]] + i32(1))
// WGSL-NOT: [[J]] = [[J]] + i32(1);
// WGSL: break;
// WGSL-NOT: [[J]] = [[J]] + i32(1);
// WGSL-NOT: [[I]] = [[I]] + i32(1);
// WGSL: return
uint nestedLoops(uint x)
{
    uint s = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (j == int(x))
                break;
            s += uint(i * j);
        }
    }
    return s;
}

// The counter is read after the loop, so it cannot be scoped to a `for` header: the loop
// keeps its current shape.
// WGSL-LABEL: fn usedAfterLoop
// WGSL: var [[I:i_[0-9]+]] : u32 = u32(0);
// WGSL-NEXT: loop
// WGSL: [[I]] = [[I]] + u32(1);
// WGSL: return [[I]];
uint usedAfterLoop(uint x)
{
    uint i = 0;
    for (; i < 8; i++)
    {
        if (buffer[i] == x)
            break;
    }
    return i;
}

// A second variable is updated on the back edge and read after the loop: it keeps its
// update in the body, and the loop counter still moves to the `for` header.
// WGSL-LABEL: fn twoCounters
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL-NOT: [[I]] = [[I]] + i32(1);
// WGSL: var [[T:k_[0-9]+]] : u32 = [[K:k_[0-9]+]] + u32(2);
// WGSL-NEXT: [[K]] = [[T]];
// WGSL: return
uint twoCounters(uint x)
{
    uint k = 0;
    for (int i = 0; i < 4; k += 2, i++)
    {
        if (buffer[i] == x)
            break;
    }
    return k;
}

// The update operand order `step + i` is accepted.
// WGSL-LABEL: fn stepFirst
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL-NOT: [[I]] = [[I]] + i32(1);
// WGSL: return
uint stepFirst(uint x)
{
    uint s = 0;
    for (int i = 0; i < 4; i = 1 + i)
        s += x << i;
    return s;
}

// An unsigned counter counting down.
// WGSL-LABEL: fn unsignedCountDown
// WGSL: for (var [[I:i_[0-9]+]] : u32 = u32(8); ; [[I]] = [[I]] - u32(1))
// WGSL-NOT: [[I]] = [[I]] - u32(1);
// WGSL: return
uint unsignedCountDown(uint x)
{
    uint s = 0;
    for (uint i = 8; i > 0; i--)
        s = s * 2 + ((x >> (i - 1)) & 1);
    return s;
}

// The continue clause reads the counter after updating it. That read becomes the update
// value, which the body keeps, so the counter itself is not read after the update.
// WGSL-LABEL: fn readAfterUpdate
// WGSL: for (var [[I:i_[0-9]+]] : i32 = i32(0); ; [[I]] = [[I]] + i32(1))
// WGSL: var [[T:i_[0-9]+]] : i32 = [[I]] + i32(1);
// WGSL-NEXT: s_{{[0-9]+}} + u32([[T]])
// WGSL-NOT: [[I]] = [[T]];
// WGSL: return
uint readAfterUpdate(uint x)
{
    uint s = 0;
    for (int i = 0; i < 4; i++, s += uint(i))
        s += x;
    return s;
}

// An initial value that has its own name is used in the header.
// WGSL-LABEL: fn namedInit
// WGSL: var [[START:start_[0-9]+]] : i32 = i32(buffer
// WGSL: for (var [[I:i_[0-9]+]] : i32 = [[START]]; ; [[I]] = [[I]] + i32(1))
// WGSL: return
uint namedInit(uint x)
{
    uint s = 0;
    int start = int(buffer[x & 7]);
    for (int i = start; i < 8; i++)
        s += uint(i);
    return s + uint(start);
}

// An initial value that would be printed inline at its only use: the loop keeps its shape,
// so the value is still evaluated where the source evaluates it.
// WGSL-LABEL: fn foldedInit
// WGSL-NOT: for (
// WGSL: loop
// WGSL: return
uint foldedInit(uint x)
{
    uint s = 0;
    for (int i = int(x) * 2; i < 8; i++)
        s += uint(i);
    return s;
}

// The body also writes the counter. The written value gets its own variable, so the back
// edge no longer updates the counter by a constant, and the loop keeps its shape.
// WGSL-LABEL: fn storeInBody
// WGSL-NOT: for (
// WGSL: loop
// WGSL: return
uint storeInBody(uint x)
{
    uint s = 0;
    for (int i = 0; i < 8; i++)
    {
        if (buffer[i] == x)
            i++;
        s += uint(i);
    }
    return s;
}

// The counter is passed by reference to a function, which could write it: the loop keeps
// its shape.
[noinline]
void bump(inout int i)
{
    i += 2;
}

// WGSL-LABEL: fn passedInout
// WGSL-NOT: for (
// WGSL: loop
// WGSL: return
uint passedInout(uint x)
{
    uint s = 0;
    for (int i = 0; i < 8; i++)
    {
        bump(i);
        s += uint(i) * x;
    }
    return s;
}

// The update `10 - i` is not a constant step: the loop keeps its shape.
// WGSL-LABEL: fn literalMinusCounter
// WGSL-NOT: for (
// WGSL: loop
// WGSL: return
uint literalMinusCounter(uint x)
{
    uint s = 0;
    for (int i = 0; i < 10; i = 10 - i)
    {
        s += uint(i);
        if (s > x)
            break;
    }
    return s;
}

// The update `x = x / 2` is not a constant step, and `s++` is but `s` is read after the
// loop: the loop keeps its current shape.
// WGSL-LABEL: fn halving
// WGSL-NOT: for (
// WGSL: loop
// WGSL: return
uint halving(uint x)
{
    uint s = 0;
    while (x > 1)
    {
        x = x / 2;
        s++;
    }
    return s;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint x = buffer[tid.x];
    buffer[tid.x] = countUp(x) + countWithContinue(x) + countDown(x) + sharedIncrement(x) +
                    nestedLoops(x) + usedAfterLoop(x) + twoCounters(x) + stepFirst(x) +
                    unsignedCountDown(x) + readAfterUpdate(x) + namedInit(x) + foldedInit(x) +
                    storeInBody(x) + passedInout(x) + literalMinusCounter(x) + halving(x);
}
