//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// A dynamic dispatch on an empty-existential local (`IFoo f = {}`, zero
// conformances) inside a helper reachable from the entry point must report the
// clean E50100 at the dispatch site, not ICE in codegen. See #12486.
//
// `useDyn` covers a directly-called helper (one hop); `innerBar` covers a
// helper reached transitively (computeMain -> outerBar -> innerBar), which pins
// the reachability walk's multi-hop behaviour — an entry-point-only walk would
// miss both and ICE. `IFoo`/`IBar` are distinct so the per-interface dedup does
// not suppress the second diagnostic. Exhaustive: the only expected diagnostics
// are the two E30521 defaults-init warnings and the two E50100 errors, so any
// unexpected diagnostic (e.g. an ICE) surfaces here as an unannotated one.

interface IFoo
{
    int get();
}

interface IBar
{
    int get();
}

int useDyn(IFoo f)
{
    return f.get();
//CHECK:        ^ E50100
//CHECK:        ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
}

int innerBar(IBar b)
{
    return b.get();
//CHECK:        ^ E50100
//CHECK:        ^ No type conformances are found for interface 'IBar'. Code generation for current target requires at least one implementation type present in the linkage.
}

int outerBar(IBar b)
{
    return innerBar(b);
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = {};
//CHECK:     ^ E30521
//CHECK:     ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
    IBar g = {};
//CHECK:     ^ E30521
//CHECK:     ^ initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized
    outputBuffer[0] = useDyn(f);
    outputBuffer[1] = outerBar(g);
}
