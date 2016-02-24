project(V1730Producer)
cmake_minimum_required(VERSION 2.8.12)

set(name "V1730Producer.exe")
add_definitions(-std=c++0x)
set(sourcefiles src/V1730Producer.cxx)

find_library(LIBLAB_LIB
  NAMES liblab.so
  HINTS "/home/testbeam/lib")

find_library(VMERCC_LIB
  NAMES libvme_rcc.so
  HINTS "/ATLAS/lib")

find_library(RCCERROR_LIB
  NAMES librcc_error.so
  HINTS "/ATLAS/lib")

find_library(CMEMRCC_LIB
  NAMES libcmem_rcc.so
  HINTS "/ATLAS/lib")

find_library(IORCC_LIB
  NAMES libio_rcc.so
  HINTS "/ATLAS/lib")

find_library(USB_LIB
  NAMES libusb.so
  HINTS "/usr/lib64")


include_directories(/ATLAS/atlas/include/ /home/testbeam/include/liblab/ include/)
add_executable(${name} ${sourcefiles})
target_link_libraries(${name} EUDAQ ${EUDAQ_THREADS_LIB} ${LIBLAB_LIB} ${USB_LIB} ${IORCC_LIB} ${VMERCC_LIB} ${CMEMRCC_LIB} ${RCCERROR_LIB} ${ext_libs})


install(TARGETS ${name}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
