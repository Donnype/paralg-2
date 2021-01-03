#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "bfs.h"
#include "parallel_bfs.h"
#include "dependency.h"
#include "Args.h"


extern long MAX_NR_VERTICES_PER_P;
extern short **adjacency_matrix;
extern Args* args;

double diff(struct timespec start, struct timespec end) {
    long seconds = end.tv_sec - start.tv_sec;
    long nano_seconds = end.tv_nsec - start.tv_nsec;

    return (double) 1000.0 * seconds + (double) nano_seconds / 1000000.0;
}


double time_bfs_linked(short **matrix) {
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    long *d = bfs_linked(matrix, 0);

    clock_gettime(CLOCK_REALTIME, &end);

    free(d);

    return diff(start, end);
}


double time_bfs_vec(short **matrix) {
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    long *d = bfs_vec(matrix, 0);

    clock_gettime(CLOCK_REALTIME, &end);

    if (args->output) {
        for (int i = 0; i < args->nr_vertices; ++i) {
            printf("%ld ", d[i]);
        }

        printf("\n");
    }

    free(d);

    return diff(start, end);
}


double time_bfs_parallel(int argc, char **argv) {
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    parallel_wrap(argc, argv);

    clock_gettime(CLOCK_REALTIME, &end);

    return diff(start, end);
}


double time_betweenness_parallel(int argc, char **argv) {
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);

    parallel_betweenness_wrap(argc, argv);

    clock_gettime(CLOCK_REALTIME, &end);

    return diff(start, end);
}


int seq_bfs() {
    double ms = 0.0, runs = 5.0;

    printf("BFS sequential: \n\n");
    printf("ms\n");

    for (int i = 0; i < runs; ++i) {
        ms += time_bfs_vec(adjacency_matrix);
    }

    ms = ms / runs;
    printf("%f\n", ms);

    free_matrix(&adjacency_matrix, args->nr_vertices);

    return 0;
}


int bfs(int argc, char **argv) {
    double ms = 0.0, runs = 5.0;

    printf("BFS parallel: \n\n");
    printf("p\tms\n");

    for (args->nr_processors = 1; args->nr_processors < 9; ++args->nr_processors) {
        MAX_NR_VERTICES_PER_P = args->nr_vertices / args->nr_processors;

        for (int i = 0; i < runs; ++i) {
            ms += time_bfs_parallel(argc, argv);
        }

        ms = ms / runs;
        printf("%ld\t%f\n", args->nr_processors, ms);
        ms = 0.0;
    }


    free_matrix(&adjacency_matrix, args->nr_vertices);

    return 0;
}


int betweenness(int argc, char **argv) {
    double ms = 0.0, runs = 5.0;
    printf("Betweenness parallel: \n\n");
    printf("p\tms\n");

    for (args->nr_processors = 1; args->nr_processors < 9; args->nr_processors++) {
        MAX_NR_VERTICES_PER_P = args->nr_vertices / args->nr_processors;

        for (int i = 0; i < runs; ++i) {
            ms += time_betweenness_parallel(argc, argv);
        }

        ms = ms / runs;
        printf("%ld\t%f\n", args->nr_processors, ms);
        ms = 0.0;
    }


    free_matrix(&adjacency_matrix, args->nr_vertices);

    return 0;
}


int main(int argc, char **argv) {
    read_args(argc, argv);

    if (args->test == 1) {
        args->nr_vertices = 10;

        short graph[10][10] = {
                {0, 0, 1, 1, 1, 0, 1, 0, 1, 0},
                {0, 1, 1, 0, 1, 0, 0, 0, 0, 1},
                {1, 1, 1, 1, 0, 1, 1, 0, 1, 0},
                {1, 0, 1, 1, 1, 0, 0, 1, 0, 0},
                {1, 1, 0, 1, 1, 1, 1, 0, 0, 1},
                {0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
                {1, 0, 1, 0, 1, 0, 0, 1, 1, 1},
                {0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
                {1, 0, 1, 0, 0, 0, 1, 0, 1, 1},
                {0, 1, 0, 0, 1, 1, 1, 0, 1, 0},
        };

        adjacency_matrix = fill_buffer(graph);
    } else {
        adjacency_matrix = generate_symmetric_matrix();
    }

    if (args->print_matrix == 1) {
        print_matrix(adjacency_matrix);
    }

//    return bfs(argc, argv);
//    return seq_bfs();
    return betweenness(argc, argv);
}
