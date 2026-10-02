/* ================================================================
 * transport.c  –  City network + Bus network implementation
 *
 * Section A  (initializeCity / displayCityNetwork):
 *   Builds the 10-location city graph with Bus and Train edges.
 *
 * Section B  (Bus module):
 *   Defines five bus services operating on the city graph.
 *   Each Bus carries a fixed stop sequence mapped to CityGraph
 *   location IDs so the two representations stay in sync.
 *
 * Realistic simulation assumptions used throughout:
 *   Bus speed  ≈ 20-25 km/h (urban, with stops)
 *   Capacity   : 50-80 passengers per city bus
 *   Frequency  : every 10-20 minutes on trunk routes,
 *                every 20-30 minutes on peripheral routes
 * ================================================================ */

#include "transport.h"
#include <stdio.h>
#include <string.h>

/* ================================================================
 * SECTION A – CITY GRAPH INITIALISATION
 * ================================================================ */

void initializeCity(CityGraph *graph) {

    /* ---- 10 canonical locations (order matters – matches LOC_* ids) ---- */
    addLocation(graph, "University of Colombo");        /* 0 */
    addLocation(graph, "Nugegoda Area");  /* 1 */
    addLocation(graph, "Pettah");      /* 2 */
    addLocation(graph, "Colombo Fort");   /* 3 */
    addLocation(graph, "Borella");          /* 4 */
    addLocation(graph, "Bambalapitiya");     /* 5 */
    addLocation(graph, "City Center");       /* 6 */
    addLocation(graph, "BI Airport");           /* 7 */
    addLocation(graph, "Industrial Area");   /* 8 */
    addLocation(graph, "Stadium");           /* 9 */
    printf("\n");

    /* ---- Bus graph edges ---- */
    printf("--- [ Graph Bus Routes ] ---\n");
    addBidirectionalRoute(graph, LOC_UNIVERSITY_OF_COLOMBO,   LOC_NUGEGODA,
                          TRANSPORT_BUS, 2.5, 8.0,  0.80, 60);
    addBidirectionalRoute(graph, LOC_UNIVERSITY_OF_COLOMBO,   LOC_BORELLA,
                          TRANSPORT_BUS, 3.8, 12.0, 1.00, 60);
    addBidirectionalRoute(graph, LOC_NUGEGODA,  LOC_PETTAH,
                          TRANSPORT_BUS, 4.2, 14.0, 1.00, 80);
    addBidirectionalRoute(graph, LOC_PETTAH, LOC_CITY_CENTER,
                          TRANSPORT_BUS, 3.5, 10.0, 0.90, 80);
    addBidirectionalRoute(graph, LOC_PETTAH, LOC_BAMBALAPITIYA,
                          TRANSPORT_BUS, 5.1, 16.0, 1.20, 80);
    addBidirectionalRoute(graph, LOC_PETTAH, LOC_INDUSTRIAL,
                          TRANSPORT_BUS, 7.5, 22.0, 1.50, 80);
    addBidirectionalRoute(graph, LOC_CITY_CENTER,  LOC_BORELLA,
                          TRANSPORT_BUS, 4.5, 13.0, 1.10, 60);
    addBidirectionalRoute(graph, LOC_CITY_CENTER,  LOC_BAMBALAPITIYA,
                          TRANSPORT_BUS, 2.8,  9.0, 0.80, 60);
    addBidirectionalRoute(graph, LOC_CITY_CENTER,  LOC_STADIUM,
                          TRANSPORT_BUS, 6.2, 18.0, 1.30, 80);
    addBidirectionalRoute(graph, LOC_BAMBALAPITIYA,LOC_STADIUM,
                          TRANSPORT_BUS, 4.0, 12.0, 1.00, 60);
    addBidirectionalRoute(graph, LOC_BI_AIRPORT,      LOC_CITY_CENTER,
                          TRANSPORT_BUS, 18.0,35.0, 4.00, 50);
    printf("\n");

    /* ---- Train graph edges ---- */
    printf("--- [ Graph Train Routes ] ---\n");
    addBidirectionalRoute(graph, LOC_COLOMBO_FORT, LOC_CITY_CENTER,
                          TRANSPORT_TRAIN, 6.0,  8.0, 2.00, 300);
    addBidirectionalRoute(graph, LOC_COLOMBO_FORT, LOC_UNIVERSITY_OF_COLOMBO,
                          TRANSPORT_TRAIN, 8.5, 10.0, 2.20, 300);
    addBidirectionalRoute(graph, LOC_COLOMBO_FORT, LOC_BI_AIRPORT,
                          TRANSPORT_TRAIN, 25.0,22.0, 6.00, 400);
    addBidirectionalRoute(graph, LOC_COLOMBO_FORT, LOC_INDUSTRIAL,
                          TRANSPORT_TRAIN, 12.0,14.0, 2.50, 350);
    addBidirectionalRoute(graph, LOC_COLOMBO_FORT, LOC_BORELLA,
                          TRANSPORT_TRAIN, 7.2,  9.0, 2.10, 250);
    addBidirectionalRoute(graph, LOC_CITY_CENTER,     LOC_STADIUM,
                          TRANSPORT_TRAIN, 5.5,  7.0, 1.80, 400);
    printf("\n");

    printf("[Transport] City graph ready: %d locations loaded.\n",
           graph->location_count);
}

