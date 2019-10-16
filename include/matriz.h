/**
 * TP1-comp-paralela-UFSJ
 *
 * matriz.c
 *
 * Júlio Resende - julio.cmdr@gmail.com
 * Paulo Tobias - paulohtobias@outlook.com
 *
 */

#ifndef MATRIZ_H
#define MATRIZ_H

#include <stdint.h>

#define MATRIZ_IJ(matriz, n, i, j) (matriz[(i) * (n) + (j)])
#define matriz_liberar(matriz) free(matriz.dados)

typedef struct Matriz {
	int32_t n;
	float *dados;
} Matriz;

Matriz matriz_criar(const char *arquivo);

Matriz *matriz_divide(Matriz matriz, int np, int q);

void matriz_acumular(Matriz m1, Matriz m2, Matriz *r);

void matriz_print(Matriz m);

#endif // MATRIZ_H
