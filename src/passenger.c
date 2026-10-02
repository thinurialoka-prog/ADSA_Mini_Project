/* ================================================================
 * passenger.c – Passenger journey simulation module
 *
 * Implements passenger creation, random generator, journey simulation
 * using Dijkstra shortest path, and journey statistics calculation.
 * ================================================================ */

#include "passenger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to format hour into 12-hour AM/PM string */
static void formatDepartureTime(int hour, char *buf, size_t buf_size) {
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;
    const char *ampm = (hour >= 12) ? "PM" : "AM";
    snprintf(buf, buf_size, "%02d:00 %s", display_hour, ampm);
}

/* ================================================================
 * createPassenger
 * ================================================================ */

Passenger createPassenger(int id, int start, int dest, int departure_hour) {
    Passenger p;
    p.passenger_id = id;
    snprintf(p.id_str, sizeof(p.id_str), "P%03d", id);
    p.start_location = start;
    p.destination_location = dest;
    p.departure_hour = (departure_hour >= 0 && departure_hour < 24) ? departure_hour : 8;
    p.total_distance = 0.0;
    p.total_travel_time = 0.0;
    p.total_waiting_time = 0.0;
    p.total_fare = 0.0;
    p.number_of_transfers = 0;
    p.successful = 0;
    return p;
}

/* ================================================================
 * generateRandomPassenger
 * ================================================================ */

Passenger generateRandomPassenger(int id, int max_locations) {
    if (max_locations <= 1) max_locations = 2;
    int start = rand() % max_locations;
    int dest = rand() % max_locations;
    while (dest == start) {
        dest = rand() % max_locations;
    }
    int dep_hour = 7 + (rand() % 14); /* 07:00 to 20:00 */
    return createPassenger(id, start, dest, dep_hour);
}

/* ================================================================
 * displayPassenger
 * ================================================================ */

void displayPassenger(const Passenger *p, const CityGraph *graph) {
    if (p == NULL || graph == NULL) return;
    char time_str[16];
    formatDepartureTime(p->departure_hour, time_str, sizeof(time_str));
    printf("Passenger %s\n", p->id_str);
    printf("Start       : %s [%d]\n", graph->locations[p->start_location].name, p->start_location);
    printf("Destination : %s [%d]\n", graph->locations[p->destination_location].name, p->destination_location);
    printf("Departure   : %s\n", time_str);
}

/* ================================================================
 * simulatePassengerJourney
 * ================================================================ */

int simulatePassengerJourney(Passenger *p, const CityGraph *graph, OptimizeMode mode) {
    if (p == NULL || graph == NULL) return 0;

    PathResult result;
    if (!findFastestRoute(graph, p->start_location, p->destination_location, mode, &result)) {
        p->successful = 0;
        printf("\n[!] Journey simulation failed: No path found between %s and %s.\n",
               graph->locations[p->start_location].name,
               graph->locations[p->destination_location].name);
        return 0;
    }

    p->successful = 1;
    p->total_distance = result.total_distance;
    p->total_travel_time = result.total_time;
    p->total_fare = result.total_fare;

    /* Count transport transfers (when mode switches between consecutive legs) */
    int transfers = 0;
    for (int i = 1; i < result.path_len - 1; i++) {
        if (result.transport_used[i] != result.transport_used[i - 1]) {
            transfers++;
        }
    }
    p->number_of_transfers = transfers;

    /* Base wait time at origin station (5 min) + 5 min per transfer */
    p->total_waiting_time = 5.0 + (transfers * 5.0);

    /* Format departure time */
    char time_str[16];
    formatDepartureTime(p->departure_hour, time_str, sizeof(time_str));

    printf("\n");
    printf("===============================================================\n");
    printf("                  PASSENGER JOURNEY SIMULATION                 \n");
    printf("===============================================================\n");
    printf("Passenger %s\n\n", p->id_str);
    printf("Start:\n%s\n\n", graph->locations[p->start_location].name);
    printf("Destination:\n%s\n\n", graph->locations[p->destination_location].name);
    printf("Departure:\n%s\n\n", time_str);
    printf("Journey:\n\n");

    /* Step-by-step vertical flow graph matching the requested format */
    for (int i = 0; i < result.path_len; i++) {
        int loc_id = result.path[i];
        printf("%s\n", graph->locations[loc_id].name);
        if (i < result.path_len - 1) {
            const char *mode_str = (result.transport_used[i] == TRANSPORT_BUS) ? "BUS" : "TRAIN";
            printf("   |\n");
            printf(" %-5s\n", mode_str);
            printf("   ↓\n");
        }
    }

    double total_journey_time = p->total_travel_time + p->total_waiting_time;

    printf("\nCalculated Statistics:\n");
    printf("---------------------------------------------------------------\n");
    printf("  Total Distance     : %7.2f km\n", p->total_distance);
    printf("  In-Vehicle Time    : %7.2f min\n", p->total_travel_time);
    printf("  Waiting Time       : %7.2f min\n", p->total_waiting_time);
    printf("  Total Journey Time : %7.2f min  (%.1f hrs)\n",
           total_journey_time, total_journey_time / 60.0);
    printf("  Total Fare         : $%6.2f\n", p->total_fare);
    printf("  Transfers          : %d transfer(s)\n", p->number_of_transfers);
    printf("  Status             : SUCCESS\n");
    printf("---------------------------------------------------------------\n\n");

    return 1;
}

