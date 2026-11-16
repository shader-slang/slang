// A system-value semantic on a field of an entry-point varying struct is silently
// discarded when an enclosing declaration carries its own semantic: the parameter
// binding "outer-most semantic wins" policy applies the enclosing semantic to every
// leaf, so `float depth : SV_Depth` inside a result struct whose entry point is
// declared `main() : SV_Target` binds as an ordinary Location-1 color output and the
// depth output vanishes. For user-defined field semantics that composition is the
// point of the policy (`o.a -> OUTPUT0, o.b -> OUTPUT1`), but for a system-value
// semantic the override silently changes shader behavior, so it is diagnosed
// (E39034). A field semantic that designates the same slot the enclosing semantic
// would assign anyway (e.g. `SV_Target0` under an enclosing `SV_Target`) is exempt,
// because the layout is identical either way.
//
// The check is evaluated once per declaration, not once per visit: the array walk
// reaches a struct field once per array element with an advanced slot counter, and
// the tuple walk (an `Optional<T>` parameter) re-uses the enclosing declaration as
// every member's decl. The ARRAY run pins the once-per-declaration behavior (an
// array-of-struct output must yield exactly one error, and the exhaustive annotation
// check fails on any unannotated second diagnostic); ARRBENIGN and OPTIONAL pin the
// corresponding no-diagnostic spellings.
//
// The positive controls use `//TEST:SIMPLE(filecheck=...)` runs rather than
// DIAGNOSTIC_TEST runs on purpose: under DIAGNOSTIC_TEST a `//<prefix>-NOT:` line is
// parsed as an ordinary comment (the annotation parser only recognizes exactly
// `//<prefix>:`), so expressing "no E39034" as a diag run would assert nothing. The
// positive `OpEntryPoint` / target-text matches are what give the `-NOT:` lines
// force: compilation failing would starve them.

//DIAGNOSTIC_TEST:SIMPLE(diag=RET):-target spirv -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=RET_HLSL):-target hlsl -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=RET_GLSL):-target glsl -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTPARAM):-target spirv -entry outParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED):-target spirv -entry nested -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=USEROUTER):-target spirv -entry userOuter -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=INPUTDIR):-target spirv -entry fragIn -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=VERTOUT):-target spirv -entry vertexOut -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=NVOUT):-target spirv -entry nvOut -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAY):-target spirv -entry arrayConflict -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=SHIFTED):-target spirv -entry shifted -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTADOPT):-target spirv -entry nestedAdopt -stage fragment
//TEST:SIMPLE(filecheck=REDUNDANT):-target spirv-asm -entry redundant -stage fragment
//TEST:SIMPLE(filecheck=INDEXED):-target spirv-asm -entry indexed -stage fragment
//TEST:SIMPLE(filecheck=ARRBENIGN):-target hlsl -entry arrayBenign -stage fragment
//TEST:SIMPLE(filecheck=OPTIONAL):-target spirv-asm -entry optionalParam -stage fragment
//TEST:SIMPLE(filecheck=ALLOWED):-target spirv-asm -entry allowCase -stage fragment
//TEST:SIMPLE(filecheck=ALLOWALL):-target spirv-asm -entry allowAll -stage fragment

struct FSOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
//RET: E39034
//RET:            ^^^^^^^^ this system-value semantic would be ignored
//RET_HLSL: E39034
//RET_HLSL:       ^^^^^^^^ this system-value semantic would be ignored
//RET_GLSL: E39034
//RET_GLSL:       ^^^^^^^^ this system-value semantic would be ignored
//OUTPARAM: E39034
//OUTPARAM:       ^^^^^^^^ this system-value semantic would be ignored
//ARRAY: E39034
//ARRAY:          ^^^^^^^^ this system-value semantic would be ignored
};

// Entry-point result semantic overrides the `SV_Depth` field semantic.
[shader("fragment")]
FSOut retStruct(float4 pos : SV_Position)
    : SV_Target
//RET:^^^^^^^^^ the enclosing semantic is declared here
/*RET_HLSL:
      ^^^^^^^^^ the enclosing semantic is declared here
*/
/*RET_GLSL:
      ^^^^^^^^^ the enclosing semantic is declared here
*/
{
    FSOut o;
    o.color = pos;
    o.depth = pos.x;
    return o;
}

