//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
//META: purpose=`CompileTimeForStmt` parses as `'$' 'for' '(' IDENT 'in' 'Range' '(' Expr (',' Expr)? ')' ')' Stmt`, where one argument means `Range(end)`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both arms of the optional second `Range` argument in one
// production. The two-argument form `Range(0, 4)` must iterate the
// half-open interval, so summing the loop variable gives 0+1+2+3 = 6;
// the one-argument form `Range(3)` must be read as `Range(end)`, so
// counting iterations gives 3. Wrong arity handling in either arm
// changes the printed number.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    $for (i in Range(0, 4))
    {
        sum += i;
    }

    int count = 0;
    $for (j in Range(3))
    {
        count += 1;
    }

    // CHECK: sum=6 count=3
    printf("sum=%d count=%d\n", sum, count);
}
