#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  if(argc > 1){
    printf(1, "clear: too much arguments");
    exit();
  }
  printf(1, "\e[1;1H\e[2J");
  exit();
}
