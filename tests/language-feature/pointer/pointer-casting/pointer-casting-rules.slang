//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests pointer casting rules: Only explicit casting is allowed between pointer types.
// All implicit conversions between pointer types should fail.
RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

__generic<AddressSpace a, CoherentScope c>
void writeToReadOnlyPointer( Ptr<int, a, Access::Read, c> ptr)
{
    ptr[0] = 1;
}

// Type aliases for testing
typedef Ptr<int, AddressSpace::UserPointer, Access::Read> NonCoherentReadPtrInt;
typedef Ptr<int, AddressSpace::UserPointer, Access::Read, CoherentScope::Device> DeviceCoherentReadPtrInt;
typedef Ptr<int, AddressSpace::UserPointer, Access::Read, CoherentScope::Workgroup> WorkgroupCoherentReadPtrInt;

typedef Ptr<float, AddressSpace::UserPointer, Access::Read> NonCoherentReadPtrFloat;
typedef Ptr<float, AddressSpace::UserPointer, Access::Read, CoherentScope::Device> DeviceCoherentReadPtrFloat;
typedef Ptr<float, AddressSpace::UserPointer, Access::Read, CoherentScope::Workgroup> WorkgroupCoherentReadPtrFloat;

typedef Ptr<int, AddressSpace::UserPointer, Access::ReadWrite> NonCoherentReadWritePtrInt;
typedef Ptr<int, AddressSpace::UserPointer, Access::ReadWrite, CoherentScope::Device> DeviceCoherentReadWritePtrInt;
typedef Ptr<int, AddressSpace::UserPointer, Access::ReadWrite, CoherentScope::Workgroup> WorkgroupCoherentReadWritePtrInt;

