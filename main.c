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
		printf("Impossível inicializar o MPI!\n");
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

	MPI_Bcast(&matriz.n, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	if ((raiz != q) || (matriz.n % q != 0)) {
		if (rank == MASTER) {
			fprintf(stderr, "Número de processos inválido.\n");
			matriz_liberar(matriz);
		}
		goto encerrar;
	}

	// Dividindo a matriz em sub-matrizes e espalhando-as para os processos.
	if (rank == MASTER) {
		//matriz_print(matriz);
		sub_matrizes = matriz_divide(matriz, size, q);

		for (i = 0; i < size; i++) {
			MPI_Send(&sub_matrizes[i].n, 1, MPI_INT, i, TAG_TAM, MPI_COMM_WORLD);
			MPI_Send(sub_matrizes[i].dados, sub_matrizes[i].n * sub_matrizes[i].n, MPI_FLOAT, i, TAG_DADOS, MPI_COMM_WORLD);
		}
	}

	// Criando comunicadores.
	MPI_Comm com_grade, com_linha;
	int rank_linha, rank_grade_baixo, rank_grade_cima;
	int linha = rank / q;

	MPI_Comm_split(MPI_COMM_WORLD, linha, rank, &com_linha);
	MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]) {q, q}, (int[]) {1, 1}, 0, &com_grade);

	MPI_Comm_rank(com_linha, &rank_linha);
	MPI_Cart_shift(com_grade, 0, 1, &rank_grade_cima, &rank_grade_baixo);


	// Recebendo a sub-matriz.
	Matriz A, A2, B, C;
	MPI_Recv(&A.n, 1, MPI_INT, MASTER, TAG_TAM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	C.n = B.n = A2.n = A.n;
	A.dados = malloc(A.n * A.n * sizeof(float));
	A2.dados = malloc(A2.n * A2.n * sizeof(float));
	B.dados = malloc(B.n * B.n * sizeof(float));
	C.dados = calloc(C.n * C.n, sizeof(float));
	MPI_Recv(B.dados, B.n * B.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	for(int g=1; 2*g < matriz.n || g >= matriz.n; g*=2){
		memcpy(A.dados, B.dados, B.n * B.n * sizeof *B.dados);

		for (i = 0; i < C.n * C.n; i++) {
			C.dados[i] = INFINITY;
		}

		// Algoritmo de Fox.
		int passo, r, u, rank_escolhido;
		r = linha;
		for (passo = 0; passo < q; passo++) {            // TODO: mudar para q
			u = (r + passo) % q;

			MPI_Cart_rank(com_grade, (int[]) {r, u}, &rank_escolhido);

			if (rank_linha == rank_escolhido % q) {
				memcpy(A2.dados, A.dados, A.n * A.n * sizeof *B.dados);
			}

			MPI_Bcast(A2.dados, A2.n * A2.n, MPI_FLOAT, rank_escolhido % q, com_linha);

			// Multiplicar a matriz recebida.
			matriz_acumular(A2, B, &C);

			// Envia a matriz B para o vizinho de cima.
			MPI_Send(B.dados, B.n * B.n, MPI_FLOAT, rank_grade_cima, TAG_DADOS, com_grade);

			// Recebe a matriz B para o vizinho de cima.
			MPI_Recv(B.dados, B.n * B.n, MPI_FLOAT, rank_grade_baixo, TAG_DADOS, com_grade, MPI_STATUS_IGNORE);
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

		matriz_print(matriz);
		
	}else{
		MPI_Send(C.dados, C.n * C.n, MPI_FLOAT, MASTER, TAG_DADOS, MPI_COMM_WORLD);
	}	

	encerrar:
	MPI_Finalize();

	return 0;
}
