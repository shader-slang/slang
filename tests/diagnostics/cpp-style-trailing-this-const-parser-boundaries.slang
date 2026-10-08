//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CONSTRUCTOR): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=1 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=ACCESSOR): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=2 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SUBSCRIPT): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-parser-boundaries.slang -DTEST_CASE=3 -no-codegen

// The ordinary function-declaration grammar accepts C++-style trailing `const` after a parameter
// list. Constructor, accessor, and subscript declarations reject it.

#if TEST_CASE == 1
struct ConstructorExample
{
    __init() const
/*CONSTRUCTOR:
             ^^^^^ unexpected token
             ^^^^^ unexpected identifier, expected '{'
*/
    {}
};
#endif

#if TEST_CASE == 2
struct AccessorExample
{
    property int value
    {
        get() const
//ACCESSOR:   ^^^^^ unexpected token
//ACCESSOR:   ^^^^^ unexpected identifier, expected ';'
        {
            return 0;
        }
    }
};
#endif

#if TEST_CASE == 3
struct SubscriptExample
{
    __subscript(int index) const -> int
//SUBSCRIPT:   ^ __subscript declaration must have a return type specified after '->'
//SUBSCRIPT:               ^^^^^ unexpected token
//SUBSCRIPT:               ^^^^^ unexpected identifier, expected ';'
    {
        get
        {
            return index;
        }
    }
};
#endif
