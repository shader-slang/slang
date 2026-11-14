//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

// Using before assigning
float regular_undefined_use(out float3 v)
{
    float r = v.x + 1.0;
//CHK:          ^ use of uninitialized out parameter
//CHK:          ^ use of uninitialized out parameter 'v'

    return r;
/*CHK:
    ^^^^^^ returning without initializing out parameter
    ^^^^^^ returning without initializing out parameter 'v'
*/
}

// Returning before assigning
float returning_undefined_use(out float x)
{
    return 0;
/*CHK:
    ^^^^^^ returning without initializing out parameter
    ^^^^^^ returning without initializing out parameter 'x'
*/
}

// Implicit, still returning before assigning
void implicit_undefined_use(out float x)
{
}
/*CHK:
^ returning without initializing out parameter
^ returning without initializing out parameter 'x'
*/

// Warn on potential return paths
void control_flow_undefined(bool b, out float y)
{
    if(b)
    {
        return;
/*CHK:
        ^^^^^^ returning without initializing out parameter
        ^^^^^^ returning without initializing out parameter 'y'
*/
    }
    y = 0;
    return;
}

// No warnings if all paths are fine
void control_flow_defined(bool b, out float y)
{
    if(b)
    {
        unused(y);
        return;
    }
    y = 0;
    return;
}

// Specialized handling for intrinsic asm
void instrincs_defined(float x, out float exp)
{
    __intrinsic_asm "frexp";
}

// Specialized handling for target switches
void target_switching_undefined(float x, out float exp)
{
    __target_switch {}
}
/*CHK:
^ returning without initializing out parameter
^ returning without initializing out parameter 'exp'
*/

void target_switching_defined(float x, out float exp)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "frexp";
    }
}

void target_switching_blocks(uint x, out half v)
{
    __target_switch
    {
    case hlsl:
        if ((x & 2) == 0)
            v = half(1);
        else
            v = half(1);
        return;
    case glsl:
    case spirv:
        {
            if ((x & 2) == 0)
                v = half(1);
            else
                v = half(1);
            return;
        }
    }
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
