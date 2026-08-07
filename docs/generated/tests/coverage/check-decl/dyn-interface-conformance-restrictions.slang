//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies what may not conform to a `dyn interface`: opaque members (E33080), unsized members (E33079), generic types (E33082), and extensions (E33078).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `validateDynInterfaceUseWithInheritanceDecl` restricts what
// may conform to a `dyn interface` under `-std 2026`. A conformer may not hold
// an opaque or unsized member (the dynamic representation has to be a copyable,
// fixed-size box), may not be generic, and may not gain the conformance through
// an `extension`. Each declaration below trips exactly one of those arms and the
// annotations pin the observed code and message for it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026 -target hlsl

dyn interface IShape
{
        int area();
}

struct WithOpaque : IShape
{
        Texture2D tex;
//CHECK:          ^^^ E33080
//CHECK:          ^^^ cannot have opaque member 'tex' when inheriting from dyn interface 'IShape'.
        int area() { return 1; }
}

struct WithUnsized : IShape
{
        int data[];
//CHECK:    ^^^^ E33079
//CHECK:    ^^^^ cannot have unsized member 'data' when inheriting from dyn interface 'IShape'.
        int area() { return 2; }
}

struct Wrapper<T> : IShape
//CHECK:            ^^^^^^ E33082
//CHECK:            ^^^^^^ cannot conform generic type 'Wrapper' to dyn interface 'IShape'.
{
        int area() { return 3; }
}

extension float : IShape
//CHECK:  ^^^^^ E33078
//CHECK:  ^^^^^ cannot use a extension to conform to a dyn interface 'IShape'.
{
        int area() { return 4; }
}
