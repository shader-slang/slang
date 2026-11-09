//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
void main()
{
    // Optional is lowered to a synthesized aggregate. Until that aggregate has a semantic
    // reflection name, HGVM cannot ask Slang for its structured-buffer layout.
    RWStructuredBuffer<Optional<float>> values =
        RWStructuredBuffer<Optional<float>>::alloc(1);
    // CHECK: error[E56006]: HGBC target does not support device-memory aggregate types without reflection names yet
}
