include_directories(${BOINC_INCLUDE_DIR})
include_directories(${BOINC_INCLUDE_DIR}/api)
include_directories(${BOINC_INCLUDE_DIR}/lib)

find_package(CUDA)

include(FindCUDA)

link_libraries("-L/${CUDA_SDK_ROOT_DIR}/C/lib -lcutil")

add_executable(SubsetSum_${SUBSET_SUM_VERSION_MAJOR}.${SUBSET_SUM_VERSION_MINOR} subset_sum_main)
target_link_libraries(SubsetSum_${SUBSET_SUM_VERSION_MAJOR}.${SUBSET_SUM_VERSION_MINOR} ${BOINC_APP_LIBRARIES})

if (CUDA_FOUND)
    cuda_add_executable(SubsetSum_${SUBSET_SUM_VERSION_MAJOR}.${SUBSET_SUM_VERSION_MINOR}_cuda subset_sum_main)
target_link_libraries(SubsetSum_${SUBSET_SUM_VERSION_MAJOR}.${SUBSET_SUM_VERSION_MINOR}_cuda ${BOINC_APP_LIBRARIES})
endif (CUDA_FOUND)
