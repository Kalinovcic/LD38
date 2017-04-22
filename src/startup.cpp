
SDL_Window* the_window = NULL;
SDL_GLContext the_context = NULL;

void APIENTRY opengl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (type == GL_DEBUG_TYPE_OTHER_ARB) return;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_LOW_ARB:     fprintf(stderr, "[LOW] ");    break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:  fprintf(stderr, "[MEDIUM] "); break;
    case GL_DEBUG_SEVERITY_HIGH_ARB:    fprintf(stderr, "[HIGH] ");   break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR_ARB:               fprintf(stderr, "ERROR: ");               break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: fprintf(stderr, "DEPRECATED_BEHAVIOR: "); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  fprintf(stderr, "UNDEFINED_BEHAVIOR: ");  break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:         fprintf(stderr, "PORTABILITY: ");         break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:         fprintf(stderr, "PERFORMANCE: ");         break;
    case GL_DEBUG_TYPE_OTHER_ARB:               fprintf(stderr, "OTHER: ");               break;
    }

    fprintf(stderr, "id=0x%u %s\n", (unsigned int) id, message);
}

void init_graphics()
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        report("SDL2 failed to initialize!");
    }

    int context_flags = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &context_flags);
    context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    the_window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!the_window)
    {
        report("SDL2 failed to create a window!");
    }

    the_context = SDL_GL_CreateContext(the_window);
    if (!the_context)
    {
        report("Failed to create a OpenGL context!");
    }

    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    glewInit();
    
    if (glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(opengl_callback, NULL);
        GLuint ids;
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &ids, true);
    }

    init_opengl();
}

void init()
{
    init_graphics();
}

void quit_graphics()
{
}

void quit()
{
    quit_graphics();
}

void handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            game_requests_close = true;
        } break;
        }
    }
}

void entry()
{
    init();
    while (!game_requests_close)
    {
        handle_events();

        glClear(GL_COLOR_BUFFER_BIT);

        camera_rotation += 0.1 / 180.0 * PI;
        camera_position = { -(5 + sin(camera_rotation) * 2), -(5 + cos(camera_rotation) * 2) };

        begin_batch();
        draw_planet(vec2(5, 5), 2.0);
        end_batch();

        SDL_GL_SwapWindow(the_window);
    }
    quit();
}
