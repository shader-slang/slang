// Verifies the Metal counter-width cap in emitted MSL, on every CI
// platform: `-target metal` emits shading-language source with no
// downstream Metal toolchain involved, so this pins the cap even on
// Linux/Windows lanes where the metallib unit tests short-circuit.
//
// MSL provides no 64-bit atomic fetch-add (`_valid_fetch_add_type`
// rejects `device atomic_ulong*`), so counting-mode coverage at the
// default 64-bit width would fail in the Metal compiler with "no
// matching function for call to 'atomic_fetch_add_explicit'". The
// compiler therefore caps counting-mode counters to uint32 on Metal
// targets. The three runs below pin, in order: the default width
// (buffer and atomics must be 32-bit, with no `ulong` after the
// coverage buffer appears), an explicitly requested 64 capped the same
// way (the accompanying W45115 diagnostic is pinned in
// `coverage-counter-width-metal-cap-warning.slang`), and boolean mode,
// which writes plain non-atomic stores that MSL accepts at either
// width, so the 64-bit default is honored there.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry computeMain -trace-coverage
//TEST:SIMPLE(filecheck=EXPLICIT):-target metal -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 64
//TEST:SIMPLE(filecheck=BOOL):-target metal -stage compute -entry computeMain -trace-coverage -trace-coverage-boolean

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint value = tid.x;
    if ((value & 1u) == 0u)
        value += 2;
    else
        value += 3;
    outputBuffer[0] = value;
}

// METAL: uint device* _slang_coverage
// METAL: atomic_fetch_add_explicit{{.*}}atomic_uint
// METAL-NOT: ulong

// EXPLICIT: uint device* _slang_coverage
// EXPLICIT: atomic_fetch_add_explicit{{.*}}atomic_uint
// EXPLICIT-NOT: ulong

// BOOL: ulong device* _slang_coverage
// BOOL: = 1ULL
