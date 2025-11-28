"""
This python script provides LLDB formatters for Slang IR types.
To use it, see the `docs/debugging.md` file in this repo.
"""

import json

import lldb


class Children:
    indices: dict[str, int]
    values: list[lldb.SBValue]

    def __init__(self):
        self.indices = {}
        self.values = []

    def append(self, value: lldb.SBValue) -> None:
        self.indices[value.name] = len(self.values)
        self.values.append(value)

    def __len__(self) -> int:
        return len(self.values)

    def get_index(self, name: str) -> int:
        return self.indices[name]

    def get_at_index(self, idx: int) -> lldb.SBValue:
        return self.values[idx]


def IRUse_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return "<invalid>"
    val = valobj.GetNonSyntheticValue()
    used_value = val.GetChildMemberWithName("usedValue")
    if not used_value.IsValid() or used_value.GetValueAsUnsigned(0) == 0:
        return "<null>"
    deref = used_value.Dereference()
    if not deref.IsValid():
        return "<invalid deref>"
    return deref.summary if deref.summary else "<no summary>"


class IRInstListBase_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        return self.children.get_index(name)

    def get_child_at_index(self, idx):
        return self.children.get_at_index(idx)

    def update(self):
        self.children = Children()
        pointer = self.valobj.GetChildMemberWithName("first")
        if not pointer.IsValid():
            return
        i = 0
        while pointer.IsValid() and pointer.GetValueAsUnsigned(0) != 0:
            child = pointer.Dereference()
            if not child.IsValid():
                break
            self.children.append(child.Clone(f"[{i}]"))
            next_ptr = child.GetNonSyntheticValue().GetChildMemberWithName("next")
            if not next_ptr.IsValid():
                break
            pointer = next_ptr
            i += 1
            if i >= 5000:
                # The debugger can call this function on uninitialized
                # values, so we need to ensure that we stop iterating at
                # some point. Ideally we'd provide a synthetic child
                # called something like `[more]` to see another batch of
                # many children in the case where there really is just a
                # very large list, but this is good enough because one
                # can always manually follow `next` pointers.
                break

    def has_children(self):
        return True


class IRInst_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        return self.children.get_index(name)

    def get_child_at_index(self, idx):
        return self.children.get_at_index(idx)

    def update(self):
        self.children = Children()

        if not self.valobj.IsValid():
            return

        valobj_type = self.valobj.GetType()
        if not valobj_type.IsValid():
            return

        if valobj_type.IsPointerType():
            if self.valobj.GetValueAsUnsigned(0) != 0:
                valobj = self.valobj.Dereference()
                if valobj.IsValid():
                    for i in range(valobj.GetNumChildren()):
                        child = valobj.GetChildAtIndex(i)
                        if child.IsValid():
                            self.children.append(child)
            return

        target = self.valobj.GetTarget()
        if not target.IsValid():
            return

        op = self.valobj.GetChildMemberWithName("m_op")
        if not op.IsValid():
            return

        # literal values
        value: list[tuple[str, lldb.SBValue]] = []
        # Using `Cast` here seems to work just fine in the LLDB CLI with
        # `v`, as well as in CodeLLDB, but for some reason it does not
        # work correctly with `p`, causing the `[value]` child to be
        # missing in that case. It is possible to fix that by using
        # `EvaluateExpression` instead, but that significantly degrades
        # performance, so we choose not to do it here.
        op_value = op.GetValue()
        if op_value == "kIROp_StringLit":
            string_lit_t = target.FindFirstType("Slang::IRStringLit")
            if string_lit_t.IsValid():
                string_lit = self.valobj.Cast(string_lit_t)
                if string_lit.IsValid():
                    val = string_lit.GetChildMemberWithName("value")
                    if val.IsValid():
                        string_val = val.GetChildMemberWithName("stringVal")
                        if string_val.IsValid():
                            value = [("[value]", string_val)]
        elif op_value == "kIROp_IntLit":
            int_lit_t = target.FindFirstType("Slang::IRIntLit")
            if int_lit_t.IsValid():
                int_lit = self.valobj.Cast(int_lit_t)
                if int_lit.IsValid():
                    val = int_lit.GetChildMemberWithName("value")
                    if val.IsValid():
                        int_val = val.GetChildMemberWithName("intVal")
                        if int_val.IsValid():
                            value = [("[value]", int_val)]

        # operands
        operands: list[tuple[str, lldb.SBValue]] = []
        offset = valobj_type.GetByteSize()
        ir_use_t = target.FindFirstType("Slang::IRUse")
        if ir_use_t.IsValid():
            ir_use_size = ir_use_t.GetByteSize()
            operand_count_val = self.valobj.GetChildMemberWithName("operandCount")
            if operand_count_val.IsValid():
                operand_count = operand_count_val.GetValueAsUnsigned(0)
                # We must ensure that we don't loop for an unbounded amount of
                # time, so we cap the number of operands displayed here. Ideally
                # we'd provide a way to view more in the case of instructions
                # with more than this many operands, though.
                for index in range(min(operand_count, 10)):
                    name = f"[operand{index}]"
                    operand = self.valobj.CreateChildAtOffset(
                        name, offset + index * ir_use_size, ir_use_t
                    )
                    if operand.IsValid():
                        operands.append((name, operand))

        # Build children list with validity checks
        debug_uid = self.valobj.GetChildMemberWithName("_debugUID")
        type_use = self.valobj.GetChildMemberWithName("typeUse")
        used_value = type_use.GetChildMemberWithName("usedValue") if type_use.IsValid() else lldb.SBValue()
        decorations = self.valobj.GetChildMemberWithName("m_decorationsAndChildren")
        parent = self.valobj.GetChildMemberWithName("parent")
        first_use = self.valobj.GetChildMemberWithName("firstUse")

        for name, child in [
            ("[op]", op),
            ("[UID]", debug_uid),
            ("[type]", used_value),
            # TODO: [exportName]
            # TODO: [importName]
            # TODO: [name]
            *value,
            *operands,
            ("[decorations/children]", decorations),
            ("[parent]", parent),
            # TODO: Traverse the linked list to show all uses next to
            # each other, rather than pointing to the first one.
            ("[uses]", first_use),
        ]:
            if child.IsValid():
                self.children.append(child.Clone(name))

    def has_children(self):
        return True


