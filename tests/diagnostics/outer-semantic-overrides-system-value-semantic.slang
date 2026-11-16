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

//DIAGNOSTIC_TEST:SIMPLE(diag=RET):-target spirv -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=RET_HLSL):-target hlsl -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=RET_GLSL):-target glsl -entry retStruct -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTPARAM):-target spirv -entry outParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED):-target spirv -entry nested -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=USEROUTER):-target spirv -entry userOuter -stage fragment
// Positive controls: spellings that designate the same slot the enclosing semantic
// assigns must keep compiling (no E39034), and real SPIR-V must come out.
//TEST:SIMPLE(filecheck=REDUNDANT):-target spirv-asm -entry redundant -stage fragment
//TEST:SIMPLE(filecheck=INDEXED):-target spirv-asm -entry indexed -stage fragment

struct FSOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
//RET: E39034
//RET:            ^^^^^^^^ this system-value semantic would be silently overridden
//RET_HLSL: E39034
//RET_HLSL:       ^^^^^^^^ this system-value semantic would be silently overridden
//RET_GLSL: E39034
//RET_GLSL:       ^^^^^^^^ this system-value semantic would be silently overridden
//OUTPARAM: E39034
//OUTPARAM:       ^^^^^^^^ this system-value semantic would be silently overridden
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

// The same clobber through an `out` parameter instead of the return value.
[shader("fragment")]
void outParam(float4 pos : SV_Position, out FSOut o : SV_Target)
//OUTPARAM:                                           ^^^^^^^^^ the enclosing semantic is declared here
{
    o.color = pos;
    o.depth = pos.x;
}

struct Inner
{
    float depth : SV_Depth;
//NESTED: E39034
//NESTED:         ^^^^^^^^ this system-value semantic would be silently overridden
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
//USEROUTER:      ^^^^^^^^ this system-value semantic would be silently overridden
};

// A user-defined enclosing semantic is just as much an override: the field's
// `SV_Depth` would become the second COLOR output.
[shader("fragment")]
void userOuter(float4 pos : SV_Position, out DepthOnly o : COLOR)
//USEROUTER:                                               ^^^^^ the enclosing semantic is declared here
{
    o.depth = pos.x;
}

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
//REDUNDANT: OpEntryPoint

struct IndexedOut
{
    float4 a : SV_Target;
    float4 b : SV_Target1;
};

// Each field's own indexed semantic matches the slot the enclosing result
// semantic assigns (`a` consumes index 0, so `b` lands on index 1), so both are
// honored and no diagnostic is due.
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
//INDEXED: OpEntryPoint
