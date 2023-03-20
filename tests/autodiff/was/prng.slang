//TEST_IGNORE_FILE:
struct PRNG
{
    /** Initializes a Xorshift32 pseudorandom generator.
        \param[in] seed The random seed.
        \return the new pseudorandom generator.
    */
    __init(uint seed)
    {
        this.state = seed;
    }

    /** Generates the next pseudorandom number in the sequence (32 bits).
        This function updates the state.
    */
    [mutating] uint next()
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }

    [mutating] uint nextFloat1D()
    {
        return (float)(next()) / UINT_MAX;
    }

    uint state;
};