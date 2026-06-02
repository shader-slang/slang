//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`static` (non-groupshared) at global scope belongs to the Thread-local address space; HLSL emits it as a plain `static` variable (per-thread instance).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table row for "Thread-local" states: `static` (global scope, not groupshared)
// is in the Thread-local address space, instance scope = thread.
// Each thread gets its own separate instance of the variable.
// HLSL: emits as `static <type> <name>` at module scope.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

static int g_perThread;

[__NoInline]
int readAndSet(int v)
{
    int old = g_perThread;
    g_perThread = v;
    return old;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<int> output)
{
    g_perThread = int(tid.x);
    int prev = readAndSet(int(tid.x) + 10);
    output[tid.x] = prev;
}

// HLSL thread-local static emits as `static` at module scope.
// CHECK: static int g_perThread{{.*}}
