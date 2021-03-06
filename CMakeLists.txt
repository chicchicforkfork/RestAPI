project(restapi)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER /usr/bin/c++)
set(CMAKE_CXX_FLAGS "-m64 -std=c++17")
add_compile_options(-W -Wall -g -fpermissive)

include(FetchContent)

FetchContent_Declare(nlohmann
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(nlohmann)

#find_package(cpprestsdk REQUIRED)

include_directories(include)

###############################
# for cpp (.so)
###############################
add_library(restapi SHARED 
    src/endpoint.cpp
    src/restapi.cpp
)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
target_include_directories (restapi
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${nlohmann_SOURCE_DIR}/include
)


target_link_libraries(restapi
PUBLIC
    cpprest ssl crypto)


#add_subdirectory(examples)
