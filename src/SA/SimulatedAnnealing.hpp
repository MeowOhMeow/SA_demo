#ifndef SIMULATEDANNEALING_HPP
#define SIMULATEDANNEALING_HPP

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <string>
#include <vector>

#include "Scheduler.hpp"
#include "../api.h"

using namespace std;

class SA
{
public:
    static void run(Scheduler &scheduler, ofstream &logFile, const API::Parameters &parm)
    {
        double temperature = parm.temperature;
        double coolingRate = parm.coolingRate;
        double absoluteTemperature = parm.absoluteTemperature;
        logFile << "temperature: " << temperature << endl;
        logFile << "coolingRate: " << coolingRate << endl;
        logFile << "absoluteTemperature: " << absoluteTemperature << endl;

        scheduler.setTemperature(temperature);
        scheduler.setCoolingRate(coolingRate);
        int targetIterations = parm.targetIterations > 0 ? parm.targetIterations : log2(absoluteTemperature / temperature) / log2(coolingRate);
        int currentIteration = 0;
        logFile << "targetIterations: " << targetIterations << endl;

        double bestCost = scheduler.evaluateState();
        double currentCost = bestCost;
        logFile << "Initial cost: " << bestCost << endl;
        logFile << setw(10) << "Time" << setw(10) << "Steps" << setw(20) << "Cost" << endl;

        int steps = 0;

        do
        {
            scheduler.initialize();
            while (scheduler.canContinue())
            {
                // make a random modification to the current tree (state)
                scheduler.makeRandomModification();

                double newCost = scheduler.evaluateState();

                if (newCost <= bestCost)
                {
                    bestCost = newCost;
                    currentCost = newCost;
                    scheduler.accept();
                }
                else
                {
                    double acceptanceProbability = exp((currentCost - newCost) / scheduler.getTemperature());
                    if (acceptanceProbability > scheduler.getRandomNumber(0.0f, 1.0f))
                    {
                        currentCost = newCost;
                        scheduler.uphill();
                    }
                    else
                    {
                        scheduler.reject();
                    }
                }

                steps++;
            }
            currentIteration++;

            logFile << setw(10) << scheduler.getElapsed() << setw(10) << steps << setw(20) << bestCost << endl;

            API::task_progress = (float)currentIteration / targetIterations;

            if (API::task_cancel)
            {
                logFile << "Task cancelled" << endl;
                break;
            }
            if (currentIteration >= targetIterations)
            {
                logFile << "Target iterations reached" << endl;
                break;
            }
        } while (scheduler.isImproving() && !scheduler.hasTimeExpired() && scheduler.getTemperature() > absoluteTemperature);

        if (scheduler.hasTimeExpired())
        {
            logFile << "Time expired" << endl;
        }
        else if (!scheduler.isImproving())
        {
            logFile << "No improvement" << endl;
        }
        else if (scheduler.getTemperature() <= absoluteTemperature)
        {
            logFile << "Temperature too low, temperature: " << scheduler.getTemperature() << endl;
        }
        logFile << "Result: " << currentCost << endl;
    }
};

#endif // SIMULATEDANNEALING_HPP