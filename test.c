#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    remove("test.dat");

    int fd = open("test.dat", O_RDWR | O_CREAT, 0755);

    write(fd, "test", strlen("test"));

    close(fd);

}
