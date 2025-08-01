# This file holds (incomplete) Python type stubs for LLDB v19.1.7:
# https://lldb.llvm.org/python_api.html

# The API includes many docstrings which describe the type signatures of
# functions, but these are not in a format that can be read by Python
# type checking tools. This has been requested upstream, but seems to be
# blocked because SWIG (the tool used to generate the LLDB Python
# bindings) doees not support generating Python type hints:
# https://github.com/llvm/llvm-project/issues/79043

# For another example of a project that takes the same approach for LLDB
# Python types, see the mongo-c-driver repo:
# https://github.com/mongodb/mongo-c-driver/blob/2.0.2/lldb.pyi

# The classes in this file, along with the members in each class, are
# listed in the same order as the autogenerated `lldb/__init__.py` file
# distributed with LLVM:
# https://packages.debian.org/sid/amd64/python3-lldb-19/filelist

# This file should not impose a maintenance burden, because although not
# all classes are present and not all methods are fully typed, each
# present class has all its members listed. As a result, language
# servers like Pylance don't give spurious errors about missing members,
# and can simply report types as "unknown" for methods that have not
# been annotated yet. Thus, when adding a new class to this file, be
# sure to include all its members, but don't feel required to include
# types for everything unless you want to.

from typing import Sequence

class SBData:
    @property
    def thisown(self): ...
    def __init__(self, *args): ...
    def GetAddressByteSize(self): ...
    def SetAddressByteSize(self, addr_byte_size): ...
    def Clear(self): ...
    def __nonzero__(self): ...
    def __bool__(self): ...
    def IsValid(self): ...
    def GetByteSize(self): ...
    def GetByteOrder(self): ...
    def SetByteOrder(self, endian): ...
    def GetFloat(self, error, offset): ...
    def GetDouble(self, error, offset): ...
    def GetLongDouble(self, error, offset): ...
    def GetAddress(self, error, offset): ...
    def GetUnsignedInt8(self, error, offset): ...
    def GetUnsignedInt16(self, error, offset): ...
    def GetUnsignedInt32(self, error, offset): ...
    def GetUnsignedInt64(self, error, offset): ...
    def GetSignedInt8(self, error, offset): ...
    def GetSignedInt16(self, error, offset): ...
    def GetSignedInt32(self, error, offset): ...
    def GetSignedInt64(self, error, offset): ...
    def GetString(self, error, offset): ...
    def ReadRawData(self, error, offset, buf): ...
    def GetDescription(self, *args): ...
    def SetData(self, error, buf, endian, addr_size): ...
    def SetDataWithOwnership(self, error, buf, endian, addr_size): ...
    def Append(self, rhs): ...
    @staticmethod
    def CreateDataFromCString(endian, addr_byte_size, data): ...
    @staticmethod
    def CreateDataFromUInt64Array(endian, addr_byte_size, array): ...
    @staticmethod
    def CreateDataFromUInt32Array(endian, addr_byte_size, array): ...
    @staticmethod
    def CreateDataFromSInt64Array(endian, addr_byte_size, array): ...
    @staticmethod
    def CreateDataFromSInt32Array(endian, addr_byte_size, array): ...
    @staticmethod
    def CreateDataFromDoubleArray(endian, addr_byte_size, array): ...
    def SetDataFromCString(self, data): ...
    def SetDataFromUInt64Array(self, array): ...
    def SetDataFromUInt32Array(self, array): ...
    def SetDataFromSInt64Array(self, array): ...
    def SetDataFromSInt32Array(self, array): ...
    def SetDataFromDoubleArray(self, array): ...
    def __repr__(self) -> str: ...
    def __len__(self): ...
    @classmethod
    def CreateDataFromInt(
        cls, value, size=None, target=None, ptr_size=None, endian=None
    ): ...
    def _make_helper(self, sbdata, getfunc, itemsize): ...
    def _make_helper_uint8(self): ...
    def _make_helper_uint16(self): ...
    def _make_helper_uint32(self): ...
    def _make_helper_uint64(self): ...
    def _make_helper_sint8(self): ...
    def _make_helper_sint16(self): ...
    def _make_helper_sint32(self): ...
    def _make_helper_sint64(self): ...
    def _make_helper_float(self): ...
    def _make_helper_double(self): ...
    def _read_all_uint8(self): ...
    def _read_all_uint16(self): ...
    def _read_all_uint32(self): ...
    def _read_all_uint64(self): ...
    def _read_all_sint8(self): ...
    def _read_all_sint16(self): ...
    def _read_all_sint32(self): ...
    def _read_all_sint64(self): ...
    def _read_all_float(self): ...
    def _read_all_double(self): ...
    @property
    def uint8(self) -> Sequence[int]: ...
    @property
    def uint16(self): ...
    @property
    def uint32(self): ...
    @property
    def uint64(self): ...
    @property
    def sint8(self): ...
    @property
    def sint16(self): ...
    @property
    def sint32(self): ...
    @property
    def sint64(self): ...
    @property
    def float(self): ...
    @property
    def double(self): ...
    @property
    def uint8s(self): ...
    @property
    def uint16s(self): ...
    @property
    def uint32s(self): ...
    @property
    def uint64s(self): ...
    @property
    def sint8s(self): ...
    @property
    def sint16s(self): ...
    @property
    def sint32s(self): ...
    @property
    def sint64s(self): ...
    @property
    def floats(self): ...
    @property
    def doubles(self): ...
    @property
    def byte_order(self): ...
    @property
    def size(self): ...

