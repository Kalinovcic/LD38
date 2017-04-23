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
#include "SDL2/SDL_mixer.h"
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

    TEXTURE_PLAYER_WALK1,
    TEXTURE_PLAYER_WALK2,
    TEXTURE_PLAYER_WALK_LAST,
    TEXTURE_PLAYER_STILL,
    TEXTURE_PLAYER_JUMP,
    TEXTURE_PLAYER_FALL,

    TEXTURE_FIREBOI,
    TEXTURE_FIREBOI_ATTACK,

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
    TEXTURE_PLATFORM,
    TEXTURE_EVILPLATFORM,

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
    ENTITY_STATIC,
    ENTITY_PLAYER,
    ENTITY_FIREBOI,
    ENTITY_ANGLE_FIRE,
    ENTITY_GRAVITY_BULLET,
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

#define ENTITY_FLAG_ENEMY           ((uint32) 0x01)
#define ENTITY_FLAG_HURTS           ((uint32) 0x02)
#define ENTITY_FLAG_STOMPABLE       ((uint32) 0x04)
#define ENTITY_FLAG_FLIP            ((uint32) 0x08)
#define ENTITY_FLAG_INFESTED        ((uint32) 0x10)
#define ENTITY_FLAG_LIFE            ((uint32) 0x20)
#define ENTITY_FLAG_MUTE            ((uint32) 0x40)

struct Entity
{
    uint32 flags = 0;
    Planet* planet;
    Layer layer;
    Texture texture;
    Entity_Kind brain;
    int frames_alive = 0;
    int frames_action = 0;
    float offset;
    float angle;
    vec2 size;
    float x_velocity;
    float y_velocity;
};

struct Particle
{
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

Mix_Chunk* sound_grass_walk[3];
Mix_Chunk* sound_grass_life[2];
Mix_Chunk* sound_bounce;
Mix_Chunk* sound_fireball;
Mix_Chunk* sound_fireball_out;

#define DEG2RAD 0.0174533

int window_width, window_height;

bool input_left;
bool input_right;
bool input_space;

float camera_rotation;
vec2 camera_position;

SDL_Window* the_window = NULL;

char* folder_run_tree   = get_run_tree_path();
char* folder_data       = concat(folder_run_tree, "data\\");
char* folder_audio      = concat(folder_data, "audio\\");
char* folder_shaders    = concat(folder_data, "shaders\\");
char* folder_textures   = concat(folder_data, "textures\\");
char* folder_fonts      = concat(folder_data, "fonts\\");

#define WINDOW_TITLE    "LD38"
#define WINDOW_WIDTH    1200
#define WINDOW_HEIGHT   675

bool game_requests_close = false;

void quit();
void entry();

#include "win32.cpp"
#include "audio.cpp"
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
