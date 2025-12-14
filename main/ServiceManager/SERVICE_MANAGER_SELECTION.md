# ServiceManager Selection Guide

## Overview

The `ServiceMngr.hpp` header provides a unified interface that allows you to select between two implementations:

1. **ServiceMngr_Generalized** (Recommended) - Factory pattern, reusable
2. **ServiceMngr_Legacy** - Hardcoded device classes, project-specific

## Selection Methods

### Method 1: Macro Definition (Code Level)

#### Use Generalized Version (Default)
```cpp
// Default behavior - uses generalized
#include "ServiceMngr.hpp"

// Or explicitly
#define USE_GENERALIZED_SERVICE_MANAGER
#include "ServiceMngr.hpp"
```

#### Use Legacy Version
```cpp
#undef USE_GENERALIZED_SERVICE_MANAGER
#include "ServiceMngr.hpp"
```

### Method 2: Kconfig (menuconfig)

Add to your `main/Kconfig`:

```kconfig
menu "Service Manager Configuration"
    config USE_GENERALIZED_SERVICE_MANAGER
        bool "Use Generalized Service Manager (Factory Pattern)"
        default y
        help
            Enable the generalized ServiceManager that uses factory pattern.
            This version is reusable across projects and requires ServiceRegistration.cpp.
            
            If disabled, uses the legacy hardcoded version.
endmenu
```

Then in `ServiceMngr.hpp`, it will automatically use the config:

```cpp
#ifdef CONFIG_USE_GENERALIZED_SERVICE_MANAGER
    #define USE_GENERALIZED_SERVICE_MANAGER
#endif
```

### Method 3: Compiler Flag (CMakeLists.txt)

In `main/CMakeLists.txt`:

```cmake
# Use generalized version
target_compile_definitions(${COMPONENT_LIB} PRIVATE USE_GENERALIZED_SERVICE_MANAGER)

# Or use legacy
# target_compile_definitions(${COMPONENT_LIB} PRIVATE -DUSE_LEGACY_SERVICE_MANAGER)
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
#undef USE_GENERALIZED_SERVICE_MANAGER
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
3. **Switch include**:
   ```cpp
   // Old
   #include "ServiceMngr_Legacy.hpp"
   
   // New
   #include "ServiceMngr.hpp"  // Uses generalized by default
   ```
4. **Test and verify**
5. **Remove legacy version** (optional, after verification)

### From Generalized to Legacy

1. **Undefine macro**:
   ```cpp
   #undef USE_GENERALIZED_SERVICE_MANAGER
   #include "ServiceMngr.hpp"
   ```
2. **Ensure device headers** are available
3. **Test and verify**

## Verification

Check which version is being used:

```cpp
#include "ServiceMngr.hpp"

#ifdef USE_GENERALIZED_SERVICE_MANAGER
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



