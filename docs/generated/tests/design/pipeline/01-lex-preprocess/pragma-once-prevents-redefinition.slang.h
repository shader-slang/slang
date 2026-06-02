// Helper for pragma-once-prevents-redefinition.slang. The
// `#pragma once` directive marks this file as include-once; the
// `once_value` declaration must therefore be parsed exactly one
// time even if the parent #includes us twice.
#pragma once
static const int once_value = 64;
