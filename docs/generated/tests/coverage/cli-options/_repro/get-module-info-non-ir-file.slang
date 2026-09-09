// Repro for finding cli-get-module-info-non-ir-file-silent-failure.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
//
// Passing this file's own path to `-get-module-info` (it is readable, but it is
// Slang source rather than a serialized IR blob) exits 1 and prints nothing.
// Passing a path that does not exist correctly reports E00001.
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}
