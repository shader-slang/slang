//TEST:SIMPLE: -target spirv -no-codegen -warnings-as-errors all

// Check that a guarded loop that fills an array, followed by a read under the same
// guard, produces no warning. The loop-exit rule treats a loop whose body stores
// the variable as initializing it, so only the infeasible path that skips the loop
// and then takes the read could reach it, and guard tracking removes that path.
float guardedFillLoop(bool condition)
{
    float values[4];
    if (condition)
    {
        for (uint i = 0; i < 4; i++)
            values[i] = 1;
    }
    if (condition)
        return values[2];
    return 0;
}

[shader("compute")]
void main() {}
