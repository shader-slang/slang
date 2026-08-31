// internal-param-modifiers.slang
//
// The internal `__ref_readonly` / `__ref_writeonly` / `__consume` parameter
// modifiers are representable (parse, check, lower to IR, mangle) but the target
// emitters do not yet handle their IR param-type ops, so a reachable use is
// diagnosed (see internal-param-modifiers-diagnostic.slang). The functions below
// are declared but left unreferenced by the entry point so they are eliminated
// before emit; the IR dump (taken before elimination) confirms each modifier
// lowers to its own distinct IR param-type op and its own mangled
// parameter-direction mnemonic, so functions differing only by these modes are
// distinct signatures rather than colliding redeclarations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -dump-ir -o /dev/null

// Each modifier lowers to a distinct IR param-type op wrapping the value type.
//CHECK-DAG: RefReadOnlyParam(Int
//CHECK-DAG: RefWriteOnlyParam(Int
//CHECK-DAG: ConsumeParam(Int

// Each mode gets a distinct mangled parameter-direction mnemonic (rr_ / rw_ /
// cn_), so these three same-named overloads coexist without colliding.
//CHECK-DAG: use{{.*}}rr_
//CHECK-DAG: use{{.*}}rw_
//CHECK-DAG: use{{.*}}cn_

int use(__ref_readonly int x)
{
    return x + 1;
}

int use(__ref_writeonly int x)
{
    x = 7;
    return 100;
}

int use(__consume int x)
{
    return x * 2;
}

// A `[ForceInline]` function using a new mode is fully inlined away before code
// generation, so it leaves no unsupported wrapper and is allowed to compile — this
// pins the gate-placement contract (the reject runs after inlining, not before).
[ForceInline]
int inlined(__ref_readonly int x)
{
    return x + 1;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 5;
    outputBuffer[0] = inlined(a);
}
