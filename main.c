#include <dirent.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define _POSIX_SOURCE -> is that needed? [02:55]
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  pid_t current_pid = getpid();

  struct dirent *dirent;
  DIR *proc_dir = opendir("/proc");

  while ((dirent = readdir(proc_dir)) != NULL) {
    // there is no process 0
    int pid = strtol(dirent->d_name, NULL, 10);
    if (pid && pid != current_pid) {
      char maps_path[32];
      char mem_path[32];

      sprintf(maps_path, "/proc/%s/maps", dirent->d_name);
      sprintf(mem_path, "/proc/%s/mem", dirent->d_name);

      printf("maps -> %s\n", maps_path);

      // maps
      FILE *maps = fopen(maps_path, "r");
      if (maps == NULL) {
        perror("[ERROR] couldn't open maps file");
        continue;
      }

      int mem = open(mem_path, O_RDWR);
      if(mem == -1){
        perror("[ERROR] couldn't open maps file");
        fclose(maps);
        continue;
      }

      // FILE *mem = fopen(mem_path, "r");
      // if (mem == NULL) {
      //   perror("[ERROR] couldn't open maps file");
      //   // if maps is readable but mem is not, check if that's possible
      //   fclose(maps);
      //   continue;
      // }

      size_t line_length = 128;
      int bytes_read = 0;
      char *map_line = NULL;

      while ((bytes_read = getline(&map_line, &line_length, maps)) != -1) {
        // TODO: not sure how to store the start and the end
        char start_str[16];
        char end_str[16];

        // use strtol directly and pass end_ptr?
        sscanf(map_line, "%12s-%12s", start_str, end_str);
        printf("[ADDRESS] %s-%s\n", start_str, end_str);

        long int start = strtol(start_str, NULL, 16);
        long int end = strtol(end_str, NULL, 16);

        // SEEK_SET or SEEK_CUR?
        lseek(mem, start, SEEK_SET);
        int size = end - start;
        char* data = malloc(size + 1);

        read(mem, data, size);
        printf("%s \n", data);

        free(data);

        printf("%ld-%ld \n", start, end);
      };

      free(map_line);
      fclose(maps);
      // fclose(mem);
    }
  }

  printf("PID %d \n", current_pid);
  closedir(proc_dir);
  return 0;
}