typedef Ptr<float, AddressSpace::UserPointer, Access::ReadWrite> NonCoherentReadWritePtrFloat;
typedef Ptr<float, AddressSpace::UserPointer, Access::ReadWrite, CoherentScope::Device> DeviceCoherentReadWritePtrFloat;
typedef Ptr<float, AddressSpace::UserPointer, Access::ReadWrite, CoherentScope::Workgroup> WorkgroupCoherentReadWritePtrFloat;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    NonCoherentReadPtrInt readPtr = &processMemory[id.x];
    DeviceCoherentReadPtrInt devicePtr = DeviceCoherentReadPtrInt(&processMemory[id.x]);

    // ===========================================
    // SECTION 1: ReadWrite -> Read Conversions
    // ===========================================
    
    // 1.1: ReadWrite -> Read, Same Type, Same CoherentScope (SHOULD FAIL - implicit conversion)
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadPtrInt fail_rw_to_r_same_type_same_scope = &processMemory[id.x];
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrInt fail_rw_to_r_same_type_same_scope_2 = DeviceCoherentReadWritePtrInt(&processMemory[id.x]);
    
    // 1.2: ReadWrite -> Read, Different Type, Same CoherentScope (SHOULD FAIL - implicit conversion)
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadPtrFloat fail_rw_to_r_diff_type_same_scope = &processMemory[id.x];
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrFloat fail_rw_to_r_diff_type_same_scope_2 = DeviceCoherentReadWritePtrInt(&processMemory[id.x]);
    
    // 1.3: ReadWrite -> Read, Same Type, Different CoherentScope (SHOULD FAIL - implicit conversion)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrInt fail_rw_to_r_same_type_diff_scope = &processMemory[id.x];
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadPtrInt fail_rw_to_r_same_type_diff_scope_2 = &processMemory[id.x];
    
    // 1.4: ReadWrite -> Read, Different Type, Different CoherentScope (SHOULD FAIL - implicit conversion)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrFloat fail_rw_to_r_diff_type_diff_scope = &processMemory[id.x];
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadPtrFloat fail_rw_to_r_diff_type_diff_scope_2 = &processMemory[id.x];
    
    // 1.5: ReadWrite -> Read, Explicit casts (SHOULD PASS - explicit conversion)
    NonCoherentReadPtrInt pass_rw_to_r_explicit_same_type_same_scope = NonCoherentReadPtrInt(&processMemory[id.x]);
    DeviceCoherentReadPtrInt pass_rw_to_r_explicit_same_type_same_scope_2 = DeviceCoherentReadPtrInt(DeviceCoherentReadWritePtrInt(&processMemory[id.x]));
    NonCoherentReadPtrFloat pass_rw_to_r_explicit_diff_type = NonCoherentReadPtrFloat(&processMemory[id.x]);
    DeviceCoherentReadPtrInt pass_rw_to_r_explicit_diff_scope = DeviceCoherentReadPtrInt(&processMemory[id.x]);
    DeviceCoherentReadPtrFloat pass_rw_to_r_explicit_diff_type_diff_scope = DeviceCoherentReadPtrFloat(&processMemory[id.x]);
    WorkgroupCoherentReadPtrInt pass_rw_to_r_explicit_diff_scope_2 = WorkgroupCoherentReadPtrInt(&processMemory[id.x]);

    // ===========================================
    // SECTION 2: Read -> ReadWrite Conversions (ALL SHOULD FAIL)
    // ===========================================
    
    // 2.1: Read -> ReadWrite, Same Type, Same CoherentScope (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadWritePtrInt fail_r_to_rw_same_type_same_scope = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadWritePtrInt fail_r_to_rw_same_type_same_scope_2 = devicePtr;
    
    // 2.2: Read -> ReadWrite, Different Type, Same CoherentScope (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadWritePtrFloat fail_r_to_rw_diff_type_same_scope = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadWritePtrFloat fail_r_to_rw_diff_type_same_scope_2 = devicePtr;
    
    // 2.3: Read -> ReadWrite, Same Type, Different CoherentScope (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadWritePtrInt fail_r_to_rw_same_type_diff_scope = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadWritePtrInt fail_r_to_rw_same_type_diff_scope_2 = devicePtr;
    
    // 2.4: Read -> ReadWrite, Different Type, Different CoherentScope (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadWritePtrFloat fail_r_to_rw_diff_type_diff_scope = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadWritePtrFloat fail_r_to_rw_diff_type_diff_scope_2 = devicePtr;

    // 2.5: Read -> ReadWrite, Explicit casts (SHOULD STILL FAIL)
    // Note: `error 30500` is thrown here since default constructor `Ptr()` is allowed & has 0 arguments, not 1 argument
    // CHECK: ([[# @LINE+1]]): error 30500
    NonCoherentReadWritePtrInt fail_r_to_rw_explicit_same_type = NonCoherentReadWritePtrInt(readPtr);
    // CHECK: ([[# @LINE+1]]): error 30500
    DeviceCoherentReadWritePtrInt fail_r_to_rw_explicit_same_type_2 = DeviceCoherentReadWritePtrInt(devicePtr);
    // CHECK: ([[# @LINE+1]]): error 30500
    WorkgroupCoherentReadWritePtrFloat fail_r_to_rw_explicit_diff_type_diff_scope = WorkgroupCoherentReadWritePtrFloat(devicePtr);

    // ===========================================
    // SECTION 3: CoherentScope Conversions (Same Access)
    // ===========================================
    
    // 3.1: CoherentScope conversions, Same Access, Same Type - Implicit (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrInt fail_coherent_scope_implicit_r_same_type = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadPtrInt fail_coherent_scope_implicit_r_same_type_2 = devicePtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadWritePtrInt fail_coherent_scope_implicit_rw_same_type = DeviceCoherentReadWritePtrInt(&processMemory[id.x]);
    
    // 3.2: CoherentScope conversions, Same Access, Different Type - Implicit (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrFloat fail_coherent_scope_implicit_r_diff_type = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadPtrFloat fail_coherent_scope_implicit_r_diff_type_2 = devicePtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    WorkgroupCoherentReadWritePtrFloat fail_coherent_scope_implicit_rw_diff_type = DeviceCoherentReadWritePtrInt(&processMemory[id.x]);
    
    // 3.3: CoherentScope conversions, Same Access, Same Type - Explicit (SHOULD PASS)
    DeviceCoherentReadPtrInt pass_coherent_scope_explicit_r_same_type = DeviceCoherentReadPtrInt(readPtr);
    WorkgroupCoherentReadPtrInt pass_coherent_scope_explicit_r_same_type_2 = WorkgroupCoherentReadPtrInt(devicePtr);
    WorkgroupCoherentReadWritePtrInt pass_coherent_scope_explicit_rw_same_type = WorkgroupCoherentReadWritePtrInt(DeviceCoherentReadWritePtrInt(&processMemory[id.x]));
    
    // 3.4: CoherentScope conversions, Same Access, Different Type - Explicit (SHOULD PASS)
    DeviceCoherentReadPtrFloat pass_coherent_scope_explicit_r_diff_type = DeviceCoherentReadPtrFloat(readPtr);
    WorkgroupCoherentReadPtrFloat pass_coherent_scope_explicit_r_diff_type_2 = WorkgroupCoherentReadPtrFloat(devicePtr);
    WorkgroupCoherentReadWritePtrFloat pass_coherent_scope_explicit_rw_diff_type = WorkgroupCoherentReadWritePtrFloat(DeviceCoherentReadWritePtrInt(&processMemory[id.x]));

    // ===========================================
    // SECTION 4: Type Conversions (Same Access, Same CoherentScope)
    // ===========================================
    
    // 4.1: Type conversions, Same Access, Same CoherentScope - Implicit (SHOULD FAIL)
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadPtrFloat fail_type_implicit_r = readPtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    DeviceCoherentReadPtrFloat fail_type_implicit_r_2 = devicePtr;
    // CHECK: ([[# @LINE+1]]): error 30019
    NonCoherentReadWritePtrFloat fail_type_implicit_rw = NonCoherentReadWritePtrInt(&processMemory[id.x]);
    
    // 4.2: Type conversions, Same Access, Same CoherentScope - Explicit (SHOULD PASS)
    NonCoherentReadPtrFloat pass_type_explicit_r = NonCoherentReadPtrFloat(readPtr);
    DeviceCoherentReadPtrFloat pass_type_explicit_r_2 = DeviceCoherentReadPtrFloat(devicePtr);

    // CHECK-NOT: error
    output[id] = *readPtr + readPtr[5];
}
