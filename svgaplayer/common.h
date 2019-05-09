#pragma  once

#ifdef BUILD_SVGA
#define SVGA_EXPORT __declspec(dllexport)
#else
#define SVGA_EXPORT __declspec(dllimport)
#endif
