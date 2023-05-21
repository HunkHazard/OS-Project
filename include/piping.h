#pragma once

#include <iostream>
#include <vector>

using namespace std;

int isPipeCommand(const vector<string> &args);

const vector<string>* splitPipeCommand(const vector<string> &args);