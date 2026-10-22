// Each reference passing mode mangles to its own code: `__ref` keeps `r_`, so existing symbols
// do not change, while `__ref_readonly` and its legacy alias `const __ref` mangle as `ro_`, and
// `__ref_writeonly` mangles as `wo_`.

//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen -dump-ir

// CHECK-DAG: "_S{{.*}}9readWritep1pr_ii"
public int readWrite(__ref int x) { return x; }

// CHECK-DAG: "_S{{.*}}14legacyReadOnlyp1pro_ii"
public int legacyReadOnly(const __ref int x) { return x; }

// CHECK-DAG: "_S{{.*}}8readOnlyp1pro_ii"
public int readOnly(__ref_readonly int x) { return x; }

// CHECK-DAG: "_S{{.*}}9writeOnlyp1pwo_iV"
public void writeOnly(__ref_writeonly int x) { x = 1; }
