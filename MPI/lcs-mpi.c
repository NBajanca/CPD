#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ERROR -1
#define SIZE_BUFFER_0 31
#define SIZE_BUFFER 32

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

typedef struct matrix_info 
{ 
	int size_x;
	int size_y;
	char *x;
	char *y;
	char *z;
} matrix_info;

void master_io(int p, MPI_Status *status, int rounds, int argc, char **argv);
void slave_io(int id, int p, MPI_Status *status, int rounds);
void read_file(int argc, char **argv, matrix_info *A);

//cost function
short cost(int x);

int main (int argc, char *argv[]) 
{

    MPI_Status status;
    int id, p,
	i, rounds;
    double secs;

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    rounds = 10;

    MPI_Barrier (MPI_COMM_WORLD);
    secs = - MPI_Wtime();
	
	if (!id) master_io(p, &status, rounds, argc, argv);
    else slave_io(id, p, &status, rounds);

    MPI_Barrier (MPI_COMM_WORLD);
    secs += MPI_Wtime();

	
	if(!id) /*Master*/
	{
		printf("Rounds= %d, N Processes = %d, Time = %12.6f sec,\n",  rounds, p, secs);
		printf ("Average time per Send/Recv = %6.2f us\n",	secs * 1e6 / (2*rounds*p));
  	}

   	MPI_Finalize();
   	return 0;
}

/* This is the master */
void master_io(int p, MPI_Status *status, int rounds, int argc, char **argv)
{
	int i;
	matrix_info *A;

	//Inicializing Matrix Info
	A = (matrix_info*)malloc(sizeof(matrix_info));
	
	if (A  == NULL)
	{
		fprintf(stdout, "Error in A malloc\n");
		exit(ERROR);
	}

	(*A).size_x=0;
	(*A).size_y = 0;
	(*A).x=NULL;
	(*A).y=NULL;
	(*A).z=NULL;

	read_file(argc, argv, A);

	printf("sixe_x: %d, size_y: %d\n", A->size_x, A->size_y );
	printf("x: %s\n y: %s\n", A->x,A->y );

	for(i = 0; i < rounds; i++)
	{
		MPI_Send(&i, 1, MPI_INT, 1, i, MPI_COMM_WORLD);
		MPI_Recv(&i, 1, MPI_INT, p-1, i, MPI_COMM_WORLD, status);
	}
}

/* This is the slave */
void slave_io(int id, int p, MPI_Status *status, int rounds)
{
	int i;

	for(i = 0; i < rounds; i++)
	{
		MPI_Recv(&i, 1, MPI_INT, id-1, i, MPI_COMM_WORLD, status);
		MPI_Send(&i, 1, MPI_INT, (id+1)%p, i, MPI_COMM_WORLD);
	}
}



void read_file(int argc, char **argv, matrix_info *A)
{
	FILE * f;
	char *buffer;
	int i, j;
	int size_xx, size_yy;

	if (argc !=2)
	{
		printf("Correct call is: $ ./lcs-mp1 exX.X.in\n");
		exit(ERROR);
	}
	
	f = fopen(argv[1], "r");
	if (f == NULL)
	{
		fprintf(stdout, "Error opening file\n");
		exit(ERROR);						
	}
	
	buffer = malloc(SIZE_BUFFER*sizeof(char));			//	Reading input file
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer malloc\n");
		exit(ERROR);
	}
	
	fgets(buffer, SIZE_BUFFER_0, f);
	sscanf(buffer,"%d %d", &(*A).size_x, &(*A).size_y);
	
	size_xx = ((*A).size_x+ SIZE_BUFFER)*sizeof(char);
	size_yy = ((*A).size_y+ SIZE_BUFFER)*sizeof(char);
	
	if((*A).size_x<(*A).size_y) //buffer reallocated to the size (plus some margin) of the biggest line
		buffer = realloc(buffer, size_yy);
	else
		buffer = realloc(buffer, size_xx);
		
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer realloc\n");
		exit(ERROR);
	}
									//////////////////////////////////////////////							
	(*A).x = malloc(((*A).size_x+1)*sizeof(char));  
	if ((*A).x == NULL)
	{
		fprintf(stdout, "Error in x malloc\n");
		exit(ERROR);
	}
	fgets(buffer, size_xx, f); 
	sscanf(buffer, "%s\n", (*A).x);
	
	(*A).y = malloc(((*A).size_y+1)*sizeof(char));
	if ((*A).y == NULL)
	{
		fprintf(stdout, "Error in y malloc\n");
		exit(ERROR);
	}
	fgets(buffer, size_yy,  f);
	sscanf(buffer, "%s\n", (*A).y);
}

short cost(int x)
{
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++) dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}
