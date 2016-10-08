/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.6 at Sat Oct  8 16:24:12 2016. */

#ifndef PB_GOOGLEMAPSDIRECTION_PB_H_INCLUDED
#define PB_GOOGLEMAPSDIRECTION_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _GoogleMapsDirection_TurnDirection {
    GoogleMapsDirection_TurnDirection_Right = 0,
    GoogleMapsDirection_TurnDirection_Left = 1,
    GoogleMapsDirection_TurnDirection_Straight = 2,
    GoogleMapsDirection_TurnDirection_TurnAround = 3
} GoogleMapsDirection_TurnDirection;
#define _GoogleMapsDirection_TurnDirection_MIN GoogleMapsDirection_TurnDirection_Right
#define _GoogleMapsDirection_TurnDirection_MAX GoogleMapsDirection_TurnDirection_TurnAround
#define _GoogleMapsDirection_TurnDirection_ARRAYSIZE ((GoogleMapsDirection_TurnDirection)(GoogleMapsDirection_TurnDirection_TurnAround+1))

/* Struct definitions */
typedef struct _GoogleMapsDirection {
    pb_callback_t distance;
    pb_callback_t direction;
    pb_callback_t eta;
    GoogleMapsDirection_TurnDirection tdirection;
/* @@protoc_insertion_point(struct:GoogleMapsDirection) */
} GoogleMapsDirection;

/* Default values for struct fields */

/* Initializer values for message structs */
#define GoogleMapsDirection_init_default         {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, (GoogleMapsDirection_TurnDirection)0}
#define GoogleMapsDirection_init_zero            {{{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, (GoogleMapsDirection_TurnDirection)0}

/* Field tags (for use in manual encoding/decoding) */
#define GoogleMapsDirection_distance_tag         1
#define GoogleMapsDirection_direction_tag        2
#define GoogleMapsDirection_eta_tag              3
#define GoogleMapsDirection_tdirection_tag       4

/* Struct field encoding specification for nanopb */
extern const pb_field_t GoogleMapsDirection_fields[5];

/* Maximum encoded size of messages (where known) */
/* GoogleMapsDirection_size depends on runtime parameters */

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define GOOGLEMAPSDIRECTION_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
