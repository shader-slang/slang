// Tests DebugValue composite building for struct local variables.
// A struct local whose fields are assigned individually via member-access stores
// must appear in 'locals' with all fields visible.  Slang emits one DebugValue
// per member store; spvdb must accumulate them into a composite rather than
// overwriting the whole variable on each update.
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

struct Point
{
    float x;
    float y;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<float> inputX,
          uniform RWStructuredBuffer<float> inputY,
          uniform RWStructuredBuffer<float> output)
{
    Point p;
    p.x = inputX[0];
    p.y = inputY[0];
    output[0] = p.x + p.y;   // line 23
}

// SPVDB-CMD: descriptor 0 0 [3.0]
// SPVDB-CMD: descriptor 0 1 [4.0]
// SPVDB-CMD: descriptor 0 2 [0.0]
// SPVDB-CMD: break struct-locals.slang 23
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run

// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}struct-locals.slang:23
// DebugValue instructions with member indices now correctly update individual
// composite elements.  Both fields must be visible: x=3, y=4 → (3, 4).
// (Member names are not set via the DebugValue path, so the unnamed format
// with parentheses is expected rather than the named {x=3, y=4} format.)
// SPVDB: local: p = (3, 4)
// SPVDB: stopped: EntryFinished
