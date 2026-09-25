//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Each input is one complete four-lane truth table. Rotate them across quads.
//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15], stride=4):name=truthTables
StructuredBuffer<uint> truthTables;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
bool anyHelper(bool predicate)
{
    return QuadAny(predicate);
}

[noinline]
bool allHelper(bool predicate)
{
    return QuadAll(predicate);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint pattern = 0; pattern < 16; ++pattern)
    {
        uint table = truthTables[(pattern + lane / 4) % 16];
        bool predicate = ((table >> (lane % 4)) & 1) != 0;
        // These expectations come directly from the truth table, without lane reads or votes.
        bool expectedAny = table != 0;
        bool expectedAll = table == 15;
        ok = (QuadAny(predicate) == expectedAny) && ok;
        ok = (QuadAll(predicate) == expectedAll) && ok;

        // SM80 synchronized shuffles rendezvous across divergent branches. Both branches
        // execute four matching reads from complete source quads, including other branches.
        bool inlineVote;
        if ((lane & 1) != 0)
            inlineVote = QuadAny(predicate);
        else
            inlineVote = QuadAll(predicate);
        ok = (inlineVote == ((lane & 1) != 0 ? expectedAny : expectedAll)) && ok;

        bool helperVote;
        if ((lane & 4) != 0)
            helperVote = anyHelper(predicate);
        else
            helperVote = allHelper(predicate);
        ok = (helperVote == ((lane & 4) != 0 ? expectedAny : expectedAll)) && ok;
    }
    outputBuffer[lane] = uint(ok);

    // Exit complete alternating quads. Surviving quads never read an exited source lane.
    if ((lane & 4) != 0)
        return;
    uint finalTable = truthTables[lane / 4];
    bool finalPredicate = ((finalTable >> (lane % 4)) & 1) != 0;
    ok = (anyHelper(finalPredicate) == (finalTable != 0)) && ok;
    ok = (allHelper(finalPredicate) == (finalTable == 15)) && ok;
    outputBuffer[lane] = uint(ok);
}

//CHECK: type: uint
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
