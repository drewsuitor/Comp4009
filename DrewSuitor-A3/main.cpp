
# include <cstdlib>
# include <iostream>
# include <fstream>
# include "mpi.h"
using namespace std;

void printArr(int** arr, int N){
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			cout << arr[i][j];
		}
		cout << endl;
	}
}

int** nextStep(int** arr, int N){
	int** arr2 = new int*[N];
	for(int i = 0; i < N; i++){
		arr2[i] = new int[N];
		for(int j = 0; j < N; j++){
			arr2[i][j] = arr[i][j];
		}
	}
	return arr2;
}

int malloc2D(int ***array, int n, int m) {
	int i;
    
	int *p = (int*)malloc(n*m*sizeof(int));
	if (!p) return -1;

	(*array) = (int**)malloc(n*sizeof(int*));
	if (!(*array)) {
		free(p);
		return -1;
	}

	for (i=0; i<n; i++)
		(*array)[i] = &(p[i*m]);

	return 0;
}

int free2D(int ***array) {
	free(&((*array)[0][0]));

	free(*array);

	return 0;
}

int main(int argc, char *argv[]){
	
	int N = atoi(argv[1]);
	int p1 = atoi(argv[2]);
	int p2 = atoi(argv[3]);
	int k = atoi(argv[4]);
	int m = atoi(argv[5]);
	char* filename = argv[6];
	
	int gridsize = N;
	int procGridSize = p1;
	int **global, **local, **localNext, **procGrid;
	int rank, p, i, j;
	double wtime;
  
	MPI::Init(argc, argv); //  Initialize MPI.
	p = MPI::COMM_WORLD.Get_size(); //  Get the number of processes.
	rank = MPI::COMM_WORLD.Get_rank(); //  Get the individual process ID.
	
	//cout << "  Process " << rank << " says 'Hello, world!'\n";

	malloc2D(&procGrid, procGridSize, procGridSize);

	int a = 0;
	for (int i = 0; i < procGridSize; i++){
		for (int j = 0; j < procGridSize; j++){
			procGrid[i][j] = a;
			a++;
		}
	}
	
	if (rank == 0){
		
		wtime = MPI::Wtime();

		malloc2D(&global, gridsize, gridsize);
		
		// READING BOARD //
		
		FILE* myfile = fopen(filename, "r");
		char* line = new char[N+5];
		
		for(int i = 0; i < N; i++){
			fgets(line, N+5, myfile);
			for(int j = 0; j < N; j++){
				global[i][j] = line[j] - '0'; //because int
			}
		}
		//printArr(global, N);
	}
	
	malloc2D(&local, N/procGridSize, N/procGridSize);
	malloc2D(&localNext, N/procGridSize, N/procGridSize);
	int sizes[2] = {gridsize, gridsize};
	int subsizes[2] = {gridsize/procGridSize, gridsize/procGridSize};
	int starts[2] = {0, 0};
	MPI_Datatype type, subarrtype;
	MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &type);
	MPI_Type_create_resized(type, 0, gridsize/procGridSize*sizeof(int), &subarrtype);
	MPI_Type_commit(&subarrtype);
	
	int *globalptr=NULL;
	if(rank==0){
		globalptr = &(global[0][0]);
	}
	
	int sendcounts[procGridSize*procGridSize];
	int displs[procGridSize*procGridSize];
	
	if(rank==0){
		for (i=0; i<procGridSize*procGridSize; i++)
			sendcounts[i] = 1;
		int disp = 0;
		for (i=0; i<procGridSize; i++) {
			for (j=0; j<procGridSize; j++) {
				displs[i*procGridSize+j] = disp;
				disp += 1;
			}
			disp += ((gridsize/procGridSize)-1)*procGridSize;
		}
	}
	
	MPI_Scatterv(globalptr, sendcounts, displs, subarrtype, &(local[0][0]), (gridsize*gridsize)/(procGridSize*procGridSize), MPI_INT, 0, MPI_COMM_WORLD);
	
	for (int k=0; k<p; k++) {
		if (rank == k) {
			printf("Local process on rank %d is:\n", rank);
			for (i=0; i<gridsize/procGridSize; i++) {
				putchar('|');
				for (j=0; j<gridsize/procGridSize; j++) {
					printf("%2d ", local[i][j]);
				}
				printf("|\n");
				}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
	int localSize = gridsize/procGridSize;
	int buffSize = localSize + localSize + 1;
	int *buff = new int[buffSize];
	int *recv = new int[buffSize];
	int sendsize[p];
	int disp[p];
	int rdisp[p];
	
	if (rank == 0){
		int sendsize[p] = {0, localSize, localSize, 1};
		int disp[p] = {0, 0, localSize, buffSize-1};
		
		for (int i = 0; i < localSize; i++){
			buff[i] = local[i][localSize-1];
		}
		
		int j = localSize;
		for (int i = 0; i < localSize; i++){
			buff[j] = local[localSize-1][i];
			j++;
		}
		
		buff[buffSize] = local[localSize-1][localSize-1];
		cout << rank << ": ";
		for (int i = 0; i < (buffSize); i++){
			cout << buff[i];
		}
		cout << endl;
		
		MPI_Alltoallv(buff, sendsize, disp, MPI_INT, recv, sendsize, disp, MPI_INT, MPI_COMM_WORLD);
		
		cout << rank << " received: ";
		for(int i = 0; i < buffSize; i++){
			cout << recv[i];
		}
		cout << endl;
	}
	
	if (rank == 1){
		int sendsize[p] = {localSize, 0, 1, localSize};
		int disp[p] = {0, localSize, buffSize-1, localSize};
			
		for (int i = 0; i < localSize; i++){
			buff[i] = local[i][0];
		}
		
		int j = localSize;
		for (int i = 0; i < localSize; i++){
			buff[j] = local[localSize-1][i];
			j++;
		}
		
		buff[buffSize-1] = local[localSize-1][0];
		cout << rank << ": ";
		for (int i = 0; i < (buffSize); i++){
			cout << buff[i];
		}
		cout << endl;
		
		MPI_Alltoallv(buff, sendsize, disp, MPI_INT, recv, sendsize, disp, MPI_INT, MPI_COMM_WORLD);
		
		cout << rank << " received: ";
		for(int i = 0; i < buffSize; i++){
			cout << recv[i];
		}
		cout << endl;
	}
	
	if (rank == 2){
		int sendsize[p] = {localSize, 1, 0, localSize};
		int disp[p] = {localSize, buffSize-1, 0, 0};
		
		for (int i = 0; i < localSize; i++){
			buff[i] = local[i][localSize-1];
		}
		
		int j = localSize;
		for (int i = 0; i < localSize; i++){
			buff[j] = local[0][i];
			j++;
		}
		
		buff[buffSize-1] = local[0][localSize-1];
		cout << rank << ": ";
		for (int i = 0; i < (buffSize); i++){
			cout << buff[i];
		}
		cout << endl;
		
		MPI_Alltoallv(buff, sendsize, disp, MPI_INT, recv, sendsize, disp, MPI_INT, MPI_COMM_WORLD);
		
		cout << rank << " received: ";
		for(int i = 0; i < buffSize; i++){
			cout << recv[i];
		}
		cout << endl;
	}
	
	if (rank == 3){
		int sendsize[p] = {1, localSize, localSize, 0};
		int disp[p] = {buffSize-1, localSize, 0, 0}; 
		
		for (int i = 0; i < localSize; i++){
			buff[i] = local[i][0];
		}
		
		int j = localSize;
		for (int i = 0; i < localSize; i++){
			buff[j] = local[0][i];
			j++;
		}
		
		buff[buffSize-1] = local[0][0];
		cout << rank << ": ";
		for (int i = 0; i < (buffSize); i++){
			cout << buff[i];
		}
		cout << endl;
		
		MPI_Alltoallv(buff, sendsize, disp, MPI_INT, recv, sendsize, disp, MPI_INT, MPI_COMM_WORLD);
		
		cout << rank << " received: ";
		for(int i = 0; i < buffSize; i++){
			cout << recv[i];
		}
		cout << endl;
	}
	
	
	
	

//  Process 0 says goodbye.
	if (rank == 0){
		wtime = MPI::Wtime() - wtime;
		cout << "  Elapsed wall clock time = " << wtime << " seconds.\n";
	}
	
//  Terminate MPI.
	MPI::Finalize();
	return 0;
}













//
