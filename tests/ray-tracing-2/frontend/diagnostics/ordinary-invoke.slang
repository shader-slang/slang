//TEST:SIMPLE: -entry main -stage compute -no-codegen

interface IOrdinaryStage
{
    void invoke();
}

struct OrdinaryStage : IOrdinaryStage
{
    void invoke() {}
}

[numthreads(1, 1, 1)]
void main()
{
    OrdinaryStage stage;
    stage.invoke();
}
