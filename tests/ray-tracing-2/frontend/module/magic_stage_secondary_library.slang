//TEST_IGNORE_FILE:

module magic_stage_secondary_library;

import slang.raytracing.portable;

namespace second
{
// A separate application library can refine the same standard stage contract.
public interface IBackgroundStage : rt::IMissShader {}
}
