//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, sibling-vs-sibling de-duplication.
//
// When two `extension`s of the same type declare the same member function, the conflict is reported
// exactly once: by the later extension, with the note pointing back at the earlier one. Each
// extension only compares against strictly-earlier siblings, so the pair is never reported twice.
//
// A third conflicting extension confirms the pairing: it too reports exactly once, against the
// first earlier sibling that matches (the comparison stops at the first match).

struct SiblingTarget
{
    int unrelated() { return 0; }
}

// EXT A: the earliest declaration of `s`. It has no earlier sibling carrying `s`, so it never
// reports; it only appears as the note target for the later extensions.
extension SiblingTarget
{
    int s() { return 1; }
//CHECK:^ see the conflicting declaration of 's' here
//CHECK:^ see the conflicting declaration of 's' here
}

// EXT B: reports its `s` against EXT A (the one earlier sibling carrying `s`).
extension SiblingTarget
{
    int s() { return 2; }
//CHECK:^ member declared in more than one place
//CHECK:^ 's' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}

// EXT C: also reports exactly once, against the first earlier match (EXT A), confirming the pair is
// formed and not double-counted.
extension SiblingTarget
{
    int s() { return 3; }
//CHECK:^ member declared in more than one place
//CHECK:^ 's' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}
