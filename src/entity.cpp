
void get_phyiscs_angles(Entity* e, float* angle, float* angle_width, float height)
{
    float radius = e->planet->radius + e->offset + e->size.y * height;
    *angle_width = 2 * atan((e->size.x / 2) / radius);
    *angle = e->angle - (*angle_width * 0.5);
    *angle = fmod(fmod(*angle, TAU) + TAU, TAU);
}

bool collision(Entity* e1, Entity* e2)
{
    float a1, w1;
    float a2, w2;
    get_phyiscs_angles(e1, &a1, &w1, 1.0f);
    get_phyiscs_angles(e2, &a2, &w2, 1.0f);
         if ((a1 - a2) > PI) a1 -= TAU;
    else if ((a2 - a1) > PI) a2 -= TAU;

    if (a1 < a2)
    {
        if (a2 > a1 + w1) return false;
    }
    else
    {
        if (a1 > a2 + w2) return false;
    }

    float o1 = e1->offset, h1 = e1->size.y;
    float o2 = e2->offset, h2 = e2->size.y;
    if (o1 < o2)
        return o2 <= o1 + h1;
    else
        return o1 <= o2 + h2;
}

float ground(Entity* walker)
{
    Planet* planet = walker->planet;
    float ea1, ew1;
    get_phyiscs_angles(walker, &ea1, &ew1, 0.0f);

    float ground = 0;
    for (Entity& e : planet->entities)
    {
        float ay = e.angle;
        float ax = ay + PI / 2;
        vec2 bottom = planet->position + vec2(sin(ay), cos(ay)) * (planet->radius + e.offset);
        for (Platform& p : texture_platforms[e.texture])
        {
            float px = e.size.x * (p.x - 0.5);
            float py = e.size.y * p.y;
            float pw = e.size.x * p.width;
            vec2 center = bottom + vec2(sin(ay), cos(ay)) * py + vec2(sin(ax), cos(ax)) * (px + pw * 0.5f);
            float dx = center.x - planet->position.x;
            float dy = center.y - planet->position.y;
            float radius = sqrt(dx * dx + dy * dy);
            float poffset = radius - planet->radius;
            if (poffset > walker->offset) continue;

            float a1 = ea1, w1 = ew1;
            float w2 = 2 * atan((pw / 2) / radius);
            float a2 = atan2(dx, dy) - w2 * 0.5;
            a2 = fmod(fmod(a2, TAU) + TAU, TAU);

            if ((a1 - a2) > PI) a1 -= TAU;
            else if ((a2 - a1) > PI) a2 -= TAU;

            if (a1 < a2)
            {
                if (a2 > a1 + w1) continue;
            }
            else
            {
                if (a1 > a2 + w2) continue;
            }

            ground = max(ground, poffset);
        }
    }
    return ground;
}

