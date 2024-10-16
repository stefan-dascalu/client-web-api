#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    char *cookie, *token;
    cookie = token = NULL;
    
    char buffer[BUFLEN]; // buffer to store the user input

    while (1) {
        memset(buffer, 0, BUFLEN);  // clear the buffer
        fgets(buffer, BUFLEN - 1, stdin); // read user input
        buffer[strcspn(buffer, "\n")] = '\0';

        if (parse_command(buffer) == CMD_EXIT)
            break;
        else
            handle_command(buffer, &cookie, &token);
    }

    return 0;
}
