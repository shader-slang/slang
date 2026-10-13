//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:48:26Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=7bc481f1925fc74337357f76e854855a3f8e8c8a24448fe55a29ec6c5a43f576
//META: purpose=`TrailingThisParamConst` follows a function parameter list before any `throws` clause, return semantic, result type, or `where` clause.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:SIMPLE_EX: -lang hlsl docs/generated/tests/design/syntax-reference/grammar/decl-func-hlsl-trailing-this-const.slang -no-codegen

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
