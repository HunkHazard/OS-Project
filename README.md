# Mini-Shell
_Author: Malte Bryndum Pedersen (s185139)_

A small shell application for a UNIX operating system written in __C++__, using UNIX system calls to provide basic bash-like shell functionality.

[Github Repo](http://github.com/maltebp/MiniShell)

[Latest Version](http://github.com/maltebp/MiniShell/releases/latest)

### Program Overview
An overview of the program may be found in the activity diagram appendix (minishell_overview.png)

### Install and Run
 1. Build by running _compile.sh_ script in project root folder
 2. Start application by running created _minishell_ in project root

### How to use / Features
The shell uses the same syntax as the bash shell. When prompting command, it displays the current working directory
 - Execute: [EXECUTABLE_PATH] [ARG1] ... [ARGN] (it checks your /bin folder, so you can just write 'ls' rather than '/bin/ls')
 - Change Directory:
    -  Home: cd
    -  Dir: cd [DIR_PATH]
    -  Back: cd ..
 - Show working directory: pwd OR dir
 - Pipe: [EXEC_1_PATH] [ARG1] ... [ARGN] | [EXEC_2_PATH] [ARG1] ... [ARGN] 

### System Calls
The shell gains functionality by implementing system calls. These are "functions"/services provided by the kernel (core of the OS). The system calls used are:
 - _execv:_ Executes a binary with the given path (+file name), with the arguments given to the function.
 - _fork:_ Copy current process (parent) into a new process (child). Required since 'execvp' takes over the process its called from.
 - _waitpid:_ Waits for child process, given by the pid (process id) to finish before continuing.
 - _chdir:_ Changes working directory to given path. If no path is given it returns you to $HOME, and if '..' is given it goes to "previous directory.
 - _getcwd_: Retrieves the current working directory.
 - _pipe:_ Creates a pipe. A pipe consists of two file descriptors (fd) for a virtual file. One fd may write and the other read. These may be given to two different processes (as in this case), allowing output of one process to be the input of the other.
 - _dup2:_ Copies a file descriptor over to another file descriptor, i.e. copy one pipe end over to the standard input file descriptor (stdin).