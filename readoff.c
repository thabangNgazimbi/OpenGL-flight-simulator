
/* OFF(object file format) reader
 - Modified to read Nedges for Geomview compatibility */

/* Usage:
  Object3D object;
  char* str = "file.off";
  ReadOFF(str, &object);
  PrintObj(&object);
 */


/* OFF syntax: (simplified)
   # comment lines start with '# ' (note it must be followed by a space)
   # and no blank lines are allowed.
   OFF                            # Header keyword
   NV NF NE            # Total number of vertices and faces (edges ignored)
   x[0] y[0] z[0]                 # 3D coordinates of vertex[0]
   x[1] y[1] z[1]                 # 3D coordinates of vertex[1] 
   ...
   x[NV-1] y[NV-1] z[NV-1]        # 3D coordinates of vertex[NV-1] 
   Nv v[0] v[1] ... v[Nv-1]       # For face[0], Nv = number of vertices,
                                  # v[i] is the index(from 0 to NV-1) of 
				  # the i-th vertex of face[0]
   ...
   Nv v[0] v[1] ... v[Nv-1]       # For face[NF-1]

   example:
   OFF
   4 4
   1.0 0.0 0.0
   0.0 1.0 0.0
   0.0 0.0 1.0
   0.0 0.0 0.0
   3 0 1 2
   3 0 3 1
   3 0 2 3
   3 1 3 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readoff.h"

int comment(char *);
char * cut(char *);
void ReadOFF(char *, Object3D *);
void PrintObj(Object3D *);

/* checks if the line is a comment line, returns 1 if yes, 0 if no */
int comment(char *line) {
  char key[100];

  sscanf(line, "%s", key);
  if(!strcmp(key, "#"))
    return 1;
  else 
    return 0;
}

/* cut the first non-whitespace from a string */
char * cut(char *line) {
  int i=0;
  int n=strlen(line);

  while(line[i] == '\t' || line[i] == ' ') 
    i++;
  while(line[i] != '\t' && line[i] != ' ' && i<n) 
    i++;
  return &(line[i]);
}

void ReadOFF(char *filename, Object3D *obj) {
  FILE *in;
  char *line, *line2, key[100];
  int i, j;

  line = (char*) malloc(sizeof(char) * 100);
  in = fopen(filename, "r");
  if(in == NULL) {
    fprintf(stderr, "file %s doesn't exist.\n", filename);
    return;
  }  

/* first get keyword OFF */
  while(fgets(line, 100, in) != NULL) {
    sscanf(line, "%s", key);
    if(!strcmp(key, "OFF")) 
      break;
    else if(comment(line) == 0) { /* not a comment line or OFF */
      fprintf(stderr, "Input error: file should start with keyword OFF.\n");
      exit(1);
    }
  }

/* get Nvertices, Nfaces, read but ignore Nedges */
  while(fgets(line, 100, in) != NULL) {
    if(comment(line) == 0) /* not a comment line */
      break;
  }
  sscanf(line, "%d%d%d", &(obj->Nvertices),
	 &(obj->Nfaces),&(obj->Nedges));

  obj->vertices = (HPoint3*) malloc(sizeof(HPoint3) * obj->Nvertices);
  obj->nv_face = (int*) malloc(sizeof(int) * obj->Nfaces);
  obj->faces = (int**) malloc(sizeof(int*) * obj->Nfaces);
  
/* get 3D coordinates of all vertices */
  i = 0;
  while(i < obj->Nvertices && fgets(line, 100, in) != NULL) {
    if(comment(line) == 0) { /* not a comment line */
      sscanf(line, "%lf%lf%lf", 
	     &(obj->vertices[i].x),
	     &(obj->vertices[i].y),
	     &(obj->vertices[i].z));
      obj->vertices[i].w = 1.0;
      i++;
    }
  }
  if(i < obj->Nvertices) {
    fprintf(stderr, "Input error: %d vertices needed, only %d supplied.\n", 
	    obj->Nvertices, i);
    exit(2);
  }

  /* get vertex-index lists for each face */
  i = 0;
  while(fgets(line, 100, in) != NULL && i < obj->Nfaces) {
    if(comment(line) == 0) { /* not a comment line */
      sscanf(line, "%d",  &(obj->nv_face[i]));
      obj->faces[i] = (int *) malloc(sizeof(int) * obj->nv_face[i]);
      line2 = line;
      line = cut(line);
      for(j=0; j<obj->nv_face[i]; j++) {
	if(sscanf(line, "%d", &(obj->faces[i][j])) == (int) NULL) {
	  fprintf(stderr, "Input error: line is too short -- %s\n", line2);
	  exit(-3);
	}
	line = cut(line);
      }
      line = line2;
      i++;
    }
  }
  if(i < obj->Nfaces) {
    fprintf(stderr, "Input error: %d faces needed, only %d supplied.\n", 
	    obj->Nfaces, i);
    exit(3);
  }
}
  
void PrintObj(Object3D *obj) {
  int i, j;

  fprintf(stderr,
          "Nvertices = %d, Nfaces = %d\n", obj->Nvertices, obj->Nfaces);
  for(i=0; i<obj->Nvertices; i++) {
    fprintf(stderr,"vertices[%d] = %f %f %f %f\n", i,
	   obj->vertices[i].x, 
	   obj->vertices[i].y, 
	   obj->vertices[i].z, 
	   obj->vertices[i].w); 
  }
  for(i=0; i<obj->Nfaces; i++) {
    fprintf(stderr,"face[%d] has %d vertices: ", i, obj->nv_face[i]);
    for(j=0; j<obj->nv_face[i]; j++) 
      fprintf(stderr,"%d ", obj->faces[i][j]);
    fprintf(stderr,"\n");
  }
}


