
void update_planet(Planet* planet)
{
    for (int i = 0; i < planet->entities.size(); i++)
        update_entity(&planet->entities[i]);

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
        float offset = length(render_position - planet->position) - planet->radius;
        if (p.life < 0 || ((p.flags & PARTICLE_PROJECTILE) && offset < 0))
            it = planet->particles.erase(it);
        else
            it++;
    }
}

void populate_planet_with_plants(Planet* planet)
{
    srand(time(0));

    Entity e;
    e.planet = planet;
    e.brain = ENTITY_STATIC;
    e.layer = LAYER_BACK_DECORATION;
    e.y_velocity = 0;
    for (int i = 0; i < 100; i++)
    {
        e.texture = (Texture)(TEXTURE_EVILPLANT1 + rand() % (TEXTURE_EVILPLANT_LAST - TEXTURE_EVILPLANT1));
        e.angle = i / (float) 100 * TAU;

        vec2 texture_size = atlas_high[e.texture] - atlas_low[e.texture];
        e.size = { 50, 50 };
        e.size *= texture_size / texture_size.y;

        float offset = (rand() % 1000) / 1000.0;
        e.offset = -(e.size.y * 0.1 + offset * offset * 50.0);

        planet->entities.push_back(e);
    }
    e.layer = LAYER_VERY_FRONT_DECORATION;
    for (int i = 0; i < 60; i++)
    {
        e.texture = (Texture)(TEXTURE_EVILPLANT1 + rand() % (TEXTURE_EVILPLANT_LAST - TEXTURE_EVILPLANT1));
        e.angle = i / (float) 60 * TAU;

        vec2 texture_size = atlas_high[e.texture] - atlas_low[e.texture];
        e.size = { 50, 50 };
        e.size *= texture_size / texture_size.y;

        float offset = (rand() % 1000) / 1000.0;
        e.offset = -(e.size.y * 0.1 + offset * offset * offset * 20.0);

        planet->entities.push_back(e);
    }
    e.offset = 0;
    for (int i = 0; i < 15; i++)
    {
        e.texture = (Texture)(TEXTURE_EVILTALLPLANT1 + rand() % (TEXTURE_EVILTALLPLANT_LAST - TEXTURE_EVILTALLPLANT1));
        e.angle = (rand() % 1000) / 1000.0 * TAU;

        vec2 texture_size = atlas_high[e.texture] - atlas_low[e.texture];
        e.size = { 120, 120 };
        e.size *= texture_size / texture_size.y;

        planet->entities.push_back(e);
    }
}
