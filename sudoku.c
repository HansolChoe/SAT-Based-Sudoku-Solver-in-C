//
//  sudoku.c
//  sodoku
//
//  Created by Hansol Choe on 2018. 6. 29..
//  Copyright © 2018년 HansolChoe. All rights reserved.
//
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_OF_ASTERISKS 4
#define OUTPUT_FILE_NAME "formula.txt"

typedef struct coordinate {
    int row;
    int col;
} Coordinate;

int p(int row, int col, int num);

int p_decode(int p);

int print_output();

int make_formula(const char *input);

int main(int argc, const char * argv[]) {
    int err_check = 0;

    if(argc != 2) {
        fprintf(stderr,"usage: sudoku [input_file]");
        return -1;
    }

    err_check = make_formula(argv[1]);
    if (err_check) return -1;

    system("minisat formula.txt sudoku.out > minisat.out");

    err_check = print_output();
    if (err_check) return -1;

    return 0;
}

int p(int row, int col, int num) {
    return row*81 + col*9 + num;
}

int p_decode(int p) {
    int ret = p % 9;
    if ( ret != 0)
        return ret;
    else
        return 9;
}

int make_formula(const char *input) {
    FILE *in = fopen(input, "r");
    FILE *out = fopen(OUTPUT_FILE_NAME, "w");

    int variables = 9*9*9; // 729
    int clauses = 3159;

    int n_asterisk = 0;
    Coordinate asterisks[MAX_NUMBER_OF_ASTERISKS];

    int sudoku_input[9*9];

    // read sudoku data
    if (in != NULL) {
        int c = 0;
        int i = 0;
        while (!feof(in)) {
            c = fgetc(in);
            if(isdigit(c) || c =='.' || c == '*') {
                sudoku_input[i++] = c;
            }
        }
    } else {
        fprintf( stderr, "%s is not opend\n", input);
        return -1;
    }
    fclose(in);

    // count number of cluases
    for (int i = 0 ; i < 9*9 ; i++ ) {
        if(isdigit(sudoku_input[i])) {
            clauses++;
        } else if(sudoku_input[i] == '*') {
            n_asterisk++;
        }
    }

    if (n_asterisk > 1) {
        if (n_asterisk < MAX_NUMBER_OF_ASTERISKS ) {
            clauses += (n_asterisk - 1) * 9;
        } else {
            fprintf(stderr, "number of asterisks should not exeed %d", MAX_NUMBER_OF_ASTERISKS);
        }
    }

    fprintf(out, "p cnf %d %d\n", variables, clauses);
    fprintf(out, "c every row contains every number\n");
    for(int i = 0 ; i < 9 ; i++) {
        for(int n = 1 ; n <= 9 ; n++) {
            for(int j = 0 ; j < 9 ; j++) {
                fprintf(out, "%d ", p(i, j, n));
            }
            fprintf(out, "0\n");
        }
    }

    fprintf(out, "c every column contains every number\n");
    for(int j = 0 ; j < 9; j++) {
        for(int n = 1 ; n <= 9; n++) {
            for(int i = 0 ; i < 9; i++) {
                fprintf(out, "%d ", p(i, j, n));
            }
            fprintf(out, "0\n");
        }
    }

    fprintf(out, "c each of the nine 3 by 3 blocks contains every number\n");
    for(int r = 0 ; r < 3 ; r++) {
        for(int s = 0 ; s < 3 ; s++) {
            for(int n = 1 ; n <= 9 ; n++) {
                for(int i = 0 ; i < 3 ; i++) {
                    for(int j = 0 ; j < 3 ; j++) {
                        fprintf(out, "%d ", p(3*r + i, 3*s + j, n));
                    }
                }
                fprintf(out, "0\n");
            }
        }
    }

    fprintf(out, "c no cell contais more than one number\n");
    for(int i = 0 ; i < 9 ; i++) {
        for(int j = 0 ; j < 9 ; j++) {
            for(int n = 1; n <= 8 ; n++) {
                for (int m = n+1; m <= 9 ; m++) {
                    fprintf(out, "%d %d 0\n", -p(i, j, n), -p(i, j, m));
                }
            }
        }
    }

    fprintf(out, "c add given numbers on the board\n");
    for(int i = 0 ; i < 9; i++) {
        fprintf(out, "c add given numbers on the board %d\n", i);
        for(int j = 0 ; j < 9; j++) {
            int c = sudoku_input[i*9 + j];
            if ( isdigit(c)) {
                int n = c - '0';
                fprintf(out, "%d 0\n", p(i, j, n));
            }
        }
    }

    if( n_asterisk > 1 ) {
        int idx = 0;
        for(int i = 0 ; i < 9 ; i++) {
            for (int j = 0 ; j < 9 ; j++) {
                if( sudoku_input[i*9 + j] == '*') {
                    asterisks[idx].row = i;
                    asterisks[idx].col = j;
                    idx++;
                }
            }
        }

        fprintf(out, "c all asterisks should have the same number\n");
        for(int n = 1; n <= 9 ; n++ ) {
            for(int i = 0; i < n_asterisk - 1 ; i++) {
                fprintf(out, "%d %d 0\n",
                        -p(asterisks[i].row, asterisks[i].col, n),
                        p(asterisks[i+1].row, asterisks[i+1].col, n));
            }
        }
    }
    fclose(out);
    return 0;
}

int print_output() {
    FILE *sudoku = NULL;
    char line[255];
    sudoku = fopen("sudoku.out","r");

    fscanf(sudoku, "%s\n", line);

    if(strcmp(line, "SAT") == 0) {
        int s[9*9] = {0, };
        int idx = 0;
        int proposition = 0;

        // printf("SAT\n");
        while(fscanf(sudoku, "%d ", &proposition) != EOF) {
            if( proposition > 0) {
                s[idx++] = p_decode(proposition);
            }
        }
        if(idx == 81) {
            for(int i = 0 ; i < 9 ; i++) {
                for (int j = 0 ; j < 9 ; j++) {
                    printf("%d", s[i*9 + j]);
                }
                printf("\n");
            }
        } else {
            fprintf(stderr, "sudoku size is not 9*9 but %d\n", idx);
            return -1;
        }

    } else if (strcmp(line, "UNSAT") == 0) {
        printf("UNSATISFIABLE\n");
    } else {
        fprintf(stderr, "Error in reading sudoku.out\n");
        return -1;
    }

    return 0;
}
