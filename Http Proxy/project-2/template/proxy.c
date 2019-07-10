#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>

#include "utils.h"

extern int     sendrequest(int sd);
extern char *  readresponse(int sd);
extern void    forwardresponse(int sd, char *msg);
extern int     startserver();

#define _MAX_SIZE_ 10
/*int fd_arr[_MAX_SIZE_];


static int add_fd_arr(int fd)
{
  //fd add to fd_arr
  int i=0;
  for(;i<_MAX_SIZE_;++i)
  {
    if(fd_arr[i]==-1)
    {
      fd_arr[i]=fd;
      return 0;
    }
  }
  return 1;
}*/



main(int argc, char *argv[])
{
  int servsock;    /* server socket descriptor */
  //printf("start \n");

  fd_set livesdset, servsdset;   /* set of live client sockets and set of live http server sockets */
  /* TODO: define largest file descriptor number used for select */
  int fd_max;
  struct pair * table = malloc(sizeof(struct pair)); /* table to keep client<->server pairs */

  char *msg;

  /* check usage */
  if (argc != 1) {
    fprintf(stderr, "usage : %s\n", argv[0]);
    exit(1);
  }

  /* get ready to receive requests */
  //printf("123\n");
  servsock = 1025;
  servsock = startserver();
  if (servsock == -1) {
    exit(1);
  }

  printf("%d\n", servsock);


  table->next = NULL;

  /* TODO: initialize all the fd_sets and largest fd numbers */
  FD_ZERO(&livesdset);
  FD_ZERO(&servsdset);

  //printf("start while loop\n");

  
  while (1) {
    int frsock;

    /* TODO: combine livesdset and servsdset 
     * use the combined fd_set for select */
    fd_max = 3;
    fd_set currset;
    FD_ZERO(&currset);
    FD_SET(servsock, &currset);
    FD_SET(3, &currset);
    for(int fd=0; fd<_MAX_SIZE_; ++fd){
        if ((FD_ISSET(fd, &livesdset)) || (FD_ISSET(fd, &servsdset))) {
        FD_SET(fd, &currset);
        printf("%d\n", fd);
        if(fd>fd_max)
          fd_max = fd;
      }
    }


    
    
    printf("largest SOCK is %d\n", fd_max);
    printf("start  select\n");
      /* TODO: select from the combined fd_set */
    select(fd_max+1,&currset,NULL,NULL,NULL);
    //printf("%d \n",result);
    /*if(result <0) {
        fprintf(stderr, "Can't select.\n");
        continue;
    }*/
    printf("finish select\n");
      /* TODO: iterate over file descriptors */
    for (frsock=3; frsock <=  _MAX_SIZE_; frsock++) {
         printf("%d times\n", frsock);

        if (frsock == servsock) continue;

        /*if(FD_ISSET(frsock, &currset))
          printf("sock located.\n");
        if(FD_ISSET(frsock, &livesdset))
          printf("input from existing client 1st.\n");
        else
          printf("error in first situation\n");*/

        /*############################ TODO: input from existing client? */





	if(FD_ISSET(frsock, &currset) && FD_ISSET(frsock, &livesdset)) {
	    /* forward the request */
      printf("input from existing client\n");
	    int newsd = sendrequest(frsock);
      printf("the sock created for server is %d\n",newsd);
            if (!newsd) {
	        printf("admin: disconnect from client\n");
		/*TODO: clear frsock from fd_set(s) */
          FD_CLR(frsock, &livesdset);
        //  FD_CLR(frosck, &currset);
	    } else {
          printf("the sock created for server is %d\n",newsd);
	        insertpair(table, newsd, frsock);
		/* TODO: insert newsd into fd_set(s) */
          FD_SET(newsd,&servsdset);
          //FD_SET(newsd,&currset);
	    }
	} 

  if(FD_ISSET(frsock, &currset))
              printf("2nd: found in current\n");
  if(FD_ISSET(frsock, &servsdset))
              printf("2nd: found in servsdset\n");


  /* ############################TODO: input from the http server? */
	if(FD_ISSET(frsock, &currset) && FD_ISSET(frsock, &servsdset)) {
    printf("input from the http server\n");
	   	char *msg;
	        struct pair *entry=NULL;	
		struct pair *delentry;
		msg = readresponse(frsock);
	   	if (!msg) {
		    fprintf(stderr, "error: server died123\n");
        	    exit(1);
		}
		
		/* forward response to client */
		entry = searchpair(table, frsock);
		if(!entry) {
		    fprintf(stderr, "error: could not find matching clent sd\n");
		    exit(1);
		}

		forwardresponse(entry->clientsd, msg);
		delentry = deletepair(table, entry->serversd);

		/* TODO: clear the client and server sockets used for 
		 * this http connection from the fd_set(s) */
    FD_CLR(entry->clientsd, &livesdset);
    FD_CLR(frsock, &servsdset);
        }
    }

    /* input from new client*/
    printf("check connection request \n");
    if(FD_ISSET(servsock, &currset)) {
       printf("input from new client\n");
	struct sockaddr_in caddr;
      	socklen_t clen = sizeof(caddr);
      	int csd = accept(servsock, (struct sockaddr*)&caddr, &clen);

	if (csd != -1) {
	    /* TODO: put csd into fd_set(s) */
    FD_SET(csd,&livesdset);
    printf("%d\n",csd);
	} else {
	    perror("accept");
            exit(0);
	}
    }
  }
}