//TEST:SIMPLE: -no-codegen

// `consume<N>({})` probes whether an empty initializer list can default-initialize
// `vector<float, N>` while `N` is still a link-time value. Overload probing asks only whether the
// conversion is possible and does not request a materialized result expression, so the successful
// probe must not try to construct one.

void consume<let N : int>(vector<float, N> value) {}

void test<let N : int>()
{
    consume<N>({});
}
