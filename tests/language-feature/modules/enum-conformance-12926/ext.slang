//TEST_IGNORE_FILE:

implementing test_enum_conformance_12926;

struct EnumBox<T : __EnumType>
{
    T value;
}

// The extension target instantiates a `__EnumType`-constrained generic with `Hints`, so the
// extension-first pass evaluates `Hints : __EnumType` -- a lookup that must complete `Hints`'s
// own conformance before it can answer.
extension EnumBox<Hints>
{
    uint dummy() { return 0u; }
}
