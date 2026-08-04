//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=50ea260e6478b155f75a6a10b59f984b9662c43b1424c04c5d8d462fa4d6ec29
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
