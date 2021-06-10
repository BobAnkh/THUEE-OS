#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 128

#define OK 0
#define NO_INPUT 1

static int getLine(char *prmpt, char *buff, size_t sz)
{
    int ch, extra;
    if (prmpt != NULL)
    {
        printf("%s", prmpt);
        fflush(stdout);
    }
    if (fgets(buff, sz, stdin) == NULL)
        return NO_INPUT;

    if (buff[strlen(buff) - 1] != '\n')
    {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? SIZE : OK;
    }

    buff[strlen(buff) - 1] = '\0';
    return OK;
}

int main(void)
{
    int ret;
    ssize_t res;
    char buffer[SIZE];

    int fd = open("/dev/mypipe", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("[ERROR] Fail to open pipe for writing data.\n");
        exit(1);
    }
    printf("Enter a string(should be less than limit, %d) you want to write to pipe", SIZE);
    ret = getLine("> ", buffer, sizeof(buffer));
    if (ret == NO_INPUT)
    {
        printf("\nNo input\n");
        return 1;
    }
    if (ret == SIZE)
    {
        printf("Input too long, exceeding the limit %d\n", SIZE);
        return 1;
    }
    res = write(fd, &buffer, strlen(buffer));
    if (res > 0)
    {
        printf("Write %zd bytes to pipe\n", res);
    }
    else
    {
        printf("Fail to write. Use command `dmesg` to debug.\n");
    }
    close(fd);
    return 0;
}