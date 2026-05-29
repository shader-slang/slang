//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM60): -target hlsl -profile sm_6_0 -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM61): -target hlsl -profile sm_6_1 -whole-program -warnings-disable 30056

// DEFAULT: warning[E41026]
// DEFAULT-NOT: {{^export$}}
// DEFAULT-NOT: select(
// DEFAULT: {{.*}}?{{.*}}:{{.*}}
// DEFAULT: case int(0):
// DEFAULT: x_{{.*}} = int(100)
// DEFAULT: result_{{.*}} = x_{{.*}} + value_{{.*}}
// DEFAULT: break
// DEFAULT: case int(1):

// SM60-NOT: warning[E41026]
// SM60-NOT: {{^export$}}
// SM60: exportedSelect
// SM60: select(
// SM60: exportedSwitch
// SM60: case int(0):
// SM60: x_{{.*}} = int(100)
// SM60-NOT: break
// SM60: case int(1):

// SM61-NOT: warning[E41026]
// SM61: {{^export$}}
// SM61: exportedSelect
// SM61: select(
// SM61: exportedSwitch
// SM61: case int(0):
// SM61: x_{{.*}} = int(100)
// SM61-NOT: break
// SM61: case int(1):

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
