cmake_minimum_required(VERSION 3.20)

project(kubys
    VERSION 4.1.0
    DESCRIPTION "KUBYS - Kutuphane Yonetim Sistemi"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KUBYS_BUILD_QT "Qt6 grafiksel arayuzunu ve ag modullerini derle" ON)

add_executable(kubys_faz1_demo
    src/faz1_demo.cpp
)

target_include_directories(kubys_faz1_demo PRIVATE
    include
)

if(MSVC)
    target_compile_options(kubys_faz1_demo PRIVATE /W4 /permissive-)
else()
    target_compile_options(kubys_faz1_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(KUBYS_BUILD_QT)
    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Charts PrintSupport Network)

    qt_standard_project_setup()

    add_executable(kubys
        src/main.cpp

        src/main_window.h
        src/main_window.cpp

        src/arka_plan_isci.h
        src/arka_plan_isci.cpp

        src/kutuphane_sunucu.h
        src/kutuphane_sunucu.cpp

        src/ag_istemci.h
        src/ag_istemci.cpp

        src/kitap_model.h
        src/kitap_model.cpp

        src/uye_model.h
        src/uye_model.cpp

        src/odunc_model.h
        src/odunc_model.cpp

        src/kitap_dialog.h
        src/kitap_dialog.cpp

        src/uye_dialog.h
        src/uye_dialog.cpp

        src/odunc_dialog.h
        src/odunc_dialog.cpp
    )

    target_include_directories(kubys PRIVATE
        include
    )

    target_link_libraries(kubys PRIVATE
        Qt6::Widgets
        Qt6::Charts
        Qt6::PrintSupport
        Qt6::Network
    )

    if(MSVC)
        target_compile_options(kubys PRIVATE /W4 /permissive-)
    else()
        target_compile_options(kubys PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
