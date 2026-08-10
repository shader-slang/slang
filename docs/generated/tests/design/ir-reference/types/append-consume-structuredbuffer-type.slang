//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=AppendStructuredBuffer<T> and ConsumeStructuredBuffer<T> uniforms lower to distinct IR type opcodes each carrying the element type and a data-layout operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The append and consume flavours of a structured buffer are separate
// type opcodes even though they share the element-type plus data-layout
// operand shape of the read-write flavour. Both uniforms are declared
// and used in one kernel; the dump is FileChecked for each named
// global_param carrying its own opcode with `Int, DefaultLayout`, which
// shows the surface flavour picks the opcode and the layout marker is
// carried as the second operand.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

AppendStructuredBuffer<int> appendBuf;
ConsumeStructuredBuffer<int> consumeBuf;

[numthreads(1, 1, 1)]
void main()
{
    appendBuf.Append(consumeBuf.Consume());
}

// IR-DAG: let %appendBuf{{.*}}: AppendStructuredBuffer(Int, DefaultLayout{{.*}}= global_param
// IR-DAG: let %consumeBuf{{.*}}: ConsumeStructuredBuffer(Int, DefaultLayout{{.*}}= global_param