def IRInst_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return "<invalid>"

    valobj_type = valobj.GetType()
    if not valobj_type.IsValid():
        return "<invalid type>"

    if valobj_type.IsPointerType():
        if valobj.GetValueAsUnsigned(0) == 0:
            return "nullptr"
        deref = valobj.Dereference()
        if not deref.IsValid():
            return "<invalid deref>"
        return deref.GetSummary() if deref.GetSummary() else "<no summary>"

    val = valobj.GetNonSyntheticValue()
    if not val.IsValid():
        return "<invalid non-synthetic>"

    op = val.GetChildMemberWithName("m_op")
    if not op.IsValid():
        return "<invalid op>"

    addr = val.GetAddress()
    if not addr.IsValid():
        return f"{{{op.GetValue()}}}"

    return f"{{{op.GetValue()} {addr}}}"


def stringval_summary(valobj: lldb.SBValue) -> str:
    if not valobj.IsValid():
        return '""'

    val = valobj.GetNonSyntheticValue()
    if not val.IsValid():
        return '""'

    num_chars_val = val.GetChildMemberWithName("numChars")
    if not num_chars_val.IsValid():
        return '""'

    num_chars = num_chars_val.GetValueAsUnsigned(0)
    if num_chars == 0:
        return '""'

    chars_val = val.GetChildMemberWithName("chars")
    if not chars_val.IsValid() or chars_val.GetValueAsUnsigned(0) == 0:
        return '""'

    chars_data = chars_val.GetPointeeData(0, num_chars)
    if not chars_data.IsValid():
        return '""'

    try:
        chars = chars_data.GetUnsignedInt8(lldb.SBError(), 0)
        char_list = []
        for i in range(num_chars):
            error = lldb.SBError()
            char_val = chars_data.GetUnsignedInt8(error, i)
            if error.Success():
                char_list.append(chr(char_val))
            else:
                break
        return json.dumps("".join(char_list))
    except:
        return '""'


def StringValue_summary(valobj: lldb.SBValue, dict) -> str:
    return stringval_summary(valobj)


def StringSliceValue_summary(valobj: lldb.SBValue, dict) -> str:
    return stringval_summary(valobj)


def __lldb_init_module(debugger: lldb.SBDebugger, internal_dict):
    commands = [
        # Slang::IRUse
        "type summary add Slang::IRUse -F slang_lldb.IRUse_summary -w slang",
        # Slang::IRInstListBase
        "type synthetic add Slang::IRInstListBase -l slang_lldb.IRInstListBase_synthetic -w slang",
        # Slang::IRInst
        "type synthetic add Slang::IRInst -l slang_lldb.IRInst_synthetic -w slang",
        "type summary add --expand Slang::IRInst -F slang_lldb.IRInst_summary -w slang",
        # Slang::IRConstant::StringValue
        "type summary add Slang::IRConstant::StringValue -F slang_lldb.StringValue_summary -w slang",
        # Slang::IRConstant::StringSliceValue
        "type summary add Slang::IRConstant::StringSliceValue -F slang_lldb.StringSliceValue_summary -w slang",
        # Enable slang category
        "type category enable slang",
    ]

    for c in commands:
        debugger.HandleCommand(c)
