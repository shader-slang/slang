//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -warnings-as-errors all
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage compute -emit-spirv-directly -warnings-as-errors all

// Case requirements do not impose restrictions or warnings on integer conversions.
enum Mode
{
    [require(hlsl)] A = 0,
    [require(spirv)] B = 1,
    [require(spirv)] duplicate = 0,
}

T fromInteger<T : ILogical>(int value)
{
    return T(value);
}

int fromGenericTag<let N : int>()
{
    return int(Mode(N));
}

struct Holder<let value : int>
{
    int payload;
}

// This declaration references Mode, but no enum case. Substitution needs no case check.
typealias Alias<let N : int> = Holder<int(Mode(N)) * 0>;

RWStructuredBuffer<int> outputBuffer;

// HLSL: void main
// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    outputBuffer[0] = int(Mode(0));
    outputBuffer[1] = int(Mode(1));
    outputBuffer[2] = int(Mode(42));
    outputBuffer[3] = int(Mode(int(threadID.x)));
    outputBuffer[4] = int(fromInteger<Mode>(int(threadID.x)));
    outputBuffer[5] = fromGenericTag<0>() + fromGenericTag<1>() + fromGenericTag<42>();
    Alias<42> value = { 7 };
    outputBuffer[6] = value.payload;
}
