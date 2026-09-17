//TEST_IGNORE_FILE:

module raytracing;

namespace rt
{
public interface IClosestHitShader<Context>
{
    void invoke();
}
}
