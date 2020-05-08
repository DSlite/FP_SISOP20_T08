#include "types.h"
#include "stat.h"
#include "user.h"

int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf(1, "chmod: wrong arguments\n");
        exit();
    }
    if (strlen(argv[1]) != 3) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    if (checkInt(argv[1]) == 0) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    int modeInt = atoi(argv[1]);
    int other = modeInt%10;
    int group = (modeInt/10)%10;
    int user = (modeInt/100)%10;
    if (user > 7 || group > 7 || other > 7) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    int mode = (user << 6) + (group << 3) + other;
    if (chmod(argv[2], mode)) {
        printf(1, "chmod: failed to change permission\n");
    }
    exit();
}