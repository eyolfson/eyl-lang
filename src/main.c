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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* POSIX */
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum { STA_MNEMONIC, STA_REGISTER, STA_NUMBER } global_state_t;

typedef enum { MNE_MOV, MNE_SYSCALL } mnemonic_id_t;
typedef struct {
    char *name;
    mnemonic_id_t id;
} mnemonic_info_t;

static const mnemonic_info_t MNEMONIC_INFO[] = {
    { "mov", MNE_MOV },
    { "syscall", MNE_SYSCALL }
};
#define MNEMONIC_INFO_SIZE (sizeof MNEMONIC_INFO / sizeof MNEMONIC_INFO[0])

typedef enum { REG_RAX, REG_RDI } reg_id_t;
typedef struct {
    char *name;
    reg_id_t id;
} register_info_t;

static const register_info_t REGISTER_INFO[] = {
    { "rax", REG_RAX },
    { "rdi", REG_RDI }
};
#define REGISTER_INFO_SIZE (sizeof MNEMONIC_INFO / sizeof MNEMONIC_INFO[0])

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

    char *input = mmap(NULL, input_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (input == MAP_FAILED) {
        perror("mmap input file");
        ret = EXIT_FAILURE;
        goto close_fd;
    }

    char *current = input;
    char *input_end = input + input_size;

    size_t machine_code_capacity = 4096;
    uint8_t *machine_code = malloc(machine_code_capacity);
    if (machine_code == NULL) {
        perror("malloc machine code");
        ret = EXIT_FAILURE;
        goto unmap_input;
    }
    uint8_t *machine_code_current = machine_code;
    /* TODO - make machine_code realloc if it runs out of capacity */

    char *start = NULL;
    global_state_t state = STA_MNEMONIC;
    bool is_valid = false;
    while (current != input_end) {
        switch (state) {
        case STA_MNEMONIC:
        case STA_REGISTER:
            is_valid = *current >= 'a' && *current <= 'z';
            break;
        case STA_NUMBER:
            is_valid = *current >= '0' && *current <= '9';
        }

        if (is_valid && start == NULL) {
            start = current;
        }

        else if (!is_valid && start != NULL) {
            size_t size = current - start;

            switch (state) {
            case STA_MNEMONIC:
                for (size_t i = 0; i < MNEMONIC_INFO_SIZE; ++i) {
                    char *name = MNEMONIC_INFO[i].name;
                    if (size == strlen(name)
                        && (strncmp(name, start, size) == 0)) {


                        switch (MNEMONIC_INFO[i].id) {
                        case MNE_MOV:
                            state = STA_REGISTER;
                            *machine_code_current = 0x48;
                            ++machine_code_current;
                            *machine_code_current = 0xc7;
                            ++machine_code_current;
                            break;
                        case MNE_SYSCALL:
                            *machine_code_current = 0x0f;
                            ++machine_code_current;
                            *machine_code_current = 0x05;
                            ++machine_code_current;
                            break;
                        }

                        printf("%s mnemonic\n", name);
                        break;
                    }
                }
                break;
            case STA_REGISTER:

                for (size_t i = 0; i < REGISTER_INFO_SIZE; ++i) {
                    char *name = REGISTER_INFO[i].name;
                    if (size == strlen(name)
                        && (strncmp(name, start, size) == 0)) {

                        switch (REGISTER_INFO[i].id) {
                        case REG_RAX:
                            *machine_code_current = 0xc0;
                            ++machine_code_current;
                            break;
                        case REG_RDI:
                            *machine_code_current = 0xc7;
                            ++machine_code_current;
                            break;
                        }
                        state = STA_NUMBER;
                        printf("%s register\n", name);
                        break;
                    }
                }
                break;
            case STA_NUMBER:
                {
                    uint32_t number = 0;
                    for (size_t i = 0; i < size; ++i) {
                        number *= 10;
                        number += *(start + i) - '0';
                    }

                    *((uint32_t*) machine_code_current) = number;
                    machine_code_current += 4;
                    printf("%d number\n", number);
                }
                state = STA_MNEMONIC;
                break;
            }

            start = NULL;
        }
        ++current;
    }
    /* need to check here as well */
    /* in case the file doesn't end with whitespace (newline) */
    if (start != NULL) {
        // TODO
    }

    size_t machine_code_size = machine_code_current - machine_code;
    printf("\ngenerated %ld bytes\n", machine_code_size);
    for (size_t i = 0; i < machine_code_size; ++i) {
        printf(" %02x", machine_code[i]);
    }
    printf("\n");
    

    free(machine_code);
 unmap_input:
    munmap(input, input_size);
 close_fd:
    close(fd);
 ret:
    return ret;
}
