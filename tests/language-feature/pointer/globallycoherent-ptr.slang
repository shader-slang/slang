//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests for invalid use of `globallycoherent` with Ptr/T*

cbuffer Globals
{
    // CHECK: error[E30078]
    globallycoherent Ptr<int> ptr1;
    // CHECK: error[E30078]
    globallycoherent int* ptr2;
    // CHECK: error[E30078]
    coherent Ptr<int> ptr3;
    // CHECK: error[E30078]
    coherent int* ptr4;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
}
