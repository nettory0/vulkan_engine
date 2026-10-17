#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "app.h"


int main()
{
    CApplication app;

    //WCHAR path[500];
    //DWORD size = GetModuleFileNameW(NULL, path, 500);

    //const WCHAR* wc = path;
   // _bstr_t b(path);
   // const char* c = b;
    //printf("%s\n", c);

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}