/* ---------------------------------------------------------------- */

void displayCityNetwork(const CityGraph *graph) {
    if (graph == NULL) return;

    printf("\n");
    printf("================================================================\n");
    printf("      SMART CITY TRANSPORT NETWORK — ROUTE SUMMARY             \n");
    printf("================================================================\n");
    printf(" %-22s %-22s %-6s %5s %5s %5s %4s\n",
           "FROM", "TO", "TYPE", "KM", "MIN", "FARE", "PAX");
    printf("----------------------------------------------------------------\n");

    int i;
    for (i = 0; i < graph->location_count; i++) {
        const Route *r = graph->locations[i].routes;
        while (r != NULL) {
            /* Skip backward half of bidirectional pairs (src > dest).
               True one-way edges where no reverse exists are kept. */
            int has_reverse = 0;
            if (i > r->destination) {
                const Route *rev = graph->locations[r->destination].routes;
                while (rev) {
                    if (rev->destination == i &&
                        rev->transport   == r->transport) {
                        has_reverse = 1; break;
                    }
                    rev = rev->next;
                }
                if (has_reverse) { r = r->next; continue; }
            }

            int bidir = 0;
            if (i < r->destination) {
                const Route *rev = graph->locations[r->destination].routes;
                while (rev) {
                    if (rev->destination == i &&
                        rev->transport   == r->transport) {
                        bidir = 1; break;
                    }
                    rev = rev->next;
                }
            }

            printf(" %-22s %-22s %-6s %5.1f %5.0f %5.2f %4d %s\n",
                   graph->locations[i].name,
                   graph->locations[r->destination].name,
                   (r->transport == TRANSPORT_BUS) ? "Bus" : "Train",
                   r->distance, r->travel_time, r->fare, r->capacity,
                   bidir ? "(bi)" : "(->)");
            r = r->next;
        }
    }

    printf("================================================================\n");
    printf("  Legend: (bi) bidirectional   (->) one-way\n");
    printf("================================================================\n\n");

    displayGraph(graph);
}

/* ================================================================
 * SECTION B – BUS NETWORK
 *
 *  Five bus services, each mapped to the same location IDs used
 *  in the CityGraph so both representations stay consistent.
 *
 *  Route overview:
 *
 *   B1 "University of Colombo Corridor"
 *       University of Colombo -> Pettah -> Borella -> Bambalapitiya
 *       Serves student / medical / retail commuters.
 *
 *   B2 "Nugegoda Express"
 *       Nugegoda -> Pettah -> City Center -> Stadium
 *       Daily commuter trunk line from suburbs to downtown.
 *
 *   B3 "Airport Shuttle"
 *       BI Airport -> City Center -> Bambalapitiya
 *       Connects the airport to the commercial district.
 *
 *   B4 "Industrial Link"
 *       Industrial Area -> Pettah -> Nugegoda -> University of Colombo
 *       Early-morning shift-worker and student route.
 *
 *   B5 "City Loop"
 *       City Center -> Borella -> University of Colombo -> Nugegoda
 *       -> Pettah -> Bambalapitiya -> Stadium -> City Center
 *       Full inner-city circular loop.
 * ================================================================ */

/* ----------------------------------------------------------------
 * Internal helper: add one bus record to the network
 * ---------------------------------------------------------------- */
