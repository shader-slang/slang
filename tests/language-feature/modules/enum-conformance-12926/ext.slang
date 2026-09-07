//TEST_IGNORE_FILE:

implementing test_enum_conformance_12926;

// A generic type constrained on the built-in `__EnumType` interface.
struct EnumBox<T : __EnumType>
{
    T value;
}

// An `extension` whose *target type* instantiates the `__EnumType`-constrained
// generic with `Hints`. `checkModule` resolves every extension's target type in
// a dedicated pass that runs *before* the general member-checking pass reaches
// the enum's own declaration, so this checks `Hints : __EnumType` while `Hints`
// still lacks its late-synthesized `__EnumType` conformance -- caching a negative
// subtype witness that is never invalidated when the conformance is added.
extension EnumBox<Hints>
{
    uint dummy() { return 0u; }
}
