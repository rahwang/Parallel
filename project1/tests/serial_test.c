#include <stdio.h>
#include <string.h>
#include "../src/funcs.h"


/* Simple test of writem()
 * Checks that file is written
 */
int testWRITEM(int *m, int n, char *fname) {
  int i, j, read;

  writem(m, n, fname);

  FILE *dst = fopen(fname, "r");
  for (i = 0; i < 25; i++) {
    read = fscanf(dst, "%d", &j);
    if (!read) {
      return 1;
    }
  }
  fclose(dst);
  return 0;
}  


/* Simple test of readm().
 * check if read matrix matches given
 * matrix matches expected values
 */
int testREADM(int *m, int *a, int n, char *fname) {

  FILE *f = fopen(fname, "r");
  if (!f){
    fprintf(stderr, "Unable to open file\n");
    exit(1);
  }    
  
  readm(a, n, f);

  return compareM(m, a, n);
}


/* Test of floyd()
 * Compares results of matrix after running floyd
 * to the expected value */
int testFLOYD(char *f1, char *f2) {
  int n, read;

  // test matrices
  FILE *in = fopen(f1, "r");
  FILE *out = fopen(f2, "r");

  // get n value
  read = fscanf(in, "%d\n", &n);
  if (!read)
    printf("Error: no values read\n");
  
  // allocate space for the matrices
  int *a = (int *)malloc(sizeof(int)*n*n);
  int *b = (int *)malloc(sizeof(int)*n*n);
  
  // Read values into adjacency matrix
  readm(a, n, in);
  readm(b, n, out);

  // Run algorithm
  floyd(a, n);

  return compareM(a, b, n);
}


int main() {
  int n = 5;
  
  int *m = randomM(n);

  int *a =  (int *) malloc(n*n*sizeof(int));
  char *fname = "scratch.txt";
  
  int t1 = testWRITEM(m, n, fname);
  int t2 = testREADM(m, a, n, fname);
  int t3 = testFLOYD("m1.txt", "m1_out.txt");
  int t4 = testFLOYD("m2.txt", "m2_out.txt");
  int t5 = testFLOYD("m3.txt", "m3_out.txt");
  int t6 = testFLOYD("m4.txt", "m4_out.txt");
  int t7 = testFLOYD("m5.txt", "m5_out.txt");

  printf("\nTESTS for serial_floyd.c\n\n");

  printf("| %25s: %20s |\n", "WRITEM", res(t1));
  printf("| %25s: %20s |\n", "READM", res(t2));
  printf("| %25s: %20s |\n", "FLOYD1 (n = 5)", res(t3));
  printf("| %25s: %20s |\n", "FLOYD2 (n = 10)", res(t4));
  printf("| %25s: %20s |\n", "FLOYD3 (n = 100)", res(t5));
  printf("| %25s: %20s |\n", "FLOYD4 (all 0)", res(t6));
  printf("| %25s: %20s |\n", "FLOYD4 (all INF)", res(t7));

  return 0;
}
