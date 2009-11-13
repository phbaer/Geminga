#include <iostream>
#include <string>
#include <vector>

#include "Geminga.h"

using namespace spica::geminga;

int main(int argc, char *argv[])
{
    // TODO: Implement the CN Arguments class for C++
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(std::string(argv[i]));
    }

    Geminga geminga(args);
    geminga.configure();
    geminga.start();

    for (;;)
    {
        usleep(100);
    }
}
