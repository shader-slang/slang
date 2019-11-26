//TEST(compute):SIMPLE: -stage compute -entry main -target hlsl

import string_literal_module;

int doSomething(String string)
{
    return 10;
}

[numthreads(1, 1, 1)]
void main(
    uint tid   : SV_DispatchThreadIndex)
{
	//doSomething("Hello \t\n\0x083 World");
    
    int value = 1 + getStringHash("Hello \t\n\0x083 World");
    
}

