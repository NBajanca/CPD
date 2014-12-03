#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ERROR -1
#define SIZE_BUFFER_0 31
#define SIZE_BUFFER 32
#define JUMPTAG 0

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )
#define min(a,b) ( ((a)<(b)) ? (a) : (b) )

#ifndef LCS_MPI
	#define LCS_MPI
	typedef struct matrix_info 
	{ 
		int size_x;
		int size_y;
		int size_xd;
		int size_yd;
		int iter;
		unsigned short **matrix_iter;
		unsigned short *matrix_dist;
		char *x;
		char *y;
	} matrix_info;

	typedef struct matrix_list
	{ 
		int id[2];
		unsigned short **matrix;
		struct matrix_list *next;
	} matrix_list;

	void master_io(int p, MPI_Status *status, matrix_info *A);
	void slave_io(int id, int p, MPI_Status *status, matrix_info *A);
	void jump_process(matrix_info *A, int p);

	matrix_info* initialize_matrix_info(char **argv);
	void read_file(char **argv, matrix_info *A);
	void divide_by_prime(matrix_info *A);
	void matrix_iter(matrix_info *A);
	void find_pos(matrix_info *A, int iter, int *x, int *y);

	matrix_list* initialize_matrix_list(matrix_info *A);
	void matrix_calc(matrix_info *A, matrix_list *B);
	short cost(int x);
	unsigned short *generate_matrix_info(matrix_list *B, int size, int size_ref, int side);
	unsigned short ***initialize_info(matrix_info *A, matrix_list *B);
	void go_back (matrix_info *A, matrix_list *B, char *z, int *k ,int **pos_final)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;

#endif

int main (int argc, char *argv[]) 
{

    MPI_Status status;
    int id, p, i;
    double secs;
	matrix_info *A;
	
	//if file name is not added to the program call
	if (argc !=2)
	{
		printf("Correct call is: $ ./lcs-mp1 exX.X.in\n");
		exit(ERROR);
	}


    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    
	A = initialize_matrix_info(argv);
	
	//Only for the master, prints matrix info -> debug only
	if (!id)
	{
		printf("sixe_x: %d, size_y: %d\n", A->size_x, A->size_y );
		printf("sixe_xd: %d, size_yd: %d\n", A->size_xd, A->size_yd );
		printf("x: %s\ny: %s\n", A->x,A->y );
	}
	
	//allocs and initializes the A-> matrix_dist
	A-> matrix_dist = (unsigned short *)calloc((A -> iter +1), sizeof(unsigned short));

	//wait for every process to be ready
    MPI_Barrier (MPI_COMM_WORLD);
    secs = - MPI_Wtime();
	
	//separates master from slaves (master is process 0)
	if (!id) master_io(p, &status, A);
    else slave_io(id, p, &status, A);
	
	//waits for all to be finished
    MPI_Barrier (MPI_COMM_WORLD);
    secs += MPI_Wtime();

	/*//prints time spent
	if(!id)
	{
		printf("Iterations= %d, N Processes = %d, Time = %12.6f sec,\n",  A-> iter, p, secs);
		printf ("Average time per Iteration/Process = %12.6f sec\n",	secs/(A->iter/p));
  	}*/

   	MPI_Finalize();
   	return 0;
}

/* master_io (p, status, A)
 * 
 * Input:
 * 			p -> number of processes
 * 			status -> MPI_status
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			MPI_Status -> due to sends and receives
 * 			A.matrix_dist -> Adds the process id that runs each iteration
 * 
 * Description:
 * 			This function is the one with the master code so it's the one that distributes all the information true all the processes.
 * 
 * Notes:
 * 			This function is only run by the master (process id=0)			
 * 
 */
