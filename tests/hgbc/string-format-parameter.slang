//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

String format<each T>(String pattern, expand each T args)
{
    return hgbc_asm {
        result : $$String = OpFormatString $pattern $expand each args;
    };
}

[host]
public String main(String value)
{
    return format("[{}]", value);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[STRING:%[A-Za-z0-9_]+]] = OpStringType
// CHECK: Section("FUNC")
// CHECK: [[VALUE:%[A-Za-z0-9_]+]] : [[STRING]] = OpParam
// CHECK: [[RESULT:%[A-Za-z0-9_]+]] : [[STRING]] = OpCall [[FORMAT:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}} [[VALUE]]
// CHECK: OpReturn [[RESULT]]
// CHECK: [[FORMAT]] : {{%[A-Za-z0-9_]+}} = OpFunc
// CHECK: [[PATTERN:%[A-Za-z0-9_]+]] : [[STRING]] = OpParam
// CHECK: [[ARGUMENT:%[A-Za-z0-9_]+]] : [[STRING]] = OpParam
// CHECK: [[FORMATTED:%[A-Za-z0-9_]+]] : [[STRING]] = OpFormatString [[PATTERN]] [[STRING]] [[ARGUMENT]]
// CHECK: OpReturn [[FORMATTED]]
