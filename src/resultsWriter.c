#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <errno.h>
#include <stdarg.h>
#include "main.h"


static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}

static int save_png_to_file (bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
   
    int status = -1;
    
    int pixel_size = 3;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    

    png_set_IHDR (png_ptr,
                  info_ptr,
                  bitmap->width,
                  bitmap->height,
                  depth,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    

    row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; y++) {
        png_byte *row = 
            png_malloc (png_ptr, sizeof (u_int8_t) * bitmap->width);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; x++) {
            pixel_t * pixel = pixel_at (bitmap, x, y);
            *row++ = pixel->color;
        }
    }
    

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    

    status = 0;
    
    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;
}



static int pix (int value, int max)
{
    if (value < 0) {
        return 0;
    }
    return (int) (256.0 *((double) (value)/(double) max));
}

int writeImage(floatPixelMatrix matrizPix, char * fileOut){
    bitmap_t pngOut;
    int xg;
    int yg;
    int status;
    status = 0;

    /* Se crean las struct */
    
    pngOut.width = matrizPix.n;
    pngOut.height = matrizPix.m;
    pngOut.pixels = calloc (pngOut.width * pngOut.height, sizeof (pixel_t));
    if (! pngOut.pixels) {
    return -1;
    }
    //Se recorre y crea una matriz con pixeles
    for (yg = 0; yg < pngOut.height; yg++) {
        for (xg = 0; xg < pngOut.width; xg++) {
            pixel_t * pixel = pixel_at (& pngOut, xg, yg);
            pixel->color = (int) matrizPix.matrix[yg][xg];
        }
    }

    /* Se escribe la imagen */

    if (save_png_to_file (& pngOut, fileOut)) {
        fprintf (stderr, "Error escribiendo archivo.\n");
        status = -1;
    }

    free (pngOut.pixels);
    return status;
}

int main(int argc, char **argv){
    int cValue = 0;
    int bFlag = 0;
    int flag;
    while( (flag = getopt(argc, argv, "c:b:")) != -1){
        switch (flag)        {
        case 'c':
            cValue = atoi(optarg);
            break;
        case 'b':
            bFlag = atoi(optarg);
            break;
        
        default:
            abort();
        }
    }

    if(bFlag == 1){
        printf("|   image   | nearly black |\n");
        printf("|-----------|--------------|\n");
    }
    
    for(int i= 0; i<cValue; i++){
        char fileName[20];
        char index[14];
        strcpy(fileName, "out_");
        sprintf(index,"%d",i+1);
        strcat(fileName,index);

        floatPixelMatrix floatPixels;
        read(STDIN_FILENO, &floatPixels, sizeof(floatPixelMatrix));

        if(bFlag == 1){
            if(floatPixels.nearlyBlack==1){
                printf("|  imagen_%d |     yes      |\n",i+1);
            }
            else if(floatPixels.nearlyBlack==0){
                printf("|  imagen_%d |      no      |\n",i+1);
            }
        }
        int status = writeImage(floatPixels, fileName);

    }
    
    
    /*
    for(int i= 0; i<cValue; i++){
        //pixelMatrix pixels;
        floatPixelMatrix floatPixels;
        printf("read\n");
        read(STDIN_FILENO, &floatPixels, sizeof(floatPixelMatrix));



        printf("(m,n): (%d,%d)\n", floatPixels.m,floatPixels.n);
        for(int i = 0; i< floatPixels.m;i++){
            for(int j = 0;j<floatPixels.n;j++){
                printf("%f ", (floatPixels.matrix)[i][j]);
            }
            printf("\n");
        }
    }
    */
    
    return 0;
}