//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-signature.slang -no-codegen

// Overload-candidate diagnostics must print trailing `const` after the parameter list, not as a
// prefix declaration modifier.

struct Argument
{};

struct Different
{};

struct Example
{
    int select(Argument value) const
//CHECK:^^^^^^ argument 0 does not match: expected 'Argument', got 'Different'
    {
        return 0;
    }

    int select(int value) const
//CHECK:^^^^^^ argument 0 does not match: expected 'int', got 'Different'
    {
        return value;
    }
};

void useCandidateDiagnostic(Example example, Different different)
{
    example.select(different);
}

//CHECK: no overload for 'select' applicable to arguments of type (Different)
//CHECK: candidate: func Example.select(Argument) const -> int
//CHECK: candidate: func Example.select(int) const -> int
