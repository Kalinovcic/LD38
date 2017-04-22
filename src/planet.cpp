
const int MAX_PLANET_COUNT = 128;
Planet planets[MAX_PLANET_COUNT];
int planet_count;

Planet* add_planet()
{
    return &planets[planet_count++];
}

void update_planet(Planet* planet)
{
    for (int i = 0; i < planet->entities.size(); i++)
        update_entity(&planet->entities[i]);

    sort(planet->remove_list.rbegin(), planet->remove_list.rend());
    for (int i : planet->remove_list)
        planet->entities.erase(planet->entities.begin() + i);
    planet->remove_list.clear();
}

void update_planets()
{
    for (int i = 0; i < planet_count; i++)
        update_planet(&planets[i]);
}

void draw_planet(Planet* planet)
{
    draw_circle(TEXTURE_PLANET_GLOW, planet->position, planet->radius + 400, 128, camera_rotation);
    end_batch();
    begin_batch();

    draw_circle(TEXTURE_PLANET, planet->position, planet->radius, 128, 0);

    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        end_batch();
        begin_batch();
        for (int i = 0; i < planet->entities.size(); i++)
        {
            if (planet->entities[i].layer != layer) continue;
            render_entity(&planet->entities[i]);
        }
    }
}

void draw_planets()
{
    for (int i = 0; i < planet_count; i++)
        draw_planet(&planets[i]);
}
