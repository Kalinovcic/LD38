
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

void update_entity(Entity* entity)
{
    const float PLAYER_JUMP_HEIGHT = 250;
    const float PLAYER_SPEED = 500;
    const float GRAVITY = 2 * PLAYER_JUMP_HEIGHT / (0.5 * 0.5);

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
        }

        vec2 target_position = -(entity->planet->position + vec2(sin(camera_rotation), cos(camera_rotation)) * (entity->planet->radius + entity->offset));
        camera_rotation += (entity->angle - camera_rotation) * 0.2f;
        camera_position += (target_position - camera_position) * 0.1f;
    } break;
    case ENTITY_ENEMY:
    {
    } break;
    }
}

void render_entity(Entity* entity)
{
    vec2 bottom = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset);
    draw_rectangle(entity->texture, bottom, entity->size, entity->angle);
}
