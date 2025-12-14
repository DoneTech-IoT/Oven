#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Custom_Log.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_log.h"

#include "ServiceMngr.hpp"  // Automatically selects Generalized or Legacy based on macro
#include "Singleton.hpp"
#include "BSP.h"

// Forward declaration - ensure services are registered before ServiceMngr is created
extern void RegisterProjectServices();

static TaskHandle_t SrvMngHandle;
static std::shared_ptr<ServiceMngr> serviceMngr;

// Define the heartbeat pattern in milliseconds
const int HeartbeatPattern[] = {
    200, // First "lub" (on time)
    100, // Pause between "lub" and "dub"
    200, // Second "dub" (on time)
    1000 // Rest time before the next heartbeat
};
const int HeartbeatPatternLength = sizeof(HeartbeatPattern) / sizeof(HeartbeatPattern[0]);
static const char *TAG = "Main";

/**
 * @brief Function to change colors based on a timer callback
 */
extern "C" void app_main()
{        
    // Ensure services are registered before creating ServiceMngr
    // Note: __attribute__((constructor)) may not execute reliably in ESP-IDF/Xtensa GCC,
    // so this manual call ensures registration happens. Registration is idempotent,
    // so it's safe to call even if constructor already executed.
    RegisterProjectServices();
    
    Log_RamOccupy("main", "service manager");        
    serviceMngr = Singleton<ServiceMngr, const char*, SharedBus::ServiceID>::
                    GetInstance(static_cast<const char*>
                        (ServiceMngr::mServiceName[SharedBus::ServiceID::SERVICE_MANAGER]),
                        SharedBus::ServiceID::SERVICE_MANAGER);     
    Log_RamOccupy("main", "service manager");        

    gpio_config_t heartBeatConf;
    heartBeatConf.intr_type = GPIO_INTR_DISABLE;
    heartBeatConf.mode = GPIO_MODE_OUTPUT;
    heartBeatConf.pin_bit_mask = (1ULL << BSP_HEARTBEAT_GPIO);
    heartBeatConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    heartBeatConf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&heartBeatConf);    

    while (true)
    {
        for (int i = 0; i < HeartbeatPatternLength; i++) 
        {            
            gpio_set_level(BSP_HEARTBEAT_GPIO, i % 2);            
            vTaskDelay(pdMS_TO_TICKS(HeartbeatPattern[i]));
        }
    }
}