
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

void init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        report("SDL2 failed to initialize!");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    the_window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (!the_window)
    {
        report("SDL2 failed to create a window!");
    }
    SDL_SetWindowFullscreen(the_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowWindow(the_window);

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

    init_audio();
}

void quit()
{
    Mix_Quit();
    SDL_Quit();
}

void handle_events()
{
    SDL_GetWindowSize(the_window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            game_requests_close = true;
        } break;
        case SDL_KEYDOWN: case SDL_KEYUP:
        {
            bool down = event.type == SDL_KEYDOWN;
            int scancode = event.key.keysym.scancode;
            if (scancode == SDL_SCANCODE_LEFT)  input_left  = down;
            if (scancode == SDL_SCANCODE_RIGHT) input_right = down;
            if (scancode == SDL_SCANCODE_SPACE) input_space = down;
            if (down && scancode == SDL_SCANCODE_F5)
            {
                auto flags = SDL_GetWindowFlags(the_window);
                if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                    SDL_SetWindowFullscreen(the_window, 0);
                else
                    SDL_SetWindowFullscreen(the_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
        } break;
        }
    }
}

void entry()
{
    atexit(quit);
    init();

    Planet planet;
    load_level(&planet, 1);
    camera_position = -planet.position;

    while (!game_requests_close)
    {
        handle_events();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        update_and_render_level(&planet);
        
        SDL_GL_SwapWindow(the_window);
    }
}
