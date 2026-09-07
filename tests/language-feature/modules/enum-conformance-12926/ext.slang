//TEST_IGNORE_FILE:

implementing test_enum_conformance_12926;

struct EnumBox<T : __EnumType>
{
    T value;
}

// The extension target instantiates a `__EnumType`-constrained generic with `Hints`, forcing
// `Hints : __EnumType` to be evaluated in the extension-first pass -- before `Hints` itself is
// checked. This is the ordering that exercises the fix.
extension EnumBox<Hints>
{
    uint dummy() { return 0u; }
}
