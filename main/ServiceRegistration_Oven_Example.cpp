/**
 * @file ServiceRegistration_Oven_Example.cpp
 * @brief Example service registration for Oven device
 * 
 * This is an EXAMPLE file showing how to register services for an Oven device.
 * Copy this file to ServiceRegistration.cpp and customize for your device.
 */

#include "ServiceFactory.hpp"
#include "sdkconfig.h"

#ifdef CONFIG_DONE_COMPONENT_UI2
#include "UICoffeeMaker.hpp"  // Note: UI might still use CoffeeMaker name
#endif

#ifdef CONFIG_DONE_COMPONENT_MATTER
#include "MatterOven.hpp"
#endif

#ifdef CONFIG_DONE_COMPONENT_MQTT
#include "MQTT_Oven.hpp"
#endif

#include "Singleton.hpp"
#include "esp_log.h"

static const char* TAG = "ServiceRegistration";

void RegisterProjectServices()
{
    ESP_LOGI(TAG, "Registering Oven device services...");

#ifdef CONFIG_DONE_COMPONENT_UI2
    // Register UI Service for Oven
    // Note: Check if UICoffeeMaker is actually used for Oven or if there's UIOven
    REGISTER_UI_SERVICE(UICoffeeMaker);
#endif

#ifdef CONFIG_DONE_COMPONENT_MATTER
    // Register Matter Service for Oven
    REGISTER_MATTER_SERVICE(MatterOven);
#endif

#ifdef CONFIG_DONE_COMPONENT_MQTT
    // Register MQTT Service for Oven
    REGISTER_MQTT_SERVICE(MQTTOven);
#endif

    ESP_LOGI(TAG, "Oven device services registered");
}

/**
 * @brief Automatic service registration using GCC/Clang constructor attribute
 * 
 * This function is automatically called by the compiler before app_main() runs,
 * thanks to the __attribute__((constructor)) attribute. This ensures all services
 * (UI, Matter, MQTT) are registered in the ServiceFactoryRegistry before ServiceMngr
 * attempts to create and use them. No manual call is needed - registration happens
 * automatically during program initialization, guaranteeing proper execution order.
 */
__attribute__((constructor))
static void AutoRegisterServices()
{
    RegisterProjectServices();
}
