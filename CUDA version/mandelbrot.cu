#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 256
#define MaxColor 255

int *a;

__device__ double CxMin = -2.5;
__device__ double CxMax = 1.5;
__device__ double CyMin = -2.0;
__device__ double CyMax = 2.0;

__device__ int seriesConverges(int x, int y,int width){
	double Cx,Cy,PixelHeight,PixelWidth;
	double Zx,Zy,Zx2,Zy2;
	PixelWidth = (CxMax-CxMin)/width;
	PixelHeight = (CyMax-CyMin)/width;
	Cy = CyMin +x*PixelHeight;
	if (fabs(Cy) < PixelHeight/2) {
		Cy = 0.0;
	}
	Cx = CxMin + y*PixelWidth;
	int color = 1;
	Zx = 0.0;
	Zy = 0.0;
	Zx2 = 0.0;
	Zy2 = 0.0;
	for (int i = 0; i < 512 && ((Zx2+Zy2)<4); i++) {
		Zx2 = Zx*Zx;
		Zy2 = Zy*Zy;
		Zy *= Zx;
		Zy += Zy+Cy;
		Zx = Zx2-Zy2+Cx;
		color++;
	}
	return color;
}

__global__ void mandelKernel(int *a, int width) {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < width) {
		for (int j = 0; j < width; j++) {
			a[i*width+j] = seriesConverges(i,j,width);
		}
	}
}	

void mandelDevice(int *a, int tam) {
	float time;
	cudaEvent_t start, stop;
	int *aD;
	int size = N*N*tam*tam*sizeof(int);
	int tam2 = N*N*tam*tam;
	int bsize =ceil((float)tam2/(float)1024);
	dim3 bloques(bsize);
	dim3 hilos (1024);
	cudaSetDevice(0);
	cudaMalloc(&aD, size);
	cudaMemcpy(aD, a, size, cudaMemcpyDefault);

	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);
	mandelKernel<<<bloques, hilos>>>(aD,N*tam);
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time, start, stop);
	printf("%3.1f \n", time);
	cudaMemcpy(a, aD, size, cudaMemcpyDefault);
	cudaFree(aD);
}

int main() {
	FILE  *fp;
	static unsigned char color[3];
	int colorValue;
	for(int counter = 50; counter<=50; counter++){
		a = (int *)calloc(N*N*counter*counter,sizeof(int));
		mandelDevice(a, counter);
		fp = fopen("mandelbrotSet2.ppm","wb");
		fprintf(fp,"P6\n #\n %d\n %d\n %d\n",N*counter,N*counter,MaxColor);
		for (int i = 0; i < N*counter; i++) {
			for(int j = 0; j < N*counter; j++) {
				colorValue = a[i*N*counter+j];
				color[0] = (unsigned char)colorValue%MaxColor;
				color[1] = (unsigned char)colorValue%MaxColor;
				color[2] = (unsigned char)colorValue%MaxColor;
				fwrite(color,1,3,fp);
			}
		}
		fclose(fp);
		free(a);
	}
	return 0;
}
