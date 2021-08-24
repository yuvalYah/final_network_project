#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "select.h"

#define TRUE (1)
#define FALSE (0)

static fd_set rfds, rfds_copy;
static int max_fd = 0;
static int initialized = FALSE;
static int *alloced_fds = NULL;
static int alloced_fds_num = 0;


static int add_fd_to_monitoring_internal(const unsigned int fd)
{
  int *tmp_alloc;
  tmp_alloc = (int*)realloc(alloced_fds, sizeof(int)*(alloced_fds_num+1));
  if (tmp_alloc == NULL)
    return -1;
  alloced_fds = tmp_alloc;
  alloced_fds[alloced_fds_num++]=fd;
  FD_SET(fd, &rfds_copy);
  if (max_fd < fd)
    max_fd = fd;

  return 0;
}

int init()
{
  FD_ZERO(&rfds_copy);
  if (add_fd_to_monitoring_internal(0) < 0)
    return -1; // monitoring standard input
  initialized = TRUE;
  return 0;
}

int add_fd_to_monitoring(const unsigned int fd)
{
  if (!initialized)
    init();
  if (fd>0)
    return add_fd_to_monitoring_internal(fd);
  return 0;
}

int wait_for_input()
{
  int i, retval;
  memcpy(&rfds, &rfds_copy, sizeof(rfds_copy));
  retval = select(max_fd+1, &rfds, NULL, NULL, NULL);
  if (retval > 0)
  {
    for (i=0; i<alloced_fds_num; ++i)
    {
      if (FD_ISSET(alloced_fds[i], &rfds))
        return alloced_fds[i];
    }
  }
  return -1;
}

//function we added to remove fd from the monitoring
int remove_fd_from_monitoring(int fd){
    for(int i=0; i<alloced_fds_num; ++i)
      {
        if ( alloced_fds[i] == fd) 
          {
            for (int j  = i ; j < alloced_fds_num -1; j++){
                  alloced_fds[j] = alloced_fds[j+1];
            }
            break;
          }
      }

    int *realloc_temp = (int*)realloc(alloced_fds, sizeof(int)*(alloced_fds_num-1));
    if (realloc_temp == NULL) return -1;

    alloced_fds = realloc_temp;
    alloced_fds_num--;

    FD_CLR(fd, &rfds_copy);

    return 0;
}