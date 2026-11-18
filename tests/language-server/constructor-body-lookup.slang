//TEST:LANG_SERVER(filecheck=CHECK):

uint stepValue(uint value)
{
    return value + 1;
}

struct CtorLookupProbe
{
    uint state;
    __init(uint seed)
    {
        state = stepValue(seed);
    }
    uint evaluate()
    {
        return stepValue(state);
    }
}

uint freeCall()
{
    return stepValue(1);
}

//HOVER:13,18
//HOVER:17,17
//HOVER:23,13
//DEFINITION:13,18
//DEFINITION:17,17
//DEFINITION:23,13

// CHECK: --------
// CHECK-NEXT: range: 12,16 - 12,25
// CHECK: func stepValue(uint value) -> uint

// CHECK: --------
// CHECK-NEXT: range: 16,15 - 16,24
// CHECK: func stepValue(uint value) -> uint

// CHECK: --------
// CHECK-NEXT: range: 22,11 - 22,20
// CHECK: func stepValue(uint value) -> uint

// CHECK: --------
// CHECK-NEXT: definitions: 1
// CHECK-NEXT: same document: true
// CHECK-NEXT: range: 2,5 - 2,14

// CHECK: --------
// CHECK-NEXT: definitions: 1
// CHECK-NEXT: same document: true
// CHECK-NEXT: range: 2,5 - 2,14

// CHECK: --------
// CHECK-NEXT: definitions: 1
// CHECK-NEXT: same document: true
// CHECK-NEXT: range: 2,5 - 2,14
