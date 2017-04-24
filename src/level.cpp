
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
}
