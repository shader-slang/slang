"""
This python script provides LLDB formatters for Slang core types.
To use it, see the `docs/debugging.md` file in this repo.
"""

import lldb

# Set to True to enable the logger
ENABLE_LOGGING = True


# log to the LLDB formatter stream
def log(msg):
    if ENABLE_LOGGING:
        lldb.formatters.Logger.Logger() >> msg


def make_string(F: lldb.SBData, L: int) -> str:
    strval = ""
    G = F.uint8
    for X in range(L):
        V = G[X]
        if V == 0:
            break
        strval = strval + chr(V % 256)
    return '"' + strval + '"'


# Return the pointer to the data in a Slang::RefPtr
def get_ref_pointer(valobj: lldb.SBValue) -> lldb.SBValue:
    return valobj.GetNonSyntheticValue().GetChildMemberWithName("pointer")


# Check if a pointer is nullptr
def is_nullptr(valobj: lldb.SBValue) -> bool:
    return valobj.GetValueAsUnsigned(0) == 0


# Slang::String summary
def String_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return '""'
    m_buffer = valobj.GetChildMemberWithName("m_buffer")
    if not m_buffer.IsValid():
        return '""'
    buffer_ptr = get_ref_pointer(m_buffer)
    if not buffer_ptr.IsValid() or is_nullptr(buffer_ptr):
        return '""'
    buffer = buffer_ptr.Dereference()
    if not buffer.IsValid():
        return '""'
    length_val = buffer.GetChildMemberWithName("length")
    if not length_val.IsValid():
        return '""'
    length = length_val.GetValueAsUnsigned(0)
    if length == 0:
        return '""'
    data = buffer_ptr.GetPointeeData(1, length)
    if not data.IsValid():
        return '""'
    return make_string(data, length)


# Slang::UnownedStringSlice summary
def UnownedStringSlice_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return '""'
    begin = valobj.GetChildMemberWithName("m_begin")
    if not begin.IsValid():
        return '""'
    end = valobj.GetChildMemberWithName("m_end")
    if not end.IsValid():
        return '""'
    begin_addr = begin.GetValueAsUnsigned(0)
    end_addr = end.GetValueAsUnsigned(0)
    if begin_addr == 0 or end_addr == 0:
        return '""'
    length = end_addr - begin_addr
    if length <= 0:
        return '""'
    data = begin.GetPointeeData(0, length)
    if not data.IsValid():
        return '""'
    return make_string(data, length)


# Slang::RefPtr synthetic provider
class RefPtr_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def has_children(self):
        return True

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        for idx in range(self.num_children()):
            if self.children[idx].GetName() == name:
                return idx
        return -1

    def get_child_at_index(self, idx):
        if idx >= 0 and idx < self.num_children():
            return self.children[idx]
        else:
            return None

    def update(self):
        valobj_nonsyn = self.valobj.GetNonSyntheticValue()
        if not valobj_nonsyn.IsValid():
            self.children = []
            return
        self.pointer = valobj_nonsyn.GetChildMemberWithName("pointer")
        self.children = []
        if self.pointer.IsValid() and not is_nullptr(self.pointer):
            pointee = self.pointer.Dereference()
            if pointee.IsValid():
                self.children = pointee.children


# Slang::RefPtr summary
def RefPtr_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return "nullptr"
    valobj_nonsyn = valobj.GetNonSyntheticValue()
    if not valobj_nonsyn.IsValid():
        return "nullptr"
    pointer = valobj_nonsyn.GetChildMemberWithName("pointer")
    if not pointer.IsValid() or is_nullptr(pointer):
        return "nullptr"
    pointee = pointer.Dereference()
    if not pointee.IsValid():
        return str(pointer.GetValue()) + " <invalid deref>"
    refcount_val = pointee.GetChildMemberWithName("referenceCount")
    if not refcount_val.IsValid():
        return str(pointer.GetValue())
    refcount = refcount_val.GetValueAsUnsigned(0)
    return str(pointer.GetValue()) + " refcount=" + str(refcount)


# Slang::ComPtr synthetic provider
class ComPtr_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def has_children(self):
        return len(self.children) > 0

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        for idx in range(self.num_children()):
            if self.children[idx].GetName() == name:
                return idx
        return -1

    def get_child_at_index(self, idx):
        if idx >= 0 and idx < self.num_children():
            return self.children[idx]
        else:
            return None

    def update(self):
        if not self.valobj.IsValid():
            self.children = []
            return
        self.pointer = self.valobj.GetChildMemberWithName("m_ptr")
        self.children = []
        if self.pointer.IsValid() and not is_nullptr(self.pointer):
            pointee = self.pointer.Dereference()
            if pointee.IsValid():
                self.children = pointee.children


# Slang::ComPtr summary
def ComPtr_summary(valobj: lldb.SBValue, dict) -> str:
    if not valobj.IsValid():
        return "nullptr"
    valobj_nonsyn = valobj.GetNonSyntheticValue()
    if not valobj_nonsyn.IsValid():
        return "nullptr"
    pointer = valobj_nonsyn.GetChildMemberWithName("m_ptr")
    if not pointer.IsValid() or is_nullptr(pointer):
        return "nullptr"
    return str(pointer.GetValue())


