add_executable(light_angel_smoke_test
  main.cpp
  test_parser.cpp
  # Add new test source files here, e.g.:
  # test_lexer.cpp
)

target_link_libraries(light_angel_smoke_test
  PRIVATE
    light_angel_core
)

light_angel_enable_warnings(light_angel_smoke_test)

add_test(
  NAME light_angel_smoke_test
  COMMAND $<TARGET_FILE:light_angel_smoke_test>
)
