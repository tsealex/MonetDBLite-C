#include "embedded.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define error(msg) {fprintf(stderr, "Failure: %s\n", msg); return -1;}

void* editRow(void* var) {

    char* err;
    void* conn;

    conn = monetdb_connect();
    int i;
    for (i = 0; i < 100; i++) {
        err = monetdb_query(conn, "UPDATE test SET category = 'newval';",
                            1, NULL, NULL, NULL);
    }
    monetdb_disconnect(conn);


    if (err != 0) {
        return conn;
    }

    return conn;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("usage: ./dustins <numthreads>\n");
        return 0;
    }

    int numthreads = atoi(argv[1]);

    char* err = 0;
    void* conn = 0;
    monetdb_result* result = 0;
    size_t r, c;

    // first argument is a string for the db directory or NULL for in-memory mode
    // we should try performance tests with both!
    err = monetdb_startup(NULL, 0, 0);
    if (err != 0) {
        error(err)
    }

    conn = monetdb_connect();
    if (conn == NULL) {
        error("Connection failed")
    }

    err = monetdb_query(conn, "CREATE TABLE test (app string, category string, rating string, reviews string,"
                              "size string, installs string, type string, price string, contentR string,"
                              "genres string, lastU string, curV string, andV string)", 1, NULL, NULL, NULL);
    if (err != 0) {
        error(err)
    }

    err = monetdb_query(conn, "COPY 10000 RECORDS INTO test FROM '/home/alex/Projects/CS736/MonetDBLite-C/googleplaystore.csv'"
                              "USING DELIMITERS ',','\\n','\"' NULL AS '';", 1, NULL, NULL, NULL);
    if (err != 0) {
        error(err)
    }

    //
    // dustin's multi-threading code
    //

    pthread_t id[numthreads];
    int i, rc;
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    for (i = 0; i < numthreads; i++) {
        rc = pthread_create(&id[i], NULL, &editRow, conn);
        if (rc != 0) {
            error("Thread creation failed.");
        }
    }

    for (i = 0; i < numthreads; i++) {
        pthread_join(id[i], NULL);
    }

    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Total time used: %fs\n", cpu_time_used);

    //
    // print some result to prove the row was updated
    //

    err = monetdb_query(conn, "SELECT * FROM test WHERE app = 'Coloring book moana';", 1, &result, NULL, NULL);
    if (err != 0) {
        error(err)
    }

    fprintf(stdout, "Query result with %d cols and %d rows\n", (int) result->ncols, (int) result->nrows);

    for (r = 0; r < result->nrows; r++) {
        for (c = 0; c < result->ncols; c++) {
            monetdb_column* rcol = monetdb_result_fetch(result, c);
            switch (rcol->type) {
                case monetdb_int32_t: {
                    monetdb_column_int32_t * col = (monetdb_column_int32_t *) rcol;
                    if (col->data[r] == col->null_value) {
                        printf("NULL");
                    } else {
                        printf("%d", (int) col->data[r]);
                    }
                    break;
                }
                case monetdb_str: {
                    monetdb_column_str * col = (monetdb_column_str *) rcol;
                    if (col->is_null(col->data[r])) {
                        printf("NULL");
                    } else {
                        printf("%s", (char*) col->data[r]);
                    }
                    break;
                }
                default: {
                    printf("UNKNOWN");
                }
            }
            if (c + 1 < result->ncols) {
                printf(", ");
            }
        }
        printf("\n");
    }

    monetdb_cleanup_result(conn, result);
    monetdb_disconnect(conn);
    monetdb_shutdown();

    return 0;
}
