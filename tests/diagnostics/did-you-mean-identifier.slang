// Test for issue #9124: an undefined identifier that is a close misspelling of
// an in-scope name produces a "did you mean ...?" suggestion note.
//
// This test runs in exhaustive mode: every emitted diagnostic must be
// annotated. That is what enforces the *negative* cases below — if a spurious
// "did you mean" note were ever produced (e.g. if a future change dropped the
// min-length guard, raised the distance cap, or removed the core-module
// filter), it would be an unannotated diagnostic and fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

float closeMatch(float myVariable)
{
    return myVariabl;
//CHECK:   ^^^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^^^ undefined identifier 'myVariabl'.
//CHECK:   ^^^^^^^^^ did you mean 'myVariable'?
}

// A name with no nearby candidate must NOT produce a suggestion note.
float noMatch()
{
    return zzzzqqq;
//CHECK:   ^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^ undefined identifier 'zzzzqqq'.
}

// Min-length threshold: identifiers shorter than 3 chars never get a
// suggestion, even when an in-scope candidate is one edit away (`ab` vs `ax`).
float tooShort(float ax)
{
    return ab;
//CHECK:   ^^ undefined identifier
//CHECK:   ^^ undefined identifier 'ab'.
}

// Min-length boundary the other side: a 3-char target is long enough, so a
// 1-edit candidate (`abz` vs `abc`) IS suggested.
float minLenOk(float abc)
{
    return abz;
//CHECK:   ^^^ undefined identifier
//CHECK:   ^^^ undefined identifier 'abz'.
//CHECK:   ^^^ did you mean 'abc'?
}

// Case-insensitive matching end-to-end: a typo differing only in case from an
// in-scope name is suggested.
float caseInsensitive(float myValue)
{
    return MYVALUE;
//CHECK:   ^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^ undefined identifier 'MYVALUE'.
//CHECK:   ^^^^^^^ did you mean 'myValue'?
}

// Distance-cap boundary. For a 6-char target the cap is
// min(3, max(1, 6/3)) = 2. A candidate at distance 2 is suggested.
float atCap(float abcdef)
{
    // `abXYef` -> `abcdef` is distance 2 (c,d substituted) == maxDistance.
    return abXYef;
//CHECK:   ^^^^^^ undefined identifier
//CHECK:   ^^^^^^ undefined identifier 'abXYef'.
//CHECK:   ^^^^^^ did you mean 'abcdef'?
}

// One past the cap: distance 3 on the same 6-char target must NOT suggest.
float overCap(float abcdef)
{
    // `abXYZf` -> `abcdef` is distance 3 (c,d,e substituted) > maxDistance.
    return abXYZf;
//CHECK:   ^^^^^^ undefined identifier
//CHECK:   ^^^^^^ undefined identifier 'abXYZf'.
}

// Core-module exclusion: a typo of a builtin type name must not be suggested,
// since core-module declarations are deliberately excluded.
float coreExcluded()
{
    return flota;
//CHECK:   ^^^^^ undefined identifier
//CHECK:   ^^^^^ undefined identifier 'flota'.
}

// Tie-break suppression: when two distinctly-named in-scope candidates are
// equally close to the typo, the winner would depend on scope/import order, so
// no suggestion is offered. Here both `abcd` and `abce` are at distance 1 from
// `abcf`.
float ambiguous(float abcd, float abce)
{
    return abcf;
//CHECK:   ^^^^ undefined identifier
//CHECK:   ^^^^ undefined identifier 'abcf'.
}

// Same-name overloads are NOT ambiguous: both `overloaded` decls would print the
// identical suggestion, so the deduped candidate set has one name and the
// suggestion is still offered.
int overloaded(int x) { return x; }
int overloaded(float x) { return 0; }
int sameNameOverloads()
{
    return overloade;
//CHECK:   ^^^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^^^ undefined identifier 'overloade'.
//CHECK:   ^^^^^^^^^ did you mean 'overloaded'?
}

// A field of the enclosing struct IS in the lexical scope of its methods, so a
// bare typo of it inside the method body is suggested. (Members reached only
// through member-access — e.g. on another value — are not searched; see the
// function comment in slang-check-expr.cpp.)
struct Holder
{
    float lengthField;
    float compute()
    {
        return lengthFiel;
//CHECK:       ^^^^^^^^^^ undefined identifier
//CHECK:       ^^^^^^^^^^ undefined identifier 'lengthFiel'.
//CHECK:       ^^^^^^^^^^ did you mean 'lengthField'?
    }
}
