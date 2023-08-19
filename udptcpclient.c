// Bibliotheken voor Windows
#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> // voor socketprogrammering
#include <ws2tcpip.h> // voor getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> // voor fprintf, perror
#include <unistd.h> // voor close
#include <stdlib.h> // voor exit
#include <string.h> // voor memset
void OSInit(void) {
    WSADATA wsaData;
    int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (WSAError != 0) {
        fprintf(stderr, "WSAStartup foutnummer = %d\n", WSAError);
        exit(-1);
    }
}
void OSCleanup(void) {
    WSACleanup();
}
#define perror(string) fprintf(stderr, string ": WSA foutnummer = %d\n", WSAGetLastError())

// Bibliotheken voor Linux
#else
#include <sys/socket.h> // voor sockaddr, socket, socket
#include <sys/types.h> // voor size_t
#include <netdb.h> // voor getaddrinfo
#include <netinet/in.h> // voor sockaddr_in
#include <arpa/inet.h> // voor htons, htonl, inet_pton, inet_ntop
#include <errno.h> // voor errno
#include <stdio.h> // voor fprintf, perror
#include <unistd.h> // voor close
#include <stdlib.h> // voor exit
#include <string.h> // voor memset
void OSInit(void) {}
void OSCleanup(void) {}
#endif

#include <stdint.h>
#include <time.h>
#include <math.h>

// Functies voor UDP
int initializationUDP(struct sockaddr **internet_address, socklen_t *internet_address_length);
void executionUDP(int internet_socket, struct sockaddr *internet_address, socklen_t internet_address_length);
void cleanupUDP(int internet_socket, struct sockaddr *internet_address);

// Functies voor TCP
int initializationTCP();
void executionTCP(int internet_socket);
void cleanupTCP(int internet_socket);
uint16_t randomNumberTCP();
char randomOperationTCP();
void checkAnswerTCP(char buffer[100], float number1, float number2, char operation);

int main(int argc, char *argv[]) {

    srand(time(NULL));

    // START UDP
    printf("\n");
    printf("\n");
    printf("                    start UDP-client\n");
    printf("\n");

    // Opstarten van socket en verbinding
    OSInit();

    struct sockaddr *internet_address = NULL;
    socklen_t internet_address_length = 0;
    int internet_socketUDP = initializationUDP(&internet_address, &internet_address_length);

    // Instellen van time-out op 1 seconde
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(internet_socketUDP, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
        perror("setsockopt mislukt\n");
    }

    // Uitvoeren van UDP
    executionUDP(internet_socketUDP, internet_address, internet_address_length);

    // Opschonen
    cleanupUDP(internet_socketUDP, internet_address);

    OSCleanup();
    // EINDE UDP

    // START TCP
    printf("\n");
    printf("Start TCP-client\n");
    printf("\n");





    // Opstarten van TCP-socket en verbinding
    OSInit();

    int internet_socketTCP = initializationTCP();

    // Uitvoeren van TCP
    executionTCP(internet_socketTCP);

    // Opruimen
    cleanupTCP(internet_socketTCP);

    OSCleanup();
    // EINDE TCP
    
    return 0;

}







