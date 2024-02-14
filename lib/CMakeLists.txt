# Set a var for the path to LJM
set(LJM_LIBRARY "/usr/local/lib/libLabJackM.dylib")

# Target LJM utility sources
target_sources(${PROJECT_NAME}
    PRIVATE
    LJMUtil.cpp
    LJMStreamUtil.cpp
        # LJM_Utilities.cpp
        # LJM_StreamUtilities.cpp
)
# Link LJM
target_link_libraries(${PROJECT_NAME} ${LJM_LIBRARY})
# Include the current directory (lib) and /usr/local/lib when searching for headers
target_include_directories(${PROJECT_NAME}
    PUBLIC
        "/usr/local/include"
        ${CMAKE_CURRENT_LIST_DIR}
)