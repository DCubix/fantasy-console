cmake_minimum_required(VERSION 3.10)
project(console LANGUAGES C CXX VERSION 0.1)

if (WIN32)
	add_definitions(-DSDL_MAIN_HANDLED)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(SDL2 CONFIG REQUIRED)

file(GLOB_RECURSE SRC "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(
	${PROJECT_NAME}
	SDL2
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()

if (UNIX AND NOT APPLE)
	target_link_libraries(${PROJECT_NAME}
		m
		pthread
	)
endif()