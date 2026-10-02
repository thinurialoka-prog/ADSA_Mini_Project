/* ================================================================
 * profiling.c – Smart City Transport System Profiling Module
 *
 * Computes dynamic system-wide performance statistics, benchmarks
 * execution runtime using ProfileTimer, calculates key performance
 * ratios (Success Rate, Avg Travel/Wait Time, Bus/Train Utilization),
 * and generates a professional profiling report with explanations.
 * ================================================================ */

#include "profiling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper to format hour into AM/PM string */
static void formatHourAMPM(int hour, char *buf, size_t buf_size) {
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;
    const char *ampm = (hour >= 12) ? "PM" : "AM";
    snprintf(buf, buf_size, "%02d:00 %s", display_hour, ampm);
}

/* ================================================================
 * TIMER FUNCTIONS
 * ================================================================ */

void start_profiling_timer(ProfileTimer *timer) {
    if (timer != NULL) {
        timer->start_time = clock();
    }
}

void stop_profiling_timer(ProfileTimer *timer) {
    if (timer != NULL) {
        timer->end_time = clock();
        timer->execution_duration_sec = (double)(timer->end_time - timer->start_time) / CLOCKS_PER_SEC;
    }
}

/* ================================================================
 * computeSystemStats
 * ================================================================ */

SystemStats computeSystemStats(const FullDaySimulationSummary *sim_summary) {
    SystemStats stats;
    memset(&stats, 0, sizeof(stats));

    if (sim_summary == NULL) return stats;

    /* Raw tracked counters */
    stats.total_passengers    = sim_summary->total_passengers_generated;
    stats.successful_journeys = sim_summary->total_successful_journeys;
    stats.failed_journeys     = sim_summary->total_failed_journeys;
    stats.total_travel_time   = sim_summary->total_travel_time;
    stats.total_waiting_time  = sim_summary->total_waiting_time;
    stats.total_distance      = sim_summary->total_distance;
    stats.total_fare          = sim_summary->total_fare;
    stats.bus_passengers      = sim_summary->total_bus_passengers;
    stats.train_passengers    = sim_summary->total_train_passengers;
    stats.total_transfers     = sim_summary->total_transfers;

    /* Find Peak Hour & Peak Demand */
    int h;
    int max_demand = 0;
    int peak_h = 8;
    for (h = 6; h <= 23; h++) {
        int demand = sim_summary->hourly_results[h].passengers_generated;
        if (demand > max_demand) {
            max_demand = demand;
            peak_h = h;
        }
    }
    stats.peak_hour        = peak_h;
    stats.peak_hour_demand = max_demand;

    /* Available Capacity during 18 operating hours (06:00 to 23:00) */
    /* 18 hrs * 10 bus directions * (60 / 10) runs/hr * 60 seats/bus = 64,800 seats */
    stats.total_available_bus_capacity = 18.0 * 10.0 * (60.0 / SIM_BUS_FREQUENCY_MIN) * SIM_BUS_CAPACITY;
    /* 18 hrs * 6 train directions * (60 / 15) runs/hr * 800 seats/train = 345,600 seats */
    stats.total_available_train_capacity = 18.0 * 6.0 * (60.0 / SIM_TRAIN_FREQUENCY_MIN) * SIM_TRAIN_CAPACITY;

    /* Used Capacity (passenger leg boardings) */
    stats.total_used_bus_capacity   = (double)sim_summary->total_bus_passengers;
    stats.total_used_train_capacity = (double)sim_summary->total_train_passengers;

    /* Calculate Performance Ratios dynamically */
    stats.success_rate = (stats.total_passengers > 0)
        ? ((double)stats.successful_journeys / (double)stats.total_passengers * 100.0)
        : 0.0;

    stats.avg_travel_time = (stats.successful_journeys > 0)
        ? (stats.total_travel_time / (double)stats.successful_journeys)
        : 0.0;

    stats.avg_waiting_time = (stats.total_passengers > 0)
        ? (stats.total_waiting_time / (double)stats.total_passengers)
        : 0.0;

    stats.avg_distance = (stats.successful_journeys > 0)
        ? (stats.total_distance / (double)stats.successful_journeys)
        : 0.0;

    stats.bus_utilization = (stats.total_available_bus_capacity > 0)
        ? (stats.total_used_bus_capacity / stats.total_available_bus_capacity * 100.0)
        : 0.0;

    stats.train_utilization = (stats.total_available_train_capacity > 0)
        ? (stats.total_used_train_capacity / stats.total_available_train_capacity * 100.0)
        : 0.0;

    return stats;
}

/* ================================================================
 * displayProfilingReport
 * ================================================================ */

