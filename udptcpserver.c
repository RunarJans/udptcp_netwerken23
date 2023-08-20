// Bibliotheken voor Windows
#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> // voor socket programmering
#include <ws2tcpip.h> // voor getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> // voor fprintf, perror
#include <unistd.h> // voor close
#include <stdlib.h> // voor exit
#include <string.h> // voor memset

void OSInit( void ) // initialiseert de Windows Sockets API
{
    WSADATA wsaData;
    int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
    if( WSAError != 0 )
    {
        fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
        exit( -1 );
    }
}

void OSCleanup( void )
{
    WSACleanup();
}
#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )

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
int OSInit( void ) {}
int OSCleanup( void ) {}
#endif

#include <time.h>
#include <stdint.h>

// Functies UDP
int initialisatieUDP();
void uitvoeringUDP( int internet_socket );
void opschonenUDP( int internet_socket );
int willekeurigGetalUDP();
void stuurGetalUDP(int internet_socket,struct sockaddr_storage client_internet_adres,socklen_t client_internet_adres_lengte,int aantal_tezenden_bytes);
void ontvangGetalUDP(char buffer[1000],int internet_socket,struct sockaddr_storage client_internet_adres,socklen_t client_internet_adres_lengte,int aantal_ontvangen_bytes);

// Functies TCP
int initialisatieTCP();
int verbindingTCP( int internet_socket );
void uitvoeringTCP( int internet_socket );
void opschonenTCP( int internet_socket, int client_internet_socket );

int main( int argc, char * argv[] ) {

    srand(time(NULL));

    // UDP starten
    printf("\n");
    printf("\n");
    printf("                    start udp-server\n");
    printf("\n");

    // Initialiseren van UDP-socket en het OS
    OSInit();

    int internet_socketUDP = initialisatieUDP();

    // Uitvoeren van de servercode 
    uitvoeringUDP(internet_socketUDP);

    // Alles opschonen
    opschonenUDP(internet_socketUDP);

    OSCleanup();
    // UDP beëindigen

    // TCP starten
    printf("\n");
    printf("                    start tcp-server\n");
    printf("\n");
    printf("\n");
    // Socket opzetten
    OSInit();

    int internet_socketTCP = initialisatieTCP();

    int client_internet_socketTCP = verbindingTCP( internet_socketTCP );

    // Code uitvoeren
    uitvoeringTCP( client_internet_socketTCP );

    // Alles opruimen
    opschonenTCP( internet_socketTCP, client_internet_socketTCP );

    OSCleanup();
    // TCP beëindigen

    return 0;
}

int initialisatieUDP()
{
    // Instellen van het internetadres
    struct addrinfo internet_adres_opzet;
    struct addrinfo * internet_adres_resultaat;
    memset( &internet_adres_opzet, 0, sizeof internet_adres_opzet );
    internet_adres_opzet.ai_family = AF_UNSPEC;
    internet_adres_opzet.ai_socktype = SOCK_DGRAM;
    internet_adres_opzet.ai_flags = AI_PASSIVE;
    int getaddrinfo_resultaat = getaddrinfo( NULL, "24042", &internet_adres_opzet, &internet_adres_resultaat );
    if( getaddrinfo_resultaat != 0 )
    {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_resultaat ) );
        exit( 1 );
    }

    int internet_socket = -1;
    struct addrinfo * internet_adres_resultaat_iterator = internet_adres_resultaat;
    while( internet_adres_resultaat_iterator != NULL )
    {
        // Socket openen
        internet_socket = socket( internet_adres_resultaat_iterator->ai_family, internet_adres_resultaat_iterator->ai_socktype, internet_adres_resultaat_iterator->ai_protocol );
        if( internet_socket == -1 )
        {
            perror( "socket" );
        }
        else
        {
            // Binden
            int bind_resultaat = bind( internet_socket, internet_adres_resultaat_iterator->ai_addr, internet_adres_resultaat_iterator->ai_addrlen );
            if( bind_resultaat == -1 )
            {
                close( internet_socket );
                perror( "bind" );
            }
            else
            {
                break;
            }
        }
        internet_adres_resultaat_iterator = internet_adres_resultaat_iterator->ai_next;
    }

    freeaddrinfo( internet_adres_resultaat );

    if( internet_socket == -1 )
    {
        fprintf( stderr, "socket: geen geldig socketadres gevonden\n" );
        exit( 2 );
    }

    return internet_socket;
}

