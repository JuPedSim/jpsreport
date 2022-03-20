# create win-exe, dmg, and deb
#-------------------------------
# Structure:
# ├── LICENSE
# ├── README.md
# ├── jpsreport
# └── jpsreport_samples
#-------------------------------

macro (cpack_write_deb_config)
  message(STATUS "Package generation - LINUX")
  list(APPEND CPACK_GENERATOR "DEB")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR})
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>=1.65), zlib1g-dev")
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org")
  set(CPACK_DEBIAN_PACKAGE_SECTION "science")
  set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "David Haensel")
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "JuPedSim: framework for simulation and analysis of pedestrian dynamics" PARENT_SCOPE)
endmacro()

macro (cpack_write_osx_config)
  message(STATUS "Package generation - MacOS")
  list(APPEND CPACK_GENERATOR "DragNDrop")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/forms/jupedsim.png")
  # set(CPACK_DMG_DS_STORE_SETUP_SCRIPT
  #   "${CMAKE_SOURCE_DIR}/jpscore/forms/DS_Store.scpt" PARENT_SCOPE)
  set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/forms/JPSreport.icns")
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
  set(CPACK_SYSTEM_NAME "OSX")
endmacro()

macro (cpack_write_windows_config)
  message(STATUS "Package generation - Windows")
  set(CPACK_GENERATOR "NSIS" CACHE STRING "Generator used by CPack")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/forms/JPSreport.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/forms/JPSreport.ico")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH)
  set(CPACK_NSIS_HELP_LINK "https://www.jupedsim.org/jpsreport_introduction.html")
  set(CPACK_NSIS_URL_INFO_ABOUT "http://www.jupedsim.org/")
  set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PROJECT_NAME})
  set(CPACK_NSIS_MODIFY_PATH TRUE)
endmacro()

macro (cpack_write_config)
  message(STATUS "Cpack write configs")
  include(GNUInstallDirs)
  include(InstallRequiredSystemLibraries)

  install(TARGETS jpsreport
          DESTINATION bin
  )

  install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
      LIBRARIES $<TARGET_FILE:report>
      EXECUTABLES $<TARGET_FILE:jpsreport>
      RESOLVED_DEPENDENCIES_VAR _r_deps
      UNRESOLVED_DEPENDENCIES_VAR _u_deps
      DIRECTORIES ${MY_DEPENDENCY_PATHS}
    )
    foreach(_file ${_r_deps})
      file(INSTALL
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        TYPE SHARED_LIBRARY
        FOLLOW_SYMLINK_CHAIN
        FILES "${_file}"
      )
    endforeach()
    list(LENGTH _u_deps _u_length)
    if("${_u_length}" GREATER 0)
      message(WARNING "Unresolved dependencies detected!")
    endif()
  ]])

  set(CPACK_PACKAGE_NAME "JPSreport")
  set(CPACK_PACKAGE_FILE_NAME "jpsreport-installer-${PROJECT_VERSION}")
  set(CPACK_PACKAGE_VENDOR "Forschungszentrum Jülich GmbH")
  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERISON_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERISON_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERISON_PATCH})

  set (CPACK_PACKAGE_INSTALL_DIRECTORY "JPSreport-${PROJECT_VERSION}")
  set(CPACK_PACKAGE_DESCRIPTION "jpsreport is a tool to analyze pedestrian trajectories.")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.jupedsim.org/jpsreport_introduction.html")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JuPedSim: framework for simulation and analysis of pedestrian dynamics")
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

  install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/README.md"
          DESTINATION .)

  # Copy sample files from demos folder
  set(CT_DATA_FILE_DIR "demos")
  file(GLOB CT_FILES "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*/*.xml" "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*/*.txt")
  install(FILES ${CT_FILES}
          DESTINATION "jpsreport_samples"
  )
endmacro()
