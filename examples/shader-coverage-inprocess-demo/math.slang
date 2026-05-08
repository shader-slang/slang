// math.slang — small math helpers, imported by physics.slang and app.slang.
// Demonstrates that the IR coverage pass instruments helper modules
// alongside the entry-point module: counters synthesized here show up
// in the manifest with this file's path, distinct from app.slang and
// physics.slang.

public uint scaleByFactor(uint value, uint factor)
{
    if (factor == 0)
    {
        // Unreachable in this demo's invocations — `factor` is always
        // non-zero. The 0-hit attribution here demonstrates the
        // visualization of dead / never-taken branches.
        return 0;
    }
    return value * factor;
}

public uint clampToRange(uint value, uint maxValue)
{
    if (value > maxValue)
        return maxValue;
    return value;
}
