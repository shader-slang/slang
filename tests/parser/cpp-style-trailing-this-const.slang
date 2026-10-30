//TEST:SIMPLE_EX: -lang hlsl tests/parser/cpp-style-trailing-this-const.slang -no-codegen

// Trailing `const` appears before `where` constraints in both traditional and modern function
// declarations.

interface IValue
{}

struct Example
{
    T traditional<T>(T value) const where T : IValue
    {
        return value;
    }

    func modern<T>(value: T) const -> T where T : IValue
    {
        return value;
    }
};
