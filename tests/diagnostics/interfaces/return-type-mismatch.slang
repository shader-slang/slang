// return-type-mismatch.slang

//DIAGNOSTIC_TEST:SIMPLE:

enum LobeTypes : uint32_t { None = 0, Diffuse = 1 }

interface IMaterialInstance {
    LobeTypes get_lobe_types();
}

struct Broken : IMaterialInstance {
    uint get_lobe_types() { return LobeTypes::Diffuse; }
}

int test()
{
    Broken b;
    return 0;
}