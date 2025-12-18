/**
 * @file ServiceFactory.hpp
 * @brief Service Factory Interface for Dynamic Service Creation
 * 
 * This file provides a factory pattern implementation that allows projects to register
 * device-specific service implementations without modifying ServiceMngr. The factory
 * enables the generalized ServiceMngr to create services dynamically based on project-specific
 * registrations provided in ServiceRegistration.cpp.
 * 
 * @details
 * The ServiceFactoryRegistry uses a registration system where each project registers
 * its device-specific service classes (e.g., UICoffeeMaker, MatterOven, MQTTOven) via
 * convenience macros. ServiceMngr then uses these registered factories to create service
 * instances without hardcoding device-specific types.
 * 
 * @note This factory pattern enables ServiceMngr to be reusable across different projects
 *       with different device types. Each project only needs to customize ServiceRegistration.cpp.
 * 
 * @see ServiceMngr_Generalized
 * @see ServiceRegistration.cpp
 * 
 * @author ESP-IDF Project
 * @date 2024
 */

#pragma once
#include "sdkconfig.h"
#include <memory>
#include <functional>
#include <utility>
#include "ServiceBase.hpp"
#include "SharedBus.hpp"
#include "Singleton.hpp"

/**
 * @typedef ServiceFactoryFunc
 * @brief Function type for creating service instances
 * 
 * This function type defines the signature for factory functions that create service instances.
 * Factory functions receive a task name and service ID, and return a shared pointer to a
 * ServiceBase instance (which is actually a device-specific service class).
 * 
 * @param taskName Name of the FreeRTOS task for the service
 * @param serviceID The service ID from SharedBus::ServiceID enum
 * @return std::shared_ptr<ServiceBase> Shared pointer to the created service instance
 * 
 * @note The returned pointer points to a device-specific service class (e.g., UICoffeeMaker,
 *       MatterOven) that inherits from ServiceBase. The factory typically uses the Singleton
 *       pattern to ensure only one instance exists.
 */
using ServiceFactoryFunc = std::function<std::shared_ptr<ServiceBase>(
    const char*,
    SharedBus::ServiceID
)>;

/**
 * @class ServiceFactoryRegistry
 * @brief Registry for service factory functions
 * 
 * This class provides a centralized registry where projects can register factory functions
 * for creating device-specific service instances. ServiceMngr uses this registry to create
 * services dynamically without hardcoding device-specific types.
 * 
 * @details
 * The registry stores factory functions in an array indexed by SharedBus::ServiceID.
 * Each service type (UI, Matter, MQTT) can have one registered factory function.
 * 
 * **Usage Pattern:**
 * 1. Projects register services in ServiceRegistration.cpp using convenience macros
 * 2. ServiceMngr queries the registry to check if services are registered
 * 3. ServiceMngr creates service instances using registered factories
 * 
 * @note All methods are static - this is a utility class with no instance state
 * @note Thread-safe for single-threaded initialization (registration happens before app_main)
 * 
 * @example
 * @code
 * // In ServiceRegistration.cpp
 * REGISTER_UI_SERVICE(UICoffeeMaker);
 * REGISTER_MATTER_SERVICE(MatterOven);
 * REGISTER_MQTT_SERVICE(MQTTOven);
 * 
 * // In ServiceMngr
 * auto service = ServiceFactoryRegistry::CreateService(
 *     SharedBus::ServiceID::UI,
 *     "UI_Task"
 * );
 * @endcode
 */
class ServiceFactoryRegistry
{
public:
    /**
     * @brief Register a service factory function for a specific service ID
     * 
     * Registers a factory function that will be used to create instances of a service
     * for the specified service ID. If a factory is already registered for the service ID,
     * it will be overwritten with a warning.
     * 
     * @param[in] serviceID The service ID to register (from SharedBus::ServiceID enum)
     * @param[in] factory Function that creates the service instance using Singleton pattern
     * 
     * @return true if registration successful
     * @return false if serviceID is invalid (NO_ID or >= MAX_ID)
     * 
     * @note Registration typically happens in ServiceRegistration.cpp via convenience macros
     * @note If a factory is already registered, it will be overwritten (warning logged)
     * 
     * @warning Invalid service IDs will be rejected and false will be returned
     * 
     * @see REGISTER_UI_SERVICE
     * @see REGISTER_MATTER_SERVICE
     * @see REGISTER_MQTT_SERVICE
     */
    static bool RegisterService(
        SharedBus::ServiceID serviceID,
        ServiceFactoryFunc factory
    );

