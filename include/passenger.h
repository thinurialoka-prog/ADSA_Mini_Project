#ifndef PASSENGER_H
#define PASSENGER_H

#include <stdbool.h>
#include "graph.h"
#include "dijkstra.h"

/* ================================================================
 * TIME OF DAY
 * ================================================================ */
typedef enum {
    TIME_MORNING_PEAK, /* 07:00 - 09:00 */
    TIME_AFTERNOON,    /* 12:00 - 14:00 */
    TIME_EVENING_PEAK, /* 17:00 - 19:00 */
    TIME_NIGHT         /* 21:00 - 23:00 */
} TimeOfDay;

/* ================================================================
 * PASSENGER STRUCTURE
 * ================================================================ */
typedef struct {
    int    passenger_id;         /* Numeric ID (e.g. 1)             */
    char   id_str[16];           /* Formatted ID (e.g. "P001")       */
    int    start_location;       /* Origin location ID              */
    int    destination_location; /* Destination location ID         */
    int    departure_hour;       /* Hour of departure (0-23)        */
    double total_distance;       /* Total travel distance (km)      */
    double total_travel_time;    /* In-vehicle travel time (min)    */
    double total_waiting_time;   /* Initial + transfer wait (min)   */
    double total_fare;           /* Total ticket fare ($)           */
    int    number_of_transfers;  /* Count of transport transfers    */
    int    successful;           /* 1 if reached destination, 0 else*/
} Passenger;

/* Legacy demand structure for Part III full-day simulation */
typedef struct {
    int       passenger_id;
    int       origin_station;
    int       destination_station;
    TimeOfDay departure_time;
    double    travel_delay;
} PassengerDemand;

/* ================================================================
 * PUBLIC API
 * ================================================================ */

/*
 * Create and return a Passenger instance with given parameters.
 */
Passenger createPassenger(int id, int start, int dest, int departure_hour);

/*
 * Generate a Passenger with random start/destination locations and departure hour.
 */
Passenger generateRandomPassenger(int id, int max_locations);

/*
 * Print basic details of a passenger.
 */
void displayPassenger(const Passenger *p, const CityGraph *graph);

/*
 * Simulate a passenger journey through the city graph using Dijkstra's shortest path.
 * Computes distance, travel time, waiting time, fare, and number of transfers.
 * Displays step-by-step journey visualization.
 * Returns 1 on success, 0 on failure.
 */
int simulatePassengerJourney(Passenger *p, const CityGraph *graph, OptimizeMode mode);

/*
 * Interactive CLI menu for testing passenger journeys.
 */
void passengerMenu(const CityGraph *graph);

/* Legacy stubs for demand module */
void init_passenger_demands(void);
void set_time_of_day_multiplier(TimeOfDay time);
void generate_variable_demand(TimeOfDay time_of_day, int total_passengers);
void print_passenger_summary(void);

#endif /* PASSENGER_H */
