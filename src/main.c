/* ================================================================
 * main.c – Smart City Public Transport Simulator
 *
 * Integrated interactive menu entry point connecting all modules:
 * Graph, Transport (Bus & Train), Dijkstra, Passenger, Simulation,
 * and Profiling.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "transport.h"
#include "dijkstra.h"
#include "passenger.h"
#include "simulation.h"
#include "profiling.h"

/* ----------------------------------------------------------------
 * Input validation helpers
 * ---------------------------------------------------------------- */

static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static int readInt(const char *prompt, int min_val, int max_val) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1) {
            if (val >= min_val && val <= max_val) {
                clearInputBuffer();
                return val;
            }
            printf("  [!] Value out of range (%d to %d). Please try again.\n", min_val, max_val);
        } else {
            printf("  [!] Invalid input. Please enter a valid integer.\n");
        }
        clearInputBuffer();
    }
}

/* ----------------------------------------------------------------
 * printMenu
 * ---------------------------------------------------------------- */

static void printMenu(void) {
    printf("========================================\n");
    printf(" SMART CITY PUBLIC TRANSPORT SYSTEM     \n");
    printf("========================================\n");
    printf(" 1. Display City Locations              \n");
    printf(" 2. Display City Map                    \n");
    printf(" 3. Display Bus Network                 \n");
    printf(" 4. Display Train Network               \n");
    printf(" 5. Display All Routes                  \n");
    printf(" 6. Find Fastest Route                  \n");
    printf(" 7. Find Shortest Route                 \n");
    printf(" 8. Find Cheapest Route                 \n");
    printf(" 9. Simulate Passenger Journey          \n");
    printf("10. Simulate Passenger Demand           \n");
    printf("11. Run Full-Day Simulation             \n");
    printf("12. Display System Profile              \n");
    printf(" 0. Exit                                \n");
    printf("________________________________________\n");
}

/* ----------------------------------------------------------------
 * main
 * ---------------------------------------------------------------- */

int main(void) {
    /* ---- Initialise city graph ---- */
    CityGraph city;
    initializeGraph(&city);
    printf("\n");
    initializeCity(&city);

    /* ---- Initialise bus fleet ---- */
    BusNetwork buses;
    printf("\n");
    initializeBuses(&buses);

    /* ---- Initialise train fleet ---- */
    TrainNetwork trains;
    printf("\n");
    initializeTrains(&trains);

    /* ---- Menu loop ---- */
    int choice = -1;

    while (choice != 0) {
        printf("\n");
        printMenu();
        choice = readInt("Enter option (0-12): ", 0, 12);
        printf("\n");

        switch (choice) {

            /* ── 1: Display City Locations ── */
            case 1:
                displayLocations(&city);
                break;

            /* ── 2: Display City Graph ── */
            case 2:
                displayGraph(&city);
                break;

            /* ── 3: Display Bus Network ── */
            case 3:
                displayBuses(&buses);
                displayBusRoutes(&buses, &city);
                break;

            /* ── 4: Display Train Network ── */
            case 4:
                displayTrains(&trains);
                displayTrainRoutes(&trains, &city);
                break;

            /* ── 5: Display All Routes ── */
            case 5:
                displayCityNetwork(&city);
                break;

            /* ── 6: Find Fastest Route (Time) ── */
            case 6: {
                displayLocations(&city);
                int src = readInt("  Enter Start Location ID (0-9): ", 0, city.location_count - 1);
                int dst = readInt("  Enter Destination ID    (0-9): ", 0, city.location_count - 1);
                if (src == dst) {
                    printf("  [!] Source and destination are the same location.\n");
                } else {
                    PathResult res;
                    findFastestRoute(&city, src, dst, OPTIMIZE_TIME, &res);
                    printRoute(&res, &city);
                }
                break;
            }

            /* ── 7: Find Shortest Route (Distance) ── */
            case 7: {
                displayLocations(&city);
                int src = readInt("  Enter Start Location ID (0-9): ", 0, city.location_count - 1);
                int dst = readInt("  Enter Destination ID    (0-9): ", 0, city.location_count - 1);
                if (src == dst) {
                    printf("  [!] Source and destination are the same location.\n");
                } else {
                    PathResult res;
                    findFastestRoute(&city, src, dst, OPTIMIZE_DISTANCE, &res);
                    printRoute(&res, &city);
                }
                break;
            }

            /* ── 8: Find Cheapest Route (Fare) ── */
            case 8: {
                displayLocations(&city);
                int src = readInt("  Enter Start Location ID (0-9): ", 0, city.location_count - 1);
                int dst = readInt("  Enter Destination ID    (0-9): ", 0, city.location_count - 1);
                if (src == dst) {
                    printf("  [!] Source and destination are the same location.\n");
                } else {
                    PathResult res;
                    findFastestRoute(&city, src, dst, OPTIMIZE_FARE, &res);
                    printRoute(&res, &city);
                }
                break;
            }

            /* ── 9: Simulate Passenger Journey ── */
            case 9:
                passengerMenu(&city);
                break;

            /* ── 10: Simulate Passenger Demand ── */
            case 10:
                displayFullDayDemand();
                break;

            /* ── 11: Run Full-Day Simulation ── */
            case 11:
                runFullDaySimulation(&city);
                break;

            /* ── 12: Display System Profile ── */
            case 12:
                runFullProfiling(&city);
                break;


            /* ── 0: Exit ── */
            case 0:
                printf("  Freeing city graph memory...\n");
                freeGraph(&city);
                printf("  Memory freed successfully. Goodbye!\n\n");
                break;

            default:
                printf("  [!] Invalid choice.\n");
                break;
        }
    }

    return 0;
}
