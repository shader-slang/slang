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

// Sometimes an inOutImplicitCast is done,
// this needs to be tracked as an alias;
// none of the following functions should 
// generate warnings
uint lcg(inout uint prev)
{
    const uint LCG_A = 1664525u;
    const uint LCG_C = 1013904223u;
    prev = (LCG_A * prev + LCG_C);
    return prev & 0x00FFFFFF;
}

float rnd(inout uint prev)
{
    return ((float) lcg(prev) / (float) 0x01000000);
}

float3 sample(inout int seed)
{
    float3 xi;
    xi.x = rnd(seed);
    xi.y = rnd(seed);
    xi.z = rnd(seed);
    return xi.z;
}

//CHK-NOT: warning 41022
//CHK-NOT: warning 41023
