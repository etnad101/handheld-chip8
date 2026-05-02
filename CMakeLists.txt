cmake_minimum_required(VERSION 3.16)

if (DEFINED ENV{IDF_PATH})
	include($ENV{IDF_PATH}/tools/cmake/project.cmake)
	project(handheld_chip8)
else()
	project(handheld_chip8 C)

	find_package(SDL2 CONFIG REQUIRED)

	set(CMAKE_C_STANDARD 11)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


	add_executable(handheld_chip8
				main/main_sdl.c
				core/chip8.c
				app/app.c
	)

	target_compile_options(handheld_chip8 PRIVATE
	-Wall -Wextra -Wpedantic -Werror)


	if(WIN32)
		target_sources(handheld_chip8 PRIVATE
		platform/platform_windows.c
	)
	elseif(APPLE)
		target_sources(handheld_chip8 PRIVATE
		platform/platform_macos.c
	)
	endif()

	if(WIN32)
		find_package(SDL2 REQUIRED)
		target_link_libraries(handheld_chip8 PRIVATE SDL2::SDL2)
	endif()

endif()

