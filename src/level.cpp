
uint8* next_line(uint8** file)
{
    uint8* f = *file;
    uint8* line = f;
    while (*f && *f != '\n' && *f != '\r')
        f++;
    if (*f == 0)
    {
        *file = NULL;
    }
    else
    {
        *f = 0;
        *file = f + 1;
    }
    return line;
}

void load_level(Planet* planet, int index)
{
    planet->position = { 0, 0 };
    planet->entities.clear();
    planet->remove_list.clear();
    planet->particles.clear();

    char name[64];
    sprintf(name, "%d.txt", index);
    char* path = concat(folder_planets, name);
    defer(free(path));
    uint8* file = (uint8*) read_entire_file(path);
    defer(free(file));

    Layer layer = LAYER_ACTORS;
    while (file)
    {
        uint8* line = next_line(&file);
        while (*line == ' ' || *line == '\t') line++;
        if (*line == 0) continue;
        else if (*line == '#') continue;
        if (*line == 'r')
        {
            sscanf((char*) line, "%*c%f", &planet->radius);
        }
        else if (*line == 'a')
        {
            float from, width, offset;
            sscanf((char*) line, "%*c%f%f%f", &from, &width, &offset);
            populate_angle_with_plants(planet, from * DEG2RAD, width * DEG2RAD, offset);
        }
        else if (*line == 'b')
        {
            float from, width, offset;
            sscanf((char*) line, "%*c%f%f%f", &from, &width, &offset);
            place_platforms(planet, TEXTURE_PLATFORM, 100, from * DEG2RAD, (from + width) * DEG2RAD, offset);
        }
        else if (*line == 'c')
        {
            float from, width, offset;
            sscanf((char*) line, "%*c%f%f%f", &from, &width, &offset);
            place_platforms(planet, TEXTURE_EVILPLATFORM, 100, from * DEG2RAD, (from + width) * DEG2RAD, offset);
        }
        else if (*line == 'l')
        {
            int index;
            sscanf((char*) line, "%*c%d", &index);
            layer = (Layer) index;
        }
        else if (*line == 'o')
        {
            float angle, offset;
            sscanf((char*) line, "%*c%f%f", &angle, &offset);
            Entity e = {};
            e.planet = planet;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            e.texture = TEXTURE_PLAYER_STILL;
            e.brain = ENTITY_PLAYER;
            e.size = scale_to_height(e.texture, 80);
            planet->entities.push_back(e);
        }
        else if (*line == 't')
        {
            float angle, offset, height;
            sscanf((char*) line, "%*c%f%f%f", &angle, &offset, &height);
            Entity e = {};
            e.planet = planet;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            e.texture = TEXTURE_TREE1;
            e.brain = ENTITY_STATIC;
            e.size = scale_to_height(e.texture, height);
            planet->entities.push_back(e);
        }
        else if (*line == 'p')
        {
            float angle, offset, height;
            sscanf((char*) line, "%*c%f%f%f", &angle, &offset, &height);
            Entity e = {};
            e.planet = planet;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            e.texture = TEXTURE_PILLAR;
            e.brain = ENTITY_STATIC;
            e.size = scale_to_height(e.texture, height);
            planet->entities.push_back(e);
        }
        else if (*line == 'f')
        {
            float angle, offset, height;
            sscanf((char*) line, "%*c%f%f%f", &angle, &offset, &height);
            Entity e = {};
            e.planet = planet;
            e.flags = ENTITY_FLAG_ENEMY | ENTITY_FLAG_STOMPABLE;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            e.texture = TEXTURE_FIREBOI;
            e.brain = ENTITY_FIREBOI;
            e.size = scale_to_height(e.texture, height);
            planet->entities.push_back(e);
        }
        else
        {
            printf("Unrecognized planet line: %s\n", line);
        }
    }

    state = STATE_INTRO;
    state_time = 0;
    camera_zoom = 5.0;
}

void update_and_render_level(Planet* planet)
{
    update_planet(planet);

    switch (state)
    {
    case STATE_INTRO:
    {
        static const float INTRO_DURATION = 5.0;
        camera_position = -planet->position;
        camera_zoom = 0.2 + smoothstep(0.0f, INTRO_DURATION, state_time) * 0.3;
        if (state_time >= INTRO_DURATION)
            state = STATE_PLAYING;
    } break;
    case STATE_PLAYING:
    {
        float target_zoom = 1.0;
        camera_zoom += (target_zoom - camera_zoom) * 0.05f;
    } break;
    case STATE_DEAD:
    {
        float target_zoom = 0.2;
        vec2 target_position = -planet->position;
        camera_position += (target_position - camera_position) * 0.01f;
        camera_zoom += (target_zoom - camera_zoom) * 0.01f;
    } break;
    }
    state_time += 1 / 60.0;

    begin_batch(the_atlas_texture);
    draw_planet(planet);
    end_batch();

    int count_infested = 0;
    int count_total = 0;
    for (auto& e : planet->entities)
    {
        if (e.flags & ENTITY_FLAG_LIFE) count_total++;
        if (e.flags & ENTITY_FLAG_INFESTED) { count_total++; count_infested++; }
    }

    float life = 1 - (count_infested / (float) count_total);
    char buff[64];
    if (count_infested)
    {
        int life_int = (int)(life * 100 + 0.5);
        if (life_int == 100) life_int--;
        sprintf(buff, "Life: %d%%", life_int);
    }
    else
        sprintf(buff, "Life is saved!");
    render_string(&regular_font, window_width / -2.0 + 50, window_height / 2.0 - 50, 1, buff);
}
