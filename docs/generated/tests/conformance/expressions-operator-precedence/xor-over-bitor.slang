//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-9 `^` binds more tightly than level-10 `|`: `5 ^ 3 | 8` groups as `(5^3)|8` = `6|8` = 14, not `5^(3|8)` = `5^11` = 14 (coincidentally same — use different values).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `^` (xor) is level 9, `|` (bitwise or) is level 10.
// Level 9 is lower numeric = higher precedence, so `^` binds before `|`.
// Use values where the two groupings differ:
// `6 ^ 2 | 1`: xor first = `(6^2)|1` = `4|1` = 5
//              or first  = `6^(2|1)` = `6^3` = 5 (coincidentally same)
// Better: `5 ^ 6 | 4`:
//   xor first: `(5^6)|4` = `3|4` = 7
//   or first:  `5^(6|4)` = `5^6` = 3

//TEST:INTERPRET(filecheck=CHECK):

int xor_or(int a, int b, int c) { return a ^ b | c; }

void main()
{
    // (5^6)|4 = 3|4 = 7
    // 5^(6|4) = 5^6 = 3
    int result = xor_or(5, 6, 4);
    // CHECK: xor-over-bitor: 7
    printf("xor-over-bitor: %d\n", result);
}
