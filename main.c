#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <ctype.h>

#include "optfetch.h"
#include "gifenc.h"

#include "table.h"


#define DEFAULT_DIM 20
#define DEFAULT_SPEED 10
#define LOOP_SETTING 0
#define NUM_FRAMES 500

#define GIF_NAME "level.gif"
#define DEFAULT_OUT_HEIGHT 50


#define WIDTH 9
#define HEIGHT 15
char const * const gv_test_level = 
    {
        "100000001\n"
        "100111001\n"
        "100111001\n"
        "100000001\n"
        "100010001\n"
        "100000001\n"
        "110000011\n"
        "111000111\n"
        "111000111\n"
        "111000011\n"
        "100000001\n"
        "111000111\n"
        "100000001\n"
        "100000111\n"
        "100000001\n"
    };


// emit a frame into the given GIF
//   speed is the number of 10 ms increments per frame
//   dim is the dimensions (width and height) of each pixel, to allow larger images
//   width is the width of the given grid of color values
//   height is the height of the given grid of color values
//   grid is a width * height array of indices into the gif's color palette
void emit_frame(ge_GIF *gif, int speed, uint32_t dim, uint32_t width, uint32_t height, uint8_t *grid);

void scroll(uint32_t width, uint32_t height, uint8_t *grid);
char *read_file(char *file_name);
char *parse_level(const char * const level_string, uint32_t *level_width, uint32_t *level_height);
void print_usage(void);

uint8_t *grid_create(char *level_string, uint32_t *level_width, uint32_t *level_height);

int main(int argc, char *argv[]) {
    assert(WIDTH <= 32);

    srand(time(NULL));

    int out_height_int = DEFAULT_OUT_HEIGHT;
    int dim = DEFAULT_DIM;
    char *file_name = NULL;
    bool print_help = false;
    int speed = DEFAULT_SPEED;

    struct opttype opts[] = {
        {"height", 'h', OPTTYPE_INT, &out_height_int},
        {"dim", 'd', OPTTYPE_INT, &dim},
        {"file", 'f', OPTTYPE_STRING, &file_name},
        {"speed", 's', OPTTYPE_INT, &speed},
        {"help", 'h', OPTTYPE_BOOL, &print_help},
    };
    fetchopts(&argc, &argv, opts);
    if (print_help || (argc != 0)) {
        print_usage();
        exit(0);
    }

    // we could use OPTTYPE_ULONG or something for out_height,
    // but lets just not.
    uint32_t out_height = out_height_int;

    uint8_t palette[] = 
    {
        0x00, 0x00, 0x00, /* 0 -> black */
        0x00, 0xFF, 0x00, /* 2 -> green */
        0xFF, 0x00, 0x00, /* 1 -> red */
        0x00, 0x00, 0xFF, /* 3 -> blue */
    };

    char *level = NULL;
    uint32_t level_width = 0;
    uint32_t level_height = 0;

    if (NULL == file_name) {
        level = parse_level(gv_test_level, &level_width, &level_height);
    } else {
        char *level_string = read_file(file_name);
        level = parse_level(level_string, &level_width, &level_height);
        free(level_string);
    }
    assert(NULL != level);

    // GIF color grid passed to gifenc
    uint8_t *grid = calloc(1, level_width * out_height);

    Table *table = table_create(level_width, level_height, level);

    ge_GIF *gif =
        ge_new_gif(GIF_NAME, level_width * dim, out_height * dim, palette, 2, LOOP_SETTING);

    // initialize to a random row
    uint32_t current_row = rand() % table->num_rows;

    // fill the initial grid up with rows
    for (uint32_t row_index = 0; row_index < out_height; row_index++) {
        scroll(level_width, out_height, grid);

        table_copy_row(table, current_row, out_height, grid);
        current_row = table_next_row(table, current_row);
    }

    // start with this filled image
    emit_frame(gif, speed, dim, level_width, out_height, (uint8_t*)grid);

    // run each frame- scroll up one row and fill in the last row with an
    // entry from the table
    for (uint32_t frame_index = 0; frame_index < NUM_FRAMES; frame_index++) {
        scroll(level_width, out_height, grid);
        table_copy_row(table, current_row, out_height, grid);
        current_row = table_next_row(table, current_row);
        emit_frame(gif, speed, dim, level_width, out_height, (uint8_t*)grid);
    }

    // clean up
    table_destroy(&table);

    ge_close_gif(gif);

    free(grid);
    free(level);

    return 0;
}

