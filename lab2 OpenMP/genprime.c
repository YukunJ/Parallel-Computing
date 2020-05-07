// must compile with -std=c99 -Wall -o checkdiv
/*
Created on Fri March 6 9:28:11 am 2020
Last edited on Fri April 11 10:52:13 am 2020
 
@author: Yukun,Jiang
@netid: jy2363
@course: <Parallel Computing Spring 2020>
@content: Lab2
          To implement a parallel Sieve's prime number algorithm using OpenMP
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

int main(int argc, char* argv[])
{
	/*
	 @func : parallel implementation of testing number 2 to N if divisible by a,b,c
	 @param in: int, char**
	 @param ret: int
	*/
	
	// ***************** Part 1 *****************
	// read command line argument and data setup
	long N = atol(argv[1]);
	int thread_count = atoi(argv[2]);
	double t_start, t_taken;
	long outer,inner,stop_floor;
	char* prime = (char*)malloc((N+1)*sizeof(char));
	stop_floor = (long)(sqrt(N)+1);
	for (outer = 0; outer <= N; outer++) // Array initialization for safety
		prime[outer] = 1;
	
	// ***************** Part 2 *****************
	// the main part of parallel computing using OpenMP
	
	#pragma omp parallel num_threads(thread_count)\
					default(none) private(outer,inner) shared(prime,N,stop_floor,t_start)
	{
		t_start = omp_get_wtime();
		for (outer = 2; outer <= stop_floor; outer++) {
			if (prime[outer]) { // mark all its multiples as not prime number
				#pragma omp for
				for (inner = 2*outer; inner <= N; inner += outer)
					prime[inner] = 0;
			}
		}
	}
	/* join back to master thread */
	t_taken = omp_get_wtime() - t_start;
	printf("Time taken for the main part: %f\n",t_taken);
	
	// ***************** Part 3 *****************
	// output the prime number into a .txt file
	char filename[100] = "";
	strcpy(filename, argv[1]);
	strcat(filename, ".txt");
	FILE* fp = fopen(filename, "w");
	for (outer = 2; outer <= N; outer++) {
		if (prime[outer] == 1)
			fprintf(fp,"%ld\n",outer);
	}
	free(prime);
	return 0;
}