int initializationUDP(struct sockaddr **internet_address, socklen_t *internet_address_length) {
    // Opzetten van internetadres
    struct addrinfo internet_address_setup;
    struct addrinfo *internet_address_result;
    memset(&internet_address_setup, 0, sizeof internet_address_setup);
    internet_address_setup.ai_family = AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_DGRAM;
    int getaddrinfo_return = getaddrinfo("::1", "24042", &internet_address_setup, &internet_address_result);
    if (getaddrinfo_return != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
        exit(1);
    }





    // Opzetten van internetsocket
    int internet_socket = -1;
    struct addrinfo *internet_address_result_iterator = internet_address_result;
    while (internet_address_result_iterator != NULL) {
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
        if (internet_socket == -1) {
            perror("socket");
        } else {
            *internet_address_length = internet_address_result_iterator->ai_addrlen;
            *internet_address = (struct sockaddr *)malloc(internet_address_result_iterator->ai_addrlen);
            memcpy(*internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
            break;
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }





    freeaddrinfo(internet_address_result);

    if (internet_socket == -1) {
        fprintf(stderr, "socket: geen geldig socketadres gevonden\n");
        exit(2);
    }

    return internet_socket;
}

void executionUDP(int internet_socket, struct sockaddr *internet_address, socklen_t internet_address_length) {
    // Initialiseren van variabelen voor UDP-uitvoering
    int num_loops = 2;
    int num_responses = 0;

    uint16_t buffer[42];

    int number_of_bytes_received = 0;
    uint16_t highest_number = 0;
    uint16_t current_number = 0;
    uint16_t previous_number = 0;
    uint16_t highest_net_number = 0;
    int i = 0;

    // "GO" verzenden
    int number_of_bytes_send = sendto(internet_socket, "GO", 2, 0, internet_address, internet_address_length);
    if (number_of_bytes_send == -1) {
        perror("sendto");
    }

    while (num_responses < num_loops) {

        // 42 getallen ontvangen
        for (i = 0; i < 42; i++) {
            number_of_bytes_received = recvfrom(internet_socket, (char *)&(buffer[i]), sizeof(buffer[i]), 0, NULL, NULL);
            if (number_of_bytes_received == -1) {
                perror("recvfrom");
                break;
            } else {
                printf("Ontvangen: %d\n", ntohs(buffer[i]));
            }
        }




        // Als client getallen heeft ontvangen, initialiseren voor volgende set getallen
        if (i != 0) {
            num_responses++;
            highest_number = 0;

            // Zoek het hoogste getal
            for (i = 0; i < 42; i++) {
                current_number = ntohs(buffer[i]);
                if (current_number > highest_number) {
                    highest_number = current_number;
                }
            }
        }




        // Stuur het hoogste getal terug naar de server
        highest_net_number = htons(highest_number);
        number_of_bytes_send = sendto(internet_socket, (const char *)&highest_net_number, sizeof(highest_net_number), 0, internet_address, internet_address_length);
        if (number_of_bytes_send == -1) {
            perror("sendto");
            break;
        }
    }




    // "OK" ontvangen
    number_of_bytes_received = recvfrom(internet_socket, (char *)&buffer, sizeof(buffer), 0, NULL, NULL);
    if (number_of_bytes_received == -1) {
        perror("recvfrom");
    } else {
        buffer[number_of_bytes_received] = '\0';
        printf("Ontvangen: %s\n", buffer);
    }
}

void cleanupUDP(int internet_socket, struct sockaddr *internet_address) {
    // Internetadres vrijgeven
    free(internet_address);

    // Socket sluiten
    close(internet_socket);
}




int initializationTCP() {
    // Internetadres instellen
    struct addrinfo internet_address_setup;
    struct addrinfo *internet_address_result;
    memset(&internet_address_setup, 0, sizeof internet_address_setup);
    internet_address_setup.ai_family = AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_STREAM;
    int getaddrinfo_return = getaddrinfo("::1", "24042", &internet_address_setup, &internet_address_result);
    if (getaddrinfo_return != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
        exit(1);
    }



    // Socket opzetten
    int internet_socket = -1;
    struct addrinfo *internet_address_result_iterator = internet_address_result;
    while (internet_address_result_iterator != NULL) {
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
        if (internet_socket == -1) {
            perror("socket");
        } else {
            int connect_return = connect(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
            if (connect_return == -1) {
                perror("connect");
                close(internet_socket);
            } else {
                break;
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }

    freeaddrinfo(internet_address_result);

    if (internet_socket == -1) {
        fprintf(stderr, "socket: geen geldig socketadres gevonden\n");
        exit(2);
    }

    return internet_socket;
}

void executionTCP(int internet_socket) {
    // Variabelen initialiseren voor TCP-uitvoering
    uint16_t number1 = 0;
    uint16_t number2 = 0;
    char operation = '0';
    char sendOperation[15];
    int number_of_bytes_send = 0;
    int number_of_bytes_received = 0;
    char buffer[1000];
    int amountOfOperations = 0;

    // Genereer een getal tussen 1 en 20
    amountOfOperations = (rand() % 20) + 1;
    printf("Aantal bewerkingen = %d\n", amountOfOperations);

    // Genereer willekeurige getallen en bewerkingen
    for (int i = 0; i < amountOfOperations; i++) {

        number1 = randomNumberTCP();
        number2 = randomNumberTCP();
        operation = randomOperationTCP();

        // Zet alles in één string
        sprintf(sendOperation, "%d %c %d", number1, operation, number2);

        // Stuur de string
        number_of_bytes_send = send(internet_socket, (const char *)sendOperation, strlen(sendOperation), 0);
        if (number_of_bytes_send == -1) {
            perror("send");
        }

        // Ontvang antwoord en controleer het antwoord
        number_of_bytes_received = recv(internet_socket, buffer, (sizeof buffer) - 1, 0);
        if (number_of_bytes_received == -1) {
            perror("recv");
        } else {
            buffer[number_of_bytes_received] = '\0';
            printf("Ontvangen: %s\n", buffer);
        }

        checkAnswerTCP(buffer, number1, number2, operation);
    }

    // Stuur "STOP" als alle wiskundige bewerkingen zijn uitgevoerd
    number_of_bytes_send = send(internet_socket, "STOP\n", strlen("STOP\n"), 0);
    if (number_of_bytes_send == -1) {
        perror("send");
    }

    // "OK" ontvangen
    number_of_bytes_received = recv(internet_socket, buffer, (sizeof buffer) - 1, 0);
    if (number_of_bytes_received == -1) {
        perror("recv");
    } else {
        buffer[number_of_bytes_received] = '\0';
        printf("Ontvangen: %s\n", buffer);
    }
    if (strcmp(buffer, "OK\n") == 0) {
        number_of_bytes_send = send(internet_socket, "KTNXBYE\n", strlen("KTNXBYE\n"), 0);
        if (number_of_bytes_send == -1) {
            perror("send");
        }
    }
}

void cleanupTCP(int internet_socket) {
    // Socket opruimen
    int shutdown_return = shutdown(internet_socket, SD_SEND);
    if (shutdown_return == -1) {
        perror("shutdown");
    }

    close(internet_socket);
}

uint16_t randomNumberTCP() {
    // Genereer een willekeurig getal tussen 0 en 99
    uint16_t randomNumber = 0;
    randomNumber = rand() % 100;
    return randomNumber;
}

char randomOperationTCP() {
    // Genereer een willekeurig getal tussen 0 en 3 voor een willekeurige bewerking
    uint16_t randomNumberOperation = rand() % 4;
    char randomOperation = '0';
    switch (randomNumberOperation) {
        case 0:
            randomOperation = '+';
            break;
        case 1:
            randomOperation = '-';
            break;
        case 2:
            randomOperation = '*';
            break;
        case 3:
            randomOperation = '/';
            break;
    }
    return randomOperation;
}

void checkAnswerTCP(char buffer[100], float number1, float number2, char operation) {
    // Controleer de ontvangen waarde



    float result = 0;
    float expectedVal = 0;

    switch (operation) {
        case '+':
            expectedVal = number1 + number2;
            break;
        case '-':
            expectedVal = number1 - number2;
            break;
        case '*':
            expectedVal = number1 * number2;
            break;
        case '/':
            if (number2 == 0)

            {
                printf("Delen door 0\n");
            } else {
                expectedVal = number1 / (float)number2;
                expectedVal = floorf(expectedVal * 100) / 100; 
            }
            break;
        default:
            printf("Ongeldige operator\n");
    }
    result = atof(buffer);
    if (result == expectedVal)
   {
        printf("Correct\n");

    } 

     else

    {
        printf("Onjuist\n");
    }
}

