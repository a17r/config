CMAKE_MINIMUM_REQUIRED (VERSION 2.6.2)

PROJECT (OpenICC)

SET (OPENICC_VERSION_MAJOR 0)
SET (OPENICC_VERSION_MINOR 1)
SET (OPENICC_VERSION_MICRO 0)

SET (PACKAGE_NAME "OpenICC")
SET (PACKAGE_DESCRIPTION "OpenICC library is a Color Management System (CMS) on operating system level. It allows to match predictably input device colors to output device colors across supporting applications. One goal is to make color management useful for all users in a automated fashion and regardless of any technical knowledge.")

SET( OI_CMMSUBPATH     "color/modules" )
SET( OI_METASUBPATH    "${PACKAGE_NAME}" )

#additional cmake modules for loading libraries
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)


# some basic and important variables
SET( DOMAIN openicc.org ) #needed by doxygen
SET( REVERSE_DOMAIN org.openicc )

SET( OPENICC_VERSION "${OPENICC_VERSION_MAJOR}.${OPENICC_VERSION_MINOR}.${OPENICC_VERSION_MICRO}" )
SET( OPENICC_VERSION_MAJORMINOR "${OPENICC_VERSION_MAJOR}.${OPENICC_VERSION_MINOR}" )

IF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(PLATFORM "OSX")
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "Linux")
ELSE( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "${CMAKE_SYSTEM_NAME}")
ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
MESSAGE( "-- Found PLATFORM: ${PLATFORM}" )

IF( CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" )
  MESSAGE( "-- Info build \"${CMAKE_BUILD_TYPE}\"" )
  SET( DEBUG 1 )
ENDIF()

FIND_PROGRAM(CMAKE_AR_TMP NAMES ar)

# detect top level projects
FIND_PACKAGE( Yajl )
IF( HAVE_YAJL )
  INCLUDE_DIRECTORIES( ${YAJL_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${YAJL_LIBRARY_DIRS} )
  INCLUDE( CheckIncludeFiles )
  CHECK_INCLUDE_FILES( yajl/yajl_version.h HAVE_YAJL_VERSION_H )
  # yajl2 should come with yajl_version.h, the included yajl is version 1.0.x
  IF( NOT HAVE_YAJL_VERSION_H )
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/yajl/ )
  ENDIF( NOT HAVE_YAJL_VERSION_H )
ELSE( HAVE_YAJL )
  IF(CMAKE_VERSION VERSION_GREATER 2.8.0)
    #include(ExternalProject)
    #ExternalProject_Add( yajl
    #SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yajl
    #CONFIGURE_COMMAND cmake ${CMAKE_CURRENT_SOURCE_DIR}/yajl
    #BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/yajl
    #BUILD_COMMAND ${MAKE}
    # skip installation
    #INSTALL_COMMAND echo )
    ADD_SUBDIRECTORY( yajl )
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/yajl )
    LINK_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/yajl )
    SET( YAJL_LIBRARIES yajl-static )
  ENDIF()
  
ENDIF( HAVE_YAJL )