void uitvoeringUDP( int internet_socket )
{
    // Parameters voor de uitvoering declareren
    int aantal_tezenden_bytes = 0;
    int aantal_ontvangen_bytes = 0;
    char buffer[1000];
    struct sockaddr_storage client_internet_adres;
    socklen_t client_internet_adres_lengte = sizeof client_internet_adres;

    // Wachten op GO
    while (strcmp(buffer,"GO")!=0){
        aantal_ontvangen_bytes = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_adres, &client_internet_adres_lengte );
        if( aantal_ontvangen_bytes == -1 )
        {
            perror( "recvfrom" );
        }
        else
        {
            buffer[aantal_ontvangen_bytes] = '\0';
            printf( "Ontvangen: %s\n", buffer );
        }
    }

    // Time-out 3s
    struct timeval time_out;
    time_out.tv_sec = 3;
    time_out.tv_usec = 0;
    if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO, &time_out,sizeof time_out) < 0){
        perror("setsockopt mislukt\n");
    }

    // 42 willekeurige getallen verzenden
    stuurGetalUDP(internet_socket,client_internet_adres,client_internet_adres_lengte,aantal_tezenden_bytes);

    // Hoogste getal ontvangen
    ontvangGetalUDP(buffer,internet_socket,client_internet_adres,client_internet_adres_lengte,aantal_ontvangen_bytes);
    
    // Getallen verzenden
    stuurGetalUDP(internet_socket,client_internet_adres,client_internet_adres_lengte,aantal_tezenden_bytes);

    // Hoogste getal van de client ontvangen en controleren of het het hoogste getal is dat de server heeft verzonden
    ontvangGetalUDP(buffer,internet_socket,client_internet_adres,client_internet_adres_lengte,aantal_ontvangen_bytes);

    // Als de client het hoogste getal 2 keer correct heeft verzonden, stuurt de server OK terug
    aantal_tezenden_bytes = sendto( internet_socket,"OK",2, 0, (struct sockaddr *) &client_internet_adres, client_internet_adres_lengte );
    if( aantal_tezenden_bytes == -1 )
    {
        perror( "sendto" );
    }
}

void opschonenUDP( int internet_socket )
{
    // De socket sluiten
    close( internet_socket );
}

int willekeurigGetal(){
    // Genereer een willekeurig getal tussen 0 en 99
    uint16_t willekeurigGetal = 0;
    willekeurigGetal = rand() % 100;
    return willekeurigGetal;
}

void stuurGetalUDP(int internet_socket,struct sockaddr_storage client_internet_adres,socklen_t client_internet_adres_lengte,int aantal_tezenden_bytes){
    uint16_t getal = 0;
    uint16_t netto_getal = htons(getal);

    // 42 keer loopen en telkens een willekeurig getal naar de server sturen
    for(int i=0;  i<42;  i++)
    {
        getal = willekeurigGetal();// willekeurig getal genereren
        printf("Willekeurig getal = %d\n",getal);
        netto_getal = htons(getal);
        aantal_tezenden_bytes = sendto( internet_socket, (const char*)&netto_getal, sizeof(netto_getal), 0, (struct sockaddr *) &client_internet_adres, client_internet_adres_lengte );// het getal verzenden
        if( aantal_tezenden_bytes == -1 )
        {
            perror( "sendto" );
        }
    }
}

void ontvangGetalUDP(char buffer[1000],int internet_socket,struct sockaddr_storage client_internet_adres,socklen_t client_internet_adres_lengte,int aantal_ontvangen_bytes){
    // Het hoogste getal ontvangen
    aantal_ontvangen_bytes = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_adres, &client_internet_adres_lengte );
    if( aantal_ontvangen_bytes == -1 )
    {
        perror( "recvfrom" );
        printf("Fout: niets ontvangen binnen 3 seconden\n");
    }
    else
    {
        buffer[aantal_ontvangen_bytes] = '\0';
        // Het grootste getal afdrukken
        printf( "Ontvangen: %hd\n", ntohs(*((short*)buffer)) );
    }
}

int initialisatieTCP()
{
    // Het internetadres initialiseren
    struct addrinfo internet_adres_opzet;
    struct addrinfo * internet_adres_resultaat;
    memset( &internet_adres_opzet, 0, sizeof internet_adres_opzet );
    internet_adres_opzet.ai_family = AF_UNSPEC;
    internet_adres_opzet.ai_socktype = SOCK_STREAM;
    internet_adres_opzet.ai_flags = AI_PASSIVE;
    int getaddrinfo_resultaat = getaddrinfo( NULL, "24042", &internet_adres_opzet, &internet_adres_resultaat );
    if( getaddrinfo_resultaat != 0 )
    {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_resultaat ) );
        exit( 1 );
    }

    // De socket initialiseren
    int internet_socket = -1;
    struct addrinfo * internet_adres_resultaat_iterator = internet_adres_resultaat;
    while( internet_adres_resultaat_iterator != NULL )
    {
        internet_socket = socket( internet_adres_resultaat_iterator->ai_family, internet_adres_resultaat_iterator->ai_socktype, internet_adres_resultaat_iterator->ai_protocol );
        if( internet_socket == -1 )
        {
            perror( "socket" );
        }
        else
        {
            // Binden
            int bind_resultaat = bind( internet_socket, internet_adres_resultaat_iterator->ai_addr, internet_adres_resultaat_iterator->ai_addrlen );
            if( bind_resultaat == -1 )
            {
                perror( "bind" );
                close( internet_socket );
            }
            else
            {
                int listen_resultaat = listen( internet_socket, 1 );
                if( listen_resultaat == -1 )
                {
                    close( internet_socket );
                    perror( "listen" );
                }
                else
                {
                    break;
                }
            }
        }
        internet_adres_resultaat_iterator = internet_adres_resultaat_iterator->ai_next;
    }

    freeaddrinfo( internet_adres_resultaat );

    if( internet_socket == -1 )
    {
        fprintf( stderr, "socket: geen geldig socketadres gevonden\n" );
        exit( 2 );
    }

    return internet_socket;
}

