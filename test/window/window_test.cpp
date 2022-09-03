#include "window/window.h"

int main( int argc, const char* argv[] )
{
    vkrender::Window window{ 1024, 768 };

    window.init();

    while( !window.quit() )
    {
        window.processEvents();
    }

    return 0;
}