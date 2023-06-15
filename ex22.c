#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>

#define MAX_LINE_LENGTH 200
#define NO_C_FILE 1
#define COMPILATION_ERROR 2
#define TIMEOUT 3
#define EXCELLENT 4
#define WRONG 5
#define SIMILAR 6

/**

* Writes a line to a file descriptor representing a CSV row with the results of a program.
* The line consists of three fields: the first is always empty, the second is a score value
* according to the program result, and the third is a description of the program result.
*
* @param option An integer representing the program result, according to a predefined set of options.
* @param fd The file descriptor to write the CSV line to.
*/
void writeToFile(int option, int fd)
{
    // printing to results.csv the correct option
    switch (option) {
    case NO_C_FILE:
        write(fd, ",0,NO_C_FILE\n", 13);
        break;
    case COMPILATION_ERROR:
        write(fd, ",10,COMPILATION_ERROR\n", 22);
        break;
    case TIMEOUT:
        write(fd, ",20,TIMEOUT\n", 12);
        break;
    case EXCELLENT:
        write(fd, ",100,EXCELLENT\n", 15);
        break;
    case WRONG:
        write(fd, ",50,WRONG\n", 10);
        break;
    case SIMILAR:
        write(fd, ",75,SIMILAR\n", 12);
        break;
    default:
        write(fd, "Invalid option selected\n", 24);
        break;
    }
}

/**
 * set absulote path.
 *
 * @param strings An array of three strings: the first is a directory, and the second and third are files.
 * @return 1 if all three items can be accessed and opened, 0 otherwise.
 */
void setStringsActualPath(char strings[3][MAX_LINE_LENGTH]) {
    char actualPath[MAX_LINE_LENGTH];
    char* res;
    int i = 0;
    for (i = 0; i < 3; i++) {
        res = realpath(strings[i], actualPath);
        if (res == NULL) {
            perror("Error in: realpath");
            exit(-1);
        }
        strcpy(strings[i], actualPath);
    }
}

/**
 * Checks if a directory and two files can be accessed and opened by the user.
 *
 * @param strings An array of three strings: the first is a directory, and the second and third are files.
 * @return 1 if all three items can be accessed and opened, 0 otherwise.
 */
int checkUserPathes(char strings[3][MAX_LINE_LENGTH]) {
    int dir_fd, file1_fd, file2_fd;
    struct stat dir_stat, file1_stat, file2_stat;

    // Check if directory can be accessed
    if (stat(strings[0], &dir_stat) != 0 || !S_ISDIR(dir_stat.st_mode)) {
        if(stat(strings[0], &dir_stat) != 0) {
            perror("Error in: stat");
            return 0;
        }
        write(STDERR_FILENO, "Not a valid directory\n", strlen("Not a valid directory\n"));
        return 0;
    }
    // Check if file1 can be opened
    if (stat(strings[1], &file1_stat) != 0 || !S_ISREG(file1_stat.st_mode) || access(strings[1], R_OK) != 0) {
        if(stat(strings[1], &file1_stat) != 0) {
            perror("Error in: stat");
            return 0;
        }
        write(STDERR_FILENO, "Input file not exist\n", strlen("Input file not exist\n"));
        return 0;
    }
    // Check if file2 can be opened
    if (stat(strings[2], &file2_stat) != 0 || !S_ISREG(file2_stat.st_mode) || access(strings[2], R_OK) != 0) {
        if(stat(strings[2], &file2_stat) != 0) {
            perror("Error in: stat");
            return 0;
        }
        write(STDERR_FILENO, "Output file not exist\n", strlen("Output file not exist\n"));
        return 0;
    }
    setStringsActualPath(strings);
    return 1;
}

/**
 * Reads the contents of a file and returns an array of strings containing
 * the first three lines of the file.
 *
 * @param filename The name of the file to read.
 * @param strings the strings of the files pathes
 * @return .
 *  */