int verbindingTCP( int internet_socket )
{
    // Verbinding maken met de client
    struct sockaddr_storage client_internet_adres;
    socklen_t client_internet_adres_lengte = sizeof client_internet_adres;
    int client_socket = accept( internet_socket, (struct sockaddr *) &client_internet_adres, &client_internet_adres_lengte );
    if( client_socket == -1 )
    {
        perror( "accept" );
        close( internet_socket );
        exit( 3 );
    }

    return client_socket;
}

void uitvoeringTCP(int internet_socket) {
    // Variabelen voor TCP-uitvoering initialiseren
    char buffer[1000];
    int aantal_ontvangen_bytes;
    // Blijf loopen zolang er geen fouten zijn
    while (1) {
        // Buffer wissen en het aantal ontvangen bytes resetten
        memset(buffer, 0, sizeof(buffer));
        aantal_ontvangen_bytes = 0;

        // String van de client ontvangen
        aantal_ontvangen_bytes = recv(internet_socket, buffer, (sizeof buffer) - 1, 0);
        if (aantal_ontvangen_bytes == -1) {
            perror("recv");
            break;
        } else if (aantal_ontvangen_bytes == 0) {
            printf("Client heeft de verbinding verbroken\n");
            break;
        }

        buffer[aantal_ontvangen_bytes] = '\0';

        // Als de string gelijk is aan STOP\n, stop de uitvoering en start de opschoning
        if (strcmp(buffer, "STOP\n") == 0) {
            printf("\n");
            printf("STOP ontvangen\n");
            printf("\n");

            // OK verzenden
            send(internet_socket, "OK\n", strlen("OK\n"), 0);

            aantal_ontvangen_bytes = recv(internet_socket, buffer, (sizeof buffer) -1 , 0);
            if(aantal_ontvangen_bytes == -1){
                perror("recv");
                break;
            }
            buffer[aantal_ontvangen_bytes] = '\0';

            // Als de string gelijk is aan KTNXBYE\n, start de opschoning
            if(strcmp(buffer, "KTNXBYE\n") == 0){
                printf("\n");
                printf("KTNXBYE ontvangen \n");
                printf("\n");
                break;
            }
        }
        printf("Ontvangen bericht: %s\n", buffer);

        // De operatie parsen
        int num1, num2;
        char op;
        if (sscanf(buffer, "%d %c %d", &num1, &op, &num2) != 3) 
        {
            printf("Ongeldige opmaak van de operatie\n");
            continue; 
        }

        // Berekeningen uitvoeren
        float resultaat;

        switch (op) 
        {
            case '+':
                resultaat = num1 + num2;
                break;
            case '-':
                resultaat = num1 - num2;
                break;
            case '*':
                resultaat = num1 * num2;
                break;
            case '/':
                if(num2 == 0)
                {
                    printf("Delen door 0\n");
                    int aantal_verzonden_bytes = send(internet_socket, "Delen door 0\n", strlen("Delen door 0\n"), 0);
                    if (aantal_verzonden_bytes == -1) {
                        perror("send");
                        break;
                    }
                }
                else 
                {
                    resultaat = num1 / (float) num2;
                }
                break;

            default:
                printf("Ongeldige operator\n");
                continue;
        }

        // Resultaat naar de client verzenden
        char resultaat_str[100];
        snprintf(resultaat_str, sizeof(resultaat_str), "%.2f", resultaat);
        int aantal_verzonden_bytes = send(internet_socket, resultaat_str, strlen(resultaat_str), 0);
        if (aantal_verzonden_bytes == -1) 
        {
            perror("send");
            break;
        } 
        else 
        {
            printf("Verzonden: %s\n", resultaat_str);
        }
    }
}

void opschonenTCP( int internet_socket, int client_internet_socket )
{
    // TCP-verbinding afsluiten
    int shutdown_resultaat = shutdown( client_internet_socket, SD_RECEIVE );
    if( shutdown_resultaat == -1 )
    {
        perror( "shutdown" );
    }
    
    close( client_internet_socket );
    close( internet_socket );
}
