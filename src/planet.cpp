
void update_planet(Planet* planet)
{
    for (int i = 0; i < planet->entities.size(); i++)
        update_entity(&planet->entities[i], i);

    sort(planet->remove_list.rbegin(), planet->remove_list.rend());
    for (int i : planet->remove_list)
        planet->entities.erase(planet->entities.begin() + i);
    planet->remove_list.clear();
}

void draw_planet(Planet* planet)
{
    draw_circle(TEXTURE_PLANET_GLOW, planet->position, planet->radius + 400, 128, camera_rotation);
    end_batch();
    begin_batch(the_atlas_texture);

    draw_circle(TEXTURE_PLANET, planet->position, planet->radius, 128, 0);

    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        end_batch();
        begin_batch(the_atlas_texture);
        for (int i = 0; i < planet->entities.size(); i++)
        {
            if (planet->entities[i].layer != layer) continue;
            render_entity(&planet->entities[i]);
        }
    }

    end_batch();
    begin_batch(the_atlas_texture);
    
    auto it = planet->particles.begin();
    while (it != planet->particles.end())
    {
        Particle& p = *it;
        p.position += p.velocity / 60.0f;
        p.velocity += p.acceleration / 60.0f;
        p.velocity *= p.damping;
        float wob = sin(p.life * 15) * p.wobble;
        vec2 render_position = p.position;
        render_position += normalize(vec2(-p.velocity.y, p.velocity.x)) * wob;
        draw_rectangle(p.texture, render_position, vec2(p.size, p.size), 0);
        p.life -= 1.0 / 60.0;
        if (p.life < 0)
            it = planet->particles.erase(it);
        else
            it++;
    }
}

void populate_angle_with_plants(Planet* planet, float angle, float angle_width, float base_offset)
{
    Entity e;
    e.flags = ENTITY_FLAG_LIFE;
    e.planet = planet;
    e.brain = ENTITY_STATIC;
    e.layer = LAYER_BACK_DECORATION;
    e.y_velocity = 0;
    int count = (int)(angle_width / TAU * 100 + 0.5);
    for (int i = 0; i < count; i++)
    {
        e.texture = (Texture)(TEXTURE_PLANT1 + rand() % (TEXTURE_PLANT_LAST - TEXTURE_PLANT1));
        e.angle = angle + i / (float) count * angle_width;
        e.size = scale_to_height(e.texture, 50);

        float offset = (rand() % 1000) / 1000.0;
        e.offset = base_offset - (e.size.y * 0.1 + offset * offset * 50.0);

        planet->entities.push_back(e);
    }
    e.layer = LAYER_VERY_FRONT_DECORATION;
    count = (int)(angle_width / TAU * 60 + 0.5);
    for (int i = 0; i < angle_width / TAU * 60; i++)
    {
        e.texture = (Texture)(TEXTURE_PLANT1 + rand() % (TEXTURE_PLANT_LAST - TEXTURE_PLANT1));
        e.angle = angle + i / (float) count * angle_width;
        e.size = scale_to_height(e.texture, 50);

        float offset = (rand() % 1000) / 1000.0;
        e.offset = base_offset - (e.size.y * 0.1 + offset * offset * offset * 20.0);

        planet->entities.push_back(e);
    }
    e.offset = base_offset;
    count = (int)(angle_width / TAU * 15 + 0.5);
    for (int i = 0; i < angle_width / TAU * 15; i++)
    {
        e.texture = (Texture)(TEXTURE_TALLPLANT1 + rand() % (TEXTURE_TALLPLANT_LAST - TEXTURE_TALLPLANT1));
        e.angle = angle + (rand() % 1000) / 1000.0 * angle_width;
        e.size = scale_to_height(e.texture, 120);

        planet->entities.push_back(e);
    }
}

void place_platforms(Planet* planet, Texture texture, float width, float angle_from, float angle_to, float offset)
{
    if (angle_to < angle_from)
        angle_to += TAU;

    Entity e;
    e.planet = planet;
    e.brain = (texture == TEXTURE_EVILPLATFORM) ? ENTITY_ANGLE_FIRE : ENTITY_STATIC;
    e.layer = LAYER_BACK_DECORATION;
    e.y_velocity = 0;
    e.texture = texture;
    e.size = scale_to_width(e.texture, width);
    e.offset = offset - e.size.y;

    float radius = planet->radius + offset;
    float angle_width = 2 * atan((width / 2) / radius);
    e.angle = angle_from + angle_width * 0.5;
    int index = 0;
    while (e.angle < angle_to)
    {
        e.flags = 0;
        if ((index % 8) != 0)
            e.flags |= ENTITY_FLAG_MUTE;
        index++;
        planet->entities.push_back(e);
        e.angle += angle_width;
    }
}
