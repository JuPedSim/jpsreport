# create win-exe, dmg, and deb
#-------------------------------
# Structure:
# ├── LICENSE
# ├── README.md
# ├── JuPedSim.pdf
# ├── bin
# │   ├── jpscore
# │   └── jpsreport
# ├── jpscore_samples
# └── jpsreport_samples
#-------------------------------
function (cpack_write_deb_config)
  message(STATUS "Package generation - LINUX")
  list(APPEND CPACK_GENERATOR "DEB")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>=1.65), zlib1g-dev" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS, "jpsvis, jpseditor" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SECTION "science" PARENT_SCOPE)
  set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} PARENT_SCOPE)
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "David Haensel" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "JuPedSim: framework for simulation and analysis of pedestrian dynamics" PARENT_SCOPE)
endfunction()

function (cpack_write_osx_config)
  message(STATUS "Package generation - MacOS")
  list(APPEND CPACK_GENERATOR "DragNDrop")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/jpscore/forms/background.png" PARENT_SCOPE)
  # set(CPACK_DMG_DS_STORE_SETUP_SCRIPT
  #   "${CMAKE_SOURCE_DIR}/jpscore/forms/DS_Store.scpt" PARENT_SCOPE)
  set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON  PARENT_SCOPE)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.icns" PARENT_SCOPE)
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}" PARENT_SCOPE)
  set(CPACK_SYSTEM_NAME "OSX" PARENT_SCOPE)
endfunction()

function (cpack_write_windows_config)
  message(STATUS "Package generation - Windows")
  set(CPACK_GENERATOR "NSIS" CACHE STRING "Generator used by CPack")

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(VCPKG_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/")
  set(DIRS ${VCPKG_DIR})
  set(DLL_FILES "${VCPKG_DIR}/fmt.dll" "${VCPKG_DIR}/spdlog.dll")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${DLL_FILES}")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BUNDLE_RUNTIME_DESTINATION}")
  include(InstallRequiredSystemLibraries)

  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin)

  # some configs for installer
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/JPSreport.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/forms/JPSreport.ico")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH)
  set(CPACK_NSIS_HELP_LINK "http://www.jupedsim.org/jupedsim_install_on_windows.html")
  set(CPACK_NSIS_URL_INFO_ABOUT "http://www.jupedsim.org/")
  set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PROJECT_NAME})
  # ----------------------------


endfunction()

function (cpack_write_config)
  message(STATUS "Cpack write configs")

  install(TARGETS jpsreport
          DESTINATION bin
          )
  install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/README.md"
          DESTINATION .)

  set(CPACK_PACKAGE_NAME "petrack-installer-${PROJECT_VERSION}")
  set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERISON_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERISON_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERISON_PATCH})

  set(CPACK_PACKAGE_DESCRIPTION "jpsreport is a tool to analyze pedestrian trajectories.")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_PACKAGE_HOMEPAGE_URL "http://www.jupedsim.org/jupedsim_install_on_windows.html")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JuPedSim: framework for simulation and analysis of pedestrian dynamics" PARENT_SCOPE)

  set(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

#    COMPONENT Documents)

#  set(jpsguide "${CMAKE_SOURCE_DIR}/docs/jps_guide/JuPedSim.pdf")
#  if(EXISTS "${jpsguide}")
#    install(FILES "${jpsguide}"
#      DESTINATION .
#      COMPONENT Documents)
#  endif()
#
#  print_var(CPACK_SOURCE_PACKAGE_FILE_NAME)

endfunction()
