#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUFFER1 2048

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

 
int main(){
	
	FILE * f, *f_out;
	char *buffer;
	int size_x, size_y, i, j, k;
	char *x=NULL, *y=NULL, *z=NULL; 
	int **c = NULL;
	
	f = fopen("ex150.200.in", "r"); //adicionar chamada atraves de argv[]
	//adicionar verificacao para abertura correcta
	
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


	//Leitura Matriz
	//Criar sequencia de caracteres de saida, 
	//quanto muito tera o tamanho da sequencia de entrada menor (se esta estiver totalmente repetida na maior)
	//(Vejam se isto faz sentido...)

	if(size_x<size_y)
		z = malloc((size_x+1)*sizeof(char));
	else
		z = malloc((size_y+1)*sizeof(char));
		
	
	printf("i=%d, j=%d\n", i-1, j-1); //debug
	printf("valor ultimo elemento: %d\n", c[i-1][j-1]);//debug
	
	//usar os valores de "i-1" e "j-1" (ultima posicao da matriz)
	i=i-1;
	j=j-1;
	
	k=0; //(indice para o vector z)
	
	while((i>0)&&(j>0)){
			
			if (x[i]==y[j]) {	//match, copiar para z, avancar diagonal	
				z[k]=x[i];
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
		
		printf("%d %d\n", i, j);
	}
	
	//sai do ciclo na posicao a seguir ao ultimo caracter
	z[k]='\0';		//terminar vector z
	
	printf("%s\n", z);
	
	//acho q e preciso inverter esta cena..
	//mais vale escrever invertido para o ficheiro, percorre-se menos vezes os vectores gigantes
	

	//Escrita no ficheiro de saida
	
	f_out = fopen("ex150.200.out", "w"); //alterar para colocar o nome do ficheiro de entrada -".in" + ".out"
	//verificar se abre correctamente
	
	//1a linha: tamanho da sequencia -> indice k (valor e o numero de cenas la dentro, pq esta uma posicao acima do ultimo caracter...)
	fprintf(f_out, "%d\n", k);
	
	//2a linha: escrever o vector z (termina-lo em '\0' e lê-lo como string?)
	
	for(i=k-1;i>0;i--){
		fprintf(f_out, "%c", z[i]);
	}
	fprintf(f_out, "%c", z[i]);
	
	//3a linha: \n
	fprintf(f_out, "\n");


	free(buffer);
	free(x);
	free(y);
	for(i = 0; i < size_x; i++) free(c[i]);
	free (c);
	
	exit(0);
}	
