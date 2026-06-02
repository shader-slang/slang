//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM62): -target hlsl -profile sm_6_2 -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM63): -target hlsl -profile sm_6_3 -whole-program -warnings-disable 30056

// The tests disable warning 30056 so the DEFAULT checks can intentionally assert that
// pre-SM6 HLSL still emits the deprecated non-short-circuiting `?:` form instead of select().

// DEFAULT: warning[E41026]
// DEFAULT-NOT: {{^export$}}
// DEFAULT-NOT: select(
// DEFAULT: {{.*}}?{{.*}}:{{.*}}
// DEFAULT: case int(0):
// DEFAULT: x_{{.*}} = int(100)
// DEFAULT: result_{{.*}} = x_{{.*}} + value_{{.*}}
// DEFAULT: break
// DEFAULT: case int(1):

// SM62-NOT: warning[E41026]
// SM62-NOT: {{^export$}}
// SM62: exportedSelect
// SM62: select(
// SM62: exportedSwitch
// SM62: case int(0):
// SM62: x_{{.*}} = int(100)
// SM62-NOT: break
// SM62: case int(1):

// SM63-NOT: warning[E41026]
// SM63: {{^export$}}
// SM63: exportedSelect
// SM63: select(
// SM63: exportedSwitch
// SM63: case int(0):
// SM63: x_{{.*}} = int(100)
// SM63-NOT: break
// SM63: case int(1):

export bool2 exportedSelect(bool2 condition, bool2 trueValue, bool2 falseValue)
{
    return condition ? trueValue : falseValue;
}

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
