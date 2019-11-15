"smallsh" was made for CS 344: Operating Systems at Oregon State University

To run:
    1) Compile by running the Makefile. Run Makefile by using "make" on the terminal.
        a)  NOTE: If makefile unavailable, compile using "gcc -o smallsh smallsh.c"
    2)  Once compiled, run the program using "./smallsh" on the terminal.

Built-in Functions:
    cd [argument]:
        No argument goes to the HOME directory.
        Otherwise, changes directory to argument if its a valid location.
    status:
        Prints the exit status of the last foreground command.
    exit:
        Exits the program