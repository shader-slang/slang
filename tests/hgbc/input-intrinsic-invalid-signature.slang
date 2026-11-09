//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

struct InputEvent
{
    uint type;
    uint key;
    uint button;
    float2 pos;
    float2 scroll;
}

InputEvent pollInputEvent()
{
    return hgbc_asm {
        result : $$InputEvent = OpPollInputEvent;
        // CHECK: error[E29123]
        // CHECK: OpPollInputEvent has an invalid number of operands
    };
}

[host]
void main()
{
    pollInputEvent();
}
