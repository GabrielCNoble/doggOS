#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        return -1;
    }

    char version_str[256];
    int str_cursor = 0;
    FILE *file = fopen(argv[1], "r");
    if(!file)
    {
        perror("couldn't find version file!\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *file_buffer = calloc(file_size, 1);
    fread(file_buffer, 1, file_size, file);
    fclose(file);

    char *version = strstr(file_buffer, "K_VERSION_BUILD");
    int version_cursor = 0;
    if(version)
    {
        
        while(version[version_cursor] != ' ') version_cursor++;
        while(version[version_cursor] == ' ') version_cursor++;
        while(version[version_cursor] >= '0' && version[version_cursor] <= '9')
        {
            version_str[str_cursor] = version[version_cursor];
            str_cursor++;
            version_cursor++;
        }

        version_str[str_cursor] = '\0';
        int current_version = atoi(version_str);
        current_version++;

        file = fopen(argv[1], "w");

        if(file)
        {
            fwrite(file_buffer, version - file_buffer, 1, file);
            fprintf(file, "K_VERSION_BUILD %d", current_version);
            // printf("%d\n", version_cursor);
            fputs(version + version_cursor, file);
            fclose(file);
        }
    }

    return 0;
}