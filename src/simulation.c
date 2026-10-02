/* ================================================================
 * simulation.c – Complete full-day smart city transport simulation
 *
 * Simulates city transit from 06:00 AM to 11:00 PM (18 operating hours).
 * For each hour:
 *   1. Determines passenger demand
 *   2. Generates weighted passengers
 *   3. Selects origins and destinations
 *   4. Computes fastest routes via Dijkstra
 *   5. Determines bus, train, or multi-modal transit
 *   6. Calculates distance, travel time, fare, and wait times
 *   7. Evaluates capacity limits
 *   8. Tracks successful vs failed journeys and transfers
 *   9. Prints hourly summary blocks and final daily summary report.
 * ================================================================ */

#include "simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Track whether random generator seed has been initialized */
static int g_rand_seeded = 0;

static void ensureRandomSeeded(void) {
    if (!g_rand_seeded) {
        srand((unsigned int)time(NULL));
        g_rand_seeded = 1;
    }
}

/* Helper to format hour into 12-hour AM/PM string */
static void formatHourAMPM(int hour, char *buf, size_t buf_size) {
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;
    const char *ampm = (hour >= 12) ? "PM" : "AM";
    snprintf(buf, buf_size, "%02d:00 %s", display_hour, ampm);
}

/* ================================================================
 * 24-Hour Demand Assumption Table
 * ================================================================ */

static const int HOURLY_DEMAND[24] = {
    40,  /* 00:00 - Low       */
    30,  /* 01:00 - Low       */
    20,  /* 02:00 - Low       */
    20,  /* 03:00 - Low       */
    30,  /* 04:00 - Low       */
    60,  /* 05:00 - Low       */
    300, /* 06:00 - Medium    */
    500, /* 07:00 - Very High (Assumed: 500) */
    600, /* 08:00 - Very High (Assumed: 600) */
    350, /* 09:00 - Medium    */
    200, /* 10:00 - Medium    (Assumed: 200) */
    250, /* 11:00 - Medium    */
    300, /* 12:00 - Medium    */
    250, /* 13:00 - Medium    */
    150, /* 14:00 - Low       (Assumed: 150) */
    160, /* 15:00 - Low       */
    180, /* 16:00 - Low       */
    550, /* 17:00 - Very High */
    600, /* 18:00 - Very High (Assumed: 600) */
    400, /* 19:00 - Medium    */
    350, /* 20:00 - Medium    */
    300, /* 21:00 - Medium    (Assumed: 300) */
    150, /* 22:00 - Low       */
    100  /* 23:00 - Low       (Assumed: 100) */
};

/* ================================================================
 * getPassengerDemand
 * ================================================================ */

int getPassengerDemand(int hour) {
    if (hour < 0 || hour >= 24) return 0;
    return HOURLY_DEMAND[hour];
}

/* ================================================================
 * getDemandLevel & getDemandLevelName
 * ================================================================ */

DemandLevel getDemandLevel(int hour) {
    if ((hour >= 7 && hour < 9) || (hour >= 17 && hour < 19)) {
        return DEMAND_VERY_HIGH;
    } else if ((hour >= 6 && hour < 7) || (hour >= 9 && hour < 14) || (hour >= 19 && hour < 22)) {
        return DEMAND_MEDIUM;
    } else {
        return DEMAND_LOW;
    }
}

const char *getDemandLevelName(DemandLevel level) {
    switch (level) {
        case DEMAND_VERY_HIGH: return "Very High";
        case DEMAND_MEDIUM:    return "Medium   ";
        case DEMAND_LOW:       return "Low      ";
        default:               return "Unknown  ";
    }
}

/* ================================================================
 * generateWeightedPassenger
 * ================================================================ */

