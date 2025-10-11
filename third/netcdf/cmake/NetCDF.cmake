set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/third)

# --- netcdf ---
set(NC_INCLUDE_DIR ${THIRDPARTY_DIR}/netcdf)

if(EXISTS "${NC_INCLUDE_DIR}")
	message(STATUS "Found local library: netCDF. Configuring manually...")

    add_library(netcdf INTERFACE IMPORTED)
    target_include_directories(netcdf INTERFACE "${NC_INCLUDE_DIR}/include")

    set(NC_LIB_DEBUG_DIR ${NC_INCLUDE_DIR}/debug/lib)
    set(NC_LIB_RELEASE_DIR ${NC_INCLUDE_DIR}/lib)

    target_link_libraries(netcdf INTERFACE
        $<$<CONFIG:Debug>:${NC_LIB_DEBUG_DIR}/netcdf.lib>
        $<$<CONFIG:Release>:${NC_LIB_RELEASE_DIR}/netcdf.lib>
    )
	
    set(DEP_NETCDF_TARGETS netcdf)
	set(DEP_NETCDF_INCLUDE_PATHS "${NC_INCLUDE_DIR}/include")
endif()