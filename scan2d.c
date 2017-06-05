#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <sys/mman.h>

// Parameters
#ifndef WIDTH
#define WIDTH 1920
#endif

#ifndef HEIGHT
#define HEIGHT 1080
#endif

static const unsigned int FRAMES = 300;

static unsigned int idx(unsigned int x, unsigned int y, unsigned int stride) {
        return y * stride + x;
}

/*
static void plot(const unsigned int * matrix, unsigned int j){

    printf("y= %u\n", j);
    for(unsigned int y = 0; y < HEIGHT; ++y){
        printf("%u %u %u %u %u\n",
        matrix[idx(0,y,WIDTH)],matrix[idx(1,y,WIDTH)],matrix[idx(2,y,WIDTH)],matrix[idx(3,y,WIDTH)],matrix[idx(4,y,WIDTH)]);
    }
    printf("\n");
}

*/

static void random_fill(unsigned char * matrix, unsigned int seed) {
    unsigned int u = seed;
    unsigned int v = 521;

    for (unsigned int x = 0; x < WIDTH; ++x) {
        for (unsigned int y = 0; y < HEIGHT; ++y) {
            v = 36969*(v & 65535) + (v >> 16);
            u = 18000*(u & 65535) + (u >> 16);
            matrix[idx(x, y, WIDTH)] = (v << 16) + u;
        }
    }
}


static void scan2d(const unsigned char * restrict in, unsigned int * restrict
        out) {
    unsigned int row_sum = 0;

    for (unsigned int x = 0; x < WIDTH; ++x) {
        row_sum += in[idx(x, 0, WIDTH)];
        out[idx(x, 0, WIDTH)] = row_sum;
        //plot(out, 0);
    }

    for (unsigned int y = 1; y < HEIGHT; ++y) {
        row_sum = 0;
        for (unsigned int x = 0; x < WIDTH; ++x) {
            row_sum += in[idx(x, y, WIDTH)];
            out[idx(x, y, WIDTH)] = row_sum + out[idx(x, y-1, WIDTH)];
            //plot(out,y);
        }
    }
}


int main() {

    // HEIGHT = atoi(argv[1]);
    // WIDTH = atoi(argv[2]);

    unsigned char * input  = malloc(WIDTH * HEIGHT * sizeof(unsigned char));
    if(input == NULL){
        printf("Out of memory. Exiting\n");
        exit(-1);
    }

    unsigned int * result = malloc(WIDTH * HEIGHT * sizeof(unsigned int));
    if(result == NULL){
        printf("Out of memory. Exiting\n");
        exit(-1);
    }
    memset(result, 0, WIDTH * HEIGHT * sizeof(unsigned int));

    madvise(result, WIDTH * HEIGHT * sizeof(unsigned int), MADV_HUGEPAGE);
    madvise(input, WIDTH * HEIGHT * sizeof(unsigned int), MADV_HUGEPAGE);

    double startTime = omp_get_wtime();
    for (unsigned int frame = 0; frame < FRAMES; ++frame) {
        random_fill(input, frame);
        scan2d(input, result);
    }
    double endTime = omp_get_wtime();

    fprintf(stderr,"%u\n", result[idx(WIDTH-1, HEIGHT-1, WIDTH)]);
    printf("%d %d %.40lf %.40lf\n", HEIGHT, WIDTH, endTime-startTime,
            (endTime-startTime)/(HEIGHT*WIDTH));
    free(input);
    free(result);

    return 0;
}
