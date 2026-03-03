//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests for invalid use of `globallycoherent` with Ptr/T*

cbuffer Globals
{
    // CHECK: ([[# @LINE+1]]): error 30078
    globallycoherent Ptr<int> ptr1;
    // CHECK: ([[# @LINE+1]]): error 30078
    globallycoherent int* ptr2;
    // CHECK: ([[# @LINE+1]]): error 30078
    coherent Ptr<int> ptr3;
    // CHECK: ([[# @LINE+1]]): error 30078
    coherent int* ptr4;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
}
