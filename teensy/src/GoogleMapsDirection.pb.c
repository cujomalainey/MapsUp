/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.6 at Sat Oct  8 16:24:12 2016. */

#include "GoogleMapsDirection.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t GoogleMapsDirection_fields[5] = {
    PB_FIELD(  1, STRING  , OPTIONAL, CALLBACK, FIRST, GoogleMapsDirection, distance, distance, 0),
    PB_FIELD(  2, STRING  , REQUIRED, CALLBACK, OTHER, GoogleMapsDirection, direction, distance, 0),
    PB_FIELD(  3, STRING  , REQUIRED, CALLBACK, OTHER, GoogleMapsDirection, eta, direction, 0),
    PB_FIELD(  4, UENUM   , REQUIRED, STATIC  , OTHER, GoogleMapsDirection, tdirection, eta, 0),
    PB_LAST_FIELD
};


/* @@protoc_insertion_point(eof) */
