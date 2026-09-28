//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-bitfield-packing-rules msvc -no-codegen

struct S
{
    uint8_t first : 1;
    uint32_t separator : 0;
//CHECK:                 ^ zero-width bit fields are not supported by `-bitfield-packing-rules msvc`
//CHECK:                 ^ this zero-width bit field has no defined MSVC packing behavior in Slang
    uint8_t second : 1;
    uint16_t another : 0;
//CHECK:               ^ zero-width bit fields are not supported by `-bitfield-packing-rules msvc`
//CHECK:               ^ this zero-width bit field has no defined MSVC packing behavior in Slang
};

class C
{
    uint32_t separator : 0;
//CHECK:                 ^ zero-width bit fields are not supported by `-bitfield-packing-rules msvc`
//CHECK:                 ^ this zero-width bit field has no defined MSVC packing behavior in Slang
    uint8_t value : 1;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    C c;
    c.value = 1;
}
