//TEST:SIMPLE_EX: -lang hlsl tests/parser/cpp-style-trailing-this-const.slang -no-codegen

// C++-style trailing `const` follows the parameter list in both `T f(...)` and
// `func f(...) -> T` declarations. These cases cover the tokens that may immediately follow the
// modifier. Separate semantic tests cover how trailing `const` changes the receiver mode.

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

    int traditionalThrowing() const throws MethodError
    {
        return 0;
    }

    func modernThrowing() const throws MethodError -> int
    {
        return 0;
    }
};
