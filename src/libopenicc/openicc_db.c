/*  @file openicc_db.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2015-2016 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2015/08/29
 */

#include "openicc_config_internal.h"
#include "openicc_db.h"
#include "xdg_bds.h"

#if HAVE_POSIX
#include <unistd.h>  /* getpid() */
#endif
#include <string.h>  /* strdup() */
#include <stdarg.h>  /* vsnprintf() */
#include <stdio.h>   /* vsnprintf() */


typedef struct {
  int dummy;
} openiccDummy_s;

typedef struct {
  openiccDummy_s ** array;
  int reserved_n;
} openiccArray_s;

int      openiccArray_Count          ( openiccArray_s    * array )
{
  int count = 0;
  while(array->array[count]) ++count;
  return count;
}

int      openiccArray_Add            ( openiccArray_s    * array,
                                       int                 add )
{
  int count = openiccArray_Count( array );

  if((count + 1) >= array->reserved_n)
  {
    int new_count = array->reserved_n * 5;
    openiccDummy_s ** ptrs;

    if(add > 1)
      new_count = count + add;

    ptrs = calloc( sizeof(openiccDummy_s*), new_count );
    if(!ptrs)
    {
      ERRc_S("%s new_count: %d", _("Could not alloc memory"), new_count );
      return 1;
    }

    memmove( ptrs, array->array, sizeof(openiccDummy_s*) * count );
    free( array->array );
    array->array = ptrs;
    array->reserved_n = new_count;
  }

  return 0;
}

int      openiccArray_Push           ( openiccArray_s    * array )
{
  return openiccArray_Add( array, 1 );
}

/** \addtogroup OpenICC_config
 *  @{
 */

/**
 *  @internal
 *  @brief    add a openiccConfig_s
 *  @memberof openiccDB_s
 */
int           openiccDB_AddScope     ( openiccDB_s       * db,
                                       const char        * top_key_name,
                                       openiccSCOPE_e      scope )
{
  int error = 0;
  const char * config_file = OPENICC_DB_PREFIX OPENICC_SLASH OPENICC_DB;
  int i;
  /* Locate the directories where the config file is, */
  /* and where we should copy the profile to. */
  int npaths;
  xdg_error er;
  char **paths;

  if ((npaths = xdg_bds(&er, &paths, xdg_conf, xdg_write, 
                        (scope == openiccSCOPE_SYSTEM) ? xdg_local : xdg_user,
                        config_file)) == 0)
  {
    ERRc_S("%s %d", _("Could not find config"), scope );
    return 1;
  }

  if(openicc_debug)
    DBGc_S("%s", _("Paths:"));
  for(i=0; i < npaths; ++i)
    if(openicc_debug)
      DBGc_S("%s", paths[i]);

  for(i = 0; i < npaths; ++i)
  {
    const char * db_file = paths[i];
    /* read JSON input file */
    size_t size = 0;
    char * text = openiccOpenFile( db_file, &size );

    /* parse JSON */
    if(text)
    {
      int count = openiccArray_Count( (openiccArray_s*)&db->ks );
      openiccConfig_s * config = openiccConfig_FromMem( text );
      if(text) free(text); text = NULL;
      openiccConfig_SetInfo ( config, db_file );

      /* reserve enough memory in list array */
      if( openiccArray_Push( (openiccArray_s*)&db->ks ))
      {
        ERRc_S("%s", _("Could not alloc memory") );
        return 1;
      }

      /* add new config to db */
      db->ks[count] = config;
    }
  }

  xdg_free(paths, npaths);

  return error;
}

/**
 *  @brief    create a new DB object
 *  @memberof openiccDB_s
 *
 *  Create a DB by scope. Locate the physical configuration DB's and 
 *  store them inside a single object.
 */
openiccDB_s * openiccDB_NewFrom      ( const char        * top_key_name,
                                       openiccSCOPE_e      scope )
{
  openiccDB_s * db = calloc( sizeof(openiccDB_s), 1 );

  if(db)
  {
    db->type = openiccOBJECT_DB;
    db->top_key_name = openiccStringCopy( top_key_name, malloc );
    db->scope = scope;
    db->ks_array_reserved_n = 10;
    db->ks = calloc( sizeof(openiccConfig_s *), db->ks_array_reserved_n );
  }

  if(db)
  {
    int error = 0;

    if(!error &&
       (db->scope == openiccSCOPE_USER_SYS || db->scope == openiccSCOPE_USER))
    {
      error = openiccDB_AddScope( db, top_key_name, openiccSCOPE_USER );
    }

    if(!error &&
       (db->scope == openiccSCOPE_USER_SYS || db->scope == openiccSCOPE_SYSTEM))
    {
      error = openiccDB_AddScope( db, top_key_name, openiccSCOPE_SYSTEM );
    }

    if(error)
      ERRc_S("%s: %s %d", _("Could not setup db objetc"), top_key_name, scope );
  }

  return db;
}


/**
 *  @brief    free a DB object
 *  @memberof openiccDB_s
 */
