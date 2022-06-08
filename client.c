#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char message[256];
char buffer[256];
char appoggio[256];

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (argc != 3)
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

    while (1)
    {
        memset(message, '\0', sizeof(message));
        memset(appoggio, '\0', sizeof(appoggio));
        memset(buffer, '\0', sizeof(buffer));

        returnStatus = read(simpleSocket, message, sizeof(message));

        if (returnStatus > 0)
        {
            if (strstr(message, "OK PERFECT") != NULL)
            {
                printf("%s", message + 11);

                break;
            }
            else if (strstr(message, "OK") != NULL)
            {
                memset(buffer, '\0', sizeof(buffer));

                printf("%s", message + 5);
                fgets(buffer, 249, stdin);

                memset(message, '\0', sizeof(message));

                if (strstr(buffer, "QUIT") != NULL)
                {
                    strcpy(appoggio, "QUIT\n");

                    write(simpleSocket, appoggio, strlen(appoggio));

                    returnStatus = read(simpleSocket, message, sizeof(message));

                    if (returnStatus > 0)
                    {
                        printf("%s", message + 5);
                    }

                    break;
                }
                else
                {
                    strcpy(appoggio, "WORD ");
                    strcat(appoggio, buffer);

                    write(simpleSocket, appoggio, strlen(appoggio));
                }
            }
            else if (strstr(message, "ERR") != NULL)
            {
                printf("%s", message + 4);

                break;
            }
            else if (strstr(message, "END") != NULL)
            {
                printf("%s", message + 6);

                break;
            }
            else if (strstr(message, "QUIT") != NULL)
            {
                printf("%s", message + 5);

                break;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    close(simpleSocket);

    return 0;
}

