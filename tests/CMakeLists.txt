add_executable(DISPatch_tests
        AppConfigTests.cpp
        DisProtocolTests.cpp)
target_link_libraries(DISPatch_tests PRIVATE DISPatch_core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(DISPatch_tests)
