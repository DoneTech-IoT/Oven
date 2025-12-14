#pragma once
/**
 * @file ServiceMngr.hpp
 * @brief Service Manager Header - Selects between Generalized and Legacy implementations
 * 
 * This header provides a unified interface for ServiceMngr, allowing selection
 * between the generalized (factory-based) and legacy (hardcoded) implementations
 * via configuration macro.
 * 
 * @def USE_GENERALIZED_SERVICE_MANAGER
 * @brief Macro to select ServiceMngr implementation
 * 
 * - If defined (or default): Uses ServiceMngr_Generalized (factory pattern, reusable)
 * - If undefined: Uses ServiceMngr_Legacy (hardcoded device classes)
 * 
 * @note The generalized version requires ServiceRegistration.cpp to be implemented
 *       in your project to register device-specific services.
 * 
 * @note The legacy version requires device-specific headers to be included
 *       and hardcodes device classes (UICoffeeMaker, MatterOven, MQTTOven).
 * 
 * Usage:
 * @code
 * // Use generalized version (default, recommended)
 * #include "ServiceMngr.hpp"
 * 
 * // Or explicitly use generalized
 * #define USE_GENERALIZED_SERVICE_MANAGER
 * #include "ServiceMngr.hpp"
 * 
 * // Use legacy version
 * #undef USE_GENERALIZED_SERVICE_MANAGER
 * #include "ServiceMngr.hpp"
 * @endcode
 */

#include "sdkconfig.h"

// Default to generalized version if not explicitly set
#ifndef USE_GENERALIZED_SERVICE_MANAGER
    #ifdef CONFIG_USE_GENERALIZED_SERVICE_MANAGER
        #define USE_GENERALIZED_SERVICE_MANAGER
    #else
        // Default to generalized for new projects
        #define USE_GENERALIZED_SERVICE_MANAGER
    #endif
#endif

// Select implementation based on macro
#ifdef USE_GENERALIZED_SERVICE_MANAGER
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
