#pragma once
/**
 * @file ServiceMngr.hpp
 * @brief Service Manager Header - Selects between Generalized and Legacy implementations
 * 
 * This header provides a unified interface for ServiceMngr, allowing selection
 * between the generalized (factory-based) and legacy (hardcoded) implementations
 * via Kconfig option.
 * 
 * @note Selection is controlled by CONFIG_USE_GENERALIZED_SERVICE_MANAGER in menuconfig:
 *       - Enabled (default): Uses ServiceMngr_Generalized (factory pattern, reusable)
 *       - Disabled: Uses ServiceMngr_Legacy (hardcoded device classes)
 * 
 * @note The generalized version requires ServiceRegistration.cpp to be implemented
 *       in your project to register device-specific services.
 * 
 * @note The legacy version requires device-specific headers to be included
 *       and hardcodes device classes (UICoffeeMaker, MatterOven, MQTTOven).
 * 
 * Usage:
 * @code
 * // Configure via menuconfig:
 * // idf.py menuconfig -> Done Main Config -> Done Firmware components
 * // -> Use Generalized Service Manager (Factory Pattern)
 * 
 * #include "ServiceMngr.hpp"  // Automatically uses selected implementation
 * @endcode
 */

#include "sdkconfig.h"

// Select implementation based on Kconfig option
#ifdef CONFIG_USE_GENERALIZED_SERVICE_MANAGER
    // Use generalized version (factory pattern, reusable)
    #include "ServiceMngr_Generalized.hpp"
    #ifndef SERVICE_MANAGER_IMPL
        #define SERVICE_MANAGER_IMPL "Generalized"
    #endif
#else
    // Use legacy version (hardcoded device classes)
    #include "ServiceMngr_Legacy.hpp"
    #ifndef SERVICE_MANAGER_IMPL
        #define SERVICE_MANAGER_IMPL "Legacy"
    #endif
#endif
