/**
 * Wii U Mii Data Checksum Fixer by GaryOderNichts
 * Based on: http://wiibrew.org/wiki/Mii_Data
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define DATA_SIZE 276544

static uint16_t getCRC(uint8_t* bytes, int length) 
{
    uint16_t crc = 0x0000;
    for (int byteIndex = 0; byteIndex < length; byteIndex++) 
    {
        for (int bitIndex = 7; bitIndex >= 0; bitIndex--) 
        {
            crc = (((crc << 1) | ((bytes[byteIndex] >> bitIndex) & 0x1)) ^ (((crc & 0x8000) != 0) ? 0x1021 : 0)); 
        }
    }
    for (int counter = 16; counter > 0; counter--) 
    {
        crc = ((crc << 1) ^ (((crc & 0x8000) != 0) ? 0x1021 : 0));
    }

    return (crc & 0xFFFF);
}

uint16_t bswap16(uint16_t value)
{
	return (uint16_t) ((0x00FF & (value >> 8)) | (0xFF00 & (value << 8)));
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Wii U Mii Data Checksum Fixer by GaryOderNichts\nUsage:\n    fixer <path to file>\n");
        return 0;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Cannot open input\n");
        return -1;
    } 

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    if (size != DATA_SIZE)
    {
        printf("Wrong size\n");
        fclose(f);
        return -1;
    }

    uint8_t* data = malloc(DATA_SIZE - 2);
    fread(data, 1, DATA_SIZE - 2, f);

    uint16_t current = 0;
    fseek(f, DATA_SIZE - 2, SEEK_SET);
    fread(&current, 1, 2, f);
    fclose(f);

    uint16_t new = bswap16(getCRC(data, DATA_SIZE - 2));

    printf("Current: %04X New: %04X\n", bswap16(current), bswap16(new));

    if (current == new)
    {
        printf("No need to update\n");
        free(data);
        return 0;
    }

    f = fopen(argv[1], "wb");
    if (!f)
    {
        printf("Cannot open output\n");
        free(data);
        return -1;
    } 

    fwrite(data, 1, DATA_SIZE - 2, f);
    fwrite(&new, 1, 2, f);
    fclose(f);

    printf("Checksum updated\n");
    return 0;
}