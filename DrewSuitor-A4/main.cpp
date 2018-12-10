#include <cstdlib>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include "mpi.h"

#define MPI_Type MPI_INT				//mpi type is int	
#define inPrefix "./inputs/"			//input file location
#define outPrefix "./outputs/"			//output file location

using namespace std;

void genRandomList(int n){
	int rank, buf, tmp;
	rank = MPI::COMM_WORLD.Get_rank();
	int random[n];
	srand((unsigned)time(NULL));
	for(int i = 0; i < n; i++){
		random[i] = rand() % 1000000;
	}
	
	FILE *outfile;
	char outname[20];
	
	outname[0] = 'r';
	outname[1] = 'a';
	outname[2] = 'n';
	outname[3] = 'd';
	outname[4] = 'o';
	outname[5] = 'm';
	outname[6] = '-';
	outname[7] = ((rank % 100)/10)+48;
	outname[8] = (rank % 10)+48;
	outname[9] = '.';
	outname[10] = 't';
	outname[11] = 'x';
	outname[12] = 't';
	outname[13] = '\0';
	
	outfile = fopen(outname, "w");
	
	fprintf(outfile, "%d\n", 250000);
	fprintf(outfile, "%d\n", 4);
	
	for(int i = 0; i < n; i++){
		buf = random[i];
		fprintf(outfile, "%d\n", buf);
	}
	
	tmp = fclose(outfile);
	
}

void fixData(int data[], int m, int n){
	int j;
	int k;
	int* b;
	
	b = data - 1;
	j = m;
	k = m*2;
	
	while(k <= n){
		if(k < n && (b[k] < b[k+1])){
			k++;
		}
		if(b[j] < b[k]){
			int temp = b[j];
			b[j] = b[k];
			b[k] = temp;
		}
		j = k;
		k *= 2;
	}
}

void heapsort(int data[], int n){
	int *b;
	b = data - 1;
	
	for(int j = n/2; j > 0; j--){
		fixData(data, j, n);
	}
	
	for(int j = n-1; j > 0; j--){
		int temp = b[1];
		b[1] = b[j+1];
		b[j+1] = temp;
		fixData(data, 1, j);
	}
}

void printArr(int data[], int n){
	for(int i = 0; i < n; i++){
		cout << data[i] << ", ";
	}
	cout << endl;
}

