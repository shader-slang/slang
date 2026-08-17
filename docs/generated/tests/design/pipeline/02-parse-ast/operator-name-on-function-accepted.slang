//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=An `operator <op>` name is legal once a parameter list has confirmed the declarator is a function, so an operator overload declares and dispatches normally.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive side of the operator-name rule, which the negative test
// alone cannot establish: the opt-in only happens on the function branch, so an
// operator declarator that does reach a parameter list must be accepted. The
// overload is then exercised through ordinary infix syntax; the printed 40
// (`(3+5) * 5`) is only produced if the parsed operator declaration became the
// callee of the `+` in `main`.

//TEST:INTERPRET(filecheck=CHECK):

struct Scaled
{
    int v;
}

Scaled operator+(Scaled a, Scaled b)
{
    Scaled r;
    r.v = (a.v + b.v) * 5;
    return r;
}

void main()
{
    Scaled a;
    a.v = 3;
    Scaled b;
    b.v = 5;
    // CHECK: v=40
    printf("v=%d\n", (a + b).v);
}
