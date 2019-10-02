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
    int tam = tam = matriz.n * matriz.n / np;

    // Separa a matriz em matrizes menores
    Matriz *matrizes = malloc(sizeof(Matriz)*np);
    float *sub_matriz;

    for (i = 0; i < q; i++){
        for (j = 0; j < q; j++){

            sub_matriz = malloc(sizeof(float)*tam);

            printf("ij: %d, %d\n", i, j);

            for (k = 0; k < matriz.n/q; k++){
                for (l = 0; l < matriz.n/q; l++){
                    MATRIZ_IJ(sub_matriz, tam, k, l) = MATRIZ_IJ(matriz.dados, matriz.n, i*matriz.n/q + k, j*matriz.n/q + l);
                }
            }

            MATRIZ_IJ(matrizes, np, i, j).dados = sub_matriz;
            MATRIZ_IJ(matrizes, np, i, j).n = tam;

            for (k = 0; k < matriz.n/q; k++){
                for (l = 0; l < matriz.n/q; l++){
                    printf("%.1f ", MATRIZ_IJ(MATRIZ_IJ(matrizes, np, i, j).dados, tam, k, l));
                }
                printf("\n");
            }
            printf("\n");
        }
    }

    return matrizes;
}
