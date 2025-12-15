# ServiceManager Selection Guide

## Overview

The `ServiceMngr.hpp` header provides a unified interface that allows you to select between two implementations:

1. **ServiceMngr_Generalized** (Recommended) - Factory pattern, reusable
2. **ServiceMngr_Legacy** - Hardcoded device classes, project-specific

## Selection Method: Kconfig (menuconfig)

The selection is controlled via Kconfig option `CONFIG_USE_GENERALIZED_SERVICE_MANAGER` in `main/Kconfig`:

```kconfig
menu "Done Firmware components"
    config USE_GENERALIZED_SERVICE_MANAGER
        bool "Use Generalized Service Manager (Factory Pattern)"
        default y
        help
            Enable the generalized ServiceManager that uses factory pattern.
            This version is reusable across projects and requires ServiceRegistration.cpp.
            
            If disabled, uses the legacy hardcoded version (ServiceMngr_Legacy).
            Legacy version requires device-specific headers and hardcodes device classes.
endmenu
```

### How to Select

1. **Use Generalized (Recommended)**:
   ```bash
   idf.py menuconfig
   # Navigate to: Done Main Config -> Done Firmware components
   # Enable: "Use Generalized Service Manager (Factory Pattern)"
   ```

2. **Use Legacy**:
   ```bash
   idf.py menuconfig
   # Navigate to: Done Main Config -> Done Firmware components
   # Disable: "Use Generalized Service Manager (Factory Pattern)"
   ```

The `ServiceMngr.hpp` header automatically uses the Kconfig setting:

```cpp
#ifdef CONFIG_USE_GENERALIZED_SERVICE_MANAGER
    #include "ServiceMngr_Generalized.hpp"
#else
    #include "ServiceMngr_Legacy.hpp"
#endif
```

## Implementation Comparison

| Feature | Generalized | Legacy |
|---------|------------|--------|
| **Reusability** | ✅ Yes | ❌ No |
| **Device-Specific Code** | ❌ No | ✅ Yes (hardcoded) |
| **Factory Pattern** | ✅ Yes | ❌ No |
| **ServiceRegistration.cpp** | ✅ Required | ❌ Not used |
| **Device Headers** | ❌ Not needed | ✅ Required |
| **Project Changes** | Only ServiceRegistration.cpp | Edit ServiceMngr files |

## Usage Examples

### Example 1: Using Generalized (Recommended)

```cpp
// app_main.cpp
#include "ServiceMngr.hpp"  // Automatically uses generalized

extern "C" void app_main()
{
    auto serviceMngr = Singleton<ServiceMngr, ...>::GetInstance(...);
    // ServiceMngr uses factory to create services
}
```

**Requires**: `ServiceRegistration.cpp` with device registrations

### Example 2: Using Legacy

```cpp
// app_main.cpp
// Disable CONFIG_USE_GENERALIZED_SERVICE_MANAGER in menuconfig first
#include "ServiceMngr.hpp"  // Uses legacy

extern "C" void app_main()
{
    auto serviceMngr = Singleton<ServiceMngr, ...>::GetInstance(...);
    // ServiceMngr uses hardcoded device classes
}
```

**Requires**: Device headers included in ServiceMngr_Legacy.hpp

## Migration Path

### From Legacy to Generalized

1. **Keep both versions** (already done)
2. **Create ServiceRegistration.cpp** with your device registrations
3. **Enable Kconfig option** (default is enabled):
   ```bash
   idf.py menuconfig
   # Navigate to: Done Main Config -> Done Firmware components
   # Enable: "Use Generalized Service Manager (Factory Pattern)"
   ```
4. **Switch include**:
   ```cpp
   // Old
   #include "ServiceMngr_Legacy.hpp"
   
   // New
   #include "ServiceMngr.hpp"  // Uses generalized (via Kconfig)
   ```
5. **Test and verify**
6. **Remove legacy version** (optional, after verification)

### From Generalized to Legacy

1. **Disable Kconfig option**:
   ```bash
   idf.py menuconfig
   # Navigate to: Done Main Config -> Done Firmware components
   # Disable: "Use Generalized Service Manager (Factory Pattern)"
   ```
2. **Ensure device headers** are available
3. **Test and verify**

## Verification

Check which version is being used:

```cpp
#include "ServiceMngr.hpp"

#ifdef CONFIG_USE_GENERALIZED_SERVICE_MANAGER
    ESP_LOGI("Main", "Using Generalized ServiceManager");
#else
    ESP_LOGI("Main", "Using Legacy ServiceManager");
#endif
```

Or check the `SERVICE_MANAGER_IMPL` macro:

```cpp
ESP_LOGI("Main", "ServiceManager implementation: %s", SERVICE_MANAGER_IMPL);
// Output: "ServiceManager implementation: Generalized"
// or: "ServiceManager implementation: Legacy"
```

## Recommendations

### ✅ Use Generalized When:
- Starting a new project
- Want reusable main module
- Supporting multiple device types
- Want clean separation of concerns

### ⚠️ Use Legacy When:
- Migrating existing project gradually
- Need to maintain exact old behavior
- Temporary compatibility needed

## File Structure

```
main/ServiceManager/
├── ServiceMngr.hpp              ← Wrapper header (selects implementation)
├── ServiceMngr_Generalized.hpp ← Generalized version
├── ServiceMngr_Generalized.cpp
├── ServiceMngr_Legacy.hpp        ← Legacy version
├── ServiceMngr_Legacy.cpp
└── ServiceFactory.*             ← Used by generalized version
```

## Summary

The `ServiceMngr.hpp` wrapper provides:
- ✅ **Unified interface** - Same class name, different implementations
- ✅ **Easy selection** - Macro or config-based
- ✅ **Backward compatibility** - Can use legacy if needed
- ✅ **Gradual migration** - Switch when ready

**Default**: Uses Generalized (recommended for new projects)






