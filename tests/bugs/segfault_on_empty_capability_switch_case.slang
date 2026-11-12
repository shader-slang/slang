/*
There was once a bug where all of these capability switch statements caused a segfault.
https://github.com/shader-slang/slang/pull/12357
*/

//TEST:SIMPLE(filecheck=CHECK): -target metal
// Some of the switches below have no case for spirv or wgsl. The compiler must report a
// capability error for those two targets, not crash while it analyzes the empty cases.
//TEST:SIMPLE(filecheck=CHECK_ERR,exit-code=-1): -target spirv
//TEST:SIMPLE(filecheck=CHECK_ERR,exit-code=-1): -target wgsl

//CHECK: comp
//CHECK_ERR: error[E36107]: unavailable features in entry point

[shader("compute")]
[numthreads(9, 1, 1)]
void comp() {
    {// Test __target_switch
        __target_switch {
        case metal:
        }

        __target_switch {
        case spirv:
            int six_seven = 67;
        case metal:
        }

        __target_switch {
        case spirv:
            int six_seven = 67;
        case metal:
        case wgsl:
        }

        __target_switch {
        default:
        }

        __target_switch {
        case spirv:
            int six_seven = 67;
        default:
        }

        __target_switch {
        case spirv:
            int six_seven = 67;
        case metal:
        default:
        }
    }

    {// Test __stage_switch
        __stage_switch {
        case compute:
        }

        __stage_switch {
        case fragment:
            int six_seven = 67;
        case compute:
        }

        __stage_switch {
        case fragment:
            int six_seven = 67;
        case compute:
        case mesh:
        }

        __stage_switch {
        default:
        }

        __stage_switch {
        case fragment:
            int six_seven = 67;
        default:
        }

        __stage_switch {
        case fragment:
            int six_seven = 67;
        case compute:
        default:
        }
    }
}
