// A reference parameter -- `__ref`, `__ref_readonly`, `__ref_writeonly`, or `groupshared`,
// which implies `__ref` -- does not guarantee exclusive access to its storage: the caller,
// and for `groupshared` other invocations, may access it through aliases. An `out` or
// `inout` parameter, and the implicit `this` of a `[mutating]` method, require exclusive
// access to their argument for the duration of the call, so forwarding a reference parameter
// to one at a call inside the callee is a mismatch the front end can see from the signatures
// alone (shader-slang/slang#13836). The warning is opt-in, in the `all` group: this test
// runs it with `-Wall` and pins that it stays off by default, that the other warning groups
// do not turn it on, and that a per-id `-Wno-` suppresses it. Bouncing the value through a
// local temporary restores the exclusivity guarantee and silences it. A direct `groupshared`
// argument such as `g[i]` is deliberately not diagnosed: barriers may legitimately
// synchronize its aliases during the call, and the checker cannot see that (the removed
// E30709/E30710 warnings fired on such valid code, which is why they were retired).

// Off by default (no -Wall): the warning must not appear.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF,exit-code=0): -target hlsl -entry computeMain -stage compute -experimental-feature

// The groups are independent, and a per-id disable wins: neither may turn this `all` warning on.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF,exit-code=0): -Wextra -target hlsl -entry computeMain -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF,exit-code=0): -Wall -Wno-passing-non-exclusive-memory-to-exclusive-parameter -target hlsl -entry computeMain -stage compute -experimental-feature

// With -Wall, and with an explicit per-id enable, the warning appears.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -Wall -target hlsl -entry computeMain -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -Wpassing-non-exclusive-memory-to-exclusive-parameter -target hlsl -entry computeMain -stage compute -experimental-feature

struct S { int f; }

struct Counter { int n; [mutating] void bump() { n += 1; } }

groupshared int g[4];

void innerInout(inout int x) { x += 1; }
void innerOut(out int x) { x = 1; }
void innerUint(inout uint x) { x += 1; }

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

// Compound assignment and `++`/`--` desugar into operator calls whose mutating operand is an
// `inout` parameter; writing through a reference that way is the normal, well-defined use of
// one, so these lines must stay silent — they are what exercises the operator exemption.
void takesRef(__ref int r) { r += 1; ++r; }

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

// An l-value-coerced argument writes the conversion's temporary back to the operand's
// storage, so a reference parameter forwarded through the coercion warns too.
void viaRefCoerce(__ref int y)
{
    innerUint(y);
//CHECK:      ^ passing non-exclusive memory ('y') to a function ('innerUint') that requires exclusive access
//CHECK:      ^ parameter 'x' requires exclusive access to its argument for the duration of the call, but reference parameter 'y' does not guarantee that
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

// The receiver of a `[mutating]` method is an implicit `inout` parameter, so a reference
// parameter's storage forwarded as the receiver warns too.
void viaMutating(__ref Counter c)
{
        c.bump();
//CHECK:^ passing non-exclusive memory ('c') to a function ('bump') that requires exclusive access
//CHECK:^ parameter 'this' requires exclusive access to its argument for the duration of the call, but reference parameter 'c' does not guarantee that
}

// --- Silent: the argument guarantees exclusivity, or the callee does not require it ---

// A bounce through a local temporary is exclusive by construction.
void viaRefBounce(__ref int y)
{
    int t = y;
    innerInout(t);
    y = t;
}

// A local variable declared in the callee has no aliases.
void takesLocal() { int l = 0; innerInout(l); }

// An `in` parameter is a copy; the callee's writes go to its own storage.
void viaIn(int v) { innerInout(v); }

// An `inout` parameter already carries the caller's exclusivity guarantee, which covers
// the nested call.
void viaInOut(inout int v) { innerInout(v); }

// A plain local receiver is exclusive.
void localMutating() { Counter c = Counter(); c.bump(); }

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    // Direct `groupshared` argument: silent by scope, see the removed E30709/E30710.
    innerInout(g[i]);

    viaRef(g[i]);
    viaRefBounce(g[i]);
    S sLocal = S();
    viaRefStruct(sLocal);
    viaRefArray(g);
    viaRefCoerce(g[i]);
    viaGroupsharedParam(g[i]);
    viaWriteonly(g[i]);
    Counter cMain = Counter();
    viaMutating(cMain);
    viaIn(1);
    viaInOut(g[i]);
    takesLocal();
    localMutating();
}
