//TEST:SIMPLE(filecheck=DEFAULT): -target hlsl -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM60): -target hlsl -profile sm_6_0 -whole-program -warnings-disable 30056
//TEST:SIMPLE(filecheck=SM61): -target hlsl -profile sm_6_1 -whole-program -warnings-disable 30056

// DEFAULT-NOT: {{^export$}}
// DEFAULT-NOT: select(
// DEFAULT: {{.*}}?{{.*}}:{{.*}}

// SM60-NOT: {{^export$}}
// SM60: exportedSelect
// SM60: select(

// SM61: {{^export$}}
// SM61: exportedSelect
// SM61: select(

export bool2 exportedSelect(bool2 condition, bool2 trueValue, bool2 falseValue)
{
    return condition ? trueValue : falseValue;
}
