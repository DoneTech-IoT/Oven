#!/bin/bash
#
# sync_bsp_to_modules.sh
#
# Syncs BSP dependency from main/idf_component.yml to all module idf_component.yml files
# This ensures all modules (Matter, MQTT, UserInterface2, etc.) have the correct BSP dependency
# without manual maintenance
#
# Usage:
#   ./sync_bsp_to_modules.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
MAIN_COMPONENT_YML="$PROJECT_DIR/main/idf_component.yml"

cd "$PROJECT_DIR"

if [ ! -f "$MAIN_COMPONENT_YML" ]; then
    echo "  ✗ ERROR: $MAIN_COMPONENT_YML not found"
    exit 1
fi

echo "Syncing BSP dependency from main/idf_component.yml to all module idf_component.yml files..."
echo ""

# Extract BSP component info from main/idf_component.yml
AWK_READ_SCRIPT=$(mktemp)
cat > "$AWK_READ_SCRIPT" << 'AWK_READ_EOF'
BEGIN { 
    in_deps=0
    comp_name=""
    git_repo=""
    path_val=""
    version_val=""
}
/^dependencies:/ { 
    in_deps=1
    next
}
in_deps && /^[[:space:]]*[^[:space:]#]+:[[:space:]]*$/ {
    if (comp_name != "" && git_repo != "") {
        print comp_name "|" git_repo "|" path_val "|" version_val
        exit
    }
    # Extract component name
    if (match($0, /^[[:space:]]*[^:]+:/)) {
        comp_name = substr($0, RSTART)
        sub(/^[[:space:]]*/, "", comp_name)
        sub(/:[[:space:]]*$/, "", comp_name)
    }
    git_repo=""
    path_val=""
    version_val=""
    next
}
comp_name != "" && /git:[[:space:]]*"/ {
    if (match($0, /git:[[:space:]]*"[^"]+"/)) {
        git_repo = substr($0, RSTART, RLENGTH)
        sub(/git:[[:space:]]*"/, "", git_repo)
        sub(/"$/, "", git_repo)
    }
}
comp_name != "" && /path:[[:space:]]*"/ {
    if (match($0, /path:[[:space:]]*"[^"]+"/)) {
        path_val = substr($0, RSTART, RLENGTH)
        sub(/path:[[:space:]]*"/, "", path_val)
        sub(/"$/, "", path_val)
    }
}
comp_name != "" && /version:[[:space:]]*"/ {
    if (match($0, /version:[[:space:]]*"[^"]+"/)) {
        version_val = substr($0, RSTART, RLENGTH)
        sub(/version:[[:space:]]*"/, "", version_val)
        sub(/"$/, "", version_val)
    }
}
END {
    if (comp_name != "" && git_repo != "") {
        print comp_name "|" git_repo "|" path_val "|" version_val
    }
}
AWK_READ_EOF

BSP_INFO=$(awk -f "$AWK_READ_SCRIPT" "$MAIN_COMPONENT_YML")
rm -f "$AWK_READ_SCRIPT"

if [ -z "$BSP_INFO" ]; then
    echo "  ✗ ERROR: Could not find BSP component in $MAIN_COMPONENT_YML"
    exit 1
fi

# Parse the result: component_name|git_repo|path|version
BSP_COMPONENT_NAME=$(echo "$BSP_INFO" | cut -d'|' -f1)
BSP_GIT_REPO=$(echo "$BSP_INFO" | cut -d'|' -f2)
BSP_PATH=$(echo "$BSP_INFO" | cut -d'|' -f3)
BSP_VERSION=$(echo "$BSP_INFO" | cut -d'|' -f4)

echo "  ✓ Found BSP component: $BSP_COMPONENT_NAME"
echo "  ✓ Found BSP repository: $BSP_GIT_REPO"
if [ -n "$BSP_PATH" ]; then
    echo "  ✓ Found BSP path: $BSP_PATH"
fi
if [ -n "$BSP_VERSION" ]; then
    echo "  ✓ Found BSP version: $BSP_VERSION"
fi
echo ""

# Find all component idf_component.yml files (exclude PrebuiltLibs)
COMPONENT_YML_FILES=$(find components -name "idf_component.yml" -type f ! -path "*/PrebuiltLibs/*" | sort)

if [ -z "$COMPONENT_YML_FILES" ]; then
    echo "  ⚠ WARNING: No component idf_component.yml files found"
    exit 0
fi

# Process each component yml file
for COMPONENT_YML in $COMPONENT_YML_FILES; do
    COMPONENT_NAME=$(basename $(dirname "$COMPONENT_YML"))
    echo "Processing: $COMPONENT_YML"
    
    # Backup component yml
    cp "$COMPONENT_YML" "$COMPONENT_YML.bak"
    
    # Update component yml with BSP dependency
    ESCAPED_COMP_NAME=$(echo "$BSP_COMPONENT_NAME" | sed 's/[\/&]/\\&/g')
    
    # Use awk to update or add BSP dependency
    AWK_UPDATE_SCRIPT=$(mktemp)
    cat > "$AWK_UPDATE_SCRIPT" << 'AWK_UPDATE_EOF'
BEGIN { 
    found=0
    in_section=0
    in_comment_section=0
    indent=""
    pattern = sprintf("^[[:space:]]*%s[[:space:]]*:[[:space:]]*$", escaped_comp)
    comment_pattern = sprintf("^[[:space:]]*#[[:space:]]*%s[[:space:]]*:[[:space:]]*$", escaped_comp)
    printed=0
}
{
    # Check if we're entering a commented BSP component section (remove it)
    if (match($0, comment_pattern)) {
        in_comment_section=1
        # Extract indentation for later use
        if (match($0, /^[[:space:]]*/)) {
            indent = substr($0, RSTART, RLENGTH)
        }
        next
    }
    
    # Skip lines in commented BSP section (including comment lines and indented lines)
    if (in_comment_section) {
        # Check if this is a new top-level dependency (end of commented section)
        # Also check for uncommented BSP entry (shouldn't happen, but handle it)
        if (match($0, /^[[:space:]]*[^[:space:]#]+:[[:space:]]*$/)) {
            in_comment_section=0
            # Print new BSP section before this line
            printf "%s%s:\n", indent, comp_name
            if (git_repo != "") {
                printf "%s  git: \"%s\"\n", indent, git_repo
            }
            if (path_val != "") {
                printf "%s  path: \"%s\"\n", indent, path_val
            }
            if (version_val != "") {
                printf "%s  version: \"%s\"\n", indent, version_val
            } else {
                printf "%s  version: \"*\"\n", indent
            }
            printed=1
            found=1
            print $0
            next
        }
        # Skip all commented lines and indented lines in commented section
        # Also skip lines that look like they're part of the commented BSP entry
        if (match($0, /^[[:space:]]*#/) || match($0, /^[[:space:]]+[^[:space:]]/)) {
            next
        }
        # If we hit a blank line or something else, end the commented section
        in_comment_section=0
    }
    
    # Check if we're entering the BSP component section (uncommented)
    if (match($0, pattern)) {
        found=1
        in_section=1
        # Extract indentation
        if (match($0, /^[[:space:]]*/)) {
            indent = substr($0, RSTART, RLENGTH)
        }
        # Don't print the old entry, we'll replace it
        next
    }
    
    # If we're in the BSP section, skip old content until we hit next top-level key
    if (in_section) {
        # Check if this is a new top-level dependency (starts with letter, has colon, same indent level)
        if (match($0, sprintf("^%s[^[:space:]]+:[[:space:]]*$", indent))) {
            in_section=0
            # Print new BSP section before this line
            printf "%s%s:\n", indent, comp_name
            if (git_repo != "") {
                printf "%s  git: \"%s\"\n", indent, git_repo
            }
            if (path_val != "") {
                printf "%s  path: \"%s\"\n", indent, path_val
            }
            if (version_val != "") {
                printf "%s  version: \"%s\"\n", indent, version_val
            } else {
                printf "%s  version: \"*\"\n", indent
            }
            printed=1
            print $0
            next
        }
        # Skip lines that are part of old BSP section
        next
    }
    
    print $0
}
END {
    # If we found the section but didn't print yet (it was at the end of file)
    if (found && !printed) {
        printf "%s%s:\n", indent, comp_name
        if (git_repo != "") {
            printf "%s  git: \"%s\"\n", indent, git_repo
        }
        if (path_val != "") {
            printf "%s  path: \"%s\"\n", indent, path_val
        }
        if (version_val != "") {
            printf "%s  version: \"%s\"\n", indent, version_val
        } else {
            printf "%s  version: \"*\"\n", indent
        }
    }
    # If BSP wasn't found at all, add it at the end
    if (!found) {
        printf "  %s:\n", comp_name
        if (git_repo != "") {
            printf "    git: \"%s\"\n", git_repo
        }
        if (path_val != "") {
            printf "    path: \"%s\"\n", path_val
        }
        if (version_val != "") {
            printf "    version: \"%s\"\n", version_val
        } else {
            printf "    version: \"*\"\n"
        }
    }
}
AWK_UPDATE_EOF
    
    # Run awk script with variables
    awk -v escaped_comp="$ESCAPED_COMP_NAME" \
        -v comp_name="$BSP_COMPONENT_NAME" \
        -v git_repo="$BSP_GIT_REPO" \
        -v path_val="$BSP_PATH" \
        -v version_val="$BSP_VERSION" \
        -f "$AWK_UPDATE_SCRIPT" "$COMPONENT_YML.bak" > "$COMPONENT_YML"
    rm -f "$AWK_UPDATE_SCRIPT"
    
    # Remove backup if update was successful
    rm -f "$COMPONENT_YML.bak"
    
    echo "  ✓ Updated $COMPONENT_YML"
done

echo ""
echo "  ✓ BSP dependency synced to all module idf_component.yml files"
