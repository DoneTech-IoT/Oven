/**
 * @file ServiceRegistration.hpp
 * @brief Project-specific service registration
 * 
 * This file is project-specific and should be customized for each device.
 * Register your device-specific service implementations here.
 * 
 * ServiceMngr will use these registrations to create service instances
 * without needing to know the specific device types.
 * 
 * @note This file is header-only for simplicity. The registration function
 *       is called automatically via __attribute__((constructor)) or manually
 *       from app_main() as a fallback.
 * 
 * @note Only needed when CONFIG_USE_GENERALIZED_SERVICE_MANAGER is enabled.
 */

#pragma once

#include "ServiceFactory.hpp"
#include "sdkconfig.h"

// Include device-specific service headers
#ifdef CONFIG_DONE_COMPONENT_UI2
#include "UICoffeeMaker.hpp"  // Note: UI might still use CoffeeMaker name for Oven
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

// Guard to prevent duplicate registration
static bool sServicesRegistered = false;

/**
 * @brief Register all services for this project
 * 
 * This function is called automatically before ServiceMngr initialization via
 * __attribute__((constructor)), or manually from app_main() as a fallback.
 * Register your device-specific implementations here.
 * 
 * @note This function is idempotent - it can be called multiple times safely.
 *       Subsequent calls will be ignored after the first successful registration.
 * 
 * @note If both __attribute__((constructor)) and manual call happen, only the
 *       first call will register services (subsequent calls are no-ops).
 */
inline void RegisterServices()
{
    // Prevent duplicate registration
    if (sServicesRegistered) {
        ESP_LOGD(TAG, "Services already registered, skipping");
        return;
    }
    
    ESP_LOGI(TAG, "Registering project services...");

#ifdef CONFIG_DONE_COMPONENT_UI2
    // Register UI Service for Oven
    // Note: Check if UICoffeeMaker is actually used for Oven or if there's UIOven
    REGISTER_SERVICE(SharedBus::ServiceID::UI, UICoffeeMaker);
#endif

#ifdef CONFIG_DONE_COMPONENT_MATTER
    // Register Matter Service for Oven
    REGISTER_SERVICE(SharedBus::ServiceID::MATTER, MatterOven);
#endif

#ifdef CONFIG_DONE_COMPONENT_MQTT
    // Register MQTT Service for Oven
    REGISTER_SERVICE(SharedBus::ServiceID::MQTT, MQTTOven);
#endif

    ESP_LOGI(TAG, "Service registration complete");
    sServicesRegistered = true;
}

/**
 * @brief Automatic service registration using GCC/Clang constructor attribute
 * 
 * This function attempts to be automatically called by the compiler before app_main()
 * runs, thanks to the __attribute__((constructor)) attribute. However, in ESP-IDF with
 * Xtensa GCC, constructor execution timing is not guaranteed, so a manual call in
 * app_main() is also provided as a fallback.
 * 
 * @note Xtensa GCC supports `__attribute__((constructor))`, but execution timing
 *       relative to app_main() may vary. The manual call in app_main() ensures
 *       reliable registration regardless of constructor behavior.
 * 
 * @note Registration is idempotent - safe to call multiple times (both constructor
 *       and manual call can execute without issues).
 */
__attribute__((constructor))
static void AutoRegisterServices()
{    
    RegisterServices(); 
}
