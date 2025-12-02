#include "nvsFlash.h"
#include <cstring>
#include "esp_heap_caps.h"

#include "ServiceMngr.hpp"
#include "Singleton.hpp"
#include "SpiffsManager.h"
#include "SharedBus.hpp"

static const char* TAG = "ServiceMngr";
TaskHandle_t ServiceMngr::SrvMngHandle = nullptr;
ServiceMngr::ServiceParams_t ServiceMngr::mServiceParams[SharedBus::ServiceID::MAX_ID];
#ifdef CONFIG_DONE_COMPONENT_UI2
TaskHandle_t ServiceMngr::LVGLHandle = nullptr;
std::shared_ptr<UICoffeeMaker> ServiceMngr::uiCoffeeMaker;
#endif  
#ifdef CONFIG_DONE_COMPONENT_MATTER
TaskHandle_t ServiceMngr::MatterHandle = nullptr;
std::shared_ptr<MatterOven> ServiceMngr::matterOven;
#endif
#ifdef CONFIG_DONE_COMPONENT_MQTT
TaskHandle_t ServiceMngr::MQTTHandle = nullptr;
std::shared_ptr<MQTTOven> ServiceMngr::mqttOvenApp;
#endif

#include "esp_heap_caps.h"

ServiceMngr::ServiceMngr(
    const char *TaskName,
    const SharedBus::ServiceID &ServiceID) :
    ServiceBase(TaskName, ServiceID)
{
    esp_err_t err;    

    nvsFlashInit();

    SpiffsInit();

    SharedBus sharedBus;
    if(sharedBus.Init() == ESP_OK)
    {
        ESP_LOGI(TAG, "Initialized SharedBus successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to Initialize SharedBus.");
    }                

#ifdef MONITORING
// char pcTaskList[TASK_LIST_BUFFER_SIZE];
#endif    

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
    esp_err_t err = ESP_OK;
    
#ifdef CONFIG_DONE_COMPONENT_UI2
    uiCoffeeMaker = Singleton<UICoffeeMaker, const char*, SharedBus::ServiceID>::
                        GetInstance(static_cast<const char*>
                        (mServiceName[SharedBus::ServiceID::UI]),
                        SharedBus::ServiceID::UI);                
            
    err = uiCoffeeMaker->TaskInit(
        &LVGLHandle,
        tskIDLE_PRIORITY + 1,
        mServiceStackSize[SharedBus::ServiceID::UI]);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created.",
            mServiceName[SharedBus::ServiceID::UI]);
    }
    else
    {
        ESP_LOGE(TAG,"failed to create %s service",
            mServiceName[SharedBus::ServiceID::UI]);
    }
    
    // Give LVGL time to initialize display and interrupt handlers
    // before Matter starts WiFi (which also uses interrupts)
    // This prevents interrupt watchdog timeout when both services run together
    #ifdef CONFIG_DONE_COMPONENT_MATTER
    vTaskDelay(pdMS_TO_TICKS(500)); // 500ms delay to allow LVGL to stabilize
    #endif
#endif //CONFIG_DONE_COMPONENT_UI2

#ifdef CONFIG_DONE_COMPONENT_MATTER 
    matterOven = Singleton<MatterOven, const char*, SharedBus::ServiceID>::
                            GetInstance(static_cast<const char*>
                            (mServiceName[SharedBus::ServiceID::MATTER]), 
                            SharedBus::ServiceID::MATTER);    
    
    err = matterOven->TaskInit(
            &MatterHandle,
            tskIDLE_PRIORITY + 1,
            mServiceStackSize[SharedBus::ServiceID::MATTER]);
    
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created",
            mServiceName[SharedBus::ServiceID::MATTER]);
    }     
    else
    {
        ESP_LOGE(TAG, "failed to create %s service",
                 mServiceName[SharedBus::ServiceID::MATTER]);
    }
#endif // CONFIG_DONE_COMPONENT_MATTER

#ifdef CONFIG_DONE_COMPONENT_MQTT
    mqttOvenApp = Singleton<MQTTOven, const char *, SharedBus::ServiceID>::
        GetInstance(static_cast<const char *>(mServiceName[SharedBus::ServiceID::MQTT]),
                    SharedBus::ServiceID::MQTT);

    err = mqttOvenApp->TaskInit(
        &MQTTHandle,
        tskIDLE_PRIORITY + 1,
        mServiceStackSize[SharedBus::ServiceID::MQTT]);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "%s service created.",
                 mServiceName[SharedBus::ServiceID::MQTT]);
    }
    else
    {
        ESP_LOGE(TAG, "failed to create %s service.",
                 mServiceName[SharedBus::ServiceID::MQTT]);
    }
#endif // CONFIG_DONE_COMPONENT_MQTT

    return err;
}