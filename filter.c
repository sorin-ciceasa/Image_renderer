#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

int num_threads;
int resize_factor;
int GaussKernell[3][3]={{1,2,1},{2,4,2},{1,2,1}};
image *copy_in;
image *copy_out;

void readInput(const char * fileName, image *img) {

	FILE *fptr;
	int i;

	if ((fptr = fopen(fileName,"r")) == NULL){
		exit(1);
    }

  	fscanf(fptr,"%s\n%u %u\n%hu\n",img->type,&img->width,&img->height,&img->maxval);

  	img->pixel_matrix = (pixel **) calloc(img->height, sizeof(pixel*));
  	
  	//read BW image
	if(strstr(img->type,"P5")!=NULL){

		for(i=0; i < img->height; i++){
			img->pixel_matrix[i]=(pixel *) calloc(img->width, 1);
		}

		for(i=0; i < img->height;i++){
			fread(img->pixel_matrix[i],sizeof(pixel), img->width, fptr);
		}

	}

	//read COLOR image
	else{

		for(i=0; i < img->height; i++){
			img->pixel_matrix[i]=(pixel *) calloc(3*img->width, sizeof(pixel));
		}

		for(i=0; i < img->height;i++){
			fread(img->pixel_matrix[i],sizeof(pixel),3*img->width, fptr);
		}

	}

    fclose(fptr);
}

void writeData(const char *fileName, image *img) {

	int i;
	FILE* fptr;

	if ((fptr = fopen(fileName,"w+")) == NULL){
		exit(1);
    }
	
	//write image header
	fwrite(img->type,2,1,fptr);
	fprintf(fptr,"\n%hu",img->width);
	fprintf(fptr," %hu\n",img->height);
	fprintf(fptr,"%hu\n",img->maxval);

	//write with*height*num_colors pixels
	if(img->type[1]=='5'){
   		for(i=0 ; i < img->height;i++){
			fwrite(img->pixel_matrix[i],sizeof(pixel),img->width,fptr);
		}
	} 

	else{
   		for(i=0 ; i < img->height;i++){
			fwrite(img->pixel_matrix[i],3*sizeof(pixel),img->width,fptr);
		}
	}

	fclose(fptr);
}

