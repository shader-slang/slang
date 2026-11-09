//TEST:SIMPLE(filecheck=CODE): -target hgbc -entry main -validate-ir -g1
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// The loop's required break block has no executable predecessor: every exit returns.
// Likewise, both branches of choose return before reaching the if-else merge block.
[noinline]
uint countUntil(uint limit)
{
    uint count = 0;
    for (;;)
    {
        if (count == limit)
            return count;
        ++count;
    }
}

[noinline]
uint choose(bool first, uint limit)
{
    if (first)
        return countUntil(limit);
    else
        return countUntil(limit + 1);
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    uint three = hgbc_asm { result : $$uint = OpIntLit 3; };
    print(choose(true, zero));
    print(choose(true, three));
    print(choose(false, three));
}

// CODE: OpBranch {{%[A-Za-z0-9_]+}}
// CODE: OpBranchConditional

// CHECK: standard output = {
// CHECK-NEXT: 0
// CHECK-NEXT: 3
// CHECK-NEXT: 4
