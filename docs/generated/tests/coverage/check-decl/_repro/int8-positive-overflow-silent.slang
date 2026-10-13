// Repro for finding check-decl-int8-positive-overflow-not-warned.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
// `-200` overflows int8_t and warns with E40016; `200` overflows int8_t
// exactly as much (it wraps to -56) and is accepted silently.
void takesInt8(int8_t x)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    takesInt8(200);  // silent — wraps to -56
    takesInt8(-200); // warning E40016
    int8_t a = 200;  // silent — wraps to -56
    int8_t b = -200; // warning E40016
}