/* ================================================================
 * passengerMenu – Interactive menu for passenger testing
 * ================================================================ */

void passengerMenu(const CityGraph *graph) {
    if (graph == NULL) return;

    printf("\n");
    printf("  ======================================== \n");
    printf("            PASSENGER JOURNEY SIMULATOR            \n");
    printf("  ======================================== \n");
    printf("   1. Run Standard Demo (P001: Residential -> Uni)     \n");
    printf("   2. Create Custom Passenger Journey                  \n");
    printf("   3. Generate & Simulate Random Passenger             \n");
    printf("  ======================================== \n");
    printf("  Enter choice (1-3): ");

    int choice = 1;
    if (scanf("%d", &choice) != 1) choice = 1;

    if (choice == 1) {
        /* Standard demo as requested in prompt: Residential Area (1) -> University (0), 07:00 AM */
        Passenger p = createPassenger(1, 1, 0, 7);
        simulatePassengerJourney(&p, graph, OPTIMIZE_TIME);
    } else if (choice == 2) {
        displayLocations(graph);
        int start = 0, dest = 0, hour = 8;
        printf("  Enter Start Location ID (0-%d): ", graph->location_count - 1);
        if (scanf("%d", &start) != 1) start = 0;
        printf("  Enter Destination ID    (0-%d): ", graph->location_count - 1);
        if (scanf("%d", &dest) != 1) dest = 1;
        printf("  Enter Departure Hour    (0-23): ");
        if (scanf("%d", &hour) != 1) hour = 8;

        Passenger p = createPassenger(101, start, dest, hour);
        simulatePassengerJourney(&p, graph, OPTIMIZE_TIME);
    } else if (choice == 3) {
        Passenger p = generateRandomPassenger(rand() % 900 + 100, graph->location_count);
        simulatePassengerJourney(&p, graph, OPTIMIZE_TIME);
    }
}

/* ================================================================
 * Legacy demand module stubs
 * ================================================================ */

void init_passenger_demands(void) {
    printf("[Passenger] Initializing passenger demand structures.\n");
}

void set_time_of_day_multiplier(TimeOfDay time) {
    (void)time;
}

void generate_variable_demand(TimeOfDay time_of_day, int total_passengers) {
    (void)time_of_day;
    (void)total_passengers;
    printf("[Passenger] Generating variable demand for time of day.\n");
}

void print_passenger_summary(void) {
    printf("[Passenger] Summary of passenger demand generated.\n");
}
