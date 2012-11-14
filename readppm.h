/* ----------------- */
/* Type definitions: */
/* ----------------- */
typedef char texel[3];
typedef char texelRGBA[4];

typedef struct
{
    int   texr;
    int   texc;
    texel *texture;
} TexMap;


/* -------------------- */
/* Function prototypes: */
/* -------------------- */

int ReadPPM(char *filename, TexMap *map);
