#include <stdio.h>
#include <stdlib.h>
#include "matriz.h"

Matriz matriz_criar(const char *arquivo) {
    Matriz matriz;

	FILE *arq;
	arq = fopen(arquivo, "r");

	if (arq == NULL) {
		perror("Erro ao abrir o arquivo!\n");
		exit(1);
	} else {
		fscanf(arq, "%d\n", &matriz.n);

		matriz.dados = malloc(matriz.n * matriz.n * sizeof(float));

		int i, j;
		for (i = 0; i < matriz.n; i++) {
			for (j = 0; j < matriz.n; j++) {
				if (j != (matriz.n -1)) {
					fscanf(arq,"%f ", &MATRIZ_IJ(matriz.dados, matriz.n, i, j));
				} else {
					fscanf(arq,"%f\n", &MATRIZ_IJ(matriz.dados, matriz.n, i, j));
				}
			}
		}
	    fclose(arq);
	}
	return matriz;
}

Matriz *matriz_divide(Matriz matriz, int np, int q) {

}