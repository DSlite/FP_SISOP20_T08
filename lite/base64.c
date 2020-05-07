#include "types.h"
#include "stat.h"
#include "user.h"

char *key = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char buf[3];
void encode (int fd) {
  int n;
  while((n = read(fd, buf, 3)) > 0){
    int bits = 0;

    for (int i = 0; i < n; i++) {
      bits = bits << 8;
      bits += buf[i];
    }

    if (n == 1) {
      bits = bits << 4;
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
      printf(1, "==");
    } else if (n == 2) {
      bits = bits << 2;
      printf(1, "%c", key[(bits >> 12) & 63]);
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
      printf(1, "=");
    } else if (n == 3) {
      printf(1, "%c", key[(bits >> 18) & 63]);
      printf(1, "%c", key[(bits >> 12) & 63]);
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
    }

  }
  printf(1, "\n");
}

int main(int argc, char *argv[]) {
  int fd;
  if (argc == 1) {
    encode(0);
    exit();
  }

  for (int i = 1; i < argc; i++) {
    fd = open(argv[i], 0);
    if (fd < 0) {
      printf(2, "base64: cannot open %s\n", argv[i]);
    } else {
      struct stat st;
      if (fstat(fd, &st) < 0) {
        printf(2, "base64: cannot stat %s\n", argv[i]);
        close(fd);
        continue;
      }
      if (st.type != T_FILE) {
        printf(2, "base64: cannot encode %s\n", argv[i]);
        close(fd);
        continue;
      }
      printf(1, "%s: ", argv[i]);
      encode(fd);
      close(fd);
    }
  }
  exit();
}
