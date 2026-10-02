#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * CONSTANTS
 * ================================================================ */

#define MAX_LOCATIONS     64
#define LOCATION_NAME_LEN 64

/* ================================================================
 * TRANSPORT TYPE ENUM
 * ================================================================ */

typedef enum {
    TRANSPORT_BUS   = 0,
    TRANSPORT_TRAIN = 1
} TransportType;

/* ================================================================
 * ROUTE  (adjacency-list edge node)
 *
 *  Each Route holds one directed connection from a Location to a
 *  destination Location.  Routes are chained into a singly-linked
 *  list anchored at Location.routes.
 * ================================================================ */

typedef struct Route {
    int           destination;   /* index into CityGraph.locations[]  */
    TransportType transport;     /* BUS or TRAIN                      */
    double        distance;      /* kilometres                         */
    double        travel_time;   /* minutes                            */
    double        fare;          /* currency units                     */
    int           capacity;      /* max passengers per trip            */
    struct Route *next;          /* pointer to next Route in the list  */
} Route;

/* ================================================================
 * LOCATION  (adjacency-list vertex)
 *
 *  Each Location is a city hub / station.  Its outgoing connections
 *  are stored as a linked list of Route nodes.
 * ================================================================ */

typedef struct {
    int    id;                        /* unique, 0-based index          */
    char   name[LOCATION_NAME_LEN];   /* human-readable station name    */
    Route *routes;                    /* head of outgoing-route list    */
} Location;

/* ================================================================
 * CITYGRAPH  (the overall graph)
 *
 *  A fixed-size array of Locations plus the count of how many have
 *  been added.  MAX_LOCATIONS caps the vertex count.
 * ================================================================ */

typedef struct {
    Location locations[MAX_LOCATIONS]; /* vertex array                  */
    int       location_count;          /* vertices added so far         */
} CityGraph;

/* ================================================================
 * FUNCTION DECLARATIONS
 * ================================================================ */

/* Initialise a CityGraph to empty state (zero locations, no routes). */
void initializeGraph(CityGraph *graph);

/*
 * Add a named location/station to the graph.
 * Returns the assigned location id (0-based), or -1 on failure.
 */
int addLocation(CityGraph *graph, const char *name);

/*
 * Add a directed route from src_id -> dest_id with the given
 * properties.  Returns 0 on success, -1 on invalid ids.
 */
int addRoute(CityGraph    *graph,
             int           src_id,
             int           dest_id,
             TransportType transport,
             double        distance,
             double        travel_time,
             double        fare,
             int           capacity);

/*
 * Add two directed routes (src->dest AND dest->src) with identical
 * properties.  Returns 0 on success, -1 on any failure.
 */
int addBidirectionalRoute(CityGraph    *graph,
                          int           id_a,
                          int           id_b,
                          TransportType transport,
                          double        distance,
                          double        travel_time,
                          double        fare,
                          int           capacity);

/* Print a numbered list of every location in the graph. */
void displayLocations(const CityGraph *graph);

/*
 * Print a full adjacency-list dump: every location and every outgoing
 * route with all attributes.
 */
void displayGraph(const CityGraph *graph);

/*
 * Release all dynamically allocated Route nodes.
 * The CityGraph itself is stack- or caller-managed.
 */
void freeGraph(CityGraph *graph);

#endif /* GRAPH_H */
