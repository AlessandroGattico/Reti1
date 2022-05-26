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

    returnStatus = read(simpleSocket, messaggio, sizeof(messaggio));

    if (returnStatus > 0)
    {
        printf("%s", messaggio + 4);
        fgets(buffer, 249, stdin);
        strcpy(appoggio, "WORD ");
        strcat(appoggio, buffer);
        write(simpleSocket, appoggio, strlen(appoggio));
    }
    else
    {
        close(simpleSocket);
    }

    while (quit == 0)
    {
        returnStatus = read(simpleSocket, messaggio, sizeof(messaggio));

        if (returnStatus > 0)
        {
            if (strstr(messaggio, "OK PERFECT") != NULL)
            {
                printf("%s", messaggio + 10);
                quit = 1;
            }
            else if (strstr(messaggio, "OK") != NULL)
            {
                printf("%s", messaggio + 4);
                fgets(buffer, 249, stdin);

                if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "QUIT") == 0)
                {
                    strcpy(appoggio, "QUIT");
                    strcat(appoggio, '\0');
                    write(simpleSocket, appoggio, strlen(appoggio));
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
                printf("%s", messaggio + 4);
                quit = 1;
            }
            else if (strstr(messaggio, "END") != NULL)
            {
                printf("%s", messaggio + 5);
                quit = 1;
            }
            else if (strstr(messaggio, "QUIT") != NULL)
            {
                printf("%s", messaggio + 6);
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

