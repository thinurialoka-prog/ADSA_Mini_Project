#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "graph.h"

/* ================================================================
 * CITY LOCATION IDs  (fixed indices matching CityGraph.locations[])
 * ================================================================ */

#define LOC_UNIVERSITY       0
#define LOC_RESIDENTIAL      1
#define LOC_BUS_TERMINAL     2
#define LOC_RAILWAY_STATION  3
#define LOC_HOSPITAL         4
#define LOC_SHOPPING_MALL    5
#define LOC_CITY_CENTER      6
#define LOC_AIRPORT          7
#define LOC_INDUSTRIAL       8
#define LOC_STADIUM          9
#define CITY_LOCATION_COUNT  10

/* ================================================================
 * SHARED TRANSPORT CONSTANTS
 * ================================================================ */

#define MAX_BUS_STOPS   12   /* max stops per bus route           */
#define MAX_BUSES       10   /* max buses in the fleet            */
#define BUS_ID_LEN      8    /* e.g. "B1", "B2" …                */
#define ROUTE_NAME_LEN  48   /* human-readable route label        */

#define MAX_TRAIN_STOPS 10   /* max stops per train route         */
#define MAX_TRAINS      8    /* max trains in the fleet           */
#define TRAIN_ID_LEN    8    /* e.g. "T1", "T2" …                */

/* ================================================================
 * BUS STRUCTURE
 *
 *  Represents a single bus service operating on a fixed route.
 *  stops[] holds location IDs (matching CityGraph.locations[]).
 * ================================================================ */

typedef struct {
    char bus_id[BUS_ID_LEN];          /* unique identifier, e.g. "B1"     */
    char route_name[ROUTE_NAME_LEN];  /* descriptive route label           */
    int  capacity;                    /* max passengers the bus can carry  */
    int  current_passengers;          /* passengers currently on board     */
    int  frequency_min;               /* service interval in minutes       */
    int  stops[MAX_BUS_STOPS];        /* ordered list of location IDs      */
    int  num_stops;                   /* number of stops in stops[]        */
} Bus;

/* ================================================================
 * BUS NETWORK  (global fleet managed by this module)
 * ================================================================ */

typedef struct {
    Bus buses[MAX_BUSES];   /* array of all bus services    */
    int bus_count;          /* how many buses are active    */
} BusNetwork;

/* ================================================================
 * CITY GRAPH FUNCTIONS
 * ================================================================ */

/*
 * Build the complete 10-location fictional smart-city graph.
 * Adds all Locations and all graph-level Bus / Train routes.
 * The graph must already be initialised with initializeGraph().
 */
void initializeCity(CityGraph *graph);

/*
 * Print the full city network: route-summary table + adjacency list.
 */
void displayCityNetwork(const CityGraph *graph);

/* ================================================================
 * BUS NETWORK FUNCTIONS
 * ================================================================ */

/*
 * Populate *network with all predefined bus services.
 * Call once at startup before any bus-related queries.
 */
void initializeBuses(BusNetwork *network);

/*
 * Print a compact table listing every bus: ID, route name,
 * capacity, frequency, and current passenger load.
 */
void displayBuses(const BusNetwork *network);

/*
 * Print the full stop sequence for every bus route,
 * cross-referencing location names from *graph*.
 */
void displayBusRoutes(const BusNetwork *network, const CityGraph *graph);

/*
 * Return the capacity of the bus at index bus_index,
 * or -1 if the index is out of range.
 */
int getBusCapacity(const BusNetwork *network, int bus_index);

/*
 * Set current_passengers to 0 for every bus in the fleet.
 * Used to reset state between simulation runs.
 */
void resetBusPassengers(BusNetwork *network);

/* ================================================================
 * TRAIN STRUCTURE
 *
 *  Represents a single train service on a fixed inter-station route.
 *  stops[] holds location IDs matching CityGraph.locations[].
 * ================================================================ */

typedef struct {
    char train_id[TRAIN_ID_LEN];       /* unique identifier, e.g. "T1"      */
    char route_name[ROUTE_NAME_LEN];   /* descriptive route label            */
    int  capacity;                     /* max passengers per train           */
    int  current_passengers;           /* passengers currently on board      */
    int  frequency_min;                /* service interval in minutes        */
    int  stops[MAX_TRAIN_STOPS];       /* ordered list of location IDs       */
    int  num_stops;                    /* number of stops in stops[]         */
} Train;

/* ================================================================
 * TRAIN NETWORK  (fleet managed by this module)
 * ================================================================ */

typedef struct {
    Train trains[MAX_TRAINS];  /* array of all train services   */
    int   train_count;         /* how many trains are active    */
} TrainNetwork;

/* ================================================================
 * TRAIN NETWORK FUNCTIONS
 * ================================================================ */

/*
 * Populate *network with all predefined train services.
 * Call once at startup before any train-related queries.
 */
void initializeTrains(TrainNetwork *network);

/*
 * Print a compact table listing every train: ID, route name,
 * capacity, frequency, and current passenger load.
 */
void displayTrains(const TrainNetwork *network);

/*
 * Print the full stop sequence for every train route,
 * cross-referencing location names from *graph*.
 */
void displayTrainRoutes(const TrainNetwork *network, const CityGraph *graph);

/*
 * Return the capacity of the train at index train_index,
 * or -1 if the index is out of range.
 */
int getTrainCapacity(const TrainNetwork *network, int train_index);

/*
 * Set current_passengers to 0 for every train in the fleet.
 * Used to reset state between simulation runs.
 */
void resetTrainPassengers(TrainNetwork *network);

/* ================================================================
 * LEGACY PLACEHOLDER STUBS  (kept for linker compatibility)
 * ================================================================ */

void init_transport_network(CityGraph *graph);
void add_bus_route(CityGraph *graph, int route_id, const char *name,
                   int capacity, int frequency);
void add_train_route(CityGraph *graph, int route_id, const char *name,
                     int capacity, int frequency);
void display_transport_network(void);

#endif /* TRANSPORT_H */
