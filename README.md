# memsteal
A program to steal memory by reading the "maps" file for every process and then looking for the given string in the appropriate segments of the "mem" file.

---
### Compile 
```bash
gcc main.c -o main
```
### Help
You can run the program with the `--help` flag to see all the available options:
```
./main --help

Usage: main [OPTION...] string_to_search
memsteal -- a simple program to look for a string in running processes

  -c, --color                Produce colorful output
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

### Sample output
![image](https://github.com/S460s/memsteal/assets/66819532/ade38079-1954-4649-a8e9-242d1bb98c80)

