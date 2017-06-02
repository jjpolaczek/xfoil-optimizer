#include "xfoil/simulation.h"

using std::ifstream;
unsigned int SimulationHandler::id_total = 0;
void SimulationHandler::ReadResults()
{
    std::ifstream infile(proxy_->GetExePath() + "/" + InstantiateFilename("result.txt"));
    if(infile.good())
    {
        //Read file contents
        std:: string line;
        bool failure = false;
        for(int i = 0; i < 12; ++i)//Skip intro lines - no real content//
        {
            if(!getline(infile, line))
            {
                //File was too short wtf//
                failure = true;
                break;
            }
            else
            {}
               // std::cout<<line<<std::endl;
        }
        if(geometry_.simResults_.calculated_ == true)
        {
            //Clear previous results from the structure
            geometry_.simResults_.results_.clear();
            geometry_.simResults_.calculated_ = false;
        }
        while(getline(infile, line))
        {
            //Read 7 double parameters in a single line//
            std::string::size_type sz;
            SimResults::ResultEntry resLine;
            //Can cause exception throws out of range
            resLine.alfa = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.cl = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.cd = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.cdp = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.cm = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.xtr_top = std::stod(line,&sz);
            line = line.substr(sz);
            resLine.xtr_bottom = std::stod(line,&sz);
            geometry_.simResults_.results_.push_back(resLine);
        }
        geometry_.simResults_.calculated_ = true;
        infile.close();
    }
    else
        throw std::exception("Invalid data file");


}
std::string SimulationHandler::InstantiateFilename(std::string filename)
{
    std::string result;
    std::string::size_type idx = filename.find_last_of('.');
    if(idx != std::string::npos)
    {
        //Found externsion can add number
        result = filename.substr(0,idx);
        result += std::to_string(id_) + filename.substr(idx);
    }
    else
    {
        result = filename + std::to_string(id_);
    }
    //std::cout<< result<<std::endl;
    return result;
}
void SimulationHandler::SaveGeometry()
{
    geometry_.Save((std::string)(proxy_->GetExePath() + "\\" + InstantiateFilename("geometry.dat")));
}

void SimulationHandler::DeleteGeometry()
{
   // std::cout<< "DELETE GEOM "<< InstantiateFilename("geometry.dat")<<std::endl;
   std::remove((proxy_->GetExePath() + "/" + InstantiateFilename("geometry.dat")).c_str());
}
void SimulationHandler::DeleteResults()
{
    //std::cout<< "DELETE RES "<<InstantiateFilename("result.dat")<<std::endl;
    std::remove((proxy_->GetExePath() + "/" + InstantiateFilename("result.txt")).c_str());
}
void SimulationHandler::Run()
{
    if(status_ != Idle)
        throw std::exception("Invaid state - simulation running?");
    std::stringstream ss;
    proxy_->AddCommand("PLOP");
    proxy_->AddCommand("G F");
    proxy_->AddCommand("\r\n");
    proxy_->AddCommand("LOAD " + InstantiateFilename("geometry.dat"));
    proxy_->AddCommand("OPER");
    //Viscous settings//
    if(params_.viscousEnable)
    {
        ss << "VISC "<<params_.reynoldsNo;
        proxy_->AddCommand(ss.str());//Set viscous flow reynolds number
        ss.str("");
        ss.clear();
    }
    proxy_->AddCommand("PACC");
    proxy_->AddCommand("");//do not save tmp
    proxy_->AddCommand("");//do not save polar
    proxy_->AddCommand("ITER");
    //Iteration settings//
    ss << params_.iterationLimit;
    proxy_->AddCommand(ss.str());
    ss.str("");
    ss.clear();
    //TODO precision settings//
    proxy_->AddCommand("ASEQ -10.0 -2.0 1.0");
    proxy_->AddCommand("ASEQ -2.0 2.0 0.2");
    proxy_->AddCommand("ASEQ 2.0 15.0 0.5");
    proxy_->AddCommand("PACC");//Disable polar accumulation

    proxy_->AddCommand("PWRT 1");
    proxy_->AddCommand(InstantiateFilename("result.txt"));

    //proxy_->AddCommand("CPWR " + InstantiateFilename("result.txt"));
    proxy_->Run();
    status_ = Running;
    //Check if proxy really started - state error if it didnt//
    if(proxy_->PollStatus() == SimulationProxy::NotRunning)
        status_ = Error;//TODO add log message
    //std::cout <<"Running";

}
SimulationHandler::Status SimulationHandler::PollStatus()
{
    if(status_ != Error && status_ != Finished)//Final states check
       {
        SimulationProxy::Status proxy_status = proxy_->PollStatus();
        switch(proxy_status)
        {
            case SimulationProxy::NotRunning:
                status_ = Idle;
            break;
            case SimulationProxy::Running://Verify timeout TODO//
                //std::cout <<"...";
                status_ = Running;
            break;
            case SimulationProxy::Error:
                //std::cout <<"ERR";
                status_ = Error;
            break;
            case SimulationProxy::Finished:
                //std::cout <<"FIN";
                ReadResults();
                status_ = Finished;
            break;
        default:
            break;
        }
    }
    return status_;
}

