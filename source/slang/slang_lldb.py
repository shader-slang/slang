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
    val = valobj.GetNonSyntheticValue()
    return val.GetChildMemberWithName("usedValue").deref.summary


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
        i = 0
        while pointer.unsigned != 0:
            child = pointer.deref
            self.children.append(child.Clone(f"[{i}]"))
            pointer = child.GetNonSyntheticValue().GetChildMemberWithName("next")
            i += 1

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
        target = self.valobj.target
        ty = self.valobj.type
        op = self.valobj.GetChildMemberWithName("m_op")

        # literal values
        value: list[tuple[str, lldb.SBValue]] = []
        match op.value:
            case "kIROp_StringLit":
                string_lit_t = target.FindFirstType("Slang::IRStringLit")
                string_lit = self.valobj.Cast(string_lit_t)
                val = string_lit.GetChildMemberWithName("value")
                value = [("[value]", val.GetChildMemberWithName("stringVal"))]
            case "kIROp_IntLit":
                int_lit_t = target.FindFirstType("Slang::IRIntLit")
                int_lit = self.valobj.Cast(int_lit_t)
                val = int_lit.GetChildMemberWithName("value")
                value = [("[value]", val.GetChildMemberWithName("intVal"))]

        # operands
        operands: list[tuple[str, lldb.SBValue]] = []
        offset = ty.GetByteSize()
        ir_use_t = target.FindFirstType("Slang::IRUse")
        ir_use_size = ir_use_t.GetByteSize()
        for index in range(self.valobj.GetChildMemberWithName("operandCount").unsigned):
            name = f"[operand{index}]"
            operand = self.valobj.CreateChildAtOffset(
                name, offset + index * ir_use_size, ir_use_t
            )
            operands.append((name, operand))

        for name, child in [
            ("[op]", op),
            ("[UID]", self.valobj.GetChildMemberWithName("_debugUID")),
            (
                "[type]",
                self.valobj.GetChildMemberWithName("typeUse").GetChildMemberWithName(
                    "usedValue"
                ),
            ),
            # TODO: [exportName]
            # TODO: [importName]
            # TODO: [name]
            *value,
            *operands,
            (
                "[decorations/children]",
                self.valobj.GetChildMemberWithName("m_decorationsAndChildren"),
            ),
            ("[parent]", self.valobj.GetChildMemberWithName("parent")),
            # TODO: Traverse the linked list to show all uses next to
            # each other, rather than pointing to the first one.
            ("[uses]", self.valobj.GetChildMemberWithName("firstUse")),
        ]:
            self.children.append(child.Clone(name))

    def has_children(self):
        return True


def IRInst_summary(valobj: lldb.SBValue, dict) -> str:
    val = valobj.GetNonSyntheticValue()
    op = val.GetChildMemberWithName("m_op")
    return f"{{{op.value} {val.address_of.value}}}"


def stringval_summary(valobj: lldb.SBValue) -> str:
    val = valobj.GetNonSyntheticValue()
    num_chars = val.GetChildMemberWithName("numChars").unsigned
    chars = val.GetChildMemberWithName("chars").GetPointeeData(0, num_chars).uint8
    return json.dumps("".join(chr(chars[i]) for i in range(num_chars)))


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
