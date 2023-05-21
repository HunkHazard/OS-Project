#pragma once

#include <vector>
#include <string>

using namespace std;


void forkCommandSingle(const vector<string> &args);
void forkCommandPipe(const vector<string> &args);
void executeCommand(const vector<string> &args);
int execvString(const vector<string> &args);