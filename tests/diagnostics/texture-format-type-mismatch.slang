//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// Textures with explicit storage formats must not coerce to or from the
// default unknown-format texture type, even when their data types match.

RWTexture2D<RGBA16F> formattedTexture;
RWTexture2D plainTexture;

void takesFormatted(RWTexture2D<RGBA16F> texture)
{
}

void takesPlain(RWTexture2D texture)
{
}

[numthreads(1, 1, 1)]
void computeMain(uint2 tid : SV_DispatchThreadID)
{
    takesFormatted(plainTexture);
//CHECK:           ^^^^^^^^^^^^ type mismatch in expression
//CHECK:           ^^^^^^^^^^^^ expected an expression of type 'RWTexture2D<vector<float,4>>', got 'RWTexture2D<vector<float,4>>'

    takesPlain(formattedTexture);
//CHECK:       ^^^^^^^^^^^^^^^^ type mismatch in expression
//CHECK:       ^^^^^^^^^^^^^^^^ expected an expression of type 'RWTexture2D<vector<float,4>>', got 'RWTexture2D<vector<float,4>>'
}
