// object-meta-begin.h

#ifndef SYNTAX_CLASS
#error The 'SYNTAX_CLASS' macro should be defined before including 'object-meta-begin.h'
#endif

#ifndef ABSTRACT_SYNTAX_CLASS
#define ABSTRACT_SYNTAX_CLASS(NAME, BASE) SYNTAX_CLASS(NAME, BASE)
#endif

#ifndef END_SYNTAX_CLASS
#define END_SYNTAX_CLASS() /* empty */
#endif

#ifndef DECL_FIELD
#define DECL_FIELD(TYPE, NAME) SYNTAX_FIELD(TYPE, NAME)
#endif

#ifndef SYNTAX_FIELD
#define SYNTAX_FIELD(TYPE, NAME) FIELD(TYPE, NAME)
#endif

#ifndef FIELD_INIT
#define FIELD_INIT(TYPE, NAME, INIT) FIELD(TYPE, NAME)
#endif

#ifndef FIELD
#define FIELD(...) /* empty */
#endif

#ifndef RAW
#define RAW(...) /* empty */
#endif

#define SIMPLE_SYNTAX_CLASS(NAME, BASE) SYNTAX_CLASS(NAME, BASE) END_SYNTAX_CLASS()

