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

        TCLAP::ValueArg<int> cleanUp("c", "cleanUpLimit", "clean up size", false, 10000, "int");
        TCLAP::ValueArg<int> algo("a", "rootAlgo", "algorithm to find root 0 old 1 new", false, 1, "int");
        TCLAP::ValueArg<bool> reverse("r", "reverseDirection", "reverse Direction of edge", false, false, "bool");
        TCLAP::ValueArg<bool> isCompressed("z", "isCompressed", "the root node is compressed", false, false, "bool");

        TCLAP::ValueArg<int> cycle("l", "cycleLenght", "cycle lenght", false, 80, "int");

        cmd.add(inputGraphArg);
        cmd.add(windowArg);
        cmd.add(resultArg);

        cmd.add(cleanUp);
        cmd.add(algo);
        cmd.add(reverse);
        cmd.add(isCompressed);

        cmd.add(cycle);


        // Parse the argv array.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        std::string inputGraph = inputGraphArg.getValue();

        std::string resultFile = resultArg.getValue();
        int window = windowArg.getValue();

        bool reverseEdge = reverse.getValue();
        int cleanUpLimit = cleanUp.getValue();
        int cyclelenght = cycle.getValue();

        int rootAlgo = algo.getValue();
        // 0 : Run naive
        // 1 : Run Root node finder then cycle detector
        // 2 : Run Root node finder only
        // 3 : Run cycle detector only
        // Do what you intend.
        Platform::Timer timer;
        timer.Start();
        std::cout << "Memory start, " << getMem() << std::endl;
        long pend = 0l;
        // findWithLength(inputGraph,resultFile,window,timeInMsec,cleanUpLimit,cyclelenght);
        if (rootAlgo == 0) {
            //  findRootNodes(inputGraph, resultFile, window, timeInMsec, cleanUpLimit);
            findAllCycleNaive(inputGraph, resultFile, window,  reverseEdge);
        } else if (rootAlgo == 1) {
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");;
            findRootNodes(inputGraph, resultFile, window, cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;
            findAllCycle(inputGraph, resultFile, cycleFile, window,  isCompressed.getValue(), reverseEdge);
            std::cout << "Found all cycles nodes and time " << timer.LiveElapsedSeconds() - pend << std::endl;
        }
        else if (rootAlgo == 2) {
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");;
            findRootNodes(inputGraph, resultFile, window,  cleanUpLimit, reverseEdge);

            pend = timer.LiveElapsedSeconds();
            std::cout << "Found all root nodes and time " << pend << std::endl;

        }
        else if (rootAlgo == 3) {
            std::string cycleFile = resultFile;
            cycleFile.replace(cycleFile.end() - 3, cycleFile.end(), "cycle");;

            findAllCycle(inputGraph, resultFile, cycleFile, window,  isCompressed.getValue(), reverseEdge);
            std::cout << "Found all cycles nodes and time " << timer.LiveElapsedSeconds() - pend << std::endl;
        }else{
            std::cout << "Un defined Algorithm param "<<rootAlgo << std::endl;
        }

        std::cout << "Memory end, " << getMem() << std::endl;
    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
    catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
}