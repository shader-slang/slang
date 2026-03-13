// return-type-mismatch.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum LobeTypes : uint32_t { None = 0, Diffuse = 1 }

interface IMaterialInstance {
    LobeTypes get_lobe_types();
//CHECK:      ^^^^^^^^^^^^^^ see interface requirement declaration of 'get_lobe_types'
}

struct Broken : IMaterialInstance {
    uint get_lobe_types() { return LobeTypes::Diffuse; }
//CHECK: ^^^^^^^^^^^^^^ return type mismatch
//CHECK: ^^^^^^^^^^^^^^ member 'get_lobe_types' return type 'uint' does not match interface requirement return type 'LobeTypes'.
}

int test()
{
    Broken b;
    return 0;
}