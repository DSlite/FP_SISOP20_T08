#include "types.h"
#include "stat.h"
#include "user.h"

int integerSqrt(int n) {
  int i = 1;
  while(i*i<=n) {
    i++;
  }
  return --i;
}

void printFactor(int n) {
  while(n%2 == 0) {
    printf(1, " %d", 2);
    n /= 2;
  }
  int end = integerSqrt(n);
  for (int i = 3; i <= end; i += 2) {
    while (n%i == 0) {
      printf(1, " %d", i);
      n/=i;
    }
  }
  if (n > 2) {
    printf(1, " %d", n);
  }
}

int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (checkInt(argv[i]) == 0) {
      printf(2, "factor: cannot calculate %s\n", argv[i]);
    } else {
      int n = atoi(argv[i]);
      printf(1, "%d:", n);
      printFactor(n);
      printf(1, "\n");
    }
  }
  exit();
}
