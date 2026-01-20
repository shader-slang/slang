implementing neural;

#ifdef UNIT_TEST
#define VISIBILITY_LEVEL public
#else
#define VISIBILITY_LEVEL internal
#endif

VISIBILITY_LEVEL typealias uvec<int Dim> = Array<uint32_t, Dim>;

VISIBILITY_LEVEL enum HashType : uint32_t
{
	Prime,
	CoherentPrime,
	ReversedPrime,
	Rng,
	BaseConvert,
}

VISIBILITY_LEVEL struct Pcg32
{
	static const uint64_t DefaultState  = 0x853c49e6748fea9bULL;
	static const uint64_t DefaultStream = 0xda3e39cb94b95bdbULL;
	static const uint64_t Mult          = 0x5851f42d4c957f2dULL;

	uint64_t state;
	uint64_t inc;

	__init(uint64_t initstate, uint64_t initseq = 1)
	{
		state = 0;
		inc = (initseq << 1) | 1;
		nextUint();
		state += initstate;
		nextUint();
	}

	[mutating]
	uint32_t nextUint()
	{
		uint64_t oldstate = state;
		state = oldstate * Mult + inc;
		uint32_t xorshifted = uint32_t(((oldstate >> 18) ^ oldstate) >> 27);
		uint32_t rot = uint32_t(oldstate >> 59);
		return (xorshifted >> rot) | (xorshifted << ((~rot + 1) & 31));
	}

	[mutating]
	void advance(int64_t delta)
	{
		uint64_t curMult = Mult;
		uint64_t curPlus = inc;
		uint64_t accMult = 1;
		uint64_t accPlus = 0;

		uint64_t d = uint64_t(delta);
		while (d > 0)
		{
			if ((d & 1) != 0)
			{
				accMult *= curMult;
				accPlus = accPlus * curMult + curPlus;
			}
			curPlus = (curMult + 1) * curPlus;
			curMult *= curMult;
			d /= 2;
		}
		state = accMult * state + accPlus;
	}
}

// LCG hash: XORs position values multiplied by prime factors
[ForceInline]
VISIBILITY_LEVEL uint32_t lcgHash<uint32_t Dimensions, uint32_t PrimeCount>(uvec<Dimensions> posGrid, uint32_t primes[PrimeCount])
{
	uint32_t result = 0;
	[ForceUnroll]
	for (uint32_t i = 0; i < Dimensions; ++i)
	{
		result ^= posGrid[i] * primes[i];
	}
	return result;
}

[ForceInline]
VISIBILITY_LEVEL uint32_t primeHash<uint32_t Dimensions>(uvec<Dimensions> posGrid)
{
	uint32_t factors[7] = { 1958374283u, 2654435761u, 805459861u, 3674653429u, 2097192037u, 1434869437u, 2165219737u };
	return lcgHash<Dimensions, 7>(posGrid, factors);
}

[ForceInline]
VISIBILITY_LEVEL uint32_t coherentPrimeHash<uint32_t Dimensions>(uvec<Dimensions> posGrid)
{
	uint32_t factors[7] = { 1u, 2654435761u, 805459861u, 3674653429u, 2097192037u, 1434869437u, 2165219737u };
	return lcgHash<Dimensions, 7>(posGrid, factors);
}

[ForceInline]
VISIBILITY_LEVEL uint32_t reversedPrimeHash<uint32_t Dimensions>(uvec<Dimensions> posGrid)
{
	uint32_t factors[7] = { 2165219737u, 1434869437u, 2097192037u, 3674653429u, 805459861u, 2654435761u, 1958374283u };
	return lcgHash<Dimensions, 7>(posGrid, factors);
}

// Base conversion hash (used in permuto-encoding)
[ForceInline]
VISIBILITY_LEVEL uint32_t baseConvertHash<uint32_t Dimensions>(uvec<Dimensions> posGrid)
{
	uint32_t k = 0;
	[ForceUnroll]
	for (uint32_t dim = 0; dim < Dimensions; ++dim)
	{
		k += posGrid[dim];
		k *= 2531011u;
	}
	return k;
}

// RNG hash using PCG32
[ForceInline]
VISIBILITY_LEVEL uint32_t rngHash<uint32_t Dimensions>(uvec<Dimensions> posGrid, uint32_t seed = 1337)
{
	static const uint32_t BitsPerDim = 64 / Dimensions;
	uint64_t step = 0;

	[ForceUnroll]
	for (uint32_t i = 0; i < Dimensions; ++i)
	{
		step ^= uint64_t(posGrid[i]) << (i * BitsPerDim);
	}

	Pcg32 rng = Pcg32(seed);
	rng.advance(int64_t(step));
	return rng.nextUint();
}

[ForceInline]
VISIBILITY_LEVEL uint32_t gridHash<HashType Type, uint32_t Dimensions>(uvec<Dimensions> positionGrid)
{
	switch (Type)
	{
		case HashType.Prime:
			return primeHash<Dimensions>(positionGrid);
		case HashType.CoherentPrime:
			return coherentPrimeHash<Dimensions>(positionGrid);
		case HashType.ReversedPrime:
			return reversedPrimeHash<Dimensions>(positionGrid);
		case HashType.Rng:
			return rngHash<Dimensions>(positionGrid);
		case HashType.BaseConvert:
			return baseConvertHash<Dimensions>(positionGrid);
		default:
			return 0;
	}
}
