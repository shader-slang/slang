//DIAGNOSTIC_TEST:SIMPLE(filecheck=NUMERIC): -target hlsl -entry main -profile sm_6_9 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SEALED): -target hlsl -entry main -profile sm_6_9 -no-codegen -DCUSTOM_FEATURE

// A numeric argument cannot bypass the feature type's capability requirement.
// NUMERIC: expected a type, got a 'int'

// Only the standard library may define feature policies and their requirements.
// SEALED: cannot inherit from type 'IRayQueryFeature' marked 'sealed'

#ifdef CUSTOM_FEATURE
struct UnregisteredFeature : IRayQueryFeature
{
    static const uint _rayQueryFlags = 1;
}
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
#ifdef CUSTOM_FEATURE
    RayQuery<RAY_FLAG_NONE, UnregisteredFeature> query;
#else
    RayQuery<RAY_FLAG_NONE, 1> query;
#endif
}
