#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

int		count = 0, max_fds = 0;

int		ids[65536];
char	*msgs[65536];

fd_set	r_fds, w_fds, a_fds;

char	read_buf[1001], write_buf[42];


// COPIED FROM MAIN.C START
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}
// COPIED FROM MAIN.C END


void	fatal_error()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	notify_others(int author, char *msg)
{
	for (int fd = 0; fd <= max_fds; fd++)
	{
		if(FD_ISSET(fd, &w_fds) && fd != author)
			send(fd, msg, strlen(msg), 0);
	}
}

void	register_client(int fd)
{
	max_fds = fd > max_fds ? fd : max_fds;
	ids[fd] = count++;
	msgs[fd] = NULL;

	FD_SET(fd, &a_fds);
	sprintf(write_buf, "server: client %d just arrived\n", ids[fd]);
	notify_others(fd, write_buf);
}

void	remove_client(int fd)
{
	sprintf(write_buf, "server: client %d just left\n", ids[fd]);
	notify_others(fd, write_buf);
	free(msgs[fd]);
	FD_CLR(fd, &a_fds);
	close(fd);
}

void	send_message(int fd)
{
	char *msg;

	while (extract_message(&(*msgs[fd]), &msg))
	{
		sprintf(write_buf, "client %d: ", ids[fd]);
		notify_others(fd, write_buf);
		notify_others(fd, msg);
		free(msg);
	}
}

int		create_socket()
{
	max_fds = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fds < 0)
		fatal_error();
	FD_SET(max_fds, &a_fds);
	return (max_fds);
}

int main(int ac, char **av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	FD_ZERO(&a_fds);
	struct sockaddr_in servaddr;

	int sockfd = create_socket();
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		fatal_error();
	if (listen(sockfd, SOMAXCONN) != 0)
		fatal_error();

	while (1)
	{
		r_fds = w_fds = a_fds;

		if (select(max_fds + 1, &r_fds, &w_fds, NULL, NULL) < 0)
			fatal_error();
		
		for (int fd = 0; fd <= max_fds; fd++)
		{
			if (!FD_ISSET(fd, &r_fds))
				continue;
			
			if (fd == sockfd)
			{
				int client_fd = accept(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
				if (client_fd >= 0)
				{
					register_client(fd);
					break;
				}
			}
			else
			{
				int read_bytes = recv(fd, read_buf, 1000, 0);
				if (read_bytes <= 0)
				{
					remove_client(fd);
					break;
				}
				read_buf[read_bytes] = '\0';
				msgs[fd] = str_join(msgs[fd], read_buf);
				send_message(fd);
			}
		}
	}
	return (0);
}