set(SELF TaffoPosit)

add_library(${SELF} SHARED
    TaffoPosit.cpp)

install(TARGETS TaffoPosit)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include)

target_include_directories(${SELF} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET ${SELF} PROPERTY POSITION_INDEPENDENT_CODE ON)
