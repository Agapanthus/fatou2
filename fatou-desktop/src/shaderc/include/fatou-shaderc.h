#pragma once

/*
#ifdef FATOULIBRARY_EXPORTS
#define FATOULIBRARY_API __declspec(dllexport)
#else
#define FATOULIBRARY_API __declspec(dllimport)
#endif
*/

#define FATOULIBRARY_API

FATOULIBRARY_API void *compileShaderFromFile(const path &path, bool isVertex,
                                             size_t &len);