#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUFFER1 32

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

 
int main(){
	
	FILE * f;
	char *buffer;
	int size_x, size_y, i, j;
	char *x=NULL, *y=NULL; 
	int **c = NULL;
	
	f = fopen("ex10.15.in", "r");
	
	buffer = malloc(SIZE_BUFFER1*sizeof(char));
	
	fgets(buffer, SIZE_BUFFER1-1, f); //Adicionar segurança para caso falhe
	sscanf(buffer,"%d %d", &size_x, &size_y);
	
	
	x = malloc((size_x+1)*sizeof(char));
	fgets(buffer, SIZE_BUFFER1-1, f);
	sscanf(buffer, "%s\n", x); //Adicionar segurança para buffer maior que x
	
	y = malloc((size_y+1)*sizeof(char));
	fgets(buffer, SIZE_BUFFER1-1,  f);
	sscanf(buffer, "%s\n", y); //Adicionar segurança para buffer maior que x
	
	printf("Linha 1: %s\nLinha 2: %s\n", x, y);

	c = (int **)calloc((size_x+1), sizeof(int*));
	for(i = 0; i < size_x; i++) c[i] = (int *)calloc((size_y+1), sizeof(int));

	
	for(i=1;i<size_x;i++){
		for(j=1;j<size_y;j++){
			if (x[i]==y[j]) c[i][j] = c[i-1][j-1]+1;
			else c[i][j] = max(c[i][j-1],c[i-1][j]);
		}
	}
	
	printf("Matriz:\n");
	for(i=0;i<size_x;i++){
		for(j=0;j<size_y;j++) printf("%d ", c[i][j]);
		printf("\n");
	}

	free(buffer);
	free(x);
	free(y);
	for(i = 0; i < size_x; i++) free(c[i]);
	free (c);
	
	exit(0);
}	