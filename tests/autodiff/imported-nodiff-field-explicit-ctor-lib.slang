//TEST_IGNORE_FILE:

// Library half of `imported-nodiff-field-explicit-ctor.slang` (regression for #13233).
//
// `Value` is an `IDifferentiable` struct that combines an explicit `__init()` with a
// `no_diff` field. Defined in a separate module, it is the third leg of the three-way
// interaction that used to drop the differentiable-type association at link: imported +
// explicit constructor + `no_diff` field. The explicit `__init()` suppresses the
// synthesized member-wise constructor, so this defining module never differentiates
// `Value` and never emits its `DifferentialPairType` association; only the importing
// module does, onto its `[import]` declaration.

module "imported-nodiff-field-explicit-ctor-lib";

public struct Value : IDifferentiable
{
    public float x = 0.0;
    public no_diff uint tag = 0;
    public __init() {}
}
