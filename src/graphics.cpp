
GLuint the_vertex_array;
GLuint the_shader;

//////////////////
// atlas
//////////////////

const int ATLAS_PADDING = 2;
const int ATLAS_SIZE = 1024;
stbrp_node the_atlas_packing_nodes[ATLAS_SIZE * 2];
stbrp_context the_packing_context;
stbrp_rect the_atlas_rects[TEXTURE_COUNT];
uint8* the_atlas_textures[TEXTURE_COUNT];
vector<Platform> texture_platforms[TEXTURE_COUNT];

GLuint the_atlas_texture;
vec2 atlas_low[TEXTURE_COUNT];
vec2 atlas_high[TEXTURE_COUNT];
uint8* atlas_data;

void begin_atlas()
{
    stbrp_init_target(&the_packing_context, ATLAS_SIZE, ATLAS_SIZE, &the_atlas_packing_nodes[0], ATLAS_SIZE * 2);
}

void end_atlas()
{
    if (!stbrp_pack_rects(&the_packing_context, &the_atlas_rects[0], TEXTURE_COUNT))
        report("Failed to pack all textures into the atlas.");

    auto size = ATLAS_SIZE * ATLAS_SIZE * 4;
    atlas_data = (uint8*) malloc(size);
    memset(atlas_data, 0, size);

    for (int texture = 0; texture < TEXTURE_COUNT; texture++)
    {
        stbrp_rect* rect = &the_atlas_rects[texture];

        auto data = the_atlas_textures[rect->id];
        int texture_width  = rect->w - 2 * ATLAS_PADDING;
        int texture_height = rect->h - 2 * ATLAS_PADDING;

        atlas_low[texture].x = (rect->x + ATLAS_PADDING) / (float) ATLAS_SIZE;
        atlas_low[texture].y = (rect->y + ATLAS_PADDING) / (float) ATLAS_SIZE;
        atlas_high[texture].x = atlas_low[texture].x + texture_width  / (float) ATLAS_SIZE;
        atlas_high[texture].y = atlas_low[texture].y + texture_height / (float) ATLAS_SIZE;

        for (int ty = 0; ty < texture_height; ty++)
        {
            int ay = rect->y + (texture_height - ty - 1) + ATLAS_PADDING;
            int platform_x = -1;
            for (int tx = 0; tx < texture_width; tx++)
            {
                int ax = rect->x + tx + ATLAS_PADDING;
                auto pixel = data + (ty * texture_width + tx) * 4;
                bool pink = pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255 && pixel[3] == 255;
                if (pink)
                {
                    if (platform_x == -1) platform_x = tx;
                    pixel[0] = pixel[1] = pixel[2] = pixel[3] = 0;
                }
                if (platform_x != -1 && (tx == (texture_width - 1) || !pink))
                {
                    int platform_width = tx - platform_x + 1;
                    Platform platform;
                    platform.x = platform_x / (float) texture_width;
                    platform.y = 1 - (ty + 2) / (float) texture_height;
                    platform.width = platform_width / (float) texture_width;
                    texture_platforms[texture].push_back(platform);
                    platform_x = -1;
                }
                memcpy(atlas_data + (ay * ATLAS_SIZE + ax) * 4, pixel, 4);
            }
        }
    }

    glGenTextures(1, &the_atlas_texture);
    glBindTexture(GL_TEXTURE_2D, the_atlas_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void add_texture(Texture id, char* name)
{
    auto path = concat(folder_textures, name);

    int width, height, channels;
    auto bytes = (uint8*) stbi_load(path, &width, &height, &channels, 4);
    if (!bytes)
        report(concat("Failed to load texture: ", path));

    the_atlas_rects[id].id = id;
    the_atlas_rects[id].w = (stbrp_coord)(width + 2 * ATLAS_PADDING);
    the_atlas_rects[id].h = (stbrp_coord)(height + 2 * ATLAS_PADDING);

    the_atlas_textures[id] = bytes;
}

void create_atlas()
{
    begin_atlas();
    add_texture(TEXTURE_PLANET, "planet.png");
    add_texture(TEXTURE_PLANET_GLOW, "planet_glow.png");
    add_texture(TEXTURE_PLAYER, "player.png");
    add_texture(TEXTURE_STUPID, "stupid.png");
    add_texture(TEXTURE_PLANT1, "plant1.png");
    add_texture(TEXTURE_PLANT2, "plant2.png");
    add_texture(TEXTURE_PLANT3, "plant3.png");
    add_texture(TEXTURE_PLANT4, "plant4.png");
    add_texture(TEXTURE_TALLPLANT1, "tallplant1.png");
    add_texture(TEXTURE_TALLPLANT2, "tallplant2.png");
    add_texture(TEXTURE_TALLPLANT3, "tallplant3.png");
    add_texture(TEXTURE_TALLPLANT4, "tallplant4.png");
    add_texture(TEXTURE_TALLPLANT5, "tallplant5.png");
    add_texture(TEXTURE_TREE1, "tree1.png");
    add_texture(TEXTURE_PILLAR, "pillar.png");
    add_texture(TEXTURE_EVILPLANT1, "evilplant1.png");
    add_texture(TEXTURE_EVILPLANT2, "evilplant2.png");
    add_texture(TEXTURE_EVILPLANT3, "evilplant3.png");
    add_texture(TEXTURE_EVILPLANT4, "evilplant4.png");
    add_texture(TEXTURE_EVILTALLPLANT1, "eviltallplant1.png");
    add_texture(TEXTURE_EVILTALLPLANT2, "eviltallplant2.png");
    add_texture(TEXTURE_EVILTALLPLANT3, "eviltallplant3.png");
    add_texture(TEXTURE_EVILTALLPLANT4, "eviltallplant4.png");
    add_texture(TEXTURE_EVILTALLPLANT5, "eviltallplant5.png");
    add_texture(TEXTURE_SPARKLE1, "sparkle1.png");
    add_texture(TEXTURE_SPARKLE2, "sparkle2.png");
    add_texture(TEXTURE_FIRE, "fire.png");
    end_atlas();
}

//////////////////
// batch
//////////////////

struct Vertex
{
    float x;
    float y;
    float u;
    float v;
};

const int BATCH_SIZE = 8192;
Vertex batch_vertices[BATCH_SIZE];
int current_batch_size = 0;
GLuint current_batch_atlas;
bool current_batch_gui;

float camera_rotation;
vec2 camera_position;

void begin_batch(GLuint atlas, bool gui = false)
{
    mat4 camera_transform;
    if (gui)
        camera_transform = scale(vec3(2.0 / window_width, 2.0 / window_height, 1));
    else
        camera_transform = scale(vec3(2.0 / window_width, 2.0 / window_height, 1)) * rotate(camera_rotation, vec3(0, 0, 1)) * translate(vec3(camera_position, 0));

    glBindVertexArray(the_vertex_array);

    glUseProgram(the_shader);

    current_batch_gui = gui;
    current_batch_atlas = atlas;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, current_batch_atlas);
    glUniform1i(glGetUniformLocation(the_shader, "atlas"), 0);
    glUniformMatrix4fv(glGetUniformLocation(the_shader, "transform"), 1, GL_FALSE, (GLfloat*) &camera_transform);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    current_batch_size = 0;
}

void end_batch()
{
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, current_batch_size * sizeof(Vertex), batch_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 8);

    glDrawArrays(GL_TRIANGLES, 0, current_batch_size);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glDeleteBuffers(1, &vertex_buffer);
}

