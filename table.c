#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "table.h"


#define INVALID_ROW 0xFFFFFFFF


void print_row(Table *table, uint32_t row_index);
uint32_t unique_rows(uint32_t width, uint32_t height, char const * const level);
uint32_t bitmap(uint32_t width, char const * const row);


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

void table_print(Table *table) {
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


void table_copy_row(Table *table, uint32_t current_row, Image *image) {
    uint32_t bitmap = table->rows[current_row].bitmap;
    for (uint32_t index = 0; index < table->row_width; index++) {
        uint32_t mask = 1 << (table->row_width - index - 1);
        uint32_t bit = mask & bitmap;

        uint32_t grid_index = table->row_width * (image->height - 1) + index;
        image->data[grid_index] = bit != 0;
    }
}

