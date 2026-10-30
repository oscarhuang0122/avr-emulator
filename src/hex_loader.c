// hex_loader.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hex_loader.h"

static uint8_t hex_to_byte(const char *hex)
{
    char buf[3];

    buf[0] = hex[0];
    buf[1] = hex[1];
    buf[2] = '\0';

    return (uint8_t)strtol(buf, NULL, 16);
}

int load_hex(const char *filename, Memory *mem)
{
    printf("[HEX] Opening: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("HEX open failed");
        return -1;
    }

    char line[512];

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] != ':')
            continue;

        int byte_count = hex_to_byte(&line[1]);
        int address = (hex_to_byte(&line[3]) << 8) | hex_to_byte(&line[5]);
        int record_type = hex_to_byte(&line[7]);

        // Data record
        if (record_type == 0x00)
        {
            for (int i = 0; i < byte_count; i++)
            {
                uint8_t data = hex_to_byte(&line[9 + i * 2]);
                if (address + i < ROM_SIZE)
                    mem->rom[address + i] = data;
            }
        }
        // End of file
        else if (record_type == 0x01)
        {
            break;
        }
    }

    fclose(file);
    printf("[HEX] Program loaded into ROM\n");
    return 0;
}