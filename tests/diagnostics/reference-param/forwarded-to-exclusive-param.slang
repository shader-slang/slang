// A reference parameter -- `__ref`, or `groupshared` which implies `__ref` -- does not
// guarantee exclusive access to its storage: the caller, and for `groupshared` other
// invocations, may access it through aliases. An `out` or `inout` parameter requires
// exclusive access to its argument for the duration of the call, so forwarding a reference
// parameter to one at a call inside the callee is a mismatch the front end can see from the
// signatures alone (shader-slang/slang#13836). The warning is opt-in, in the `all` group, so
// this test passes `-Wall`. Bouncing the value through a local temporary restores the
// exclusivity guarantee and silences it. A direct `groupshared` argument such as `g[i]` is
// deliberately not diagnosed: barriers may legitimately synchronize its aliases during the
// call, and the checker cannot see that (the removed E30709 warnings fired on such valid
// code, which is why they were retired).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -target hlsl -entry computeMain -stage compute -Wall -experimental-feature

struct S { int f; }

groupshared int g[4];

void innerInout(inout int x) { x += 1; }
void innerOut(out int x) { x = 1; }
void takesRef(__ref int r) { r += 1; }

// --- Warns: a reference parameter forwarded to an `out`/`inout` parameter ---

void viaRef(__ref int y)
{
    innerInout(y);
//CHECK:       ^ passing non-exclusive memory ('y') to a function ('innerInout') that requires exclusive access
//CHECK:       ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'y' does not guarantee that
    innerOut(y);
//CHECK:     ^ passing non-exclusive memory ('y') to a function ('innerOut') that requires exclusive access
//CHECK:     ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'y' does not guarantee that
    takesRef(y);
}

// Projections of a reference parameter inherit its storage, so forwarding them warns too.
void viaRefStruct(__ref S s)
{
    innerInout(s.f);
//CHECK:         ^ passing non-exclusive memory ('s') to a function ('innerInout') that requires exclusive access
//CHECK:         ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 's' does not guarantee that
}

void viaRefArray(__ref int a[4])
{
    innerInout(a[0]);
//CHECK:        ^ passing non-exclusive memory ('a') to a function ('innerInout') that requires exclusive access
//CHECK:        ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'a' does not guarantee that
}

// A `groupshared` parameter is implicitly a `__ref` parameter, so forwarding it warns too.
void viaGroupsharedParam(groupshared int p)
{
    innerInout(p);
//CHECK:       ^ passing non-exclusive memory ('p') to a function ('innerInout') that requires exclusive access
//CHECK:       ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'p' does not guarantee that
}

// A `__ref_writeonly` parameter is the same kind of reference, so forwarding it warns too.
void viaWriteonly(__ref_writeonly int w)
{
    innerInout(w);
//CHECK:       ^ passing non-exclusive memory ('w') to a function ('innerInout') that requires exclusive access
//CHECK:       ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'w' does not guarantee that
}

// --- Silent: the argument guarantees exclusivity, or the callee does not require it ---

// A bounce through a local temporary is exclusive by construction.
void viaRefBounce(__ref int y)
{
    int t = y;
    innerInout(t);
    y = t;
}

// A plain local has no aliases.
void takesLocal(int l) { innerInout(l); }

// An `in` parameter is a copy; the callee's writes go to its own storage.
void viaIn(int v) { innerInout(v); }

// An `inout` parameter already carries the caller's exclusivity guarantee, which covers
// the nested call.
void viaInOut(inout int v) { innerInout(v); }

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    // Direct `groupshared` argument: silent by scope, see the removed E30709.
    innerInout(g[i]);

    viaRef(g[i]);
    viaRefBounce(g[i]);
    S sLocal = S();
    viaRefStruct(sLocal);
    viaRefArray(g);
    viaGroupsharedParam(g[i]);
    viaWriteonly(g[i]);
    viaIn(1);
    viaInOut(g[i]);
}
