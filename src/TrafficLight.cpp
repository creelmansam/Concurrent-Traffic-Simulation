#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);

    _condition.wait(lock, [this] {return !_queue.empty();});

    auto nextPhase = _queue.back();
    _queue.clear();

    return std::move(nextPhase);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _queue.push_back(msg);

    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        if(_messageQueue.receive() == TrafficLightPhase::green)
            return;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread( &TrafficLight::cycleThroughPhases, this ));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device r;
    std::mt19937 gen(r());
    std::uniform_int_distribution<int> dist(4000, 6000);

    while (true)
    {
        int waitTime = dist(gen);

        std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

        TrafficLightPhase newPhase;
        switch (getCurrentPhase())
        {
        default:
        case TrafficLightPhase::green:
            _currentPhase = TrafficLightPhase::red;
            newPhase = TrafficLightPhase::red;
            break;
        
        case TrafficLightPhase::red:
            _currentPhase = TrafficLightPhase::green;
            newPhase = TrafficLightPhase::green;
            break;
        } 

        _messageQueue.send(std::move(newPhase));
    }
    
}

