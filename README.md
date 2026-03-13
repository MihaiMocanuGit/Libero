# Libero

Libero is a c++23 game development library designed to give users the necessary tools to begin
writing games. 

It's divided into different modules, the most important one being the fully compile time Entity
Component System module: ECS. Note that no RTTI is necessary for this system.

The other modules deal with SDL Utilities, vector math, smart resources
etc. Evidently, more modules are to come.

Documentation and unit tests for the bundled modules can be found in their respective module
folders, when present.

The library and tests can be built using the bash script `./build.sh`. Run `./build.sh -h` for more
details. Following the build, the test executable can be found in `build_<BUILD_TYPE>/unit_tests`.
As Catch2 is used for the testing framework, the usual options apply.