static void addBus(BusNetwork  *net,
                   const char  *id,
                   const char  *route_name,
                   int          capacity,
                   int          frequency_min,
                   const int   *stops,
                   int          num_stops)
{
    if (net->bus_count >= MAX_BUSES) {
        fprintf(stderr, "[Bus] ERROR: Fleet is full (max %d buses).\n",
                MAX_BUSES);
        return;
    }
    if (num_stops > MAX_BUS_STOPS) {
        fprintf(stderr,
                "[Bus] ERROR: Route '%s' has %d stops (max %d).\n",
                id, num_stops, MAX_BUS_STOPS);
        return;
    }

    Bus *b = &net->buses[net->bus_count];

    strncpy(b->bus_id,     id,         BUS_ID_LEN    - 1);
    strncpy(b->route_name, route_name, ROUTE_NAME_LEN - 1);
    b->bus_id[BUS_ID_LEN    - 1] = '\0';
    b->route_name[ROUTE_NAME_LEN - 1] = '\0';

    b->capacity           = capacity;
    b->current_passengers = 0;
    b->frequency_min      = frequency_min;
    b->num_stops          = num_stops;

    int i;
    for (i = 0; i < num_stops; i++) {
        b->stops[i] = stops[i];
    }

    net->bus_count++;
}

/* ----------------------------------------------------------------
 * initializeBuses
 * ---------------------------------------------------------------- */
void initializeBuses(BusNetwork *network) {
    if (network == NULL) {
        fprintf(stderr, "[Bus] ERROR: NULL BusNetwork pointer.\n");
        return;
    }

    network->bus_count = 0;

    /* B1 – University of Colombo Corridor */
    {
        const int stops[] = {
            LOC_UNIVERSITY_OF_COLOMBO, LOC_PETTAH,
            LOC_BORELLA,   LOC_BAMBALAPITIYA
        };
        addBus(network, "B1", "University of Colombo Corridor",
               70, 15, stops, 4);
    }

    /* B2 – Nugegoda Express */
    {
        const int stops[] = {
            LOC_NUGEGODA, LOC_PETTAH,
            LOC_CITY_CENTER, LOC_STADIUM
        };
        addBus(network, "B2", "Nugegoda Express",
               80, 10, stops, 4);
    }

    /* B3 – Airport Shuttle */
    {
        const int stops[] = {
            LOC_BI_AIRPORT, LOC_CITY_CENTER, LOC_BAMBALAPITIYA
        };
        addBus(network, "B3", "Airport Shuttle",
               50, 30, stops, 3);
    }

    /* B4 – Industrial Link */
    {
        const int stops[] = {
            LOC_INDUSTRIAL,  LOC_PETTAH,
            LOC_NUGEGODA, LOC_UNIVERSITY_OF_COLOMBO
        };
        addBus(network, "B4", "Industrial Link",
               80, 20, stops, 4);
    }

    /* B5 – City Loop (circular) */
    {
        const int stops[] = {
            LOC_CITY_CENTER,  LOC_BORELLA,
            LOC_UNIVERSITY_OF_COLOMBO,   LOC_NUGEGODA,
            LOC_PETTAH, LOC_BAMBALAPITIYA,
            LOC_STADIUM,      LOC_CITY_CENTER
        };
        addBus(network, "B5", "City Loop",
               60, 12, stops, 8);
    }

    printf("[Bus] Fleet initialised: %d bus service(s) ready.\n",
           network->bus_count);
}

/* ----------------------------------------------------------------
 * displayBuses
 * ---------------------------------------------------------------- */
void displayBuses(const BusNetwork *network) {
    if (network == NULL) return;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║               BUS FLEET — SERVICE SUMMARY                   ║\n");
    printf("╠════╦══════════════════════╦═════╦═══════╦═══════╦═══════════╣\n");
    printf("║ ID ║ Route Name           ║ Cap ║ Freq  ║ Pax   ║ Load      ║\n");
    printf("╠════╬══════════════════════╬═════╬═══════╬═══════╬═══════════╣\n");

    int i;
    for (i = 0; i < network->bus_count; i++) {
        const Bus *b = &network->buses[i];
        double load_pct = (b->capacity > 0)
                          ? (100.0 * b->current_passengers / b->capacity)
                          : 0.0;

        /* Visual load bar (10 chars wide) */
        char bar[11];
        int filled = (int)(load_pct / 10.0);
        int j;
        for (j = 0; j < 10; j++) bar[j] = (j < filled) ? '#' : '.';
        bar[10] = '\0';

        printf("║ %-2s ║ %-20s ║ %3d ║ %3d min ║ %3d/%3d ║ [%s] %3.0f%% ║\n",
               b->bus_id,
               b->route_name,
               b->capacity,
               b->frequency_min,
               b->current_passengers,
               b->capacity,
               bar,
               load_pct);
    }

    printf("╚════╩══════════════════════╩═════╩═══════╩═══════╩═══════════╝\n");
    printf("  Total buses: %d\n\n", network->bus_count);
}

