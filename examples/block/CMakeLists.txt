add_custom_command(OUTPUT copyPathFile POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${EEROS_SOURCE_DIR}/examples/block/path.txt" "${EEROS_BINARY_DIR}/examples/block")

add_executable(blockTest BlockTest.cpp)
target_link_libraries(blockTest eeros ${EEROS_LIBS})

add_executable(mouseTest MouseTest.cpp)
target_link_libraries(mouseTest eeros ${EEROS_LIBS})

add_executable(xBoxTest XBoxTest.cpp)
target_link_libraries(xBoxTest eeros ${EEROS_LIBS})

add_executable(keyboardTest KeyboardTest.cpp)
target_link_libraries(keyboardTest eeros ${EEROS_LIBS})

add_executable(traceTest TraceTest.cpp)
target_link_libraries(traceTest eeros ${EEROS_LIBS})

add_executable(spaceNavigatorTest SpaceNavigatorTest.cpp)
target_link_libraries(spaceNavigatorTest eeros ${EEROS_LIBS})

add_executable(transitionBlockTest1 TransitionBlockTest.cpp)
target_link_libraries(transitionBlockTest1 eeros ${EEROS_LIBS})

add_executable(pathPlannerTest1 PathPlannerTest1.cpp)
target_link_libraries(pathPlannerTest1 eeros ${EEROS_LIBS})

add_executable(pathPlannerTest2 PathPlannerTest2.cpp)
target_link_libraries(pathPlannerTest2 eeros ${EEROS_LIBS})

add_executable(pathPlannerTest3 PathPlannerTest3.cpp copyPathFile)
target_link_libraries(pathPlannerTest3 eeros ${EEROS_LIBS})
