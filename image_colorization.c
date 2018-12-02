// convert the input RGB colored image into grayscale
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "imcore.h"

int k2counter(int counter, int k, int M, int N) {

    int I0[8] = {-1, 0, +1, +1, +1, 0, -1, -1};
    int J0[8] = {-1, -1, -1, 0, +1, +1, +1, 0};

    int row = counter/M + J0[k];
    int col = counter%M + I0[k];

    if( row < 0 || row > (N-1)) return counter;
    if( col < 0 || col > (M-1)) return counter;

    return col + row*M;
}


void iterative_solver(double *A[9], double *UV, int M, int N, int S) {

    double s,eps=10,ox, w = 1.0;
    int counter,i,j,jj,k,t, Max_Iter = 10000, kcounter;

    double *x  = array_create(double, S);
    memcpy(x, UV, S*sizeof(double));

    for(t=0; t < Max_Iter; t++) {
        counter = -1;
        eps = 0;
        for(i=0; i < N; i++) {
            for(j=0; j < M; j++) {

                counter++;
                s = 0;
                for(k=0;   k < 8; k++) {
                    kcounter = k2counter(counter,k,M,N);
                    if(counter==kcounter) { continue; }
                    s += A[k][counter]*x[kcounter];
                }

                ox = x[counter];
                x[counter] = (1-w)*x[counter] + (w)*(UV[counter]-s);
                eps += fabs(x[counter]-ox);
            }
        }
        printf("Iter[%03d]  E: %3.5lf\r",t, eps);
        if(eps < 0.5)
        {
            break;
        }
    }
    memcpy(UV, x, S*sizeof(double));
    array_free(x);
}



void colorize(matrix_t *gray_image, matrix_t *color_map, matrix_t *output) {

    int M = cols(gray_image);
    int N = rows(gray_image);
    int S = M*N;

    int i,j,k, kk, counter = -1, kcounter;

    double tsum, ssum, val, varc, min_varc, sigma;

    double  D[8] = {0};

    double *A[8];
    for(i=0; i < 8; i++)
    {
        A[i]  = array_create(double, S);
    }

    // Convert RGB color to YUV color space
    double *Y = array_create(double, S);
    double *U = array_create(double, S);
    double *V = array_create(double, S);

    // get the pointer to the data
    uint8_t *gray_image_data  = data(uint8_t, gray_image);
    uint8_t *cmap_image_data = data(uint8_t, color_map);

    counter = 0;
    for(int n=0; n < N; n++) {
        for(int m=0; m < M; m++) {

            uint32_t gidx = idx(gray_image, n,m,0);
            uint32_t cidx = idx(color_map , n,m,0);

            uint8_t gray = gray_image_data[gidx];

            uint8_t cred   = cmap_image_data[cidx + 2];
            uint8_t cgreen = cmap_image_data[cidx + 1];
            uint8_t cblue  = cmap_image_data[cidx + 0];

            Y[counter] = 0.00392156*(gray);
            U[counter] = 0.00392156*(0.596*cred - 0.274*cgreen - 0.322*cblue);
            V[counter] = 0.00392156*(0.211*cred - 0.523*cgreen + 0.312*cblue);
            counter++;
        }
    }
    FILE *report = fopen("report.txt", "w");

    counter = 0;
    for(i=0; i < N; i++) {
        for(j=0; j < M; j++) {

            if( (U[counter]*U[counter] + V[counter]*V[counter]) < 1e-9 ) {

                // first add center values
                tsum = Y[counter];
                ssum = Y[counter]*Y[counter];
                min_varc = 1.0;
                kk = 0;

                for(k=0; k < 8; k++) {
                    // Y value of the neighbors
                    kcounter = k2counter(counter,k,M,N);
                    if(counter==kcounter) { continue; }
                    // add the neighbors to the sum
                    tsum += Y[kcounter];
                    ssum += Y[kcounter]*Y[kcounter];
                    // neighbors to the center distance
                    D[k] = (Y[kcounter]-Y[counter])*(Y[kcounter]-Y[counter]);
                    if(D[k] < min_varc) { min_varc = D[k]; }
                    //printf("[%d,%d]:  %3.5f\n",i,j,Y[kcounter]);
                    kk++;
                }
                //printf("s: %3.6f ssum: %3.5f tsum: %3.5f  k: %01d\n", sigma,ssum, tsum, kk);
                //system("pause");

                sigma = max( (ssum -tsum*tsum/(kk+1))/(kk+1), min_varc/4.6, 2e-6 );

                tsum = 0;
                for(k=0; k < 8; k++) {
                    kcounter = k2counter(counter,k,M,N);
                    if(counter==kcounter) { D[k]=0; continue; }
                    D[k] = exp(-D[k]/sigma);
                    tsum += D[k];
                }

                for(k=0; k < 8; k++) { A[k][counter] = -D[k]/tsum; }

                fprintf(report, "[%05d, %03d, %03d]  s: %3.6f k: %01d\n",counter,i,j, sigma, kk);
            }
            counter++;
        }
    }
    fclose(report);

    printf("Solving for U channel..\n");
    iterative_solver(A, U, M, N, S);
    printf("Solving for V channel..\n");
    iterative_solver(A, V, M, N, S);

    uint8_t *output_data = data(uint8_t, output);

    counter = 0;
    for(i=0; i < N; i++) {
        for(j=0; j < M; j++) {

            double R = Y[counter] + 0.956*U[counter] + 0.621*V[counter];
            double G = Y[counter] - 0.272*U[counter] - 0.647*V[counter];
            double B = Y[counter] - 1.106*U[counter] + 1.703*V[counter];

            uint32_t cidx = idx(output, i, j, 0);

            output_data[cidx + 2]   = clamp(255*R, 0,255);//Y[counter] + 0.956*U[counter] + 0.621*V[counter] - 201.856);
            output_data[cidx + 1]   = clamp(255*G, 0,255);//Y[counter] - 0.272*U[counter] - 0.647*V[counter] + 117.632);
            output_data[cidx + 0]   = clamp(255*B, 0,255);//Y[counter] - 1.106*U[counter] + 1.703*V[counter] - 076.416);
            counter++;
        }
    }
    return;
}

int main(int argc, unsigned char *argv[]) {

    // read the test image
    unsigned char filename_gray[256] = "..//data/example1.bmp";
    unsigned char filename_map[256] = "..//data/example1_marked.bmp";

    if(argc > 2) {
        strncpy(filename_gray, argv[1], 256);
        strncpy(filename_map, argv[2], 256);
    }
    // read the input image
    matrix_t *input_image = imread(filename_gray);
    matrix_t *color_map = imread(filename_map);
    // create the gray images
    matrix_t *gray_image = matrix_create(uint8_t);

    // create the output image
    matrix_t *colorized_image = matrix_create(uint8_t, rows(input_image), cols(input_image), channels(input_image));

    // convert the input into grascale
    rgb2gray(input_image, gray_image);

    // colorize the image using the grayscale and sampled colors
    colorize(gray_image, color_map, colorized_image);

    // write the colorized output
    imwrite(colorized_image, "colorized_image.bmp");

    return 0;
}