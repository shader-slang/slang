// Regression coverage for shader-slang/slang#12525: a statement-form
// `__intrinsic_asm` l-value argument must be lowered to a value, not stored as
// a raw address.
//
// Bare `-dump-ir` (rather than `-dump-ir-after <pass>`) is deliberate: `test`
// is never called, so it is DCE'd before any named pass runs, and its
// `GenericAsm` and the operand under test exist only in the initial
// LOWER-TO-IR dump. FileCheck matches the first occurrence in output order, so
// the checks bind to that lowering dump.

//TEST:SIMPLE(filecheck=CHECK):-dump-ir

struct S { int x; }

// `inout` makes `s` an l-value, so `s.x` lowers through `get_field_addr` (a
// `Ptr`-flavored value); a by-value parameter would already be a `get_field`
// value and would not exercise the fix.
void test(inout S s)
{
    // CHECK-LABEL: func %test
    // CHECK: [[LOADED:%[0-9]+]]{{.*}}= load(
    // CHECK: GenericAsm("USE", [[LOADED]])
    __intrinsic_asm "USE", s.x;
}
