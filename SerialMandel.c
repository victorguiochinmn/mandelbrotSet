#include <stdio.h>
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

void main()  {
        int iX,iY;
        double Cx,Cy;
        double PixelWidth=(CxMax-CxMin)/iXmax;
        double PixelHeight=(CyMax-CyMin)/iYmax;
        FILE * fp;
        static unsigned char color[3];
        double Zx, Zy, Zx2, Zy2;
	int colorValue;
        fp = fopen("mandelbrotSet.ppm","wb"); 
	fprintf(fp,"P6\n #\n %d\n %d\n %d\n",iXmax,iYmax,MaxColor);
        for(iY=0;iY<iYmax;iY++) {
        	Cy=CyMin + iY*PixelHeight;
             	if (fabs(Cy)< PixelHeight/2) Cy=0.0;
		for(iX=0;iX<iXmax;iX++) {
                	Cx=CxMin + iX*PixelWidth;
                        Zx2=Zy2=Zy=Zx=0.0;
			colorValue = 1;
                        for (int i = 0; i < IterationMax && ((Zx2+Zy2)<ER2); i++) {
                            Zx2=Zx*Zx;
                            Zy2=Zy*Zy;
			    Zy *= Zx;
			    Zy += Zy + Cy;
			    Zx = Zx2-Zy2+Cx;
			    colorValue++;
                        }
	                color[0]=(unsigned char)colorValue%MaxColor;
	                color[1]=(unsigned char)colorValue%MaxColor;
	                color[2]=(unsigned char)colorValue%MaxColor;
                        fwrite(color,1,3,fp);
                }
        }
        fclose(fp);
 }
