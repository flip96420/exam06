#if 0

#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

int		ids[65536];
char	*msgs[65536];
fd_set	r_fds, w_fds, a_fds;
char	buf_read[1001], buf_write[42];

// IN NOTIFY OTHERS
send(fd, msg, strlen(msg), 0);

// IN SEND_MSG
extract_message(&(*msgs[fd]), &msg);

// IN CREATE SOCKET
max_fd = socket(AF_INET, SOCK_STREAM, 0);

// IN MAIN
select(max_fd + 1, &rfds, &wfds, NULL, NULL);
socklen_t addr_len = sizeof(servaddr);
int client_fd = accept(sockfd, (struct sockaddr *)&servaddr, &addr_len);
int read_bytes = recv(fd, buf_read, 1000, 0);

#endif