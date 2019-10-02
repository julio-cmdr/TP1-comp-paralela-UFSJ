#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
	Matriz *sub_matrizes;

	if (rank == MASTER) {
		matriz = matriz_criar(argv[1]);
	}

	// Verificando se a quantidade de processos é valida.
	float raiz = sqrt(size);
	if ((raiz != (int) raiz) || (matriz.n % (int) raiz != 0)) {
		if (rank == MASTER) {
			fprintf(stderr, "Número de processos inválido.\n");
			matriz_liberar(matriz);
		}
		goto encerrar;
	}

	if (rank == MASTER) {
		matriz_print(matriz);
		sub_matrizes = matriz_divide(matriz, size, raiz);

		for(i = 0; i < size; i++){
			// enviar N aqui também
			MPI_Send(sub_matrizes[i].dados, sub_matrizes[i].n*sub_matrizes[i].n, MPI_FLOAT, i, 1, MPI_COMM_WORLD);			
		}
	}

	/* receber o N e alocar B.dados
	
	Matriz B;	
	B.n = 
	B.dados = malloc()
	MPI_Recv();

	*/

	MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]){raiz, raiz}, (int[]){1, 1}, 0, &com_grade);

	int passo, r, u;
	for(passo = 0; passo < raiz; passo++){
		for(r = 0; r < raiz; r++){
			u = (r + passo) % (int)raiz;
			Matriz a = MATRIZ_IJ(sub_matrizes, raiz, r, u);

		}
	}

	encerrar:
	MPI_Finalize();

	return 0;
}