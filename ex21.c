#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * open_files - Open two files and retrieve their file descriptors.
 *
 * @param pathOne: The path to the first file.
 * @param pathTwo: The path to the second file.
 * @param fileOne: A pointer to an integer to store the file descriptor of the first file.
 * @param fileTwo: A pointer to an integer to store the file descriptor of the second file.
 *
 * @return: 0 if both files were successfully opened, 1 otherwise.
 */
int open_files(char *pathOne, char *pathTwo, int *fileOne, int *fileTwo) {
    *fileOne = open(pathOne, O_RDONLY);
    *fileTwo = open(pathTwo, O_RDONLY);

    if (*fileOne == -1 && *fileTwo == -1) {
        printf("Error: Failed to open file(s)\n");
        return 1;
    }

    if (*fileOne == -1) {
        printf("Error: Failed to open file %s\n", pathOne);
        close(*fileTwo);
        return 1;
    }

    if (*fileTwo == -1) {
        printf("Error: Failed to open file %s\n", pathTwo);
        close(*fileOne);
        return 1;
    }

    return 0;
}

/**
 * checkLowerCase - Check if two characters are the same letter, ignoring case.
 *
 * @param c1: The first character to compare.
 * @param c2: The second character to compare.
 *
 * @return: 1 if c1 and c2 are the same letter (ignoring case), 0 otherwise.
 */
int checkLowerCase(char c1,  char c2){
    if (tolower(c1) == tolower(c2)) {
        return 1; // c1 and c2 are the same letter
    } else {
        return 0; // c1 and c2 are different letters
    }
}

/**
 * main - Compare the contents of two files character by character.
 *
 * @param argc: The number of command-line arguments.
 * @param argv: An array of command-line argument strings.
 *
 * @return: 0 if the files are identical, 1 if they differ, 2 if an error occurred.
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Error: Not enough arguments\n");
        return -1;
    }

    char *pathOne = argv[1];
    char *pathTwo = argv[2];

    int fileOne;
    int fileTwo;

    int result = open_files(pathOne, pathTwo, &fileOne, &fileTwo);
    if (result != 0) {
        //failure
        exit(-1);
    }

    char c1, c2;
    int bytes_read1, bytes_read2;
    do {
        bytes_read1 = read(fileOne, &c1, 1);
        bytes_read2 = read(fileTwo, &c2, 1);
        if (bytes_read1 == -1 || bytes_read2 == -1) {
            perror("Error in: read");
            exit(-1);
        }
        if (bytes_read1 == 0 && bytes_read2 == 0) {
            //finish to scan
            exit(1);
        }
        if (c1 != c2 || bytes_read1 != 1 || bytes_read2 != 1) {
            break;
        }
    } while (1);
    do {
        while (bytes_read1 != 0 && (c1 == ' ' || c1 == '\n')) {
            if (bytes_read2 == 0) {
                while (read(fileOne, &c1, 1) != 0) {
                    if ((c1 != ' ' && c1 != '\n') ) {
                        return 2;
                    }
                    
                }
                //printf("retrn 3c1\n");
                return 3;
            }
            bytes_read1 = read(fileOne, &c1, 1);
        };
        while (bytes_read2 != 0 && (c2 == ' ' || c2 == '\n')) {
            if (bytes_read1 == 0) {
                while (read(fileTwo, &c2, 1) != 0) {
                    if ((c2 != ' ' && c2 != '\n') ) {
                        return 2;
                    }
                }
                return 3;
            }
            bytes_read2 = read(fileTwo, &c2, 1);
        };       
        if (!checkLowerCase(c1, c2)) {
            close(fileOne);
            close(fileTwo);
            return 2;
        }
        bytes_read1 = read(fileOne, &c1, 1);
        bytes_read2 = read(fileTwo, &c2, 1);
        if (bytes_read1 == -1 || bytes_read2 == -1) {
            //failure
            perror("Error in: read");
            exit(-1);
        }
        if (bytes_read1 == 0 && bytes_read2 == 0) {
            close(fileOne);
            close(fileTwo);
            return 3;
        }
    } while (1);
    close(fileOne);
    close(fileTwo);
    return 2;
}
