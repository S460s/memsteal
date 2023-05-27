#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

  struct dirent *dirent;
  DIR *proc_dir = opendir("/proc");

  while ((dirent = readdir(proc_dir)) != NULL) {
    // there is no process 0
    int pid = strtol(dirent->d_name, NULL, 10);
    if (pid) {
      char maps_path[32];
      char mem_path[32];

      sprintf(maps_path, "/proc/%s/maps", dirent->d_name);
      sprintf(mem_path, "/proc/%s/mem", dirent->d_name);


      printf("maps -> %s\n", maps_path);

      // maps
      FILE* maps = fopen(maps_path, "r");
      if(maps == NULL){
        perror("[ERROR] couldn't open maps file");
        continue;
      }

      size_t line_length = 128;
      int bytes_read = 0;
      char* map_line = NULL; 
      bytes_read = getline(&map_line, &line_length, maps);

      // printf("%s\n", map_line);

      // free(map_line);
      // fclose(maps);
     
      exit(0);
    }

  }

  closedir(proc_dir);
  return 0;
}
