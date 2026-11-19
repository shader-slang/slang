// A chain of exported extern constants deeper than the fixed pass budget in
// `ComponentType::tryFoldIntVal` must end unresolved — one layout-time binding
// diagnostic — rather than looping. (A literal self-referential extern crashes IR
// lowering in a pre-existing recursion tracked as issue #14556; the deep chain here
// pins the same property the pass cap exists to guarantee: bounded resolution,
// deterministic outcome.)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

export extern static const int E0 = 1;
export extern static const int E1 = E0 + 1;
export extern static const int E2 = E1 + 1;
export extern static const int E3 = E2 + 1;
export extern static const int E4 = E3 + 1;
export extern static const int E5 = E4 + 1;
export extern static const int E6 = E5 + 1;
export extern static const int E7 = E6 + 1;
export extern static const int E8 = E7 + 1;
export extern static const int E9 = E8 + 1;
export extern static const int E10 = E9 + 1;
export extern static const int E11 = E10 + 1;
export extern static const int E12 = E11 + 1;
export extern static const int E13 = E12 + 1;
export extern static const int E14 = E13 + 1;
export extern static const int E15 = E14 + 1;
export extern static const int E16 = E15 + 1;
export extern static const int E17 = E16 + 1;
export extern static const int E18 = E17 + 1;
export extern static const int E19 = E18 + 1;
export extern static const int E20 = E19 + 1;
export extern static const int E21 = E20 + 1;
export extern static const int E22 = E21 + 1;
export extern static const int E23 = E22 + 1;
export extern static const int E24 = E23 + 1;
export extern static const int E25 = E24 + 1;
export extern static const int E26 = E25 + 1;
export extern static const int E27 = E26 + 1;
export extern static const int E28 = E27 + 1;
export extern static const int E29 = E28 + 1;
export extern static const int E30 = E29 + 1;
export extern static const int E31 = E30 + 1;
export extern static const int E32 = E31 + 1;
export extern static const int E33 = E32 + 1;
export extern static const int E34 = E33 + 1;
export extern static const int E35 = E34 + 1;
export extern static const int E36 = E35 + 1;
export extern static const int E37 = E36 + 1;
export extern static const int E38 = E37 + 1;
export extern static const int E39 = E38 + 1;
export extern static const int E40 = E39 + 1;
export extern static const int E41 = E40 + 1;
export extern static const int E42 = E41 + 1;
export extern static const int E43 = E42 + 1;
export extern static const int E44 = E43 + 1;
export extern static const int E45 = E44 + 1;
export extern static const int E46 = E45 + 1;
export extern static const int E47 = E46 + 1;
export extern static const int E48 = E47 + 1;
export extern static const int E49 = E48 + 1;
export extern static const int E50 = E49 + 1;
export extern static const int E51 = E50 + 1;
export extern static const int E52 = E51 + 1;
export extern static const int E53 = E52 + 1;
export extern static const int E54 = E53 + 1;
export extern static const int E55 = E54 + 1;
export extern static const int E56 = E55 + 1;
export extern static const int E57 = E56 + 1;
export extern static const int E58 = E57 + 1;
export extern static const int E59 = E58 + 1;
export extern static const int E60 = E59 + 1;
export extern static const int E61 = E60 + 1;
export extern static const int E62 = E61 + 1;
export extern static const int E63 = E62 + 1;
export extern static const int E64 = E63 + 1;
export extern static const int E65 = E64 + 1;
export extern static const int E66 = E65 + 1;
export extern static const int E67 = E66 + 1;
export extern static const int E68 = E67 + 1;
export extern static const int E69 = E68 + 1;
export extern static const int E70 = E69 + 1;
export extern static const int E71 = E70 + 1;
export extern static const int E72 = E71 + 1;
export extern static const int E73 = E72 + 1;
export extern static const int E74 = E73 + 1;
export extern static const int E75 = E74 + 1;
export extern static const int E76 = E75 + 1;
export extern static const int E77 = E76 + 1;
export extern static const int E78 = E77 + 1;
export extern static const int E79 = E78 + 1;
export extern static const int E80 = E79 + 1;
export extern static const int E81 = E80 + 1;
export extern static const int E82 = E81 + 1;
export extern static const int E83 = E82 + 1;
export extern static const int E84 = E83 + 1;
export extern static const int E85 = E84 + 1;
export extern static const int E86 = E85 + 1;
export extern static const int E87 = E86 + 1;
export extern static const int E88 = E87 + 1;
export extern static const int E89 = E88 + 1;
export extern static const int E90 = E89 + 1;
export extern static const int E91 = E90 + 1;
export extern static const int E92 = E91 + 1;
export extern static const int E93 = E92 + 1;
export extern static const int E94 = E93 + 1;
export extern static const int E95 = E94 + 1;
export extern static const int E96 = E95 + 1;
export extern static const int E97 = E96 + 1;
export extern static const int E98 = E97 + 1;
export extern static const int E99 = E98 + 1;

[[vk::binding(E99, 1)]]
/*CHECK:
      ^^^^^^^ binding attribute requires a concrete value
      ^^^^^^^ an argument of '[[vk::binding(...)]]' or the GLSL 'binding'/'set' layout qualifier is a link-time constant that is not a concrete value here; provide a resolvable default, link a module that defines it, or shorten a chain of definitions that is too deep to resolve
*/
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    result[threadID.x] = threadID.x;
}
