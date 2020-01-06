#include <stdio.h>
#include <mpi.h>
#include <math.h>
 
#define CxMin -2.5
#define CxMax 1.5
#define CyMin -2.0
#define CyMax 2.0
#define iXmax 1000
#define iYmax 1000
#define MaxColor 255
#define ER2 4
#define IterationMax 512
#define MASTER_TO_SLAVE_TAG 1
#define SLAVE_TO_MASTER_TAG 4 

int rank;
int size;
int i,j,k;
int colorMatrix[iYmax][iXmax];
double start_time;
double end_time;
int low_bound; 
int upper_bound; 
int portion; 
MPI_Status status; // store status of a MPI_Recv
MPI_Request request; //capture request of a MPI_Isend
int iX,iY;
double Cx,Cy;
double PixelWidth=(CxMax-CxMin)/iXmax;
double PixelHeight=(CyMax-CyMin)/iYmax;
int colorValue;
double Zx, Zy, Zx2, Zy2;

int main() {

	MPI_Init(NULL, NULL);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Comm_size(MPI_COMM_WORLD, &size);
   
    	if (rank == 0) {
		start_time = MPI_Wtime();
		for (i = 1; i < size; i++) {
			portion = (iYmax / (size - 1));
			low_bound = (i - 1) * portion;
			if (((i + 1) == size) && ((iYmax % (size - 1)) != 0)) {
				upper_bound = iYmax;
            		} else {
				upper_bound = low_bound + portion;
            		}
			MPI_Isend(&low_bound,1,MPI_INT,i,MASTER_TO_SLAVE_TAG,MPI_COMM_WORLD,&request);
            		MPI_Isend(&upper_bound,1,MPI_INT,i,MASTER_TO_SLAVE_TAG+1,MPI_COMM_WORLD,&request);
        	}
    	}

    	if (rank > 0) {
        	MPI_Recv(&low_bound, 1, MPI_INT, 0, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
        	MPI_Recv(&upper_bound, 1, MPI_INT, 0, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD, &status);
        	for (iY = low_bound; iY < upper_bound; iY++) {
			Cy=CyMin + iY*PixelHeight;
             		if (fabs(Cy)< PixelHeight/2) Cy=0.0;
             		for(iX=0;iX<iXmax;iX++) {
                        	Cx=CxMin + iX*PixelWidth;
                        	Zx2=Zy2=Zy=Zx=0.0;
				colorValue = 1;
                        	for (i = 0; i < IterationMax && ((Zx2+Zy2)<ER2); i++) {
                            		Zx2=Zx*Zx;
                            		Zy2=Zy*Zy;
			    		Zy *= Zx;
			    		Zy += Zy + Cy;
			    		Zx = Zx2-Zy2+Cx;
			    		colorValue++;
                        	}
				colorMatrix[iY][iX] = colorValue%MaxColor;
        		}
		}
        	MPI_Isend(&low_bound,1,MPI_INT,0,SLAVE_TO_MASTER_TAG,MPI_COMM_WORLD,&request);
   //     	MPI_Isend(&upper_bound,1,MPI_INT,0,SLAVE_TO_MASTER_TAG+1,MPI_COMM_WORLD,&request);
  //      	MPI_Isend(&colorMatrix[low_bound],(upper_bound-low_bound)*iXmax,MPI_INT,0,SLAVE_TO_MASTER_TAG + 2,MPI_COMM_WORLD,&request);
    	}
    	if (rank == 0) {
        	for (i = 1; i < size; i++) {
			MPI_Recv(&low_bound, 1, MPI_INT, i, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD, &status);
     //       		MPI_Recv(&upper_bound, 1, MPI_INT, i, SLAVE_TO_MASTER_TAG + 1, MPI_COMM_WORLD, &status);
//            		MPI_Recv(&colorMatrix[low_bound], (upper_bound-low_bound)*iXmax, MPI_INT, i, SLAVE_TO_MASTER_TAG + 2, MPI_COMM_WORLD, &status);
        	}
        	end_time = MPI_Wtime();
        	printf("\nRunning Time = %f\n\n", end_time - start_time);
		FILE * fp;
        	static unsigned char color[3];
		fp = fopen("mandelbrotSet.ppm","wb");
		fprintf(fp,"P6\n #\n %d\n %d\n %d\n",iXmax,iYmax,MaxColor);
		for(iY=0;iY<iYmax;iY++){
			for(iX=0;iX<iXmax;iX++) {
				color[0] = (unsigned char) colorMatrix[iY][iX];
				color[1] = (unsigned char) colorMatrix[iY][iX];
				color[2] = (unsigned char) colorMatrix[iY][iX];
				fwrite(color,1,3,fp);
			}
		}	
        	fclose(fp);
    	}
   	MPI_Finalize(); //finalize MPI operations
    	return 0;
}
