#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32 // 32 connections may wait for accept()
// error macro, print message based on errno
#define handle_error(msg)                                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0; // number of message "chunks", since every successful call is counted
                             // as 1 "message". Note that one long terminal line may be split into
                             // multiple client write() calls because the client buffer is only
                             // 64 bytes
int client_id_counter = 1;

// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;       // client connected socket
  int client_id; // ID assigned by the server
};

void *handle_client(void *arg) {
  struct client_info *client = arg;
  // TODO: print the message received from client
  // TODO: increase total_message_count per message
  int cfd = client->cfd;
  int cid = client->client_id;
  free(client); // safe since it is copied
  ssize_t num_read;
  char buf[BUF_SIZE + 1]; // the last space is for the NULL char
  printf("New client created! ID %d on socket FD %d\n", cid, cfd);

  while ((num_read = read(cfd, buf, BUF_SIZE)) > 0) { // read from its connected client
                                                      // if no data currently available, and the
                                                      // client remains connected, this thread
                                                      // blocks inside read()
    buf[num_read] = '\0';
    pthread_mutex_lock(&count_mutex);
    // let's say the terminal input has 70 chars, then the client has to make 2 read() and 2 writes
    // to sfd. But for the server, based on its buffer limit, e.g. 64 or 70, it can make just one
    // read (70 bytes at once) or two read (64 bytes and then 6 bytes). For this serveer, a 70 byte
    // strem causes at least 2 increments since the count increases for each successful read()
    int msg_num = ++total_message_count;
    printf("Msg # %3d; Client ID %d: %s", msg_num, cid, buf); // keeping this printf helps prevent
                                                              // different threads' output from
                                                              // being mixed together
    if (buf[num_read - 1] != '\n') {
      printf("\n"); // add a new line if the current chunk doesn't have \n at the end
    }
    pthread_mutex_unlock(&count_mutex);
  }
  // when the client closes its socket, read() returns 0, the loop ends
  // so we have to check if the loop ended normally or not
  if (num_read == -1) {
    perror("read");
  }

  if (close(cfd) == -1) {
    perror("close");
  }

  printf("Ending thread for client %d\n", cid);
  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0); // create Ipv4 TCP socket i.e. TCP bytestream, normal
                                         // protocol
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;   // matching the socket
  addr.sin_port = htons(PORT); // the port number is up to what we chose
                               // the client and the server must connect through the matching port
                               // number
                               // the port number should be > 1023
  addr.sin_addr.s_addr = htonl(INADDR_ANY); // accept connections sent to port 800 through  any
                                            // local IPv4 interface e.g. Wifi-address:8000,
                                            // Ethernet-address:8000

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    // The kernel now knows that incoming TCP connections to port 8000 should be direceted to this
    // server socket
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    // changed the socket to a listening socket
    handle_error("listen");
  }

  for (;;) {
    // TODO: create a new thread when a new connection is encountered
    // TODO: call handle_client() when launching a new thread, and provide
    // client_info
    struct client_info *client = (struct client_info *)malloc(sizeof(struct client_info));
    if (client == NULL) {
      handle_error("malloc");
    }
    client->cfd =
        accept(sfd, NULL, NULL); // sfd is the server's listening socket descriptor
                                 // accept() creates a new conected server-side socket
                                 // while cfd is the fd for the newly connected socket
                                 // the server uses cfd to communicate with that particular
                                 // client
                                 // accept() blocks until a client connect
    if (client->cfd == -1) {
      free(client);
      handle_error("accept");
    }

    pthread_mutex_lock(&client_id_mutex);
    client->client_id = client_id_counter++;
    pthread_mutex_unlock(&client_id_mutex);
    pthread_t tid;
    if (pthread_create(&tid, NULL, handle_client, client) != 0) {
      free(client);
      handle_error("pthread create");
    }
    // when the thread is done, let the system automatically cleans it
    pthread_detach(tid);
  }
  // unreachable during normal execution
  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
