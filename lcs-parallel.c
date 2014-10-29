#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h> 

#define ERROR -1
#define SIZE_BUFFER_0 31
#define SIZE_BUFFER 32

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

short cost(int x)
{
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++) dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}
 
int main(int argc, char *argv[]){
	
	FILE * f;
	char *buffer;
	int size_x, size_y, i, j;
	int size_xx, size_yy;
	char *x=NULL, *y=NULL, *z=NULL; 
	unsigned short **c = NULL;
	
		
	if (argc !=2)
	{
		fprintf(stdout, "Correct call is: $ ./lcs-serial ex1.in\n");
		exit(ERROR);
	}
	
	f = fopen(argv[1], "r");
	if (f == NULL)
	{
		fprintf(stdout, "Error opening file\n");
		exit(ERROR);
	}
	
	buffer = malloc(SIZE_BUFFER*sizeof(char));
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer malloc\n");
		exit(ERROR);
	}
	
	fgets(buffer, SIZE_BUFFER_0, f);
	sscanf(buffer,"%d %d", &size_x, &size_y);
	
	size_xx = (size_x+ SIZE_BUFFER)*sizeof(char);
	size_yy = (size_y+ SIZE_BUFFER)*sizeof(char);
	
	if(size_x<size_y) //buffer reallocated to the size (plus some margin) of the biggest line
		buffer = realloc(buffer, size_yy);
	else
		buffer = realloc(buffer, size_xx);
		
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer realloc\n");
		exit(ERROR);
	}
		
	x = malloc((size_x+1)*sizeof(char));
	if (x == NULL)
	{
		fprintf(stdout, "Error in x malloc\n");
		exit(ERROR);
	}
	fgets(buffer, size_xx, f); 
	sscanf(buffer, "%s\n", x);
	
	y = malloc((size_y+1)*sizeof(char));
	if (y == NULL)
	{
		fprintf(stdout, "Error in y malloc\n");
		exit(ERROR);
	}
	fgets(buffer, size_yy,  f);
	sscanf(buffer, "%s\n", y);

	c = (unsigned short **)calloc((size_x+2), sizeof(unsigned short*));
	if (c == NULL)
	{
		fprintf(stdout, "Error in c malloc\n");
		exit(ERROR);
	}
	
	
	//not worth to parellize
	for(i = 0; i <= size_x; i++)
	{
		c[i] = (unsigned short *)calloc((size_y+2), sizeof(unsigned short));
		if (c[i] == NULL)
		{
		fprintf(stdout, "Error in c[%d] calloc\n",i);
		exit(ERROR);
		}
		
	}
	
	int h;
	
	if(size_x<=size_y){
		
		
		for(h=1;h <= size_y;h++)
		{	
			#pragma omp parallel for private(i)
			for(j=h; j>0; j--){
				i=h-j+1;
				if(i<=size_x){	
					if (x[i-1]==y[j-1]) 
						c[i][j] = c[i-1][j-1]+ cost(i); //match
					else 
						c[i][j] = max(c[i][j-1],c[i-1][j]);
				}
			}
			
		}
		
		for(h= size_y-size_x ;h <= size_y;h++)
		{
			#pragma omp parallel for private(i)
			for(j=h ; j<=size_y; j++ ){
				i=h-j+size_x;
					if(i>=1){
						if (x[i-1]==y[j-1]) 
							c[i][j] = c[i-1][j-1]+ cost(i); //match
						else 
							c[i][j] = max(c[i][j-1],c[i-1][j]);
					}
			}
			
		}
	}
	else{
		for(h=1;h <= size_x;h++)
		{
			
			#pragma omp parallel for private(j)
			for(i=h; i>0; i--){
				j=h-i+1;
				if(j<=size_y){	
					if (x[i-1]==y[j-1]) 
						c[i][j] = c[i-1][j-1]+ cost(i); //match
					else 
						c[i][j] = max(c[i][j-1],c[i-1][j]);
				}
			}
		}	
		
		for(h=abs(size_x-size_y);h <= size_x;h++)
		{
			#pragma omp parallel for private(j)
			for(i=h; i<=size_x; i++){
				j=h-i+size_y;
					if(j>=1){
						if (x[i-1]==y[j-1]) 
							c[i][j] = c[i-1][j-1]+ cost(i); //match
						else 
							c[i][j] = max(c[i][j-1],c[i-1][j]);
					}
			}
		}
	}
	
	
	
	/*for(i=0;i<= size_x;i++)
	{
		for(j=0;j<= size_y;j++)
		{
			printf("%d ", c[i][j]);
		}
		printf("\n");
	}*/
 

	if(size_x<size_y)
		z = malloc((size_x+1)*sizeof(char));
	else
		z = malloc((size_y+1)*sizeof(char));
		
	if (z == NULL)
	{
		fprintf(stdout, "Error in z malloc\n");
		exit(ERROR);
	}
		
	int k=0; //(indice para o vector z)
	
	i = size_x+1;
	j = size_y+1;
	
	while((i>0)&&(j>0))
	{
			
		if (x[i-1]==y[j-1]) //match
		{	
			z[k]=x[i-1];
			i=i-1;
			j=j-1;
			k++;
		}
			
		else
		{
			
			if(c[i-1][j]>c[i][j-1])	
				i=i-1;	//up
			else
				j=j-1;	//left
		}
	}
	
	z[k]='\0';	//end vector z
	
	fprintf(stdout, "%d\n", k-1);
	for(i=k-1;i>0;i--)
	{
		fprintf(stdout, "%c", z[i]);
	}
	fprintf(stdout, "\n");

	


	free(buffer);
	free(x);
	free(y);
	for(i = 0; i <= size_x; i++) 
		free(c[i]);
	free (c);
	free(z);
	fclose(f);
	
	exit(0);
	
}		
	
