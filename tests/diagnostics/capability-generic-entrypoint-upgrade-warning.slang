// The body already requires sm_6_6. Specializing with int adds no requirements and must
// not repeat the profile-upgrade warning emitted when the generic entry point is selected.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WARN): -target hlsl -profile sm_6_0 -entry bodyMain<int> -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WARN): -target hlsl -profile sm_6_0 -entry bodyMain -specialize int -no-codegen

// Requirements first introduced by specialization must still warn, or error in restrictive mode.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WARN): -target hlsl -profile sm_6_0 -entry argumentMain -specialize Needs66 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -target hlsl -profile sm_6_0 -entry argumentMain -specialize Needs66 -no-codegen -restrictive-capability-check

// A body upgrade and a further argument upgrade each need their own diagnostic.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ADDITIONAL): -target hlsl -profile sm_6_0 -entry bodyMain -specialize Needs67 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR67): -target hlsl -profile sm_6_6 -entry bodyMain -specialize Needs67 -no-codegen -restrictive-capability-check

// Match exactly one warning, including its profile upgrade, rather than only its presence.
//WARN: warning[E41012]
//WARN-NOT: warning[E41012]
//WARN: specified profile 'sm_6_0'{{.*}}'sm_6_6'
//WARN-NOT: warning[E41012]
//ERROR-NOT: warning[E41012]
//ERROR: error[E41013]
//ERROR-NOT: warning[E41012]
//ADDITIONAL: warning[E41012]
//ADDITIONAL: capabilities: 'sm_6_6'
//ADDITIONAL: warning[E41012]
//ADDITIONAL-NOT: sm_6_6
//ADDITIONAL: capabilities: 'sm_6_7'
//ADDITIONAL-NOT: sm_6_6
//ADDITIONAL-NOT: warning[E41012]
//ERROR67-NOT: warning[E41012]
//ERROR67: error[E41013]
//ERROR67: specified profile 'lib_6_6'{{.*}}'sm_6_7'
//ERROR67-NOT: warning[E41012]

[require(sm_6_6)]
void needsNewProfile()
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void bodyMain<T>()
{
    needsNewProfile();
}

[require(sm_6_6)]
struct Needs66
{
    uint value;
}

[require(sm_6_7)]
struct Needs67
{
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void argumentMain<T>()
{
}
