find_package(MySQL REQUIRED)
find_package(BOINC REQUIRED)

#find_library(gmp REQUIRED)

find_path(BOINC_INCLUDE_DIR config.h PATHS /boinc/src/boinc/ /boinc/)


link_directories(/home/tdesell/boost_1_53_0/stage/lib)

include_directories(
    ${PROJECT_SOURCE_DIR}
	${BOINC_INCLUDE_DIR}
    ${BOINC_INCLUDE_DIR}/api
    ${BOINC_INCLUDE_DIR}/db
    ${BOINC_INCLUDE_DIR}/lib
    ${BOINC_INCLUDE_DIR}/sched
	${MYSQL_INCLUDE_DIR}
)

add_executable(sss_validator
    ${BOINC_INCLUDE_DIR}/sched/validator
    ${BOINC_INCLUDE_DIR}/sched/validate_util
    ${BOINC_INCLUDE_DIR}/sched/validate_util2
    sss_validation_policy)
target_link_libraries(sss_validator ${UNDVC_COMMON_LIBRARY} ${BOINC_SERVER_LIBRARIES}
    ${MYSQL_LIBRARIES} sss_common boost_system boost_filesystem)

add_executable(sss_assimilator
    ${BOINC_INCLUDE_DIR}/sched/assimilator
    ${BOINC_INCLUDE_DIR}/sched/validate_util
    sss_assimilation_policy)
target_link_libraries(sss_assimilator ${UNDVC_COMMON_LIBRARY} ${BOINC_SERVER_LIBRARIES}
    ${MYSQL_LIBRARIES} ${Boost_LIBRARIES} sss_common boost_system
    boost_filesystem)

#find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${BOINC_INCLUDE_DIR}/tools/
)

add_executable(sss_work_generator sss_work_generator)
target_link_libraries(sss_work_generator sss_common ${UNDVC_COMMON_LIBRARY} 
    ${BOINC_SERVER_LIBRARIES} ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES} sss_common
    boost_system boost_filesystem)

add_definitions( -DHTML_OUTPUT -DENABLE_COLOR)
add_executable(sss_webpage_generator sss_webpage_generator)
target_link_libraries(sss_webpage_generator ${UNDVC_COMMON_LIBRARY} ${BOINC_SERVER_LIBRARIES}
    ${MYSQL_LIBRARIES} sss_common boost_system boost_filesystem)

