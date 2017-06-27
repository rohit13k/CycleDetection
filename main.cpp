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

        TCLAP::ValueArg<int> cycle("l", "cycleLenght", "cycle lenght", false, 80, "int");

        cmd.add(inputGraphArg);
        cmd.add(windowArg);
        cmd.add(resultArg);
        cmd.add( time_param);
        cmd.add( cleanUp);
        cmd.add( cycle);


        // Parse the argv array.
        cmd.parse(argc, argv);

        // Get the value parsed by each arg.
        std::string inputGraph = inputGraphArg.getValue();
        std::string resultFile = resultArg.getValue();
        int window = windowArg.getValue();
        bool timeInMsec=false;
        int cleanUpLimit=cleanUp.getValue();
       int cyclelenght=cycle.getValue();
        if(time_param.getValue().compare("sec")!=0){
            timeInMsec=true;
        }

        // Do what you intend.
        Platform::Timer timer;
        timer.Start();
        long pend=0l;
       // findWithLength(inputGraph,resultFile,window,timeInMsec,cleanUpLimit,cyclelenght);
       findRootNodes(inputGraph,resultFile,window,timeInMsec,cleanUpLimit);
   //     findAllCycle(inputGraph,resultFile,"",window,timeInMsec,false);

        //findRootNodesNew(inputGraph,resultFile,window,timeInMsec);
        pend=timer.LiveElapsedSeconds();
        std::cout<<"Found all root nodes and time "<<pend<<std::endl;

        findAllCycle(inputGraph,resultFile,"",window,timeInMsec,false);
        std::cout<<"Found all cycles nodes and time "<<timer.LiveElapsedSeconds()-pend<<std::endl;



    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
    catch(std::exception &e){
        std::cerr << "error: " << e.what() << std::endl;
    }
}