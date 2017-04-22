
char* concat(char* left, char* right)
{
    auto len_left = strlen(left);
    auto len_right = strlen(right);
    auto together = (char*) malloc(len_left + len_right + 0);
    memcpy(together, left, len_left);
    memcpy(together + len_left, right, len_right);
    together[len_left + len_right] = 0;
    return together;
}

uint8* read_entire_file(char* path)
{
    auto file = fopen(path, "rb");
    if (!file)
        report(concat("Couldn't open file ", path));
    
    fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto buffer = (uint8*) malloc(size + 1);
    if (fread(buffer, size, 1, file) != 1)
        report(concat("Couldn't read file ", path));
    buffer[size] = 0;

    return buffer;
}
