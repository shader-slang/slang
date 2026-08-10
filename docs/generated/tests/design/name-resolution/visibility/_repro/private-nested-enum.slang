// Repro for finding `private-nested-enum-locationless-visibility-errors`.
// Not a test: no //TEST directive, so slang-test skips it.
//
// A `private` enum nested inside a struct is a member of a type, so the
// declaration-site private check should accept it (the same enum marked
// `internal` in the same position compiles cleanly). Instead the compiler
// emits 28 E30604 / E30601 diagnostics with no source location at all.

struct S
{
    private enum Mode { Fast, Slow }
}