//most of the work will be done here
void sort(int data[], int n){
	int rank;
	int p;
	int resultSize;
	int i, j, temp, l, r, left, right;
	int *scounts, *displs, *sdispls, *recvtype, *bloc, *bsize, 
	*counts, *send, *receive, *result, *sendBuff, *recvcounts;
	
	p = MPI::COMM_WORLD.Get_size();		//  Get the number of processes.
	rank = MPI::COMM_WORLD.Get_rank();	//  Get the individual process ID.
	
	scounts = (int*) malloc(p*sizeof(int)+1);
	displs = (int*) malloc(p*sizeof(int)+1);
	sdispls = (int*) malloc(p*sizeof(int)+1);
	recvtype = (int*) malloc(p*sizeof(int)+1);
	bloc = (int*) malloc(p*sizeof(int)+1);
	bsize = (int*) malloc(p*sizeof(int)+1);
	counts = (int*) malloc(p*sizeof(int)+1);
	send = (int*) malloc(p*sizeof(int)+1);
	receive = (int*) malloc(p*sizeof(int)+1);
	result = (int*) malloc((2*n)*sizeof(int)+1);
	sendBuff = (int*) malloc(p*sizeof(int)+1);
	recvcounts = (int*) malloc((p*p)*sizeof(int)+1);
	
	heapsort(data, n);
	
	for(i = 0; i < p; i++){
		sendBuff[i] = data[i*n/p];
		recvtype[i] = i*p;
		displs[i] = p;
	}
	
	MPI_Gatherv(sendBuff, p, MPI_Type, recvcounts, displs, recvtype, MPI_INT, 0, MPI_COMM_WORLD);
	
	if(rank == 0){
		heapsort(recvcounts, p*p);
		for(i = 0; i < p; i++){
			sendBuff[i] = recvcounts[i*p];
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(sendBuff, p, MPI_Type, 0, MPI_COMM_WORLD);
	
	j = 0;
	bloc[0] = 0;
	
	for(i = 1; i < p; i++){
		while(data[j] < sendBuff[i]){
			j++;
		}
		bloc[i] = j;
	}
	
	for(i = 0; i < p-1; i++){
		bsize[i] = bloc[i+1]-bloc[i];
	}
	
	bsize[p-1] = n - bloc[p-1];
	
	for(i = 0; i < p; i++){
		scounts[i] = 1;
		sdispls[i] = i;
		displs[i] = 1;
		recvtype[i] = i;
	}
	
	MPI_Alltoallv(bsize, scounts, sdispls, MPI_INT, counts, displs, recvtype, MPI_Type, MPI_COMM_WORLD);
	
	resultSize = 1;
	
	for(i = 0; i < p; i++){
		scounts[i] = bsize[i];
		sdispls[i] = bloc[i];
		displs[i] = counts[i];
		recvtype[i] = resultSize - 1;
		resultSize += counts[i];
	}
	resultSize--;
	
	MPI_Alltoallv(data, scounts, sdispls, MPI_Type, result, displs, recvtype, MPI_Type, MPI_COMM_WORLD);
	
	heapsort(result, resultSize);
	//printArr(result, resultSize);
	
	for(i = 0; i < p; i++){
		displs[i] = 1;
		recvtype[i] = i;
	}
	
	MPI_Allgatherv(&resultSize, 1, MPI_INT, counts, displs, recvtype, MPI_INT, MPI_COMM_WORLD);
	
	left = 0;
	for(i = 0; i < rank; i++){
		left += counts[i];
	}
	right = left + counts[rank] - 1;
	
	for(i = 0; i < p; i++){
		l = i * n;
		r = ((i+1) * n) - 1;
		send[i] = 0;
		if((left <= l) && (l <= right) && (right <= r)){
			send[i] = right-l+1;
		}
		if((l <= left) && (left <= r) && (r <= right)){
			send[i] = r-left+1;
		}
		if((l <= left) && (right <= r)){
			send[i] = right-left+1;
		}
		if((left <= l) && (r <= right)){
			send[i] = r-l+1;
		}
	}
	
	for(i = 0; i < p; i++){
		scounts[i] = 1;
		sdispls[i] = i;
		displs[i] = 1;
		recvtype[i] = i;
	}
	
	MPI_Alltoallv(send, scounts, sdispls, MPI_INT, receive, displs, recvtype, MPI_Type, MPI_COMM_WORLD);
	
	l = 0; 
	r = 0;
	
	for(i = 0; i < p; i++){
		scounts[i] = send[i];
		sdispls[i] = l;
		l += scounts[i];
		displs[i] = receive[i];
		recvtype[i] = r;
		r += displs[i];
	}
	
	MPI_Alltoallv(result, scounts, sdispls, MPI_Type, data, displs, recvtype, MPI_Type, MPI_COMM_WORLD);
	//printArr(data, n);
}

int main(int argc, char *argv[]){
	int rank;
	int p, p2;
	int *data;
	double wtime;
	FILE *infile, *outfile;
	char inName[100] = inPrefix;
	char outName[100] = outPrefix;
	char infPostfix[20], outfPostfix[20];
	int n, buf, tmp, i, startTime, stopTime;
	
	MPI::Init(argc, argv); 				//Initialize MPI.
	startTime = MPI_Wtime();			//Start timer.
	p = MPI::COMM_WORLD.Get_size(); 	//Get the number of processes.
	rank = MPI::COMM_WORLD.Get_rank();	//Get the individual process ID.
	
	cout << "starting: " << rank << endl;
	
	//yeah its janky, but it works
	infPostfix[0] = 'i';
	infPostfix[1] = 'n';
	infPostfix[2] = 'p';
	infPostfix[3] = 'u';
	infPostfix[4] = 't';
	infPostfix[5] = '-';
	infPostfix[6] = ((rank % 100)/10)+48;
	infPostfix[7] = (rank % 10)+48;
	infPostfix[8] = '.';
	infPostfix[9] = 't';
	infPostfix[10] = 'x';
	infPostfix[11] = 't';
	infPostfix[12] = '\0';
	
	strcat(inName, infPostfix);
	infile = fopen(inName, "r");
	fscanf(infile, "%d\n", &n);
	fscanf(infile, "%d\n", &p2);
	
	n = n/p2;
	data = ((int*) malloc(n*sizeof(int)+1));
	
	for(i = 0; i < n; i++){
		fscanf(infile, "%d", &buf);
		data[i] = buf;
	}
	
	tmp = fclose(infile);
	
	sort(data, n);
	
	outfPostfix[0] = 'o';
	outfPostfix[1] = 'u';
	outfPostfix[2] = 't';
	outfPostfix[3] = 'p';
	outfPostfix[4] = 'u';
	outfPostfix[5] = 't';
	outfPostfix[6] = '-';
	outfPostfix[7] = ((rank % 100)/10)+48;
	outfPostfix[8] = (rank % 10)+48;
	outfPostfix[9] = '.';
	outfPostfix[10] = 't';
	outfPostfix[11] = 'x';
	outfPostfix[12] = 't';
	outfPostfix[13] = '\0';
	
	strcat(outName, outfPostfix);
	outfile = fopen(outName, "w");
	
	for(i = 0; i < n; i++){
		buf = data[i];
		fprintf(outfile, "%d\n", buf);
	}
	
	tmp = fclose(outfile);
	
	stopTime = MPI_Wtime();
	
	if(rank == 0){
		cout << "wall clock runtime: " << (stopTime - startTime) << endl;
	}
	
	//genRandomList(250000);
	
	MPI::Finalize();
	return 0;
}











//
