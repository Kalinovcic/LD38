
void update_entity(Entity* entity)
{
    switch (entity->brain)
    {
    case ENTITY_PLAYER:
    {
        if (input_left)  entity->angle -= 80 * DEG2RAD / 60.0;
        if (input_right) entity->angle += 80 * DEG2RAD / 60.0;
        camera_rotation += (entity->angle - camera_rotation) * 0.2;
        camera_position = -(entity->planet->position + vec2(sin(camera_rotation), cos(camera_rotation)) * entity->planet->radius);
    } break;
    }
}

void render_entity(Entity* entity)
{
    vec2 bottom = entity->planet->position + vec2(sin(entity->angle), cos(entity->angle)) * (entity->planet->radius + entity->offset);
    draw_rectangle(entity->texture, bottom, entity->size, entity->angle);
}
