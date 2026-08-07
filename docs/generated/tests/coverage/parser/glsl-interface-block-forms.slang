//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the three GLSL interface-block forms the declaration parser distinguishes under -allow-glsl: a `uniform` block becomes a constant buffer, an `out` block becomes an entry-point output, and an `out gl_*` redeclaration is dropped.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: under `-allow-glsl`, `ParseDeclWithModifiers` checks the
// already-parsed modifier list for `uniform` / `in` / `out` and, if the next
// two tokens look like `Name {`, takes the interface-block path instead of the
// ordinary declarator path. Which modifier it found decides what gets built:
//
//  - `uniform Blk { ... } name;` is routed to the constant-buffer parser with
//    the block's data layout, so it lowers exactly like `cbuffer`. The `std140`
//    qualifier reaches emission as the `_std140` suffix on the generated
//    parameter-group type name.
//  - `out Blk { ... } name;` is parsed as a plain buffer block, and the fields
//    become entry-point varying outputs -- `vOut.color` here.
//  - a redeclaration of a builtin block is detected purely by the block name
//    starting with `gl_`; the parsed declaration is then thrown away and
//    replaced with an `EmptyDecl`, leaving the compiler's own `gl_Position` in
//    place. That is why no `gl_PerVertex` type appears in the output while
//    `gl_Position` still does.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -allow-glsl -entry main -stage vertex

#version 450

layout(std140, binding = 0) uniform MyUniforms { vec4 color; } u;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out VertexOut { vec4 color; } vOut;

void main()
{
    vOut.color = u.color;
    gl_Position = u.color;
}

//CHECK: OpEntryPoint Vertex %main "main" {{.*}}%u %entryPointParam_main_vOut_color %gl_Position
//CHECK: OpName %SLANG_ParameterGroup_MyUniforms_std140
//CHECK: OpName %entryPointParam_main_vOut_color
//CHECK: OpDecorate %gl_Position BuiltIn Position
