mkdir -p out
cd out

# Default to VS2022 generator; allow override via CMAKE_GENERATOR.
: "${CMAKE_GENERATOR:=Visual Studio 17 2022}"
cmake -G"${CMAKE_GENERATOR}" -A x64 ${COMMON_CMAKE_CONFIG_PARAMS} ../
cmake --build . --config Debug