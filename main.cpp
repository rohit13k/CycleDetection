#include <iostream>
#include <tclap/CmdLine.h>
#include "countCycleFrequency.h"
#include "countWithPath.h"
#include "DetectCycle.h"
#include <map>
#include <iostream>
#include "FileIndexer.h"
#include <utility>
#include <string>
#include "Timer.h"
#include "MemoryMonitor.h"

int main(int argc, char **argv) {
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        TCLAP::CmdLine cmd("Cycle Detection in Temporal Graphs", ' ', "0.1");

        TCLAP::ValueArg<std::string> inputGraphArg("i", "input", "path of the temporal graph edge list", true, "",
                                                   "string");
        TCLAP::ValueArg<int> windowArg("w", "window", "time window in hours", false, 1, "int");
        TCLAP::ValueArg<std::string> resultArg("o", "result", "path to store the result", true, "", "string");
        TCLAP::ValueArg<std::string> time_param("t", "time_param", "timestamp is msec or sec", false, "sec", "string");
        TCLAP::ValueArg<int> cleanUp("c", "cleanUpLimit", "clean up size", false, 10000, "int");
        TCLAP::ValueArg<int> algo("a", "rootAlgo", "algorithm to find root 0 old 1 new", false, 1, "int");
        TCLAP::ValueArg<bool> reverse("r", "reverseDirection", "reverse Direction of edge", false, false, "bool");

        TCLAP::ValueArg<int> cycle("l", "cycleLenght", "cycle lenght", false, 80, "int");

        cmd.add(inputGraphArg);
        cmd.add(windowArg);
        cmd.add(resultArg);
        cmd.add(time_param);
        cmd.add(cleanUp);
        cmd.add(algo);
        cmd.add(reverse);

        cmd.add(cycle);


        // Parse the argv array.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        std::string inputGraph = inputGraphArg.getValue();
        std::string resultFile = resultArg.getValue();
        int window = windowArg.getValue();
        bool timeInMsec = false;
        bool reverseEdge = reverse.getValue();
        int cleanUpLimit = cleanUp.getValue();
        int cyclelenght = cycle.getValue();
        if (time_param.getValue().compare("sec") != 0) {
            timeInMsec = true;
        }
        int rootAlgo = algo.getValue();
        // Do what you intend.
        Platform::Timer timer;
        timer.Start();
        std::cout << "Memory start, " << getMem() << std::endl;
        long pend = 0l;
        // findWithLength(inputGraph,resultFile,window,timeInMsec,cleanUpLimit,cyclelenght);
        if (rootAlgo == 0) {
          //  findRootNodes(inputGraph, resultFile, window, timeInMsec, cleanUpLimit);
            findAllCycleNaive( inputGraph,  resultFile,  window,  timeInMsec,reverseEdge);
        } else {
            findRootNodesNew(inputGraph, resultFile, window, timeInMsec, cleanUpLimit,reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;
            findAllCycle(inputGraph, resultFile, "D:\\dataset\\sms_paths.csv", window, timeInMsec, false,reverseEdge);
        }
        std::cout << "Found all cycles nodes and time " << timer.LiveElapsedSeconds() - pend << std::endl;
        std::cout << "Memory end, " << getMem() << std::endl;
    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
    catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
}