# Slang::Array synthetic provider
class Array_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def has_children(self):
        return True

    def num_children(self):
        return self.count.GetValueAsUnsigned(0)

    def get_child_index(self, name):
        return int(name.lstrip("[").rstrip("]"))

    def get_child_at_index(self, idx):
        if idx >= 0 and idx < self.num_children():
            offset = idx * self.data_size
            return self.buffer.CreateChildAtOffset(
                "[" + str(idx) + "]", offset, self.data_type
            )
        else:
            return None

    def update(self):
        if not self.valobj.IsValid():
            self.count = lldb.SBValue()
            return
        self.count = self.valobj.GetChildMemberWithName("m_count")
        self.buffer = self.valobj.GetChildMemberWithName("m_buffer")
        if not self.buffer.IsValid():
            return
        buffer_type = self.buffer.GetType()
        if not buffer_type.IsValid():
            return
        self.data_type = buffer_type.GetArrayElementType()
        if not self.data_type.IsValid():
            return
        self.data_size = self.data_type.GetByteSize()


# Slang::List synthetic provider
class List_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def has_children(self):
        return True

    def num_children(self):
        return self.count.GetValueAsUnsigned(0)

    def get_child_index(self, name):
        return int(name.lstrip("[").rstrip("]"))

    def get_child_at_index(self, idx):
        if idx >= 0 and idx < self.num_children():
            offset = idx * self.data_size
            return self.buffer.CreateChildAtOffset(
                "[" + str(idx) + "]", offset, self.data_type
            )
        else:
            return None

    def update(self):
        if not self.valobj.IsValid():
            self.count = lldb.SBValue()
            return
        self.count = self.valobj.GetChildMemberWithName("m_count")
        self.buffer = self.valobj.GetChildMemberWithName("m_buffer")
        if not self.buffer.IsValid():
            return
        buffer_type = self.buffer.GetType()
        if not buffer_type.IsValid():
            return
        self.data_type = buffer_type.GetPointeeType()
        if not self.data_type.IsValid():
            return
        self.data_size = self.data_type.GetByteSize()


# Slang::ShortList synthetic provider
class ShortList_synthetic(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj: lldb.SBValue, dict):
        self.valobj = valobj

    def has_children(self):
        return True

    def num_children(self):
        return self.count.GetValueAsUnsigned(0)

    def get_child_index(self, name):
        return int(name.lstrip("[").rstrip("]"))

    def get_child_at_index(self, idx):
        if idx >= 0 and idx < self.short_count:
            offset = idx * self.data_size
            return self.short_buffer.CreateChildAtOffset(
                "[" + str(idx) + "]", offset, self.data_type
            )
        elif idx >= self.short_count and idx < self.num_children():
            offset = (idx - self.short_count) * self.data_size
            return self.buffer.CreateChildAtOffset(
                "[" + str(idx) + "]", offset, self.data_type
            )
        else:
            return None

    def update(self):
        if not self.valobj.IsValid():
            self.count = lldb.SBValue()
            self.short_count = 0
            return
        self.count = self.valobj.GetChildMemberWithName("m_count")
        self.buffer = self.valobj.GetChildMemberWithName("m_buffer")
        self.short_buffer = self.valobj.GetChildMemberWithName("m_shortBuffer")
        if not self.short_buffer.IsValid():
            self.short_count = 0
        else:
            self.short_count = self.short_buffer.GetNumChildren()
        if not self.buffer.IsValid():
            return
        buffer_type = self.buffer.GetType()
        if not buffer_type.IsValid():
            return
        self.data_type = buffer_type.GetPointeeType()
        if not self.data_type.IsValid():
            return
        self.data_size = self.data_type.GetByteSize()


def __lldb_init_module(debugger: lldb.SBDebugger, internal_dict):
    if ENABLE_LOGGING:
        lldb.formatters.Logger._lldb_formatters_debug_level = 2

    commands = [
        # Slang::String
        "type summary add Slang::String -F core_lldb.String_summary -w slang",
        # Slang::UnownedStringSlice
        "type summary add Slang::UnownedStringSlice -F core_lldb.UnownedStringSlice_summary -w slang",
        # Slang::RefPtr
        'type synthetic add -x "^Slang::RefPtr<.+>$" -l core_lldb.RefPtr_synthetic -w slang',
        'type summary add -x "^Slang::RefPtr<.+>$" -F core_lldb.RefPtr_summary -w slang',
        # Slang::ComPtr
        'type synthetic add -x "^Slang::ComPtr<.+>$" -l core_lldb.ComPtr_synthetic -w slang',
        'type summary add -x "^Slang::ComPtr<.+>$" -F core_lldb.ComPtr_summary -w slang',
        # Slang::Array
        'type synthetic add -x "^Slang::Array<.+>$" -l core_lldb.Array_synthetic -w slang',
        'type summary add --x "^Slang::Array<.+>$" --summary-string "size=${svar%#}" -w slang',
        # Slang::List
        'type synthetic add -x "^Slang::List<.+>$" -l core_lldb.List_synthetic -w slang',
        'type summary add --expand -x "^Slang::List<.+>$" --summary-string "size=${var.m_count} capacity=${var.m_capacity}" -w slang',
        # Slang::ShortList
        'type synthetic add -x "^Slang::ShortList<.+>$" -l core_lldb.ShortList_synthetic -w slang',
        'type summary add --expand -x "^Slang::ShortList<.+>$" --summary-string "size=${var.m_count} capacity=${var.m_capacity}" -w slang',
        # Enable slang category
        "type category enable slang",
    ]

    for c in commands:
        debugger.HandleCommand(c)
