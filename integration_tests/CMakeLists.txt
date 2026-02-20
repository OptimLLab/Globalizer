project(Globalizer_IntegrationTests)

add_definitions(-DGLOBALIZER_MAX_DIMENSION=10)
add_definitions(-DGLOBALIZER_MAX_Number_Of_Function=20)

file(GLOB BENCHMARK_INTERFACE_SOURCES 
    "${CMAKE_SOURCE_DIR}/third_party/Problems/Interface/src/*.cpp"
)

add_executable(Globalizer_IntegrationTests 
	src/main.cpp
	src/json.hpp
	${BENCHMARK_INTERFACE_SOURCES}
)

# Оставляем только Interface (Globalizer'а)
target_include_directories(Globalizer_IntegrationTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/Problems/Interface/include
)

target_link_libraries(Globalizer_IntegrationTests PRIVATE Globalizer_l)

# Копируем tests.json
add_custom_command(TARGET Globalizer_IntegrationTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/references/tests.json"
    "$<TARGET_FILE_DIR:Globalizer_IntegrationTests>/tests.json"
)