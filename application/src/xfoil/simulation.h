#pragma once

#include <vector>
#include "xfoil/simulation_results.h"
#include "optimizer/geometry.h"
#include "xfoil/simulation_proxy.h"
#include "xfoil/qsimulation.h"

//!  Class controlling execution of external simulation tools
/*!
  Controls multiple instances of xfoil optimizer running in parallel
*/
class SimulationScheduler
{
public:
private:
	const int _parallelInstances = 4;
    std::vector<Geometry*> tasks;



};
//!  Class controlling execution single simulation tool using proxy interface
/*!
  Controls single instance of xfoil optimizer
*/
class SimulationHandler
{
public:
    SimulationHandler()
    {
        proxy_ = new QSimulationProxy();
    }
    ~SimulationHandler()
    {
        delete proxy_;
    }

private:
    SimulationProxy *proxy_;
    Geometry geometry_;
    SimResults *results_;
};


