#include <stdio.h>

// compile with: gcc -Wall math.c -o a.out

// defines the denominator of the fraction (must be square):
#define FRAC 65536
#define FRACRT 256
#define NUM(n) (n * FRAC)

// in hexadecimal, pi = 3.243F6
#define PI 0x3243F

// degree of the taylor series
#define NUM_TERMS 12


// define the "num" datatype
typedef int num;


// returns 0 if math does not work
int check_math() {
  return sizeof(num) == 4;
}

int num2int(num x) {
    return x / FRAC;
}

num int2num(int x) {
    return x * FRAC;
}

num mul(num a, num b) {
    return ((a / FRACRT) * (b / FRACRT));
}

num div(num a, num b) {
    return ((a * FRACRT) / (b / FRACRT));
}

num sine(num x) {
    num terms[NUM_TERMS];
    int i;
    
    // compute terms of exponential series
    terms[0] = NUM(1);
    for (i = 1; i < NUM_TERMS; i++) {
        terms[i] = div(mul(x, terms[i-1]), NUM(i));
    }
    return terms[1] + terms[5] + terms[9] - (terms[3] + terms[7] + terms[11]);
}



int main () {
  if (! check_math())
    return 0;
  num i = NUM(7) + (NUM(1) >> 2); // 7.25
  num j = NUM(8) + (NUM(3) >> 2); // 8.75
  num k = NUM(-3);
  
  printf("%d %d %d %d %d %d %d %d \n",
    num2int(mul(i,j)), 
    num2int(i + j), 
    num2int(256 * div(i, j)), 
    num2int(sine(PI)), 
    num2int(sine(PI / 2)),
    num2int(100 * PI),
    num2int(100 * sine(-PI/6)),
    num2int(mul(i, k)));
  printf("expect:\n");
  printf("63 16 212 0 1 314 -50 -21\n");
  return 0;
}