void print_usage(void) {
    printf("Usage: downgen [OPTION]...\n");
    printf("  --file, -f FILE    Use the given file as the input.\n");
    printf("                     The file should contain 0's and 1's, one column per\n");
    printf("                     line, with the same number of characters in each line\n");
    printf("  --dim,-d  N        Set the GIF dimensions (width and height in pixels of each block.\n");
    printf("                     For example, 5 makes each cell in the output a 5x5 pixel block\n");
    printf("                     Defaults to %d.\n", DEFAULT_DIM);
    printf("  --height,-h N      Set the number of rows in the output image\n");
    printf("                     Defaults to %d\n", DEFAULT_OUT_HEIGHT);
    printf("  --speed,-s N       Set the speed of the gif- 1 means 10ms per frame\n");
    printf("                     Defaults to %d\n", DEFAULT_SPEED);
    printf("  --help             Print this help message\n");
    printf("\n");
}

char *parse_level(const char * const level_string, uint32_t *level_width, uint32_t *level_height) {
    assert(NULL != level_width);
    assert(NULL != level_height);

    uint32_t level_len = strlen(level_string);

    uint32_t num_level_chars = 0;
    *level_width = 0;
    *level_height = 0;
    for (uint32_t chr_index = 0; chr_index < level_len; chr_index++) {
        if (isspace(level_string[chr_index])) {
            if (*level_width == 0) {
                // if the level starts with a space, it is invalid
                assert(0 != chr_index);

                *level_width = chr_index;
            }

            if (level_string[chr_index] == '\n') {
                *level_height = *level_height + 1;
            }
        } else {
            if ((level_string[chr_index] != '0') &&
                (level_string[chr_index] != '1')) {
                fprintf(stderr, "Character '%c' is invalid in a level file!\n", level_string[chr_index]);
                return NULL;
            }
            num_level_chars++;
        }
    }

    // check for no trailing newline- in this case add one to level height
    // for the last line
    if (!isspace(level_string[level_len])) {
        *level_height = *level_height + 1;
    }

    uint8_t *level = (uint8_t*)calloc(1, num_level_chars);
    assert(NULL != level);

    uint32_t level_index = 0;
    for (uint32_t chr_index = 0; chr_index < level_len; chr_index++) {
        if (!isspace(level_string[chr_index])) {
            level[level_index] = level_string[chr_index];
            level_index++;
        }
    }
    
    return level;
}

char *read_file(char *file_name) {
    FILE *file = fopen(file_name, "r");

    if (NULL == file) {
        fprintf(stderr, "Could not open '%s'!\n", file_name);
        exit(0);
    }

    // get file size
    int retval = 0;
    retval = fseek(file, 0, SEEK_END);
    assert(0 == retval);

    int file_size = ftell(file);
    assert(file_size > 0);

    retval = fseek(file, 0, SEEK_SET);
    assert(0 == retval);

    // allocate space for file data
    char *file_data = (char*)calloc(1, file_size + 1);
    assert(NULL != file_data);

    // read in full file
    int read_result = fread(file_data, 1, file_size, file);
    assert(read_result == file_size);

    file_data[file_size - 1] = '\0';

    return file_data;
}

void emit_frame(ge_GIF *gif, int speed, uint32_t dim, uint32_t width, uint32_t height, uint8_t *grid) {
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t index = x + y * width;

            uint32_t y_offset = y * dim;
            uint32_t x_offset = x * dim;

            uint8_t color = grid[index];
            for (uint32_t w = 0; w < dim; w++) {
                for (uint32_t h = 0; h < dim; h++) {

                    uint32_t pixel_index = x_offset + w + (dim * width) * (y_offset + h);

                    gif->frame[pixel_index] = color;
                }
            }
        }
    }

    ge_add_frame(gif, speed);

}

void scroll(uint32_t width, uint32_t height, uint8_t *grid) {
    for (uint32_t y = 0; y < (height - 1); y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t index = x + y * width;

            grid[index] = grid[x + (y + 1) * width];
        }
    }

    for (uint32_t x = 0; x < width; x++) {
        uint32_t index = x + (height - 1) * width;
        grid[index] = 0;
    }
}