/* ----------------------------------------------------------------
 * displayBusRoutes
 * ---------------------------------------------------------------- */
void displayBusRoutes(const BusNetwork *network, const CityGraph *graph) {
    if (network == NULL || graph == NULL) return;

    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                  BUS ROUTES — STOP SEQUENCES                  \n");
    printf("════════════════════════════════════════════════════════════════\n");

    int i;
    for (i = 0; i < network->bus_count; i++) {
        const Bus *b = &network->buses[i];

        printf("\n  ┌─────────────────────────────────────────────┐\n");
        printf("  │  Bus %-3s │ %-28s │\n", b->bus_id, b->route_name);
        printf("  │  Capacity: %-3d pax  │  Every %-3d min          │\n",
               b->capacity, b->frequency_min);
        printf("  ├─────────────────────────────────────────────┤\n");

        int s;
        for (s = 0; s < b->num_stops; s++) {
            int loc_id = b->stops[s];
            const char *loc_name =
                (loc_id >= 0 && loc_id < graph->location_count)
                    ? graph->locations[loc_id].name
                    : "Unknown";

            if (s == 0) {
                printf("  │  [START] [%d] %-31s│\n", loc_id, loc_name);
            } else if (s == b->num_stops - 1 &&
                       b->stops[s] == b->stops[0]) {
                /* Circular route: last stop same as first */
                printf("  │     |                                         │\n");
                printf("  │  [LOOP]  [%d] %-31s│\n", loc_id, loc_name);
            } else if (s == b->num_stops - 1) {
                printf("  │     |                                         │\n");
                printf("  │  [ END ] [%d] %-31s│\n", loc_id, loc_name);
            } else {
                printf("  │     |                                         │\n");
                printf("  │  [STOP%d] [%d] %-31s│\n", s, loc_id, loc_name);
            }
        }
        printf("  └─────────────────────────────────────────────┘\n");
    }

    printf("\n════════════════════════════════════════════════════════════════\n\n");
}

/* ----------------------------------------------------------------
 * getBusCapacity
 * ---------------------------------------------------------------- */
int getBusCapacity(const BusNetwork *network, int bus_index) {
    if (network == NULL || bus_index < 0 ||
        bus_index >= network->bus_count) {
        fprintf(stderr,
                "[Bus] ERROR: Invalid bus index %d in getBusCapacity.\n",
                bus_index);
        return -1;
    }
    return network->buses[bus_index].capacity;
}

/* ----------------------------------------------------------------
 * resetBusPassengers
 * ---------------------------------------------------------------- */
void resetBusPassengers(BusNetwork *network) {
    if (network == NULL) return;
    int i;
    for (i = 0; i < network->bus_count; i++) {
        network->buses[i].current_passengers = 0;
    }
    printf("[Bus] All bus passenger counts reset to 0.\n");
}

/* ================================================================
 * SECTION C – TRAIN NETWORK
 *
 *  Three train services modelled on the city's rail infrastructure.
 *  All stop IDs match CityGraph.locations[] and the TRANSPORT_TRAIN
 *  graph edges added in initializeCity().
 *
 *  Realistic assumptions:
 *    Train speed  ≈ 60-80 km/h (metro / commuter rail)
 *    Capacity     : 250-400 passengers per train
 *    Frequency    : every 8-20 minutes depending on line demand
 *
 *  Route overview:
 *
 *   T1 "Northern Line"
 *       Nugegoda  -> Colombo Fort  -> City Center -> BI Airport
 *       Main commuter trunk connecting suburbs, the rail hub,
 *       downtown, and the BI Airport.
 *
 *   T2 "Industrial Shuttle"
 *       Industrial Area -> Colombo Fort  -> University of Colombo
 *       Early-morning shift-worker and student express;
 *       limited stops for maximum speed.
 *
 *   T3 "Airport Metro"
 *       BI Airport -> Colombo Fort -> City Center -> Stadium
 *       Premium BI Airport link continuing into the entertainment
 *       district; elevated frequency on event days.
 * ================================================================ */

