#include "nvsFlash.h"
#include <cstring>
#include "esp_heap_caps.h"
#include "ServiceMngr_Generalized.hpp"
#include "ServiceFactory.hpp"
#include "SpiffsManager.h"
#include "SharedBus.hpp"
#include "esp_log.h"

static const char* TAG = "ServiceMngr";

// Static member initialization
TaskHandle_t ServiceMngr::SrvMngHandle = nullptr;
std::map<SharedBus::ServiceID, std::shared_ptr<ServiceBase>> ServiceMngr::sServiceInstances;
std::map<SharedBus::ServiceID, TaskHandle_t> ServiceMngr::sServiceHandles;

ServiceMngr::ServiceMngr(
    const char *TaskName,
    const SharedBus::ServiceID &ServiceID) :
    ServiceBase(TaskName, ServiceID)
{
    esp_err_t err;    

    nvsFlashInit();
    SpiffsInit();

    if(SharedBus::Init() == ESP_OK)
    {
        ESP_LOGI(TAG, "Initialized SharedBus successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to Initialize SharedBus.");
    }                

    err = TaskInit(
            &SrvMngHandle,
            tskIDLE_PRIORITY + 1,
            mServiceStackSize[SharedBus::ServiceID::SERVICE_MANAGER]);
    
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created.",
            mServiceName[SharedBus::ServiceID::SERVICE_MANAGER]);
    }     
    else 
    {
        ESP_LOGE(TAG,"failed to create %s service.",
            mServiceName[SharedBus::ServiceID::SERVICE_MANAGER]);
    }        
}

ServiceMngr::~ServiceMngr()
{                
}

esp_err_t ServiceMngr::OnMachineStateStart()
{
    esp_err_t firstError = ESP_OK;

    // Iterate over ServiceID enum values (skip NO_ID and LOG)
    for (int id = static_cast<int>(SharedBus::ServiceID::UI);
         id < static_cast<int>(SharedBus::ServiceID::MAX_ID);
         ++id) {
        auto serviceID = static_cast<SharedBus::ServiceID>(id);
        if (serviceID == SharedBus::ServiceID::LOG) {
            continue;
        }

        if (ServiceFactoryRegistry::IsServiceRegistered(serviceID)) {
            auto initErr = InitializeService(serviceID);
            if (initErr != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize %s service", mServiceName[serviceID]);
                if (firstError == ESP_OK) {
                    firstError = initErr; // preserve first failure
                }
            }
        } else {
            ESP_LOGI(TAG, "%s service not registered, skipping", mServiceName[serviceID]);
        }
    }

    return firstError;
}

esp_err_t ServiceMngr::InitializeService(SharedBus::ServiceID serviceID)
{
    esp_err_t err = ESP_OK;
    
    // Create service instance using factory
    auto service = ServiceFactoryRegistry::CreateService(
        serviceID,
        mServiceName[serviceID]
    );
    
    if (service == nullptr) {
        ESP_LOGE(TAG, "Failed to create service %d", static_cast<int>(serviceID));
        return ESP_FAIL;
    }
    
    // Store service instance
    sServiceInstances[serviceID] = service;
    
    // Initialize task
    TaskHandle_t taskHandle = nullptr;
    err = service->TaskInit(
        &taskHandle,
        tskIDLE_PRIORITY + 1,
        mServiceStackSize[serviceID]
    );
    
    if (err == ESP_OK) {
        sServiceHandles[serviceID] = taskHandle;
        ESP_LOGI(TAG, "%s service created successfully", mServiceName[serviceID]);
    } else {
        ESP_LOGE(TAG, "Failed to initialize %s service task", mServiceName[serviceID]);
        sServiceInstances.erase(serviceID);
    }
    
    return err;
}
