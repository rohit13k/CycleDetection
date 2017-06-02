#include <iostream>
#include <tclap/CmdLine.h>
#include "countCycleFrequency.h"
#include <map>
#include <string>

int main(int argc, char **argv) {
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        TCLAP::CmdLine cmd("Cycle Detection in Temporal Graphs", ' ', "0.1");

        TCLAP::ValueArg<std::string> inputGraphArg("i", "input", "path of the temporal graph edge list", true, "",
                                                   "string");
        TCLAP::ValueArg<int> windowArg("w", "window", "time window in hours", false, 10l, "int");
        TCLAP::ValueArg<std::string> resultArg("o", "result", "path to store the result", true, "", "string");

        cmd.add(inputGraphArg);
        cmd.add(windowArg);
        cmd.add(resultArg);



        // Parse the argv array.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        std::string inputGraph = inputGraphArg.getValue();
        std::string resultFile = resultArg.getValue();
        int window = windowArg.getValue();

        // Do what you intend.
       count(inputGraph,resultFile,window);



    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}