/*
===============================================================================

	BMP2C
	Written by: Shlomi Nissan
	Company: 1Byte Beta, www.1bytebeta.com (2015)

	I’ve been making an effort to relive my childhood memories by creating
	VGA (mode-13h) games for MS-DOS in C. I’ve created this tool to convert
	256-color Windows Bitmaps to C arrays. Each Bitmap produces two arrays,
	one for the pixel data and another for the palette. You can then include
	these arrays in your project, reset the palette and load the pixel data
	into the video memory in order to display the Bitmap on screen.

	Usage:

	BMP2C -i <filename.bmp> -o <filename.c>

===============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE	1
#define FALSE	0

typedef unsigned char	byte;
typedef unsigned short 	word;

typedef struct tagBitmap {

	word width;
	word height;
	byte palette[768]; // 256 * 3 = 768
	byte *data;

}BITMAP;

//

int read_bitmap(char *file, BITMAP *b);
int write_bitmap(char *file, char *source, BITMAP *b);

void fskip(FILE *fp, int num_bytes);

//

int main(int argc, char * argv[]) {
	
	char *input;
	char *output;

	int i;

	BITMAP bitmap;

	printf("\nBMP2C - visit www.1bytebeta.com for more information.\n");
	printf("Copyright (c) 2008-2015 1Byte Beta hello@1bytebeta.com\n\n");

	for (i = 1; i < argc; ++i) {

		if (argv[i][0] == '-') {

			switch(argv[i][1]) {

				case 'i':

					input = argv[i+1];

				break;

				case 'o':

					output = argv[i+1];

				break;

				break;

			}

		}

	}

	if( !input || !output ) {

		printf("Usage: BMP2C -i <filename.bmp> -o <filename.h>\n\n");
		exit(1);

	}

	if( read_bitmap(input, &bitmap) ){

		if( write_bitmap(output, input, &bitmap) ) {

			printf("something.c has been created succesfully!\n\n");

		}

	}

	return 0;

}

int read_bitmap(char *file, BITMAP *b) {

	long 	index;
	int 	x;
	FILE 	*fp;
	word	num_colors;

	if ( (fp = fopen(file, "rb")) == NULL ) {
		
		printf("Error opening file %s.\n", file);
		exit(1);

	}

	if ( fgetc(fp) != 'B' || fgetc(fp) != 'M' ) {
		
		fclose(fp);
		printf("%s is not a bitmap file.\n", file);
		exit(1);

	}

	printf("Reading the file...\n");
	
	fskip(fp,16);
	fread(&b->width, sizeof(word), 1, fp);

	fskip(fp, 2);
	fread(&b->height, sizeof(word), 1, fp);

	fskip(fp, 22);
	fread(&num_colors, sizeof(word), 1, fp);

	fskip(fp, 6);

	// assume we’re working 256 colors

	if( num_colors == 0 ) { 
		
		num_colors = 256;

	}

	// read the palette

	for( index = 0; index < num_colors; index++ ) {

		b->palette[(int)(index*3+2)] = fgetc(fp) >> 2;
		b->palette[(int)(index*3+1)] = fgetc(fp) >> 2;
		b->palette[(int)(index*3+0)] = fgetc(fp) >> 2;

		x = fgetc(fp);

	}

	// allocate memory for the data

	if( (b->data = (byte *)malloc( (word)( b->width * b->height ) ) ) == NULL ) {

		fclose(fp);
		printf("Error allocating memory for file %s.\n",file);
		exit(1);

	}

	// read the bitmap

	for( index = (b->height - 1) * b->width; index >= 0; index -= b->width ) {

		for( x=0; x<b->width; x++ ) {

			b->data[(word)index+x] = (byte)fgetc(fp);

		}

	}

	fclose(fp);

	return TRUE;

}

int write_bitmap(char *file, char *source, BITMAP *b) {

    FILE *fp;

    int len		= strlen(source);
    int counter = 0;
    char *name 	= source;
    int size 	= b->width * b->height;

    name[len-4] = '\0';

    fp = fopen(file, "w");

    if( fp == NULL ){
        printf("Failed to open file %s for writing\n", file);
        exit(1);
    }

    printf("Writing the file...\n\n");

    fprintf(fp, "//\n");
    fprintf(fp, "// File generated by BMP2C (www.1bytebeta.com) \n");
    fprintf(fp, "// Input: %s (%ix%ipx)\n", source, b->width, b->height);
    fprintf(fp, "//\n");
    fprintf(fp, "\n");

    // write the palette

    fprintf(fp, "unsigned char %s_palette[] = {\n", name);

    for (int i = 0; i < 768; ++i)
    {
    	++counter;

		if( i == 767 ) {

			fprintf(fp, "0x%04x };\n\n ", b->palette[i]);

		} else {
		
			fprintf(fp, "0x%04x, ", b->palette[i]);

		}

		if( counter == 5 ) {

			counter = 0;
			fprintf(fp, "\n");

		}

    }

    // write the data

    fprintf(fp, "unsigned char %s_data[] = {\n", name);

    counter = 0;

    for (int i = 0; i < size; ++i)
    {

    	++counter;

		if( i == (size - 1) ) {

			fprintf(fp, "0x%04x };\n\n ", b->data[i]);

		} else {
		
			fprintf(fp, "0x%04x, ", b->data[i]);

		}

		if( counter == 5 ) {

			counter = 0;
			fprintf(fp, "\n");

		}

    }

    fclose(fp);

    return TRUE;

}

void fskip(FILE *fp, int num_bytes) {

  int i;

  for( i=0; i<num_bytes; i++ ) {
    
    fgetc(fp);

  }

}
