/**
 * @file ServiceFactory.cpp
 * @brief Implementation of Service Factory Registry
 * 
 * This file implements the ServiceFactoryRegistry class, providing the functionality
 * for registering and creating service instances dynamically. The registry allows
 * projects to register device-specific service implementations without modifying
 * ServiceMngr code.
 * 
 * @details
 * The implementation uses a static array to store factory functions, indexed by
 * SharedBus::ServiceID. Factory functions are registered during program initialization
 * (via __attribute__((constructor)) in ServiceRegistration.cpp) and are used by
 * ServiceMngr to create service instances on demand.
 * 
 * @see ServiceFactory.hpp
 * @see ServiceRegistration.cpp
 * @see ServiceMngr_Generalized
 * 
 * @author ESP-IDF Project
 * @date 2024
 */

#include "ServiceFactory.hpp"
#include "ServiceBase.hpp"
#include "SharedBus.hpp"
#include "ServiceMngr.hpp"  // For mServiceName array
#include <esp_log.h>

static const char* TAG = "ServiceFactory";

/**
 * @brief Get service name string from ServiceID using ServiceMngr::mServiceName
 * 
 * @param serviceID The service ID to get name for
 * @return const char* Service name string, or "NO_ID"/"UNKNOWN" if invalid
 */
static const char* GetServiceName(SharedBus::ServiceID serviceID)
{
    // Use ServiceMngr::mServiceName array (available in both Generalized and Legacy)
    if (serviceID >= SharedBus::ServiceID::MAX_ID || serviceID == SharedBus::ServiceID::NO_ID) {
        return (serviceID == SharedBus::ServiceID::NO_ID) ? "NO_ID" : "UNKNOWN";
    }
    
    const char* name = ServiceMngr::mServiceName[static_cast<int>(serviceID)];
    // Handle empty string for NO_ID (though we check above)
    return (name && name[0] != '\0') ? name : "UNKNOWN";
}

/**
 * @brief Static factory function array initialization
 * 
 * Initializes the factory function array with all entries set to nullptr.
 * Factory functions are registered at runtime via RegisterService() calls
 * in ServiceRegistration.cpp.
 */
ServiceFactoryFunc ServiceFactoryRegistry::sFactories[SharedBus::ServiceID::MAX_ID] = {nullptr};

/**
 * @brief Register a service factory function for a specific service ID
 * 
 * @details
 * This function stores the provided factory function in the registry array at the
 * index corresponding to the service ID. If a factory is already registered for
 * the service ID, it will be overwritten with a warning logged.
 * 
 * **Validation:**
 * - Checks if serviceID is valid (not NO_ID and < MAX_ID)
 * - Logs error and returns false for invalid service IDs
 * - Logs warning if overwriting an existing registration
 * 
 * **Registration:**
 * - Stores factory function in sFactories array
 * - Logs success message
 * 
 * @param[in] serviceID The service ID to register (from SharedBus::ServiceID enum)
 * @param[in] factory Function that creates the service instance using Singleton pattern
 * 
 * @return true if registration successful
 * @return false if serviceID is invalid (NO_ID or >= MAX_ID)
 * 
 * @note This function is typically called via convenience macros (REGISTER_UI_SERVICE, etc.)
 *       in ServiceRegistration.cpp during program initialization
 * 
 * @warning Invalid service IDs will be rejected and false will be returned
 * @warning Overwriting an existing registration will log a warning but proceed
 */
bool ServiceFactoryRegistry::RegisterService(
    SharedBus::ServiceID serviceID,
    ServiceFactoryFunc factory
)
{
    // Validate service ID
    if (serviceID >= SharedBus::ServiceID::MAX_ID || serviceID == SharedBus::ServiceID::NO_ID) {
        ESP_LOGE(TAG, "Invalid service ID: %d (%s)", static_cast<int>(serviceID), GetServiceName(serviceID));
        return false;
    }

    // Check if already registered (overwriting)
    if (sFactories[static_cast<int>(serviceID)] != nullptr) {
        ESP_LOGW(TAG, "Service %d (%s) already registered, overwriting", static_cast<int>(serviceID), GetServiceName(serviceID));
    }

    // Register factory function
    sFactories[static_cast<int>(serviceID)] = factory;
    ESP_LOGI(TAG, "Service %d (%s) registered successfully", static_cast<int>(serviceID), GetServiceName(serviceID));
    return true;
}

/**
 * @brief Create a service instance using the registered factory
 * 
 * @details
 * This function retrieves the factory function registered for the specified service ID
 * and calls it to create a service instance. The factory function is responsible for
 * creating the service using the Singleton pattern and returning a shared pointer.
 * 
 * **Process:**
 * 1. Validates service ID
 * 2. Retrieves factory function from registry
 * 3. Checks if factory is registered (not nullptr)
 * 4. Calls factory function with taskName and serviceID
 * 5. Returns created service instance
 * 
 * @param[in] serviceID The service ID to create (from SharedBus::ServiceID enum)
 * @param[in] taskName Name of the FreeRTOS task for the service
 * 
 * @return std::shared_ptr<ServiceBase> Shared pointer to service instance on success
 * @return nullptr if serviceID is invalid or no factory is registered for the service ID
 * 
 * @note The returned pointer points to a device-specific service class (e.g., UICoffeeMaker,
 *       MatterOven) that inherits from ServiceBase
 * 
 * @note This function is called by ServiceMngr::InitializeService() to create service instances
 * 
 * @warning Returns nullptr if the service is not registered - always check the return value
 * @warning Invalid service IDs will return nullptr
 */
std::shared_ptr<ServiceBase> ServiceFactoryRegistry::CreateService(
    SharedBus::ServiceID serviceID,
    const char* taskName
)
{
    // Validate service ID
    if (serviceID >= SharedBus::ServiceID::MAX_ID || serviceID == SharedBus::ServiceID::NO_ID) {
        ESP_LOGE(TAG, "Invalid service ID: %d (%s)", static_cast<int>(serviceID), GetServiceName(serviceID));
        return nullptr;
    }

    // Get factory function from registry
    ServiceFactoryFunc factory = sFactories[static_cast<int>(serviceID)];
    
    // Check if factory is registered
    if (factory == nullptr) {
        ESP_LOGW(TAG, "Service %d (%s) not registered", static_cast<int>(serviceID), GetServiceName(serviceID));
        return nullptr;
    }

    // Call factory function to create service instance
    return factory(taskName, serviceID);
}

/**
 * @brief Check if a service factory is registered for a service ID
 * 
 * @details
 * This function queries the registry to determine if a factory function has been
 * registered for the specified service ID. It performs validation and checks if
 * the factory function pointer is not nullptr.
 * 
 * **Process:**
 * 1. Validates service ID
 * 2. Checks if factory function is registered (not nullptr)
 * 3. Returns registration status
 * 
 * @param[in] serviceID The service ID to check (from SharedBus::ServiceID enum)
 * 
 * @return true if a factory is registered for the service ID
 * @return false if serviceID is invalid or no factory is registered
 * 
 * @note This method is typically used by ServiceMngr to check which services
 *       are available before attempting to create them
 * 
 * @note Invalid service IDs return false (not registered)
 * 
 * @see CreateService() to create service instances after checking registration
 */
bool ServiceFactoryRegistry::IsServiceRegistered(SharedBus::ServiceID serviceID)
{
    // Validate service ID
    if (serviceID >= SharedBus::ServiceID::MAX_ID || serviceID == SharedBus::ServiceID::NO_ID) {
        return false;
    }
    
    // Check if factory is registered (not nullptr)
    return sFactories[static_cast<int>(serviceID)] != nullptr;
}
