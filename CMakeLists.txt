cmake_minimum_required(VERSION 3.16)
project(GraphicEditor)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.87
)
FetchContent_MakeAvailable(ImGui)

FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v2.6
)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(SFML_DIR ${sfml_BINARY_DIR})
FetchContent_MakeAvailable(ImGui-SFML)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${sfml_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui-sfml_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE sfml-graphics sfml-window sfml-system
    PRIVATE ImGui-SFML::ImGui-SFML
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fonts)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Textures)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/Textures)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/Textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
