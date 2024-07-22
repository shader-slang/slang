//TEST:SIMPLE(filecheck=CHK): -target spirv

struct State {
    float3 v;
    float3 n;
    int rnd;
};

//CHK-DAG: warning 41022: inout parameter 'x' is never written to, consider making it an in parameter instead
void int_never_assigned(inout int x) {}

//CHK-DAG: warning 41022: inout parameter 'state' is never written to, consider making it an in parameter instead
void state_never_assigned(inout State state, inout float v)
{
    v = state.v.x;
}

void state_assigned(inout State state)
{
    int3 vi = __slang_noop_cast <int3, float3> (state.v);
    int3 ni = __slang_noop_cast <int3, float3> (state.n);
    state.rnd = dot(vi, ni);
}

//CHK-NOT: warning 41022
