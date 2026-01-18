cmake_minimum_required(VERSION 3.25.0)
project(book_management VERSION 0.1.0 LANGUAGES C)

add_executable(book_management
    main.c
    user.c
    data.c
    logic.c
    store.c
)

include(CTest)
enable_testing()
