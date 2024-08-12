//TEST:SIMPLE(filecheck=CHK): -target spirv

struct State
{
    float3 v;
    float3 n;
    int rnd;
};

//CHK-DAG: ([[# @LINE + 1]]): warning 41022: inout parameter 'x' is never written to
void int_never_assigned(inout int x) {}

//CHK-DAG: ([[# @LINE + 1]]): warning 41022: inout parameter 'state' is never written to
void state_never_assigned(inout State state, inout float v)
{
    v = state.v.x;
}

void state_assigned(inout State state)
{
    state.rnd = (int) dot(state.v, state.n);
}

struct A
{
    int state;

    //CHK-DAG: ([[# @LINE + 1]]): warning 41023: method marked `[mutable]` but never modifies `this`
    [mutating] int next() { return state; }
    
    // TODO: unmodified(...)
    [mutating] int progress()
    {
        unmodified(state);
        return state;
    }
};

__generic <T>
struct B
{
    int state;

    //CHK-DAG: ([[# @LINE + 1]]): warning 41023: method marked `[mutable]` but never modifies `this`
    [mutating] int next() { return state; }
};

//CHK-NOT: warning 41022
//CHK-NOT: warning 41023