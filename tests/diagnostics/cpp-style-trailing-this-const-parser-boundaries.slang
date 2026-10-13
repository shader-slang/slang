//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CTOR): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=1 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=ACCESSOR): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=2 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SUBSCRIPT): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=3 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=FUNC_EXTENSION): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=4 -experimental-feature -no-codegen

// The parser accepts C++-style trailing `const` only after a `FuncDecl` parameter list. For these
// other callable declarations, it diagnoses the misplaced suffix and resumes at the following body
// or result type without producing unrelated recovery diagnostics.

#if TEST_CASE == 1
struct ConstructorExample
{
    __init() const
//CTOR:      ^^^^^ trailing `const` is not valid on this declaration
//CTOR:      ^^^^^ trailing `const` is only valid on an ordinary function declaration
    {}
};
#endif

#if TEST_CASE == 2
struct AccessorExample
{
    property int value
    {
        get() const
//ACCESSOR:   ^^^^^ trailing `const` is not valid on this declaration
//ACCESSOR:   ^^^^^ trailing `const` is only valid on an ordinary function declaration
        {
            return 0;
        }
    }
};
#endif

#if TEST_CASE == 4
float primal(float value)
{
    return value;
}

__func_extension fwd_diff(primal)(DifferentialPair<float> value) const
//FUNC_EXTENSION:                                                ^^^^^ trailing `const` is not valid on this declaration
//FUNC_EXTENSION:                                                ^^^^^ trailing `const` is only valid on an ordinary function declaration
    -> DifferentialPair<float>
{
    return value;
}
#endif

#if TEST_CASE == 3
struct SubscriptExample
{
    __subscript(int index) const -> int
//SUBSCRIPT:               ^^^^^ trailing `const` is not valid on this declaration
//SUBSCRIPT:               ^^^^^ trailing `const` is only valid on an ordinary function declaration
    {
        get
        {
            return index;
        }
    }
};
#endif