void SimulationScheduler::ConsumeTask()
{
    //Initialize pointers for handler objects//
    SimulationHandler **handlers = new SimulationHandler*[params_.parallelSimulations];
    for(int i = 0; i < params_.parallelSimulations; ++i)
        handlers[i] = nullptr;
    while(workerEnable_)
    {
        //Free existing queue places//
        for(int i = 0; i < params_.parallelSimulations; ++i)
        {
            if(handlers[i] != nullptr)
            {
                handlerStatus_[i] = handlers[i]->PollStatus();
                if(handlerStatus_[i] != SimulationHandler::Running)
                {
                    std::cout<<"Task Finished, freeing spot"<<i<<"\r\n";
                    delete handlers[i];
                    handlers[i] = nullptr;
                    handlerStatus_[i] = SimulationHandler::NotExisting;
                }
            }
        }
        //Look for new task//
        if(!taskQueue_.empty())
        {
            for(int i = 0; i < params_.parallelSimulations; ++i)
            {
                //Search for empty queue place and run if available//
                if(handlers[i] == nullptr)
                {
                    //Mutex it//
                    queueMutex_.lock();
                    Task task = taskQueue_.front();
                    taskQueue_.pop();
                    queueMutex_.unlock();
                    handlers[i] = new SimulationHandler(*task.geometry, params_);
                    handlers[i]->Run();
                    handlerStatus_[i] = handlers[i]->PollStatus();
                    break;
                }
            }
        }
        //Poll/ update task states
        for(int i = 0; i < params_.parallelSimulations; ++i)
        {
            if(handlers[i] != nullptr)
            {
                handlerStatus_[i] = handlers[i]->PollStatus();
            }
        }
    }
    //Finish up all the tasks//
    bool finished = false;
    while(!finished)
    {
        finished = true;
        for(int i = 0; i < params_.parallelSimulations; ++i)
        {
            if(handlers[i] != nullptr)
            {
                int timeout = 1000;//delay of 10s//
                while(handlers[i]->PollStatus() == SimulationHandler::Running)
                {
                    if(--timeout < 0)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                delete handlers[i];
                handlers[i] = nullptr;
                handlerStatus_[i] = SimulationHandler::NotExisting;
            }
        }
    }
    delete handlers;
}
bool SimulationScheduler::IsTasksFinished() const
{
    //Check for awaiting tasks
    if(taskQueue_.size() != 0)
        return false;
    //Check also for running tasks//
    for(int i = 0; i < params_.parallelSimulations; ++i)
    {
        if(handlerStatus_[i] != SimulationHandler::NotExisting)
            return false;
    }
    return true;
}
