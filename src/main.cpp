#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <limits>
#include <sstream>

#include "exec.h"
#include "piping.h"
#include "directory.h"

#define PROMPT "NapoleonShell:"

using namespace std;

// Prints the help message
void help()
{
    cout << "\nNapoleonShell is a small application implementing basic functionality:\n"
         << "  cd [DIR]             \t Change working directory. '..' goes one directory back, and 'cd' goes to home directory.\n"
         << "  dir/pwd              \t Prints current working directory.\n"
            "  [FILE] [ARG1] ..     \t Executes selected file if it's executable, including the given arguments.\n"
         << "  [FILE1] | [FILE2]    \t Pipes the output of file 1 into the output of filee 2.\n"
         << endl;
}

/*  Prints the prompt message (MiniShell:WORKING_DIR$).
    It also uses the system call 'getcwd()' to get the
    current working dir. */
static void printPrompt()
{
    char *dir = getcwd(NULL, 0);
    if (dir == NULL)
    {
        cout << "Error while printing working dir: " << strerror(errno) << endl;
        cout << "\033[93m" << PROMPT << "$ ";
    }
    else
    {
        cout << "\033[93m" << PROMPT << "\033[96m" << dir << "\033[37m$ ";
    }
}

/** Prints prompt message and reads the input of the user
 *  It returns the number of arguments (including command)
 *  givne by the user */
static int getInput(vector<string> &args)
{

    printPrompt();
    args.clear();

    string input;

    // Read entire line
    getline(cin, input);

    // Split line into arguments seperated by ' '
    stringstream ss(input);

    while (getline(ss, input, ' '))
    {
        args.push_back(input);
    }
    return args.size();
}

void printNapoleon()
{
    string var = R"(
         ________
        /        \
     __/       (o)\__
    /     ______\\   \
    |____/__  __\____|
       [  --~~--  ]
        |(  L   )|
  ___----\  __  /----___
 /   |  < \____/ >   |   \
/    |   < \--/ >    |    \
||||||    \ \/ /     |||||| 
|          \  /   o       |
|     |     \/   === |    |
|     |      |o  ||| |    |
|     \______|   +#* |    |
|            |o      |    |
 \           |      /     /
 |\__________|o    /     /
 |           |    /     /
                         _                  
                        | |                 
 _ __   __ _ _ __   ___ | | ___  ___  _ __  
| '_ \ / _` | '_ \ / _ \| |/ _ \/ _ \| '_ \ 
| | | | (_| | |_) | (_) | |  __/ (_) | | | |
|_| |_|\__,_| .__/ \___/|_|\___|\___/|_| |_|
            | |                             
            |_|      
 )";

    cout << var << endl;
}

int main()
{

    // cout << "\n\033[93mNapoleonShell started! Type 'help' to see available functionality\033[37m" << endl;
    // ;
    printNapoleon();
    // Loop continues until the program is terminated externally (i.e. CTRL+C);
    while (1)
    {
        vector<string> args;
        if (getInput(args))
        {

            if (args[0] == "help")
                help();

            // Check if its a dir command
            else if (!runDirCommand(args))
            {

                // Check if its a pipe command
                if (isPipeCommand(args))
                    forkCommandPipe(args);

                // Single command
                else
                    forkCommandSingle(args);
            }
        }
    }
    return 0;
}