void batch_triangle(Vertex a, Vertex b, Vertex c)
{
    if (current_batch_size + 3 >= BATCH_SIZE)
    {
        end_batch();
        begin_batch(current_batch_atlas, current_batch_gui);
    }

    batch_vertices[current_batch_size + 0] = a;
    batch_vertices[current_batch_size + 1] = b;
    batch_vertices[current_batch_size + 2] = c;
    current_batch_size += 3;
}

void draw_rectangle(Texture texture, vec2 bottom, vec2 size, float angle)
{
    float s0 = 0.5 * size.x * sin(angle + PI / 2);
    float c0 = 0.5 * size.x * cos(angle + PI / 2);
    float s1 = size.y * sin(angle);
    float c1 = size.y * cos(angle);

    auto low  = atlas_low [texture];
    auto high = atlas_high[texture];
    Vertex rectangle[4];
    rectangle[0] = { bottom.x - s0, bottom.y - c0,  low.x, low.y };
    rectangle[1] = { bottom.x + s0, bottom.y + c0, high.x, low.y };
    rectangle[2] = { bottom.x + s0 + s1, bottom.y + c0 + c1, high.x, high.y };
    rectangle[3] = { bottom.x - s0 + s1, bottom.y - c0 + c1,  low.x, high.y };
    batch_triangle(rectangle[0], rectangle[1], rectangle[2]);
    batch_triangle(rectangle[0], rectangle[3], rectangle[2]);
}

