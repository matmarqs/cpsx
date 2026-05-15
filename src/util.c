#include "util.h"
#include "stdio.h"

bool read_file(const char *filename, void *buffer, size_t buffer_size)
{
    if (!filename || !buffer || buffer_size == 0) return false;

    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;

    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return false;
    }

    long file_size = ftell(fp);
    rewind(fp);

    if (file_size < 0 || (size_t)file_size > buffer_size) {
        fclose(fp);
        return false;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)file_size, fp);

    fclose(fp);
    return bytes_read == (size_t)file_size;
}