class SBDebugger:
    def HandleCommand(self, command: str) -> None: ...

class SBTarget:
    @property
    def thisown(self): ...
    @property
    def eBroadcastBitBreakpointChanged(self): ...
    @property
    def eBroadcastBitModulesLoaded(self): ...
    @property
    def eBroadcastBitModulesUnloaded(self): ...
    @property
    def eBroadcastBitWatchpointChanged(self): ...
    @property
    def eBroadcastBitSymbolsLoaded(self): ...
    @property
    def eBroadcastBitSymbolsChanged(self): ...
    def __init__(self, *args): ...
    @property
    def __swig_destroy__(self): ...
    def __nonzero__(self): ...
    __bool__ = __nonzero__
    def IsValid(self): ...
    def EventIsTargetEvent(event): ...
    def GetTargetFromEvent(event): ...
    def GetNumModulesFromEvent(event): ...
    def GetModuleAtIndexFromEvent(idx, event): ...
    def GetBroadcasterClassName(): ...
    def GetProcess(self): ...
    def SetCollectingStats(self, v): ...
    def GetCollectingStats(self): ...
    def GetStatistics(self, *args): ...
    def GetPlatform(self): ...
    def GetEnvironment(self): ...
    def Install(self): ...
    def LoadCore(self, *args): ...
    def LaunchSimple(self, argv, envp, working_directory): ...
    def Launch(self, *args): ...
    def Attach(self, attach_info, error): ...
    def AttachToProcessWithID(self, listener, pid, error): ...
    def AttachToProcessWithName(self, listener, name, wait_for, error): ...
    def ConnectRemote(self, listener, url, plugin_name, error): ...
    def GetExecutable(self): ...
    def AppendImageSearchPath(self, _from, to, error): ...
    def AddModule(self, *args): ...
    def GetNumModules(self): ...
    def GetModuleAtIndex(self, idx): ...
    def RemoveModule(self, module): ...
    def GetDebugger(self): ...
    def FindModule(self, file_spec): ...
    def FindCompileUnits(self, sb_file_spec): ...
    def GetByteOrder(self): ...
    def GetAddressByteSize(self): ...
    def GetTriple(self): ...
    def GetABIName(self): ...
    def GetLabel(self): ...
    def SetLabel(self, label): ...
    def GetDataByteSize(self): ...
    def GetCodeByteSize(self): ...
    def GetMaximumNumberOfChildrenToDisplay(self): ...
    def SetSectionLoadAddress(self, section, section_base_addr): ...
    def ClearSectionLoadAddress(self, section): ...
    def SetModuleLoadAddress(self, module, sections_offset): ...
    def ClearModuleLoadAddress(self, module): ...
    def FindFunctions(self, *args): ...
    def FindFirstGlobalVariable(self, name): ...
    def FindGlobalVariables(self, *args): ...
    def FindGlobalFunctions(self, name, max_matches, matchtype): ...
    def Clear(self): ...
    def ResolveFileAddress(self, file_addr): ...
    def ResolveLoadAddress(self, vm_addr): ...
    def ResolvePastLoadAddress(self, stop_id, vm_addr): ...
    def ResolveSymbolContextForAddress(self, addr, resolve_scope): ...
    def ReadMemory(self, addr, buf, error): ...
    def BreakpointCreateByLocation(self, *args): ...
    def BreakpointCreateByName(self, *args): ...
    def BreakpointCreateByNames(self, *args): ...
    def BreakpointCreateByRegex(self, *args): ...
    def BreakpointCreateBySourceRegex(self, *args): ...
    def BreakpointCreateForException(self, language, catch_bp, throw_bp): ...
    def BreakpointCreateByAddress(self, address): ...
    def BreakpointCreateBySBAddress(self, address): ...
    def BreakpointCreateFromScript(
        self, class_name, extra_args, module_list, file_list, request_hardware=False
    ): ...
    def BreakpointsCreateFromFile(self, *args): ...
    def BreakpointsWriteToFile(self, *args): ...
    def GetNumBreakpoints(self): ...
    def GetBreakpointAtIndex(self, idx): ...
    def BreakpointDelete(self, break_id): ...
    def FindBreakpointByID(self, break_id): ...
    def FindBreakpointsByName(self, name, bkpt_list): ...
    def GetBreakpointNames(self, names): ...
    def DeleteBreakpointName(self, name): ...
    def EnableAllBreakpoints(self): ...
    def DisableAllBreakpoints(self): ...
    def DeleteAllBreakpoints(self): ...
    def GetNumWatchpoints(self): ...
    def GetWatchpointAtIndex(self, idx): ...
    def DeleteWatchpoint(self, watch_id): ...
    def FindWatchpointByID(self, watch_id): ...
    def WatchAddress(self, addr, size, read, modify, error): ...
    def WatchpointCreateByAddress(self, addr, size, options, error): ...
    def EnableAllWatchpoints(self): ...
    def DisableAllWatchpoints(self): ...
    def DeleteAllWatchpoints(self): ...
    def GetBroadcaster(self): ...
    def FindFirstType(self, type: str) -> SBType: ...
    def FindTypes(self, type): ...
    def GetBasicType(self, type): ...
    def CreateValueFromAddress(self, name, addr, type): ...
    def CreateValueFromData(self, name, data, type): ...
    def CreateValueFromExpression(self, name, expr): ...
    def GetSourceManager(self): ...
    def ReadInstructions(self, *args): ...
    def GetInstructions(self, base_addr, buf): ...
    def GetInstructionsWithFlavor(self, base_addr, flavor_string, buf): ...
    def FindSymbols(self, *args): ...
    def __eq__(self, rhs) -> bool: ...
    def __ne__(self, rhs) -> bool: ...
    def GetDescription(self, description, description_level): ...
    def EvaluateExpression(self, *args): ...
    def GetStackRedZoneSize(self): ...
    def IsLoaded(self, module): ...
    def GetLaunchInfo(self): ...
    def SetLaunchInfo(self, launch_info): ...
    def GetTrace(self): ...
    def CreateTrace(self, error): ...
    def __repr__(self) -> str: ...
    def get_modules_access_object(self): ...
    def get_modules_array(self): ...
    def module_iter(self): ...
    def breakpoint_iter(self): ...
    def get_bkpts_access_object(self): ...
    def get_target_bkpts(self): ...
    def watchpoint_iter(self): ...
    def get_watchpoints_access_object(self): ...
    def get_target_watchpoints(self): ...
    @property
    def modules(self): ...
    @property
    def module(self): ...
    @property
    def process(self): ...
    @property
    def executable(self): ...
    @property
    def debugger(self): ...
    @property
    def num_breakpoints(self): ...
    @property
    def breakpoints(self): ...
    @property
    def breakpoint(self): ...
    @property
    def num_watchpoints(self): ...
    @property
    def watchpoints(self): ...
    @property
    def watchpoint(self): ...
    @property
    def broadcaster(self): ...
    @property
    def byte_order(self): ...
    @property
    def addr_size(self): ...
    @property
    def triple(self): ...
    @property
    def data_byte_size(self): ...
    @property
    def code_byte_size(self): ...
    @property
    def platform(self): ...

