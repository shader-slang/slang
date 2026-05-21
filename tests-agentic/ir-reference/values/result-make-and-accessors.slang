//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=a4ceb599938f068a1086e6ea24a1acb5127549fc407caedc96c472c16ac1c013
//META: purpose=`Result<T, E>` helpers are observable through Slang's throwing-function surface: a `throws` function lowers `return x;` to `makeResultValue(x)` and `throw e;` to `makeResultError(e)`, while a `try` / `catch` site in the caller lowers to `isResultError`, `getResultValue`, and `getResultError` accessors.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Result / Optional / Conditional helpers table lists five
// related opcodes for the `Result<T, E>` arm of the family:
// `makeResultValue`, `makeResultError`, `isResultError`,
// `getResultValue`, `getResultError`. The doc's "AST origin" column
// cites `Result<T, E>::makeValue` lowering / `Result<T, E>::isError`
// etc., but the natural Slang surface that triggers all five is the
// error-handling surface (`throws` / `throw` / `try` / `catch`): the
// LOWER-TO-IR error-handling pass rewrites `return val;` in a
// throwing function to `return makeResultValue(val);`, rewrites
// `throw e;` to `return makeResultError(e);`, and at every `try
// callee()` site introduces `isResultError`, `getResultValue`, and
// `getResultError` on the call result.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

enum MyError
{
    Fail
}

int maybeBad(int n) throws MyError
{
    if (n == 1)
        throw MyError.Fail;
    return n;
}

[numthreads(1, 1, 1)]
void main()
{
    do
    {
        int x = try maybeBad(a);
        buf[0] = x;
    }
    catch (e: MyError)
    {
        buf[0] = int(e);
    }
}

// IR-LABEL: func %maybeBad
// IR-DAG: makeResultError(0 : Int)
// IR-DAG: makeResultValue(%n)
// IR-LABEL: func %main
// IR-DAG: isResultError(%{{[0-9]+}})
// IR-DAG: getResultError(%{{[0-9]+}})
// IR-DAG: getResultValue(%{{[0-9]+}})
