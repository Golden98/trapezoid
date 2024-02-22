/* File:    trap.c
 * Purpose: Calculate definite integral using trapezoidal 
 *          rule.
 *
 * Input:   a, b, n
 * Output:  Estimate of integral from a to b of f(x)
 *          using n trapezoids.
 *
 * Compile: gcc -g -Wall -o trap trap.c
 * Usage:   ./trap
 *
 * Note:    The function f(x) is hardwired.
 *
 * IPP:     Section 3.2.1 (pp. 94 and ff.) and 5.2 (p. 216)
 */


#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>
#include <vector>

using namespace std;

// Function Declarations
double f(double x);    /* Function we're integrating */
double Trap(double a, double b, int n, double h);
void* runTrap(void* args);

typedef struct {
    sem_t mutex;
    double sum;
} sum_t;

sum_t shared;

typedef struct {
    double a;
    double b;
    int n;
    double h;
} arg_t;

// START of main function
int main() {
   double  integral;   /* Store result in integral   */
   double  a, b;                 /* Left and right endpoints      */
   int     n;                    /* Total number of trapezoids    */
   double  h;                    /* Width of trapezoids       */
   int     thread_count;
   
   //*************************************************
   // Establish paramaters for the integral function
   //************************************************
   // Request input from user : THREAD COUNT
   cout << "Enter Thread Count : " ;
   cin >> thread_count;
    if (cin.fail())
	{cout << "ERROR: ";
		cout << "   Input error on left or right endpoints \n" ;
		return EXIT_FAILURE;
	}
   // Request input from user : ENDPOINTS
   cout << "Enter the left and right enpoints of the integral: ";
   cin >> a >> b;
   if (cin.fail())
	{cout << "ERROR: ";
		cout << "   Input error on left or right endpoints \n" ;
		return EXIT_FAILURE;
	}
 // Request input from user : NUMBER OF TRAPEZOIDAL SECTIONS		
   cout << "Enter the number of trapezoids: " ;
   cin >> n;
   if ((n <=0) or (n%thread_count!= 0) or cin.fail())
   {cout << "ERROR: \n";
	cout << "Number of trapzoids " << n << "   Thread count " <<thread_count 
		 << " division " << thread_count%n << " cin " << cin.fail() << endl;
	cout << "   number of trapezoids must be evenly divisible by the thread count\n"
	     << "   and a number greater than zero \n";
		 return EXIT_FAILURE;
   }
   // Calculate Width of the trapezoid
   h = (b-a)/n;

    // initialize shared mem
    sem_init(&shared.mutex, 0, 1);
    shared.sum = 0;

    // length of each seqment
    double segmentLen = (b - a) / thread_count;

    vector<pthread_t> threads;
    for(int i = 0; i < thread_count; i++) {
        // create new arg_t for each thread
        arg_t args;
        
        args.a = a + (i * segmentLen);
        args.b = a + ((i+1) * segmentLen);
        args.h = h;
        args.n = n / thread_count;      // since the number of trapezoids must be evenly divisable by thread_count

        pthread_t id;
        pthread_create(&id, NULL, runTrap, (void*) &args); // create new thread to run this segment
        threads.emplace_back(id);
    }
    
    for (int i = 0; i < thread_count; i++) {
        // may just use an iterator TODO
        //pthread_join(threads.pop_back(), NULL);
    }

   integral = Trap(a, b, n, h);
   
   cout << "With n = " << n << " trapezoids, our estimate of the integral from points "  
        << a << " to " << b << " is " << integral  << endl;
  
   return EXIT_SUCCESS;
}  /* main */

/*------------------------------------------------------------------
 * Function:    Trap
 * Purpose:     Estimate integral from a to b of f using trap rule and
 *              n trapezoids
 * Input args:  a, b, n, h
 * Return val:  Estimate of the integral 
 */
double Trap(double a, double b, int n, double h) {
   double integral;
   int k;

   integral = (f(a) + f(b))/2.0;
   for (k = 1; k <= n-1; k++) {
     integral += f(a+k*h);
   }
   integral = integral*h;

   return integral;
}  /* Trap */

/*------------------------------------------------------------------
 * Function:    f
 * Purpose:     Compute value of function to be integrated
 * Input args:  x
 */
double f(double x) {
   double return_val;

   return_val = x*x;
   return return_val;
}  /* f */


/*
Function: runTrap
Purpose:  run trap function 
Input args: args, a pointer to an arg_t struct
*/
void* runTrap(void* args) {
    arg_t arg = *((arg_t*)args); // check this

    double res = Trap(arg.a, arg.b, arg.n, arg.h);
    sem_wait(&shared.mutex);
    shared.sum += res;
    sem_post(&shared.mutex);
}