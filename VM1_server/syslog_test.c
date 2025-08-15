#include <syslog.h> // Biblioteka do obsługi systemowego dziennika zdarzeń
#include <unistd.h> // Biblioteka zawierająca funkcje do zarządzania procesami

int main(int argc, char **argv) {
    // Ustawienie maski dziennika zdarzeń na poziomie LOG_NOTICE i wyższych
    //setlogmask(LOG_UPTO(LOG_NOTICE));
    setlogmask(LOG_UPTO(LOG_INFO));
	
    // Otwarcie połączenia z systemowym dziennikiem zdarzeń
    // Parametry:
    //   argv[0] - nazwa programu, która zostanie wykorzystana jako identyfikator w dzienniku
    //   LOG_CONS - powiadomienie o błędach zostanie przekierowane do konsoli
    //   LOG_PID - do każdego komunikatu zostanie dodany identyfikator procesu
    //   LOG_NDELAY - nawiązanie połączenia z dziennikiem jest natychmiastowe
    //   LOG_LOCAL7 - używany poziom dziennika
    openlog(argv[0], LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL7);
    
    // Zapisanie komunikatu o uruchomieniu programu przez danego użytkownika
    syslog(LOG_NOTICE, "Program started by User %d", getuid());
    
    // Zapisanie bieżącego kodu błędu do dziennika
    syslog(LOG_NOTICE, "ERRNO = %m");
    
    // Zapisanie informacyjnego komunikatu do dziennika
    syslog(LOG_INFO, "A tree falls in a forest");
    
    // Zapisanie komunikatu o błędzie do dziennika
    syslog(LOG_ERR, "A big tree falls in a forest");
    
    // Zamknięcie połączenia z systemowym dziennikiem zdarzeń
    closelog();
    
    return 0;
	}
