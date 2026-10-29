//TEST:SIMPLE: -experimental-feature -no-codegen

import slang.raytracing.portable;

interface IClosestHitShader
{
    void invoke();
}

interface ITraceProgramSchema {}

struct OrdinaryStage : IClosestHitShader
{
    void invoke() {}
}

struct OrdinarySchema : ITraceProgramSchema {}

void useOrdinaryTypes(OrdinaryStage stage, OrdinarySchema schema)
{
    stage.invoke();
    OrdinaryStage();
    OrdinarySchema();
}
