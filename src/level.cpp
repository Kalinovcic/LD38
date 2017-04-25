
const int LEVEL_COUNT = 4;
int current_level_index = 1;

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
    for (auto hint : planet->hints)
        free(hint);
    planet->hints.clear();
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
        else if (*line == 'h')
        {
            line += 2;
            while (*line)
            {
                uint8* hint = line;
                while (*line && *line != '@')
                    line++;
                if (*line == '@')
                {
                    *line = 0;
                    line++;
                }
                int length = line - hint;
                auto buf = (char*) malloc(length + 1);
                memcpy(buf, hint, length);
                buf[length] = 0;
                planet->hints.push_back(buf);
            }
        }
        else if (*line == 'r')
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
            place_platforms(planet, TEXTURE_PLATFORM, 25, from * DEG2RAD, (from + width) * DEG2RAD, offset);
        }
        else if (*line == 'c')
        {
            float from, width, offset;
            sscanf((char*) line, "%*c%f%f%f", &from, &width, &offset);
            place_platforms(planet, TEXTURE_EVILPLATFORM, 25, from * DEG2RAD, (from + width) * DEG2RAD, offset);
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
            int size;
            sscanf((char*) line, "%*c%f%f%f%d", &angle, &offset, &height, &size);
            Entity e = {};
            e.planet = planet;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            Texture pillars[] = { TEXTURE_PILLAR_TINY, TEXTURE_PILLAR_SHORT, TEXTURE_PILLAR, TEXTURE_PILLAR_TALL, TEXTURE_PILLAR_TALLER, TEXTURE_PILLAR_VERY_TALL, TEXTURE_PILLAR_VERY_VERY_TALL, TEXTURE_PILLAR_TALLEST };
            e.texture = pillars[size - 1];
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
        else if (*line == 'm')
        {
            float angle, offset, height;
            sscanf((char*) line, "%*c%f%f%f", &angle, &offset, &height);
            Entity e = {};
            e.planet = planet;
            e.flags = ENTITY_FLAG_ENEMY;
            e.angle = angle * DEG2RAD;
            e.offset = offset;
            e.layer = layer;
            e.texture = TEXTURE_MISSILE;
            e.brain = ENTITY_MISSILE;
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
    if (!(state == STATE_TITLE || state == STATE_TITLE_STORY || state == STATE_STORY))
    {
        update_planet(planet);
    }

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
        {
            state = STATE_PLAYING;
            state_time = 0;
        }
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
        {
            state = STATE_PLAYING;
            state_time = 0;
        }
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
            load_level(planet, current_level_index);
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
    case STATE_ENDING:
    {
        static const float ENDING_DURATION = 12.0;
        static const float ZOOM_DURATION = ENDING_DURATION * 0.6;
        static const float FLY_AWAY_BEGINNING = ENDING_DURATION * 0.8;

        float light = 1 - smoothstep(FLY_AWAY_BEGINNING, ENDING_DURATION, state_time);
        camera_color = vec4(light);
        sky_color *= light;

        camera_zoom = 1.0 - smoothstep(0.0f, ZOOM_DURATION, state_time) * 0.8;
        if (state_time >= FLY_AWAY_BEGINNING)
        {
            vec2 target_position = -vec2(50000, 10000);
            camera_position += (target_position - camera_position) * 0.001f;
            camera_rotation -= 0.01;
        }
        else
        {
            vec2 target_position = -planet->position;
            camera_position += (target_position - camera_position) * 0.01f;
            camera_rotation += 0.001;
        }

        if (state_time >= ENDING_DURATION)
        {
            if (current_level_index == LEVEL_COUNT)
            {
                state = STATE_STORY;
                state_time = 0;
            }
            else
            {
                current_level_index++;
                load_level(planet, current_level_index);
            }
        }
    } break;
    case STATE_TITLE:
    {
        glClearColor(0, 0, 0, 1);
        defer(state_time += 1 / 60.0);
        camera_color = vec4(1, 1, 1, 1);
        render_string_centered(&regular_font, 0, 0, 2, "CARETAKER");
        camera_color = vec4(1, 1, 1, 0.5);
        render_string_centered(&regular_font, 0, -50, 0.5, "< space to begin >");
        render_string_centered(&regular_font, 0, -80, 0.5, "< F5 to toggle fullscreen >");
        render_string_centered(&regular_font, 0, -110, 0.5, "< F6 to toggle sound >");

        if (input_space)
        {
            state = STATE_TITLE_STORY;
            state_time = 0;
        }
        return;
    } break;
    case STATE_TITLE_STORY:
    {
        int line_count;
        char* lines[10];
        line_count = 5;
        lines[0] = "Hi,";
        lines[1] = "Life in our star system is being infested.";
        lines[2] = "You'll be sent to several smaller planets";
        lines[3] = "try your best to help the dying plant life.";
        lines[4] = "Thank you!";

        glClearColor(0, 0, 0, 1);
        defer(state_time += 1 / 60.0);

        static const float LINE_DURATION = 5.0;
        static const float STORY_DURATION = line_count * LINE_DURATION;
        int current_line = (int)(state_time / LINE_DURATION);
        if (current_line >= line_count || input_skip_level)
        {
            input_skip_level = false;
            current_level_index = 1;
            load_level(planet, current_level_index);
            return;
        }

        float line_time = fmod(state_time, LINE_DURATION);
        float light = 1.0f;
        if (line_time < LINE_DURATION * 0.2)
            light = smoothstep(0.0f, LINE_DURATION * 0.2f, line_time);
        if (line_time > LINE_DURATION * 0.8)
            light = 1 - smoothstep(LINE_DURATION * 0.8f, LINE_DURATION, line_time);
        camera_color = vec4(1, 1, 1, light);
        render_string_centered(&regular_font, 0, 0, 1, lines[current_line]);
        camera_color = vec4(1);
        return;
    }
    case STATE_STORY:
    {
        char custom[256];
        char custom2[256];
        int line_count;
        char* lines[10];
        if (murder_count)
        {
            char* username = get_user_name();
            if (username)
            {
                char* c = username;
                while (*c)
                {
                    if (*c < 32 || *c >= 128)
                    {
                        username = NULL;
                        break;
                    }
                    c++;
                }
            }
            if (username)
                sprintf(custom, "But, %s...", username);
            else
                sprintf(custom, "But...");
            sprintf(custom2, "You murdered %d infestor%s.", murder_count, (murder_count > 1) ? "s" : "");

            line_count = 5;
            lines[0] = "You've done well!";
            lines[1] = "Life in this star system is restored";
            lines[2] = custom;
            lines[3] = custom2;
            lines[4] = "Isn't that hypocritical...";
        }
        else
        {
            line_count = 3;
            lines[0] = "You've done well!";
            lines[1] = "Life in this star system is restored!";
            lines[2] = "Take care";
        }

        glClearColor(0, 0, 0, 1);
        defer(state_time += 1 / 60.0);

        static const float LINE_DURATION = 5.0;
        static const float STORY_DURATION = line_count * LINE_DURATION;
        int current_line = (int)(state_time / LINE_DURATION);
        if (current_line >= line_count)
        {
            current_line = line_count - 1;
            camera_color = vec4(1, 1, 1, 0.5);
            render_string_centered(&regular_font, 0, -50, 0.5, "< esc to quit >");
            render_string_centered(&regular_font, 0, -80, 0.5, "< space to restart >");
            render_string_centered(&regular_font, 0, -window_height / 2 + 60, 0.5, "Made by Lovro Kalinovcic");
            render_string_centered(&regular_font, 0, -window_height / 2 + 30, 0.5, "for Ludum Dare 38");

            if (input_escape)
            {
                game_requests_close = true;
            }
            if (input_space)
            {
                input_space = false;
                state = STATE_TITLE;
                state_time = 0;
            }
        }

        float line_time = fmod(state_time, LINE_DURATION);
        float light = 1.0f;
        if (line_time < LINE_DURATION * 0.2 && state_time < STORY_DURATION)
            light = smoothstep(0.0f, LINE_DURATION * 0.2f, line_time);
        if (line_time > LINE_DURATION * 0.8 && current_line < line_count - 1)
            light = 1 - smoothstep(LINE_DURATION * 0.8f, LINE_DURATION, line_time);
        camera_color = vec4(1, 1, 1, light);
        render_string_centered(&regular_font, 0, 0, 1, lines[current_line]);
        camera_color = vec4(1);

        return;
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
    render_string(&regular_font, window_width / -2.0 + 50, window_height / 2.0 - 70, 1, buff);

    if (state == STATE_PLAYING)
    {
        vec4 previous_color = camera_color;
        camera_color = vec4(1, 1, 1, smoothstep(0.0f, 1.0f, state_time));
        float y = window_height / 2.0 - 70;
        for (char* hint : planet->hints)
        {
            render_string_centered(&regular_font, 0, y, 0.5, hint);
            y -= 30;
        }
        camera_color = previous_color;
    }

    if (state == STATE_ENDING)
    {
        float light = smoothstep(0.0f, 1.0f, state_time);
        camera_color *= light;
        render_string_centered(&regular_font, 0, -window_height / 2.0 + 50, 1, "Life is restored!");
    }

    if (state == STATE_PLAYING && (count_infested <= 2))
    {
        state = STATE_ENDING;
        state_time = 0;
        for (int index = 0; index < planet->entities.size(); index++)
        {
            auto& e = planet->entities[index];
            if (e.flags & ENTITY_FLAG_INFESTED)
            {
                e.flags &= ~ENTITY_FLAG_INFESTED;
                e.flags |=  ENTITY_FLAG_LIFE;
                if (e.texture >= TEXTURE_EVILPLANT1 && e.texture < TEXTURE_EVILPLANT_LAST)
                    e.texture = (Texture)(TEXTURE_PLANT1 + (e.texture - TEXTURE_EVILPLANT1));
                if (e.texture >= TEXTURE_EVILTALLPLANT1 && e.texture < TEXTURE_EVILTALLPLANT_LAST)
                    e.texture = (Texture)(TEXTURE_TALLPLANT1 + (e.texture - TEXTURE_EVILTALLPLANT1));
            }
        }
    }

    if (input_skip_level)
    {
        input_skip_level = false;
        if (current_level_index == LEVEL_COUNT)
        {
            state = STATE_STORY;
            state_time = 0;
        }
        else
        {
            current_level_index++;
            load_level(planet, current_level_index);
        }
    }
}
