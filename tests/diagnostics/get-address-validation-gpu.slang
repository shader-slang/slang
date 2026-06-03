//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-stage compute -entry computeMain -target spirv

// Tests that __getAddress diagnoses function-local addresses on GPU targets.

struct Foo
{
    int data;
}

ConstantBuffer<Foo> cb;
RWStructuredBuffer<int> output;
uniform int* devicePointer;

// Helpers to ensure pointer results survive DCE, so the IR validation pass sees them.
void sink(int* p) { output[0] = *p; }
void sink2(int** pp) { sink(*pp); }

void testCases(int param, int* p2, ConstantBuffer<Foo> cbParam)
{
    // & param; // not allowed
    int* a = __getAddress(param);
//diag:      ^^^^^^^^^^^^ invalid __getAddress usage
//diag:      ^^^^^^^^^^^^ cannot take the address of a function-local variable
    sink(a);

    // &p2; // not allowed
    int** b = __getAddress(p2);
//diag:       ^^^^^^^^^^^^ invalid __getAddress usage
//diag:       ^^^^^^^^^^^^ cannot take the address of a function-local variable
    sink2(b);

    // &localVar; // not allowed
    int localVar = 0;
    int* c = __getAddress(localVar);
//diag:      ^^^^^^^^^^^^ invalid __getAddress usage
//diag:      ^^^^^^^^^^^^ cannot take the address of a function-local variable
    sink(c);

    // &localVar2; // not allowed
    int* localVar2 = devicePointer;
    int** d = __getAddress(localVar2);
//diag:       ^^^^^^^^^^^^ invalid __getAddress usage
//diag:       ^^^^^^^^^^^^ cannot take the address of a function-local variable
    sink2(d);

    // &cb; // allowed
    ConstantBuffer<Foo>* e = __getAddress(cbParam);
    output[0] = (*e).data;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    testCases(threadId.x, devicePointer, cb);
}
