// Functions that are kept for the target without being called from an entry point are generated,
// so a `__target_switch` with no case for the target in one of them is E41037 even without
// `-ignore-capabilities`: exported functions, `[DllExport]` functions, and methods reached through
// dynamic dispatch.

//DIAGNOSTIC_TEST:SIMPLE(diag=EXPORT):-target hlsl -entry main -stage compute -DEXPORTED
// For the last two, the "see call to" note points at a compiler-generated call (the export wrapper
// or the dispatch function) that has no source location, so those lanes check only the error.
//DIAGNOSTIC_TEST:SIMPLE(diag=DLLEXPORT, non-exhaustive):-target cpp -entry main -stage compute -DDLL_EXPORTED
//DIAGNOSTIC_TEST:SIMPLE(diag=DYNAMIC, non-exhaustive):-target cuda -entry main -stage compute -DDYNAMIC -conformance "A:IFoo=0" -conformance "B:IFoo=1"

RWStructuredBuffer<float> output;

#if defined(EXPORTED)
export void exportedMetalOnly(inout float x)
{
    __target_switch
/*EXPORT:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'exportedMetalOnly' contains a __target_switch with no case for target 'hlsl'
*/
    {
    case metal:
        x = 2;
    }
}
#endif

#if defined(DLL_EXPORTED)
[DllExport]
void dllExportedMetalOnly(inout float x)
{
    __target_switch
/*DLLEXPORT:
    ^^^^^^^^^^^^^^^ E41037
    ^^^^^^^^^^^^^^^ 'dllExportedMetalOnly' contains a __target_switch with no case for target 'cpp'
*/
    {
    case metal:
        x = 2;
    }
}
#endif

#if defined(DYNAMIC)
[anyValueSize(8)]
interface IFoo
{
    float f(float x);
}

struct A : IFoo
{
    float v;
    float f(float x)
    {
        __target_switch
/*DYNAMIC:
        ^^^^^^^^^^^^^^^ E41037
        ^^^^^^^^^^^^^^^ 'A.f' contains a __target_switch with no case for target 'cuda'
*/
        {
        case metal:
            return x;
        }
    }
}

struct B : IFoo
{
    float v;
    float f(float x) { return x * 2; }
}
#endif

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
#if defined(DYNAMIC)
    IFoo obj = createDynamicObject<IFoo>(int(id.x), 0.0f);
    output[0] = obj.f(1.0);
#else
    output[0] = 1;
#endif
}
