
#include <cstdlib>
#include <iostream>
#include <cilk/cilk.h>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include "hwtimer.h"
using namespace std;

//convertz is a function which takes as an input i, the position in the array,
//and references to x and y. it then calculates the position x and y based on
//the position of i in z-order.
void convertz(int i, int& x, int& y){
	x = 0;
	y = 0;

	for(int j = 0; j < 32; j += 2){
		int temp_x = 1 << j;
		int temp_y = 1 << (j+1);
		int nx = i & temp_x;
		int ny = i & temp_y;
		int x_bit = nx >> j;
		int y_bit = ny >> (j+1);

		x += x_bit*(int)pow((double)2, j/2);
		y += y_bit*(int)pow((double)2, j/2);
	}
}

//MultA takes 3 array pointers of ints and one int for the length of the sides
//of a squar matrix.
//The first array is where the multiplication of the second and third array
//will go.
void MultA(int* C, int* A, int* B, int n){
	//base case
	if(n == 2){
		C[0] += A[0] * B[0] + A[1] * B[2];
		C[1] += A[0] * B[1] + A[1] * B[3];
		C[2] += A[2] * B[0] + A[3] * B[2];
		C[3] += A[2] * B[1] + A[3] * B[3];
	}
	else{
		int size = n*n;
		MultA(C, A, B, n/2);
		MultA(C+(size/4), A, B+(size/4), n/2);
		MultA(C+((3*size)/4), A+(size/2), B+(size/4), n/2);
		MultA(C+(size/2), A+(size/2), B, n/2);

		MultA(C+(size/2), A+((3*size)/4), B+(size/2), n/2);
		MultA(C+((3*size)/4), A+((3*size)/4), B+((3*size)/4), n/2);
		MultA(C+(size/4), A+(size/4), B+((3*size)/4), n/2);
		MultA(C, A+(size/4), B+(size/2), n/2);

		return;
	}
}

//outputAs2D takes a 1D array, the side length of the 2D array, and the file to write to.
void outputAs2D(int* arr, int n, ofstream& file){
	int x;
	int y;
	int size = n*n;
	//declaring new 2D array
	int** arr2d;
	//initializing all positions as an array of ints
	arr2d = new int*[n];

	//initializing all positions as ints
	for(int i = 0; i < n; i++){
		arr2d[i] = new int[n];
	}

	for(int i = 0; i < size; i++){
		//passing the i value to be converted into x, y coordinates.
		convertz(i, x, y);
		//assigning the x, y position in 2D to the old i position in 1D.
		arr2d[x][y] = arr[i];
	}

	//outputting all of the items in the 2D array into the file passed.
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			file << arr2d[i][j] << " ";
		}
		file << endl;
	}
}

int main(int argc, char* argv[]){
	ofstream file;

	char filename[20];
	strcat(filename, "output");
	strcat(filename, argv[1]);
	strcat(filename, ".txt");
	file.open(filename);
	
	//taking input from user.
	int input = atoi(argv[1]);
	int size = input*input;

	//initializing 3 arrays
	int* A = new int[size];
	int* B = new int[size];
	int* C = new int[size];

	//setting all of C to zero, A and B to random ints from 1-9
	for(int i = 0; i < size; i++){
		C[i] = 0;
		A[i] = rand()%10;
		B[i] = rand()%10;
	}

	//Multiplying A and B, storing in C.
	hwtimer_t timer;
    initTimer(&timer);
	startTimer(&timer);
	MultA(C, A, B, input);
    stopTimer(&timer);

	int multTime = getTimerNs(&timer);
    cout << "multiplied two " << input << "x" << input << " matrices in " << multTime << " Ns" << endl;

	//writing all 3 matrices to output.txt
	file << "Matrix A" << endl << endl;
	outputAs2D(A, input, file);
	file << endl;

	file << "Matrix B" << endl << endl;
	outputAs2D(B, input, file);
	file << endl;

	file << "Matrix C" << endl << endl;
	outputAs2D(C, input, file);
	file << endl;

	return 0;
};

//end
