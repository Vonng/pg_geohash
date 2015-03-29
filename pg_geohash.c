#include "postgres.h"
#include "fmgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "geohash.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

Datum geohash_lat_lon (PG_FUNCTION_ARGS);

/*
 * Turn this into a PostreSQL callable PL/C function
 *
 * Ref. http://www.postgresql.org/docs/9.2/static/xfunc-c.html
 *
 * Function: TEXT geohash_lat_lon (FLOAT8 lat, FLOAT8 lon)
 *
 * CREATE FUNCTION geohash (FLOAT8, FLOAT8) RETURNS TEXT
 *   AS 'pg_geohash', 'geohash_lat_lon'
 *   LANGUAGE C;
 *
 *   psql      C           header
 * -----------------------------------------------
 *  float8   float8*     postgres.h
 *  varchar  VarChar*    postgres.h
 *  text     text*       postgres.h
 *
 */

PG_FUNCTION_INFO_V1 (geohash_lat_lon);
Datum
geohash_lat_lon (PG_FUNCTION_ARGS)
{
    /*
     * char* GEOHASH_encode(double latitude, double longitude, unsigned int hash_length);
     * double atof(const char *nptr);
     *
     * Earth radius = 6371 km
     * Distance per degree: ds = PI/180.0 * 6371.0e+03 m = 111,195 m/degree,
     * so if we have lat/lon values to 1.0e-15, we get down to ridiculous precision
     * 1 m precision is about 5 digits to the right of the decimal point
     *
     * Example input lat/lon: 41.884559527232234 -87.62610231958186
     *
     * Need to determine the size of the area per character chopped off the right hand
     * side of the geohash.
     */
    char *hash;
    double lat, lon;
    text *rv;
    /* Max. length of geohash is 12 */
    int hash_len = 12;
    int rv_len;

    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
      PG_RETURN_NULL();
    }

    lat = PG_GETARG_FLOAT8(0);
    lon = PG_GETARG_FLOAT8(1);
    hash = GEOHASH_encode(lat, lon, hash_len);

    /* log the length of hash? */
    /* elog(INFO, "MIKE: geohash \"%s\" has length %d", hash, (int) strlen(hash)); */

    rv_len = strlen(hash) + 1;
    rv = (text *) palloc(VARHDRSZ + rv_len);
    SET_VARSIZE(rv, VARHDRSZ + rv_len);
    /*
     * VARDATA is a pointer to the data region of the struct.
     */
    memcpy(VARDATA(rv), hash, rv_len);
    free(hash);
    PG_RETURN_TEXT_P(rv);
}

