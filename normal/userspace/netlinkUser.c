//Taken from https://stackoverflow.com/questions/15215865/netlink-sockets-in-c-using-the-3-x-linux-kernel?lq=1

#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>

#define NETLINK_USER 31
#define NETLINK_USER_GROUP (1<<0)

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

char DAEMON=1;
int nl_group=NETLINK_USER_GROUP;

void sigifup(int signum)
{
    (void)signum;
    DAEMON = 0;
}

int main(int argc, char **argv)
{
  fd_set rfds;
  struct timeval tv;
  int retval, max_fd=0;
  char c;

  sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
  if(sock_fd<0)
  return -1;

  /* init signal handler */
  signal(SIGUSR1, sigifup);	// exit daemon

  while ((c = getopt(argc, argv, "g:")) != -1){
    switch (c) {
    case 'g':
      nl_group=atoi(optarg);
      break;
    default:
      printf("unknown parameter(%c)\n", c);
      break;
    }
  }

  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid(); /* self pid */
  src_addr.nl_groups = nl_group;

  bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid = 0; /* For Linux Kernel */

#ifdef NETLINK_MULTICAST
  /*
   * 270 is SOL_NETLINK. See
   * http://lxr.free-electrons.com/source/include/linux/socket.h?v=4.1#L314
   * and
   * https://stackoverflow.com/questions/17732044/
   */
  if (setsockopt(sock_fd, 270/*SOL_NETLINK*/, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) < 0) {
      printf("setsockopt < 0, %s\n", strerror(errno));
      return -1;
  }
#endif

  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
  nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  nlh->nlmsg_pid = getpid();
  nlh->nlmsg_flags = 0;

  strcpy(NLMSG_DATA(nlh), "Hello");

  iov.iov_base = (void *)nlh;
  iov.iov_len = nlh->nlmsg_len;
  msg.msg_name = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  printf("[%d:%d]Sending message to kernel\n", getpid(), nl_group);
  sendmsg(sock_fd,&msg,0);
  printf("[%d:%d]Waiting for message from kernel\n", getpid(), nl_group);

  while(DAEMON){
    FD_ZERO(&rfds);
    FD_SET(sock_fd, &rfds);
    if(sock_fd > max_fd) max_fd = sock_fd+1;

    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(max_fd, &rfds, NULL, NULL, &tv);
    if (retval == -1){
      perror("select()");
    }else if (retval){
      /* Read message from kernel */
      //recvmsg(sock_fd, &msg, 0);
      recvmsg(sock_fd, &msg, MSG_DONTWAIT );
      printf("[%d:%d]Received message payload: %s\n", getpid(), nl_group, (char *)NLMSG_DATA(nlh));
    }else{
      //printf("[%d:%d]No data within five seconds.\n", getpid(), nl_group);
    }
  }
  close(sock_fd);
  printf("[%d:%d]Daemon exit.\n", getpid(), nl_group);
}
