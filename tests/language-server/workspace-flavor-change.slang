//TEST:LANG_SERVER:

// Changing `slang.workspaceFlavor` must take effect for already-open documents: the VFX flavor
// defines FRAGMENT_ANNOTATION, which selects the other branch below. The hover before and after
// the setting change must therefore describe different declarations of `marker`.

#ifdef FRAGMENT_ANNOTATION
int vfxMarker;
#define marker vfxMarker
#else
int standardMarker;
#define marker standardMarker
#endif

void test()
{
    int v = marker;
//HOVER:17,13
//CONFIG:slang.workspaceFlavor="vfx"
//HOVER:17,13
// An unknown flavor value must keep the previous flavor (vfx), not reset it to standard.
//CONFIG:slang.workspaceFlavor="unknownFlavor"
//HOVER:17,13
//CONFIG:slang.workspaceFlavor="standard"
//HOVER:17,13
//CONFIG:slang.workspaceFlavor="vfx"
//HOVER:17,13
// An empty value selects the default flavor (standard), whatever the previous flavor was.
//CONFIG:slang.workspaceFlavor=""
//HOVER:17,13
}
