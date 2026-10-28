# CMake generated Testfile for 
# Source directory: /mnt/c/Users/gobli/Downloads/309Project3
# Build directory: /mnt/c/Users/gobli/Downloads/309Project3/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(p2_regression "/mnt/c/Users/gobli/Downloads/309Project3/build/test_p2")
set_tests_properties(p2_regression PROPERTIES  WORKING_DIRECTORY "/mnt/c/Users/gobli/Downloads/309Project3/build" _BACKTRACE_TRIPLES "/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;82;add_test;/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;0;")
add_test(p3 "/mnt/c/Users/gobli/Downloads/309Project3/build/test_p3")
set_tests_properties(p3 PROPERTIES  WORKING_DIRECTORY "/mnt/c/Users/gobli/Downloads/309Project3/build" _BACKTRACE_TRIPLES "/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;86;add_test;/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;0;")
add_test(tool_loop "/mnt/c/Users/gobli/Downloads/309Project3/build/test_loop")
set_tests_properties(tool_loop PROPERTIES  WORKING_DIRECTORY "/mnt/c/Users/gobli/Downloads/309Project3/build" _BACKTRACE_TRIPLES "/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;90;add_test;/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;0;")
add_test(e2e "bash" "/mnt/c/Users/gobli/Downloads/309Project3/tests/e2e/run_e2e.sh" "/mnt/c/Users/gobli/Downloads/309Project3/build/miniharness")
set_tests_properties(e2e PROPERTIES  WORKING_DIRECTORY "/mnt/c/Users/gobli/Downloads/309Project3" _BACKTRACE_TRIPLES "/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;99;add_test;/mnt/c/Users/gobli/Downloads/309Project3/CMakeLists.txt;0;")
