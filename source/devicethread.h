#ifndef DEVICETHREAD_H
#define DEVICETHREAD_H

#include <string>
#include <vector>

#include "utils.h"

//these are functions to manage a thread which polls the filesystem in 3 second intervals for attached/removed storage
//access to the current list of devices is given via CurrentDevList(), and you can check if there is any change to the list with
//DevListChanged().  if you only care to see changed items, you can pass the old list to DevicesAdded() and DevicesRemoved()
//and they will return a list of changed entries

namespace DeviceThread
{
//initialize & destroy this thread
//! be sure to call Resume() after initializing to start the thread working
void Init();
void Shutdown();

//pasue and resume the thread
void Halt();
void Resume();

//check if the list has changed since last time CurrentDevList() is called
bool DevListChanged();

//gets the current list
std::vector<std::string> CurrentDevList();

//compare the current device list with a different one
std::vector<std::string> DevicesAdded( std::vector<std::string> &oldList );
std::vector<std::string> DevicesRemoved( std::vector<std::string> &oldList );

}
#endif // DEVICETHREAD_H