/* ----------------------------------------------------------------
 * Internal helper: add one train record to the network
 * ---------------------------------------------------------------- */
static void addTrain(TrainNetwork *net,
                     const char   *id,
                     const char   *route_name,
                     int           capacity,
                     int           frequency_min,
                     const int    *stops,
                     int           num_stops)
{
    if (net->train_count >= MAX_TRAINS) {
        fprintf(stderr, "[Train] ERROR: Fleet is full (max %d trains).\n",
                MAX_TRAINS);
        return;
    }
    if (num_stops > MAX_TRAIN_STOPS) {
        fprintf(stderr,
                "[Train] ERROR: Route '%s' has %d stops (max %d).\n",
                id, num_stops, MAX_TRAIN_STOPS);
        return;
    }

    Train *t = &net->trains[net->train_count];

    strncpy(t->train_id,    id,         TRAIN_ID_LEN   - 1);
    strncpy(t->route_name,  route_name, ROUTE_NAME_LEN - 1);
    t->train_id[TRAIN_ID_LEN   - 1] = '\0';
    t->route_name[ROUTE_NAME_LEN - 1] = '\0';

    t->capacity           = capacity;
    t->current_passengers = 0;
    t->frequency_min      = frequency_min;
    t->num_stops          = num_stops;

    int i;
    for (i = 0; i < num_stops; i++) {
        t->stops[i] = stops[i];
    }

    net->train_count++;
}

/* ----------------------------------------------------------------
 * initializeTrains
 * ---------------------------------------------------------------- */
void initializeTrains(TrainNetwork *network) {
    if (network == NULL) {
        fprintf(stderr, "[Train] ERROR: NULL TrainNetwork pointer.\n");
        return;
    }

    network->train_count = 0;

    /* T1 – Northern Line
     *   Nugegoda -> Colombo Fort -> City Center -> BI Airport
     *   Core commuter service; highest capacity, highest frequency. */
    {
        const int stops[] = {
            LOC_NUGEGODA,    LOC_COLOMBO_FORT,
            LOC_CITY_CENTER,    LOC_BI_AIRPORT
        };
        addTrain(network, "T1", "Northern Line",
                 350, 8, stops, 4);
    }

    /* T2 – Industrial Shuttle
     *   Industrial Area -> Colombo Fort -> University of Colombo
     *   Shift-worker and student express; fewer stops, fast journey. */
    {
        const int stops[] = {
            LOC_INDUSTRIAL, LOC_COLOMBO_FORT, LOC_UNIVERSITY_OF_COLOMBO
        };
        addTrain(network, "T2", "Industrial Shuttle",
                 300, 15, stops, 3);
    }

    /* T3 – Airport Metro
     *   Airport -> Colombo Fort -> City Center -> Stadium
     *   Premium airport link into the entertainment district.
     *   Elevated frequency on match/event days. */
    {
        const int stops[] = {
            LOC_BI_AIRPORT,     LOC_COLOMBO_FORT,
            LOC_CITY_CENTER,    LOC_STADIUM
        };
        addTrain(network, "T3", "Airport Metro",
                 400, 12, stops, 4);
    }

    printf("[Train] Fleet initialised: %d train service(s) ready.\n",
           network->train_count);
}

/* ----------------------------------------------------------------
 * displayTrains
 * ---------------------------------------------------------------- */
