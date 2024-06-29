#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <fstream> // std::ofstream
#include <atomic>  // std::atomic
#include <sstream> // std::stringstream
#include <iomanip> // std::put_time

namespace API
{
    extern std::atomic<bool> task_running;
    extern std::atomic<bool> task_done;
    extern std::atomic<float> task_progress;
    extern std::atomic<bool> task_cancel;

    extern std::atomic<char *> error_message;

    extern std::string logFileName;

    /**
     * Get the log file.
     */
    std::ofstream getLogFile();

    /**
     * Parameters for the API.
     *
     * Required:
     * inputFile: The path to the input file.
     * outputFile: The path to the output file.
     * temperature: The initial temperature for the simulated annealing algorithm.
     * coolingRate: The cooling rate for the simulated annealing algorithm.
     * absoluteTemperature: The absolute temperature for the simulated annealing algorithm.
     *
     * Optional:
     * targetIterations: The number of iterations to run the algorithm for.
     */
    struct Parameters
    {
        char inputFile[256] = "./testcases/floorplan_6.txt";
        char outputFile[256] = "./output/output.gp";
        double temperature = 1;
        double coolingRate = 0.95;
        double absoluteTemperature = 0.01;

        /**
         * Optional. The number of iterations to run the algorithm for.
         * If set to 0, the algorithm will run until the absolute temperature is reached.
         * Default is 0.
         */
        int targetIterations = 0;
    };

    void run(const Parameters &parameters);
}