// The same clobber through an `out` parameter instead of the return value, and
// through an array of the struct: the check must fire exactly once per declaration
// regardless of how many array elements re-walk it.
[shader("fragment")]
void outParam(float4 pos : SV_Position, out FSOut o : SV_Target)
//OUTPARAM:                                           ^^^^^^^^^ the enclosing semantic is declared here
{
    o.color = pos;
    o.depth = pos.x;
}

[shader("fragment")]
void arrayConflict(float4 pos : SV_Position, out FSOut arr[2] : SV_Target)
//ARRAY:                                                        ^^^^^^^^^ the enclosing semantic is declared here
{
    arr[0].color = pos;
    arr[0].depth = pos.x;
    arr[1].color = pos;
    arr[1].depth = pos.x;
}

// An array spelling whose field semantic names the starting slot of the array's
// expansion: the enclosing semantic assigns exactly that sequence, so the spelling
// is redundant rather than conflicting and must stay legal.
struct S
{
    float4 c : SV_Target;
};

[shader("fragment")]
void arrayBenign(float4 pos : SV_Position, out S arr[2] : SV_Target)
{
    arr[0].c = pos;
    arr[1].c = pos;
}
//ARRBENIGN-NOT: E39034
//ARRBENIGN: void arrayBenign

struct Inner
{
    float depth : SV_Depth;
//NESTED: E39034
//NESTED:         ^^^^^^^^ this system-value semantic would be ignored
};

struct NestedOut
{
    float4 color : SV_Target;
    Inner inner;
};

// The enclosing result semantic reaches through nested structs and still
// overrides the `SV_Depth` field.
[shader("fragment")]
NestedOut nested(float4 pos : SV_Position)
    : SV_Target
/*NESTED:
      ^^^^^^^^^ the enclosing semantic is declared here
*/
{
    NestedOut o;
    o.color = pos;
    o.inner.depth = pos.x;
    return o;
}

struct DepthOnly
{
    float depth : SV_Depth;
//USEROUTER: E39034
//USEROUTER:      ^^^^^^^^ this system-value semantic would be ignored
};

// A user-defined enclosing semantic is just as much an override: the field's
// `SV_Depth` would become the second COLOR output.
[shader("fragment")]
void userOuter(float4 pos : SV_Position, out DepthOnly o : COLOR)
//USEROUTER:                                               ^^^^^ the enclosing semantic is declared here
{
    o.depth = pos.x;
}

struct FIn
{
    float4 pos : SV_Position;
//INPUTDIR:      ^^^^^^^^^^^ this system-value semantic would be ignored
    float2 uv : TEXCOORD0;
};

// The same override on the input side: the `SV_Position` field of an input struct
// is silently rebound as an ordinary TEXCOORD varying. The `uv` field shows the
// user-defined composition case stays silent.
[shader("fragment")]
float4 fragIn(FIn i : TEXCOORD)
//INPUTDIR:           ^^^^^^^^ the enclosing semantic is declared here
{
    return float4(i.uv, 0.0, 1.0);
}
//INPUTDIR: E39034

struct VSOut
{
    float4 pos : SV_Position;
//VERTOUT:       ^^^^^^^^^^^ this system-value semantic would be ignored
    float2 uv : TEXCOORD0;
};

// The check is not fragment-specific: a vertex-stage result semantic overrides the
// `SV_Position` field the same way.
[shader("vertex")]
VSOut vertexOut(float4 v : POSITION)
    : COLOR
/*VERTOUT:
      ^^^^^ the enclosing semantic is declared here
*/
{
    VSOut o;
    o.pos = v;
    o.uv = v.xy;
    return o;
}
//VERTOUT: E39034

struct NVOut
{
    uint4 mask : NV_VIEWPORT_MASK;
//NVOUT:         ^^^^^^^^^^^^^^^^ this system-value semantic would be ignored
};

// An `NV_`-prefixed field semantic is classified as a system value just like `SV_`.
[shader("vertex")]
NVOut nvOut(float4 v : POSITION)
    : COLOR
/*NVOUT:
      ^^^^^ the enclosing semantic is declared here
*/
{
    NVOut o;
    o.mask = uint4(1, 0, 0, 0);
    return o;
}
//NVOUT: E39034

