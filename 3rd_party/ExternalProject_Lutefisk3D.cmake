libname(lutefisk3d Lutefisk3D)
ExternalProject_Add(
   l3d_BUILD
   GIT_REPOSITORY https://github.com/Lutefisk3D/lutefisk3d
   GIT_SHALLOW 1
   UPDATE_COMMAND ""
   INSTALL_DIR ${ThirdParty_Install_Dir}
   CMAKE_ARGS -DQt5_DIR:PATH=${Qt5_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_STAGING_PREFIX:PATH=${ThirdParty_Install_Dir} -DCMAKE_INSTALL_PREFIX:PATH=${ThirdParty_Install_Dir}
   CMAKE_GENERATOR "${CMAKE_GENERATOR}"
   CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
   BUILD_BYPRODUCTS ${lutefisk3d_LIBRARY_SHARED} ${lutefisk3d_LIBRARY_IMP}
)
ADD_LIBRARY(lutefisk3d_IMP SHARED IMPORTED GLOBAL)
add_dependencies(lutefisk3d_IMP l3d_BUILD)
#set_shared_lib_properties(lutefisk3d)
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_LOCATION ${lutefisk3d_LIBRARY_SHARED} )
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_LOCATION_DEBUG ${lutefisk3d_LIBRARY_SHARED} )
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_LOCATION_RELEASE ${lutefisk3d_LIBRARY_SHARED} )
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_IMPLIB ${lutefisk3d_LIBRARY_IMP} )
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_IMPLIB_RELEASE ${lutefisk3d_LIBRARY_IMP} )
if(MSVC)
SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_IMPLIB_DEBUG ${ThirdParty_Install_Dir}/lib/Lutefisk3D_Debug.lib )
else()
    SET_PROPERTY(TARGET lutefisk3d_IMP APPEND PROPERTY IMPORTED_IMPLIB_DEBUG ${lutefisk3d_LIBRARY_IMP_D} )
endif()
#set_property(TARGET lutefisk3d_IMP APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS ACE_HAS_DLL=1)
