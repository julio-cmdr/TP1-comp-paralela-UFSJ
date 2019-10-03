#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <mpi.h>
#include "matriz.h"

#define MASTER 0

enum{
	TAG_TAM = 1,
	TAG_DADOS = 2
};

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
			MPI_Send(&sub_matrizes[i].n, 1, MPI_INT, i, TAG_TAM, MPI_COMM_WORLD);
			MPI_Send(sub_matrizes[i].dados, sub_matrizes[i].n*sub_matrizes[i].n, MPI_FLOAT, i, TAG_DADOS, MPI_COMM_WORLD);
		}
	}
	
	Matriz A, B;
	MPI_Recv(&B.n, 1, MPI_INT, MASTER, TAG_TAM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	A.n = B.n;
	B.dados = malloc(B.n*B.n * sizeof(float));
	A.dados = malloc(A.n*A.n * sizeof(float));
	MPI_Recv(B.dados, B.n*B.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	
	MPI_Comm com_grade, com_linha;
	MPI_Group grupo_linha, grupo_global;
	int rank_linha, rank_grade;

	int *ranks = malloc(sizeof(*ranks) * (int)raiz);
	int inicio_linha = rank/(int)raiz;
	for(i = 0; i < raiz; i++){
		ranks[i] = inicio_linha + i;               //  fazer correções aqui
	}
	
	MPI_Comm_group(MPI_COMM_WORLD, &grupo_global);	
	MPI_Group_incl(grupo_global, (int)raiz, ranks, &grupo_linha);
	MPI_Comm_create(MPI_COMM_WORLD, grupo_linha, &com_linha);
	MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]){raiz, raiz}, (int[]){1, 1}, 0, &com_grade);
	MPI_Comm_rank(com_linha, &rank_linha);

	int passo, r, u, rank_escolhido;

	for(passo = 0; passo < 1; passo++){            // mudar para raiz
		for(r = 0; r < 1; r++){
			u = (r + passo) % (int)raiz;
			MPI_Cart_rank(com_grade, (int[]){r, u}, &rank_escolhido);
			printf("Meu rank: %d, Rank escolhido: %d\n", rank, rank_escolhido);
			
			if(rank == rank_escolhido){
				for (i = 0; i < raiz; i++){
					MPI_Bcast(B.dados, B.n*B.n, MPI_FLOAT, rank_linha, com_linha);		
				}
			}

			MPI_Recv(A.dados, A.n*A.n, MPI_FLOAT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);			

		}
	}

	encerrar:
	MPI_Finalize();

	return 0;
}