#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "optfetch.h"
#include "gifenc.h"


#define DIM 10
#define SPEED 10
#define LOOP_SETTING 0
#define INVALID_ROW 0xFFFFFFFF
#define NUM_FRAMES 500
#define GIF_NAME "level.gif"
#define OUT_HEIGHT 50


typedef struct {
    uint32_t bitmap;
    uint32_t total_transitions;
} Row;

typedef struct {
    uint32_t row_width;
    uint32_t num_rows;
    Row *rows;
    uint32_t *transitions;
} Table;


#define WIDTH 9
#define HEIGHT 15
char const * const gv_test_level = 
    {
        "100000001"
        "100111001"
        "100111001"
        "100000001"
        "100010001"
        "100000001"
        "110000011"
        "111000111"
        "111000111"
        "111000011"
        "100000001"
        "111000111"
        "100000001"
        "100000111"
        "100000001"
    };


void emit_frame(ge_GIF *gif, uint32_t width, uint32_t height, uint8_t *grid);

void scroll(uint32_t width, uint32_t height, uint8_t *grid);
uint32_t unique_rows(uint32_t width, uint32_t height, char const * const level);
uint32_t bitmap(uint32_t width, char const * const row);
void print_row(Table *table, uint32_t row_index);
void print_table(Table *table);
void copy_row(Table *table, uint32_t current_row, uint32_t height, uint8_t *grid);

Table *table_create(uint32_t width, uint32_t height, char const * const level);
void table_destroy(Table **table);
uint32_t table_bitmap_index(Table *table, uint32_t bitmap);
uint32_t table_next_row(Table *table, uint32_t current_row);

int main(int argc, char *argv[]) {
    assert(WIDTH <= 32);

    srand(time(NULL));

    int out_height_int = OUT_HEIGHT;

    struct opttype opts[] = {
        {"height", 'h', OPTTYPE_INT, &out_height_int},
    };
    fetchopts(&argc, &argv, opts);

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

    ge_GIF *gif =
        ge_new_gif(GIF_NAME, WIDTH * DIM, out_height * DIM, palette, 2, LOOP_SETTING);

    uint8_t grid[WIDTH * out_height];

    for (uint32_t y = 0; y < out_height; y++) {
        for (uint32_t x = 0; x < WIDTH; x++) {
            uint32_t index = x + y * WIDTH;

            grid[index] = gv_test_level[index] == '1';
        }
    }

    Table *table = table_create(WIDTH, HEIGHT, gv_test_level);

    //print_table(table);

    // initialize to a random row
    uint32_t current_row = rand() % table->num_rows;

    // fill the initial grid up with rows
    for (uint32_t row_index = 0; row_index < out_height; row_index++) {
        scroll(WIDTH, out_height, grid);

        copy_row(table, current_row, out_height, grid);
        current_row = table_next_row(table, current_row);
    }

    // start with this fill image
    emit_frame(gif, WIDTH, out_height, (uint8_t*)grid);

    // run each frame- scroll up one row and fill in the last row with an
    // entry from the table
    for (uint32_t frame_index = 0; frame_index < NUM_FRAMES; frame_index++) {
        scroll(WIDTH, out_height, grid);
        copy_row(table, current_row, out_height, grid);
        current_row = table_next_row(table, current_row);
        emit_frame(gif, WIDTH, out_height, (uint8_t*)grid);
    }

    // clean up
    table_destroy(&table);

    ge_close_gif(gif);

    return 0;
}

void emit_frame(ge_GIF *gif, uint32_t width, uint32_t height, uint8_t *grid) {
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t index = x + y * width;

            uint32_t y_offset = y * DIM;
            uint32_t x_offset = x * DIM;

            uint8_t color = grid[index];
            for (uint32_t w = 0; w < DIM; w++) {
                for (uint32_t h = 0; h < DIM; h++) {

                    uint32_t pixel_index = x_offset + w + (DIM * width) * (y_offset + h);

                    gif->frame[pixel_index] = color;
                }
            }
        }
    }

    ge_add_frame(gif, SPEED);

}

