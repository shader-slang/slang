# Type Traits

**TODO**: Describe all language-related type traits.

Incomplete list:
- copyable/non-copyable (do we have movable/non-movable?)
- opaque/non-opaque
- known size/unknown size. Note: this might need further classification into implementation-specified sizes
  known by `slangc` vs sizes that only the target compiler knows (but sizeof() would still work)
- allowed storage duration: static (globals), block (function locals/params/return)
- serializable/non-serializable (types that can be assigned to interface-typed variables)
- addressable/non-addressable (types that are valid types for pointers)
- etc
