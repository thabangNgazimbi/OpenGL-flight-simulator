/* ----------------------------------------------------------------- */
/* Texture Map Demonstration - PPM Reader                            */
/* Indiana University                                                */
/* 1 November , 1997                                                 */
/*                                                                   */
/* This file is a modification from the code used by Alan Keahey     */
/* and Eric Wernert in their C482 class taught at Indiana Univ.      */
/* ----------------------------------------------------------------- */
/* modified by Mitja Hmeljak for B481 - Spring 2009 */
/* ------------------------------------------------------------------ */
/*                                                                    */
/* if no texture file is available or readable,
   don't just exit the program: return with error code:  return(-1);  */
/*                                                                    */
/* ------------------------------------------------------------------ */


#include <stdlib.h>		/* defines NULL = 0L  */
#include <stdio.h>
#include <string.h>             /* defines strcpy */
#include <ctype.h>
#ifndef __glut_h__
    /* GL includes: glut.h  also takes care of gl.h and glu.h */
    #if defined (__APPLE__) || defined(MACOSX) /* GLUT/ on Mac OS X */
      #include <GLUT/glut.h>
    #else                                      /* GL/ on IRIX */
      #include <GL/glut.h>
    #endif
#endif
#include <math.h>

#include "readppm.h"


/* ---------- */
/* File type: */
/* ---------- */
enum mapType
{
    ppmRawType,
    pgmRawType,
    unknownType
};


/* ----------------------- */
/* Some useful functions:  */
/* ----------------------- */
void FlushLine(FILE *fp)
{
    char inputLine[72];
    fgets(inputLine, 70, fp);
}

void NextToken(FILE *fp, char *tokenread)
{
    char token[72];

    fscanf(fp, "%s", token);

    while(token[0] == '#')
    {
	FlushLine(fp);
	fscanf(fp, "%s", token);
    }

    strcpy(tokenread, token);
}

int GetNextInt(FILE *fp)
{
    int  i;
    char token[72];

    NextToken(fp, token);
    sscanf(token, "%d", &i);
    return i;
}

int GetValue(FILE *fp)
{
    return fgetc(fp);
}

int GetRGB(FILE *fp, int *rgb)
{
    int i;

    for (i = 0; i < 3; i++)
	rgb[i] = fgetc(fp);

    return 1;
}

int GetTypeFromFile(FILE *fp)
{
    char token[72];

    NextToken(fp, token);

    if((token[0] == 'P')&&(token[1] == '6'))
	return ppmRawType;
    else if((token[0] == 'P')&&(token[1] == '5'))
	return pgmRawType;
    else
	fprintf(stderr,"Unknown File format\n");

  return unknownType;
}

/* ----------------------------------------------------------- */
/* This function ties all this together to read in a PPM file: */
/* ----------------------------------------------------------- */
int ReadPPM(char *filename, TexMap *map) {
    int   i, j, type;
    int   numRows, numCols;
    int   newRows, newCols;
    int   maxVal, rgb[3];
    FILE  *fp;
    texel *data;

    /* Open the RGB file for reading: */
    fp = fopen(filename, "rb");

    if (fp == NULL) {
	printf("ERROR: Cannot open file %s\n", filename);
	return(-1); /* don't just exit the program: return with error code! */
    }

    type    = GetTypeFromFile(fp);
    numCols = GetNextInt(fp);
    numRows = GetNextInt(fp);
    maxVal  = GetNextInt(fp);
    fgetc(fp);

    printf("Reading %s with %d rows, %d cols, %d max val\n",
	   filename, numRows, numCols, maxVal);

    /* Allocate space for the texture map: */
    map->texture = (texel*) malloc ((numRows * numCols) * sizeof (texel));

    /* Read the file!: */
    for (i = 0; i < numRows; i++)
	for (j = 0; j < numCols; j++)
	{
	    GetRGB(fp, rgb);
	    map->texture[(numCols * i) + j][0] = rgb[0];
	    map->texture[(numCols * i) + j][1] = rgb[1];
	    map->texture[(numCols * i) + j][2] = rgb[2];
	}

    /* Close the file: */
    fclose(fp);

    newRows = numRows;
    newCols = numCols;

    /* Do something weird so that it is always a power of 2: */
    if(numRows <= 2)          newRows = 2;
    else if(numRows <= 4)     newRows = 4;
    else if(numRows <= 8)     newRows = 8;
    else if(numRows <= 16)    newRows = 16;
    else if(numRows <= 32)    newRows = 32;
    else if(numRows <= 64)    newRows = 64;
    else if(numRows <= 128)   newRows = 128;
    else if(numRows <= 256)   newRows = 256;
    else if(numRows <= 512)   newRows = 512;
    else if(numRows > 512)    newRows = 512;

    if(numCols <= 2)          newCols = 2;
    else if(numCols <= 4)     newCols = 4;
    else if(numCols <= 8)     newCols = 8;
    else if(numCols <= 16)    newCols = 16;
    else if(numCols <= 32)    newCols = 32;
    else if(numCols <= 64)    newCols = 64;
    else if(numCols <= 128)   newCols = 128;
    else if(numCols <= 256)   newCols = 256;
    else if(numCols <= 512)   newCols = 512;
    else if(numCols > 512)    newCols = 512;

    if ((newRows != numRows) ||
	(newCols != numCols))
    {
	printf("Rescaling to %d x %d\n", newRows, newCols);
	
	/* Allocate temporary storage: */
	data = (texel*) malloc ((newRows * newCols) * sizeof (texel));

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
	gluScaleImage(GL_RGB, 
		      numCols, numRows, GL_UNSIGNED_BYTE, map->texture,
		      newCols, newRows, GL_UNSIGNED_BYTE, data);
    
	numRows = newRows;
	numCols = newCols;
      
	free(map->texture);
	map->texture = data;
    }

    map->texr = numRows;
    map->texc = numCols;
    return (numRows * numCols);
} /* ReadPPM() */
