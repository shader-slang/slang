//TEST:SIMPLE_EX: -lang hlsl tests/parser/cpp-style-trailing-this-const.slang -no-codegen

// C++-style trailing `const` follows the parameter list in both traditional and modern function
// declarations. These cases cover the tokens that may immediately follow the modifier; semantic
// tests cover its receiver-mode effect.

interface IValue
{}

enum MethodError
{
    Failed
};

struct Example
{
    T constrained<T>(T value) const where T : IValue
    {
        return value;
    }

    float4 withSemantic() const : SV_Target
    {
        return 0;
    }

    func modern() const -> int
    {
        return 0;
    }

    func modernConstrained<T>(value: T) const -> T where T : IValue
    {
        return value;
    }

    int traditionalThrowing() const throws MethodError
    {
        return 0;
    }

    func modernThrowing() const throws MethodError -> int
    {
        return 0;
    }
};
