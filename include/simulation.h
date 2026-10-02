#ifndef SIMULATION_H
#define SIMULATION_H

#include "graph.h"
#include "passenger.h"
#include "dijkstra.h"
#include "transport.h"

/* ================================================================
 * VEHICLE CAPACITY & FREQUENCY SIMULATION ASSUMPTIONS
 * ================================================================ */

#define SIM_BUS_CAPACITY          60   /* Max seats per bus            */
#define SIM_TRAIN_CAPACITY       800   /* Max seats per train          */
#define SIM_BUS_FREQUENCY_MIN     10   /* Bus headway in minutes       */
#define SIM_TRAIN_FREQUENCY_MIN   15   /* Train headway in minutes     */

/* ================================================================
 * DEMAND LEVEL ENUMERATION
 * ================================================================ */

typedef enum {
    DEMAND_LOW,       /* Off-peak / Night (Low volume: ~100-180)     */
    DEMAND_MEDIUM,    /* Daytime / Normal (Medium volume: ~200-350)  */
    DEMAND_VERY_HIGH  /* Peak commute   (Very High volume: ~500-600) */
} DemandLevel;

/* Information struct for 24-hour demand summary */
typedef struct {
    int         hour;                  /* 0 - 23                      */
    char        time_range[16];        /* e.g., "07:00 - 08:00"       */
    DemandLevel level;                 /* LOW, MEDIUM, VERY_HIGH      */
    int         passenger_count;       /* Total passengers generated  */
    int         morning_commute_count; /* Residential -> Work/Uni     */
    int         evening_commute_count; /* Work/Uni -> Residential     */
    int         other_travel_count;    /* General urban travel        */
} HourlyDemandInfo;

/* ================================================================
 * FULL-DAY SIMULATION HOURLY & DAILY RESULTS
 * ================================================================ */

typedef struct {
    int    hour;                     /* 6 to 23                       */
    char   time_str[16];             /* e.g. "08:00 AM"               */
    int    passengers_generated;     /* Total passengers generated    */
    int    successful_journeys;      /* Completed journeys            */
    int    failed_journeys;          /* Abandoned/unreachable journeys*/
    double total_travel_time;        /* In-vehicle travel time (min)  */
    double total_waiting_time;       /* Total wait time (min)         */
    double avg_travel_time;          /* Avg travel time per success   */
    double avg_waiting_time;         /* Avg wait time per success    */
    double total_distance;           /* Total km traveled             */
    double total_fare;               /* Total fare collected ($)      */
    int    bus_passengers;           /* Passengers taking bus         */
    int    train_passengers;         /* Passengers taking train       */
    int    total_transfers;          /* Total transfers made          */
} FullDayHourlyResult;

typedef struct {
    int                 total_passengers_generated;
    int                 total_successful_journeys;
    int                 total_failed_journeys;
    double              total_distance;
    double              total_travel_time;
    double              total_waiting_time;
    double              total_fare;
    double              avg_travel_time;
    double              avg_waiting_time;
    int                 total_bus_passengers;
    int                 total_train_passengers;
    int                 total_transfers;
    FullDayHourlyResult hourly_results[24];
} FullDaySimulationSummary;

/* Legacy capacity metrics struct */
typedef struct {
    int    hour;
    int    total_demand;
    int    passengers_served;
    int    passengers_waiting;
    double total_waiting_time_min;
    double avg_waiting_time_min;
    double total_travel_time_min;
    double total_offered_capacity;
    double vehicle_utilization_pct;
    int    bus_trips;
    int    train_trips;
    int    overflow_events;
} SimulationMetrics;

/* ================================================================
 * PUBLIC API
 * ================================================================ */

int getPassengerDemand(int hour);
DemandLevel getDemandLevel(int hour);
const char *getDemandLevelName(DemandLevel level);

Passenger generateWeightedPassenger(int id, int hour, int max_locations);
Passenger *generateHourlyPassengers(int hour, const CityGraph *graph, int *out_count);

void displayFullDayDemand(void);

/*
 * Run complete full-day transportation simulation from 06:00 AM to 11:00 PM.
 * Outputs hourly summary blocks and prints the final daily summary report.
 */
FullDaySimulationSummary runFullDaySimulation(const CityGraph *graph);

/* Legacy simulation functions */
SimulationMetrics runHourlyCapacitySimulation(const CityGraph *graph, int hour);
SimulationMetrics runFullDayCapacitySimulation(const CityGraph *graph);
void printSimulationMetricsReport(const SimulationMetrics *metrics, const char *title);

void simulationMenu(const CityGraph *graph);

void init_simulation(CityGraph *graph);
void run_simulation(TimeOfDay time_of_day);
SimulationMetrics get_simulation_results(void);

#endif /* SIMULATION_H */