    /**
     * @brief Create a service instance using the registered factory
     * 
     * Creates a service instance by calling the factory function registered for the
     * specified service ID. The factory function is responsible for creating the
     * service instance (typically using the Singleton pattern).
     * 
     * @param[in] serviceID The service ID to create (from SharedBus::ServiceID enum)
     * @param[in] taskName Name of the FreeRTOS task for the service
     * 
     * @return std::shared_ptr<ServiceBase> Shared pointer to service instance on success
     * @return nullptr if serviceID is invalid or no factory is registered for the service ID
     * 
     * @note The returned pointer points to a device-specific service class that inherits
     *       from ServiceBase (e.g., UICoffeeMaker, MatterOven, MQTTOven)
     * 
     * @warning Returns nullptr if the service is not registered - always check the return value
     * 
     * @see IsServiceRegistered() to check if a service is registered before creating
     */
    static std::shared_ptr<ServiceBase> CreateService(
        SharedBus::ServiceID serviceID,
        const char* taskName
    );

    /**
     * @brief Check if a service factory is registered for a service ID
     * 
     * Queries the registry to determine if a factory function has been registered
     * for the specified service ID. This is useful for conditional service initialization.
     * 
     * @param[in] serviceID The service ID to check (from SharedBus::ServiceID enum)
     * 
     * @return true if a factory is registered for the service ID
     * @return false if serviceID is invalid or no factory is registered
     * 
     * @note This method is typically used by ServiceMngr to check which services
     *       are available before attempting to create them
     * 
     * @see CreateService()
     */
    static bool IsServiceRegistered(SharedBus::ServiceID serviceID);

private:
    /**
     * @brief Array of factory functions indexed by SharedBus::ServiceID
     * 
     * Stores registered factory functions. Array size is SharedBus::ServiceID::MAX_ID.
     * Unregistered services have nullptr entries.
     */
    static ServiceFactoryFunc sFactories[SharedBus::ServiceID::MAX_ID];
};

/**
 * @brief Helper function template for creating service instances
 * 
 * This template function provides a convenient way to create service instances using
 * the Singleton pattern. It handles the type casting required for perfect forwarding
 * in the Singleton template.
 * 
 * @tparam ServiceClass The device-specific service class type (e.g., UICoffeeMaker, MatterOven)
 * 
 * @param[in] taskName Name of the FreeRTOS task for the service
 * @param[in] id The service ID from SharedBus::ServiceID enum
 * 
 * @return std::shared_ptr<ServiceBase> Shared pointer to the created service instance
 * 
 * @note This function is used internally by the registration macros
 * @note The ServiceClass must inherit from ServiceBase
 * 
 * @see REGISTER_UI_SERVICE
 * @see REGISTER_MATTER_SERVICE
 * @see REGISTER_MQTT_SERVICE
 */
template<typename ServiceClass>
inline std::shared_ptr<ServiceBase> CreateServiceInstance(const char* taskName, SharedBus::ServiceID id)
{
    return Singleton<ServiceClass, const char*, SharedBus::ServiceID>::GetInstance(
        static_cast<const char*&&>(taskName),
        static_cast<SharedBus::ServiceID&&>(id)
    );
}

/**
 * @def REGISTER_SERVICE
 * @brief Generic macro for registering any service implementation
 * 
 * Registers a service class with the ServiceFactoryRegistry for the provided ServiceID.
 * Use this instead of the per-service macros to select the ID explicitly.
 * 
 * @param ServiceIdEnum SharedBus::ServiceID value (e.g., SharedBus::ServiceID::UI)
 * @param ServiceClass The service class to register (must inherit from ServiceBase)
 * 
 * @example
 * @code
 * // In ServiceRegistration.cpp
 * REGISTER_SERVICE(SharedBus::ServiceID::UI, UICoffeeMaker);
 * REGISTER_SERVICE(SharedBus::ServiceID::MATTER, MatterOven);
 * REGISTER_SERVICE(SharedBus::ServiceID::MQTT, MQTTOven);
 * @endcode
 */
#define REGISTER_SERVICE(ServiceIdEnum, ServiceClass) \
    ServiceFactoryRegistry::RegisterService( \
        ServiceIdEnum, \
        [](const char* taskName, SharedBus::ServiceID id) -> std::shared_ptr<ServiceBase> { \
            return CreateServiceInstance<ServiceClass>(taskName, id); \
        } \
    )
