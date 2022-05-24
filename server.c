#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LEN 5
#define W 50

char words[W][LEN];
char message[256];
char appoggio[5];

char *select_word(FILE *fp);

char *crea_risposta(char *buffer, char *target);

char *select_word(FILE *fp)
{
    int n;
    int i = 0;

    while (!feof(fp))
    {
        fscanf(fp, "%s", words[i]);
        i++;
    }

    n = rand() % 50;

    return words[n];
}

char *crea_risposta(char *buffer, char *target)
{
    for (int j = 0; j < strlen(buffer); ++j)
    {
        if (&target[j] == &buffer[j])
        {
            appoggio[j] = '*';
        }
        else
        {
            for (int k = 0; k < strlen(target) - 1; ++k)
            {
                if (target[j] == buffer[k])
                {
                    appoggio[j] = '+';
                }
                else
                {
                    appoggio[j] = '-';
                }
            }
        }
    }

    return appoggio;
}

int main(int argc, char *argv[])
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    char buffer[256] = "";
    int attempts;
    char target[5] = "";
    FILE *fp;

    srand(time(NULL));

    if (argc < 2 || argc > 4)
    {
        exit(1);
    }
    else if (3 == argc)
    {
        attempts = 6;
        fp = fopen(argv[3], "r");

        if (fp == NULL)
        {
            exit(1);
        }
    }
    else if (4 == argc)
    {
        attempts = atoi(argv[2]);

        if (attempts > 10)
        {
            attempts = 10;
        }
        if (attempts < 6)
        {
            attempts = 6;
        }

        fp = fopen(argv[4], "r");

        if (fp == NULL)
        {
            exit(1);
        }
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket, (struct sockaddr *) &simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Bind completed!\n");
    }
    else
    {
        fprintf(stderr, "Could not bind to address!\n");
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Cannot listen on socket!\n");
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in clientName = {0};
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);

        /* wait here */

        simpleChildSocket = accept(simpleSocket, (struct sockaddr *) &clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);
        }

        strcpy(target, select_word(fp));

        strcpy(message, "OK ");
        sprintf(message, "%d", attempts);
        strcat(message, "Indovina la parola\n");

        write(simpleChildSocket, message, strlen(message));
        strcpy(message, "");

        /* handle the new connection request  */
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

        if (returnStatus > 0)
        {
            char a[LEN + 1];

            for (int j = 0; j < 4; ++j)
            {
                a[j] = message[j];
            }

            a[LEN] = '\0';

            if (strcmp("WORD", a) == 0)
            {
                int h = 0;
                for (int i = 5; i < 10; i++)
                {
                    buffer[h] = message[i];
                    h++;
                }

                if (strlen(buffer) != 6 && strcmp(&buffer[0], " ") == 0)
                {
                    strcpy(message, "ERROR\n");
                }
                else if (strcmp(target, buffer) == 0 && attempts)
                {
                    strcpy(message, "OK PERFECT\n");
                    write(simpleChildSocket, message, strlen(message));
                    close(simpleChildSocket);
                }
                else
                {
                    strcpy(message, "OK ");
                    sprintf(message, "%d ", attempts);
                    strcat(message, crea_risposta(&buffer[0], &target[0]));
                    strcat(message, "\n");
                    write(simpleChildSocket, message, strlen(message));
                }
            }
            else if (strcmp("QUIT", a) == 0)
            {
                close(simpleChildSocket);
            }
            else
            {
                strcpy(message, "ERR condo non valido\n");
                write(simpleChildSocket, message, strlen(message));
                close(simpleChildSocket);
            }

        }
        else
        {
            fprintf(stderr, "Return Status = %d \n", returnStatus);
        }
        close(simpleChildSocket);

    }

    close(simpleSocket);
    return 0;

}

