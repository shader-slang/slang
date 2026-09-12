module structural_stage_name_collision_a;

import slang.raytracing;
import structural_stage_name_collision_common;

// This module deliberately gives its stage the same source declaration path as module B. The
// exported alias lets the consuming schema refer to both types without relying on ambiguous
// unqualified lookup for `Miss`.
public struct Miss : rt::IMissShader
{
    typealias Context = SharedMissContext;
    void invoke(rt::MissInput<SharedMissContext> input) { input.payload.value = 1; }
}

public typealias CollisionStageA = Miss;
