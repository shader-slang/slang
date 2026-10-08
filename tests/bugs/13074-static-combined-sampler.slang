//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A combined texture-sampler value is split for some targets but not for others. The replacement
// pass does not yet support that target-dependent representation, so the file-scope `static`
// declaration must be rejected for every target.
static Sampler2D<float4> cachedSampler;
//CHECK:                 ^^^^^^^^^^^^^ opaque type is not supported for mutable static storage
//CHECK:                 ^^^^^^^^^^^^^ mutable file- or namespace-scope `static` variable 'cachedSampler' has the opaque type 'Sampler2D<vector<float,4>>', which cannot be represented by mutable storage
//CHECK:                 ^^^^^^^^^^^^^ do you intend to define a `uniform` parameter instead?
