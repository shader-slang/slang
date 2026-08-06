//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a conjunction constraint `where T : IA & IB` is split into one constraint per conjunct so the generic body can call both interfaces' methods.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `visitGenericTypeConstraintDecl` flattens an `AndType`
// super-type into one `GenericTypeConstraintDecl` per conjunct, rewriting the
// original decl to the first conjunct and appending fresh sibling constraints
// for the rest. The body of `both` calls a method from each interface, which is
// only resolvable if both conjuncts became real constraints, and the call site
// must satisfy both to specialize. CHECK pins the interpreted result 3 + 4.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

int both<T>(T v) where T : IA & IB
{
        return v.a() + v.b();
}

struct S : IA, IB
{
        int a() { return 3; }
        int b() { return 4; }
}

void main()
{
        // CHECK: sum=7
        printf("sum=%d\n", both(S()));
}
