#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

/* ================================================================
 * ROUTE OPTIMIZATION MODE
 *
 *  Selects which edge weight Dijkstra minimises.
 * ================================================================ */

typedef enum {
    OPTIMIZE_TIME     = 0,  /* minimize total travel time (minutes)  */
    OPTIMIZE_DISTANCE = 1,  /* minimize total distance  (kilometres) */
    OPTIMIZE_FARE     = 2   /* minimize total fare      (currency)   */
} OptimizeMode;

/* ================================================================
 * PATH RESULT
 *
 *  Returned by findFastestRoute().  Stores the full predecessor
 *  array and per-vertex cost so the caller can reconstruct and
 *  display any source-to-destination path.
 *
 *  path[]   – reconstructed stop sequence (location IDs), indices
 *             0 … path_len-1, in source-to-destination order.
 *  transport_used[] – transport type taken on each leg i → i+1
 *                     (valid for indices 0 … path_len-2).
 *  leg_distance[], leg_time[], leg_fare[] – per-hop attributes.
 * ================================================================ */

#define MAX_PATH_LEN MAX_LOCATIONS

typedef struct {
    /* Raw Dijkstra arrays (one slot per graph vertex) */
    double cost[MAX_LOCATIONS];    /* best cost to reach vertex v     */
    int    parent[MAX_LOCATIONS];  /* predecessor of v on best path   */
    int    visited[MAX_LOCATIONS]; /* 1 once v is finalised           */

    /* Reconstructed path */
    int           path[MAX_PATH_LEN];           /* location-ID sequence  */
    TransportType transport_used[MAX_PATH_LEN]; /* transport per leg     */
    double        leg_distance[MAX_PATH_LEN];   /* km per leg            */
    double        leg_time[MAX_PATH_LEN];       /* minutes per leg       */
    double        leg_fare[MAX_PATH_LEN];       /* fare per leg          */
    int           path_len;                     /* number of stops       */

    /* Totals */
    double total_distance; /* km                  */
    double total_time;     /* minutes             */
    double total_fare;     /* currency units      */

    /* Meta */
    int  source;           /* start location id   */
    int  destination;      /* end location id     */
    int  reachable;        /* 1 if path exists    */
    OptimizeMode mode;     /* mode used for this run */
} PathResult;

/* ================================================================
 * PUBLIC API
 * ================================================================ */

/*
 * Run Dijkstra from source to destination on *graph* using *mode*
 * as the edge-weight selector.
 *
 * Populates *result* and returns 1 if a path was found, 0 otherwise.
 *
 * Time complexity: O(V^2) with the linear-scan min-extraction used
 * here (suitable for the small city graph; a binary-heap priority
 * queue would give O((V + E) log V)).
 */
int findFastestRoute(const CityGraph *graph,
                     int              source,
                     int              destination,
                     OptimizeMode     mode,
                     PathResult      *result);

/*
 * Print the full route stored in *result* in a formatted table,
 * showing each stop, transport type, and per-leg costs, plus totals.
 */
void printRoute(const PathResult *result, const CityGraph *graph);

/*
 * Compute and return the total distance (km) along the path in
 * *result*.  Returns -1.0 if the path is not reachable.
 */
double calculateRouteDistance(const PathResult *result);

/*
 * Compute and return the total travel time (minutes).
 * Returns -1.0 if not reachable.
 */
double calculateRouteTime(const PathResult *result);

/*
 * Compute and return the total fare (currency units).
 * Returns -1.0 if not reachable.
 */
double calculateRouteFare(const PathResult *result);

/*
 * Interactive helper: prompt the user to enter source, destination,
 * and optimization mode, then call findFastestRoute + printRoute.
 */
void routeFinderMenu(const CityGraph *graph);

#endif /* DIJKSTRA_H */
