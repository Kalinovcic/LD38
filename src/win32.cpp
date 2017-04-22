
char* get_run_tree_path()
{
    auto buffer_size = 1024 * 8;
    auto buffer = (char*) malloc(buffer_size);

    GetModuleFileNameA(0, buffer, buffer_size);

    auto length = (int) strlen(buffer);
    while (length && buffer[length - 1] != '\\' && buffer[length - 1] != '/')
    {
        buffer[length - 1] = 0;
        length--;
    }

    return buffer;
}

void report(const char* message)
{
    MessageBox(NULL, message, "Error", MB_ICONERROR);
    quit();
    exit(EXIT_FAILURE);
}
