//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
// non-exhaustive: capability errors include entry-point and provenance diagnostics.

enum Mode
{
    [require(hlsl)] hlslMode = 1,
}

int readDefault(Mode value = Mode.hlslMode) { return int(value); }

int readForTarget()
{
    __target_switch
    {
    case hlsl:
        return readDefault();
    case spirv:
        // The same checked default must be visited again in this capability context.
        return readDefault();
        //CHECK: that is conflicting with the 'target_switch's current capability requirement
    default:
        return 0;
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = readForTarget();
}
