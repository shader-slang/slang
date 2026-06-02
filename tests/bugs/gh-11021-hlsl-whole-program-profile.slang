//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl -whole-program
//TEST:SIMPLE(filecheck=SM62): -target hlsl -profile sm_6_2 -whole-program
//TEST:SIMPLE(filecheck=SM63): -target hlsl -profile sm_6_3 -whole-program

// E41026 reports switch fall-through restructuring, expected only for the SM5.1 fallback.

// DEFAULT: warning[E41026]
// DEFAULT-NOT: {{^export$}}
// DEFAULT: case int(0):
// DEFAULT: x_{{.*}} = int(100)
// DEFAULT: result_{{.*}} = x_{{.*}} + value_{{.*}}
// DEFAULT: break
// DEFAULT: case int(1):

// SM62-NOT: warning[E41026]
// SM62-NOT: {{^export$}}
// SM62: exportedSwitch
// SM62: case int(0):
// SM62: x_{{.*}} = int(100)
// SM62-NOT: break
// SM62: case int(1):

// SM63-NOT: warning[E41026]
// SM63: {{^export$}}
// SM63: exportedSwitch
// SM63: case int(0):
// SM63: x_{{.*}} = int(100)
// SM63-NOT: break
// SM63: case int(1):

export int exportedSwitch(int value)
{
    int result = 0;
    int x = 10;

    switch (value)
    {
    case 0:
        x = 100;
    case 1:
        result = x + value;
        break;
    default:
        result = -1;
        break;
    }

    return result;
}
