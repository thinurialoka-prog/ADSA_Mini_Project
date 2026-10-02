/* ================================================================
 * dijkstra.c – Dijkstra's shortest-path algorithm
 *
 * Operates directly on the CityGraph adjacency-list built by
 * graph.c / transport.c.  Edges may be BUS or TRAIN routes;
 * the algorithm is transport-agnostic and treats every edge
 * according to the selected OptimizeMode weight.
 *
 * Algorithm overview
 * ──────────────────
 *  1. Initialise cost[] = +INF for all vertices except source (= 0).
 *  2. Repeat V times:
 *     a. Pick the unvisited vertex u with the smallest cost[u].
 *     b. Mark u as visited (finalised).
 *     c. For each outgoing edge u → v (Route node in linked list):
 *        - Extract edge weight w according to OptimizeMode.
 *        - If cost[u] + w < cost[v]: relax → cost[v] = cost[u] + w,
 *          parent[v] = u, and record which Route was taken.
 *  3. Reconstruct the path by back-tracing parent[] from dest → src,
 *     then reversing the sequence.
 *
 * Time complexity
 * ───────────────
 *  V = number of vertices (locations), E = number of edges (routes).
 *
 *  This implementation uses a LINEAR SCAN to find the minimum-cost
 *  unvisited vertex (step 2a), giving:
 *    • Min-extraction: O(V) per iteration × V iterations = O(V²)
 *    • Edge relaxation: O(E) total across all iterations
 *    • Overall:  O(V²)
 *
 *  For the 10-vertex city graph this is perfectly efficient.
 *  A binary-heap priority queue would reduce this to O((V+E) log V),
 *  which is preferable for large sparse graphs.
 * ================================================================ */

#include "dijkstra.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>    /* DBL_MAX */

/* ================================================================
 * Internal helpers
 * ================================================================ */

/* Sentinel representing "no predecessor" */
#define NO_PARENT (-1)

/* Extract the edge weight from a Route node according to mode */
static double edgeWeight(const Route *r, OptimizeMode mode) {
    switch (mode) {
        case OPTIMIZE_TIME:     return r->travel_time;
        case OPTIMIZE_DISTANCE: return r->distance;
        case OPTIMIZE_FARE:     return r->fare;
        default:                return r->travel_time;
    }
}

/* Return the name of an OptimizeMode as a string */
static const char *modeName(OptimizeMode mode) {
    switch (mode) {
        case OPTIMIZE_TIME:     return "Fastest  (min travel time)";
        case OPTIMIZE_DISTANCE: return "Shortest (min distance)";
        case OPTIMIZE_FARE:     return "Cheapest (min fare)";
        default:                return "Unknown";
    }
}

/* Return the name of a TransportType */
static const char *transportName(TransportType t) {
    return (t == TRANSPORT_BUS) ? "Bus" : "Train";
}

/*
 * Linear-scan O(V) extraction of the unvisited vertex with
 * the smallest cost.  Returns -1 when all remaining vertices
 * are unreachable (cost == DBL_MAX).
 */
static int extractMin(const double *cost,
                      const int    *visited,
                      int           n)
{
    int    best_v    = -1;
    double best_cost = DBL_MAX;
    int    v;
    for (v = 0; v < n; v++) {
        if (!visited[v] && cost[v] < best_cost) {
            best_cost = cost[v];
            best_v    = v;
        }
    }
    return best_v;
}

/* ================================================================
 * findFastestRoute
 * ================================================================ */

