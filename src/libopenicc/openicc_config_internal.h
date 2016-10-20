/*  @file openicc_config_internal.h
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/10/21
 */

#ifndef __OPENICC_CONFIG_INTERNAL_H__
#define __OPENICC_CONFIG_INTERNAL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>           /* va_list */
#include <time.h>

#include "oyjl_tree.h"

#include "openicc_conf.h"
#include "openicc_config.h"
#include "openicc_version.h"

#ifdef USE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(text) dgettext( "OpenICC", text )
#else
# define _(text) text
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__unix__) || (!defined(_WIN32) && !defined(_MSC_VER)) || (defined(_WIN32) && defined(__CYGWIN__)) || defined(__MINGW32__) || defined(__MINGW32)
# include <unistd.h>
# if defined(_POSIX_VERSION)
#  define HAVE_POSIX 1
# endif
#endif

#if defined(__GNUC__)
# define  OI_DBG_FORMAT_ "%s:%d %s() "
# define  OI_DBG_ARGS_   strrchr(__FILE__,'/') ? strrchr(__FILE__,'/')+1 : __FILE__,__LINE__,__func__
#else
# define  OI_DBG_FORMAT_ "%s:%d "
# define  OI_DBG_ARGS_   strrchr(__FILE__,'/') ? strrchr(__FILE__,'/')+1 : __FILE__,__LINE__
#endif

#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC
#define DBGc_S( format_,...) openiccMessage_p( openiccMSG_DBG, NULL, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)
#define DBGcc_S( obj, format_,...) openiccMessage_p( openiccMSG_DBG, obj, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)
#define WARNc_S( format_,...) openiccMessage_p( openiccMSG_WARN, NULL, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)
#define WARNcc_S( obj, format_,...) openiccMessage_p( openiccMSG_WARN, obj, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)
#define ERRc_S( format_,...) openiccMessage_p( openiccMSG_ERROR, NULL, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)
#define ERRcc_S( obj, format_,...) openiccMessage_p( openiccMSG_ERROR, obj, \
                                                OI_DBG_FORMAT_ format_, OI_DBG_ARGS_, __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
  openiccOBJECT_NONE,
  openiccOBJECT_CONFIG = 312,
  openiccOBJECT_DB
} openiccOBJECT_e;

struct openiccConfig_s {
  openiccOBJECT_e type;
  char     * json_text;
  oyjl_val   oyjl;
  char     * dbg_text;
};

struct openiccDB_s {
  openiccOBJECT_e type;
  openiccSCOPE_e   scope;
  char * top_key_name;
  openiccConfig_s ** ks;
  int ks_array_reserved_n;
};

int          openiccStringAddPrintf  ( char             ** string,
                                       const char        * format,
                                                           ... );
void         openiccStringAdd_       ( char             ** text,
                                       const char        * append );
#define STRING_ADD( t, append ) openiccStringAdd_( &t, append )
char *       openiccStringCopy       ( const char        * text,
                                       openiccAlloc_f      alloc );
#define openiccNoEmptyString_m_(t) (t?t:"")


extern openiccMessage_f     openiccMessage_p;
int  openiccMessageFunc              ( openiccMSG_e        code,
                                       void              * context_object,
                                       const char        * format,
                                       ... );


char * openiccOpenFile( const char * file_name,
                        size_t     * size_ptr );
char * openiccReadFileSToMem(
                        FILE       * fp,
                        size_t     * size);
size_t openiccWriteFile(const char * file_name,
                        void       * ptr,
                        size_t       size );
char * openiccExtractPathFromFileName_(const char        * file_name );
int    openiccIsDirFull_             ( const char        * name );
char * openiccPathGetParent_         ( const char        * name );


/* system variables - exposed by the openiccGetInstallPath() API */
#define OPENICC_COLOR_DL_CACHE_PATH "openicc/device_link"
#define OPENICC_ICCDIRNAME "icc"
#define OPENICC_SETTINGSDIRNAME "settings"

#if defined(__APPLE__)
  /* Apples ColorSync default paths */

# define OS_USER_DIR    "~/Library"
# define OS_GLOBAL_DIR  "/Library"
# define OS_MACHINE_DIR "/System/Library"

# define OS_ICC_PATH         "/ColorSync/Profiles"
# define OS_ICC_USER_DIR               OS_USER_DIR         OS_ICC_PATH
# define OS_ICC_SYSTEM_DIR             OS_GLOBAL_DIR       OS_ICC_PATH
# define OS_ICC_MACHINE_DIR            OS_MACHINE_DIR      OS_ICC_PATH
# define CSNetworkPath                 "/Network/Library"  OS_ICC_PATH

# define OS_CACHE_PATH       "/Caches/org.freedesktop." OPENICC_COLOR_DL_CACHE_PATH
# define OS_SETTING_PATH     "/Preferences/org.freedesktop.openicc/" OPENICC_SETTINGSDIRNAME
# define OS_SETTINGS_USER_DIR          OS_USER_DIR           OS_SETTING_PATH
# define OS_SETTINGS_SYSTEM_DIR        OS_GLOBAL_DIR         OS_SETTING_PATH
# define OS_SETTINGS_MACHINE_DIR       OS_MACHINE_DIR        OS_SETTING_PATH

#else

# define OS_USER_DIR    "~/."
# define OS_GLOBAL_DIR  "/usr/share/"
# define OS_MACHINE_DIR "/var/"

# define OS_ICC_PATH         "color/" OPENICC_ICCDIRNAME
# define OS_ICC_USER_DIR               OS_USER_DIR "local/share/" OS_ICC_PATH
# define OS_ICC_SYSTEM_DIR             OS_GLOBAL_DIR         OS_ICC_PATH
# define OS_ICC_MACHINE_DIR            OS_MACHINE_DIR "lib/" OS_ICC_PATH

# define OS_CACHE_PATH       "cache/color/" OPENICC_COLOR_DL_CACHE_PATH
# define OS_SETTING_PATH     "color/" OPENICC_SETTINGSDIRNAME
# define OS_SETTINGS_USER_DIR          OS_USER_DIR "config/" OS_SETTING_PATH
# define OS_SETTINGS_SYSTEM_DIR        OS_GLOBAL_DIR         OS_SETTING_PATH
# define OS_SETTINGS_MACHINE_DIR       OS_MACHINE_DIR "lib/" OS_SETTING_PATH
#endif

# define OS_DL_CACHE_USER_DIR          OS_USER_DIR           OS_CACHE_PATH
# define OS_DL_CACHE_SYSTEM_DIR        OS_MACHINE_DIR        OS_CACHE_PATH
# define OS_DL_CACHE_MACHINE_DIR       OS_MACHINE_DIR        OS_CACHE_PATH


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __OPENICC_CONFIG_H__ */
