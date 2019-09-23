#include <stdio.h>
#include <stdlib.h>

float* preenche_matriz(char *nome_arquivo){
	int numVertices;
	float *matriz;
	
	FILE *arq;
	arq = fopen(nome_arquivo, "r");
	
	if(arq == NULL){
		printf("Erro ao abrir o arquivo!\n");
		exit(0);
	}else{		
		fscanf(arq,"%d\n", &numVertices);
		
		matriz = (float*)malloc(sizeof(float)*numVertices*numVertices + 1);
		matriz[0] = numVertices;
		matriz = matriz + 1;
		
		int i, j;
		
		for(i=0; i<numVertices; i++){
			for(j=0; j<numVertices; j++){
				if(j != (numVertices -1)){
					fscanf(arq,"%f ",&matriz[i*numVertices + j]);	
				}else{
					fscanf(arq,"%f\n",&matriz[i*numVertices + j]);
				}
			}
		}					
	}
	fclose(arq);
	return matriz;
}

int main(int argc, char **argv){
	int i, j, numVertices;

	if(argc < 2){
		printf("Favor passar um arquivo de entrada!\n");
		exit(0);
	}

	float *matriz = preenche_matriz(argv[1]);
	numVertices = (int)matriz[-1];
	
	for(i=0; i < numVertices; i++){
		for(j=0; j < numVertices; j++){
			printf("%.1f ", matriz[i*numVertices + j]);
		}
		printf("\n");
	}
	
}

	
