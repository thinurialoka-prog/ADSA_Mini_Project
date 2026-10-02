/* ================================================================
 * graph.c – Adjacency-list city graph implementation
 *
 * Each city location is a vertex stored in a fixed-size array.
 * Outgoing routes from each vertex are stored as a singly-linked
 * list of Route nodes allocated on the heap.
 * ================================================================ */

#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------- */

/* Return the transport type as a readable string. */
static const char *transportName(TransportType t) {
    switch (t) {
        case TRANSPORT_BUS:   return "Bus";
        case TRANSPORT_TRAIN: return "Train";
        default:              return "Unknown";
    }
}

/*
 * Validate that id is a legal index into graph->locations[].
 * Returns 1 (true) if valid, 0 (false) otherwise.
 */
static int isValidId(const CityGraph *graph, int id) {
    return (graph != NULL) && (id >= 0) && (id < graph->location_count);
}

/*
 * Allocate and populate a new Route node.
 * Returns a pointer on success, or NULL if malloc fails.
 */
static Route *createRoute(int           dest_id,
                           TransportType transport,
                           double        distance,
                           double        travel_time,
                           double        fare,
                           int           capacity)
{
    Route *r = (Route *)malloc(sizeof(Route));
    if (r == NULL) {
        fprintf(stderr, "[Graph] ERROR: malloc failed for Route node.\n");
        return NULL;
    }
    r->destination = dest_id;
    r->transport   = transport;
    r->distance    = distance;
    r->travel_time = travel_time;
    r->fare        = fare;
    r->capacity    = capacity;
    r->next        = NULL;
    return r;
}

/*
 * Append a Route node to the tail of the linked list that is
 * anchored at location->routes.  Appending to the tail keeps the
 * display order consistent with the insertion order.
 */
