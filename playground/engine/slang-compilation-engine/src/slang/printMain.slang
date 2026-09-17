import user;
import playground;

[format("rgba8")]
[playground::BLACK_SCREEN(1.0, 1.0)]
WTexture2D                          outputTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
[playground::CALL(1, 1, 1)]
[playground::CALL::ONCE]
void printMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    printMain();
}
