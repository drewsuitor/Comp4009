Name: 		    Drew Suitor
Student Number:	101003158

Developed on Prof. Dehne's Virtual Machine in c++.

main.cpp is the parallel program.
nopar.cpp is the sequential program.

in a command window navigate to this folder.

to compile the program
execute ./_complile

to multiply and n x n array with random numbers from 0-9 in them in parallel
execute ./main <n>
or sequentially
execute ./nopar <n>

to execute the tests on arrays of width 128, 256, 512, 1024, and 2048 in both parallel and sequential execute
./_run

all of the files are output as .txt files in the same directory.

My parallel speedup has been absolutely terrible. I have seen an increase in processing time
by around a factor of 10 when processing in parallel. I suspect this may vary depending on the
base case of the MultA function. if each thread had more to do there would be less overhead
and there may be much better performance.
