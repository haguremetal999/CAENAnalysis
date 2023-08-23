add_executable(CAEN_Analysis main.cpp)
target_link_libraries(CAEN_Analysis  greetings)
target_include_directories(CAEN_Analysis PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(CAEN_Analysis ${ROOT_LIBRARIES})

add_executable(Data_Decoade data_decode.cpp)
target_link_libraries(Data_Decoade  decodeLibs)
target_include_directories(Data_Decoade PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(Data_Decoade ${ROOT_LIBRARIES})
