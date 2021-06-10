#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
    int read_size = 0;
    ssize_t ret;
    char *buffer;
    int fd = open("/dev/mypipe", O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("[ERROR] Fail to open pipe for reading data.\n");
        exit(1);
    }
    printf("Input a interger of how much bytes you want to read from the pipe> ");
    scanf("%d", &read_size);
    buffer = (char *)malloc(read_size * sizeof(char));
    ret = read(fd, buffer, read_size);
    if (ret > 0)
    {
        printf("Read %zd bytes from pipe, result is:\n%s\n", ret, buffer);
    }
    else
    {
        printf("Fail to read. Use command `dmesg` to debug.\n");
    }
    close(fd);
    free(buffer);
    return 0;
}