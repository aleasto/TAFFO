/**
 * correlation.cu: This file is part of the PolyBench/GPU 1.0 test suite.
 *
 *
 * Contact: Scott Grauer-Gray <sgrauerg@gmail.com>
 * Will Killian <killian@udel.edu>
 * Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://www.cse.ohio-state.edu/~pouchet/software/polybench/GPU
 */
#include "correlation.cuh"
#include "correlation_sh_ann.h"
#include <polybench.h>
#include <polybenchUtilFuncts.h>


extern "C" __global__ void mean_kernel(int m, int n, DATA_TYPE *mean ANN_MEAN, DATA_TYPE *data ANN_DATA,  DATA_TYPE float_n ANN_FLOAT_N)
{
	int j = blockIdx.x * blockDim.x + threadIdx.x;

	if (j < _PB_M)
	{
		mean[j] = 0.0;

		int i;
		for(i=0; i < _PB_N; i++)
		{
			mean[j] += data[i*M + j];
		}
	
		mean[j] /= float_n;
	}
}


extern "C" __global__ void std_kernel(int m, int n, DATA_TYPE *mean ANN_MEAN, DATA_TYPE *std ANN_STD, DATA_TYPE *data ANN_DATA, DATA_TYPE float_n ANN_FLOAT_N, DATA_TYPE eps ANN_EPS)
{
	int j = blockIdx.x * blockDim.x + threadIdx.x;
	
	if (j < _PB_M)
	{
		std[j] = 0.0;

		int i;
		for(i = 0; i < _PB_N; i++)
		{
			DATA_TYPE __attribute__((annotate("scalar()"))) tmp = (data[i*m + j] - mean[j]);
			tmp = tmp * tmp;
			std[j] += tmp;
		}
		DATA_TYPE __attribute__((annotate("scalar(range(0,5000) final)"))) tmp = std[j];
		std[j] = sqrt(tmp / float_n);
		if(std[j] <= eps) 
		{
			std[j] = 1.0;
		}
	}
}


extern "C" __global__ void reduce_kernel(int m, int n, DATA_TYPE *mean ANN_MEAN, DATA_TYPE *std ANN_STD, DATA_TYPE *data ANN_DATA, DATA_TYPE float_n ANN_FLOAT_N)
{
	int j = blockIdx.x * blockDim.x + threadIdx.x;
	int i = blockIdx.y * blockDim.y + threadIdx.y;
	
	if ((i < _PB_N) && (j < _PB_M))
	{
		data[i*m + j] -= mean[j];
		DATA_TYPE tmp = sqrt(float_n) * std[j];
		data[i*m + j] /= tmp;
	}
}


extern "C" __global__ void corr_kernel(int m, int n, DATA_TYPE *symmat ANN_SYMMAT, DATA_TYPE *data ANN_DATA)
{
	int j1 = blockIdx.x * blockDim.x + threadIdx.x;

	int i, j2;
	if (j1 < (_PB_M-1))
	{
		symmat[j1*M + j1] = 1.0;

		for (j2 = (j1 + 1); j2 < _PB_M; j2++)
		{
			symmat[j1*M + j2] = 0.0;

			for(i = 0; i < _PB_N; i++)
			{
				symmat[j1*M + j2] += data[i*M + j1] * data[i*M + j2];
			}
			symmat[j2*M + j1] = symmat[j1*M + j2];
		}
	}
}