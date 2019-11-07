#include <stdio.h>

#define VECTOR_LEN  8
#define HIST_LEN  8



unsigned int
H (unsigned int v1)
{

  int W;
  W = (v1 >> (VECTOR_LEN - 1));
  W = (W ^ (v1 % 2)) << (VECTOR_LEN - 1);
  W = W + (v1 >> 1);
  
  W = W % (1 << VECTOR_LEN);

  return W;
}

unsigned int
H_inversed (unsigned int v1)
{

  int W, k;

  if((v1 >> (VECTOR_LEN - 1) & 1) == (v1 >> (VECTOR_LEN - 2) & 1))
    {

      k = 0;

    }
  else
    {

      k = 1;

    }
    

  W = (v1 << 1) + k;
  W = W % (1 << VECTOR_LEN);
  
  return W;

}

int
main ()
{

  int f0, f1, f2, numPhtEntries, PC, ghr;

  PC = 0b10111010;
  ghr = 0b11000010;
  

  numPhtEntries = (1 << VECTOR_LEN);

  long int v = (PC << HIST_LEN) + ghr;

  int v1 = v % numPhtEntries;
  int v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  
  printf("v1 ");
  printBits(1, &v1);
  
  printf("v2 ");
  printBits(1, &v2);


  f0 = H (v1) ^ H_inversed (v2) ^ v2;
  f1 = H (v1) ^ H_inversed (v2) ^ v1;
  f2 = H_inversed (v1) ^ H (v2) ^ v2;
  
  
  printf("\nf0 ");
  printBits(1, &f0);
  
  printf("\nf1 ");
  printBits(1, &f1);
  
  printf("\nf2 ");
  printBits(1, &f2);
 
}
