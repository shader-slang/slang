// Pin the resulting `BaseType` for each well-formed integer-literal suffix.
// The diagnostic-only test in `tests/diagnostics/int-literal-suffix.slang`
// covers acceptance/rejection of the syntax shapes; this test guards against
// the orthogonal regression where a well-formed suffix is accepted but maps
// to the wrong base type (e.g. `LL` silently selecting `Int` instead of
// `Int64`). #11201.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // Use `sizeof()` to pin the storage size, and `-1` plus the matching
    // printf format to pin the signedness:
    //   - signed type prints `-1`
    //   - unsigned 32-bit prints `4294967295`
    //   - unsigned 64-bit prints `18446744073709551615`

    // Length 32 / signed.
    let aL  = -1L;
    // Length 32 / signed (lowercase).
    let al  = -1l;
    // Length 32 / unsigned.
    let aU  = -1U;
    // Length 32 / unsigned (lowercase).
    let au  = -1u;
    // Length 64 / signed.
    let aLL = -1LL;
    // Length 64 / signed (lowercase).
    let all = -1ll;
    // Length 64 / unsigned.
    let aULL = -1ULL;
    let aLLU = -1LLU;
    // Length 32 / unsigned (mixed-case allowed across U and L specifiers).
    let aUL  = -1UL;
    let aLU  = -1LU;
    let auL  = -1uL;
    let aLu  = -1Lu;
    let alU  = -1lU;
    let aUl  = -1Ul;
    // Pointer-sized.
    let aZ   = -1Z;
    let az   = -1z;
    let auZ  = -1uZ;
    let aZu  = -1Zu;

    printf("L=%d l=%d U=%u u=%u LL=%lld ll=%lld ULL=%llu LLU=%llu UL=%u LU=%u uL=%u Lu=%u lU=%u Ul=%u Z=%lld z=%lld uZ=%llu Zu=%llu\n",
        aL, al, aU, au, aLL, all, aULL, aLLU,
        aUL, aLU, auL, aLu, alU, aUl,
        aZ, az, auZ, aZu);
    //CHECK: L=-1 l=-1 U=4294967295 u=4294967295 LL=-1 ll=-1 ULL=18446744073709551615 LLU=18446744073709551615 UL=4294967295 LU=4294967295 uL=4294967295 Lu=4294967295 lU=4294967295 Ul=4294967295 Z=-1 z=-1 uZ=18446744073709551615 Zu=18446744073709551615

    printf("size: L=%d LL=%d UL=%d ULL=%d Z=%d uZ=%d U=%d\n",
        (int)sizeof(aL), (int)sizeof(aLL), (int)sizeof(aUL), (int)sizeof(aULL),
        (int)sizeof(aZ), (int)sizeof(auZ), (int)sizeof(aU));
    //CHECK: size: L=4 LL=8 UL=4 ULL=8 Z=8 uZ=8 U=4
}