void read_file(char *filename, char strings[3][MAX_LINE_LENGTH]) {
    // Open the file for reading.
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        // If there was an error opening the file, print an error message and exit.
        perror("Error in: open");
        exit(-1);
    }
    // Read the first line of the file and store it in the first string.
    int n = 0;
    char c;
    while (n < MAX_LINE_LENGTH && read(fd, &c, 1) > 0) {
        if (c == '\n') {
            break;
        }
        strings[0][n++] = c;
    }
    strings[0][n] = '\0';
    // Read the second line of the file and store it in the second string.
    n = 0;
    while (n < MAX_LINE_LENGTH && read(fd, &c, 1) > 0) {
        if (c == '\n') {
            break;
        }
        strings[1][n++] = c;
    }
    strings[1][n] = '\0';
    // Read the third line of the file and store it in the third string.
    n = 0;
    while (n < MAX_LINE_LENGTH && read(fd, &c, 1) > 0) {
        if (c == '\n') {
            break;
        }
        strings[2][n++] = c;
    }
    strings[2][n] = '\0';
    // Close the file.
    if(close(fd) == -1){
        perror("Error in: close");
    }
}

/**
 * Compiles a C file using gcc and generates an executable file.
 *
 * @param fileName The name of the C file to compile.
 * @return Returns 1 on success and 0 on failure.
 */
int compileFile(char *fileName, int erfd) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == -1) {
        // Failed to create child process
        perror("Error in: fork");
        return 0;
    }
    else if (pid == 0) {
        char *args[] = {"gcc", fileName, "-o", "b.out", NULL};
        if (dup2(erfd, STDERR_FILENO) == -1) {
            perror("Error in: dup2");
        }
        // Child process - execute gcc command
        execvp(args[0], args);
        perror("Error in: execvp");
        // If execvp returns, an error occurred
        return 0;
    }
    else {
        // Parent process - wait for child to finish
        if (waitpid(pid, &status, 0) == -1) {
            perror("Error in: waitpid");   
            return 0;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Compilation succeeded
            return 1;
        }
        else {
            // Compilation failed
            return 0;
        }
    }
}

/**
 * Searches for a C source file with the ".c" extension in a directory and returns its name.
 *
 * @param dirName The directory to search for the C file.
 * @param fileName A pointer to a char array that will store the name of the C file found.
 * @return Returns 1 if a C file is found and its name is successfully stored in fileName, or 0 otherwise.
 */
int findCFile(char *dirName, char *fileName) {
    // try to open dir
    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        perror("Error in: opendir");
        return 0;
    }
    struct dirent *entry;
    // run on the files in the dir
    while ((entry = readdir(dir)) != NULL) {
        // Check if the file name ends with ".c"
        if (strlen(entry->d_name) > 2 && strcmp(entry->d_name + strlen(entry->d_name) - 2, ".c") == 0) {
            sprintf(fileName, "%s", entry->d_name);
            if (closedir(dir) == -1) {
                perror("Error in: closedir");
                return 0;
            }
            return 1;
        }
    }
    if (closedir(dir) == -1) {
        perror("Error in: closedir");
        return 0;
    }
    return 0;
}

/**
 * Executes an external program named b.out with input from a file and redirects its output to a file named "user.txt".
 * If the program runs for more than 5 seconds, it is terminated.
 *
 * @param inputPath The path of the input file to be used by b.out.
 *
 * @return Returns 1 if the program runs successfully and 0 if it runs for more than 5 seconds.
 *         Returns -1 if an error occurred.
 */