static void appendRoute(Location *location, Route *new_route) {
    if (location->routes == NULL) {
        location->routes = new_route;
        return;
    }
    /* Walk to last node */
    Route *cursor = location->routes;
    while (cursor->next != NULL) {
        cursor = cursor->next;
    }
    cursor->next = new_route;
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void initializeGraph(CityGraph *graph) {
    if (graph == NULL) {
        fprintf(stderr, "[Graph] ERROR: NULL pointer passed to initializeGraph.\n");
        return;
    }

    graph->location_count = 0;

    /* Zero-initialise every Location slot so no stale pointers exist */
    int i;
    for (i = 0; i < MAX_LOCATIONS; i++) {
        graph->locations[i].id       = -1;
        graph->locations[i].name[0]  = '\0';
        graph->locations[i].routes   = NULL;
    }

    printf("[Graph] City graph initialised (capacity: %d locations).\n",
           MAX_LOCATIONS);
}

/* ---------------------------------------------------------------- */

int addLocation(CityGraph *graph, const char *name) {
    if (graph == NULL || name == NULL) {
        fprintf(stderr, "[Graph] ERROR: NULL argument in addLocation.\n");
        return -1;
    }
    if (graph->location_count >= MAX_LOCATIONS) {
        fprintf(stderr,
                "[Graph] ERROR: Graph is full (max %d locations).\n",
                MAX_LOCATIONS);
        return -1;
    }

    int id = graph->location_count;
    Location *loc = &graph->locations[id];

    loc->id     = id;
    loc->routes = NULL;
    strncpy(loc->name, name, LOCATION_NAME_LEN - 1);
    loc->name[LOCATION_NAME_LEN - 1] = '\0'; /* guarantee NUL-termination */

    graph->location_count++;

    printf("[Graph] Location added  -> [%d] %s\n", id, loc->name);
    return id;
}

/* ---------------------------------------------------------------- */

int addRoute(CityGraph    *graph,
             int           src_id,
             int           dest_id,
             TransportType transport,
             double        distance,
             double        travel_time,
             double        fare,
             int           capacity)
{
    if (graph == NULL) {
        fprintf(stderr, "[Graph] ERROR: NULL graph pointer in addRoute.\n");
        return -1;
    }
    if (!isValidId(graph, src_id)) {
        fprintf(stderr,
                "[Graph] ERROR: Invalid source id %d in addRoute.\n", src_id);
        return -1;
    }
    if (!isValidId(graph, dest_id)) {
        fprintf(stderr,
                "[Graph] ERROR: Invalid destination id %d in addRoute.\n",
                dest_id);
        return -1;
    }
    if (src_id == dest_id) {
        fprintf(stderr,
                "[Graph] ERROR: Self-loop not allowed (id=%d).\n", src_id);
        return -1;
    }
    if (distance < 0.0 || travel_time < 0.0 || fare < 0.0 || capacity < 0) {
        fprintf(stderr,
                "[Graph] ERROR: Negative attribute value in addRoute.\n");
        return -1;
    }

    Route *new_route = createRoute(dest_id, transport, distance,
                                   travel_time, fare, capacity);
    if (new_route == NULL) {
        return -1; /* createRoute already printed the error */
    }

    appendRoute(&graph->locations[src_id], new_route);

    printf("[Graph] Route added     -> [%d] %-20s --(%s, %.1f km, %.0f min, $%.2f)--> [%d] %s\n",
           src_id, graph->locations[src_id].name,
           transportName(transport), distance, travel_time, fare,
           dest_id, graph->locations[dest_id].name);

    return 0;
}

/* ---------------------------------------------------------------- */

int addBidirectionalRoute(CityGraph    *graph,
                           int           id_a,
                           int           id_b,
                           TransportType transport,
                           double        distance,
                           double        travel_time,
                           double        fare,
                           int           capacity)
{
    /* Add A -> B */
    if (addRoute(graph, id_a, id_b, transport,
                 distance, travel_time, fare, capacity) != 0) {
        return -1;
    }
    /* Add B -> A (same attributes – symmetric route) */
    if (addRoute(graph, id_b, id_a, transport,
                 distance, travel_time, fare, capacity) != 0) {
        return -1;
    }
    return 0;
}

/* ---------------------------------------------------------------- */

void displayLocations(const CityGraph *graph) {
    if (graph == NULL) {
        fprintf(stderr,
                "[Graph] ERROR: NULL pointer passed to displayLocations.\n");
        return;
    }

    printf("\n+------------------------------------------+\n");
    printf("|          CITY LOCATIONS / STATIONS       |\n");
    printf("+------+-----------------------------------+\n");
    printf("| ID   | Name                              |\n");
    printf("+------+-----------------------------------+\n");

    int i;
    for (i = 0; i < graph->location_count; i++) {
        printf("| %-4d | %-33s |\n",
               graph->locations[i].id,
               graph->locations[i].name);
    }

    printf("+------+-----------------------------------+\n");
    printf("  Total: %d location(s)\n\n", graph->location_count);
}

/* ---------------------------------------------------------------- */

void displayGraph(const CityGraph *graph) {
    if (graph == NULL) {
        fprintf(stderr,
                "[Graph] ERROR: NULL pointer passed to displayGraph.\n");
        return;
    }

    printf("\n============================================================\n");
    printf("          SMART CITY TRANSPORT GRAPH (Adjacency List)      \n");
    printf("============================================================\n");

    int i;
    for (i = 0; i < graph->location_count; i++) {
        const Location *loc = &graph->locations[i];

        printf("\n[%d] %s\n", loc->id, loc->name);
        printf("    |\n");

        if (loc->routes == NULL) {
            printf("    +-- (no outgoing routes)\n");
            continue;
        }

        Route *r = loc->routes;
        while (r != NULL) {
            const char *arrow = (r->next != NULL) ? "|" : " ";

            printf("    +-- %-6s --> [%d] %-20s "
                   "| dist: %5.1f km | time: %4.0f min "
                   "| fare: $%5.2f | cap: %4d pax\n",
                   transportName(r->transport),
                   r->destination,
                   graph->locations[r->destination].name,
                   r->distance,
                   r->travel_time,
                   r->fare,
                   r->capacity);

            /* Suppress unused warning for 'arrow' – it's kept for
               potential future tree-style formatting */
            (void)arrow;

            r = r->next;
        }
    }

    printf("\n============================================================\n\n");
}

/* ---------------------------------------------------------------- */

void freeGraph(CityGraph *graph) {
    if (graph == NULL) {
        return;
    }

    int freed_routes = 0;
    int i;
    for (i = 0; i < graph->location_count; i++) {
        Route *current = graph->locations[i].routes;
        while (current != NULL) {
            Route *next = current->next;
            free(current);
            current = next;
            freed_routes++;
        }
        graph->locations[i].routes = NULL; /* dangle-safe */
    }

    graph->location_count = 0;
    printf("[Graph] Memory freed: %d route node(s) released.\n",
           freed_routes);
}
