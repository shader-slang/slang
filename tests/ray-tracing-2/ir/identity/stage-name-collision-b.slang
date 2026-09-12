module structural_stage_name_collision_b;

import slang.raytracing;
import structural_stage_name_collision_common;

// `Miss` has the same user-facing stage name as module A's type but a distinct module-qualified
// semantic identity. Both would otherwise claim the same native symbol.
public struct Miss : rt::IMissShader
{
    typealias Context = SharedMissContext;
    void invoke(rt::MissInput<SharedMissContext> input) { input.payload.value = 2; }
}

public typealias CollisionStageB = Miss;
