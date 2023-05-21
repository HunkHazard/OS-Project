
#include "directory.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <cstdlib>


// Prints working directory using system call getcwd
void printDirectory(){
    char * dir = getcwd(NULL,0); // Fetching pointer to str holding name
    if( dir == NULL ) cout<<"Error while printing working dir: "<<strerror(errno)<<endl;
    else cout<<dir<<endl;
}

/* Changes the directory to the given path.
     "cd .." works and using only 'cd' makes you go to $home */
void changeDirectory(string path){
    if(chdir(path.c_str()) == -1){
        cout<<"Error: "<<strerror(errno)<<endl;;
    }
}

/* Checks if a given command is a directory command (cd/pwd/dir), and in
    that case runs the appropriate functionality. */
int runDirCommand(const vector<string> &args){
    if( args[0] == "cd" ){
        if(args.size() > 2)
            cout<<"Too many arguments"<<endl;
        else{
            if(args.size() == 1) changeDirectory(getenv("HOME"));
            else changeDirectory(args[1]);
        }
        return 1;
    }
    if( args[0] == "pwd" || args[0] == "dir"){
        if(args.size() > 1 )
            cout<<"Too many arguments"<<endl;
        else{
            printDirectory();
        }
        return 1;
    }

    return 0;
}
