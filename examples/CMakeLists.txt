
add_executable(test_restapi
    test_restapi.cpp)

target_include_directories(test_restapi
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${restapi_SOURCE_DIR}/include
)

target_link_libraries(test_restapi
PRIVATE
  restapi  
)
