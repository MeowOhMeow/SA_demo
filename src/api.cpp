#include "api.h"
#include "SA/SimulatedAnnealing.hpp"
#include "SA/Macro.hpp"
#include "SA/Scheduler.hpp"

std::atomic<bool> API::task_running(false);
std::atomic<bool> API::task_done(false);
std::atomic<float> API::task_progress(0.0f);
std::atomic<bool> API::task_cancel(false);
std::atomic<char *> API::error_message(nullptr);

std::string API::logFileName = "";

std::ofstream API::getLogFile()
{
    if (logFileName != "")
    {
        return ofstream(logFileName, ios::app);
    }
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
    logFileName = "logs/" + ss.str() + ".log";
    ofstream logFile(logFileName);
    return logFile;
}

static vector<string> split(string &str, char delim)
{
    vector<string> res;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delim))
    {
        res.push_back(token);
    }
    return res;
}

static vector<Macro> getMacros(string filename, float &minAspectRatio, float &maxAspectRatio)
{
    vector<Macro> macros;
    ifstream file(filename);

    if (!file.is_open())
    {
        throw runtime_error("Could not open file");
    }

    string line;
    int nums;
    getline(file, line);
    vector<string> tokens = split(line, ' ');
    nums = stoi(tokens[1]);
    getline(file, line);
    tokens = split(line, ' ');
    minAspectRatio = stof(tokens[1]);
    getline(file, line);
    tokens = split(line, ' ');
    maxAspectRatio = stof(tokens[1]);
    for (int i = 0; i < nums; i++)
    {
        getline(file, line);
        vector<string> tokens = split(line, ' ');
        string name = tokens[0];
        int w = stoi(tokens[1]);
        int h = stoi(tokens[2]);
        macros.push_back(Macro(name, w, h));
    }
    file.close();

    return macros;
}

void API::run(const Parameters &parameters)
{
    task_running = true;
    task_done = false;
    task_progress = 0.0f;
    task_cancel = false;
    if (error_message)
    {
        delete error_message;
        error_message = nullptr;
    }

    ofstream logFile = getLogFile();
    auto in_time_t = chrono::system_clock::to_time_t(chrono::system_clock::now());
    logFile << "Start time: " << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << endl;
    logFile << "Input file: " << parameters.inputFile << endl;
    logFile << "Output file: " << parameters.outputFile << endl;

    static float minAspectRatio = 0, maxAspectRatio = 0;
    static vector<Macro> macros;
    static Scheduler *scheduler = nullptr;
    static char lastInputFile[256] = "";
    if (memcmp(lastInputFile, parameters.inputFile, 256) != 0)
    {
        try
        {
            macros = getMacros(parameters.inputFile, minAspectRatio, maxAspectRatio);
        }
        catch (exception &e)
        {
            logFile << "Error: " << e.what() << endl;
            error_message = strdup(e.what());
            task_running = false;
            return;
        }
        if (scheduler)
        {
            delete scheduler;
        }
        scheduler = new Scheduler(macros, minAspectRatio, maxAspectRatio);
        strcpy(lastInputFile, parameters.inputFile);
    }

    SA::run(*scheduler, logFile, parameters);

    in_time_t = chrono::system_clock::to_time_t(chrono::system_clock::now());
    logFile << "End time: " << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << endl;

    try
    {
        scheduler->saveFloorplan(parameters.outputFile);
        std::system(("gnuplot " + string(parameters.outputFile)).c_str());
    }
    catch (exception &e)
    {
        logFile << "Error: " << e.what() << endl;
        error_message = strdup(e.what());
    }

    task_done = true;
}