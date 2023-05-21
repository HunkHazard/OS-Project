
#include "piping.h"

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