Passenger generateWeightedPassenger(int id, int hour, int max_locations) {
    ensureRandomSeeded();

    int start = 1; /* Default: Residential Area */
    int dest  = 0; /* Default: University */

    if (max_locations < 10) max_locations = 10;

    /* Morning peak: 07:00 - 09:00 (hours 7 & 8) */
    if (hour >= 7 && hour < 9) {
        int r = rand() % 100;
        if (r < 75) {
            start = 1;
            int target_r = rand() % 3;
            if (target_r == 0)      dest = 0; /* University */
            else if (target_r == 1) dest = 8; /* Industrial Area */
            else                    dest = 6; /* City Center */
        } else {
            start = rand() % max_locations;
            dest  = rand() % max_locations;
            while (dest == start) dest = rand() % max_locations;
        }
    }
    /* Evening peak: 17:00 - 19:00 (hours 17 & 18) */
    else if (hour >= 17 && hour < 19) {
        int r = rand() % 100;
        if (r < 75) {
            dest = 1;
            int origin_r = rand() % 3;
            if (origin_r == 0)      start = 0; /* University */
            else if (origin_r == 1) start = 8; /* Industrial Area */
            else                    start = 6; /* City Center */
        } else {
            start = rand() % max_locations;
            dest  = rand() % max_locations;
            while (dest == start) dest = rand() % max_locations;
        }
    }
    /* Daytime / Midday: 09:00 - 17:00 */
    else if (hour >= 9 && hour < 17) {
        int r = rand() % 100;
        if (r < 50) {
            int hubs[5] = {5, 4, 6, 2, 3};
            start = rand() % max_locations;
            dest  = hubs[rand() % 5];
            while (dest == start) dest = rand() % max_locations;
        } else {
            start = rand() % max_locations;
            dest  = rand() % max_locations;
            while (dest == start) dest = rand() % max_locations;
        }
    }
    /* Night / Off-peak: 19:00 - 06:00 */
    else {
        start = rand() % max_locations;
        dest  = rand() % max_locations;
        while (dest == start) dest = rand() % max_locations;
    }

    return createPassenger(id, start, dest, hour);
}

/* ================================================================
 * generateHourlyPassengers
 * ================================================================ */

Passenger *generateHourlyPassengers(int hour, const CityGraph *graph, int *out_count) {
    if (graph == NULL || out_count == NULL) return NULL;

    int demand = getPassengerDemand(hour);
    *out_count = demand;

    if (demand <= 0) return NULL;

    Passenger *passengers = malloc(demand * sizeof(Passenger));
    if (passengers == NULL) {
        fprintf(stderr, "[!] Memory allocation failed for passenger demand.\n");
        *out_count = 0;
        return NULL;
    }

    int i;
    for (i = 0; i < demand; i++) {
        passengers[i] = generateWeightedPassenger(i + 1, hour, graph->location_count);
    }

    return passengers;
}

/* ================================================================
 * displayFullDayDemand
 * ================================================================ */

void displayFullDayDemand(void) {
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║             24-HOUR VARIABLE PASSENGER DEMAND PROFILE               ║\n");
    printf("  ╠══════╦═══════════════╦═══════════╦════════════╦══════════════════════╣\n");
    printf("  ║ Hour ║ Time Range    ║ Demand    ║ Passengers ║ Primary Flow Pattern ║\n");
    printf("  ╠══════╬═══════════════╬═══════════╬════════════╬══════════════════════╣\n");

    int total_passengers = 0;
    int hour;
    for (hour = 0; hour < 24; hour++) {
        int demand = getPassengerDemand(hour);
        DemandLevel lvl = getDemandLevel(hour);
        total_passengers += demand;

        char time_range[16];
        snprintf(time_range, sizeof(time_range), "%02d:00 - %02d:00", hour, (hour + 1) % 24);

        const char *pattern = "General / Off-peak";
        if (hour >= 7 && hour < 9) {
            pattern = "Residential -> Work/Uni (Peak)";
        } else if (hour >= 17 && hour < 19) {
            pattern = "Work/Uni -> Residential (Peak)";
        } else if (hour >= 12 && hour < 14) {
            pattern = "Midday Commercial Hubs";
        }

        char bar[16];
        int bar_len = demand / 40;
        if (bar_len > 15) bar_len = 15;
        int b;
        for (b = 0; b < bar_len; b++) bar[b] = '#';
        bar[bar_len] = '\0';

        printf("  ║  %02d  ║ %-13s ║ %-9s ║    %4d    ║ %-20s ║ %-15s\n",
               hour, time_range, getDemandLevelName(lvl), demand, pattern, bar);
    }

    printf("  ╠══════╩═══════════════╩═══════════╩════════════╩══════════════════════╣\n");
    printf("  ║  TOTAL DAILY GENERATED PASSENGERS : %5d passengers                   ║\n",
           total_passengers);
    printf("  ╚══════════════════════════════════════════════════════════════════════╝\n\n");
}

/* ================================================================
 * runFullDaySimulation  (06:00 AM to 11:00 PM)
 * ================================================================ */

