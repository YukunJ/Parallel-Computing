// must compile with -std=c99 -Wall -o checkdiv
/*
Created on Fri March 6 9:28:11 am 2020
Last edited on Sun March 8 10:52:13 am 2020
 
@author: Yukun,Jiang
@netid: jy2363
@course: <Parallel Computing Spring 2020>
@content: Lab1
          To implement a parallel divisibilty checker
@innovative: after several attempts, I decided to use bit-wise manipulation
						 to achieve the best performance enhancement
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#define bit0 0b00000001
#define bit1 0b00000010
#define bit2 0b00000100
#define bit3 0b00001000
#define bit4 0b00010000
#define bit5 0b00100000
#define bit6 0b01000000
#define bit7 0b10000000

/* array for bit manipulation */
/* eachb bit will indicate if an integer is divisible by our conidtions */
const char bits[8] = {bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7};

double ceil(double x) {
	/*
	 @func : the math ceiling
	 @param in: double
	 @param ret: double
	*/
	if ((double)(int) x == x)
		return x;
	else
		return (double)(int)x + 1;
}

int main(int argc, char *argv[]) {
	/*
	 @func : parallel implementation of testing number 2 to N if divisible by a,b,c
	 @param in: int, char**
	 @param ret: int
	*/
  unsigned int a, b, c, n;
	FILE * fp; //for creating the output file
	char filename[100]=""; // the file name
	clock_t start_p1, start_p3, end_p1,  end_p3;
	double t2, reduced_t2; // to be used in part2 MPI_Reduce Function.
	/////////////////////////////////////////
	// start of part 1
	start_p1 = clock();
	
	/* Set up the MPI system, variables */
	MPI_Init(&argc, &argv);
	int my_rank, comm_sz, param_lst[4] = {0, 0, 0, 0};
	
	/* Find my rank within the communicator */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	/* Find how many processes are there in the communicator */
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	
	// Check that the input from the user is correct.
	/* Only applicable in Process 0 */
	if (my_rank == 0) { // only process 0 will read command line
		if(argc != 5){
			printf("usage:  ./checkdiv N a b c\n");
			printf("N: the upper bound of the range [2,N]\n");
			printf("a: first divisor\n");
			printf("b: second divisor\n");
			printf("c: third divisor\n");
			MPI_Abort(MPI_COMM_WORLD,1); /* If command line input is wrong, kill all processes */
		}
		n = (unsigned int)atoi(argv[1]);
		a = (unsigned int)atoi(argv[2]);
		b = (unsigned int)atoi(argv[3]);
		c = (unsigned int)atoi(argv[4]);
		param_lst[0] = n;
		param_lst[1] = a;
		param_lst[2] = b;
		param_lst[3] = c;
	}
// Process 0 must send the a, c, and n to each process.
// Other processes must, after receiving the variables, calculate their own range.
	
	/* Broadcast the parameter n, a, b, c as an array of 4 */
	MPI_Bcast(param_lst, 4, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	
	/* unpack the parameter in each process */
	n = param_lst[0];
	a = param_lst[1];
	b = param_lst[2];
	c = param_lst[3];
	end_p1 = clock();
//end of part 1
/////////////////////////////////////////


/////////////////////////////////////////
//start of part 2
// The main computation part starts here
	double start_p2 = MPI_Wtime();
	/*
	 Calculate the range each process need to cover
	 if not evenly divisible, last process do some vain effort
	 using math.h ceil function
	*/
	int slice = (int) ceil(((1.0 * (n-1))/comm_sz)/8);
	char* local_lst_ptr = (char*) malloc(slice * sizeof(char));
	for (int i = 0; i < slice; i++)
		*(local_lst_ptr + i) = 0;         // initialize local list each bit to “Undivisible" for safety
	int local_start = 2 + my_rank * slice * 8;		// compute the responsible range for each process
	char every_8bit = 0b00000000; // initialize every bit to 0
	for (int j = 0, i = local_start; j < slice; j++) {
		for (int k = 0; k <= 7; k++, i++) {
			if (i % a == 0 || i % b == 0 || i % c == 0) // divisible by any of a, b, c
				every_8bit |= bits[k];
		}
		*(local_lst_ptr + j) = every_8bit; // record the answer for these 8 ints;
		every_8bit ^= every_8bit; // reset every bit to 0
	}
	
	/*
		 allocate total storage in process 0
	*/
	char* total_lst_ptr = (char*) malloc(slice * comm_sz * sizeof(char));
	for (int i = 0; i < comm_sz * slice; i++)
		*(total_lst_ptr + i) = 0b00000000;   // initialize total list to all "Undivisible" for safety
	/*
	 gather local list from each process
	*/
	MPI_Gather(local_lst_ptr, slice, MPI_CHAR, total_lst_ptr, slice, MPI_CHAR, 0, MPI_COMM_WORLD);
	free(local_lst_ptr); // clear up the heap allocation in each process
	
	// end of the main compuation part, record the time
	double end_p2 = MPI_Wtime();
	t2 = end_p2 - start_p2;
	/* use reduce operation the get the max of t2 */
	MPI_Reduce(&t2, &reduced_t2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
//end of part 2
/////////////////////////////////////////


/////////////////////////////////////////
//start of part 3
	if (my_rank == 0) { // only process 0 will write to disk about the result
		start_p3 = clock();
		strcpy(filename, argv[1]);
		strcat(filename, ".txt");
		if( !(fp = fopen(filename,"w+t")))
		{
			printf("Cannot create file %s\n", filename);
			exit(1);
		}
		for (int i = 0; i < slice * comm_sz; i++) {
			for (int j = 0; j <= 7; j++) {
				if (2 + 8 * i + j > n) // go out of designated range, break
					break;
				if ( (*(total_lst_ptr + i) & bits[j]) != 0)
					fprintf(fp, "%d\n", 2 + 8 * i + j);
			}
		}
		fclose(fp);
		end_p3 = clock();
	}
	free(total_lst_ptr); // clear up the heap allocation
//end of part 3
/////////////////////////////////////////

/* Print here the times of the three parts as indicated in the lab description */
	if (my_rank == 0) {
		printf("Times of part1 = %lf s, part2 = %lf s, part3 = %lf s.\n", (double)(end_p1 - start_p1)/CLOCKS_PER_SEC, reduced_t2, (double)(end_p3 - start_p3)/CLOCKS_PER_SEC);
	}
	MPI_Finalize(); // clear up MPI system and exit
	return 0;
}
