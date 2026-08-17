//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=A declarator name that is a reserved type keyword parses but draws the W20103 keyword-used-as-name warning uniformly across var, typedef, field, parameter, and let declarations.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the keyword-used-as-name check sits on the shared declarator
// path, so all five declaration shapes are covered without a per-kind rule.
// One file names `enum`, `typedef`, `typealias`, `struct`, and `class` as the
// declarator in a global var, a typedef alias, a struct field, a function
// parameter, and a `let` binding; each CHECK pins E20103 at warning severity
// on that declarator's caret span. If the check lived on only one declaration
// path, four of the five sites would be silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static int enum = 0;
//CHECK:   ^^^^ warning E20103
//CHECK:   ^^^^ 'enum' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

typedef int typedef;
//CHECK:    ^^^^^^^ warning E20103
//CHECK:    ^^^^^^^ 'typedef' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

struct Holder
{
    int typealias;
//CHECK:^^^^^^^^^ warning E20103
//CHECK:^^^^^^^^^ 'typealias' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
}

int useParam(int struct)
//CHECK:         ^^^^^^ warning E20103
//CHECK:         ^^^^^^ 'struct' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
{
    return 0;
}

void main()
{
    let class = 3;
//CHECK:^^^^^ warning E20103
//CHECK:^^^^^ 'class' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
}
