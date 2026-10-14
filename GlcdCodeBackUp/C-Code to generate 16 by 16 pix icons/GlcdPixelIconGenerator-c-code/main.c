#include <stdio.h>
#include <stdint.h>

#define WIDTH 16
#define HEIGHT 16
#define HALF 8

#pragma pack(push, 1)

typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;

#pragma pack(pop)

/*
 * Read a 16x16, 24-bit BMP file.
 *
 * Black pixels = 1
 * White pixels = 0
 */
int read_bmp(const char *filename,
             unsigned char pixels[HEIGHT][WIDTH])
{
    FILE *file;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error opening %s\n", filename);
        return 0;
    }

    /* Read BMP file header */
    if (fread(&fileHeader,
              sizeof(fileHeader),
              1,
              file) != 1)
    {
        printf("Error reading BMP file header\n");
        fclose(file);
        return 0;
    }

    /* Read BMP information header */
    if (fread(&infoHeader,
              sizeof(infoHeader),
              1,
              file) != 1)
    {
        printf("Error reading BMP information header\n");
        fclose(file);
        return 0;
    }

    /* Check BMP signature */
    if (fileHeader.bfType != 0x4D42)
    {
        printf("Error: Not a BMP file\n");
        fclose(file);
        return 0;
    }

    /* Check image size */
    if (infoHeader.biWidth != WIDTH ||
        infoHeader.biHeight != HEIGHT)
    {
        printf("Error: BMP must be exactly 16x16 pixels\n");
        fclose(file);
        return 0;
    }

    /* Check color depth */
    if (infoHeader.biBitCount != 24)
    {
        printf("Error: BMP must be 24-bit\n");
        fclose(file);
        return 0;
    }

    /* Check compression */
    if (infoHeader.biCompression != 0)
    {
        printf("Error: Compressed BMP is not supported\n");
        fclose(file);
        return 0;
    }

    /*
     * Move to the beginning of the pixel data.
     */
    fseek(file, fileHeader.bfOffBits, SEEK_SET);

    /*
     * BMP rows are padded to multiples of 4 bytes.
     *
     * 16 pixels x 3 bytes = 48 bytes.
     */
    int rowSize = ((WIDTH * 3 + 3) / 4) * 4;

    unsigned char row[WIDTH * 3 + 3];

    /*
     * BMP normally stores the image upside down:
     * first row in the file = bottom row.
     */
    for (int bmpRow = 0; bmpRow < HEIGHT; bmpRow++)
    {
        if (fread(row, 1, rowSize, file) != rowSize)
        {
            printf("Error reading BMP pixel data\n");
            fclose(file);
            return 0;
        }

        /*
         * Convert BMP row number to normal image row.
         */
        int imageRow = HEIGHT - 1 - bmpRow;

        for (int col = 0; col < WIDTH; col++)
        {
            /*
             * 24-bit BMP stores pixels as:
             *
             * B G R
             */
            unsigned char blue =
                row[col * 3 + 0];

            unsigned char green =
                row[col * 3 + 1];

            unsigned char red =
                row[col * 3 + 2];

            /*
             * Calculate brightness.
             */
            int brightness =
                (red + green + blue) / 3;

            /*
             * Black = 1
             * White = 0
             */
            if (brightness < 128)
                pixels[imageRow][col] = 1;
            else
                pixels[imageRow][col] = 0;
        }
    }

    fclose(file);

    return 1;
}

/*
 * Rotate an 8x16 image clockwise.
 *
 * Input:
 *     8 rows x 16 columns
 *
 * Output:
 *     16 rows x 8 columns
 */
void rotate_cw(
    const unsigned char input[HALF][WIDTH],
    unsigned char output[WIDTH][HALF])
{
    for (int col = 0; col < WIDTH; col++)
    {
        for (int row = 0; row < HALF; row++)
        {
            output[col][row] =
                input[HALF - 1 - row][col];
        }
    }
}

/*
 * Convert 8 bits into one hexadecimal byte.
 */
unsigned int row_to_hex(
    const unsigned char row[HALF])
{
    unsigned int value = 0;

    for (int i = 0; i < HALF; i++)
    {
        value <<= 1;

        if (row[i])
        {
            value |= 1;
        }
    }

    return value;
}

int main(void)
{
    /*
     * Complete 16x16 image.
     */
    unsigned char pixels[HEIGHT][WIDTH];

    /*
     * Upper and lower 8x16 halves.
     */
    unsigned char upper[HALF][WIDTH];
    unsigned char lower[HALF][WIDTH];

    /*
     * Rotated output:
     * 16 rows x 8 bits.
     */
    unsigned char output[WIDTH][HALF];

    FILE *file;

    /*
     * =========================
     * READ rightArrow.bmp
     * =========================
     */

    if (!read_bmp("img.bmp", pixels))
    {
        return 1;
    }

    printf(".bmp file loaded successfully.\n\n");

    /*
     * =========================
     * SPLIT IMAGE
     * =========================
     *
     * Rows 0-7  = upper half
     * Rows 8-15 = lower half
     */

    for (int row = 0; row < HALF; row++)
    {
        for (int col = 0; col < WIDTH; col++)
        {
            upper[row][col] =
                pixels[row][col];

            lower[row][col] =
                pixels[row + HALF][col];
        }
    }

    /*
     * =========================
     * CREATE output.txt
     * =========================
     */

    file = fopen("output.txt", "w");

    if (file == NULL)
    {
        printf("Error opening output.txt\n");
        return 1;
    }

    /*
     * =========================
     * TOP 8
     * =========================
     */

    rotate_cw(upper, output);

    fprintf(file, "/* Top 8 */\n");

    for (int i = 0; i < WIDTH; i++)
    {
        unsigned int value = row_to_hex(output[i]);

        if (i % 4 == 0)
        {
            fprintf(file, "\t  ");
        }

        fprintf(file, "0x%02X", value);

        /*
         * Comma after EVERY value,
         * including the last Top 8 value.
         */
        fprintf(file, ",");

        if ((i + 1) % 4 == 0)
        {
            fprintf(file, "\n");
        }
    }

    /*
     * =========================
     * BOTTOM 8
     * =========================
     */

    fprintf(file, "\n\t  /* Bottom 8 */\n");

    rotate_cw(lower, output);

    for (int i = 0; i < WIDTH; i++)
    {
        unsigned int value = row_to_hex(output[i]);

        if (i % 4 == 0)
        {
            fprintf(file, "\t  ");
        }

        fprintf(file, "0x%02X", value);

        /*
         * Comma after every value except
         * the final Bottom 8 value.
         */
        if (i < WIDTH - 1)
        {
            fprintf(file, ",");
        }

        if ((i + 1) % 4 == 0)
        {
            fprintf(file, "\n");
        }
    }

    /*
     * =========================
     * FINISH
     * =========================
     */

    fclose(file);

    printf("Done! Output saved to output.txt\n");

    return 0;
}
