#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <algorithm>
#include <vector>
using std::sort;
using std::vector;

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

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

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

#define PI  3.14159265359
#define TAU 6.28318530718

char* get_run_tree_path();
void report(const char* message);

#include "util.cpp"

enum Texture
{
    TEXTURE_PLANET,
    TEXTURE_PLANET_GLOW,
    TEXTURE_PLAYER,
    TEXTURE_STUPID,

    TEXTURE_PLANT1,
    TEXTURE_PLANT2,
    TEXTURE_PLANT3,
    TEXTURE_PLANT4,
    TEXTURE_PLANT_LAST, // doesn't have a texture assigned

    TEXTURE_TALLPLANT1,
    TEXTURE_TALLPLANT2,
    TEXTURE_TALLPLANT3,
    TEXTURE_TALLPLANT4,
    TEXTURE_TALLPLANT5,
    TEXTURE_TALLPLANT_LAST, // doesn't have a texture assigned

    TEXTURE_TREE1,
    TEXTURE_PILLAR,

    TEXTURE_EVILPLANT1,
    TEXTURE_EVILPLANT2,
    TEXTURE_EVILPLANT3,
    TEXTURE_EVILPLANT4,
    TEXTURE_EVILPLANT_LAST, // doesn't have a texture assigned

    TEXTURE_EVILTALLPLANT1,
    TEXTURE_EVILTALLPLANT2,
    TEXTURE_EVILTALLPLANT3,
    TEXTURE_EVILTALLPLANT4,
    TEXTURE_EVILTALLPLANT5,
    TEXTURE_EVILTALLPLANT_LAST, // doesn't have a texture assigned

    TEXTURE_SPARKLE1,
    TEXTURE_SPARKLE2,
    TEXTURE_SPARKLE_LAST, // doesn't have a texture assigned

    TEXTURE_FIRE,

    TEXTURE_COUNT
};

enum Entity_Kind
{
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_STATIC,
};

enum Layer
{
    LAYER_BACK_DECORATION,
    LAYER_ACTORS,
    LAYER_FRONT_DECORATION,
    LAYER_VERY_FRONT_DECORATION,

    LAYER_COUNT
};

struct Planet;

struct Entity
{
    Planet* planet;
    Layer layer;
    Texture texture;
    Entity_Kind brain;
    float offset;
    float angle;
    vec2 size;
    float y_velocity;
};

#define PARTICLE_PROJECTILE 0x01

struct Particle
{
    uint32 flags;
    Texture texture;
    vec2 position;
    vec2 velocity;
    vec2 acceleration;
    float damping;
    float life;
    float wobble;
    float size;
};

struct Planet
{
    vec2 position;
    float radius;
    vector<Entity> entities;
    vector<int> remove_list;
    vector<Particle> particles;
};

struct Platform
{
    float x;
    float y;
    float width;
};

struct Font
{
    stbtt_bakedchar cdata[96]; // ASCII 32..126
    GLuint texture;
};

Font regular_font;

#define DEG2RAD 0.0174533

int window_width, window_height;

bool input_left;
bool input_right;
bool input_space;

SDL_Window* the_window = NULL;

char* folder_run_tree   = get_run_tree_path();
char* folder_data       = concat(folder_run_tree, "data\\");
char* folder_shaders    = concat(folder_data, "shaders\\");
char* folder_textures   = concat(folder_data, "textures\\");
char* folder_fonts      = concat(folder_data, "fonts\\");

#define WINDOW_TITLE    "LD38"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

bool game_requests_close = false;

void quit();
void entry();

#include "win32.cpp"
#include "graphics.cpp"
#include "entity.cpp"
#include "planet.cpp"
#include "startup.cpp"

}

// int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main(int argc, char** argv)
{
    ld38::entry();
    return EXIT_SUCCESS;
}
