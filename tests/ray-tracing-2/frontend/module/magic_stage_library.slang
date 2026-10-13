//TEST_IGNORE_FILE:

module magic_stage_library;

import slang.raytracing;

namespace first
{
// An application library can refine the standard stage contract without compiler annotations.
public interface IBackgroundStage : rt::IMissShader {}
}