void update_entity(Entity* entity, int entity_index)
{
    const float PLAYER_JUMP_HEIGHT = 250;
    const float PLAYER_SPEED = 500;
    const float GRAVITY = 2 * PLAYER_JUMP_HEIGHT / (0.5 * 0.5);

    entity->time_alive += 1 / 60.0;
    switch (entity->brain)
    {
    case ENTITY_PLAYER:
    {
        float legs_radius = (entity->planet->radius + entity->offset) * TAU;
        float move_distance = 0;
        if (input_left)  move_distance -= PLAYER_SPEED / 60.0;
        if (input_right) move_distance += PLAYER_SPEED / 60.0;
        entity->angle += move_distance / legs_radius * TAU;

        float ground_offset = ground(entity);
        if (input_space && entity->offset == ground_offset)
            entity->y_velocity = sqrt(2 * GRAVITY * PLAYER_JUMP_HEIGHT);
        entity->offset += entity->y_velocity / 60.0;
        if (entity->offset <= ground_offset)
        {
            entity->offset = ground_offset;
            entity->y_velocity = 0;
        }
        else
        {
            entity->y_velocity -= GRAVITY / 60.0;
        }

        for (int other_index = 0; other_index < entity->planet->entities.size(); other_index++)
        {
            Entity* other = &entity->planet->entities[other_index];
            if (other->brain == ENTITY_ENEMY)
            {
                if (collision(entity, other))
                {
                    bool kill = (entity->y_velocity < 0) && ((other->offset + other->size.y * 0.5) < entity->offset);
                    if (kill)
                    {
                        entity->y_velocity = -entity->y_velocity * 0.95;
                        entity->planet->remove_list.push_back(other_index);
                    }
                }
            }

            vec2 my_bottom = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset);
            vec2 other_bottom = other->planet->position + vec2(sin(other->angle), cos(other->angle)) * (other->planet->radius + other->offset);
            if (length(my_bottom - other_bottom) < 80)
            {
                bool converted = false;
                if (other->texture >= TEXTURE_EVILPLANT1 && other->texture < TEXTURE_EVILPLANT_LAST)
                {
                    other->texture = (Texture)(TEXTURE_PLANT1 + (other->texture - TEXTURE_EVILPLANT1));
                    converted = true;
                }
                if (other->texture >= TEXTURE_EVILTALLPLANT1 && other->texture < TEXTURE_EVILTALLPLANT_LAST)
                {
                    other->texture = (Texture)(TEXTURE_TALLPLANT1 + (other->texture - TEXTURE_EVILTALLPLANT1));
                    converted = true;
                }

                if (converted)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        Particle p;
                        p.texture = (Texture)(TEXTURE_SPARKLE1 + rand() % (TEXTURE_SPARKLE_LAST - TEXTURE_SPARKLE1));
                        p.position = other_bottom + vec2(rand() % 1000 / 1000.0 * 8.0, rand() % 1000 / 1000.0 * 8.0);
                        p.velocity = normalize(p.position - other->planet->position) * (float)(rand() % 1000 / 1000.0 * 700.0);
                        p.acceleration = { 0, 0 };
                        p.damping = 0.99;
                        p.life = 0.3 + (rand() % 1000 / 1000.0 * 0.3);
                        p.wobble = (float)(rand() % 1000 / 1000.0 * 20.0);
                        p.size = 12.0;
                        other->planet->particles.push_back(p);
                    }
                }
            }
        }

        vec2 target_position = -(entity->planet->position + vec2(sin(camera_rotation), cos(camera_rotation)) * (entity->planet->radius + entity->offset));
        camera_rotation += (entity->angle - camera_rotation) * 0.2f;
        camera_position += (target_position - camera_position) * 0.1f;
    } break;
    case ENTITY_ENEMY:
    {
        float ground_offset = ground(entity);
        entity->offset += entity->y_velocity / 60.0;
        if (entity->offset <= ground_offset)
        {
            entity->offset = ground_offset;
            entity->y_velocity = 0;
        }
        else
        {
            entity->y_velocity -= GRAVITY / 60.0;
        }

        if (rand() % 100 == 0)
        {
            vec2 my_top = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset + entity->size.y);

            Entity e;
            e.planet = entity->planet;
            e.layer = LAYER_ACTORS;
            e.texture = TEXTURE_FIRE;
            e.brain = ENTITY_GRAVITY_BULLET;
            e.offset = entity->offset;
            e.angle = entity->angle;
            e.size = scale_to_height(TEXTURE_FIRE, 20);
            Entity_Kind brain;
            float offset;
            float angle;
            vec2 size;
            float x_velocity;
            float y_velocity;

            for (int i = 0; i < 4; i++)
            {
                float da = (i - 1.5) / 1.5 * (PI / 5);
                vec2 velocity = normalize(vec2(sin(entity->angle + da), cos(entity->angle + da))) * 800.0f;
                e.x_velocity = velocity.x;
                e.y_velocity = velocity.y;
                entity->planet->entities.push_back(e);
            }
        }
    } break;
    case ENTITY_GRAVITY_BULLET:
    {
        vec2 my_bottom = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset);
        vec2 down = normalize(entity->planet->position - my_bottom);

        vec2 velocity = vec2(entity->x_velocity, entity->y_velocity);
        my_bottom += velocity / 60.0f;
        velocity += down * 1400.0f / 60.0f;
        velocity *= 0.98f;
        entity->x_velocity = velocity.x;
        entity->y_velocity = velocity.y;

        float dx = my_bottom.x - entity->planet->position.x;
        float dy = my_bottom.y - entity->planet->position.y;
        entity->offset = length(my_bottom - entity->planet->position) - entity->planet->radius;
        entity->angle = atan2(dx, dy);

        float ground_offset = ground(entity);
        if (entity->offset <= ground_offset)
        {
            entity->planet->remove_list.push_back(entity_index);
            break;
        }
    } break;
    }
}

void render_entity(Entity* entity)
{
    vec2 bottom = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset);
    draw_rectangle(entity->texture, bottom, entity->size, entity->angle);
}
