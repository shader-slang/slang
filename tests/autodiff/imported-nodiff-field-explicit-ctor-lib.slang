//TEST_IGNORE_FILE:

// Library half of `imported-nodiff-field-explicit-ctor.slang` (regression for #13233).
//
// The test needs `Value` to be imported and to combine an explicit `__init()` with a
// `no_diff` field. The explicit `__init()` suppresses the synthesized member-wise
// constructor, so this defining module never differentiates `Value` and its `[export]`
// carries no `DifferentialPairType` association; only the importing module records that
// association, onto its `[import]` declaration. That split across declarations is what the
// test exercises, so `Value` lives in its own module here.

module "imported-nodiff-field-explicit-ctor-lib";

public struct Value : IDifferentiable
{
    public float x = 0.0;
    public no_diff uint tag = 0;
    public __init() {}
}
