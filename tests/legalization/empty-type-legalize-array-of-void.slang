// Regression for shader-slang/slang#11917: the `legalizeEmptyTypes` entry-time early-out
// (`hasEmptyTypeLegalizationWork` -> `isEmptyTypeShapeToLegalize` -> `isEmptyTypeToLegalize`) must run
// the pass when the only empty-type root is an array whose element is `void`. A zero-length inner
// dimension makes `int[0][2]` lower to `Array(Void, 2)` at IR-gen — before legalization — so this
// array-of-void reaches the pass's globals-scope presence scan as an input, exercising its array
// branch directly. The module has no empty structs, `PseudoPtr`s, or generics, so that branch is the
// only thing that keeps the pass from early-outing.
//
// `-o /dev/null` is needed so the backend pipeline runs (the pass, and thus `-dump-ir-after`, only
// fires when an output is requested); the `export` function has no entry point of its own.

//TEST:SIMPLE(filecheck=IR):-target cpp -o /dev/null -dump-ir-before legalizeEmptyTypes -dump-ir-after legalizeEmptyTypes

// `int[0][2]` is `Array(Array(Int, 0), 2)`; the zero-length `int[0]` legalizes to `void`, leaving the
// immediate array-of-void `Array(Void, 2)` as the parameter type the pass must drop.
export __extern_cpp int useNestedZeroArray(int[0][2] dropped, int kept)
{
    return kept;
}

// IR: ### BEFORE legalizeEmptyTypes
// IR: func %useNestedZeroArray {{.*}}: Func(Int, Array(Void, 2 {{.*}}), Int)
// IR: ### AFTER legalizeEmptyTypes
// IR: func %useNestedZeroArray {{.*}}: Func(Int, Int)
// IR-NOT: func %useNestedZeroArray {{.*}}: Func(Int, Array(Void
