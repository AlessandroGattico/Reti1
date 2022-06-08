#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define LEN 6
#define W 50

char words[W][LEN];
char message[256];
char appoggio[256];
int tentativi;

char *select_word(FILE *fp);
char *crea_risposta(char *buffer, char *target);

char *select_word(FILE *fp)
{
    int n;
    int i = 0;

    while (feof(fp) == 0)
    {
        fscanf(fp, "%s", words[i]);
        i++;
    }

    n = rand() % 50;

    return words[n];
}

char *crea_risposta(char *buffer, char *target)
{
    int j = 0;
    int k = 0;

    for (j = 0; j < strlen(buffer); ++j)
    {
        if (target[j] == buffer[j])
        {
            appoggio[j] = '*';
        }
        else
        {
            for (k = 0; k < strlen(target); k++)
            {
                if (buffer[j] == target[k])
                {
                    appoggio[j] = '+';
                }
                else
                {
                    appoggio[j] = '-';
                }

                if (appoggio[j] == '+')
                {
                    k = strlen(target);
                }
            }
        }
    }

    appoggio[j + 1] = '\n';

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
    char target[LEN] = "";
    FILE *fp;

    srand(time(NULL));

    if (argc < 2 || argc > 3)
    {
        exit(1);
    }
    else if (argc == 2)
    {
        tentativi = 6;
        fp = fopen("words.txt", "r");

        if (fp == NULL)
        {
            exit(1);
        }
    }
    else if (argc == 3)
    {
        tentativi = atoi(argv[2]);

        if (tentativi > 10)
        {
            tentativi = 10;
        }
        if (tentativi < 6)
        {
            tentativi = 6;
        }

        fp = fopen("words.txt", "r");

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

        simpleChildSocket = accept(simpleSocket, (struct sockaddr *) &clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);
        }

        attempts = tentativi;

        strcpy(target, select_word(fp));

        sprintf(message, "OK %d Indovina la parola\n", attempts);

        write(simpleChildSocket, message, strlen(message));

        while (1)
        {
            memset(buffer, '\0', sizeof(buffer));
            memset(appoggio, '\0', sizeof(appoggio));
            memset(message, '\0', sizeof(message));

            returnStatus = read(simpleChildSocket, message, sizeof(message));

            if (returnStatus > 0)
            {
                if (strstr(message, "QUIT") != NULL)
                {
                    strcpy(appoggio, "QUIT Vai via così presto?\n");

                    write(simpleChildSocket, appoggio, strlen(appoggio));

                    break;
                }
                else if (strstr(message, "WORD") != NULL)
                {
                    if (attempts > 0)
                    {
                        strcpy(buffer, message + 5);
                        memset(message, '\0', sizeof(message));

                        for (int i = 0; i < strlen(buffer) - 1; ++i)
                        {
                            if (!isalpha(buffer[i]))
                            {
                                strcpy(message, "ERR La parola contiene caratteri non validi\n");

                                write(simpleChildSocket, message, strlen(message));

                                break;
                            }
                            else
                            {
                                message[i] = tolower(buffer[i]);
                            }
                        }
                        if (strlen(buffer) != 6)
                        {
                            memset(message, '\0', sizeof(message));
                            strcpy(message, "ERR La parola deve essere di 5 caratteri\n");

                            write(simpleChildSocket, message, strlen(message));

                            break;
                        }

                        attempts--;

                        crea_risposta(message, target);

                        memset(message, '\0', sizeof(message));

                        if (strstr(appoggio, "*****") != NULL)
                        {
                            strcpy(message, "OK PERFECT Hai indovinato la parola!\n");

                            write(simpleChildSocket, message, strlen(message));

                            break;
                        }
                        else
                        {
                            sprintf(message, "OK %d %s\n", attempts, appoggio);

                            write(simpleChildSocket, message, strlen(message));
                        }
                    }
                    else
                    {
                        sprintf(appoggio, "END %d La parola era: %s\n", tentativi, target);

                        write(simpleChildSocket, appoggio, strlen(appoggio));

                        break;
                    }
                }

            }
            else
            {
                break;
            }
        }

        close(simpleChildSocket);
    }

    close(simpleSocket);
    return 0;
}

