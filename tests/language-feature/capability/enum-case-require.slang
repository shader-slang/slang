//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute -emit-spirv-directly

// Unused cases do not restrict the enum type or functions that accept it.
enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 1,
    [require(spirv)]
    spirvOnly = 2,
    portable = 3,
    nextPortable,
}

enum NarrowValue : uint8_t
{
    [require(hlsl)]
    zero = 0,
}

enum GenericValue<T>
{
    [require(hlsl)]
    hlslOnly = 5,
}

int consume(TargetValue value)
{
    TargetValue copy = value;
    return int(copy);
}

// Logical operations do not require the arbitrary integer-construction contract.
TargetValue combine(TargetValue value)
{
    value |= value;
    value &= value;
    value ^= value;
    return ~value;
}

int selectForTarget()
{
    __target_switch
    {
    case hlsl:
        let value = TargetValue.hlslOnly;
        return int(value);
    case spirv:
        let value = TargetValue.spirvOnly;
        return int(value);
    default:
        return int(TargetValue.portable);
    }
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = consume(TargetValue.hlslOnly);
    outputBuffer[1] = consume(TargetValue.portable);
    outputBuffer[2] = TargetValue.hlslOnly == TargetValue.portable;
    outputBuffer[3] = bool(TargetValue.hlslOnly);
    outputBuffer[4] = consume(TargetValue(1));
    static const int hlslTag = 1;
    outputBuffer[5] = consume((TargetValue)(hlslTag + 0));
    outputBuffer[6] = consume(TargetValue.nextPortable);
    outputBuffer[7] = consume(combine(TargetValue.portable));
    outputBuffer[8] = selectForTarget();
    // Match the checked value after conversion to the underlying tag type.
    outputBuffer[9] = int(NarrowValue(256));
    outputBuffer[10] = int(GenericValue<float>(5));
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    outputBuffer[0] = consume(TargetValue.spirvOnly);
    outputBuffer[1] = consume(TargetValue.portable);
    outputBuffer[2] = TargetValue.spirvOnly == TargetValue.portable;
    outputBuffer[3] = bool(TargetValue.spirvOnly);
    outputBuffer[4] = consume(TargetValue(2));
    outputBuffer[5] = consume((TargetValue)(1 + 1));
    outputBuffer[6] = consume(TargetValue.nextPortable);
    outputBuffer[7] = consume(combine(TargetValue.portable));
    outputBuffer[8] = selectForTarget();
}

// Ordinary enums retain direct and generic integer construction.
enum Ordinary { zero, one }

T fromInteger<T : ILogical>(int value)
{
    return T(value);
}

T fromTag<T : __EnumType>(T.__Tag value)
{
    return T(value);
}

Ordinary ordinaryCasts(int value)
{
    return Ordinary(value) | fromInteger<Ordinary>(value) | fromTag<Ordinary>(value);
}
