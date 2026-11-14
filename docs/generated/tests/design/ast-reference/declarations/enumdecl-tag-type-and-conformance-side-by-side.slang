//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=251490201dcfc0b19979a5dc8ef85362085273b02d52eb3644361b8c66763bf0
//META: purpose=An enum's tag type is written where a base list would go and is parsed by the same inheritance clause a struct uses, so a tag type and an interface conformance can arrive side by side in one list and both take effect.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `enum Level : uint8_t, ILabeled` is one inheritance clause
// carrying two different things. The test reads both back: `sizeof(Level)` is
// 1 only if the `uint8_t` entry was picked out of the list as the tag type,
// and `viaInterface(a)` type-checks only if the `ILabeled` entry of the same
// list was kept as an ordinary conformance. Dropping `, ILabeled` from the
// list makes the call fail with E38029 even with the extension still present,
// so the conformance really does come from the enum's own clause and not from
// the extension.

//TEST:INTERPRET(filecheck=CHECK):

interface ILabeled
{
    int label();
}

enum Level : uint8_t, ILabeled
{
    Low = 1,
    High = 200,
}

extension Level : ILabeled
{
    int label() { return int(this) + 1; }
}

int viaInterface<T : ILabeled>(T x)
{
    return x.label();
}

void main()
{
    Level a = Level.High;
    printf("tag=%d size=%d iface=%d\n", int(a), int(sizeof(Level)), viaInterface(a));
}

//CHECK: tag=200 size=1 iface=201
