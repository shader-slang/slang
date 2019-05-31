// slang-object-meta-end.h

#undef SYNTAX_CLASS
#undef ABSTRACT_SYNTAX_CLASS
#undef END_SYNTAX_CLASS
#undef SYNTAX_FIELD
#undef FIELD
#undef FIELD_INIT
#undef DECL_FIELD
#undef RAW
#undef SIMPLE_SYNTAX_CLASS

// Hack to remove 'warning C4702: unreachable code' on VS2017, blocking compilation
// Note! This is matched in object-meta-begin.h 
#if _MSC_VER >= 1910
#pragma warning(pop)
#endif