class SBType:
    @property
    def thisown(self): ...
    def __init__(self, *args): ...
    def __nonzero__(self): ...
    def IsValid(self): ...
    def GetByteSize(self) -> int: ...
    def GetByteAlign(self): ...
    def IsPointerType(self): ...
    def IsReferenceType(self): ...
    def IsFunctionType(self): ...
    def IsPolymorphicClass(self): ...
    def IsArrayType(self): ...
    def IsVectorType(self): ...
    def IsTypedefType(self): ...
    def IsAnonymousType(self): ...
    def IsScopedEnumerationType(self): ...
    def IsAggregateType(self): ...
    def GetPointerType(self): ...
    def GetPointeeType(self) -> SBType: ...
    def GetReferenceType(self): ...
    def GetTypedefedType(self): ...
    def GetDereferencedType(self): ...
    def GetUnqualifiedType(self): ...
    def GetArrayElementType(self) -> SBType: ...
    def GetArrayType(self, size): ...
    def GetVectorElementType(self): ...
    def GetCanonicalType(self): ...
    def GetEnumerationIntegerType(self): ...
    def GetBasicType(self, *args): ...
    def GetNumberOfFields(self): ...
    def GetNumberOfDirectBaseClasses(self): ...
    def GetNumberOfVirtualBaseClasses(self): ...
    def GetFieldAtIndex(self, idx): ...
    def GetDirectBaseClassAtIndex(self, idx): ...
    def GetVirtualBaseClassAtIndex(self, idx): ...
    def GetStaticFieldWithName(self, name): ...
    def GetEnumMembers(self): ...
    def GetNumberOfTemplateArguments(self): ...
    def GetTemplateArgumentType(self, idx): ...
    def GetTemplateArgumentKind(self, idx): ...
    def GetFunctionReturnType(self): ...
    def GetFunctionArgumentTypes(self): ...
    def GetNumberOfMemberFunctions(self): ...
    def GetMemberFunctionAtIndex(self, idx): ...
    def GetModule(self): ...
    def GetName(self): ...
    def GetDisplayTypeName(self): ...
    def GetTypeClass(self): ...
    def IsTypeComplete(self): ...
    def GetTypeFlags(self): ...
    def GetDescription(self, description, description_level): ...
    def FindDirectNestedType(self, name): ...
    def __eq__(self, rhs): ...
    def __ne__(self, rhs): ...
    def __repr__(self): ...
    def template_arg_array(self): ...
    def __len__(self): ...
    @property
    def module(self): ...
    @property
    def name(self): ...
    @property
    def size(self): ...
    @property
    def is_pointer(self): ...
    @property
    def is_reference(self): ...
    @property
    def num_fields(self): ...
    @property
    def num_bases(self): ...
    @property
    def num_vbases(self): ...
    @property
    def num_template_args(self): ...
    @property
    def template_args(self): ...
    @property
    def type(self): ...
    @property
    def is_complete(self): ...
    def get_bases_array(self): ...
    def get_vbases_array(self): ...
    def get_fields_array(self): ...
    def get_members_array(self): ...
    def get_enum_members_array(self): ...
    @property
    def bases(self): ...
    @property
    def vbases(self): ...
    @property
    def fields(self): ...
    @property
    def members(self): ...
    @property
    def enum_members(self): ...

