#ifndef ARRAY_SUM_H
#define ARRAY_SUM_H

struct SumArgs {
    int *array;
    int begin;
    int end;
};

int Sum(const struct SumArgs *args);

#endif
