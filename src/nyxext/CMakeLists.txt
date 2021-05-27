FIND_PACKAGE( NyxGPU )

IF( ${NyxGPU_FOUND} )

  SET( MARS_NYXEXT_SOURCES 
       Model.cpp
       Texture.cpp
       TextureArray.cpp
       Skeleton.cpp
       Font.cpp
     )
        
  SET( MARS_NYXEXT_HEADERS
       Model.h
       Texture.h
       TextureArray.h
       Skeleton.h
       Font.h
     )
  
  SET( MARS_NYXEXT_INCLUDE_DIRS
     )
  
  SET( MARS_NYXEXT_LIBRARIES
       nyx_library
       nyx_loaders
       nyx_vkg
      )
  
  ADD_LIBRARY               ( mars_nyxext SHARED  ${MARS_NYXEXT_SOURCES} ${MARS_NYXEXT_HEADERS} )
  TARGET_LINK_LIBRARIES     ( mars_nyxext PUBLIC  ${MARS_NYXEXT_LIBRARIES}                      )
  TARGET_INCLUDE_DIRECTORIES( mars_nyxext PRIVATE ${MARS_NYXEXT_INCLUDE_DIRS}                   )
  
  BUILD_TEST( TARGET mars_nyxext DEPENDS nyx_vkg nyx_library ) 
  
  INSTALL( FILES  ${MARS_NYXEXT_HEADERS} DESTINATION ${HEADER_INSTALL_DIR}/ COMPONENT devel )
  INSTALL( TARGETS mars_nyxext EXPORT Mars COMPONENT release  
               ARCHIVE  DESTINATION ${EXPORT_LIB_DIR}
               RUNTIME  DESTINATION ${EXPORT_LIB_DIR}
               LIBRARY  DESTINATION ${EXPORT_LIB_DIR}
               INCLUDES DESTINATION ${EXPORT_INCLUDE_DIR} )
ENDIF() 