class SBValue:
    @property
    def thisown(self): ...
    def __init__(self, *args): ...
    def __nonzero__(self): ...
    def IsValid(self): ...
    def Clear(self): ...
    def GetError(self): ...
    def GetID(self): ...
    def GetName(self) -> str: ...
    def GetTypeName(self): ...
    def GetDisplayTypeName(self): ...
    def GetByteSize(self): ...
    def IsInScope(self): ...
    def GetFormat(self): ...
    def SetFormat(self, format): ...
    def GetValue(self) -> str: ...
    def GetValueAsSigned(self, *args): ...
    def GetValueAsUnsigned(self, fail_value: int = 0) -> int: ...
    def GetValueAsAddress(self): ...
    def GetValueType(self): ...
    def GetValueDidChange(self): ...
    def GetSummary(self, *args): ...
    def GetObjectDescription(self): ...
    def GetDynamicValue(self, use_dynamic): ...
    def GetStaticValue(self): ...
    def GetNonSyntheticValue(self) -> SBValue: ...
    def GetSyntheticValue(self): ...
    def GetPreferDynamicValue(self): ...
    def SetPreferDynamicValue(self, use_dynamic): ...
    def GetPreferSyntheticValue(self): ...
    def SetPreferSyntheticValue(self, use_synthetic): ...
    def IsDynamic(self): ...
    def IsSynthetic(self): ...
    def IsSyntheticChildrenGenerated(self): ...
    def SetSyntheticChildrenGenerated(self, arg2): ...
    def GetLocation(self): ...
    def SetValueFromCString(self, *args): ...
    def GetTypeFormat(self): ...
    def GetTypeSummary(self): ...
    def GetTypeFilter(self): ...
    def GetTypeSynthetic(self): ...
    def CreateChildAtOffset(self, name: str, offset: int, type: SBType) -> SBValue: ...
    def Cast(self, type: SBType) -> SBValue: ...
    def CreateValueFromExpression(self, *args): ...
    def CreateValueFromAddress(self, name, address, type): ...
    def CreateValueFromData(self, name, data, type): ...
    def GetChildAtIndex(self, *args): ...
    def GetIndexOfChildWithName(self, name): ...
    def GetChildMemberWithName(self, name: str) -> SBValue: ...
    def GetValueForExpressionPath(self, expr_path): ...
    def AddressOf(self): ...
    def GetLoadAddress(self): ...
    def GetAddress(self): ...
    def GetPointeeData(self, item_idx: int = 0, item_count: int = 1) -> SBData: ...
    def GetData(self): ...
    def SetData(self, data, error): ...
    def Clone(self, new_name: str) -> SBValue: ...
    def GetDeclaration(self): ...
    def MightHaveChildren(self): ...
    def IsRuntimeSupportValue(self): ...
    def GetNumChildren(self, max: int | None = None) -> int: ...
    def GetOpaqueType(self): ...
    def GetTarget(self): ...
    def GetProcess(self): ...
    def GetThread(self): ...
    def GetFrame(self): ...
    def Dereference(self) -> SBValue: ...
    def TypeIsPointerType(self): ...
    def GetType(self) -> SBType: ...
    def Persist(self): ...
    def GetDescription(self, description): ...
    def GetExpressionPath(self, *args): ...
    def EvaluateExpression(self, *args): ...
    def Watch(self, *args): ...
    def WatchPointee(self, resolve_location, read, write, error): ...
    def GetVTable(self): ...
    def __repr__(self): ...
    def __get_dynamic__(self): ...
    def get_child_access_object(self): ...
    def get_value_child_list(self): ...
    def __hex__(self): ...
    def __iter__(self): ...
    def __len__(self): ...
    @property
    def children(self) -> list[SBValue]: ...
    @property
    def child(self): ...
    @property
    def name(self) -> str: ...
    @property
    def type(self) -> SBType: ...
    @property
    def size(self): ...
    @property
    def is_in_scope(self): ...
    @property
    def format(self): ...
    @property
    def value(self) -> str: ...
    @property
    def value_type(self): ...
    @property
    def changed(self): ...
    @property
    def data(self): ...
    @property
    def load_addr(self): ...
    @property
    def addr(self): ...
    @property
    def deref(self) -> SBValue: ...
    @property
    def address_of(self) -> SBValue: ...
    @property
    def error(self): ...
    @property
    def summary(self) -> str: ...
    @property
    def description(self): ...
    @property
    def dynamic(self): ...
    @property
    def location(self): ...
    @property
    def target(self) -> SBTarget: ...
    @property
    def process(self): ...
    @property
    def thread(self): ...
    @property
    def frame(self): ...
    @property
    def num_children(self): ...
    @property
    def unsigned(self) -> int: ...
    @property
    def signed(self): ...
    def get_expr_path(self): ...
    @property
    def path(self): ...
    def synthetic_child_from_expression(self, name, expr, options=None): ...
    def synthetic_child_from_data(self, name, data, type): ...
    def synthetic_child_from_address(self, name, addr, type): ...
    def __eol_test(val): ...
    def linked_list_iter(self, next_item_name, end_of_list_test=__eol_test): ...

class SBSyntheticValueProvider:
    def __init__(self, valobj: SBValue) -> None: ...
    def num_children(self) -> int: ...
    def get_child_index(self, name: str) -> int: ...
    def get_child_at_index(self, idx: int) -> SBValue | None: ...
    def update(self) -> bool | None: ...
    def has_children(self) -> bool: ...
    def __len__(self) -> int: ...
    def __iter__(self): ...
