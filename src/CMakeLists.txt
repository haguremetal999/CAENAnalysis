add_library(
  greetings
  SHARED
  hello.cpp
  good_morning.cpp
  )

target_include_directories(
  greetings
  PUBLIC home/kotarotakahashi/share_windows/hsuzuki/src/CAEN_inTsukuba_20230821/include
  )

add_library(
  decodeLibs
  SHARED
  ReadTxtData.cpp
  )

target_include_directories(
  decodeLibs
  PUBLIC home/kotarotakahashi/share_windows/hsuzuki/src/CAEN_inTsukuba_20230821/include
  )
