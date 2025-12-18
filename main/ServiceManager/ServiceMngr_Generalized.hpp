#pragma once 
#include "sdkconfig.h"
#include <memory>
#include <map>
#include "ServiceBase.hpp"
#include "SharedBus.hpp"
#include "ServiceFactory.hpp"

/**
 * @brief Generalized Service Manager
 * 
 * This version uses the ServiceFactory to create services dynamically.
 * Device-specific implementations are registered via ServiceRegistration.cpp
 * in each project, eliminating the need to modify ServiceMngr for different devices.
 */
class ServiceMngr : public ServiceBase
{
public:
    static constexpr char* mServiceName[SharedBus::ServiceID::MAX_ID] =
    {
        "",             //NO_ID
        "SRV_MNGR",     //Service Manager
        "UI",
        "MATTER",
        "MQTT",
        "LOG",      
    };    

    static constexpr uint32_t mServiceStackSize[SharedBus::ServiceID::MAX_ID] =
    {
        0 ,             //NO_ID
        20  * 1024,     //Service Manager
        50  * 1024,     //UI
        50  * 1024,     //MATTER
        20  * 1024,     //MQTT
        0               //LOG
    };
    
    explicit ServiceMngr(
        const char *TaskName,
        const SharedBus::ServiceID &ServiceID);

    ~ServiceMngr();

private:    
    static TaskHandle_t SrvMngHandle;
    
    // Generic service instances (using base class)
    static std::map<SharedBus::ServiceID, std::shared_ptr<ServiceBase>> sServiceInstances;
    static std::map<SharedBus::ServiceID, TaskHandle_t> sServiceHandles;

    /**
     * @brief Handles the transition to the machine's start state.
     * This function is called when the state machine enters the start state. 
     * It creates and initializes all registered services using the factory.
     * @return ESP_OK on successful execution.
     * @return Appropriate error code if the state transition fails.
     */
    esp_err_t OnMachineStateStart() override;

    /**
     * @brief Initialize a service by ID
     * @param serviceID The service ID to initialize
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t InitializeService(SharedBus::ServiceID serviceID);
};
