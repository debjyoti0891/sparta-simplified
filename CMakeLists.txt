cmake_minimum_required (VERSION 3.14)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project(SKELETON_3S_PIPELINE)

# include (../ExamplesMacros.cmake)

set(sparta_install_home /imec/other/memseat/bhatta53/ekanta/latest_map/map/sparta/build/install/)
add_compile_options( -g )
add_link_options( -g )

add_executable(sparta_3s_skeleton
  src/main.cpp
  src/SkeletonSimulator.cpp
  src/Producer.cpp
  src/Consumer.cpp)

target_compile_features(sparta_3s_skeleton PRIVATE cxx_std_17)
target_include_directories(sparta_3s_skeleton PUBLIC 
	${sparta_install_home}/include 
	)

target_link_directories(sparta_3s_skeleton PUBLIC 
	${sparta_install_home}/lib 
	)

target_link_libraries(sparta_3s_skeleton
  pthread
	sparta simdb  
	yaml-cpp sqlite3 boost_serialization boost_filesystem boost_timer hdf5 z boost_program_options )


## this is the new executable 
add_executable(sparta_3stage
	src/main_3s.cpp
	src/StageSim.cpp
	src/Producer.cpp
	src/Consumer.cpp
	src/Pipe.cpp)
  
  target_compile_features(sparta_3stage PRIVATE cxx_std_17)
  target_include_directories(sparta_3stage PUBLIC 
	  ${sparta_install_home}/include 
	  )
  
  target_link_directories(sparta_3stage PUBLIC 
	  ${sparta_install_home}/lib 
	  )
  
  target_link_libraries(sparta_3stage
	pthread
	  sparta simdb  
	  yaml-cpp sqlite3 boost_serialization boost_filesystem boost_timer hdf5 z boost_program_options )

# sparta_application(sparta_3s_skeleton)

# #
# # Testing
# #
# include (../../test/TestingMacros.cmake)
# sparta_copy(sparta_3s_skeleton "test_final_cfg.sh")
# sparta_copy(sparta_3s_skeleton "test_arch_with_override.sh")

# # build this guy as part of the regression
# sparta_regress(sparta_3s_skeleton)
# sparta_named_test(sparta_3s_skeleton_200000 sparta_3s_skeleton -r 200000)
# sparta_named_test(sparta_3s_skeleton_producers sparta_3s_skeleton -p top.consumer.params.num_producers 10 --show-tree)
# sparta_named_test(sparta_3s_skeleton_v sparta_3s_skeleton -v)
# sparta_named_test(sparta_3s_skeleton_files sparta_3s_skeleton file1 file2)
# sparta_named_test(sparta_3s_skeleton_final_cfg sparta_3s_skeleton -p top.producer0.params.test_param 1000 --write-final-config final)
# # Notice the below tests don't use sparta macros because we are just invoking a shell
# # script that does a few calls to sparta_3s_skeleton and some other unix commands
# # to verify correctness. It'll still get picked up by make regress.
# add_test(NAME sparta_3s_skeleton_arch_with_override COMMAND ./test_arch_with_override.sh)
# add_test(NAME sparta_3s_skeleton_test_final_cfg COMMAND ./test_final_cfg.sh)
