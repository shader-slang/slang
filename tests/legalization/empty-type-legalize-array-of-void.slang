// Regression for shader-slang/slang#11917: the `legalizeEmptyTypes` entry-time early-out
// (`hasEmptyTypeLegalizationWork`) must run the pass when the only empty-type root is an array whose
// element is `void`. With `int[0][2]` the
// earlier `legalizeEmptyArray` pass rewrites the zero-length `int[0]` to `void`, leaving
// `Array(Void, 2)` for `legalizeEmptyTypes` to drop, so that array-of-void reaches this pass's
// globals-scope presence scan as an input and exercises its array branch directly. The module has no
// empty structs, `PseudoPtr`s, or generics, so that branch is the only thing that keeps the pass from
// early-outing.
//
// `-o -` is needed so the backend pipeline runs (the pass, and thus `-dump-ir-after`, only fires when
// an output is requested) — the `export` function has no entry point of its own. Writing to stdout
// rather than a null device keeps the invocation valid on every host platform.

//TEST:SIMPLE(filecheck=IR):-target cpp -o - -dump-ir-before legalizeEmptyTypes -dump-ir-after legalizeEmptyTypes

// `int[0][2]` is `Array(Array(Int, 0), 2)`; `legalizeEmptyArray` rewrites the zero-length `int[0]` to
// `void`, leaving the immediate array-of-void `Array(Void, 2)` as the parameter type the pass drops.
export __extern_cpp int useNestedZeroArray(int[0][2] dropped, int kept)
{
    return kept;
}

// The compile must succeed — a failing invocation could still print the expected IR and pass.
// IR: result code = 0
// IR: ### BEFORE legalizeEmptyTypes
// IR: func %useNestedZeroArray {{.*}}: Func(Int, Array(Void, 2 {{.*}}), Int)
// IR: ### AFTER legalizeEmptyTypes
// IR: func %useNestedZeroArray {{.*}}: Func(Int, Int)
// IR-NOT: func %useNestedZeroArray {{.*}}: Func(Int, Array(Void
