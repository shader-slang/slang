//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#body
//META: doc_section_digest=fdc286f331c0c685dd422ee1d30a0c012594b17c6e2b446f9862de4629795eb8
//META: purpose=A function declaration without a body (forward declaration) is allowed and is terminated with a semicolon.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a body-less function declaration (forward declaration, terminated with `;`) compiles.
// `func getCount() -> int;` declares without a body; the subsequent definition satisfies it.
// Emitting `void main` (asserted below) confirms the program compiled successfully.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

func getCount() -> int;

int getCount() { return 42; }

[numthreads(1, 1, 1)]
void main() {}

// CHECK: void main
