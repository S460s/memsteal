#include <dirent.h>
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
      char process_path[32];
      sprintf(process_path, "/proc/%s", dirent->d_name);
    
      DIR *process_dir = opendir(process_path);
      struct dirent* process_dirent;
      while((process_dirent = readdir(process_dir)) != NULL){
        printf("%s \n", process_dirent->d_name);
      }

    }

  }

  closedir(proc_dir);
  return 0;
}
