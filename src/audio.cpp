
Mix_Chunk* load_sound_effect(char* name)
{
    auto path = concat(folder_audio, name);
    defer(free(path));

    auto chunk = Mix_LoadWAV(path);
    if (chunk == NULL)
        report(concat("Failed to load sound effect: ", path));

    return chunk;
}

void play_sound_effect(Mix_Chunk* chunk, vec2 source_position, float volume_multiplier)
{
    static const float SOUND_CONSTANT_DISTANCE = 400;
    static const float SOUND_FALLOFF_DISTANCE  = 500;

    float distance = length(source_position + camera_position);
    float volume;
    if (distance <= SOUND_CONSTANT_DISTANCE)
        volume = MIX_MAX_VOLUME;
    else if (distance <= SOUND_CONSTANT_DISTANCE + SOUND_FALLOFF_DISTANCE)
        volume = (1.0 - (distance - SOUND_CONSTANT_DISTANCE) / SOUND_FALLOFF_DISTANCE) * MIX_MAX_VOLUME;
    else
        return;
    volume *= volume_multiplier;
    int channel = Mix_PlayChannel(-1, chunk, 0);
    Mix_Volume(channel, (int) volume);
}

void init_audio()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0)
        report("SDL_mixer failed to initialize!");

    sound_grass_walk[0] = load_sound_effect("grasswalk1.wav");
    sound_grass_walk[1] = load_sound_effect("grasswalk2.wav");
    sound_grass_walk[2] = load_sound_effect("grasswalk3.wav");
    sound_grass_life[0] = load_sound_effect("grasslife1.wav");
    sound_grass_life[1] = load_sound_effect("grasslife2.wav");
    sound_bounce = load_sound_effect("bounce.wav");
    sound_fireball = load_sound_effect("fireball.wav");
    sound_fireball_out = load_sound_effect("fireballout.wav");
}

void quit_audio()
{
    if (sound_grass_walk[0]) Mix_FreeChunk(sound_grass_walk[0]);
    if (sound_grass_walk[1]) Mix_FreeChunk(sound_grass_walk[1]);
    if (sound_grass_walk[2]) Mix_FreeChunk(sound_grass_walk[2]);
    if (sound_grass_life[0]) Mix_FreeChunk(sound_grass_life[0]);
    if (sound_grass_life[1]) Mix_FreeChunk(sound_grass_life[1]);
    if (sound_bounce) Mix_FreeChunk(sound_bounce);
    if (sound_fireball) Mix_FreeChunk(sound_fireball);
    if (sound_fireball_out) Mix_FreeChunk(sound_fireball_out);
}
