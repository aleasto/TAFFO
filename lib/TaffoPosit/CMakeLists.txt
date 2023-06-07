add_library(TaffoPosit SHARED
    TaffoPosit.cpp)

install(TARGETS TaffoPosit)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include)
