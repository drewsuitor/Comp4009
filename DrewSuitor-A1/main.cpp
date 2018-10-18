
#include <cstdlib>
#include <iostream>
#include <cilk/cilk.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <bits/stdc++.h> 
#include "hwtimer.h"
using namespace std;

int** arr;
int** arr2;
int m;
int n;

/*
 * the fileToArray function takes a char* array of characters (string) and
 * opens the file then initializes and n*m 2D array global variable which it 
 * inserts all of the integer values into.
 */
int** fileToArray(char* filename){
	int k;
	string line;
	ifstream input(filename);
	
	if(input.good()){
		getline(input, line);
		m = atoi(line.c_str());
			
		getline(input, line);
		n = atoi(line.c_str());
			
		getline(input, line);
		k = atoi(line.c_str());
	}
	int** arr = new int*[m];
	arr2 = new int*[m];
	
	for(int h = 0; h < m; h++){
		arr[h] = new int[n];
		arr2[h] = new int[n];
	}
	
	int i = 0;
	while(getline(input, line) && i < m){
		int j = 0;
		stringstream ssin(line);
		while(ssin.good() && j < n){
			ssin >> arr[i][j];
			j++;
		}
		i++;
	}
	
	return arr;
}

/*
 * findMedian takes an input array and the arrays size.
 * it sorts the array and returns the middle value.
 */
int findMedian(int a[], int n){
	sort(a, a+n);
	int mid;
	
	if(n%2 !=0){
		mid = n/2;
	}
	
	else{
		mid = n/2 + 1;
	}
	
	return a[mid];
}
/*
 * median function takes a position value in a 2D array as 2 integers, x and y,
 * and how large of a square it is taking the median of as k.
 * It takes all of the values in the x by y range and adds them to a new temporary
 * array, finding the closest value if it is out of bounds.
 * It then passes the new array to the findMedian function.
 */
int median(int x, int y, int k){
	int size = ((2*k)+1)*((2*k)+1);
	int temp[size];
	//initializing a 'box's corners
	int startX = x-k;
	int endX = x+k+1;
	int startY = y-k;
	int endY = y+k+1;
	int a = 0;
	for(int i = startX; i < endX; i++){
		for(int j = startY; j < endY; j++){
			int posX = i;
			int posY = j;
			//if out of bounds set to the nearest value
			if(posX < 0){posX = 0;}
			if(posX >= m - 1){posX = m - 1;}
			if(posY < 0){posY = 0;}
			if(posY >= n - 1){posY = n - 1;}
			//add the value to the temp array
			temp[a] = arr[posX][posY];
			a++;
		}
	}
	
	int med = findMedian(temp, size);
	
	return med;
}

/*
 * processImage function takes the top left and bottom right corner of an 'image' 2D array 
 * and spawns a thread for each 'pixel' in the 2D array.
 * Once it reaches the base case, which is when it is at a single position, it will call
 * the median function on that position which will calculate the median based on the k
 * value that is passed in.
 */
void processImage(float x1, float y1, float x2, float y2, int k){
	//base case is when a single item is selected
	//ex: when the position of the top left corner is also the position of the bottom right corner
	if(x1 == x2 && y1 == y2){
		arr2[(int)x1][(int)y1] = median(x1, y1, k);
		return;
	}
	//if not base case, 'cilk_spawn's on each quatrant, spawning a new thread for each.
	else{
		cilk_spawn processImage(x1, y1, floor((x1+x2)/2), floor((y1+y2)/2), k);
		cilk_spawn processImage(ceil((x1+x2)/2), y1, x2, floor((y1+y2)/2), k);
		cilk_spawn processImage(x1, ceil((y1+y2)/2), floor((x1+x2)/2), y2, k);
		cilk_spawn processImage(ceil((x1+x2)/2), ceil((y1+y2)/2), x2, y2, k);
		cilk_sync;
	}
}

int main(int argc, char* argv[]){
	
	//collecting input from user
	char* filename;
	char* outputfile;
	filename = argv[1];
	outputfile = argv[2];
	
	int k;
	
	string line;
		
	ifstream input(filename);
		
	//reading file that user entered for m, n, and k values.
	if(input.good()){
		getline(input, line);
		m = atoi(line.c_str());
			
		getline(input, line);
		n = atoi(line.c_str());
			
		getline(input, line);
		k = atoi(line.c_str());
		
		//calling function to build 2d array
		arr = fileToArray(filename);
		//calling function to process 'image'
		processImage(0, 0, m-1, n-1, k);
		//writing processed image to a file called 
		ofstream myfile;
		myfile.open(outputfile);
		for(int i = 0; i < m; i++){
			for(int j = 0; j < n; j++){
				if(j == n-1){
					myfile << arr2[i][j];
				}
				else{
					myfile << arr2[i][j] << " ";
				}
			}
			myfile << endl;
		}
	}
	else{
		cout << "****" << endl;
		cout << "	You have entered an invalid input." << endl;
		cout << "	Process file by entering command:" << endl;
		cout << "	./main <input file location> <output file location>" << endl << endl;
		cout << "	See _run command or enter ./_run to process all 5 files." << endl;
		cout << "****" << endl;
	}

	
	return 0;
}















//
