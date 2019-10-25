#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
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
		printf("Impossível inicializar o MPI!\n");
		return 1;
	}

	if (argc < 2) {
		printf("Favor passar um arquivo de entrada!\n");
		goto encerrar;
	}

	int calcular_tempo = argc > 2 && strcmp(argv[2], "-t") == 0;

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

	MPI_Bcast(&matriz.n, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	if ((raiz != q) || (matriz.n % q != 0)) {
		if (rank == MASTER) {
			fprintf(stderr, "Número de processos inválido.\n");
			matriz_liberar(matriz);
		}
		goto encerrar;
	}

	struct timeval tvi, tvf;

	// Dividindo a matriz em sub-matrizes e espalhando-as para os processos.
	if (rank == MASTER) {
		// Começa a contar o tempo.
		if (calcular_tempo) {
			gettimeofday(&tvi, NULL);
		}

		sub_matrizes = matriz_divide(matriz, size, q);

		for (i = 0; i < size; i++) {
			MPI_Send(&sub_matrizes[i].n, 1, MPI_INT, i, TAG_TAM, MPI_COMM_WORLD);
			if (i != MASTER) {
				MPI_Send(sub_matrizes[i].dados, sub_matrizes[i].n * sub_matrizes[i].n, MPI_FLOAT, i, TAG_DADOS, MPI_COMM_WORLD);
			}
		}
	}

	// Recebendo a sub-matriz.
	Matriz A, A2, B, C;
	MPI_Recv(&A.n, 1, MPI_INT, MASTER, TAG_TAM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	C.n = B.n = A2.n = A.n;
	A.dados = malloc(A.n * A.n * sizeof(float));
	A2.dados = malloc(A2.n * A2.n * sizeof(float));
	B.dados = malloc(B.n * B.n * sizeof(float));
	C.dados = calloc(C.n * C.n, sizeof(float));
	if (rank != MASTER) {
		MPI_Recv(B.dados, B.n * B.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	} else {
		B.dados = sub_matrizes[rank].dados;
	}

	// Criando comunicadores.
	MPI_Comm com_grade, com_linha;
	int rank_linha, rank_grade_baixo, rank_grade_cima;
	int linha = rank / q;

	MPI_Comm_split(MPI_COMM_WORLD, linha, rank, &com_linha);
	MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]) {q, q}, (int[]) {1, 1}, 0, &com_grade);

	MPI_Comm_rank(com_linha, &rank_linha);
	MPI_Cart_shift(com_grade, 0, 1, &rank_grade_cima, &rank_grade_baixo);

	MPI_Request request;
	for(int g=1; g < matriz.n; g*=2){
		memcpy(A.dados, B.dados, B.n * B.n * sizeof *B.dados);

		for (i = 0; i < C.n * C.n; i++) {
			C.dados[i] = INFINITY;
		}

		// Algoritmo de Fox.
		int passo, r, u, rank_escolhido;
		r = linha;
		for (passo = 0; passo < q; passo++) {
			u = (r + passo) % q;

			MPI_Cart_rank(com_grade, (int[]) {r, u}, &rank_escolhido);

			if (rank_linha == rank_escolhido % q) {
				memcpy(A2.dados, A.dados, A.n * A.n * sizeof *B.dados);
			}

			MPI_Bcast(A2.dados, A2.n * A2.n, MPI_FLOAT, rank_escolhido % q, com_linha);

			// Multiplicar a matriz recebida.
			matriz_acumular(A2, B, &C);

			// Recebe a matriz B para o vizinho de cima.
			MPI_Irecv(B.dados, B.n * B.n, MPI_FLOAT, rank_grade_baixo, TAG_DADOS, com_grade, &request);

			// Envia a matriz B para o vizinho de cima.
			MPI_Send(B.dados, B.n * B.n, MPI_FLOAT, rank_grade_cima, TAG_DADOS, com_grade);

			MPI_Wait(&request, MPI_STATUS_IGNORE);
		}

		memcpy(B.dados, C.dados, C.n * C.n * sizeof * C.dados);
	}

	// Junta as submatrizes C calculadas.
	if (rank == MASTER) {
		memcpy(sub_matrizes[rank].dados, C.dados, C.n * C.n * sizeof C.dados[0]);

		for (i = 0; i < size; i++) {
			if (i != MASTER) {
				MPI_Recv(sub_matrizes[i].dados, sub_matrizes[i].n * sub_matrizes[i].n, MPI_FLOAT, i, TAG_DADOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}

		// Junta as submatrizes recebidas
		float *sub_matriz;
		for (i = 0; i < q; i++){
			for (j = 0; j < q; j++){

				sub_matriz = MATRIZ_IJ(sub_matrizes, q, i, j).dados;

				for (int k = 0; k < matriz.n/q; k++){
					for (int l = 0; l < matriz.n/q; l++){
						MATRIZ_IJ(matriz.dados, matriz.n, i*matriz.n/q + k, j*matriz.n/q + l) = MATRIZ_IJ(sub_matriz, matriz.n/q, k, l);
					}
				}
			}
		}

		// Termina a contagem de tempo
		if (calcular_tempo) {
			gettimeofday(&tvf, NULL);
		}

		matriz_print(matriz);

		if (calcular_tempo) {
			/// TODO: consertar.
			printf("\n%f\n", (tvf.tv_sec + 1e-6 * tvf.tv_usec) - (tvi.tv_sec + 1e-6 * tvi.tv_usec));
		}

	} else {
		MPI_Send(C.dados, C.n * C.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD);
	}

	encerrar:
	MPI_Finalize();

	return 0;
}
