# OS2
This project consists of two programs, ex21 and ex22, which work together to compare the contents of two files.

# ex21.c:

The ex21 program compares the contents of two files character by character.
It provides a function called open_files that opens two files and retrieves their file descriptors.
The checkLowerCase function is used to check if two characters are the same letter, ignoring case.
The main function reads the characters from both files, compares them, and returns a result code:
2 if an error occurred.
1 if the files are identical.
2 if the files are similar.
3 if the files differ.

ex22.c:

The ex22 program uses ex21 as a library to compare the contents of two files.
It provides additional functionality to compile and execute C programs.
The main function:
Checks if the directory and files provided by the user are valid.
Reads the contents of a configuration file to get the directory and file paths.
Compiles the C file found in the directory using ex21.c as a library.
Executes the compiled program and redirects its output to a file.
Compares the output file with a provided reference output file using ex21.c.
Writes the result to a CSV file.
Usage:

Compile ex21.c:
gcc ex21.c -o ex21

Compile ex22.c:
gcc ex22.c -o ex22

Run ex22 program with the directory and file paths as command-line arguments:
./ex22 <directory> <input_file> <output_file>

the gradeing system is:
no c file:           0
compilation error:   10
timeout(wait 5 sec): 20
wrong output:        50
similar output:      75
correct putput:      100

Check the generated CSV file "results.csv" for the program result.

Note: Make sure you have appropriate permissions to access and execute the necessary files and directories.
