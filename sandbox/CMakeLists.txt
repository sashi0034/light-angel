add_executable(light_angel_sandbox
    main.cpp
)

target_link_libraries(light_angel_sandbox
    PRIVATE
        light_angel_core
)

light_angel_enable_warnings(light_angel_sandbox)