void master_io(int p, MPI_Status *status, matrix_info *A)
{
	MPI_Request request;
	int i, j, 
		send_id = 1, 
		x=1, y=1,
		iter_aux= 1;
	matrix_list *B, *B_last, *B_aux;
	unsigned short *info_blanck[2], *info_aux[2];
	unsigned short ***info;
	
	//Print iteraction matrix
	for(i=0;i<=A->size_x/A->size_xd;i++)
	{
		for(j=0; j<=A->size_y/A->size_yd;j++)
		{
			printf("[%d]", A-> matrix_iter [i][j]);
		}
		printf ("\n");
	}
	
	info = initialize_info(A, B);
	
	B = initialize_matrix_list(A); //first B structure of master
	info[0][0] = generate_matrix_info(B, A->size_yd, A->size_xd, 1);
	info[0][1] = generate_matrix_info(B, A->size_xd,A->size_yd, 2);
	info_aux[0] = generate_matrix_info(B, A->size_yd,A->size_xd, 1);
	info_aux[1] = generate_matrix_info(B, A->size_xd,A->size_yd, 2);
	
	matrix_calc(A, B); //calc of the iteration 1
	B_last = B;
	
	//matrix calculated print for debug porpuse
	printf("matrix [%d][%d] from process: 0\n", B_last-> id[0], B_last-> id[1]);
	for (i=1; i <=A->size_xd; i++)
	{
		for (j=1; j<= A->size_yd; j++)
			printf("[%d]",B_last->matrix[i][j]);
		printf("\n");
	}printf("\n");
	
	info[1][0] = generate_matrix_info(B, A->size_yd,A->size_xd, 1);
	info[1][1] = generate_matrix_info(B, A->size_xd,A->size_yd, 2);
	
	for (j=0; j <= A->size_yd; j++)
		info[1][0][j];
	
	for (j=0; j <= A->size_xd; j++)
		info[1][1][j];

	
	for(i = 2; i < A->iter; i++)
	{	
		info[i][0] = generate_matrix_info(B, A->size_yd,A->size_xd, 1);
		info[i][1] = generate_matrix_info(B, A->size_xd,A->size_yd, 2);
			
		find_pos(A, i, &x, &y); //finds the position for the iteration that is about to send
		A -> matrix_dist[i] = send_id;
		
		printf("iter = %d to proc = %d\n", i, send_id);
		iter_aux = A-> matrix_iter [x-1][y];
		//master sends the data needed to calc the [x][y] matrix to the process (send_id)
		for (j=0; j <= A->size_yd; j++)
			MPI_Send(&info[iter_aux][0][j], 1, MPI_UNSIGNED_SHORT, send_id, x, MPI_COMM_WORLD);
		
		iter_aux = A-> matrix_iter [x][y-1];
		for (j=0; j <= A->size_xd; j++)
			MPI_Send(&info[iter_aux][1][j], 1, MPI_UNSIGNED_SHORT, send_id, y, MPI_COMM_WORLD);
		
		//Next process
		send_id = send_id+1;
		if (send_id == p) send_id = 1;

		//Master waits to receive from any slave
		MPI_Recv(&info_aux[0][0], 1,  MPI_UNSIGNED_SHORT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		for (iter_aux =i; iter_aux>0; iter_aux--)
			if ( A -> matrix_dist[iter_aux] == status->MPI_SOURCE) break;

		info[iter_aux][0][0] = info_aux[0][0];
		//Master receives the rest of the message from the slave that was already sending
		for (j=1; j <= A->size_yd; j++)
			MPI_Recv(&info[iter_aux][0][j], 1,  MPI_UNSIGNED_SHORT, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, status);
		for (j=0; j <= A->size_xd; j++)
			MPI_Recv(&info[iter_aux][1][j], 1,  MPI_UNSIGNED_SHORT, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, status);
	

	}	
	
	jump_process(A, p);
	//Calc of last iteration
	find_pos(A, A->iter, &x, &y);
	B_aux = initialize_matrix_list(A);
	B_last->next = B_aux;
	B_last = B_aux;
	
	B_last-> id[0] = x;
	B_last-> id[1] = y;
	
	iter_aux = A-> matrix_iter [x-1][y];
	for (j=0; j <= A->size_yd; j++)
		B_last->matrix[0][j] = info[iter_aux][0][j];
		
	iter_aux = A-> matrix_iter [x][y-1];
	for (j=0; j <= A->size_xd; j++)
		B_last->matrix[j][0] = info[iter_aux][1][j];
		
	matrix_calc(A, B_last);
	
	//matrix calculated print for debug porpuse
	printf("matrix [%d][%d] from process: 0\n", B_last-> id[0], B_last-> id[1]);
	for (i=1; i <=A->size_xd; i++)
	{
		for (j=1; j<= A->size_yd; j++)
			printf("[%d]",B_last->matrix[i][j]);
		printf("\n");
	}printf("\n");
	
	
	//Obtain the Subsequence
	int k=1, k_aux,
		**pos_final;
	char *z=NULL,
		*z_aux=NULL;
	
	pos_final = (int **)malloc(2*sizeof(int*));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final malloc\n");
		exit(ERROR);
	}
	
	pos_final[0] = (int *)calloc(2,sizeof(int));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final[0] calloc\n");
		exit(ERROR);
	}
	pos_final[1] = (int *)calloc(2,sizeof(int));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final[1] calloc\n");
		exit(ERROR);
	}
	
	if(A->size_x<A->size_y)
	{
		z = malloc((A->size_x+1)*sizeof(char));
		z_aux = malloc((A->size_x+1)*sizeof(char));
	}
	else
	{
		z = malloc((A->size_y+1)*sizeof(char));
		z_aux = malloc((A->size_y+1)*sizeof(char));
	}	
	if (z == NULL)
	{
		fprintf(stdout, "Error in z malloc\n");
		exit(ERROR);
	}
		if (z_aux == NULL)
	{
		fprintf(stdout, "Error in z_aux malloc\n");
		exit(ERROR);
	}
	
	//last position
	pos_final[1][0] = A->size_x;
	pos_final[1][1] = A->size_y;
	pos_final[0][0] = A->size_xd;
	pos_final[0][1] = A->size_yd;
	/*printf("[%d][%d] -> [%d][%d]\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1]);*/
	
	//first go_back
	go_back (A, B_last, z, &k ,pos_final);
	
	/*printf("[%d][%d] -> [%d][%d]\n\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1]);*/
	find_pos(A, A->iter, &x, &y);
	
	if (pos_final[0][0] == 0)
		x = x-1;
	if (pos_final[0][1] == 0)
		y = y-1;
	
	//finds the process that has the matrix wanted		
	iter_aux = A-> matrix_iter [x][y];
	send_id = A->matrix_dist[iter_aux];
	
	
	while (iter_aux > 1){
		//sends the position on the "c" matrix and the cordinates for the matrix_dist
		MPI_Send(&pos_final[1][0], 1, MPI_UNSIGNED_SHORT, send_id, x, MPI_COMM_WORLD);
		MPI_Send(&pos_final[1][1], 1, MPI_UNSIGNED_SHORT, send_id, y, MPI_COMM_WORLD);
		
		//receives the position on the "c" matrix and the cordinates for the matrix_dist
		MPI_Recv(&pos_final[1][0], 1,  MPI_UNSIGNED_SHORT, send_id, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		x = status->MPI_TAG;
		
		MPI_Recv(&pos_final[1][1], 1,  MPI_UNSIGNED_SHORT, send_id, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		y = status->MPI_TAG;
		
		//receives the string with the matches
		MPI_Recv(&k_aux, 1,  MPI_UNSIGNED_SHORT, send_id, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		for (i = 0;i < k_aux; i ++)
			MPI_Recv(&z_aux[i], 1,  MPI_CHAR, send_id, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		//print received for debug
		/*for (i = 0;i < k_aux; i ++)
			printf("%d -> %c from %d\n", i , z_aux[i], send_id);
		*/
		for (i = 0;i < k_aux; i ++)
		{
			z[k] = z_aux[i]; //adds the last matches to the vector
			k++; //increments the number of the matches
		}
		
		//finds the process that has the matrix wanted	
		iter_aux = A-> matrix_iter [x][y];
		send_id = A->matrix_dist[iter_aux];
	}
	
	pos_final[0][0] = x * A->size_xd -(A->size_xd -1);
	pos_final[0][1] = y * A->size_yd -(A->size_yd -1);
	pos_final[0][0] = pos_final[1][0] - pos_final[0][0] + 1;
	pos_final[0][1] = pos_final[1][1] - pos_final[0][1] + 1;
	
	/*printf("[%d][%d] -> [%d][%d]\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1]);*/
	if (iter_aux == 1)
		go_back (A, B, z, &k ,pos_final);
	
	/*printf("[%d][%d] -> [%d][%d]\n\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1]);	*/	
	z[k]='\0';	//end vector z
	
	
	printf("%d\n", k-1);
	for(i=k-1;i>0;i--)
		printf("%c", z[i]);
	printf("\n");
	
	jump_process(A, p);
	

}

/* slave_io(id, p, status, A)
 * 
 * Input:
 * 			id -> id of the process
 * 			p -> number of processes
 * 			status -> MPI_status
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			MPI_Status -> due to sends and receives
 * 
 * Description:
 * 			This function only does calculation. It receives matrix info from master and returns that information to it.
 * 
 * Notes:
 * 			This function is only run by the slaves (process id!=0)			
 * 
 */
void slave_io(int id, int p, MPI_Status *status, matrix_info *A)
{
	int i, j;
	matrix_list *B, *B_last, *B_aux;
	unsigned short *info1, *info2;
	
	B = initialize_matrix_list(A); //first structure B
	B_last = B; //first structure is also the last one
	
	while(1) //Runs until master sends info to stop
	{
		//receives upper line from master
		for (j=0; j <= A->size_yd; j++)
			MPI_Recv(&B_last->matrix[0][j], 1, MPI_UNSIGNED_SHORT, 0 , MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		//receives x cordinate from master thrue info on TAG	
		B_last-> id[0] = status->MPI_TAG; 
		
		//receives left collum from master
		for (j=0; j <= A->size_xd; j++)
			MPI_Recv(&B_last->matrix[j][0], 1, MPI_UNSIGNED_SHORT, 0 , MPI_ANY_TAG, MPI_COMM_WORLD, status);
		
		//receives y cordinate from master thrue info on TAG		
		B_last-> id[1] = status->MPI_TAG;
		
		//receives info from master to stop this operation and jump to the next
		if (status->MPI_TAG == JUMPTAG) 
			break;
		
		//calc the matrix indicated by the master
		matrix_calc(A, B_last);
		
		//matrix calculated print for debug porpuse
		printf("matrix [%d][%d] from process: %d\n", B_last-> id[0], B_last-> id[1],id);
		for (i=1; i <=A->size_xd; i++)
		{
			for (j=1; j<= A->size_yd; j++)
				printf("[%d]",B_last->matrix[i][j]);
			printf("\n");
		}printf("\n");
		
		//Send results to master
		for (j=0; j <= A->size_yd; j++)
			MPI_Send(&B_last->matrix[A->size_xd][j], 1, MPI_UNSIGNED_SHORT, 0, id, MPI_COMM_WORLD);
		for (j=0; j <= A->size_xd; j++)
			MPI_Send(&B_last->matrix[j][A->size_yd], 1, MPI_UNSIGNED_SHORT, 0, id, MPI_COMM_WORLD);
		
		//creates new structure, point the last stucture to the new one
		B_aux = initialize_matrix_list(A);
		B_last->next = B_aux;
		B_last = B_aux;
	}
	
	//Runs Subsequence
	int k=0,x, y,
		**pos_final;
	char *z=NULL;
	
	pos_final = (int **)malloc(2*sizeof(int*));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final malloc\n");
		exit(ERROR);
	}
	
	pos_final[0] = (int *)calloc(2,sizeof(int));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final[0] calloc\n");
		exit(ERROR);
	}
	pos_final[1] = (int *)calloc(2,sizeof(int));
	if (pos_final == NULL)
	{
		fprintf(stdout, "Error in pos_final[1] calloc\n");
		exit(ERROR);
	}
	
	//Alocs z matrix where each string is going to be stored
	if(A->size_xd<A->size_yd)
		z = malloc((A->size_xd+1)*sizeof(char));
	else
		z = malloc((A->size_yd+1)*sizeof(char));
	if (z == NULL)
	{
		fprintf(stdout, "Error in z malloc\n");
		exit(ERROR);
	}

	while(1) //Runs until master sends info to stop
	{
		k=0; //no match
		
		MPI_Recv(&pos_final[1][0], 1, MPI_UNSIGNED_SHORT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		x = status->MPI_TAG;
		
		MPI_Recv(&pos_final[1][1], 1, MPI_UNSIGNED_SHORT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		y = status->MPI_TAG;	
			
		//receives info from master to stop this operation and jump to the next
		if (status->MPI_TAG == JUMPTAG) 
			break;
		
		// from x and y (cordinates in matrix_iter) finds the real position in matrix ([1][1])
		pos_final[0][0] = x * A->size_xd -(A->size_xd -1);
		pos_final[0][1] = y * A->size_yd -(A->size_yd -1);
		
		//from the position in the "c" matrix finds the real position to start in matrix
		pos_final[0][0] = pos_final[1][0] - pos_final[0][0] + 1;
		pos_final[0][1] = pos_final[1][1] - pos_final[0][1] + 1;
		/*printf("[%d][%d] -> [%d][%d] in %d\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1], id);*/
		B_aux = B;
		
		while(B_aux -> next != NULL)
		{
			if ((B_aux -> id[0] == x) && (B_aux -> id[1] == y)) break;
			B_aux = B_aux -> next;
		}
		
		//runs he matrix backwards
		go_back (A, B_aux, z, &k ,pos_final);
		
		if (pos_final[0][0] == 0)
			x = x-1;
		if (pos_final[0][1] == 0)
			y = y-1;
		
		// sends the positions and the matrix
		MPI_Send(&pos_final[1][0], 1,  MPI_UNSIGNED_SHORT,0, x, MPI_COMM_WORLD);
		MPI_Send(&pos_final[1][1], 1,  MPI_UNSIGNED_SHORT, 0, y, MPI_COMM_WORLD);
		MPI_Send(&k, 1,  MPI_UNSIGNED_SHORT, 0, id, MPI_COMM_WORLD);
		for (i = 0;i < k; i ++)
			MPI_Send(&z[i], 1,  MPI_CHAR, 0, id, MPI_COMM_WORLD);
		/*printf("[%d][%d] -> [%d][%d]\n\n", pos_final[1][0], pos_final[1][1], pos_final[0][0], pos_final[0][1]);	*/
	}

}

/* jump_process(A, p)
 * 
 * Input:
 *			A -> structure matrix_info
 * 			p -> number of processes
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			Nothing
 * 
 * Description:
 * 			This function sends trash message with a JUMPTAG to slave that makes it jump to the next task
 * 
 * Notes:
 * 			This function is only run by the master (process id==0)			
 * 
 */
void jump_process(matrix_info *A, int p)
{
	int j, send_id;
	for (send_id=1; send_id < p; send_id ++)
	{
		for (j=0; j <= A->size_yd; j++)
			MPI_Send(&j, 1, MPI_UNSIGNED_SHORT, send_id, JUMPTAG, MPI_COMM_WORLD);
		for (j=0; j <= A->size_xd; j++)
			MPI_Send(&j, 1, MPI_UNSIGNED_SHORT, send_id, JUMPTAG, MPI_COMM_WORLD);
	}
}

/* initialize_matrix_info(argv)
 * 
 * Input:
 * 			argv -> function call arguments, includes the name of the file to be used
 * 
 * Output:
 * 			structure matrix_info initialized
 * 
 * Changes:
 * 			Nothing
 * 
 * Description:
 * 			This function allocs memory for the structure and calls function to initialize different parts of the structure.
 * 
 * Notes:		
 * 
 */
matrix_info* initialize_matrix_info(char **argv)
{
	int i;
	matrix_info *A;
	
	A = (matrix_info*)malloc(sizeof(matrix_info));
	if (A  == NULL)
	{
		fprintf(stdout, "Error in A malloc\n");
		exit(ERROR);
	}

	//values initialization
	A -> size_x = 0;
	A -> size_y = 0;
	A -> size_xd= 0;
	A -> size_yd= 0;
	A -> x=NULL;
	A -> y=NULL;
	A -> matrix_dist = NULL;
	
	//reads the files and changes A based on it
	read_file(argv, A);
	
	//finds the best way to divide the matrix in small ones and changes A based on it
	divide_by_prime(A);
	
	//number of iterations
	A -> iter = ((A->size_x)/(A->size_xd))*((A->size_y)/(A->size_yd));
	
	//
	A-> matrix_dist = (unsigned short *)calloc((A->iter +1), sizeof(unsigned short));
	if (A-> matrix_dist  == NULL)
	{
		fprintf(stdout, "Error in A-> matrix_dist calloc\n");
		exit(ERROR);
	}
		
	//Alocation and initialization of A->matrix_iter
	A-> matrix_iter = (unsigned short **)calloc((A->size_x/A->size_xd +2), sizeof(unsigned short*));
	if (A-> matrix_iter  == NULL)
	{
		fprintf(stdout, "Error in A-> matrix_dist malloc\n");
		exit(ERROR);
	}
	
	for(i = 0; i < A->size_x/A->size_xd +1 ; i++)
	{
		A-> matrix_iter [i] = (unsigned short *)calloc((A->size_y/A->size_yd +2), sizeof(unsigned short));
		if (A-> matrix_iter[i] == NULL)
		{
		fprintf(stdout, "Error in A-> matrix_dist[i] calloc\n");
		exit(ERROR);
		}	
	}
	
	//defines the iteration to be made in each small matrix
	matrix_iter(A);
	
	return A;
}

/* read_file(argv, A)
 * 
 * Input:
 * 			argv -> function call arguments, includes the name of the file to be read
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			A->size_x (information from file)
 * 			A->size_y (information from file)
 * 			A->x (information from file - allocs memmory for vector)
 * 			A->y (information from file - allocs memmory for vector)
 * 
 * Description:
 * 			This function reads the file with the info and updates structure A
 * 
 * Notes:		
 * 			Same procedure from the serial impletation
 */
void read_file(char **argv, matrix_info *A)
{
	FILE * f;
	char *buffer;
	int i, j, a, b;
	int size_xx, size_yy;
	
	f = fopen(argv[1], "r");
	if (f == NULL)
	{
		printf ("Error opening file\n");
		exit(ERROR);						
	}
	
	buffer = malloc(SIZE_BUFFER*sizeof(char));			
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer malloc\n");
		exit(ERROR);
	}
	
	//Reading input file
	fgets(buffer, SIZE_BUFFER_0, f);
	sscanf(buffer,"%d %d", &a, &b);
	
	(*A).size_x = max(a,b);
	(*A).size_y = min(a,b);
	
	size_xx = ((*A).size_x+ SIZE_BUFFER)*sizeof(char);
	size_yy = ((*A).size_y+ SIZE_BUFFER)*sizeof(char);
		
	//buffer reallocated to the size (plus some margin) of the biggest line
	
	buffer=realloc(buffer, max(size_xx, size_yy));
		
	if (buffer == NULL)
	{
		fprintf(stdout, "Error in buffer realloc\n");
		exit(ERROR);
	}
						
	(*A).x = malloc((max((*A).size_x, (*A).size_y)+1)*sizeof(char));  
	if ((*A).x == NULL)
	{
		fprintf(stdout, "Error in x malloc\n");
		exit(ERROR);
	}
	(*A).y = malloc((min((*A).size_x, (*A).size_y)+1)*sizeof(char));
	if ((*A).y == NULL)
	{
		fprintf(stdout, "Error in y malloc\n");
		exit(ERROR);
	}
	
	if(a>=b)
	{
		fgets(buffer, size_xx, f);
		sscanf(buffer, "%s\n", (*A).x);
		fgets(buffer, size_yy,  f);
		sscanf(buffer, "%s\n", (*A).y);
	}
	else
	{
		fgets(buffer, size_xx, f);
		sscanf(buffer, "%s\n", (*A).y);
		fgets(buffer, size_yy,  f);
		sscanf(buffer, "%s\n", (*A).x);
	}

	//printf("x= %s\ny= %s\n", (*A).x, (*A).y);
	
}

/* divide_by_prime(A)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			A->size_xd
 * 			A->size_yd
 * 
 * Description:
 * 			This function finds the prime numbers for witch the number of lines and collums can be divided.
 * 			This assures that the matrix is divided in equal peaces
 * 
 * Notes:		
 * 			Doesn't work if the proposed run number is prime
 * 
 */
void divide_by_prime(matrix_info *A)
{
	int i,
		prime_num[10]={2,3,5,7,11,13,17,19,23,29};
	
	//lines
	for (i=0; i<10; i++)
	{
		if (((*A).size_x % prime_num[i]) == 0) //checks if the rest is zero
		{
			(*A).size_xd = prime_num[i];
			break;
		}
	}
	
	//collums
	for (i=0; i<10; i++)
	{
		if (((*A).size_y % prime_num[i]) == 0)
		{
			(*A).size_yd = prime_num[i];
			break;
		}
	}

}

/* matrix_iter(A)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			A->matrix_iter
 * 
 * Description:
 * 			This function runs the matrix in a wave front manner and gives iteration numbers to the matrix positions
 * 
 * Notes:		
 * 
 */
void matrix_iter(matrix_info *A)
{
	int size_x, size_y;
	size_x = A->size_x/A->size_xd;
	size_y = A->size_y/A->size_yd;
	int i, j, h, iter = 1;
	
	printf("size_x = %d, size_y= %d\n", size_x, size_y);
	printf("A->size_x = %d, A->size_y= %d\n", A->size_x,A->size_y);
	printf("A->size_xd = %d, A->size_yd= %d\n", A->size_xd,A->size_yd);
	printf("1\n");
	if(size_x==size_y){
		printf("2\n");
		for(h=1; h<=size_y; h++)		
			for(i=1, j=h; j>0; i++, j--)
			{
				A-> matrix_iter [i][j] = iter;
				iter ++;
			}printf("3\n");
		for(h=size_y+1; h<size_y+size_x; h++)
			for(i=h-size_y+1, j=size_x; i<=size_x; j--, i++)
			{
				A-> matrix_iter [i][j] = iter;
				iter ++;
			}printf("4\n");
	}
	else
	{printf("5\n");
		for(h=1; h<=size_y; h++)
			for(i=1, j=h; j>0; i++, j--)
			{printf("i = %d, h = %d\n",i, h);
				A-> matrix_iter [i][j] = iter;
				iter ++;
			}printf("6\n");
		for(h=size_y+1; h<=size_x+size_y; h++)
			for(i=h-size_y+1, j=size_y; j>0 && i<=size_x; i++, j--)
			{
				A-> matrix_iter [i][j] = iter;
				iter ++;
			}printf("7\n");
	}
	printf("8\n");
}


/* find_pos(A, iter, x, y)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 			iter -> number of the iteration
 * 			x, y -> cordinate for the matrix_iter
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			x, y
 * 
 * Description:
 * 			This function runs the matrix to find the cordinates for the given iteration
 * 
 * Notes:		
 * 
 */
void find_pos(matrix_info *A, int iter, int *x, int *y)
{
	for((*x)=1;(*x) <= A->size_x/A->size_xd ;(*x)++)
	{
		for((*y)=1;(*y)<= A->size_y/A->size_yd ;(*y)++)
		{
			if ((A-> matrix_iter [*x][*y]) == iter) return;
		}
	}
	
}

/* initialize_matrix_list(matrix_info *A)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 
 * Output:
 * 			stucture matrix_list initialized
 * 
 * Changes:
 * 			Nothing
 * 
 * Description:
 * 			This function allocs memory for the structure matrix_list
 * 
 * Notes:
 * 			The initialization is made for the first iteration
 * 
 */
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
	
	//cordinates for the first iteranion
	B-> id[0] = 1; 
	B-> id[1] = 1;	
	B->next = NULL;

	return B;
}

/* matrix_calc(A, B)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 			B -> structure matrix_list (pointer to the last one)
 * 
 * Output:
 * 			None
 * 
 * Changes:
 * 			B->matrix
 * 
 * Description:
 * 			LCS matrix calculator 
 * 
 * Notes:
 * 			Same implemation from the serial 
 * 
 */
void matrix_calc(matrix_info *A, matrix_list *B)
{
	int i, j,
		id[2];
	
	id[0] = 1 + (B-> id[0]-1)*A->size_xd;
	id[1] = 1 + (B-> id[1]-1)*A->size_yd; 
	
	for(i = 1;i <= A->size_xd;i++)
	{
		for(j =1 ;j<= A->size_yd;j++)
		{
			if (A->x[id[0]+ i-2]==A->y[id[1] + j-2]) 				//	computation of matrix c
				B->matrix[i][j] = B->matrix[i-1][j-1]+ cost(i); //match
			else 
				B->matrix[i][j] = max(B->matrix[i][j-1],B->matrix[i-1][j]);
		}
	}
}

/* cost(x)
 * 
 * Input:
 * 			x -> iteration number
 * 
 * Output:
 * 			A short number with the value 1
 * 
 * Changes:
 * 			Nothing
 * 
 * Description:
 * 			Slow operations to make the LCS computation take more time
 * 
 * Notes:
 * 			Function by the professor
 * 
 */
short cost(int x)
{
	int i, n_iter = 20;
	double dcost = 0;
	for(i = 0; i < n_iter; i++) dcost += pow(sin((double) x),2) + pow(cos((double) x),2);
	return (short) (dcost / n_iter + 0.1);
}

/* generate_matrix_info( B, size, side)
 * 
 * Input:
 * 			B -> structure matrix_list (pointer to the last one)
 * 			size -> number of lines/collums
 * 			side -> last line/collum (1 for last line, 2 for last collum)
 * 
 * Output:
 * 			vector with the line/collum asked
 * 
 * Changes:
 * 			Nothing
 * 
 * Description:
 * 			Returns the last line/collum of the small matrix
 * 
 * Notes:
 * 
 */
unsigned short *generate_matrix_info(matrix_list *B, int size, int size_ref, int side)
{
	unsigned short *info;
	int i;
	
	info = (unsigned short *)calloc((size+1), sizeof(unsigned short));
	if (info  == NULL)
	{
		printf( "Error in info[] malloc\n");
		exit(ERROR);
	}
	
	for (i = 0; i <= size; i++)
	{
		if(side == 1) info[i] = B->matrix[size_ref][i];
		if(side == 2) info[i] = B->matrix[i][size_ref];
	} 
	
	return info;
}

/* initialize_info(A, B)
 * 
 * Input:
 * 			A -> structure matrix_info
 * 			B -> structure matrix_list (pointer to the first iteration)
 * 
 * Output:
 * 			3 dimensional matrix that is going to hold the information received from the slaves
 * 
 * Changes:
 * 			None
 * 
 * Description:
 * 			Allocates memory
 * 
 * Notes:
 * 			The last alloc (for the *unsigned short) is made in function generate_matrix_info
 * 
 */
unsigned short ***initialize_info(matrix_info *A, matrix_list *B)
{
	int i;
	unsigned short *** info;
	
	info = (unsigned short***) malloc((A->iter +2)*sizeof(unsigned short**));
	if (info  == NULL)
	{
		fprintf(stdout, "Error in info[][][] malloc\n");
		exit(ERROR);
	}
	
		for (i=0; i <= A->iter; i++)
	{
		info[i] = (unsigned short**) malloc((2)*sizeof(unsigned short*));
		if (info[i]  == NULL)
		{
			fprintf(stdout, "Error in info[%d] malloc\n", i);
			exit(ERROR);
		}
	}

	return info;
}

void go_back (matrix_info *A, matrix_list *B, char *z, int *k ,int **pos_final)
{
	int i, j, i2, j2;
	
	i = pos_final[0][0];
	j= pos_final[0][1];
	i2 = pos_final[1][0];
	j2= pos_final[1][1];
	
	while((i>0)&&(j>0))
	{
			
		if (A->x[i2-1]==A->y[j2-1]) //match
		{	
			z[*k]=A->x[i2-1];
			/*printf("%d -> %c; ", *k, A->x[i2-1]);*/
			i=i-1;
			i2=i2-1;
			j=j-1;
			j2=j2-1;
			
			*k+=1;
		}
			
		else
		{							//	computation of LCS from matrix
			
			if(B->matrix[i-1][j]>B->matrix[i][j-1])
			{
				i=i-1;	//up
				i2=i2-1;
			}
			else
			{
				j=j-1;	//left
				j2=j2-1;
			}	
		}
	}
	/*printf("\n");*/
	pos_final[0][0] = i;
	pos_final[0][1] = j;
	pos_final[1][0] = i2;
	pos_final[1][1] = j2;
}
