//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-signature.slang -no-codegen

// Candidate diagnostics print trailing `const` after both ordinary and generic parameter lists.

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

    int genericSelect<T>(T key, Argument value) const
//CHECK:^^^^^^^^^^^^^ argument 1 does not match: expected 'Argument', got 'Different'
    {
        return 0;
    }

    int genericSelect<T>(T key, int value) const
//CHECK:^^^^^^^^^^^^^ argument 1 does not match: expected 'int', got 'Different'
    {
        return value;
    }
};

void useCandidateDiagnostic(Example example, Different different)
{
    example.select(different);
    example.genericSelect(0, different);
}

//CHECK: no overload for 'select' applicable to arguments of type (Different)
//CHECK: candidate: func Example.select(Argument) const -> int
//CHECK: candidate: func Example.select(int) const -> int
//CHECK: no overload for 'genericSelect' applicable to arguments of type (int, Different)
//CHECK: candidate: func Example.genericSelect<int>(int, Argument) const -> int
//CHECK: candidate: func Example.genericSelect<int>(int, int) const -> int
