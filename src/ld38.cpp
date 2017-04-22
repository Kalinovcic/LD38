#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
using namespace glm;

#include "SDL2/SDL.h"
#undef main

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace ld38
{

template <class Lambda> class AtScopeExit {
  Lambda& m_lambda;
public:
  AtScopeExit(Lambda& action) : m_lambda(action) {}
  ~AtScopeExit() { m_lambda(); }
};

#define Auto_INTERNAL2(lname, aname, ...) \
    auto lname = [&]() { __VA_ARGS__; }; \
    AtScopeExit<decltype(lname)> aname(lname);

#define Auto_TOKENPASTE(x, y) Auto_ ## x ## y

#define Auto_INTERNAL1(ctr, ...) \
    Auto_INTERNAL2(Auto_TOKENPASTE(func_, ctr), \
                   Auto_TOKENPASTE(instance_, ctr), __VA_ARGS__)

#define defer(...) Auto_INTERNAL1(__COUNTER__, __VA_ARGS__)


typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define PI 3.14159265

char* get_run_tree_path();
void report(const char* message);

#include "util.cpp"

char* folder_run_tree   = get_run_tree_path();
char* folder_data       = concat(folder_run_tree, "data\\");
char* folder_shaders    = concat(folder_data, "shaders\\");
char* folder_textures   = concat(folder_data, "textures\\");

#define WINDOW_TITLE    "LD38"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

bool game_requests_close = false;

void quit();
void entry();

#include "win32.cpp"
#include "graphics.cpp"
#include "startup.cpp"

}

// int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
    ld38::entry();
    return EXIT_SUCCESS;
}
