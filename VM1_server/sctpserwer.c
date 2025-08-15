#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include 	<unistd.h>
 #include <netinet/sctp.h>
 #include <pthread.h>


#define MAXLINE 1024

//#define SA struct sockaddr

#define LISTENQ 2

void log_message(const char *message) { //jako argument przyjmuje wiadomość - dane od klienta
    char log_file[256]; //nazwa pliku
    snprintf(log_file, sizeof(log_file), "mojelogi.log"); //do wartości logfile przyporządkowuję nazwę mojelogi.log

    FILE *file = fopen(log_file, "a"); //otwieram ten plik w trybie dopisywania (append)
    if (file) { //jeśli plik został otwarty
        fprintf(file, "%s\n", message); //do pliku zapisz wiadomość
        fclose(file); //zamknij plik
    } else { //jeśli nie
        perror("Failed to open log file"); //nie można otworzyć pliku
    }
}

struct thread_arg { //struktura o charakterze wątku przechowująca:
    int connfd; //deskryptor połączenia
    struct sockaddr_in cliaddr; //adres klienta
};

void *handle_client(void *arg) { // funkcja handle_client typu void - może wskazywać na dowolny typ danych, trzeba ją rzutować
    struct thread_arg *targ = (struct thread_arg *)arg; //rzutuje argument arg (wątku) na wskaźnik do struktury thread_arg
    struct sockaddr_in cliaddr = targ->cliaddr; //deklaruję zmienną cliaddr, targ jest wskaźnikiem do cliaddr, która została przekierowana jako argument do handle_client
    //czyli mój cliaddr typu sockaddr_in to będzie cliaddr przekazany w strukturze thread_arg i mam to wskazywane przez strukturę targ
    char buff[MAXLINE];
    int flags;
    ssize_t n;

    free(targ); //zwalnia pamięć przydzieloną dla argumentu wątku (ten przekazywany w argumencie funkcji handle_client)

    char str[INET_ADDRSTRLEN+1];
    inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)); //konwertuje IP klienta na format tekstowy i wypisuje w standardowe wyjście
    //inet_ntop(rodzina, referencja do adresu klienta, string z linijki wyżej, wielkość)
    printf("Connection from %s\n", str); //wypisuje na ekran od kogo otrzymał połączenie

    while ((n = sctp_recvmsg(connfd, buff, sizeof(buff), NULL, 0, NULL, &flags)) > 0) {
        //sctp(deksryptor gniazda, bufor w którym będzie przechowywana wiadomość, rozmiar, wskaźnik do struktury z adresem nadawcy, wielkość, wskaźnik do doadtkowych wiadomości, flagi)
        buff[n] = '\0';  // Upewnij się, że bufor jest zakończony zerem
        log_message(buff);  // Zapisz wiadomość do pliku logu - wywoływana jest funkcja log z argumentem buff - danymi odebranymi od klienta
    }

    if (n < 0) {
        fprintf(stderr, "read error: %s\n", strerror(errno));
    }

    close(connfd); //zamyka połączenie z klientem
    return NULL;
}



int main(int argc, char **argv) {
    int listenfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr; //struktura adresowa klienta i serwera

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0) { //standardowe gniazdo nasłuchujące (rodzina adresów, gniazdo strumieniowe, protokół - domyślnie tcp)
    //socket zwraca deskryptor pliku - liczbę, która definiuje gniazdo
        fprintf(stderr, "socket error : %s\n", strerror(errno));
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr)); //wyzeruj strukturę adresową
    servaddr.sin_family = AF_INET; //rodzina ipv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //host to network - z postaci hosta na postać sieciową
    servaddr.sin_port = htons(13); /* daytime server */ //tak samo

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { //wiażę gniazdo listenfd z adresem serwera
        fprintf(stderr, "bind error : %s\n", strerror(errno));
        return 1;
    }

    if (listen(listenfd, LISTENQ) < 0) { //nasłuchuję na tym gnieździe, LISTENQ - długość kolejki
        fprintf(stderr, "listen error : %s\n", strerror(errno));
        return 1;
    }

    fprintf(stderr, "Waiting for clients ... \n");

    for (;;) { //nieskończona pętla, dopóki program nie zakończy działania
        len = sizeof(cliaddr);
        int *connfd = malloc(sizeof(int)); //funkcja rezerwująca pamięć na gniazdo połączone
        if ((*connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            //chcemy nawiązać połaczenie - żeby mieć stan estblished
            fprintf(stderr, "accept error : %s\n", strerror(errno));
            free(connfd);
            continue;
        }

        pthread_t tid; //wątek
        struct thread_arg *targ = malloc(sizeof(struct thread_arg)); //rezerwujemy pamieć na argumenty wątku
        targ->connfd = *connfd; //connfd w tym wątku - to moje przed momentem nawiązane
        targ->cliaddr = cliaddr;//adres klienta - ten z którym właśnie nawiązalam połączenie 

        if (pthread_create(&tid, NULL, handle_client, (void *)targ) != 0) { //utwórz wątek (wskaźnik do wątku, parametry, wskaźnik na funkcję wykonywaną przez wątek, argument przekazywany funkcji)
            fprintf(stderr, "pthread_create error: %s\n", strerror(errno));
            close(*connfd);
            free(targ);
            free(connfd);
        }

        pthread_detach(tid);  // wątek staje się samodzielny - nie trzeba się z nim synchronizować 
    }
}

