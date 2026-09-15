// Tests that a variable declared inside a lexical block goes out of scope
// after the block closes (DebugLexicalBlock scope boundary).
//TEST:SPVDB_DEBUGGER(filecheck=SPVDB): -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<uint> input,
          uniform RWStructuredBuffer<uint> output)
{
    uint x = input[0];
    {
        uint inner = x * 2u;   // line 12: inner is in scope here
        x = inner + 1u;
    }                           // line 14: inner goes out of scope
    output[0] = x;             // line 15: only x is in scope
}

// SPVDB-CMD: descriptor 0 0 [3]
// SPVDB-CMD: descriptor 0 1 [0]
// SPVDB-CMD: break lexical-scope.slang 13
// SPVDB-CMD: run
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: step
// SPVDB-CMD: location
// SPVDB-CMD: locals
// SPVDB-CMD: run
// SPVDB-CMD: read 0 1

// Inside the block at line 13 (x = inner + 1u): both x and inner are visible.
// SPVDB: stopped: Breakpoint
// SPVDB: location: {{.*}}lexical-scope.slang:13
// SPVDB: local: inner = 6
// SPVDB: local: x = 3
// After stepping past the closing brace (no DebugLine on 14): land on line 15.
// SPVDB: location: {{.*}}lexical-scope.slang:15
// SPVDB: local: x = 7
// SPVDB: stopped: EntryFinished
// SPVDB: read[0][1]: [7]
