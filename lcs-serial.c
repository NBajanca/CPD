#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR -1
#define SIZE_BUFFER_0 31
#define SIZE_BUFFER 32

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

#ifndef CALC
#define CALC
	short cost(int x);
#endif

 
int main(int argc, char *argv[]){
	
	FILE * f;
	char *buffer;
	int size_x, size_y, i, j;
	int size_xx, size_yy;
	char *x=NULL, *y=NULL, *z=NULL; 
	int **c = NULL;
	
	
	if (argc !=2){
		fprintf(stdout, "Correct call is: $ ./lcs-serial ex1.in\n");
		exit(ERROR);
	}
	
	f = fopen(argv[1], "r"); //adicionar chamada atraves de argv[]
	//adicionar verificacao para abertura correcta
	
	buffer = malloc(SIZE_BUFFER*sizeof(char)); //Adicionar segurança para caso falhe
	
	fgets(buffer, SIZE_BUFFER_0, f);
	sscanf(buffer,"%d %d", &size_x, &size_y);
	
	size_xx = (size_x+ SIZE_BUFFER)*sizeof(char);
	size_yy = (size_y+ SIZE_BUFFER)*sizeof(char);
	
	if(size_x<size_y) //buffer reallocated to the size (plus some margin) of the biggest line
		buffer = realloc(buffer, size_yy);
	else
		buffer = realloc(buffer, size_xx);
		
	x = malloc((size_x+1)*sizeof(char)); //Adicionar segurança para caso falhe
	fgets(buffer, size_xx, f); 
	sscanf(buffer, "%s\n", x);
	
	y = malloc((size_y+1)*sizeof(char)); //Adicionar segurança para caso falhe
	fgets(buffer, size_yy,  f);
	sscanf(buffer, "%s\n", y);

	c = (int **)calloc((size_x+2), sizeof(int*));
	for(i = 0; i <= size_x; i++) c[i] = (int *)calloc((size_y+2), sizeof(int));

	
	for(i=1;i<= size_x;i++){
		for(j=1;j<= size_y;j++){
			if (x[i-1]==y[j-1]) c[i][j] = c[i-1][j-1]+ 1 cost(i); //match
			else c[i][j] = max(c[i][j-1],c[i-1][j]);
		}
	}


	if(size_x<size_y)
		z = malloc((size_x+1)*sizeof(char));
	else
		z = malloc((size_y+1)*sizeof(char));
		
	int k=0; //(indice para o vector z)
	
	while((i>0)&&(j>0)){
			
			if (x[i-1]==y[j-1]) {	//match, copiar para z, avancar diagonal	
				z[k]=x[i-1];
				i=i-1;
				j=j-1;
				k++;
			}
			
			else{
				
				if(c[i-1][j]>c[i][j-1])		//maior, esquerda ou acima?
					i=i-1;		//move para cima
				else
					j=j-1;		//move para a esquerda
			}
	}
	
	//sai do ciclo na posicao a seguir ao ultimo caracter
	z[k]='\0';		//terminar vector z
	
	fprintf(stdout, "%d\n", k-1);
	for(i=k-1;i>0;i--){
		fprintf(stdout, "%c", z[i]);
	}
	fprintf(stdout, "\n");

	free(buffer);
	free(x);
	free(y);
	for(i = 0; i <= size_x; i++) free(c[i]);
	free (c);
	free(z);
	fclose(f);
	
	exit(0);
}	

short cost(int x)
{
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++) dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}