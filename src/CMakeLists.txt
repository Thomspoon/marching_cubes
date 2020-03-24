file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.hpp)

# Copy textures
file(COPY custom/drawables/textures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)
file(COPY custom/drawables/textures/ DESTINATION ${CMAKE_SOURCE_DIR}/textures)

# Copy drawable shaders
file(COPY custom/drawables/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY custom/drawables/shaders/ DESTINATION ${CMAKE_SOURCE_DIR}/shaders)

#Copy compute shaders
file(COPY custom/computables/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY custom/computables/shaders/ DESTINATION ${CMAKE_SOURCE_DIR}/shaders)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} 
    glad
    glfw
    glm
    stb_image
    imgui
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")
