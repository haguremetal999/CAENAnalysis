add_executable(CAEN_Analysis main.cpp)
target_link_libraries(CAEN_Analysis  greetings)
target_include_directories(CAEN_Analysis PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(CAEN_Analysis ${ROOT_LIBRARIES})

add_executable(Data_Decoade data_decode.cpp)
target_link_libraries(Data_Decoade  decodeLibs)
target_include_directories(Data_Decoade PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(Data_Decoade ${ROOT_LIBRARIES})

add_executable(Raw_Analysis raw_analysis.cpp)
target_link_libraries(Raw_Analysis  decodeLibs)
target_include_directories(Raw_Analysis PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(Raw_Analysis ${ROOT_LIBRARIES})

add_executable(Show_Histgrams show_histgrams.cpp)
target_link_libraries(Show_Histgrams  decodeLibs)
target_include_directories(Show_Histgrams PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(Show_Histgrams ${ROOT_LIBRARIES})
