//TEST_IGNORE_FILE:

__exported import IMaterialInstance;

interface IMaterial
{
    associatedtype MaterialInstance : IMaterialInstance;

    [BackwardDifferentiable]
    MaterialInstance setupMaterialInstance(out MaterialInstanceData miData);
}
