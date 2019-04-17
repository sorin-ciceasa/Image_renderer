#ifndef FILTER_H
#define FILTER_H

typedef struct{
	unsigned char value;
}pixel;

typedef struct {
	char type[3];
	unsigned int width; 
	unsigned int height;
	unsigned short maxval;
	pixel **pixel_matrix;
}image;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image *out);

#endif /* FILTER_H */
