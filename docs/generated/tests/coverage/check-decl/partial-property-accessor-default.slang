//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an interface `property` with a body on only one accessor is rejected with E30318, and that giving every accessor a body is rejected too (E30317).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: characterization-unverified=true
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: subscript accessor defaults inside an interface are
// represented by the parser as an `InterfaceDefaultImplDecl`, but properties have
// no such representation, so `checkInterfaceRequirement` detects the mixed shape
// itself via `isPartialPropertyAccessorDefaultImplementation`: some sibling
// accessor has a body and some does not. Property `p` gives `set` a body while
// leaving `get` body-less and gets the targeted E30318. Property `q` follows
// E30318's own advice and gives *every* accessor a body, and CHECK pins that
// this is rejected as well, one diagnostic per accessor.
// characterization-unverified: E30318 tells the author to "define all accessors
// or none", but the `q` case shows that advice does not actually produce an
// accepted declaration for a property; see finding
// `check-decl-property-accessor-default-advice-unactionable`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

interface IProp
{
        property int p { get; set { } }
//CHECK:                      ^^^ E30318
//CHECK:                      ^^^ interface accessor default implementation is incomplete; define all accessors or none.

        property int q { get { return 0; } set { } }
//CHECK:                 ^^^ E30317
//CHECK:                 ^^^ non-method interface requirement cannot have a body.
//CHECK:                                   ^^^ E30317
//CHECK:                                   ^^^ non-method interface requirement cannot have a body.
}
