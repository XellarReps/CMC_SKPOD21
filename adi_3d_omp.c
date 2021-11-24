#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define  Max(a,b) ((a)>(b)?(a):(b))
#define  N   (2 * 2 * 2 * 2 * 2 * 2 + 2)

double   maxeps = 0.1e-7;
int itmax = 100;
int i, j, k;

double eps;
double A [N][N][N];

int threads = 4;

void relax();
void init();
void verify();
double rtclock();

double 
rtclock()
{
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, NULL);
    if (stat != 0) {
    	printf ("Error return from gettimeofday: %d", stat);
	}
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

int 
main(int an, char **as)
{
	int it;
	double bench_t_start = rtclock();
	init();
	for (it = 1; it <= itmax; ++it) {
		eps = 0.;
		relax();
		//printf( "it=%4i   eps=%f\n", it, eps);
		if (eps < maxeps) {
			break;
		}
	}
	verify();
	double bench_t_end = rtclock();
	printf ("Time in seconds = %0.6lf\n", bench_t_end - bench_t_start);
	return 0;
}


void 
init()
{ 
	#pragma omp parallel for private(k, j, i) num_threads(threads)
	for (k = 0; k <= N - 1; ++k) {
		for (j = 0; j <= N - 1; ++j) {
			for (i = 0; i <= N - 1; ++i) {
				if (i == 0 || i == N - 1 || j == 0 || j == N - 1 || k == 0 || k == N - 1) {
					A[i][j][k] = 0.;
				} else {
					A[i][j][k] = ( 4. + i + j + k);
				}
			}
		}
	}
} 

void 
relax()
{
	for (i = 1; i <= N - 2; ++i) {
		#pragma omp parallel for private(k, j) num_threads(threads)
		for (k = 1; k <= N - 2; ++k) {
			for (j = 1; j <= N - 2; ++j) {
				A[i][j][k] = (A[i - 1][j][k] + A[i + 1][j][k]) / 2.;
			}
		}
	}
	for (j = 1; j <= N - 2; ++j) {
		#pragma omp parallel for private(k, i) num_threads(threads)
		for (k = 1; k <= N - 2; ++k) {
			for (i = 1; i <= N - 2; ++i) {
				A[i][j][k] = (A[i][j - 1][k] + A[i][j + 1][k]) / 2.;
			}
		}
	}
	for (k = 1; k <= N - 2; ++k) {
		#pragma omp parallel for private(j, i) num_threads(threads)
		for (j = 1; j <= N - 2; ++j) {
			for (i = 1; i <= N - 2; ++i) {
				double e;
				e = A[i][j][k];
				A[i][j][k] = (A[i][j][k - 1] + A[i][j][k + 1]) / 2.;
				e = fabs(e-A[i][j][k]);
				if (eps < e) {
					#pragma omp critical
					eps = e;
				}
			}
		}
	}
}

void 
verify()
{
	double s;
	s = 0.;
	#pragma omp parallel for private(k, j, i) reduction(+:s) num_threads(threads)
	for (k = 0; k <= N - 1; ++k) {
		for (j = 0; j <= N - 1; ++j) {
			for (i = 0; i <= N - 1; ++i) {
				s = s + A[i][j][k] * (i + 1) * (j + 1) * (k + 1) / (N * N * N);
			}
		}
	}
	//printf("  S = %f\n",s);
}