#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <mpi.h>
#include "matriz.h"

#define MASTER 0

enum {
	TAG_TAM = 1,
	TAG_DADOS = 2
};

int main(int argc, char *argv[]) {
	int i, j;

	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		// TODO: mensagem de erro
		return 1;
	}

	if (argc < 2) {
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
	int q = raiz;
	if ((raiz != q) || (matriz.n % q != 0)) {
		if (rank == MASTER) {
			fprintf(stderr, "Número de processos inválido.\n");
			matriz_liberar(matriz);
		}
		goto encerrar;
	}

	// Dividindo a matriz em sub-matrizes e espalhando-as para os processos.
	if (rank == MASTER) {
		matriz_print(matriz);
		sub_matrizes = matriz_divide(matriz, size, q);

		for (i = 0; i < size; i++) {
			MPI_Send(&sub_matrizes[i].n, 1, MPI_INT, i, TAG_TAM, MPI_COMM_WORLD);
			MPI_Send(sub_matrizes[i].dados, sub_matrizes[i].n * sub_matrizes[i].n, MPI_FLOAT, i, TAG_DADOS, MPI_COMM_WORLD);
		}
	}

	// Recebendo a sub-matriz.
	Matriz A, B;
	MPI_Recv(&B.n, 1, MPI_INT, MASTER, TAG_TAM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	A.n = B.n;
	B.dados = malloc(B.n * B.n * sizeof(float));
	A.dados = malloc(A.n * A.n * sizeof(float));
	MPI_Recv(B.dados, B.n * B.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


	// Criando comunicadores.
	MPI_Comm com_grade, com_linha;
	int rank_linha, rank_grade;

	MPI_Comm_split(MPI_COMM_WORLD, rank / q, rank, &com_linha);
	MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]) {q, q}, (int[]) {1, 1}, 0, &com_grade);
	MPI_Comm_rank(com_linha, &rank_linha);


	// Algoritmo de Fox.
	int passo, r, u, rank_escolhido;
	for (passo = 0; passo < 1; passo++) {            // TODO: mudar para q
		for (r = 0; r < 1; r++) {
			u = (r + passo) % q;
			MPI_Cart_rank(com_grade, (int[]) {r, u}, &rank_escolhido);
			printf("Meu rank: %d (%d, %d), Rank escolhido: %d\n", rank, r, u, rank_escolhido);

			if (rank == rank_escolhido) {
				memcpy(A.dados, B.dados, B.n * B.n * sizeof *B.dados);
			}

			MPI_Bcast(A.dados, A.n * A.n, MPI_FLOAT, rank_escolhido, com_linha);
		}
	}

	encerrar:
	MPI_Finalize();

	return 0;
}
