/*******************************************************************************
Copyright 2015 Jonathan Eyolfson

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

/* C */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* POSIX */
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct { char *ptr; size_t size; } token_t;

char opcode[][3] = { {'m', 'o', 'v'},
                     {'s', 'v', 'c'} };

int main(int argc, char **argv)
{
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    int ret = EXIT_SUCCESS;
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("opening input file");
        ret = EXIT_FAILURE;
        goto ret;
    }

    size_t input_size;
    {
        struct stat stat;
        if (fstat(fd, &stat) == -1) {
            perror("stating input file");
            ret = EXIT_FAILURE;
            goto close_fd;
        }
        input_size = stat.st_size;
    }

    char *input = mmap(NULL, input_size, PROT_READ, MAP_SHARED, fd, 0);
    if (input == MAP_FAILED) {
        perror("mmap input file");
        ret = EXIT_FAILURE;
        goto close_fd;
    }

    char *current = input;
    char *input_end = input + input_size;
    char *ins_start = NULL;
    while (current != input_end) {
        bool is_alpha = *current >= 'a' && *current <= 'z';
        if (is_alpha && ins_start == NULL) {
            ins_start = current;
        }
        else if (!is_alpha) {
            if ((current - ins_start) == 3) {
                printf("possible instruction\n");
            }
            ins_start = NULL;
        }
        ++current;
    }

 unmap_input:
    munmap(input, input_size);
 close_fd:
    close(fd);
 ret:
    return ret;
}
