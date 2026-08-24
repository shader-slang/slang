//TEST:SIMPLE: -experimental-feature -no-codegen

import slang.raytracing;

interface IClosestHitShader
{
    void invoke();
}

interface ITraceProgramLayout {}

struct OrdinaryStage : IClosestHitShader
{
    void invoke() {}
}

struct OrdinaryLayout : ITraceProgramLayout {}

void useOrdinaryTypes(OrdinaryStage stage, OrdinaryLayout layout)
{
    stage.invoke();
    OrdinaryStage();
    OrdinaryLayout();
}