int findFastestRoute(const CityGraph *graph,
                     int              source,
                     int              destination,
                     OptimizeMode     mode,
                     PathResult      *result)
{
    if (graph == NULL || result == NULL) return 0;
    if (source < 0 || source >= graph->location_count) return 0;
    if (destination < 0 || destination >= graph->location_count) return 0;

    int n = graph->location_count;

    /* ── Step 1: Initialise ── */
    int v;
    for (v = 0; v < n; v++) {
        result->cost[v]    = DBL_MAX;
        result->parent[v]  = NO_PARENT;
        result->visited[v] = 0;
    }
    result->cost[source] = 0.0;

    /*
     * best_route[v] stores the Route pointer that was used when the
     * shortest path to v was last improved.  It lets us reconstruct
     * per-leg attributes (transport, distance, time, fare) at the end.
     */
    const Route *best_route[MAX_LOCATIONS];
    for (v = 0; v < n; v++) best_route[v] = NULL;

    /* ── Step 2: Main Dijkstra loop (V iterations) ── */
    int iter;
    for (iter = 0; iter < n; iter++) {

        /* 2a. Extract unvisited vertex with minimum cost — O(V) */
        int u = extractMin(result->cost, result->visited, n);
        if (u == -1) break;          /* all remaining unreachable */
        if (u == destination) break; /* early exit: target finalised */

        /* 2b. Mark u as visited (finalised) */
        result->visited[u] = 1;

        /* 2c. Relax all outgoing edges from u */
        const Route *r = graph->locations[u].routes;
        while (r != NULL) {
            int    nb  = r->destination;
            double w   = edgeWeight(r, mode);
            double new_cost = result->cost[u] + w;

            if (!result->visited[nb] && new_cost < result->cost[nb]) {
                result->cost[nb]    = new_cost;
                result->parent[nb]  = u;
                best_route[nb]      = r;
            }
            r = r->next;
        }
    }

    /* ── Step 3: Check reachability ── */
    result->source      = source;
    result->destination = destination;
    result->mode        = mode;

    if (result->cost[destination] == DBL_MAX) {
        result->reachable = 0;
        result->path_len  = 0;
        return 0;
    }
    result->reachable = 1;

    /* ── Step 4: Reconstruct path by back-tracing parent[] ── */
    /* Trace backward from destination to source */
    int temp_path[MAX_PATH_LEN];
    int temp_len = 0;
    int cur = destination;

    while (cur != NO_PARENT && temp_len < MAX_PATH_LEN) {
        temp_path[temp_len++] = cur;
        cur = result->parent[cur];
    }

    /* Reverse to get source → destination order */
    result->path_len = temp_len;
    int i;
    for (i = 0; i < temp_len; i++) {
        result->path[i] = temp_path[temp_len - 1 - i];
    }

    /* ── Step 5: Fill per-leg attributes ── */
    result->total_distance = 0.0;
    result->total_time     = 0.0;
    result->total_fare     = 0.0;

    for (i = 0; i < result->path_len - 1; i++) {
        int from = result->path[i];
        int to   = result->path[i + 1];

        /* Find the specific route edge from → to that was selected.
           We stored best_route[to] during relaxation. */
        const Route *leg = best_route[to];
        if (leg != NULL && leg->destination == to) {
            result->transport_used[i] = leg->transport;
            result->leg_distance[i]   = leg->distance;
            result->leg_time[i]       = leg->travel_time;
            result->leg_fare[i]       = leg->fare;
        } else {
            /* Fallback: scan adjacency list of 'from' for edge to 'to' */
            const Route *scan = graph->locations[from].routes;
            while (scan != NULL) {
                if (scan->destination == to) {
                    result->transport_used[i] = scan->transport;
                    result->leg_distance[i]   = scan->distance;
                    result->leg_time[i]       = scan->travel_time;
                    result->leg_fare[i]       = scan->fare;
                    break;
                }
                scan = scan->next;
            }
        }

        result->total_distance += result->leg_distance[i];
        result->total_time     += result->leg_time[i];
        result->total_fare     += result->leg_fare[i];
    }

    return 1;
}

/* ================================================================
 * printRoute
 * ================================================================ */

