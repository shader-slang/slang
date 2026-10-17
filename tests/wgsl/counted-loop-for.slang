//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

// Loops with a simple counter (initialized just before the loop, used only inside it, and
// updated once by a constant on the back edge) are emitted as WGSL `for` loops with the
// update in the `for` header. Tint can prove such loops finite, so with robustness enabled
// it does not add its iteration counter to them. The update must not be repeated at
// `continue` sites, since a WGSL `for` already runs it on `continue`.

[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> buffer;

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
// WGSL: break;
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

// No counter updated by a constant: the loop keeps its current shape.
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
                    nestedLoops(x) + usedAfterLoop(x) + twoCounters(x) + halving(x);
}
