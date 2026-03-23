module "bug-module";

public struct MyAddress<T : __BuiltinFloatingPointType>
{
    public RWStructuredBuffer<T>.Handle handle;
    public uint baseIndex;

    [ForceInline]
    public void atomicAdd(uint index, vector<T, 2> value)
    {
        __target_switch
        {
        case cuda:
            RWStructuredBuffer<vector<T, 2>> vecBuffer = reinterpret<RWStructuredBuffer<vector<T, 2>>>(*handle);
            __atomic_reduce_add(vecBuffer[baseIndex + index], value);
        default:
            __atomic_reduce_add(handle[baseIndex + index], value[0]);
            __atomic_reduce_add(handle[baseIndex + index + 1], value[1]);
        }
    }
}
