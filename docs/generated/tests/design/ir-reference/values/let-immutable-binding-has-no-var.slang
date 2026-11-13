//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:26+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/values.md#var-vs-globalvar-vs-alloca
//META: doc_section_digest=015fc32433b67b42977bcc4cdd1f50489d71e13c08f3ea9e36824d7ef792babc
//META: purpose=An initialized immutable `let` lowers to its initializer's SSA value with no `var`, while a `var` that needs storage (here, one passed as `inout`) lowers to a `var` instruction.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the refreshed `var` paragraph: local variables that need
// storage become `var`, but an initialized immutable `let` lowers to
// its initializer's SSA value. `a` is a `let` initialized from a buffer
// load; `c` is a `var` passed by `inout`, so it must live in memory.
// CHECK asserts the `add` that computes `a` appears with no `var`
// before the `var` named `c`, that `c` is the (only) `var` in `main`,
// and that the later multiply consumes the `add` result directly.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[noinline]
void bump(inout int x) { x += 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let a = buf[0] + 1;
    var c = buf[1];
    bump(c);
    buf[tid.x] = a * c;
}

// IR-LABEL: func %main
// IR: let %[[ADD:[0-9]+]]{{.*}}= add(%{{[0-9]+}}, 1 : Int)
// IR-NOT: = var
// IR: [nameHint("c")]
// IR-NEXT: let %c{{.*}}: Ptr(Int){{.*}}= var
// IR: call %bump(%c)
// IR-NOT: = var
// IR: mul(%[[ADD]], %{{[0-9]+}})
