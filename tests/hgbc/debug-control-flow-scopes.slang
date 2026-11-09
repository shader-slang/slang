// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK,SCOPE_COUNT): -target hgbc -entry main -g2 -validate-ir

[noinline]
bool getPredicate(bool value)
{
    return value;
}

[host]
void main(bool input)
{
    Conditional<uint, true> optional = 7u;
    if (let selected = optional.get())
    {
        bool shortCircuit = input && getPredicate(input);
        print(selected);
        print(shortCircuit);
    }

    __target_switch
    {
    case hgbc:
        uint targetLocal = 9;
        print(targetLocal);
    default:
        uint fallbackLocal = 10;
    }
}

// CHECK: Section("META")
// CHECK-DAG: [[FUNCTION:%[A-Za-z0-9_]+]] = OpDebugFunction %strMain
// CHECK-DAG: [[BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[FUNCTION]] {{.*}} 12 5 28 1
// CHECK-DAG: [[IF_LET:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[BODY]] {{.*}} 13 5 20 5
// CHECK-DAG: [[POSITIVE_ARM:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[IF_LET]] {{.*}} 14 5 20 5
// CHECK-DAG: [[POSITIVE_BODY:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[POSITIVE_ARM]] {{.*}} 15 9 18 5
// CHECK-DAG: [[TARGET_CASE:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[BODY]] {{.*}} 22 5 25 5
// CHECK: Section("FUNC")
// CHECK-DAG: OpDebugVariable %strSelected {{.*}} [[POSITIVE_ARM]]
// CHECK-DAG: OpDebugVariable %strShortcircuit {{.*}} [[POSITIVE_BODY]]
// CHECK-DAG: OpDebugVariable %strTargetlocal {{.*}} [[TARGET_CASE]]

// SCOPE_COUNT: Section("META")
// SCOPE_COUNT-COUNT-5: OpDebugLexicalBlock
// SCOPE_COUNT-NOT: OpDebugLexicalBlock
