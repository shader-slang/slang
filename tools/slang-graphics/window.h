// window.h
#pragma once

namespace slang_graphics {

struct WindowDesc
{
    char const* title;
    int width;
    int height;
};

typedef struct Window Window;

Window* createWindow(WindowDesc const& desc);
void showWindow(Window* window);
void* getPlatformWindowHandle(Window* window);

/// Opaque state provided by platform for a running application.
typedef struct ApplicationContext ApplicationContext;

/// User-defined application entry-point function.
typedef void(*ApplicationFunc)(ApplicationContext* context);

/// Dispatch any pending events for application.
///
/// @returns `true` if application should keep running.
bool dispatchEvents(ApplicationContext* context);

/// Exit the application with a given result code
void exitApplication(ApplicationContext* context, int resultCode);

/// Report an error to an appropriate logging destination.
int reportError(char const* message, ...);

/// Run an application given the specified callback and command-line arguments.
int runApplication(
    ApplicationFunc     func,
    int                 argc,
    char const* const*  argv);

#define SG_CONSOLE_MAIN(APPLICATION_ENTRY)  \
    int main(int argc, char** argv) {       \
        return slang_graphics::runApplication(&(APPLIATION_ENTRY), argc, argv); \
    }

#ifdef _WIN32

int runWindowsApplication(
    ApplicationFunc     func,
    void*               instance,
    int                 showCommand);

#define SG_UI_MAIN(APPLICATION_ENTRY)   \
    int __stdcall WinMain(              \
        void*   instance,               \
        void*   /* prevInstance */,     \
        void*   /* commandLine */,      \
        int     showCommand) {          \
        return slang_graphics::runWindowsApplication(&(APPLICATION_ENTRY), instance, showCommand); \
    }

#else

#define SG_UI_MAIN(APPLICATION_ENTRY) SG_CONSOLE_MAIN(APPLICATION_ENTRY)

#endif

} // slang_graphics