int runBOut(char *inputPath, int erfd) {
    pid_t pid;
    int status, in_fd, out_fd, null_fd;
    char outputFilename[] = "user.txt";
    char *argv[] = {"./b.out", NULL};
    char inFilePath[MAX_LINE_LENGTH];
    in_fd = open(inputPath, O_RDONLY);
    if (in_fd < 0) {
        char errMsg[300];
        char cwd[MAX_LINE_LENGTH];
        getcwd(cwd, sizeof(cwd));
        snprintf(errMsg, 300, "Error opening file '%s' from directory '%s': %s", inputPath, cwd, strerror(errno));
        perror(errMsg);
        perror("Error in: open");
        return -1;
    }
    // Create output file
    if ((out_fd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror("Error in: open");
        if (close(in_fd) == -1) {
            perror("Error in: close");
        }
        return -1;
    }
    // Fork process
    if ((pid = fork()) < 0) {
        perror("Error in: fork");
        if (close(in_fd) == -1) {
            perror("Error in: close");
        }
        if (close(out_fd) == -1) {
            perror("Error in: close");
        }
        return -1;
    }
    else if (pid == 0) {
        // Child process
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            perror("Error in: dup2");
        }
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            perror("Error in: dup2");
        }
        if (dup2(erfd, STDERR_FILENO) == -1) {
            perror("Error in: dup2");
        }
        // Execute b.out
        execvp(argv[0], argv);
        // If execvp returns, there was an error
        perror("Error in: execvp");
        return -1;
    }
    else {
        // Parent process: Close input and output files in parent process
        if (close(in_fd) == -1) {
            perror("Error in: close");
        }
        if (close(out_fd) == -1) {
            perror("Error in: close");
        }
        // Get start time
        struct timeval start_time;
        struct timeval tv;
        if (gettimeofday(&start_time, NULL) == -1) {
            perror("Error in: gettimeofday");
        }
        // Wait for child process to finish
        while (waitpid(pid, &status, WNOHANG) == 0) {
            // Get current time
            struct timeval current_time;
            if (gettimeofday(&current_time, NULL) == -1) {
                perror("Error in: gettimeofday");
            }
            // Check if 5 seconds have passed
            if (current_time.tv_sec - start_time.tv_sec >= 5) {
                // Child process has exceeded the time limit
                if (kill(pid, SIGTERM) == -1) {
                    perror("Error in: kill");
                }
                return 0;
            }
        }
        // Check child process status
        if (WIFEXITED(status)) {
            // Child process exited normally
            return 1;
        }
        else if (WIFSIGNALED(status)) {
            // Child process terminated due to a signal
            return 1;
        }
    }
    return 1;
}

/**
*Compares the content of two files using an external program and returns the exit code of the program.

*@param outputPath The path of the first file to be compared.
*@param compPath The path of the second file to be compared.
*@return The exit code of the comparison program. Returns -1 if an error occurred.
*/
int compareBetweenFiles(char *outputPath, char *compPath, int erfd) {
    pid_t pid;
    int status;
    // get the path to the file
    char cwd[MAX_LINE_LENGTH];
    char filePath[MAX_LINE_LENGTH * 2];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Concatenate the file name to the directory path
        snprintf(filePath, MAX_LINE_LENGTH * 2, "%s/%s", cwd, "user.txt");
    }
    else {
        perror("Error in: getcwd");
        return -1;
    }
    // set the args for exec
    char *argv[] = {compPath, outputPath, filePath, NULL};
    // fork and check if succes
    if ((pid = fork()) < 0) {
        perror("Error in: fork");
        return -1;
    }
    else if (pid == 0) {
        // Child process
        if (dup2(erfd, STDERR_FILENO) == -1) {
            perror("Error in: dup2");
        }
        execvp(argv[0], argv);
        // If execvp returns, there was an error
        perror("Error in: execvp");
        return -1;
    }
    else {
        // Parent process Wait for child process to finish
        if (waitpid(pid, &status, 0) == -1) {
            perror("Error in: waitpid");
            return -1;
        }
        // Check child process status
        if (WIFEXITED(status)) {
            // Child process exited normally
            return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            // Child process terminated due to a signal
            return -1;
        }
    }
    return 0;
}

/**
 * removeExtraFiles remove a b.out and user.txt files
 */
void removeExtraFiles() {
    if (remove("b.out") != 0) {
        perror("Error in: remove");
    }
    if (remove("user.txt") != 0) {
        perror("Error in: remove");
    }
}

/**
 * Grades the C source code in the specified directory by attempting to compile it,
 *  run it with the given input file, and compare its output to the expected output file using an external
 *  comparison program. The results of the grading operation are written to the file descriptor `fd`.
 *
 * @param dirName The name of the directory containing the C source code to grade.
 * @param inputPath The path to the input file to use when running the C program.
 * @param outputPath The path to the expected output file to compare against.
 * @param fd The file descriptor of the file to write the results of the grading operation to.
 * @param compPath The path to the comparison program to use when comparing the program output to the expected output.
 * @return 1 on success, -1 on fatal error.
 */
