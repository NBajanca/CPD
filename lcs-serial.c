#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUFFER1 32

 
int main(){
	
	FILE * f;
	char *buffer;
	int size_x, size_y, i, n;
	char *x=NULL, *y=NULL; 
	int **c = NULL;
	
	f = fopen("ex10.15.in", "r");
	
	buffer = malloc(SIZE_BUFFER1*sizeof(char));
	
	fgets(buffer, SIZE_BUFFER1-1, f); //Adicionar segurança para caso falhe
	sscanf(buffer, "%d %d", &size_x, &size_y);
	
	
	x = malloc(size_x*sizeof(char));
	fgets(buffer, SIZE_BUFFER1-1, f);
	sscanf(buffer, "%s\n", x); //Adicionar segurança para buffer maior que x
	
	y = malloc(size_y*sizeof(char));
	fgets(buffer, SIZE_BUFFER1-1, f);
	sscanf(buffer, "%s\n", y); //Adicionar segurança para buffer maior que x
	
	printf("Linha 1: %s\nLinha 2: %s\n", x, y);

	c = (int **)malloc(size_x * sizeof(int*));
	for(i = 0; i < size_x; i++) c[i] = (int *)malloc(size_y * sizeof(int));
	
	for(i=0;i<size_x;i++) 
	c[i][0]=1;
	
	printf("Matriz:\n");
	for(i=0;i<size_x;i++){
		for(n=0;n<size_y;n++) printf("%d ", c[i][n]);
		printf("\n");
	}
	
	free(buffer);
	free(x);
	free(y);
	
	exit(0);
}	
