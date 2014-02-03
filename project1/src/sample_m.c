#include <stdio.h>
#include <stdlib.h>
#include "funcs.h"

int main(int argc, char *argv[]) {
  int i, j;
  int n = atoi(argv[1]);
  int *m = randomM(n);

  FILE *dst = fopen("scratch.txt", "w");
  if (!dst){
    fprintf(stderr, "Unable to open file\n");
    exit(1);
  }    
  
  fprintf(dst, "%i\n", n);
  for (i = 0; i < n; i++) {
    for (j = 0; j <n; j++) {
      fprintf(dst, "%i ", m[i*n + j]);
    }
    fprintf(dst, "\n");
  }
  fclose(dst);
  return 0;
}
