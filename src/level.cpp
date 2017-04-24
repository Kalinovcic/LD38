
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
    srand(time(0));
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
}

void update_and_render_level(Planet* planet)
{
    update_planet(planet);

    vec4 sky_color = vec4(86/255.0, 162/255.0, 239/255.0, 1);
    switch (state)
    {
    case STATE_INTRO:
    {
        static const float INTRO_DURATION = 5.0;
        static const float ZOOM_DURATION = INTRO_DURATION * 0.5;
        static const float FADE_DURATION = INTRO_DURATION * 0.1;

        float light = smoothstep(0.0f, FADE_DURATION, state_time);
        camera_color = vec4(light);
        sky_color *= light;

        camera_position = -planet->position;
        camera_zoom = 0.2 + smoothstep(0.0f, ZOOM_DURATION, state_time) * 0.3;
        if (state_time >= ZOOM_DURATION)
        {
            int count_total = 0;
            int count_infested = 0;
            vector<int> life_indices;

            for (int index = 0; index < planet->entities.size(); index++)
            {
                auto& e = planet->entities[index];
                if (e.brain == ENTITY_PLAYER)
                    camera_rotation = e.angle;
                if (e.flags & ENTITY_FLAG_LIFE)
                {
                    count_total++;
                    life_indices.push_back(index);
                }
                if (e.flags & ENTITY_FLAG_INFESTED)
                {
                    count_total++;
                    count_infested++;
                }
            }

            int target_count_infested = (int)(count_total * (state_time - ZOOM_DURATION) / (INTRO_DURATION - ZOOM_DURATION) + 0.5);
            if (target_count_infested > count_total || state_time >= INTRO_DURATION)
                target_count_infested = count_total;
            int to_infest = target_count_infested - count_infested;

            std::random_shuffle(life_indices.begin(), life_indices.end());
            for (int i = 0; i < to_infest; i++)
            {
                auto& e = planet->entities[life_indices[i]];
                e.flags &= ~ENTITY_FLAG_LIFE;
                e.flags |=  ENTITY_FLAG_INFESTED;
                if (e.texture >= TEXTURE_PLANT1 && e.texture < TEXTURE_PLANT_LAST)
                    e.texture = (Texture)(TEXTURE_EVILPLANT1 + (e.texture - TEXTURE_PLANT1));
                if (e.texture >= TEXTURE_TALLPLANT1 && e.texture < TEXTURE_TALLPLANT_LAST)
                    e.texture = (Texture)(TEXTURE_EVILTALLPLANT1 + (e.texture - TEXTURE_TALLPLANT1));
            }
        }
        else
        {
            for (int index = 0; index < planet->entities.size(); index++)
            {
                auto& e = planet->entities[index];
                if (e.brain == ENTITY_PLAYER)
                    camera_rotation = e.angle;
            }
        }
        if (state_time >= INTRO_DURATION)
            state = STATE_PLAYING;
    } break;
    case STATE_SHORT_INTRO:
    {
        static const float INTRO_DURATION = 1.5;
        static const float ZOOM_DURATION = INTRO_DURATION * 1.0;
        static const float FADE_DURATION = INTRO_DURATION * 0.4;

        float light = smoothstep(0.0f, FADE_DURATION, state_time);
        camera_color = vec4(light);
        sky_color *= light;

        for (int index = 0; index < planet->entities.size(); index++)
        {
            auto& e = planet->entities[index];
            if (e.brain == ENTITY_PLAYER)
            {
                vec2 bottom = planet->position + vec2(sin(e.angle), cos(e.angle)) * (planet->radius + e.offset);
                camera_rotation = e.angle;
                camera_position = -bottom;
            }
        }
        camera_zoom = 0.4 + smoothstep(0.0f, ZOOM_DURATION, state_time) * 0.6;

        if (state_time >= INTRO_DURATION)
            state = STATE_PLAYING;
    } break;
    case STATE_PLAYING:
    {
        float target_zoom = 1.0;
        camera_zoom += (target_zoom - camera_zoom) * 0.05f;
        camera_color = vec4(1, 1, 1, 1);
    } break;
    case STATE_DEAD:
    {
        static const float DEAD_DURATION = 1.5;
        static const float ZOOM_DURATION = DEAD_DURATION * 1.0;
        static const float FADE_DURATION = DEAD_DURATION * 0.8;

        float light = 1 - smoothstep(0.0f, FADE_DURATION, state_time);
        camera_color = vec4(light);
        sky_color *= light;

        camera_zoom = 1.0 - smoothstep(0.0f, ZOOM_DURATION, state_time) * 0.8;

        vec2 target_position = -planet->position;
        camera_position += (target_position - camera_position) * 0.01f;

        if (state_time >= DEAD_DURATION)
        {
            load_level(planet, 1);
            state = STATE_SHORT_INTRO;
            state_time = 0;

            for (int index = 0; index < planet->entities.size(); index++)
            {
                auto& e = planet->entities[index];
                if (e.flags & ENTITY_FLAG_LIFE)
                {
                    e.flags &= ~ENTITY_FLAG_LIFE;
                    e.flags |=  ENTITY_FLAG_INFESTED;
                    if (e.texture >= TEXTURE_PLANT1 && e.texture < TEXTURE_PLANT_LAST)
                        e.texture = (Texture)(TEXTURE_EVILPLANT1 + (e.texture - TEXTURE_PLANT1));
                    if (e.texture >= TEXTURE_TALLPLANT1 && e.texture < TEXTURE_TALLPLANT_LAST)
                        e.texture = (Texture)(TEXTURE_EVILTALLPLANT1 + (e.texture - TEXTURE_TALLPLANT1));
                }
            }
        }
    } break;
    }
    glClearColor(sky_color.r, sky_color.g, sky_color.b, sky_color.a);
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
    int life_int = (int)(life * 100 + 0.5);
    if (count_infested && life_int == 100) life_int--;
    sprintf(buff, "Life: %d%%", life_int);
    render_string(&regular_font, window_width / -2.0 + 50, window_height / 2.0 - 50, 1, buff);
}
