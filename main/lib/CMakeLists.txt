option(USE_ROOT "Compiling main library using ROOT" OFF)
if (USE_ROOT)
   FIND_PACKAGE( ROOT REQUIRED )
else (USE_ROOT)
   FIND_PACKAGE( ROOT REQUIRED )
endif (USE_ROOT)

if (ROOT_FOUND)
  INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIR} )
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES}  ${ROOT_LIBRARIES} -lSpectrum)
  ADD_DEFINITIONS(-DROOT_FOUND)
endif (ROOT_FOUND)

AUX_SOURCE_DIRECTORY( src library_sources )
AUX_SOURCE_DIRECTORY( plugins plugins_sources )

list(REMOVE_ITEM plugins_sources plugins/GenericConverterPlugin.cc)

FIND_PACKAGE(PXAR)
IF(PXARCORE_FOUND)
  INCLUDE_DIRECTORIES(${PXARCORE_INCLUDE_DIRS})
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES}  ${PXARCORE_LIBRARIES})
ELSE(PXARCORE_FOUND)
  MESSAGE(STATUS "NOT building converter plugin for CMSPixel.")
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelConverterPlugin.cc)
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelTRPConverterPlugin.cc)
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelREFConverterPlugin.cc)
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelDUTConverterPlugin.cc)
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelANAConverterPlugin.cc)
  list(REMOVE_ITEM plugins_sources plugins/CMSPixelDIGConverterPlugin.cc)
ENDIF(PXARCORE_FOUND)

if (PROTOBUF_FOUND AND GEN_proto)
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/pb_gen )
    add_custom_target(
            GEN_PROTO ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${PROJECT_SOURCE_DIR}/main/include/eudaq --cpp_out=${CMAKE_BINARY_DIR}/pb_gen --python_out=${PROJECT_SOURCE_DIR}/python ${PROJECT_SOURCE_DIR}/main/include/eudaq/StandardEvent.proto
            #COMMAND mv ${PROJECT_SOURCE_DIR}/main/lib/plugins/StandardEvent.pb.h ${PROJECT_SOURCE_DIR}/main/include/eudaq
            #COMMAND sed -i 's/StandardEvent.pb.h/eudaq\\/StandardEvent.pb.h/g' ${PROJECT_SOURCE_DIR}/main/lib/plugins/StandardEvent.pb.cc
        )
    list(APPEND plugins_sources plugins/GenericConverterPlugin.cc)    
    
    list(APPEND plugins_sources ${CMAKE_BINARY_DIR}/pb_gen/StandardEvent.pb.cc)    
    set_source_files_properties(${CMAKE_BINARY_DIR}/pb_gen/StandardEvent.pb.cc PROPERTIES GENERATED TRUE)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/pb_gen)

endif()

ADD_LIBRARY( ${PROJECT_NAME} SHARED ${library_sources} ${plugins_sources})

if (PROTOBUF_FOUND AND GEN_proto)
    add_dependencies(${PROJECT_NAME} GEN_PROTO)   
endif ()

SOURCE_GROUP(plugins .*Plugin.*cc)
SOURCE_GROUP(FileIO .*File.*cc)
SOURCE_GROUP(TCPIP .*Transport.*cc)
SOURCE_GROUP(TCPIP FILES src\\Serializable.cc src\\Producer.cc src\\CommandReceiver.cc src\\DataCollector.cc src\\DataSender.cc  src\\LogCollector.cc)
SOURCE_GROUP(Event .*Event.*cc)
SOURCE_GROUP(Utilities FILES src\\Utils.cc src\\Time.cc src\\Mutex.cc src\\EudaqThread.cc)

target_link_libraries( ${PROJECT_NAME} ${EUDAQ_THREADS_LIB} ${ADDITIONAL_LIBRARIES})

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
