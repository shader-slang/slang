//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:31:34+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/architecture/module-map.md#sourceslang-core-module-embedded-core-module
//META: doc_section_digest=782123943bdd2753d299c8ccb3ce6761a512f811a2aaa435c5b76200a573af07
//META: purpose=The core module is compiled into libslang; built-in vector types like `int4` and `float3` are available without any import.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the consequence of the embedded core module: built-in
// vector types and their constructors are visible to user code with
// no `import` statement. If the core module were not embedded into
// libslang, these names would be unresolved.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int4 v = int4(1, 2, 3, 4);
    float3 f = float3(1.5, 2.5, 3.5);
    //CHECK: 10
    printf("%d\n", v.x + v.y + v.z + v.w);
    //CHECK: 7
    printf("%d\n", int(f.x + f.y + f.z));
}
