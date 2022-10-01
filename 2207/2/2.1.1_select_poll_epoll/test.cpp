#include <sys/socket.h>

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    return 0;
}