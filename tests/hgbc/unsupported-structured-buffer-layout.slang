//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
void main()
{
    RWStructuredBuffer<float, DefaultPushConstantDataLayout> values =
        RWStructuredBuffer<float, DefaultPushConstantDataLayout>::alloc(1);
    // CHECK: error[E56006]: HGBC target does not support structured-buffer data layouts other than DefaultDataLayout and ScalarDataLayout yet
}
