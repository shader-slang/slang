//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`IfLetStmt` parses as `'if' '(' 'let' IDENT '=' Expr ')' Stmt ('else' Stmt)?`, selected over `IfStmt` when `let` follows the `(`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `let` two tokens after `if` selects the binding form
// rather than an ordinary condition, and that the binding is in scope
// in the then-branch. Both arms are exercised from locals: the bound
// arm reads the payload through the new name (42 could not be produced
// without the binding), and the else arm proves the empty case takes
// the alternative statement.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> some = 41;
    Optional<int> nothing = none;

    int bound = 0;
    if (let v = some) { bound = v + 1; }
    else { bound = -1; }

    int empty = 0;
    if (let v = nothing) { empty = v + 1; }
    else { empty = -1; }

    // CHECK: bound=42 empty=-1
    printf("bound=%d empty=%d\n", bound, empty);
}
