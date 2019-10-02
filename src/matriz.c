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
	int i, j, k, l;
	int tam = matriz.n * matriz.n / np;
	int largura_bloco = matriz.n/q;

	// Separa a matriz em matrizes menores
	Matriz *matrizes = malloc(np * sizeof(Matriz));
	float *sub_matriz;

	for (i = 0; i < q; i++){
		for (j = 0; j < q; j++){

			sub_matriz = malloc(tam * sizeof(float));

			for (k = 0; k < largura_bloco; k++){
				for (l = 0; l < largura_bloco; l++){
					MATRIZ_IJ(sub_matriz, largura_bloco, k, l) = MATRIZ_IJ(matriz.dados, matriz.n, i*largura_bloco + k, j*matriz.n/q + l);
				}
			}

			MATRIZ_IJ(matrizes, q, i, j).dados = sub_matriz;
			MATRIZ_IJ(matrizes, q, i, j).n = tam;
		}
	}

	return matrizes;
}
