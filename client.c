#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char messaggio[256];
char buffer[256];


int main(int argc, char *argv[])
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    int quit = 0;
    char *p;

    if (3 != argc)
    {
        exit(1);
    }

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {
        exit(1);
    }

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    //bzero(&simpleServer, sizeof(simpleServer));
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *) &simpleServer, sizeof(simpleServer));

    if (returnStatus != 0)
    {
        close(simpleSocket);
        exit(1);
    }

    while (quit == 0)
    {
        returnStatus = read(simpleSocket, messaggio, sizeof(messaggio));

        if (returnStatus > 0)
        {
            p = strtok(messaggio, " ");

            int attempts = 0;

            if (strcmp(p, "OK") == 0)
            {
                p = strtok(NULL, " ");

                attempts = atoi(p);

                p = strtok(NULL, " ");
                printf("%s\n", p);

                if (attempts > 0)
                {
                    scanf("%s", buffer);

                    if (strstr(buffer, " ") != 0 || strstr(buffer, "\n") != 0)
                    {
                        strcpy(messaggio, "WORD ");
                        strcat(messaggio, buffer);
                        strcat(messaggio, "\n");

                        write(simpleSocket, messaggio, strlen(messaggio));

                        strcpy(messaggio, "");
                    }
                }
                else
                {
                    quit = 1;
                }
            }
            else if (strcmp(p, "QUIT") == 0)
            {
                break;
            }
            else if (strcmp(p, "END") == 0)
            {
                p = strtok(NULL, " ");

                attempts = atoi(p);

                p = strtok(NULL, " ");
                strcpy(buffer, p);

                printf("%s\n", buffer);

                strcpy(buffer, "");

                quit = 1;
            }
            else if (strcmp(p, "ERR") == 0)
            {
                p = strtok(NULL, " ");
                strcpy(buffer, p);

                printf("%s\n", buffer);
                strcpy(buffer, "");

                quit = 1;
            }
        }
        else
        {
            quit = 1;
        }
    }

    close(simpleSocket);

    return 0;
}

