add_library(
  greetings
  SHARED
  hello.cpp
  good_morning.cpp
  )

target_include_directories(
  greetings
  PRIVATE ../include
  )

add_library(
  decodeLibs
  SHARED
  ReadTxtData.cpp
  )

target_include_directories(
  decodeLibs
  PRIVATE ../include
  )
