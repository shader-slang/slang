//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=NEG):-target spirv-asm -g2 -emit-spirv-directly -O0

// Regression test for shader-slang/slang#11550.
//
// With -g2 -O0 a compiler-synthesized struct initializer (default or member-wise)
// must NOT receive source-level debug info: it has no user-authored body, and its
// source locations are inherited from the struct/member declarations. Previously it
// was given a DebugFunction/DebugScope plus DebugLines on the struct/member decl
// lines, so a debugger could step *into* the synthesized initializer.
//
// The fix discriminates by constructor FLAVOR, not by the "$init" name (a
// user-written `__init` is also mangled to "<Type>.$init"), so:
//   - synthesized initializers (Pair member-wise, DefaultMade default) lose debug info;
//   - the user-written UserMade ctor KEEPS it; main is unaffected.
//
// Check robustness against -g2 embedding the source text (incl. these directives)
// as a DebugSource OpString: quoted names match only the real (unescaped) records,
// and DebugLine checks use resolved @LINE numbers absent from the embedded literal.
// The NEG prefix has only -NOT directives, so each spans the whole disassembly
// (order-independent — not bounded to a window around other matches).

//NEG-NOT: OpString "Pair.$init"
//NEG-NOT: OpString "DefaultMade.$init"

struct Pair
{
    float x;
    float y;
};

struct UserMade
{
    float v;
    __init(float a)
    {
        v = a;
    }
};

struct DefaultMade
{
    float w;
};

RWStructuredBuffer<float> outputBuffer;

// The user-written constructor is named "UserMade.$init" too, but keeps its debug
// name (this fails if the suppression is ever re-keyed on the name instead of the
// flavor). Emitted in the debug-constants section, before any function body.
//CHECK: OpString "UserMade.$init"

// The user-written initializer (UserMade) exposes its object under construction as a
// `this` debug variable (#11565). A synthesized initializer must NOT — so exactly one
// `this` DebugLocalVariable may exist module-wide (UserMade's): the synthesized Pair /
// DefaultMade member-wise/default inits contribute none. A bare CHECK-NOT on OpString
// "this" would be wrong (the user ctor legitimately has one), so bind the name string
// and bound the count instead. This pins the synthesized-init exclusion — the #11550
// regression class — against a `this` var ever leaking into compiler-generated code.
//CHECK: [[THIS_STR:%[0-9]+]] = OpString "this"
//CHECK-COUNT-1: DebugLocalVariable [[THIS_STR]]
//CHECK-NOT: DebugLocalVariable [[THIS_STR]]

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p = {1.0, 2.0};
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
    UserMade u = UserMade(3.0);
    DefaultMade d = {};
    outputBuffer[tid.x] = p.x + p.y + u.v + d.w;
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
}
