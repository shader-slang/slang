//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A combined texture-sampler value is split for some targets but not for others. The replacement
// pass does not yet support that target-dependent representation, so the source declaration must
// be rejected for every target.
static Sampler2D<float4> cachedSampler;
//CHECK:                 ^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                 ^^^^^^^^^^^^^ global variable cannot have opaque type.
//CHECK:                 ^^^^^^^^^^^^^ do you intend to define a `uniform` parameter instead?
