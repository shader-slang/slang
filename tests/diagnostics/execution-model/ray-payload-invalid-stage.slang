// Test that an invalid stage name in ray payload access qualifier produces an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct [raypayload] MyPayload
{
    float value : read(caller) : write(badstage);
    /*CHECK:
                                       ^^^^^^^^ invalid stage name in ray payload access qualifier
                                       ^^^^^^^^ invalid stage name 'badstage' in ray payload access qualifier; valid stages are 'anyhit', 'closesthit', 'miss', and 'caller'
    */
};

[shader("miss")]
void missMain(inout MyPayload payload)
{
}
