//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#missingreturn-and-unreachable
//META: doc_section_digest=7e3f42f8fab6cf15c065c860a8c454a04a6aa3b295c1f860ae00e9034b7f4b99
//META: purpose=A value-returning function whose body can fall off the end terminates that block with missingReturn rather than an implicit return_val.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the split between the two UnreachableBase terminators: the
// fall-off-the-end block of a value-returning function gets `missingReturn`,
// not `unreachable` and not a fabricated return value, so a later dataflow
// check still has something to complain about. The helper here is never
// called, which keeps that check quiet (it only reports when the block is
// reachable from an entry point) while the terminator itself stays visible
// in the dump. The CHECK pins `missingReturn` as the terminator of the
// else-side block of the guarded return.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int fallsOffTheEnd(int v)
{
    if (v > 0)
        return 1;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = n;
}

// CHECK-LABEL: func %fallsOffTheEnd
// CHECK: return_val(1 : Int)
// CHECK: missingReturn