void draw_circle(Texture texture, vec2 pos, float radius, int detail, float angle)
{
    auto low  = atlas_low [texture];
    auto high = atlas_high[texture];
    for (int i = 0; i < detail; i++)
    {
        auto a0 = (i + 0) / (float) detail * 2 * PI;
        auto a1 = (i + 1) / (float) detail * 2 * PI;

        Vertex triangle[3];
        triangle[0] = { (float) sin(a0 + angle), (float) cos(a0 + angle), (float) sin(a0), (float) cos(a0) };
        triangle[1] = { (float) sin(a1 + angle), (float) cos(a1 + angle), (float) sin(a1), (float) cos(a1) };
        triangle[2] = { 0, 0, 0, 0 };
        for (int j = 0; j < 3; j++)
        {
            triangle[j].u = low.x + ((float) triangle[j].u + 1.0) / 2.0 * (high.x - low.x);
            triangle[j].v = low.y + ((float) triangle[j].v + 1.0) / 2.0 * (high.y - low.y);
            triangle[j].x = triangle[j].x * radius + pos.x;
            triangle[j].y = triangle[j].y * radius + pos.y;
        }
        batch_triangle(triangle[0], triangle[1], triangle[2]);
    }
}

//////////////////
// font
//////////////////

const int FONT_BITMAP_SIZE = 1024;

void load_font(Font* font, char* name)
{
    char* path = concat(folder_fonts, name);
    defer(free(path));
    uint8* ttf = (uint8*) read_entire_file(path);
    defer(free(ttf));

    auto alphamap = (uint8*) malloc(FONT_BITMAP_SIZE * FONT_BITMAP_SIZE);
    defer(free(alphamap));
    stbtt_BakeFontBitmap(ttf, 0, 32.0, alphamap, FONT_BITMAP_SIZE, FONT_BITMAP_SIZE, 32, 96, &font->cdata[0]);

    auto bitmap = (uint8*) malloc(FONT_BITMAP_SIZE * FONT_BITMAP_SIZE * 4);
    defer(free(bitmap));
    for (int y = 0; y < FONT_BITMAP_SIZE; y++)
    {
        for (int x = 0; x < FONT_BITMAP_SIZE; x++)
        {
            int ia = y * FONT_BITMAP_SIZE + x;
            int ib = ia * 4;
            bitmap[ib] = bitmap[ib + 1] = bitmap[ib + 2] = 1;
            bitmap[ib + 3] = alphamap[ia];
        }
    }

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FONT_BITMAP_SIZE, FONT_BITMAP_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void render_string(Font* font, float x, float y, float scale, char* text)
{
    end_batch();
    begin_batch(font->texture, true);

    float tx = 0, ty = 0;
    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(&font->cdata[0], FONT_BITMAP_SIZE, FONT_BITMAP_SIZE, *text - 32, &tx, &ty, &q, 1);

            Vertex rectangle[4];
            rectangle[0] = { x + scale * q.x0, y + scale * q.y0, q.s0, q.t1 };
            rectangle[1] = { x + scale * q.x1, y + scale * q.y0, q.s1, q.t1 };
            rectangle[2] = { x + scale * q.x1, y + scale * q.y1, q.s1, q.t0 };
            rectangle[3] = { x + scale * q.x0, y + scale * q.y1, q.s0, q.t0 };

            batch_triangle(rectangle[0], rectangle[1], rectangle[2]);
            batch_triangle(rectangle[0], rectangle[3], rectangle[2]);
        }
        text++;
    }

    end_batch();
}

//////////////////
// generic stuff
//////////////////

GLuint load_shader(char* vertex_name, char* fragment_name)
{
    char* vertex_path   = concat(folder_shaders, vertex_name);
    char* fragment_path = concat(folder_shaders, fragment_name);

    GLchar* vertex_source   = (GLchar*) read_entire_file(vertex_path);
    GLchar* fragment_source = (GLchar*) read_entire_file(fragment_path);

    GLint result = GL_FALSE;
    int info_log_length;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length)
    {
        char* message = (char*) malloc(info_log_length);
        glGetShaderInfoLog(vertex_shader, info_log_length, NULL, message);
        fprintf(stderr, "%s\n", message);
    }

    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length)
    {
        char* message = (char*) malloc(info_log_length);
        glGetShaderInfoLog(fragment_shader, info_log_length, NULL, message);
        fprintf(stderr, "%s\n", message);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length)
    {
        char* message = (char*) malloc(info_log_length);
        glGetProgramInfoLog(program, info_log_length, NULL, message);
        fprintf(stderr, "%s\n", message);
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void init_opengl()
{
    glClearColor(86/255.0, 162/255.0, 239/255.0, 1);

    glGenVertexArrays(1, &the_vertex_array);
    glBindVertexArray(the_vertex_array);

    the_shader = load_shader("test.vs", "test.fs");
    create_atlas();
    load_font(&regular_font, "Newlandn.ttf");
}