void displayTrains(const TrainNetwork *network) {
    if (network == NULL) return;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              TRAIN FLEET — SERVICE SUMMARY                  ║\n");
    printf("╠════╦══════════════════════╦═════╦═══════╦════════╦══════════╣\n");
    printf("║ ID ║ Route Name           ║ Cap ║ Freq  ║ Pax    ║ Load     ║\n");
    printf("╠════╬══════════════════════╬═════╬═══════╬════════╬══════════╣\n");

    int i;
    for (i = 0; i < network->train_count; i++) {
        const Train *t = &network->trains[i];
        double load_pct = (t->capacity > 0)
                          ? (100.0 * t->current_passengers / t->capacity)
                          : 0.0;

        /* Visual load bar (10 chars wide) */
        char bar[11];
        int filled = (int)(load_pct / 10.0);
        int j;
        for (j = 0; j < 10; j++) bar[j] = (j < filled) ? '#' : '.';
        bar[10] = '\0';

        printf("║ %-2s ║ %-20s ║ %3d ║ %3d min ║ %3d/%3d ║ [%s]%3.0f%% ║\n",
               t->train_id,
               t->route_name,
               t->capacity,
               t->frequency_min,
               t->current_passengers,
               t->capacity,
               bar,
               load_pct);
    }

    printf("╚════╩══════════════════════╩═════╩═══════╩════════╩══════════╝\n");
    printf("  Total trains: %d\n\n", network->train_count);
}

/* ----------------------------------------------------------------
 * displayTrainRoutes
 * ---------------------------------------------------------------- */
void displayTrainRoutes(const TrainNetwork *network, const CityGraph *graph) {
    if (network == NULL || graph == NULL) return;

    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                TRAIN ROUTES — STOP SEQUENCES                  \n");
    printf("════════════════════════════════════════════════════════════════\n");

    int i;
    for (i = 0; i < network->train_count; i++) {
        const Train *t = &network->trains[i];

        printf("\n  ┌─────────────────────────────────────────────┐\n");
        printf("  │  Train %-3s │ %-26s │\n", t->train_id, t->route_name);
        printf("  │  Capacity: %-3d pax  │  Every %-3d min          │\n",
               t->capacity, t->frequency_min);
        printf("  ├─────────────────────────────────────────────┤\n");

        int s;
        for (s = 0; s < t->num_stops; s++) {
            int loc_id = t->stops[s];
            const char *loc_name =
                (loc_id >= 0 && loc_id < graph->location_count)
                    ? graph->locations[loc_id].name
                    : "Unknown";

            if (s == 0) {
                printf("  │  [START] [%d] %-31s│\n", loc_id, loc_name);
            } else if (s == t->num_stops - 1 &&
                       t->stops[s] == t->stops[0]) {
                /* Circular route */
                printf("  │     |                                         │\n");
                printf("  │  [LOOP]  [%d] %-31s│\n", loc_id, loc_name);
            } else if (s == t->num_stops - 1) {
                printf("  │     |                                         │\n");
                printf("  │  [ END ] [%d] %-31s│\n", loc_id, loc_name);
            } else {
                printf("  │     |                                         │\n");
                printf("  │  [STOP%d] [%d] %-31s│\n", s, loc_id, loc_name);
            }
        }
        printf("  └─────────────────────────────────────────────┘\n");
    }

    printf("\n════════════════════════════════════════════════════════════════\n\n");
}

/* ----------------------------------------------------------------
 * getTrainCapacity
 * ---------------------------------------------------------------- */
int getTrainCapacity(const TrainNetwork *network, int train_index) {
    if (network == NULL || train_index < 0 ||
        train_index >= network->train_count) {
        fprintf(stderr,
                "[Train] ERROR: Invalid train index %d in getTrainCapacity.\n",
                train_index);
        return -1;
    }
    return network->trains[train_index].capacity;
}

/* ----------------------------------------------------------------
 * resetTrainPassengers
 * ---------------------------------------------------------------- */
void resetTrainPassengers(TrainNetwork *network) {
    if (network == NULL) return;
    int i;
    for (i = 0; i < network->train_count; i++) {
        network->trains[i].current_passengers = 0;
    }
    printf("[Train] All train passenger counts reset to 0.\n");
}

/* ================================================================
 * LEGACY PLACEHOLDER STUBS
 * ================================================================ */

void init_transport_network(CityGraph *graph) {
    (void)graph;
    printf("[Transport] Use initializeCity() instead.\n");
}
void add_bus_route(CityGraph *graph, int route_id, const char *name,
                   int capacity, int frequency) {
    (void)graph; (void)route_id; (void)name;
    (void)capacity; (void)frequency;
}
void add_train_route(CityGraph *graph, int route_id, const char *name,
                     int capacity, int frequency) {
    (void)graph; (void)route_id; (void)name;
    (void)capacity; (void)frequency;
}
void display_transport_network(void) {
    printf("[Transport] Use displayCityNetwork() instead.\n");
}

