add_executable(CAEN_Analysis main.cpp)
target_link_libraries(CAEN_Analysis  greetings)

add_executable(Data_Decoade data_decode.cpp)
target_link_libraries(Data_Decoade  decodeLibs)
