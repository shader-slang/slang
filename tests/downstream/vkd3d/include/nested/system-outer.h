#undef INCLUDE_CHOICE
#include <include-choice.h>
#if INCLUDE_CHOICE != 2
#error Wrong nested system include search order
#endif

#undef INCLUDE_CHOICE
#include "include-choice.h"
#if INCLUDE_CHOICE != 3
#error Wrong nested local include search order
#endif
