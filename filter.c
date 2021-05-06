#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned char r, g, b;
} pixelRGB;

typedef struct {
	unsigned int width, height, max;
	pixelRGB **image;
} PPM;

typedef struct {
	int size, divisor;
	int **array;
} kernel;

PPM *readFile(char *nameOfFile) {
	FILE *inputF;
	inputF = fopen(nameOfFile, "r");

	if (!inputF) {
		printf("Unable to open file: %s\n", nameOfFile);
		return NULL;
	}

	char fileType[3];
	fscanf(inputF, "%s\n", fileType);
	if (strcmp(fileType, "P3")) return NULL;

	PPM *ppm = malloc(sizeof(PPM));

	if (fscanf(inputF, "%u %u %u", &ppm->width, &ppm->height, &ppm->max) != 3) return NULL;

	ppm->image = malloc(sizeof(pixelRGB *)*ppm->height);

	int i, j;
	for (i = 0; i < ppm->height; i++)
		ppm->image[i] = malloc(sizeof(pixelRGB)*ppm->width);
	
	for (i = 0; i < ppm->height; i++) {
		for (j = 0; j < ppm->width; j++) {
			int pixels_read = fscanf(inputF, "%hhu %hhu %hhu", &(ppm->image[i][j].r), &(ppm->image[i][j].g), &(ppm->image[i][j].b));
			if (pixels_read != 3) return NULL;
		}
	}

	fclose(inputF);
	return ppm;
}

kernel *readKernel(char *nameOfFile) {
	FILE *kFile;
	kFile = fopen(nameOfFile, "r");

	kernel *kernel = malloc(sizeof(kernel));
	
	int size, divisor;
	fscanf(kFile, "%d\n", &size);
	fscanf(kFile, "%d\n", &divisor);

	if (size % 2 != 1) return NULL;

	int array[size][size];

	kernel->array = malloc(size);
	
	int h, i, j;
	for (h = 0; h < size; h++)
		kernel->array[h] = malloc(size);

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			fscanf(kFile, "%d", &array[i][j]);
			kernel->array[i][j] = array[i][j];
		}
	}

	kernel->size = size;
	kernel->divisor = divisor;
	
	fclose(kFile);
	return kernel;
}

PPM *makeOutPPM(PPM *ppmIn) {
	PPM *ppmOut = malloc(sizeof(PPM));

	ppmOut->width = ppmIn->width;
	ppmOut->height = ppmIn->height;
	ppmOut->max = ppmIn->max;

	ppmOut->image = malloc(sizeof(pixelRGB *)*ppmOut->height);
	
	int i, j;
	for (i = 0; i < ppmOut->height; i++)
		ppmOut->image[i] = malloc(sizeof(pixelRGB)*ppmOut->width);

	for (i = 0; i < ppmOut->height; i++) {
		for (j = 0; j < ppmOut->width; j++) {
			ppmOut->image[i][j].r = 0;
			ppmOut->image[i][j].g = 0;
			ppmOut->image[i][j].b = 0;
		}
	}
	return ppmOut;
}

void applyKernel(PPM *ppmIn, kernel *kernel, PPM *ppmOut) {
	int accumulatorRed, accumulatorGreen, accumulatorBlue;
	int ppmMaxVal = (int) ppmOut->max;

	int i, j, k, l;
	for (i = 0; i < ppmOut->height; i++) {
		for (j = 0; j < ppmOut->width; j++) {
			accumulatorRed = 0;
			accumulatorGreen = 0;
			accumulatorBlue = 0;
			for (k = 0; k < kernel->size; k++) {
				for (l = 0; l < kernel->size; l++) {
					int heightIndexImage = i-((kernel->size)/2)+k;
					int widthIndexImage = j-((kernel->size)/2)+l;
					if (heightIndexImage >= 0 && heightIndexImage < ppmOut->height && widthIndexImage >= 0 && widthIndexImage < ppmOut->width) {
						int redIntValue = (int) ppmIn->image[heightIndexImage][widthIndexImage].r;
						int greenIntValue = (int) ppmIn->image[heightIndexImage][widthIndexImage].g;
						int blueIntValue = (int) ppmIn->image[heightIndexImage][widthIndexImage].b;
						
						accumulatorRed += (redIntValue * (kernel->array[k][l])) / (kernel->divisor);
						accumulatorGreen += (greenIntValue * (kernel->array[k][l])) / (kernel->divisor);
						accumulatorBlue += (blueIntValue * (kernel->array[k][l])) / (kernel->divisor);
					}
				}
			}
			if (accumulatorRed > ppmMaxVal) accumulatorRed = ppmMaxVal;
			if (accumulatorGreen > ppmMaxVal) accumulatorGreen = ppmMaxVal;
			if (accumulatorBlue > ppmMaxVal) accumulatorBlue = ppmMaxVal;
			
			if (accumulatorRed < 0) accumulatorRed = 0;
			if (accumulatorGreen < 0) accumulatorGreen = 0;
			if (accumulatorBlue < 0) accumulatorBlue = 0;

			ppmOut->image[i][j].r = (unsigned char) accumulatorRed;
			ppmOut->image[i][j].g = (unsigned char) accumulatorGreen;
			ppmOut->image[i][j].b = (unsigned char) accumulatorBlue;
		}
	}				
}

void writeFile(PPM *ppm, char *nameOfFile) {
	FILE *outputF;
	outputF = fopen(nameOfFile, "w");
	
	fprintf(outputF, "P3\n");
	fprintf(outputF, "%u %u\n%u\n", ppm->width, ppm->height, ppm->max);
	
	int i, j;
	for (i = 0; i < ppm->height; i++) {
		for (j = 0; j < ppm->width; j++) {
			fprintf(outputF, "%u %u %u ", ppm->image[i][j].r, ppm->image[i][j].g, ppm->image[i][j].b);
		}
		fprintf(outputF, "\n");
	}
	fclose(outputF);
}

int main(int argc, char** argv) {
	if (argc != 4) {
		printf("Usage: ./filter input.ppm kernal output.ppm\n");
		return -1;
	}

	char* inFile = argv[1];
	char* kFile = argv[2];
	char* outFile = argv[3];
	
	PPM *ppmIn;
	PPM *ppmOut;
	kernel *kernel;
	ppmIn = readFile(inFile);
	kernel = readKernel(kFile);
	ppmOut = makeOutPPM(ppmIn);
	applyKernel(ppmIn, kernel, ppmOut);
	writeFile(ppmOut, outFile);
	
	return 0;
}