void displayProfilingReport(const SystemStats *stats, double execution_time_sec) {
    if (stats == NULL) return;

    char peak_str[16];
    formatHourAMPM(stats->peak_hour, peak_str, sizeof(peak_str));

    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║       SMART CITY TRANSPORT SYSTEM — SYSTEM PROFILING REPORT          ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  Algorithm Execution Time  : %9.6f seconds                         ║\n", execution_time_sec);
    printf("  ║  Simulated Operating Hours : 18 hours (06:00 AM - 11:00 PM)          ║\n");
    printf("  ║  Peak Operating Hour       : %-9s (%d passengers/hr)       ║\n", peak_str, stats->peak_hour_demand);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  RAW SYSTEM COUNTERS                                                 ║\n");
    printf("  ║  Total Passenger Demand    : %7d passengers                      ║\n", stats->total_passengers);
    printf("  ║  Successful Journeys       : %7d passengers                      ║\n", stats->successful_journeys);
    printf("  ║  Failed / Abandoned        : %7d passengers                      ║\n", stats->failed_journeys);
    printf("  ║  Total Distance Traveled   : %9.1f km                              ║\n", stats->total_distance);
    printf("  ║  Total Travel Time         : %9.1f min  (%6.1f hrs)              ║\n", stats->total_travel_time, stats->total_travel_time / 60.0);
    printf("  ║  Total Waiting Time        : %9.1f min  (%6.1f hrs)              ║\n", stats->total_waiting_time, stats->total_waiting_time / 60.0);
    printf("  ║  Total Fares Collected     : $%9.2f                                ║\n", stats->total_fare);
    printf("  ║  Total Bus Passenger Legs  : %7d passenger-legs                 ║\n", stats->bus_passengers);
    printf("  ║  Total Train Passenger Legs: %7d passenger-legs                 ║\n", stats->train_passengers);
    printf("  ║  Total Inter-Modal Transfers: %6d transfers                      ║\n", stats->total_transfers);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  VEHICLE FLEET CAPACITY & UTILIZATION                                ║\n");
    printf("  ║  Total Available Bus Cap   : %7.0f seats                          ║\n", stats->total_available_bus_capacity);
    printf("  ║  Total Used Bus Capacity   : %7.0f seats (leg boardings)          ║\n", stats->total_used_bus_capacity);
    printf("  ║  Total Available Train Cap : %7.0f seats                          ║\n", stats->total_available_train_capacity);
    printf("  ║  Total Used Train Capacity : %7.0f seats (leg boardings)          ║\n", stats->total_used_train_capacity);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  CALCULATED PERFORMANCE RATIOS                                       ║\n");
    printf("  ║  Success Rate              : %6.2f%% (successful / total * 100)      ║\n", stats->success_rate);
    printf("  ║  Average Travel Time       : %6.2f min (travel_time / successful)   ║\n", stats->avg_travel_time);
    printf("  ║  Average Waiting Time      : %6.2f min (waiting_time / total pax)   ║\n", stats->avg_waiting_time);
    printf("  ║  Average Distance Traveled : %6.2f km  (total_dist / successful)    ║\n", stats->avg_distance);
    printf("  ║  Bus Fleet Utilization     : %6.2f%% (used_bus / avail_bus * 100)   ║\n", stats->bus_utilization);
    printf("  ║  Train Fleet Utilization   : %6.2f%% (used_train / avail_train * 100)║\n", stats->train_utilization);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  METRIC DEFINITIONS & PERFORMANCE EXPLANATIONS                       ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║ 1. Success Rate (%.2f%%):                                          ║\n", stats->success_rate);
    printf("  ║    The percentage of passenger trip requests completed. Journeys     ║\n");
    printf("  ║    fail when heavy peak congestion forces queue waits > 30 minutes.  ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ║ 2. Average Travel Time (%.2f mins):                                 ║\n", stats->avg_travel_time);
    printf("  ║    Mean duration spent inside transit vehicles per completed trip.   ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ║ 3. Average Waiting Time (%.2f mins):                                ║\n", stats->avg_waiting_time);
    printf("  ║    Mean time passengers spend waiting at origin stations and         ║\n");
    printf("  ║    transfer hubs per total passenger trip request.                   ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ║ 4. Average Distance (%.2f km):                                       ║\n", stats->avg_distance);
    printf("  ║    Mean geographic route distance covered per completed trip.        ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ║ 5. Bus Utilization (%.2f%%):                                         ║\n", stats->bus_utilization);
    printf("  ║    Ratio of occupied bus seats to total offered bus seat capacity.   ║\n");
    printf("  ║                                                                      ║\n");
    printf("  ║ 6. Train Utilization (%.2f%%):                                       ║\n", stats->train_utilization);
    printf("  ║    Ratio of occupied train seats to total offered rail seat capacity.║\n");
    printf("  ╚══════════════════════════════════════════════════════════════════════╝\n\n");
}

/* ================================================================
 * runFullProfiling
 * ================================================================ */

void runFullProfiling(const CityGraph *graph) {
    if (graph == NULL) return;

    ProfileTimer timer;
    start_profiling_timer(&timer);

    FullDaySimulationSummary summary = runFullDaySimulation(graph);

    stop_profiling_timer(&timer);

    SystemStats stats = computeSystemStats(&summary);
    displayProfilingReport(&stats, timer.execution_duration_sec);
}

/* ================================================================
 * Legacy stub
 * ================================================================ */

void report_system_efficiency(double total_passengers, double avg_travel_time, double execution_time) {
    printf("\n==================================================\n");
    printf("     SMART CITY TRANSPORTATION SYSTEM PROFILING   \n");
    printf("==================================================\n");
    printf(" Total Passengers Simulated: %.0f\n", total_passengers);
    printf(" Average Travel Time       : %.2f mins\n", avg_travel_time);
    printf(" Algorithm Execution Time  : %.6f seconds\n", execution_time);
    printf("==================================================\n\n");
}
