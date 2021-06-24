#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <Windows.h>

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008

const char prog_ver[] = "0.1";

void Help()
{
    printf("BMP to ASCII (BMP2Asc)\n\n");
    printf("Syntax:\n\n");
    printf("bmp2asc <file.bmp> [-v] [-i] [-d] [-o]\n\n");
    printf("Where:\n");
    printf("-v\n   Show program version\n");
    printf("-i\n   Show BMP file information\n");
    printf("-d\n   Print the image on the screen\n");
    printf("-o\n   Print the offset of every line\n");
    printf("-r\n   Show RGB values\n");
    printf("\nExample:\n");
    printf("bmp2asc picture.bmp -o\n\n");
    printf("Made by anic17 with help of Kvc\n");
}

void print_hex(char *bin_str)
{
    for (int i = 0; i < strlen(bin_str); i++)
    {
        printf("%X ", bin_str[i]);
    }
    return;
}

int main(int argc, char *argv[])
{
    int display_info = 0, show_offset = 0, show_rgb = 0;
    if (argc < 2)
    {
        fprintf(stderr, "Missing arguments. See 'bmp2asc --help' to get program help");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
    {
        Help();
        return 0;
    }
    if (!strcmp(argv[1], "--ver") || !strcmp(argv[1], "-v"))
    {
        printf("BMP2Asc version %s\n", prog_ver);
        return 0;
    }
    if (argc > 2)
    {
        int i = argc;
        while (i > 2)
        {

            if (!strcmp(argv[i - 1], "-i"))
            {
                display_info = 1;
            }
            if (!strcmp(argv[i - 1], "-o"))
            {
                show_offset = 1;
            }
            if (!strcmp(argv[i - 1], "-r"))
            {
                show_rgb = 1;
            }
            i--;
        }
    }
    int n = 0, chr = 0;
    char bmp_line[32758];
    unsigned char bmp_header[100];
    unsigned int bmp_file_size = 0, bmp_bpp;
    int offset = 0x12;
    unsigned int bmp_pixel_x, bmp_pixel_y, bmp_compression;
    unsigned char bmp_offset_data[4];
    int r, g, b, monochrome;
    FILE *bmpread = fopen(argv[1], "rb");

    if (!bmpread)
    {
        fprintf(stderr, "%s: %s", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    // Read header: Magic number, resolution
    for (int i = 0; i < 32; i++)
    {
        if ((chr = getc(bmpread)) != EOF)
        {
            bmp_header[n++] = chr;
        }
    }
    DWORD l_mode;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdout, &l_mode);
    SetConsoleMode(hStdout, l_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);

    if (bmp_header[0] != 66 || bmp_header[1] != 'M')
    {
        fprintf(stderr, "Specified file is not a BMP file");
        return EXIT_FAILURE;
    }
    // Offset 0x02h (4 bytes): BMP File size
    bmp_file_size = bmp_header[2] + (0x100 * bmp_header[3]) + (0x10000 * bmp_header[4]) + (0x1000000 * bmp_header[5]);

    // Offset 0x0Ah (4 bytes): Beginning of file to the beginning of the bitmap data
    offset = 0x0A;
    for (int j = 0; j < 4; j++)
    {
        bmp_offset_data[j] = bmp_header[offset++];
    }
    // Offset 0x1Ch (2 bytes): Bits per pixel (BPP)
    offset = 0x1C;
    bmp_bpp = bmp_header[offset] + (0x100 * bmp_header[offset + 1]);

    // Offset 0x12h (4 bytes): X Dimension
    offset = 0x12;

    bmp_pixel_x = bmp_header[offset] + (0x100 * bmp_header[offset + 1]) + (0x10000 * bmp_header[offset + 2]) + (0x1000000 * bmp_header[offset + 3]);

    // Offset 0x16h (4 bytes): Y Dimension
    offset = 0x16;
    bmp_pixel_y = bmp_header[offset] + (0x100 * bmp_header[offset + 1]) + (0x10000 * bmp_header[offset + 2]) + (0x1000000 * bmp_header[offset + 3]);
    fseek(bmpread, bmp_offset_data[0], SEEK_SET);

    // Offset 0x1Eh (4 bytes): Compression
    offset = 0x1e;
    bmp_compression = bmp_header[offset];
    if (bmp_compression == 0x01 || bmp_compression == 0x02)
    {
        fprintf(stderr, "Error: Compressed BMP images are not supported.");
        return EXIT_FAILURE;
    }
    if (bmp_bpp == 16)
    {
        fprintf(stderr, "16-bit palette images are not supported");
    }
    fseek(bmpread, 0, SEEK_END);
    if (bmp_bpp == 24)
    {
        bmp_pixel_x++;
    }
    for (int i = bmp_pixel_y; i > 0; i--)
    {
        // Sets cursor to bottom

        fseek(bmpread, (-(bmp_bpp / 8) * (bmp_pixel_x)), SEEK_CUR);

        for (int j = 0; j < bmp_pixel_x; j++)
        {
            b = getc(bmpread);
            g = getc(bmpread);
            r = getc(bmpread);

            if (bmp_bpp == 32)
            {
                n = getc(bmpread);
            }

            if (show_rgb)
            {
                printf("\t%d, %d; %d, %d, %d\n", j, i, r, g, b);
            }
            else
            {
                printf("\e[48;2;%d;%d;%dm ", r, g, b);
            }
        }
        if (!show_rgb)
        {
            printf("\e[0m\n");
        }
        fseek(bmpread, (-(bmp_bpp / 8) * (bmp_pixel_x)), SEEK_CUR);

        if (show_offset == 1)
        {
            fprintf(stderr, "0x%xh\n", ftell(bmpread));
        }
    }

    /*
        chr = getc(bmpread);
        printf("%d", chr);
    */
    if (bmp_bpp == 24)
    {
        bmp_pixel_x--;
    }
    if (display_info)
    {
        printf("\nFile information:\n");
        printf("Resolution %d x %d\n", bmp_pixel_x, bmp_pixel_y);
        printf("Offset: 0x%x\n", bmp_offset_data[0]);
        printf("File size: %d bytes\n", bmp_file_size);
        printf("Bits per pixel (BPP): %d", bmp_bpp);
    }
}
