#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_E.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_E::Method_E()
{
}

Method_E::~Method_E() {}

bool Method_E::Process()
{
    LOG_INFO("------------------------Analyzing with Method E-----------------------------");
    return true;
}