void* threadFunction(void *var)
{
	int thread_id = *(int*)var;
	unsigned int t;
	double max,maxi, maxj,start;
	int i,j,ik,jk;
	int left,right;

	//partition image rows in equal groups for each thread to compute
	start=thread_id*ceil(copy_out->height/num_threads);
	max=(thread_id+1)*ceil(copy_out->height/num_threads);
	
	if (max>copy_out->height){
		max=copy_in->height;
	}

	//last thread needs to check for rows that were lost during resize
	if(thread_id==(num_threads - 1))
		maxi=max+(copy_out->height%num_threads);
	else
		maxi=max;
	
	maxj=copy_out->width;
	
	// BW image
	// Each pixel in the new image is made from resize_factor*resize_factor
	//pixels in the original matrix.
	// ik,jk are used to create a matrix of size resize_factor*resize_factor. 
	if(copy_out->type[1]=='5'){		

		//resize is an even number
		if(resize_factor%2==0){

			for(i=start;i < maxi;i++){
				for(j=0 ; j < maxj;j++){
					t=0;
					for(ik=0;ik < resize_factor;ik++){
						for(jk=0;jk < resize_factor;jk++){
							t = t + copy_in->pixel_matrix[i*resize_factor+ik][j*resize_factor+jk].value;
						}
					}
					t=t/(resize_factor*resize_factor);
					copy_out->pixel_matrix[i][j].value=(unsigned char)t;
				}
			}
		}

		//resize is 3
		else{

			// multiply with the GaussianKernell
			for(i=start;i < maxi;i++){
				for(j=0 ; j < maxj; j++){
	
					t=0;
					left=i*3;
					right=j*3;
					t = t + copy_in->pixel_matrix[left][right].value*GaussKernell[0][0] +
							copy_in->pixel_matrix[left][right+1].value*GaussKernell[1][0] +
							copy_in->pixel_matrix[left][right+2].value*GaussKernell[2][0] +
							
							copy_in->pixel_matrix[left+1][right].value*GaussKernell[0][1] +
							copy_in->pixel_matrix[left+1][right+1].value*GaussKernell[1][1] +
							copy_in->pixel_matrix[left+1][right+2].value*GaussKernell[2][1] +
							
							copy_in->pixel_matrix[left+2][right].value*GaussKernell[0][2] +
							copy_in->pixel_matrix[left+2][right+1].value*GaussKernell[1][2] +
							copy_in->pixel_matrix[left+2][right+2].value*GaussKernell[2][2];
					t=t/16;
					copy_out->pixel_matrix[i][j].value=(unsigned char)t;
				}
			}
		}
	}

	//COLOR image
	//For colored images, in order to get the scaled pixel each
	//pixel must be matched with others that have the same color.
	//This means that the distance between pixels that are to be 
	//computed from the original image is 3 on the width direction.
	//The "for"s below compute an ij index from the new matrix using
	//the pixels from the old matrix.
	//ik,jk are used to create a matrix of size resize_factor*resize_factor.

	else{

		//resize is an even number
		if(resize_factor%2==0){		

			int resizex3=resize_factor*3;
			int jdiv3,jmod3;

			for(i=start;i < maxi;i++){
				for(j=0 ; j < 3*maxj;j++){
					
					t=0;
					left=i*resize_factor;
					jdiv3=j/3;
					jmod3=j%3;

					for(ik=0;ik < resize_factor;ik++){

						//right=(j/3)*resizex3;
					
						//left=i*resize_factor+ik;
						for(jk=0;jk < resize_factor;jk++){

							right=jdiv3*resizex3+3*jk;
							if(jmod3==0){
								t = t + copy_in->pixel_matrix[left][right].value;
							}else if (jmod3==1){
								t = t + copy_in->pixel_matrix[left][right+1].value;								
							}
							else{
								t = t + copy_in->pixel_matrix[left][right+2].value;
							}
						}
						
						left++;
					}

					t=t/(resize_factor*resize_factor);
					copy_out->pixel_matrix[i][j].value=(unsigned char)t;
				}
			}

		}
		
		//resize is 3
		else{

			// multiply with the GaussianKernell
			for(i=start;i < maxi;i++){
				for(j=0 ; j < 3*maxj;j++){

					t=0;
					left=i*3;
					right=j*3;
					
					if(j%3==0)
						t = t + copy_in->pixel_matrix[left][right].value*GaussKernell[0][0] +
								copy_in->pixel_matrix[left][right+3].value*GaussKernell[1][0] +
								copy_in->pixel_matrix[left][right+6].value*GaussKernell[2][0] +
							
								copy_in->pixel_matrix[left+1][right].value*GaussKernell[0][1] +
								copy_in->pixel_matrix[left+1][right+3].value*GaussKernell[1][1] +
								copy_in->pixel_matrix[left+1][right+6].value*GaussKernell[2][1] +
								
								copy_in->pixel_matrix[left+2][right].value*GaussKernell[0][2] +
								copy_in->pixel_matrix[left+2][right+3].value*GaussKernell[1][2] +
								copy_in->pixel_matrix[left+2][right+6].value*GaussKernell[2][2];
				
					else if(j%3==1)
						t = t + copy_in->pixel_matrix[left][right-2].value*GaussKernell[0][0] +
								copy_in->pixel_matrix[left][right+1].value*GaussKernell[1][0] +
								copy_in->pixel_matrix[left][right+4].value*GaussKernell[2][0] +
							
								copy_in->pixel_matrix[left+1][right-2].value*GaussKernell[0][1] +
								copy_in->pixel_matrix[left+1][right+1].value*GaussKernell[1][1] +
								copy_in->pixel_matrix[left+1][right+4].value*GaussKernell[2][1] +
								
								copy_in->pixel_matrix[left+2][right-2].value*GaussKernell[0][2] +
								copy_in->pixel_matrix[left+2][right+1].value*GaussKernell[1][2] +
								copy_in->pixel_matrix[left+2][right+4].value*GaussKernell[2][2];
					else
						t = t + copy_in->pixel_matrix[left][right-4].value*GaussKernell[0][0] +
								copy_in->pixel_matrix[left][right-1].value*GaussKernell[1][0] +
								copy_in->pixel_matrix[left][right+2].value*GaussKernell[2][0] +
							
								copy_in->pixel_matrix[left+1][right-4].value*GaussKernell[0][1] +
								copy_in->pixel_matrix[left+1][right-1].value*GaussKernell[1][1] +
								copy_in->pixel_matrix[left+1][right+2].value*GaussKernell[2][1] +
								
								copy_in->pixel_matrix[left+2][right-4].value*GaussKernell[0][2] +
								copy_in->pixel_matrix[left+2][right-1].value*GaussKernell[1][2] +
								copy_in->pixel_matrix[left+2][right+2].value*GaussKernell[2][2];

					t=t/16;
					copy_out->pixel_matrix[i][j].value=(unsigned char)t;
				}
			}
		}
	}

	//this shouldnt be here, but otherwise there are warnings
	return 0;
}


void resize(image *in, image *out) { 

	int i;

	out->type[0]='P';
	out->type[1]=in->type[1];
	out->type[2]='\0';

	// find the new dimensions
	out->width=in->width/resize_factor;
	out->height=in->height/resize_factor;
	out->maxval=in->maxval;

	//initialize matrix
	out->pixel_matrix = (pixel **) calloc(out->height, sizeof(pixel*));

	if(out->type[1]=='5'){
		for(i=0; i < out-> height; i++){
			out->pixel_matrix[i]=(pixel *) calloc(out->width, sizeof(pixel));
		}
	}
	else{
		for(i=0; i < out->height; i++){
			out->pixel_matrix[i]=(pixel *) calloc(3*out->width, sizeof(pixel));
		}

	}

	//make references for matrices so that they can be seen by each thread
	copy_out=out;
	copy_in=in;

	pthread_t tid[num_threads];
	int thread_id[num_threads];

	for(i = 0;i < num_threads; i++)
		thread_id[i] = i;

	for(i = 0; i < num_threads; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	for(i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
	}
	
}
