//TEST:SIMPLE(filecheck=CHK): -target spirv

// Using before assigning
float regular_undefined_use(out float3 v)
{
    //CHK-DAG: warning 41015: use of uninitialized out parameter 'v'
    float r = v.x + 1.0;
    
    //CHK-DAG: warning 41018: returning without initializing out parameter 'v'
    return r;
}

// Returning before assigning
float returning_undefined_use(out float x)
{
    //CHK-DAG: warning 41018: returning without initializing out parameter 'x'
    return 0;
}

// Implicit, still returning before assigning
void implicit_undefined_use(out float x) 
{
    //CHK-DAG: warning 41018: returning without initializing out parameter 'x'
}

// Warn on potential return paths
void control_flow_undefined(bool b, out float y)
{
    if(b)
    {
        //CHK-DAG: warning 41018: returning without initializing out parameter 'y'
        return;
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

//CHK-NOT: warning 41015
//CHK-NOT: warning 41018
