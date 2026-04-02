// Verify diagnostic errors for malformed `-conformance` CLI options.
// E71: syntax error (missing `:` separator)
// E73: impossible conformance (ImplA does not conform to IBar)
//
// Note: E72 (type not found) is tested separately in
// tests/diagnostics/type-conformance-invalid-type.slang and
// tests/diagnostics/type-conformance-invalid-interface.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_71): -target spirv -conformance "bad syntax"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_73): -target spirv -conformance "ImplA:IBar=0"

[anyValueSize(8)]
interface IFoo
{
    int get();
}

[anyValueSize(8)]
interface IBar
{
    float calc();
}

struct ImplA : IFoo
{
    int val;
    int get() { return val; }
}

//CHECK_71: syntax error in type conformance option
//CHECK_73: cannot create type conformance
