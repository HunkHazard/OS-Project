#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <limits>
#include <sstream>
#include <iostream>
#include <cstring>
#include <sys/wait.h>

#define PROMPT "NapoleonShell:"

using namespace std;

/* Wrapper function for execv function, with C++ friendly arguments
    (string vector + path) */
int execvString(const string path, const vector<string> &args)
{

    // Size + 1 for NULL pointer
    char *c_arr[args.size() + 1];

    // Copy vector strings into newly allocated strings
    for (unsigned int i = 0; i < args.size(); i++)
    {
        // Allocate memory for the string copy
        c_arr[i] = new char[args[i].size() + 1];
        // Copy the string
        strcpy(c_arr[i], args[i].c_str());
    }

    // Adding the null pointer to end of argument list to conform to execv
    c_arr[args.size()] = NULL;
    return execv(path.c_str(), c_arr);
}

/* Executes a given command, and automatically check if the command
    exists in the /bin/ directory, if it cant find it in the given path*/
void executeCommand(const vector<string> &args)
{

    execvString(args[0], args);

    // 2 = No such directory, then we check bin folder
    if (errno == 2)
    {

        // Create path for same file, but in bin
        stringstream ss;
        ss << "/bin/" << args[0];
        string binName = ss.str();

        execvString(binName, args);
    }

    /* Since execv will "ignore" remaining code if it's succesful,
        we can safely assume that an error has occured if we reach
        this point  */
    string errormsg(strerror(errno));
    cout << "Error running program: " << errormsg << endl;
}

// Creates fork for running a single command
// Example Commands: ls, ls -l, ls -l -a
void forkCommandSingle(const vector<string> &args)
{
    // creates a child process
    int pid = fork();
    if (pid == -1)
        cout << "Forking error" << endl;
    if (pid == 0)
    {
        // Child
        executeCommand(args);
    }
    if (pid > 0)
    {
        // Parent
        int status;
        waitpid(pid, &status, 0);
    }
}
// HELPER FUNCTION 1
// Checks to see if there is a pipe symbol in argument list
int isPipeCommand(const vector<string> &args)
{
    for (int i = 0; i < args.size(); i++)
    {
        if (args[i] == "|" && i > 0 && i < args.size() - 1)
            return 1;
    }
    return 0;
}

// HELPER FUNCTION 2
// Returns the  index of the pipe symbol in the argument list
int getPipeIndex(const vector<string> &args)
{
    for (int i = 0; i < args.size(); i++)
    {
        if (args[i] == "|")
            return i;
    }
    return -1;
}

// MAIN FUNCTION
/* Split a pipe command ('cmd1 arg1 | cmd2 arg1') into two seperate
    lists (vectors) commands.
    Returns a pointer to the array of string vectors (size = 2) */
const vector<string> *splitPipeCommand(const vector<string> &args)
{

    // Check if there is a pipe command
    if (!isPipeCommand)
        return NULL;

    // Get index of pipe symbol
    int pipeIndex = getPipeIndex(args);

    // Create array of size 2, with to string vectors
    vector<string> *cmds;
    cmds = new vector<string>[2];

    // Create first string vector
    for (int i = 0; i < pipeIndex; i++)
    {
        cmds[0].push_back(args[i]);
    }

    // Create seconds string vector
    for (int i = pipeIndex + 1; i < args.size(); i++)
    {
        cmds[1].push_back(args[i]);
    }

    return cmds;
}

// Creates 2 forks for running a command through a pipe into another command
void forkCommandPipe(const vector<string> &args)
{
    const vector<string> *commands = splitPipeCommand(args);

    // a unidirectional pipe
    // the write end is pipefd[1]
    // the read end is pipefd[0]
    int pipefd[2];
    if (pipe(pipefd))
        cout << "Piping error!" << endl;

    // First command/fork
    int pid1 = fork();

    if (pid1 == -1)
        cout << "Forking error!" << endl;

    // First Child
    if (pid1 == 0)
    {
        // Sets pipeinput to stdin (read)
        // as if it was a file or reading from keyboard
        dup2(pipefd[0], 0);

        // Close original pipe fds, other it will block
        close(pipefd[1]);
        close(pipefd[0]);

        executeCommand(commands[1]);
    }

    // Second command/fork
    if (pid1 > 0) // Parent Block
    {
        // create another child process
        int pid2 = fork();

        if (pid2 == -1)
            cout << "Forking error!" << endl;
        if (pid2 == 0)
        {
            // Sets pipeoutput to stdout (write)
            dup2(pipefd[1], 1);

            // Close original pipe fds, other it will block
            close(pipefd[0]);
            close(pipefd[1]);
            executeCommand(commands[0]);
        }
        if (pid2 > 0)
        {
            close(pipefd[1]);
            close(pipefd[0]);

            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
    }

    delete commands;
}

// Prints working directory using system call getcwd
void printDirectory()
{
    char *dir = getcwd(NULL, 0); // Fetching pointer to str holding name

    // If an error occured, print it
    if (dir == NULL)
        cout << "Error while printing working dir: " << strerror(errno) << endl;

    // Else print the directory
    else
        cout << dir << endl;
}

/* Changes the directory to the given path.
     "cd .." works and using only 'cd' makes you go to $home */
void changeDirectory(string path)
{
    if (chdir(path.c_str()) == -1)
    {
        cout << "Error: " << strerror(errno) << endl;
        ;
    }
}

/* Checks if a given command is a directory command (cd/pwd/dir), and in
    that case runs the appropriate functionality. */
int runDirCommand(const vector<string> &args)
{
    if (args[0] == "cd")
    {
        if (args.size() > 2)
            cout << "Too many arguments" << endl;
        else
        {
            if (args.size() == 1)
                changeDirectory(getenv("HOME"));
            else
                changeDirectory(args[1]);
        }
        return 1;
    }
    if (args[0] == "pwd" || args[0] == "dir")
    {
        if (args.size() > 1)
            cout << "Too many arguments" << endl;
        else
        {
            printDirectory();
        }
        return 1;
    }

    return 0;
}

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
