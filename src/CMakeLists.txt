file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

# Copy textures also copy them to source directory for VS
file(COPY custom/drawables/textures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)

IF(WIN32)
file(COPY custom/drawables/textures/ DESTINATION ${CMAKE_SOURCE_DIR}/textures)
ENDIF()

# Copy drawable shaders also copy them to source directory for VS
file(COPY custom/drawables/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

IF(WIN32)
file(COPY custom/drawables/shaders/ DESTINATION ${CMAKE_SOURCE_DIR}/shaders)
ENDIF()

#Copy compute shaders also copy them to source directory for VS
file(COPY custom/computables/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

IF(WIN32)
file(COPY custom/computables/shaders/ DESTINATION ${CMAKE_SOURCE_DIR}/shaders)
ENDIF()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} 
    glad
    glfw
    glm
    stb_image
    imgui
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")