void printRoute(const PathResult *result, const CityGraph *graph) {
    if (result == NULL || graph == NULL) return;

    printf("\n");
    printf("  ========================================\n");
    printf("               ROUTE FINDER — RESULT                     \n");
    printf("  ========================================\n");
    printf("  Mode  : %-47s\n", modeName(result->mode));
    printf("  From  : %-47s\n",
           graph->locations[result->source].name);
    printf("  To    : %-47s\n",
           graph->locations[result->destination].name);
    printf("  ========================================\n");

    if (!result->reachable) {
        printf("    [!] No path found between these locations.            \n");
        printf("  ========================================\n\n");
        return;
    }

    /* ── Path sequence header ── */
    printf("    PATH SEQUENCE                                          \n");
    printf("  ========================================\n");

    int i;
    for (i = 0; i < result->path_len; i++) {
        int loc_id = result->path[i];
        const char *name = graph->locations[loc_id].name;

        if (i == 0) {
            printf("  ║  [START] [%d] %-43s║\n", loc_id, name);
        } else if (i == result->path_len - 1) {
            /* Print the incoming leg before the final stop */
            printf("  ║  │  %-7s  dist: %5.1f km  time: %4.0f min  "
                   "fare: $%5.2f  ║\n",
                   transportName(result->transport_used[i - 1]),
                   result->leg_distance[i - 1],
                   result->leg_time[i - 1],
                   result->leg_fare[i - 1]);
            printf("  ║  [  END ] [%d] %-43s║\n", loc_id, name);
        } else {
            printf("  ║  │  %-7s  dist: %5.1f km  time: %4.0f min  "
                   "fare: $%5.2f  ║\n",
                   transportName(result->transport_used[i - 1]),
                   result->leg_distance[i - 1],
                   result->leg_time[i - 1],
                   result->leg_fare[i - 1]);
            printf("  ║  [STOP %-1d] [%d] %-43s║\n", i, loc_id, name);
        }
    }

    /* ── Summary totals ── */
    printf("  ========================================╣\n");
    printf("    TOTALS                                                 \n\n");
    printf("    Total Distance : %7.2f km                            \n",
           result->total_distance);
    printf("    Total Time     : %7.2f min  (%4.1f hrs)              \n",
           result->total_time, result->total_time / 60.0);
    printf("    Total Fare     : $%7.2f                              \n",
           result->total_fare);
    printf("    Stops (legs)   : %d stop(s), %d leg(s)                ",
           result->path_len, result->path_len - 1);
    /* Pad to fill the box */
    printf("        \n");
    printf("  ========================================\n\n");
}

/* ================================================================
 * calculateRoute* helpers
 * ================================================================ */

double calculateRouteDistance(const PathResult *result) {
    if (result == NULL || !result->reachable) return -1.0;
    return result->total_distance;
}

double calculateRouteTime(const PathResult *result) {
    if (result == NULL || !result->reachable) return -1.0;
    return result->total_time;
}

double calculateRouteFare(const PathResult *result) {
    if (result == NULL || !result->reachable) return -1.0;
    return result->total_fare;
}

/* ================================================================
 * routeFinderMenu – interactive user interface
 * ================================================================ */

void routeFinderMenu(const CityGraph *graph) {
    if (graph == NULL) return;

    /* ── Print the location index for the user to reference ── */
    printf("\n");
    displayLocations(graph);

    /* ── Get source ── */
    int source = -1;
    printf("  Enter start location ID (0-%d): ", graph->location_count - 1);
    if (scanf("%d", &source) != 1 || source < 0 ||
        source >= graph->location_count) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("  [!] Invalid location ID.\n");
        return;
    }

    /* ── Get destination ── */
    int destination = -1;
    printf("  Enter destination ID    (0-%d): ", graph->location_count - 1);
    if (scanf("%d", &destination) != 1 || destination < 0 ||
        destination >= graph->location_count) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("  [!] Invalid location ID.\n");
        return;
    }

    if (source == destination) {
        printf("  [!] Source and destination are the same location.\n");
        return;
    }

    /* ── Get optimization mode ── */
    printf("\n");
    printf("  Optimization modes:\n");
    printf("    1. Fastest  — minimize travel time\n");
    printf("    2. Shortest — minimize distance\n");
    printf("    3. Cheapest — minimize fare\n");
    printf("  Enter mode (1-3): ");

    int mode_input = 1;
    if (scanf("%d", &mode_input) != 1 || mode_input < 1 || mode_input > 3) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("  [!] Invalid mode. Defaulting to Fastest.\n");
        mode_input = 1;
    }

    OptimizeMode mode;
    switch (mode_input) {
        case 2:  mode = OPTIMIZE_DISTANCE; break;
        case 3:  mode = OPTIMIZE_FARE;     break;
        default: mode = OPTIMIZE_TIME;     break;
    }

    /* ── Run Dijkstra and display result ── */
    PathResult result;
    findFastestRoute(graph, source, destination, mode, &result);
    printRoute(&result, graph);

    /* ── Optionally show all three modes for comparison ── */
    printf("  Show comparison across all 3 modes? (1=yes / 0=no): ");
    int show_all = 0;
    if (scanf("%d", &show_all) != 1) show_all = 0;

    if (show_all == 1) {
        OptimizeMode modes[3] = {
            OPTIMIZE_TIME, OPTIMIZE_DISTANCE, OPTIMIZE_FARE
        };
        int m;
        for (m = 0; m < 3; m++) {
            if (modes[m] == mode) continue; /* already printed */
            PathResult r2;
            findFastestRoute(graph, source, destination, modes[m], &r2);
            printRoute(&r2, graph);
        }
    }
}
