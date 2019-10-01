#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "matriz.h"

#define MASTER 0

int main(int argc, char *argv[]) {
	int i, j;

	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		// TODO: mensagem de erro
		return 1;
	}

	if(argc < 2){
		printf("Favor passar um arquivo de entrada!\n");
		goto encerrar;
	}

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	Matriz matriz;

	if (rank == MASTER) {
		matriz = matriz_criar(argv[1]);
	}

	// Verificando se a quantidade de processos é valida.
	float raiz = sqrt(size);
	if ((raiz - (int) raiz != 0) || (matriz.n % (int) raiz != 0)) {
		if (rank == MASTER) {
			fprintf(stderr, "Número de processos inválido.\n");
			matriz_liberar(matriz);
		}
		goto encerrar;
	}

	if (rank == MASTER) {
		for (i=0; i < matriz.n; i++){
			for(j=0; j < matriz.n; j++){
				printf("%.1f ", MATRIZ_IJ(matriz.dados, matriz.n, i, j));
			}
			printf("\n");
		}
	}

	encerrar:
	MPI_Finalize();

	return 0;
}
