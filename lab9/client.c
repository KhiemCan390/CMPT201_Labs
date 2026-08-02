/*
Questions to answer at top of client.c:
(You should not need to change the code in client.c)
1. What is the address of the server it is trying to connect to (IP address and port number).
Ans: IP address: 127.0.0.1, port: 8000
2. Is it UDP or TCP? How do you know?:
Ans: CP because socket() uses SOCK_STREAM.
3. The client is going to send some data to the server. Where does it get this data from? How can
you tell in the code?
Ans: From the user's input: read(STDIN_FILENO, buf, BUF_SIZE)
4. How does the client program end? How can you tell that in the code?
Ans: The client stops reading and closes its socket when read() returns
1 byte or fewer: while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1), e.g. inputting an empty
line or Ctrl + D
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)

int main() {
  struct sockaddr_in addr; // holds the server's IP address and port number
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0); // creating a unconnected TCP client socket file descriptor
  // we use SOCK_STREAM, and with AF_INET + protocol 0, so the kernel would use TCP. If the domain
  // is not AF_INET, e.g. F_UNIX, then this is a socket for local inter-processes communication, not
  // TCP
  if (sfd == -1) {
    handle_error("socket");
  }

  // Preparing the server's address
  memset(&addr, 0, sizeof(struct sockaddr_in)); // clearing the structure before filling its field
  addr.sin_family = AF_INET;                    // matched with the socket's IPv4
  addr.sin_port = htons(PORT);                  // storing the port number in network byte order
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) { // convert the IP address texts into binary
                                                       // format
    handle_error("inet_pton");
  }

  // Asks the kernel to establish TCP connection from the client to the server
  // using connect() doesn't mean the connection is TCP
  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    // NOTES: edge case: if we have an input = 64 chars + one new line char,
    // that new line char is consumed at the second iteration since 1 doesn't > 1
    // thus the client exits.

    // Pressing Ctrl+D may cause read() to return 0, meaning end of file, which also ends
    // the loop

    // Let's say we input something with more than 64 bytes
    // Beside the first 64 bytes that got copied into buf,
    // the rest stays at the terminal's input buffer (not the socket's receive buffer)
    // i.e. first iteration - num_read = 64, second iteration - num_read = 7 (we input 70 chars)
    if (write(sfd, buf, num_read) != num_read) {
      // we send the num_read bytes to teh server socket
      // treat both returning -1 or else (partial write) as failure
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    // if read() failes
    handle_error("read");
  }

  // close the connection
  // closing sfd tells teh kernel that the client is finished with teh TCP connection
  // the server's read will return 0, which tells the serever that the client has closed it
  // connection
  // although exit() will close all file descriptors of the process, close() is intentional and
  // intermediate. If exit() doesn't exit immediately, socket stays open, the server's read may keep
  // blocking, expecting more dat
  close(sfd);
  exit(EXIT_SUCCESS);
}
