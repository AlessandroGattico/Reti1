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
char appoggio[256];

int main(int argc, char *argv[])
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    int quit = 0;

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
        memset(messaggio, '\0', sizeof(messaggio));
        memset(appoggio, '\0', sizeof(appoggio));

        returnStatus = read(simpleSocket, messaggio, sizeof(messaggio));

        if (returnStatus > 0)
        {
            if (strstr(messaggio, "OK PERFECT") != NULL)
            {
                printf("%s\n", messaggio + 11);
                quit = 1;
            }
            else if (strstr(messaggio, "OK") != NULL)
            {
                memset(buffer, '\0', sizeof(buffer));

                printf("%s\n", messaggio + 5);
                fgets(buffer, 249, stdin);

                memset(messaggio, '\0', sizeof(messaggio));
                memset(appoggio, '\0', sizeof(appoggio));

                if (strstr(buffer, "QUIT") != NULL)
                {
                    strcpy(appoggio, "QUIT\n");

                    write(simpleSocket, appoggio, strlen(appoggio));

                    returnStatus = read(simpleSocket, messaggio, sizeof(messaggio));

                    printf("%s", messaggio + 5);

                    quit = 1;
                }
                else
                {
                    strcpy(appoggio, "WORD ");
                    strcat(appoggio, buffer);

                    write(simpleSocket, appoggio, strlen(appoggio));

                    memset(messaggio, '\0', sizeof(messaggio));
                    memset(buffer, '\0', sizeof(buffer));
                    memset(appoggio, '\0', sizeof(appoggio));
                }
            }
            else if (strstr(messaggio, "ERR") != NULL)
            {
                printf("%s\n", messaggio + 4);

                quit = 1;
            }
            else if (strstr(messaggio, "END") != NULL)
            {
                printf("%s\n", messaggio + 6);

                quit = 1;
            }
            else if (strstr(messaggio, "QUIT") != NULL)
            {
                printf("%s\n", messaggio + 5);

                quit = 1;
            }
            else
            {
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

