#include <argp.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

const char *argp_program_version = "v1.0.0";
const char *argp_prgram_bug_address = "/dev/null";

static char args_doc[] = "string_to_search";
static char doc[] =
    "memsteal -- a simple program to look for a string in running processes";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"color", 'c', 0, 0, "Produce colorful output"},
    {0}};

struct arguments {
  char *search_str;
  int verbose, colorful;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {

  case 'v':
    arguments->verbose = 1;
    break;
  case 'c':
    arguments->colorful = 1;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 1)
      argp_usage(state);
    arguments->search_str= arg;
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
      argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

void *memmem(const void *haystack, size_t hlen, const void *needle,
             size_t nlen) {
  int needle_first;
  const void *p = haystack;
  size_t plen = hlen;

  if (!nlen)
    return NULL;

  needle_first = *(unsigned char *)needle;

  while (plen >= nlen && (p = memchr(p, needle_first, plen - nlen + 1))) {
    if (!memcmp(p, needle, nlen))
      return (void *)p;

    p++;
    plen = hlen - (p - haystack);
  }

  return NULL;
}

typedef struct {
  unsigned long start;
  unsigned long end;
} mem_offset;

mem_offset *parse_offset(char *map_line) {
  char *end_ptr;
  unsigned long int start = strtol(map_line, &end_ptr, 16);
  // map_line starts with ffff..-aaaa.. thus the end_ptr++ to skip the
  end_ptr++;
  unsigned long int end = strtol(end_ptr, NULL, 16);

  mem_offset *offset = malloc(sizeof(mem_offset));
  offset->start = start;
  offset->end = end;

  return offset;
}

void parse_mem(int mem, FILE *maps, int pid) {
  size_t line_length = 128;
  int bytes_read = 0;
  char *map_line = NULL;

  while ((bytes_read = getline(&map_line, &line_length, maps)) != -1) {
    mem_offset *offset = parse_offset(map_line);

    lseek(mem, offset->start, SEEK_SET);
    unsigned long int size = offset->end - offset->start;

    char *data = malloc(size);
    read(mem, data, size);

    char *search_str = "password";
    unsigned int str_length = strlen(search_str);

    char *match = memmem(data, size, search_str, str_length);

    if (match != NULL) {
      printf("PID: %d\n", pid);
      printf(ANSI_COLOR_RED);
      for (int i = 0; i < 300; i++) {
        if (isascii(match[i]))
          putchar(match[i]);
        // color only the searched string
        if (i == str_length)
          printf(ANSI_COLOR_RESET);
      }
      putchar('\n');
    }

    free(offset);
    free(data);
  };

  free(map_line);
}

int main(int argc, char **argv) {


  struct arguments arguments;
  arguments.colorful = 0;
  arguments.verbose = 0;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  printf("input -> %s \n", arguments.search_str);

  exit(0);
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

      FILE *maps = fopen(maps_path, "r");
      if (maps == NULL) {
        perror("[ERROR] couldn't open maps file");
        continue;
      }

      int mem = open(mem_path, O_RDWR);
      if (mem == -1) {
        perror("[ERROR] couldn't open mem file");
        fclose(maps);
        continue;
      }

      parse_mem(mem, maps, pid);

      fclose(maps);
      close(mem);
    }
  }

  closedir(proc_dir);
  return 0;
}
