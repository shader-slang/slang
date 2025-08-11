// return-type-mismatch.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

enum LobeTypes : uint32_t { None = 0, Diffuse = 1 }

interface IMaterialInstance {
    LobeTypes get_lobe_types();
}

struct Broken : IMaterialInstance {
    // CHECK: ([[# @LINE+1]]): error 38106:
    uint get_lobe_types() { return LobeTypes::Diffuse; }
}

int test()
{
    Broken b;
    return 0;
}