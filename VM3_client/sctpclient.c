#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <sys/inotify.h>
#include <fcntl.h>

#define MAXLINE 1024
#define SA struct sockaddr
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

void
Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		perror("fputs error");
}


char *
Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		perror("fgets error");

	return (rptr);
}

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		perror("writen error");
}


void
//str_cli(FILE* fp, int sockfd)
str_cli(FILE *fp, int sockfd, const char *logfile)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
	int n;
	int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    // Initialize inotify
    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }
	//printf("Zainicjowane inotify\n");

    // Add watch on the log file
    wd = inotify_add_watch(fd, "/var/log/local7dir", IN_MODIFY);
    if (wd == -1) {
        fprintf(stderr, "Cannot watch :(");
        exit(EXIT_FAILURE);
    }

    // Open the log file
    FILE *log_fp = fopen(logfile, "r");
     if (!log_fp) {
       perror("fopen");
       exit(EXIT_FAILURE);
	 }

    // Seek to the end of the file
    fseek(log_fp, 0, SEEK_END);
	//printf("halo brysia");

    while (1) {
        length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

		// Check if server closed connection
        if (length == 0) {
            printf("Server closed connection. Exiting...\n");
			fflush(stdout);
            exit(EXIT_SUCCESS);
        }

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & IN_MODIFY) {
                    char sendline[MAXLINE];
					fseek(log_fp, 0, SEEK_CUR);
                    while (fgets(sendline, MAXLINE, log_fp) != NULL) {
						printf("New log: %s", sendline);
                        Writen(sockfd, sendline, strlen(sendline));
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
        i = 0;  // Reset the index for the next batch of events
    }

    // Clean up
    inotify_rm_watch(fd, wd);
    close(fd);
    fclose(log_fp);
}

int
main(int argc, char **argv)
{
	int					sockfd, n;
	struct sockaddr_in	servaddr;
	char				recvline[MAXLINE + 1];
	int err;

	if (argc != 3){
		fprintf(stderr, "ERROR: usage: a.out <IPaddress> <logfile>\n");
		return 1;
	}
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0){ //jeśli miałoby być SCTP to zamiast 0 wpisuję IPPROTO_SCTP/
		fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(13);	/* daytime server */
	if ( (err=inet_pton(AF_INET, argv[1], &servaddr.sin_addr)) <= 0){
		if(err == 0 )
			fprintf(stderr,"inet_pton error for %s \n", argv[1] );
		else
			fprintf(stderr,"inet_pton error for %s : %s \n", argv[1], strerror(errno));
		return 1;
	}
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr,"connect error : %s \n", strerror(errno));
		return 1;
	}

	//str_cli(stdin, sockfd);		/* do it all */
	str_cli(stdin, sockfd, argv[2]);

	fprintf(stderr,"Mega Brysia\n");
	fflush(stdout);

	exit(0);
}
