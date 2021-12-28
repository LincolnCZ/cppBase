include_directories(${PROJECT_SOURCE_DIR}/muduo)
include_directories(${PROJECT_SOURCE_DIR})

#--------------------------服务器端-----------------------------------
# reactor 并发模式
add_executable(sudoku_solver_basic 05_server_basic.cc sudoku.cc)
target_link_libraries(sudoku_solver_basic muduo_net)

# reactor + thread pool 并发模式
# 主线程 IO，工作线程计算
add_executable(sudoku_solver_threadpool 08_server_threadpool.cc sudoku.cc)
target_link_libraries(sudoku_solver_threadpool muduo_net)

# reactors in threads 并发模式
# one loop per thread
add_executable(sudoku_solver_multiloop 09_server_multiloop.cc sudoku.cc)
target_link_libraries(sudoku_solver_multiloop muduo_net)

# reactors + thread pool 并发模式
add_executable(sudoku_solver_reactors_threadpool 11_server_threadpool.cc sudoku.cc)
target_link_libraries(sudoku_solver_reactors_threadpool muduo_net)

add_executable(sudoku_solver_hybrid server_hybrid.cc sudoku.cc)
target_link_libraries(sudoku_solver_hybrid muduo_inspect)

add_executable(sudoku_solver_prod server_prod.cc sudoku.cc)
target_link_libraries(sudoku_solver_prod muduo_inspect)

#--------------------------客户端-----------------------------------
add_executable(sudoku_client_batch batch.cc sudoku.cc)
target_link_libraries(sudoku_client_batch muduo_net)

add_executable(sudoku_client_pipeline pipeline.cc sudoku.cc)
target_link_libraries(sudoku_client_pipeline muduo_net)

add_executable(sudoku_loadtest loadtest.cc sudoku.cc)
target_link_libraries(sudoku_loadtest muduo_net)


if(BOOSTTEST_LIBRARY)
add_executable(sudoku_stat_unittest stat_unittest.cc)
target_link_libraries(sudoku_stat_unittest muduo_base boost_unit_test_framework)
endif()

