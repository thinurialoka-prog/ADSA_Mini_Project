#ifndef PROFILING_H
#define PROFILING_H

#include <time.h>
#include "graph.h"
#include "simulation.h"

/* ================================================================
 * SYSTEM STATS STRUCTURE
 *
 *  Stores raw tracked statistics and dynamically computed performance
 *  ratios for the entire smart city transportation network.
 * ================================================================ */

typedef struct {
    /* Raw tracked counters */
    int    total_passengers;               /* Total passenger trip requests    */
    int    successful_journeys;            /* Completed trips                  */
    int    failed_journeys;                /* Abandoned / unreachable trips    */
    double total_travel_time;              /* In-vehicle travel time (min)     */
    double total_waiting_time;             /* Total wait time (min)            */
    double total_distance;                 /* Total distance traveled (km)     */
    double total_fare;                     /* Total revenue collected ($)      */
    int    bus_passengers;                 /* Passengers using bus             */
    int    train_passengers;               /* Passengers using train           */
    int    total_transfers;                /* Total mode transfers             */
    int    peak_hour;                      /* Hour of maximum demand (0-23)    */
    int    peak_hour_demand;               /* Passenger count in peak hour     */
    double total_available_bus_capacity;   /* Offered bus seats                */
    double total_used_bus_capacity;        /* Used bus seats (leg boardings)   */
    double total_available_train_capacity; /* Offered train seats              */
    double total_used_train_capacity;      /* Used train seats (leg boardings) */

    /* Calculated performance ratios */
    double success_rate;                   /* (successful / total) * 100       */
    double avg_travel_time;                /* travel_time / successful         */
    double avg_waiting_time;               /* waiting_time / total_passengers  */
    double avg_distance;                   /* total_distance / successful      */
    double bus_utilization;                /* (used_bus / avail_bus) * 100     */
    double train_utilization;              /* (used_train / avail_train) * 100 */
} SystemStats;

/* Execution timer struct */
typedef struct {
    clock_t start_time;
    clock_t end_time;
    double  execution_duration_sec;
} ProfileTimer;

/* ================================================================
 * PUBLIC API
 * ================================================================ */

/* Timer utilities */
void start_profiling_timer(ProfileTimer *timer);
void stop_profiling_timer(ProfileTimer *timer);

/* Compute SystemStats dynamically from simulation summary */
SystemStats computeSystemStats(const FullDaySimulationSummary *sim_summary);

/* Display full professional profiling report with metric explanations */
void displayProfilingReport(const SystemStats *stats, double execution_time_sec);

/* Run full profiling suite (runs full-day simulation, benchmarks time, prints report) */
void runFullProfiling(const CityGraph *graph);

/* Legacy stub */
void report_system_efficiency(double total_passengers, double avg_travel_time, double execution_time);

#endif /* PROFILING_H */