void copy_row(Table *table, uint32_t current_row, uint32_t height, uint8_t *grid) {
    uint32_t bitmap = table->rows[current_row].bitmap;
    for (uint32_t index = 0; index < table->row_width; index++) {
        uint32_t mask = 1 << (table->row_width - index - 1);
        uint32_t bit = mask & bitmap;

        uint32_t grid_index = table->row_width * (height - 1) + index;
        grid[grid_index] = bit != 0;
    }
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

uint32_t bitmap(uint32_t width, char const * const row) {
    uint32_t map = 0;

    for (uint32_t index = 0; index < width; index++) {
        map = map << 1;
        map |= row[index] == '1';
    }

    return map;
}

uint32_t unique_rows(uint32_t width, uint32_t height, char const * const level) {
    uint32_t reprs[height];
    memset(reprs, 0, sizeof(height * sizeof(uint32_t)));

    uint32_t num_reprs = 0;

    for (uint32_t row_index = 0; row_index < height; row_index++) {
        uint32_t map = bitmap(width, &level[row_index * width]);

        bool new_repr = true;
        for (uint32_t repr_index = 0; repr_index < num_reprs; repr_index++) {
            if (reprs[repr_index] == map) {
                new_repr = false;
                break;
            }
        }

        if (new_repr) {
            reprs[num_reprs] = map;
            num_reprs++;
        }
    }

    return num_reprs;
}

uint32_t table_bitmap_index(Table *table, uint32_t bitmap) {
    for (uint32_t index = 0; index < table->num_rows; index++) {
        if (table->rows[index].bitmap == bitmap) {
            return index;
        }
    }

    return INVALID_ROW;
}

Table *table_create(uint32_t width, uint32_t height, char const * const level) {
    Table *table = (Table*)calloc(1, sizeof(Table));

    table->row_width = width;

    uint32_t num_unique_rows = unique_rows(width, height, level);
    assert(num_unique_rows > 0);

    table->num_rows = num_unique_rows;

    table->rows = (Row*)calloc(table->num_rows, sizeof(Row));

    uint32_t num_reprs = 0;
    for (uint32_t row_index = 0; row_index < height; row_index++) {
        uint32_t map = bitmap(width, &level[row_index * width]);

        bool new_repr = true;
        for (uint32_t repr_index = 0; repr_index < num_reprs; repr_index++) {
            if (table->rows[repr_index].bitmap == map) {
                new_repr = false;
                break;
            }
        }

        if (new_repr) {
            table->rows[num_reprs].bitmap = map;
            num_reprs++;
        }
    }

    assert(num_reprs == num_unique_rows);

    table->transitions = (uint32_t*)calloc(table->num_rows, table->num_rows * sizeof(uint32_t)); 

    for (uint32_t row_index = 0; row_index < height; row_index++) {
        uint32_t start_bitmap = bitmap(width, &level[row_index * width]);

        uint32_t next_row_index = (row_index + 1) % table->num_rows;
        uint32_t prev_row_index = row_index - 1;
        if (row_index == 0) {
            prev_row_index = table->num_rows - 1;
        }
        uint32_t next_bitmap = bitmap(width, &level[next_row_index * width]);
        uint32_t prev_bitmap = bitmap(width, &level[prev_row_index * width]);

        uint32_t start_index = table_bitmap_index(table, start_bitmap);
        assert(INVALID_ROW != start_index);

        uint32_t next_index = table_bitmap_index(table, next_bitmap);
        assert(INVALID_ROW != next_index);

        uint32_t prev_index = table_bitmap_index(table, prev_bitmap);
        assert(INVALID_ROW != prev_index);

        uint32_t transition_index = 0;

        transition_index = start_index * table->num_rows + next_index;
        table->transitions[transition_index]++;

        transition_index = start_index * table->num_rows + prev_index;
        table->transitions[transition_index]++;

        table->rows[start_index].total_transitions += 2;
    }
    
    return table;
}

void print_table(Table *table) {
    printf("Unique Rows:\n");
    for (uint32_t row_index = 0; row_index < table->num_rows; row_index++) {
        print_row(table, row_index);
    }

    printf("\nTransition Table:\n");
    for (uint32_t row_index = 0; row_index < table->num_rows; row_index++) {
        for (uint32_t trans_index = 0; trans_index < table->num_rows; trans_index++) {
            uint32_t index = row_index * table->num_rows + trans_index;
            printf("%d ", table->transitions[index]);
        }
        printf(" = %d\n", table->rows[row_index].total_transitions);
    }
    printf("\n");
}

void print_row(Table *table, uint32_t row_index) {
    for (uint32_t index = 0; index < table->row_width; index++) {
        uint32_t mask = 1 << (table->row_width - index - 1);
        uint32_t bit = mask & table->rows[row_index].bitmap;
        printf("%c ", '0' + (bit != 0));
    }
    printf("\n");
}

uint32_t table_next_row(Table *table, uint32_t current_row) {
    uint32_t count = rand() % (table->rows[current_row].total_transitions + 1);
    uint32_t last_counted_row = INVALID_ROW;

    for (uint32_t trans_index = 0; trans_index < table->num_rows; trans_index++) {
        uint32_t count_index = current_row * table->num_rows + trans_index;

        if (table->transitions[count_index] != 0) {
            if (count < table->transitions[count_index]) {
                return trans_index;
            }

            count -= table->transitions[count_index];
            last_counted_row = trans_index;
        }
    }

    return last_counted_row;
}

void table_destroy(Table **table) {
    free((*table)->rows);
    free((*table)->transitions);
    free(*table);
    *table = NULL;
}

