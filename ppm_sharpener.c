#include <stdio.h>
#include <stdlib.h>

#define HISTOGRAM 5

typedef struct {
    unsigned char red, green, blue;
} Pixel;

typedef struct {
    int width, height, intenzity;
    Pixel* pixels;
} Image;

void saveImage(const char* fileName, Image* image){
    FILE* pFile = fopen(fileName, "wb+");

    fprintf(pFile, "P6\n%d\n%d\n%d\n", image->width, image->height, image->intenzity);
    fwrite(image->pixels, 3 * image->width, image->height, pFile);

    fclose(pFile);
}

void loadImage(const char* fileName, Image* image){
    FILE* pFile = fopen(fileName, "rb");

    if (getc(pFile) != 'P' || getc(pFile) != '6'){
        fprintf(stderr, "Error: Wrong image format!\n");
        exit(1);
    }
    
    fscanf(pFile, "%d %d %d", &image->width, &image->height, &image->intenzity);
    while (getc(pFile) != '\n');

    image->pixels = malloc(image->width * image->height * 3);
    fread(image->pixels, 3 * image->width, image->height, pFile);

    fclose(pFile);
}

void saveHistogram(const char* fileName, int* histogram){
    FILE* pFile = fopen(fileName, "w+");

    int lastIndex = HISTOGRAM - 1;
    for (int i = 0; i < lastIndex; i++){
        fprintf(pFile, "%d ", histogram[i]);
    }
    fprintf(pFile, "%d", histogram[lastIndex]);

    fclose(pFile);
}

static inline void addHistogram(Pixel p, int* histogram){
    int grey = (int)(
        0.2126 * (float)p.red +
        0.7152 * (float)p.green +
        0.0722 * (float)p.blue +
        0.5
    );

    if (grey <= 152){
        if (grey <= 101){
            if (grey <= 50){
                histogram[0] += 1;
            } else {
                histogram[1] += 1;
            }
        } else {
            histogram[2] += 1;
        }
    } else {
        if (grey <= 203){
            histogram[3] += 1;
        } else {
            histogram[4] += 1;
        }
    }
}

static inline int checkInterval(int n){
    if (n < 0){
        return 0;
    } else if (n > 255){
        return 255;
    }

    return n;
}

void sharpenImage(Image* image, int* histogram){
    Pixel* sharpPixels = malloc(image->width * image->height * 3);

    for (int i = 1; i < image->height; i++){
        int index = image->width * i;

        sharpPixels[index] = image->pixels[index];
        sharpPixels[index - 1] = image->pixels[index - 1];

        addHistogram(sharpPixels[index], histogram);
        addHistogram(sharpPixels[index - 1], histogram);
    }

    register int index = image->width * image->height - 1;
    for (int i = 0; i < image->width - 1; i++){
        sharpPixels[i] = image->pixels[i];
        sharpPixels[index - i] = image->pixels[index - i];

        addHistogram(sharpPixels[i], histogram);
        addHistogram(sharpPixels[index - i], histogram);
    }    

    for (int row = 1; row < image->height - 1; row++){
        for (int col = 1; col < image->width - 1; col++){
            index = image->width * row + col;

            Pixel p0 = image->pixels[index];
            Pixel p1 = image->pixels[index - 1];
            Pixel p2 = image->pixels[index + 1];
            Pixel p3 = image->pixels[index + image->width];
            Pixel p4 = image->pixels[index - image->width];

            int red = (int)(5 * p0.red - p1.red - p2.red - p3.red - p4.red);
            red = checkInterval(red);
            sharpPixels[index].red = red;

            int green = (int)(5 * p0.green - p1.green - p2.green - p3.green - p4.green);
            green = checkInterval(green);
            sharpPixels[index].green = green;

            int blue = (int)(5 * p0.blue - p1.blue - p2.blue - p3.blue - p4.blue);
            blue = checkInterval(blue);
            sharpPixels[index].blue = blue;

            addHistogram(sharpPixels[index], histogram);
        }
    }

    free(image->pixels);
    image->pixels = sharpPixels;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Invalid arguments!\n");
        exit(1);
    }

    const char* fileName = argv[1];
    Image image;
    loadImage(fileName, &image);
    
    int histogram[HISTOGRAM] = {0};
    sharpenImage(&image, histogram);

    saveHistogram("output.txt", histogram);
    saveImage("output.ppm", &image);

    free(image.pixels);
    return 0;
}