// An `Optional<T>` parameter expands to a tuple whose members are laid out with the
// parameter's own decl; the enclosing semantic it adopted is not "overriding" it, so
// no diagnostic is due.
[shader("fragment")]
void optionalParam(float4 pos : SV_Position, out Optional<float4> o : SV_Target)
{
    o = pos;
}
//OPTIONAL-NOT: E39034
//OPTIONAL: OpEntryPoint

// An explicit `[allow(...)]` opts out of the diagnostic for legacy spellings that
// really do want the enclosing semantic to win.
struct FSOutAllowed
{
    float4 color : SV_Target;
    [allow("outer-semantic-overrides-system-value-semantic")]
    float depth : SV_Depth;
};

[shader("fragment")]
FSOutAllowed allowCase(float4 pos : SV_Position)
    : SV_Target
{
    FSOutAllowed o;
    o.color = pos;
    o.depth = pos.x;
    return o;
}
//ALLOWED-NOT: E39034
//ALLOWED: OpEntryPoint

// The boundary between "redundant" and "conflict" inside the exemption: same
// base name as the enclosing semantic, but the field's own index names slot 0
// while the counter has advanced past it, so the index conjunct must reject.
struct ShiftedOut
{
    float4 a : COLOR0;
    float4 b : SV_Target;
//SHIFTED: E39034
//SHIFTED:     ^^^^^^^^^ this system-value semantic would be ignored
};

[shader("fragment")]
ShiftedOut shifted(float4 pos : SV_Position)
    : SV_Target
/*SHIFTED:
      ^^^^^^^^^ the enclosing semantic is declared here
*/
{
    ShiftedOut o;
    o.a = pos;
    o.b = pos;
    return o;
}

// A semantic can also be adopted at a nested struct field rather than the entry
// point; the walk's adopting frame is then the field, and the same override
// verdict applies beneath it.
struct Inner2
{
    float depth : SV_Depth;
//NESTADOPT: E39034
//NESTADOPT:      ^^^^^^^^ this system-value semantic would be ignored
};

struct Outer2
{
    Inner2 i : TEXCOORD0;
//NESTADOPT:   ^^^^^^^^^ the enclosing semantic is declared here
};

[shader("fragment")]
void nestedAdopt(float4 pos : SV_Position, out Outer2 o)
{
    o.i.depth = pos.x;
}

// `[allow]` placed on the enclosing struct, not the field: the suppression
// lookup walks the parent chain, so one attribute silences every nested
// field.
[allow("outer-semantic-overrides-system-value-semantic")]
struct FSOutAllowAll
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

[shader("fragment")]
FSOutAllowAll allowAll(float4 pos : SV_Position)
    : SV_Target
{
    FSOutAllowAll o;
    o.color = pos;
    o.depth = pos.x;
    return o;
}
//ALLOWALL-NOT: E39034
//ALLOWALL: OpEntryPoint

struct SingleOut
{
    float4 c : SV_Target;
};

// Redundant but harmless: the field's own `SV_Target` names the same slot the
// enclosing result semantic assigns (SV_Target0), so no diagnostic and identical
// layout.
[shader("fragment")]
SingleOut redundant(float4 pos : SV_Position)
    : SV_Target
{
    SingleOut o;
    o.c = pos;
    return o;
}
//REDUNDANT-NOT: E39034
//REDUNDANT-DAG: OpEntryPoint
//REDUNDANT-DAG: OpDecorate %entryPointParam_redundant_c Location 0

struct IndexedOut
{
    float4 a : SV_Target;
    float4 b : SV_Target1;
};

// Each field's own indexed semantic names the same slot the enclosing result
// semantic assigns (`a` lands on index 0, `b` on index 1), so the layout is
// identical with or without the field semantics, and no diagnostic is due.
[shader("fragment")]
IndexedOut indexed(float4 pos : SV_Position)
    : SV_Target
{
    IndexedOut o;
    o.a = pos;
    o.b = pos;
    return o;
}
//INDEXED-NOT: E39034
//INDEXED-DAG: OpEntryPoint
//INDEXED-DAG: OpDecorate %entryPointParam_indexed_a Location 0
//INDEXED-DAG: OpDecorate %entryPointParam_indexed_b Location 1
