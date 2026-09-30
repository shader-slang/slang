//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3
//TEST:SIMPLE(filecheck=LIVE): -target cuda -entry computeMain -stage compute -DLIVE_UNAVAILABLE_SWITCH

// CUDA: result code = 0
// CUDA-NEXT: standard error = {
// CUDA-NEXT: }
// CUDA: __global__ void computeMain
// LIVE: result code = {{-?[1-9][0-9]*}}
// LIVE: error[E41011]

// Helpers on both sides of their callers exercise dependency order. The generic helper also
// requires target-switch selection inside the function returned by an IR generic.
uint helperBefore(uint value)
{
    __target_switch
    {
    case nvvm: return value + 1;
    }
}

T genericHelper<T : IInteger>(T value)
{
    __target_switch
    {
    case nvvm: return value + T(2);
    }
}

uint routed(uint value)
{
    __target_switch
    {
    case nvvm: return helperBefore(value) + helperAfter(value) + genericHelper(value);
    case cuda: return value;
    }
}

uint helperAfter(uint value)
{
    __target_switch
    {
    case nvvm: return value + 3;
    }
}

uint nested(uint value)
{
    __target_switch
    {
    case nvvm:
        // This failed inner switch belongs to a retained function, but an unreachable block.
        __target_switch
        {
        case nvvm: return value + 4;
        }
    case cuda: return value;
    }
}

[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
#ifdef LIVE_UNAVAILABLE_SWITCH
    output[1] = helperBefore(output[0]);
#else
    output[1] = routed(output[0]) + nested(output[0]);
#endif
}