void     openiccDB_Release           ( openiccDB_s      ** db )
{
  openiccDB_s * s;

  if(db)
  {
    int count, i;
    s = *db;

    if(!s)
      return;

    free( s->top_key_name );
    count = openiccArray_Count( (openiccArray_s*)&s->ks );
    for(i = 0; i < count; ++i)
      openiccConfig_Release( &s->ks[i] );
    free( s->ks );
    s->ks_array_reserved_n = 0;
    free( s );
    *db = 0;
  }
}

/**
 *  @brief    get a value
 *  @memberof openiccConfig_s
 *
 *  @param[in]     db                  a data base object
 *  @param[in]     xpath               key name to ask for
 *  @param[out]    value               found value; optional
 *  @return                            0 - success, >=1 - error, <0 - issue
 */
int                openiccDB_GetString (
                                       openiccDB_s       * db,
                                       const char        * xpath,
                                       const char       ** value )
{
  int error = !db || !xpath;

  if(error == 0)
  {
    int count = openiccArray_Count( (openiccArray_s*)&db->ks ), i;
    for(i = 0; i < count; ++i)
    {
      error = openiccConfig_GetString( db->ks[i], xpath, value );
      if(error == 0) break;
    }
  }

  return error;
}

/**
 *  @brief    get a filtered list of key names
 *  @memberof openiccDB_s
 *
 *  @param[in]     db                  a data base object
 *  @param[in]     xpath               top key name to filter for
 *  @param[in]     child_levels        how deeply nested child levels are
 *                                     desired; 0 - means all levels
 *  @param[in]     alloc               user allocation function; optional -
 *                                     default: malloc
 *  @param[out]    key_names           found full keys with path part; optional
 *  @param[out]    n                   number of found keys; optional
 *  @return                            0 - success, >=1 - error, <0 - issue
 */
int                openiccDB_GetKeyNames (
                                       openiccDB_s       * db,
                                       const char        * xpath,
                                       int                 child_levels,
                                       openiccAlloc_f      alloc,
                                       openiccDeAlloc_f    dealloc,
                                       char            *** key_names,
                                       int               * n )
{
  int error = !db || !xpath;

  if(!error)
  {
    int count = openiccArray_Count( (openiccArray_s*)&db->ks ), i;
    char ** ks = NULL;
    int     ks_n = 0;
    for(i = 0; i < count; ++i)
    {
      char ** ks_tmp = NULL;
      int     ks_tmp_n = 0;
      
      error = openiccConfig_GetKeyNames( db->ks[i], xpath, child_levels, alloc, &ks_tmp, &ks_tmp_n );
      if(ks_tmp)
      {
        oyjl_string_list_add_list( &ks, &ks_n, (const char **)ks_tmp, ks_tmp_n,
                                   alloc, dealloc );
        oyjl_string_list_release( &ks_tmp, ks_tmp_n, dealloc );
        oyjl_string_list_free_doubles( ks, &ks_n, dealloc );
      }
    }

    if(key_names)
      *key_names = ks;
    else
      oyjl_string_list_release( &ks, ks_n, dealloc );
    if(n) *n = ks_n;
  }

  return error;
}

/**
 *  @brief    get a plain key name
 *
 *  This function takes in a key of pattern:
 *  "path1/path2/key.attribute" and returns a pure key: "key" without path
 *  parts or attributes.
 *
 *  @param[in]     key                 a key name string
 *  @param[in]     temp                a temporary string to be freed by the user
 *  @return                            the short key name
 */
const char * openiccGetShortKeyFromFullKeyPath( const char * key, char ** temp )
{
  /* key starts after the path */
  const char * key_short = strrchr( key, '/' ),
             * k;
  char * k_temp = NULL;
  if(key_short) ++key_short; else key_short = key;
  /* The key ends with before '.', which are essentially attributes, 
   *  or end of string. Cut '.' parts off. */
  k = strchr( key_short, '.' );
  if(k)
  {
    k_temp = openiccStringCopy( key_short, malloc );
    k_temp[strlen(key_short) - strlen(k)] = '\000';
    key_short = k_temp;
  }
  *temp = k_temp;
  return key_short;
}

int      openiccDBSetString          ( const char        * keyName,
                                       openiccSCOPE_e      scope,
                                       const char        * value,
                                       const char        * comment )
{
  openiccDB_s * db = openiccDB_NewFrom( keyName, scope );
  int error = !db || !keyName;
  return error;
}

char *   openiccDBSearchEmptyKeyname ( const char        * keyParentName,
                                       openiccSCOPE_e      scope )
{
  char * key = NULL;
  int end = 0;
  const char * xpath = keyParentName;
  openiccDB_s * db = openiccDB_NewFrom( xpath, scope );
  int error = !db || !xpath,
      xpath_is_array = 1;

  if(error == 0)
  {
    int count = openiccArray_Count( (openiccArray_s*)&db->ks ), i;
    for(i = 0; i < count; ++i)
    {
      oyjl_val o = oyjl_tree_get_value( db->ks[i]->oyjl, xpath );
      error = !o ? -1:0;
      if(o && !OYJL_IS_ARRAY(o))
        xpath_is_array = 0;
      end = oyjl_value_count( o );
      if(error == 0) break;
    }
  }

  openiccDB_Release( &db );

  if(xpath_is_array)
    openiccStringAddPrintf( &key, 0,0, "%s/[%d]", keyParentName, end );

  return key;
}

/*  @} *//* OpenICC_config */
