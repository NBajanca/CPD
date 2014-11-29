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
	int size_xd;
	int size_yd;
	char *x;
	char *y;
	char *z;
} matrix_info;

typedef struct matrix_list
{ 
	int id[2];
	unsigned short **matrix;
	struct matrix_list *next;
} matrix_list;

void master_io(int p, MPI_Status *status, int rounds, int argc, char **argv, matrix_info *A);
void slave_io(int id, int p, MPI_Status *status, int rounds, matrix_info A);
void read_file(int argc, char **argv, matrix_info *A);
matrix_info* initialize_matrix_info(int argc, char **argv);
void divide_by_prime(matrix_info *A);
void matrix_calc(matrix_info *A, matrix_list *B);
matrix_list* initialize_matrix_list(matrix_info *A);
unsigned short *generate_matrix_info(matrix_list *B, int size, int side);

//cost function
short cost(int x);

int main (int argc, char *argv[]) 
{

    MPI_Status status;
    int id, p,
	i, rounds;
    double secs;
	matrix_info *A;


    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    

	
	A = initialize_matrix_info(argc, argv);

	if (!id)
	{
		printf("sixe_x: %d, size_y: %d\n", A->size_x, A->size_y );
		printf("sixe_xd: %d, size_yd: %d\n", A->size_xd, A->size_yd );
		printf("x: %s\ny: %s\n", A->x,A->y );
	}

	rounds = ((A->size_x)/(A->size_xd))*((A->size_y)/(A->size_yd));

    MPI_Barrier (MPI_COMM_WORLD);
    secs = - MPI_Wtime();
	
	if (!id) master_io(p, &status, rounds, argc, argv, A);
    else slave_io(id, p, &status, rounds, *A);

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
void master_io(int p, MPI_Status *status, int rounds, int argc, char **argv, matrix_info *A)
{
	int i, j;
	MPI_Request request;
	matrix_list *B;
	unsigned short *info1, *info2;
	
	B = initialize_matrix_list(A);
	matrix_calc(A, B);
	
	/*//pint small matrix
	printf("[%d][%d][%d]\n",B->matrix[0][0], B->matrix[0][1], B->matrix[0][2]);
	printf("[%d][%d][%d]\n",B->matrix[1][0], B->matrix[1][1], B->matrix[1][2]);
	printf("[%d][%d][%d]\n",B->matrix[2][0], B->matrix[2][1], B->matrix[2][2]);*/
	
	info1 = generate_matrix_info(B, A->size_yd+1, 1);
	info2 = generate_matrix_info(B, A->size_xd+1, 2);
	
	for(i = 0; i < rounds; i++/*= (p-1)*/)
	{
		for (j=0; j <= A->size_yd; j++)
			MPI_Isend(&info1[j], 1, MPI_UNSIGNED_SHORT, 1, i, MPI_COMM_WORLD, &request);
		for (j=0; j <= A->size_xd; j++)
			MPI_Isend(&info2[j], 1, MPI_UNSIGNED_SHORT, 1, i, MPI_COMM_WORLD, &request);
			
		MPI_Waitall(A->size_yd+A->size_xd,&request, status);
		
		/*printf("info1:[%d][%d][%d]\n",info1[0], info1[1], info1[2]);
		printf("info2:[%d][%d][%d]\n",info2[0], info2[1], info2[2]);*/
		
		
		for (j=0; j <= A->size_yd; j++)
			MPI_Irecv(&info1[j], 1,  MPI_UNSIGNED_SHORT, p-1, i, MPI_COMM_WORLD, &request);
		for (j=0; j <= A->size_xd; j++)
			MPI_Irecv(&info2[j], 1,  MPI_UNSIGNED_SHORT, p-1, i, MPI_COMM_WORLD, &request);
			
		MPI_Waitall(A->size_yd+A->size_xd,&request, status);

	}
	free(info1);
	free(info2);
}

/* This is the slave */
void slave_io(int id, int p, MPI_Status *status, int rounds, matrix_info A)
{
	int i, j;
	MPI_Request request;
	printf( "Hello world from process %d of %d\n", id, p);
	unsigned short *info1, *info2;
	
	info1 = (unsigned short *)calloc((A.size_yd+1), sizeof(unsigned short));
	info2 =	(unsigned short *)calloc((A.size_xd+1), sizeof(unsigned short));
	
	
	for(i = 0; i < rounds; i++/*= (p-1)*/)
	{
		for (j=0; j <= A.size_yd; j++)
			MPI_Irecv(&info1[j], 1, MPI_UNSIGNED_SHORT, id-1, i, MPI_COMM_WORLD, &request);
		for (j=0; j <= A.size_xd; j++)
			MPI_Irecv(&info2[j], 1, MPI_UNSIGNED_SHORT, id-1, i, MPI_COMM_WORLD, &request);
			
		MPI_Waitall(A.size_yd+A.size_xd,&request, status);
		
		/*printf("info1:[%d][%d][%d], id:%d\n",info1[0], info1[1], info1[2],id);
		printf("info2:[%d][%d][%d], id:%d\n",info2[0], info2[1], info2[2],id);*/
	
		for (j=0; j <= A.size_yd; j++)
			MPI_Isend(&info1[j], 1, MPI_UNSIGNED_SHORT, (id+1)%p, i, MPI_COMM_WORLD, &request);
		for (j=0; j <= A.size_xd; j++)
			MPI_Isend(&info2[j], 1, MPI_UNSIGNED_SHORT, (id+1)%p, i, MPI_COMM_WORLD, &request);
			
		MPI_Waitall(A.size_yd+A.size_xd,&request, status);
	}
	
	free(info1);
	free(info2);
}

unsigned short *generate_matrix_info(matrix_list *B, int size, int side)
{
	unsigned short *info;
	int i;
	
	info = (unsigned short *)calloc((size+1), sizeof(unsigned short));
	
	for (i = 0; i < size; i++)
	{
		if(side == 1) info[i] = B->matrix[size-1][i];
		if(side == 2) info[i] = B->matrix[i][size-1];
	} 

	return info;
}


matrix_list* initialize_matrix_list(matrix_info *A)
{
	int i;
	matrix_list *B;
	
	B = (matrix_list*)malloc(sizeof(matrix_list));
	if (B  == NULL)
	{
		fprintf(stdout, "Error in B malloc\n");
		exit(ERROR);
	}
	
	B-> matrix = (unsigned short **)calloc((A->size_xd+2), sizeof(unsigned short*));
	if (B-> matrix  == NULL)
	{
		fprintf(stdout, "Error in B-> matrix malloc\n");
		exit(ERROR);
	}
	
	for(i = 0; i <= A->size_xd+1; i++)
	{
		B-> matrix [i] = (unsigned short *)calloc((A->size_yd+2), sizeof(unsigned short));
		if (B-> matrix[i] == NULL)
		{
		fprintf(stdout, "Error in B->matrix[i] calloc\n");
		exit(ERROR);
		}	
	}
	
	B-> id[0] = 1; 
	B-> id[1] = 1;
	
	B->next = (matrix_list*)malloc(sizeof(matrix_list));
	
	
	return B;
}

void matrix_calc(matrix_info *A, matrix_list *B)
{
	int i, j;
	for(i = 1;i <= A->size_xd;i++)
	{
		for(j =1 ;j<= A->size_yd;j++)
		{
			if (A->x[B-> id[0]+ i-2]==A->y[B-> id[1] + j-2]) 				//	computation of matrix c
				B->matrix[i][j] = B->matrix[i-1][j-1]+ cost(i); //match
			else 
				B->matrix[i][j] = max(B->matrix[i][j-1],B->matrix[i-1][j]);
		}
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
	
matrix_info* initialize_matrix_info(int argc, char **argv)
{
	matrix_info *A;
	
	A = (matrix_info*)malloc(sizeof(matrix_info));
	if (A  == NULL)
	{
		fprintf(stdout, "Error in A malloc\n");
		exit(ERROR);
	}

	(*A).size_x = 0;
	(*A).size_y = 0;
	(*A).size_xd= 0;
	(*A).size_yd= 0;
	(*A).x=NULL;
	(*A).y=NULL;
	(*A).z=NULL;

	read_file(argc, argv, A);
	divide_by_prime(A);

	return A;
}

void divide_by_prime(matrix_info *A)
{
	int prime_num[10]={2,3,5,7,11,13,17,19,23,29};
	int i;

	for (i=0; i<10; i++)
	{
		if (((*A).size_x % prime_num[i]) == 0)
		{
			(*A).size_xd = prime_num[i];
			break;
		}
	}

	for (i=0; i<10; i++)
	{
		if (((*A).size_y % prime_num[i]) == 0)
		{
			(*A).size_yd = prime_num[i];
			break;
		}
	}

}

short cost(int x)
{
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++) dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}