FullDaySimulationSummary runFullDaySimulation(const CityGraph *graph) {
    FullDaySimulationSummary summary;
    memset(&summary, 0, sizeof(summary));

    if (graph == NULL) return summary;

    printf("\n");
    printf("================================================================\n");
    printf("     STARTING FULL-DAY SMART CITY TRANSPORT SIMULATION          \n");
    printf("                (06:00 AM to 11:00 PM)                          \n");
    printf("================================================================\n");

    int h;
    for (h = 6; h <= 23; h++) {
        FullDayHourlyResult *hr = &summary.hourly_results[h];
        hr->hour = h;
        formatHourAMPM(h, hr->time_str, sizeof(hr->time_str));

        int demand = 0;
        Passenger *passengers = generateHourlyPassengers(h, graph, &demand);
        hr->passengers_generated = demand;

        if (demand == 0 || passengers == NULL) {
            printf("\n========================================\n");
            printf("%s\n", hr->time_str);
            printf("========================================\n");
            printf("Passengers generated : %d\n", 0);
            printf("Successful journeys  : %d\n", 0);
            printf("Failed journeys      : %d\n", 0);
            printf("Average travel time  : 0.0 minutes\n");
            printf("Average waiting time : 0.0 minutes\n");
            printf("Bus passengers       : %d\n", 0);
            printf("Train passengers     : %d\n", 0);
            printf("========================================\n");
            continue;
        }

        /* Edge capacity queue counters */
        int current_bus_pos[MAX_LOCATIONS][MAX_LOCATIONS];
        int current_train_pos[MAX_LOCATIONS][MAX_LOCATIONS];
        memset(current_bus_pos, 0, sizeof(current_bus_pos));
        memset(current_train_pos, 0, sizeof(current_train_pos));

        int i;
        for (i = 0; i < demand; i++) {
            PathResult res;
            findFastestRoute(graph,
                             passengers[i].start_location,
                             passengers[i].destination_location,
                             OPTIMIZE_TIME,
                             &res);

            if (!res.reachable) {
                hr->failed_journeys++;
                continue;
            }

            int transfers = 0;
            int uses_bus = 0;
            int uses_train = 0;
            int max_overflow_cycles = 0;

            int leg;
            for (leg = 0; leg < res.path_len - 1; leg++) {
                int u = res.path[leg];
                int v = res.path[leg + 1];

                if (leg > 0 && res.transport_used[leg] != res.transport_used[leg - 1]) {
                    transfers++;
                }

                if (res.transport_used[leg] == TRANSPORT_BUS) {
                    uses_bus = 1;
                    int pos = current_bus_pos[u][v]++;
                    int cycle = pos / SIM_BUS_CAPACITY;
                    if (cycle > max_overflow_cycles) max_overflow_cycles = cycle;
                } else {
                    uses_train = 1;
                    int pos = current_train_pos[u][v]++;
                    int cycle = pos / SIM_TRAIN_CAPACITY;
                    if (cycle > max_overflow_cycles) max_overflow_cycles = cycle;
                }
            }

            double initial_wait = 5.0;
            double transfer_wait = transfers * 5.0;
            double overflow_wait = max_overflow_cycles * 10.0;
            double pax_wait_time = initial_wait + transfer_wait + overflow_wait;

            /*
             * Success vs Failure Threshold:
             * If peak congestion forces wait time > 30 minutes (3 vehicle cycles),
             * the passenger abandons the journey (failed journey).
             */
            if (max_overflow_cycles >= 3) {
                hr->failed_journeys++;
            } else {
                hr->successful_journeys++;
                hr->total_travel_time += res.total_time;
                hr->total_waiting_time += pax_wait_time;
                hr->total_distance += res.total_distance;
                hr->total_fare += res.total_fare;
                hr->total_transfers += transfers;

                if (uses_bus) hr->bus_passengers++;
                if (uses_train) hr->train_passengers++;
            }
        }

        if (hr->successful_journeys > 0) {
            hr->avg_travel_time = hr->total_travel_time / hr->successful_journeys;
            hr->avg_waiting_time = hr->total_waiting_time / hr->successful_journeys;
        }

        /* Print exact hourly summary block as requested */
        printf("\n========================================\n");
        printf("%s\n", hr->time_str);
        printf("========================================\n");
        printf("Passengers generated : %d\n", hr->passengers_generated);
        printf("Successful journeys  : %d\n", hr->successful_journeys);
        printf("Failed journeys      : %d\n", hr->failed_journeys);
        printf("Average travel time  : %.1f minutes\n", hr->avg_travel_time);
        printf("Average waiting time : %.1f minutes\n", hr->avg_waiting_time);
        printf("Bus passengers       : %d\n", hr->bus_passengers);
        printf("Train passengers     : %d\n", hr->train_passengers);
        printf("========================================\n");

        /* Accumulate daily totals */
        summary.total_passengers_generated += hr->passengers_generated;
        summary.total_successful_journeys  += hr->successful_journeys;
        summary.total_failed_journeys      += hr->failed_journeys;
        summary.total_distance             += hr->total_distance;
        summary.total_travel_time          += hr->total_travel_time;
        summary.total_waiting_time         += hr->total_waiting_time;
        summary.total_fare                 += hr->total_fare;
        summary.total_bus_passengers       += hr->bus_passengers;
        summary.total_train_passengers     += hr->train_passengers;
        summary.total_transfers            += hr->total_transfers;

        free(passengers);
    }

    if (summary.total_successful_journeys > 0) {
        summary.avg_travel_time  = summary.total_travel_time / summary.total_successful_journeys;
        summary.avg_waiting_time = summary.total_waiting_time / summary.total_successful_journeys;
    }

    /* Print complete daily summary report */
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("  ║          COMPLETE FULL-DAY SIMULATION SUMMARY REPORT                 ║\n");
    printf("  ║                  (06:00 AM – 11:00 PM Operating Window)              ║\n");
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  Total Operating Hours     : 18 hours (06:00 AM - 11:00 PM)         ║\n");
    printf("  ║  Total Demand Generated    : %6d passengers                      ║\n",
           summary.total_passengers_generated);
    printf("  ║  Successful Journeys       : %6d passengers (%5.1f%% completion) ║\n",
           summary.total_successful_journeys,
           (summary.total_passengers_generated > 0) ? (100.0 * summary.total_successful_journeys / summary.total_passengers_generated) : 0.0);
    printf("  ║  Failed / Abandoned        : %6d passengers (%5.1f%% failure)    ║\n",
           summary.total_failed_journeys,
           (summary.total_passengers_generated > 0) ? (100.0 * summary.total_failed_journeys / summary.total_passengers_generated) : 0.0);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  JOURNEY METRICS & TIME TOTALS                                       ║\n");
    printf("  ║  Total Distance Traveled   : %9.1f km                              ║\n",
           summary.total_distance);
    printf("  ║  Total In-Vehicle Time     : %9.1f min  (%6.1f hrs)              ║\n",
           summary.total_travel_time, summary.total_travel_time / 60.0);
    printf("  ║  Total Passenger Wait Time : %9.1f min  (%6.1f hrs)              ║\n",
           summary.total_waiting_time, summary.total_waiting_time / 60.0);
    printf("  ║  Average Travel Time       : %9.1f min / passenger                 ║\n",
           summary.avg_travel_time);
    printf("  ║  Average Waiting Time      : %9.1f min / passenger                 ║\n",
           summary.avg_waiting_time);
    printf("  ║  Total Fares Collected     : $%9.2f                                ║\n",
           summary.total_fare);
    printf("  ╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("  ║  MODAL BREAKDOWN & TRANSFERS                                         ║\n");
    printf("  ║  Bus Passengers            : %6d passengers                      ║\n",
           summary.total_bus_passengers);
    printf("  ║  Train Passengers          : %6d passengers                      ║\n",
           summary.total_train_passengers);
    printf("  ║  Total Mode Transfers      : %6d transfers                       ║\n",
           summary.total_transfers);
    printf("  ╚══════════════════════════════════════════════════════════════════════╝\n\n");

    return summary;
}

/* ================================================================
 * runHourlyCapacitySimulation & runFullDayCapacitySimulation (Legacy)
 * ================================================================ */

SimulationMetrics runHourlyCapacitySimulation(const CityGraph *graph, int hour) {
    SimulationMetrics metrics;
    memset(&metrics, 0, sizeof(metrics));
    metrics.hour = hour;

    if (graph == NULL || hour < 0 || hour >= 24) return metrics;

    int demand = 0;
    Passenger *passengers = generateHourlyPassengers(hour, graph, &demand);
    metrics.total_demand = demand;

    if (demand == 0 || passengers == NULL) return metrics;

    PathResult *results = malloc(demand * sizeof(PathResult));
    if (results == NULL) {
        free(passengers);
        return metrics;
    }

    int current_bus_pos[MAX_LOCATIONS][MAX_LOCATIONS];
    int current_train_pos[MAX_LOCATIONS][MAX_LOCATIONS];
    memset(current_bus_pos, 0, sizeof(current_bus_pos));
    memset(current_train_pos, 0, sizeof(current_train_pos));

    int i;
    for (i = 0; i < demand; i++) {
        findFastestRoute(graph,
                         passengers[i].start_location,
                         passengers[i].destination_location,
                         OPTIMIZE_TIME,
                         &results[i]);

        if (!results[i].reachable) continue;

        metrics.passengers_served++;
        metrics.total_travel_time_min += results[i].total_time;

        double pax_wait_time = 5.0;
        int pax_overflowed = 0;

        int leg;
        for (leg = 0; leg < results[i].path_len - 1; leg++) {
            int u = results[i].path[leg];
            int v = results[i].path[leg + 1];

            if (leg > 0 && results[i].transport_used[leg] != results[i].transport_used[leg - 1]) {
                pax_wait_time += 5.0;
            }

            if (results[i].transport_used[leg] == TRANSPORT_BUS) {
                metrics.bus_trips++;
                int pos = current_bus_pos[u][v]++;
                int vehicle_run = pos / SIM_BUS_CAPACITY;
                if (vehicle_run > 0) {
                    double overflow_delay = vehicle_run * SIM_BUS_FREQUENCY_MIN;
                    pax_wait_time += overflow_delay;
                    pax_overflowed = 1;
                    metrics.overflow_events++;
                }
            } else {
                metrics.train_trips++;
                int pos = current_train_pos[u][v]++;
                int vehicle_run = pos / SIM_TRAIN_CAPACITY;
                if (vehicle_run > 0) {
                    double overflow_delay = vehicle_run * SIM_TRAIN_FREQUENCY_MIN;
                    pax_wait_time += overflow_delay;
                    pax_overflowed = 1;
                    metrics.overflow_events++;
                }
            }
        }

        if (pax_overflowed) metrics.passengers_waiting++;
        metrics.total_waiting_time_min += pax_wait_time;
    }

    metrics.total_offered_capacity = (10 * (60 / SIM_BUS_FREQUENCY_MIN) * SIM_BUS_CAPACITY) +
                                     (6  * (60 / SIM_TRAIN_FREQUENCY_MIN) * SIM_TRAIN_CAPACITY);

    if (metrics.passengers_served > 0) {
        metrics.avg_waiting_time_min = metrics.total_waiting_time_min / metrics.passengers_served;
    }

    int total_leg_boardings = metrics.bus_trips + metrics.train_trips;
    metrics.vehicle_utilization_pct = (metrics.total_offered_capacity > 0)
        ? (100.0 * total_leg_boardings / metrics.total_offered_capacity)
        : 0.0;

    free(results);
    free(passengers);

    return metrics;
}

SimulationMetrics runFullDayCapacitySimulation(const CityGraph *graph) {
    SimulationMetrics daily;
    memset(&daily, 0, sizeof(daily));
    daily.hour = -1;

    int h;
    for (h = 0; h < 24; h++) {
        SimulationMetrics hourly = runHourlyCapacitySimulation(graph, h);
        daily.total_demand            += hourly.total_demand;
        daily.passengers_served       += hourly.passengers_served;
        daily.passengers_waiting      += hourly.passengers_waiting;
        daily.total_waiting_time_min  += hourly.total_waiting_time_min;
        daily.total_travel_time_min   += hourly.total_travel_time_min;
        daily.total_offered_capacity  += hourly.total_offered_capacity;
        daily.bus_trips               += hourly.bus_trips;
        daily.train_trips             += hourly.train_trips;
        daily.overflow_events         += hourly.overflow_events;
    }

    if (daily.passengers_served > 0) {
        daily.avg_waiting_time_min = daily.total_waiting_time_min / daily.passengers_served;
    }

    int total_leg_boardings = daily.bus_trips + daily.train_trips;
    daily.vehicle_utilization_pct = (daily.total_offered_capacity > 0)
        ? (100.0 * total_leg_boardings / daily.total_offered_capacity)
        : 0.0;

    return daily;
}

/* ================================================================
 * printSimulationMetricsReport
 * ================================================================ */

void printSimulationMetricsReport(const SimulationMetrics *m, const char *title) {
    if (m == NULL) return;

    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║  %-56s  ║\n", title ? title : "SIMULATION METRICS REPORT");
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    if (m->hour >= 0) {
        printf("  ║  Hour Simulated     : %02d:00 - %02d:00                  ║\n",
               m->hour, (m->hour + 1) % 24);
        printf("  ║  Demand Level       : %-35s║\n",
               getDemandLevelName(getDemandLevel(m->hour)));
    } else {
        printf("  ║  Time Period        : Full 24-Hour Day                   ║\n");
    }
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    printf("  ║  CAPACITY & DEMAND STATISTICS                            ║\n");
    printf("  ║  Total Demand       : %6d passengers                   ║\n", m->total_demand);
    printf("  ║  Passengers Served  : %6d passengers (%5.1f%%)           ║\n",
           m->passengers_served,
           (m->total_demand > 0) ? (100.0 * m->passengers_served / m->total_demand) : 0.0);
    printf("  ║  Passengers Delayed : %6d passengers (%5.1f%% forced wait)║\n",
           m->passengers_waiting,
           (m->passengers_served > 0) ? (100.0 * m->passengers_waiting / m->passengers_served) : 0.0);
    printf("  ║  Capacity Overflow  : %6d overflow event(s)            ║\n", m->overflow_events);
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    printf("  ║  WAITING TIME & TRAVEL METRICS                           ║\n");
    printf("  ║  Total Waiting Time : %9.1f min  (%5.1f hrs)          ║\n",
           m->total_waiting_time_min, m->total_waiting_time_min / 60.0);
    printf("  ║  Avg Waiting Time   : %9.2f min per passenger          ║\n", m->avg_waiting_time_min);
    printf("  ║  Total In-Vehicle   : %9.1f min  (%5.1f hrs)          ║\n",
           m->total_travel_time_min, m->total_travel_time_min / 60.0);
    printf("  ╠══════════════════════════════════════════════════════════╣\n");
    printf("  ║  VEHICLE UTILIZATION & LEG TRIPS                         ║\n");
    printf("  ║  Bus Leg Trips      : %6d passenger-trips              ║\n", m->bus_trips);
    printf("  ║  Train Leg Trips    : %6d passenger-trips              ║\n", m->train_trips);
    printf("  ║  Offered Capacity   : %6.0f seats                        ║\n", m->total_offered_capacity);
    printf("  ║  Fleet Utilization  : %6.2f%%                              ║\n", m->vehicle_utilization_pct);
    printf("  ╚══════════════════════════════════════════════════════════╝\n\n");
}

/* ================================================================
 * simulationMenu – Interactive menu for full-day simulation
 * ================================================================ */

void simulationMenu(const CityGraph *graph) {
    if (graph == NULL) return;

    printf("\n");
    printf("  ┌──────────────────────────────────────────────────────┐\n");
    printf("  │       FULL-DAY SMART CITY TRANSPORT SIMULATOR        │\n");
    printf("  ├──────────────────────────────────────────────────────┤\n");
    printf("  │  1. Run Complete Full-Day Simulation (06:00-23:00)   │\n");
    printf("  │  2. Display 24-Hour Passenger Demand Profile         │\n");
    printf("  │  3. Run Peak Morning Simulation (08:00 AM)           │\n");
    printf("  │  4. Run Peak Evening Simulation (18:00 PM)           │\n");
    printf("  └──────────────────────────────────────────────────────┘\n");
    printf("  Enter choice (1-4): ");

    int choice = 1;
    if (scanf("%d", &choice) != 1) choice = 1;

    switch (choice) {
        case 1:
            runFullDaySimulation(graph);
            break;
        case 2:
            displayFullDayDemand();
            break;
        case 3: {
            SimulationMetrics peak_morn = runHourlyCapacitySimulation(graph, 8);
            printSimulationMetricsReport(&peak_morn, "MORNING PEAK SIMULATION REPORT (08:00)");
            break;
        }
        case 4: {
            SimulationMetrics peak_eve = runHourlyCapacitySimulation(graph, 18);
            printSimulationMetricsReport(&peak_eve, "EVENING PEAK SIMULATION REPORT (18:00)");
            break;
        }
        default:
            runFullDaySimulation(graph);
            break;
    }
}

/* ================================================================
 * Legacy simulation stubs
 * ================================================================ */

void init_simulation(CityGraph *graph) {
    (void)graph;
    printf("[Simulation] Simulation engine initialised.\n");
}

void run_simulation(TimeOfDay time_of_day) {
    (void)time_of_day;
    printf("[Simulation] Simulation step executed.\n");
}

SimulationMetrics get_simulation_results(void) {
    SimulationMetrics m;
    memset(&m, 0, sizeof(m));
    return m;
}
