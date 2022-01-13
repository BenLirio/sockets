#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500

int
main(int argc, char *argv[])
{
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int sfd, s, j;
   size_t len;
   ssize_t nread;
   char buf[BUF_SIZE];
   char *host = "0.0.0.0";
   char *port = "3000";
   char *message = "Hello from Client\n";

   /* Obtain address(es) matching host/port */

   // Zero out hints
   memset(&hints, 0, sizeof(struct addrinfo));
   // Use either IPv4 or IPv6
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   // 
   hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */

   // result is an addrinfo
   s = getaddrinfo(host, port, &hints, &result);
   if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
   }

   /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully connect(2).
      If socket(2) (or connect(2)) fails, we (close the socket
      and) try the next address. */

   // Rp is the res_pointer
   for (rp = result; rp != NULL; rp = rp->ai_next) {
       // First create a socket. This does not send packets.
       sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
       if (sfd == -1)
           continue;

       // Connect to the socket using the ai_addr
       if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
           break;                  /* Success */

       // If there was a failed attempt, then close the current sfd
       close(sfd);
   }

   // No address was found
   if (rp == NULL) {               /* No address succeeded */
       fprintf(stderr, "Could not connect\n");
       exit(EXIT_FAILURE);
   }

   // Addresses can not be found
   freeaddrinfo(result);           /* No longer needed */

   /* Send remaining command-line arguments as separate
      datagrams, and read responses from server */

   len = strlen(message) + 1;
           /* +1 for terminating null byte */

   if (len > BUF_SIZE) {
       fprintf(stderr,
               "Ignoring long message in argument %d\n", j);
   } else {
       if (write(sfd, message, len) != len) {
           fprintf(stderr, "partial/failed write\n");
           exit(EXIT_FAILURE);
       }

       nread = read(sfd, buf, BUF_SIZE);
       if (nread == -1) {
           perror("read");
           exit(EXIT_FAILURE);
       }
       printf("Received %zd bytes: %s\n", nread, buf);
   }

   exit(EXIT_SUCCESS);
}