#now add all directories
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( cmake )
#ADD_SUBDIRECTORY( extras )
#ADD_SUBDIRECTORY( settings )
#ADD_SUBDIRECTORY( doc )

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/OpenIccUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/OpenIccUninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET( uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/OpenIccUninstall.cmake")

ADD_CUSTOM_TARGET( dist
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ && git archive --prefix=libopenicc-${OPENICC_VERSION}/ --format=tar HEAD > ${CMAKE_CURRENT_BINARY_DIR}/libopenicc-${OPENICC_VERSION}.tar 
                COMMAND echo "git:       git clone git://guthub.com/openicc-cms/libopenicc.git" > ${CMAKE_CURRENT_BINARY_DIR}/libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND echo "git sha1:  " | tr -d "\\n" >> ${CMAKE_CURRENT_BINARY_DIR}/libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ && git rev-parse HEAD >> ${CMAKE_CURRENT_BINARY_DIR}/libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum 
                COMMAND rm -rf libopenicc-${OPENICC_VERSION} && tar xf libopenicc-${OPENICC_VERSION}.tar 
                COMMAND cp -ar ${CMAKE_SOURCE_DIR}/elektra-0.7.1/ libopenicc-${OPENICC_VERSION} && tar cf - libopenicc-${OPENICC_VERSION} | bzip2 > libopenicc-${OPENICC_VERSION}.tar.bz2
                COMMAND echo  "package:   http://sourceforge.net/projects/openicc/files/OpenICC/libopenicc%20${OPENICC_VERSION_MAJORMINOR}/libopenicc-${OPENICC_VERSION}.tar.bz2" >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND echo  "size:      " | tr -d "\\n" >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND du -b libopenicc-${OPENICC_VERSION}.tar.bz2 | sed s/[[:space:]][[:print:]]*// >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum 
                COMMAND echo  "md5sum:    " | tr -d "\\n" >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/ && md5sum libopenicc-${OPENICC_VERSION}.tar.bz2 >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND echo  "sha1sum:   " | tr -d "\\n" >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND sha1sum libopenicc-${OPENICC_VERSION}.tar.bz2 >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND echo  "sha256sum: " | tr -d "\\n" >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
                COMMAND sha256sum libopenicc-${OPENICC_VERSION}.tar.bz2 >> libopenicc-${OPENICC_VERSION}.tar.bz2.hashsum
		COMMENT Make a libopenicc source package by using git
		VERBATIM
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		)
EXECUTE_PROCESS(COMMAND rpmbuild --showrc
                COMMAND awk '/^build arch/ {print $$4}'
                OUTPUT_VARIABLE RPMARCH
                ERROR_VARIABLE RPMBUILD_ERROR
               )
ADD_CUSTOM_TARGET( rpm
		COMMAND test -d ${PACKAGE_NAME}-${OPENICC_VERSION}/ && rm -rf ${PACKAGE_NAME}-${OPENICC_VERSION}/
		COMMAND tar xjf ${PACKAGE_NAME}-${OPENICC_VERSION}.tar.bz2
                COMMAND mkdir -p rpmdir/BUILD rpmdir/SPECS rpmdir/SOURCES rpmdir/SRPMS rpmdir/RPMS/${RPMARCH}
                COMMAND cp -f ${PACKAGE_NAME}-${OPENICC_VERSION}.tar.bz2 rpmdir/SOURCES
                COMMAND rpmbuild --clean -ba extras/${PACKAGE_NAME}.spec --define "_topdir ${CMAKE_CURRENT_BINARY_DIR}/rpmdir"
                DEPENDS dist
		COMMENT Make a libopenicc RPM package
		VERBATIM
		)

ADD_CUSTOM_TARGET( debsrc
		COMMAND test -d ${PACKAGE_NAME}-${OPENICC_VERSION}/ && rm -rf ${PACKAGE_NAME}-${OPENICC_VERSION}/
		COMMAND tar xjf ${PACKAGE_NAME}-${OPENICC_VERSION}.tar.bz2
                COMMAND mv ${PACKAGE_NAME}-${OPENICC_VERSION}.tar.bz2 ${PACKAGE_NAME}_${OPENICC_VERSION}.orig.tar.bz2
                COMMAND cp -R ${CMAKE_SOURCE_DIR}/extras/debian ${PACKAGE_NAME}-${OPENICC_VERSION}/
                COMMAND dpkg-source -b ${PACKAGE_NAME}-${OPENICC_VERSION}/
                DEPENDS dist
		COMMENT Make a libopenicc# DEB source package
		VERBATIM
		)
ADD_CUSTOM_TARGET( debian
		COMMAND cd ${PACKAGE_NAME}-${OPENICC_VERSION}/ && debuild
                DEPENDS debsrc
		COMMENT Make a libopenicc DEB package
		VERBATIM
		)
