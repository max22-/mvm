#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

ssize_t assemble(const char *file_name, const char *source, uint8_t *rom,
                 size_t rom_capacity);