int grade(char *dirName, char *inputPath, char *outputPath, int fd, char *compPath, int erfd) {
    // search for c file if not found write to results and return
    char fileName[MAX_LINE_LENGTH];
    if (findCFile(dirName, fileName) == 0) {
        // Handle the case where no c file file was found
        writeToFile(NO_C_FILE, fd);
        return 1;
    }
    // move to the dir and try to compile the found c file
    if (chdir(dirName) == -1) {
        perror("Error in: chdir");
        exit(-1);
    }
    if (compileFile(fileName, erfd) == 0) {
        // failed in compile so print to  result and return
        writeToFile(COMPILATION_ERROR, fd);
        if (chdir("..") == -1) {
            perror("Error in: chdir");
        }
        return 1;
    }
    // try to run the file and in case failed write to results
    int runTheFile = runBOut(inputPath, erfd);
    if (runTheFile == 0) {
        writeToFile(TIMEOUT, fd);
        if (chdir("..") == -1) {
            perror("Error in: chdir");
        }
        return 1;
    }
    else if (runTheFile == -1) {
        // something else failed
        return -1;
    }
    // compare the userOutput.txt file
    int compare = compareBetweenFiles(outputPath, compPath, erfd);
    writeToFile(compare + 3, fd);
    removeExtraFiles();
    if (chdir("..") == -1) {
        perror("Error in: chdir");
        exit(-1);
    }
    return 1;
}

/**
 * Searches the directory specified in `strings[0]` for subdirectories,
 * and runs the `grade()` function on each subdirectory that is found.
 *
 * @param strings An array of strings containing the directory path to search in (`strings[0]`),
 *           the path of the file to input (`strings[1]`),
 *           and the name of the file containing the compare to (`strings[2]`).
 * @param compPath A string containing the path to the file comp.out.
 * @return 0 on success, or a non-zero value on error.
 *
 */
int fillResults(char strings[3][MAX_LINE_LENGTH], char *compPath) {
    // open results.csv and save file descreptor
    char *filename = "results.csv";
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error in: open");
        exit(-1);
    }
    char *filenameEr = "errors.txt";
    int erfd = open(filenameEr, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (erfd == -1) {
        perror("Error in: open");
        exit(-1);
    }
    
    // change dir to the wanted dir and try open it
    chdir(strings[0]);
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Error in: opendir");
        exit(1);
    }
    // search for dirs in this dir and for each sub dir run grade function
    struct dirent *dp;
    // printf("\nprinting only directories:\n");
    while ((dp = readdir(dir)) != NULL) {
        struct stat st;
        if (stat(dp->d_name, &st) == -1) {
            perror("Error in: stat");
            exit(-1);
        }
        if (S_ISDIR(st.st_mode) && dp->d_name[0] != '.') {
            write(fd, dp->d_name, strlen(dp->d_name));
            // here run grade on each dir.
            int dirGrade = grade(dp->d_name, strings[1], strings[2], fd, compPath, erfd);
        }
    }
    if (closedir(dir) == -1) {
        perror("Error in: closedir");
        exit(-1);
    }
    return 0;
}

/**
 * The main function of the program. Parses command-line arguments,
 *  reads input data from a file, and grades the source code in the student
 *  directories found in the current working directory.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on success, or a non-zero value on error.
 */
int main(int argc, char *argv[]) {
    char cwd[MAX_LINE_LENGTH];
    char compPath[MAX_LINE_LENGTH * 2];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // the file name to the directory path
        snprintf(compPath, MAX_LINE_LENGTH * 2, "%s/%s", cwd, "comp.out");
    }
    else {
        perror("Error in: snprintf");
        return -1;
    }
    // check if there is error in param
    if (argc != 2) {
        perror("Not inough param");
        exit(1);
    }
    char strings[3][MAX_LINE_LENGTH];
    // assign the lines from the file in strings
    read_file(argv[1], strings);
    // check if the file didnt contained correct pathes
    if (checkUserPathes(strings) == 0) {
        exit(-1);
    }
    // fill the result
    fillResults(strings, compPath);
    return 0;
}
