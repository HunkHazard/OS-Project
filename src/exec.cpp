#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <sys/wait.h>

#include "exec.h"
#include "piping.h"

using namespace std;

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
