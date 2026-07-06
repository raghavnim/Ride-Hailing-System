#include "ride.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== SAFE INPUT HELPERS ===== */

static int safeIntInput(const char *msg, int *out) {
    char buf[128];
    printf("%s", msg);
    fflush(stdout);
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    /* Strip trailing newline */
    buf[strcspn(buf, "\n")] = '\0';
    if (buf[0] == '\0') return 0;
    char *end;
    long val = strtol(buf, &end, 10);
    if (end == buf || *end != '\0') return 0;
    *out = (int)val;
    return 1;
}

static int safeFloatInput(const char *msg, float *out) {
    char buf[128];
    printf("%s", msg);
    fflush(stdout);
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    buf[strcspn(buf, "\n")] = '\0';
    if (buf[0] == '\0') return 0;
    char *end;
    float val = strtof(buf, &end);
    if (end == buf || *end != '\0') return 0;
    *out = val;
    return 1;
}

static int safeStringInput(const char *msg, char *out, int maxLen) {
    printf("%s", msg);
    fflush(stdout);
    if (!fgets(out, maxLen, stdin)) return 0;
    out[strcspn(out, "\n")] = '\0';
    if (out[0] == '\0') return 0;
    return 1;
}

/* ===== MAIN ===== */

int main(void) {
    DriverTree    DT = {NULL};
    PassengerTree PT = {NULL};
    BookingTree   BT = {NULL};

    printf("Loading data from files...\n");
    loadDrivers(&DT);
    loadPassengers(&PT);
    loadBookings(&BT);
    printf("Data loaded successfully.\n");

    int ch;
    while (1) {
        
        printf("   RIDE-HAILING SYSTEM MENU   \n");
        printf("  1. Add Driver               \n");
        printf("  2. Add Passenger            \n");
        printf("  3. Request Ride             \n");
        printf("  4. Complete Ride            \n");
        printf("  5. Top Drivers              \n");
        printf("  6. Frequent Pairs           \n");
        printf("  7. Available Vehicles       \n");
        printf("  8. Update Driver Location   \n");
        printf("  9. Delete Driver            \n");
        printf(" 10. Booking History          \n");
        printf(" 11. Range Search (Passengers)\n");
        printf(" 12. Save                     \n");
        printf(" 13. Exit                     \n");
        

        if (!safeIntInput("Enter choice: ", &ch)) {
            printf("ERROR: Invalid input! Please enter a number between 1 and 13.\n");
            continue;
        }

        /* ---- 1. Add Driver ---- */
        if (ch == 1) {
            int id, type, x, y;
            char name[50];
            if (!safeIntInput("Driver ID   : ", &id) || id <= 0) {
                printf("ERROR: Invalid ID! Must be a positive integer.\n"); continue;
            }
            if (!safeStringInput("Name        : ", name, 50)) {
                printf("ERROR: Name cannot be empty.\n"); continue;
            }
            if (!safeIntInput("Type (0=Cab, 1=Bike): ", &type)) {
                printf("ERROR: Invalid type.\n"); continue;
            }
            if (!safeIntInput("X coordinate: ", &x)) {
                printf("ERROR: Invalid X coordinate.\n"); continue;
            }
            if (!safeIntInput("Y coordinate: ", &y)) {
                printf("ERROR: Invalid Y coordinate.\n"); continue;
            }
            addDriver(&DT, id, name, type, x, y);
        }

        /* ---- 2. Add Passenger ---- */
        else if (ch == 2) {
            int id;
            char name[50], mobile[20];
            if (!safeIntInput("Passenger ID: ", &id) || id <= 0) {
                printf("ERROR: Invalid ID! Must be a positive integer.\n"); continue;
            }
            if (!safeStringInput("Name        : ", name, 50)) {
                printf("ERROR: Name cannot be empty.\n"); continue;
            }
            if (!safeStringInput("Mobile No.  : ", mobile, 20)) {
                printf("ERROR: Mobile number cannot be empty.\n"); continue;
            }
            addPassenger(&PT, id, name, mobile);
        }

        /* ---- 3. Request Ride ---- */
        else if (ch == 3) {
            int pid, x, y, pref;
            if (!safeIntInput("Passenger ID      : ", &pid) || pid <= 0) {
                printf("ERROR: Invalid Passenger ID.\n"); continue;
            }
            if (!safeIntInput("Pickup X          : ", &x)) {
                printf("ERROR: Invalid X coordinate.\n"); continue;
            }
            if (!safeIntInput("Pickup Y          : ", &y)) {
                printf("ERROR: Invalid Y coordinate.\n"); continue;
            }
            if (!safeIntInput("Preference (-1=Any, 0=Cab, 1=Bike): ", &pref)) {
                printf("ERROR: Invalid preference.\n"); continue;
            }
            requestRide(&DT, &PT, &BT, pid, x, y, pref);
        }

        /* ---- 4. Complete Ride ---- */
        else if (ch == 4) {
            int bid;
            float dist;
            if (!safeIntInput("Booking ID  : ", &bid) || bid <= 0) {
                printf("ERROR: Invalid Booking ID.\n"); continue;
            }
            if (!safeFloatInput("Distance (km): ", &dist)) {
                printf("ERROR: Invalid distance. Enter a non-negative number.\n"); continue;
            }
            if (dist < 0) {
                printf("ERROR: Distance cannot be negative!\n"); continue;
            }
            completeRide(&DT, &PT, &BT, bid, dist);
        }

        /* ---- 5. Top Drivers ---- */
        else if (ch == 5) {
            displayTopDrivers(&DT, &BT);
        }

        /* ---- 6. Frequent Pairs ---- */
        else if (ch == 6) {
            displayFrequentPairs(&DT, &PT, &BT);
        }

        /* ---- 7. Available Vehicles ---- */
        else if (ch == 7) {
            displayAvailableVehicles(&DT);
        }

        /* ---- 8. Update Driver Location ---- */
        else if (ch == 8) {
            int id, x, y;
            if (!safeIntInput("Driver ID   : ", &id) || id <= 0) {
                printf("ERROR: Invalid Driver ID.\n"); continue;
            }
            if (!safeIntInput("New X       : ", &x)) {
                printf("ERROR: Invalid X coordinate.\n"); continue;
            }
            if (!safeIntInput("New Y       : ", &y)) {
                printf("ERROR: Invalid Y coordinate.\n"); continue;
            }
            updateDriverLocation(&DT, id, x, y);
        }

        /* ---- 9. Delete Driver ---- */
        else if (ch == 9) {
            int id;
            if (!safeIntInput("Driver ID   : ", &id) || id <= 0) {
                printf("ERROR: Invalid Driver ID.\n"); continue;
            }
            deleteDriver(&DT, id);
        }

        /* ---- 10. Booking History ---- */
        else if (ch == 10) {
            displayBookingHistory(&BT);
        }

        /* ---- 11. Range Search Passengers ---- */
        else if (ch == 11) {
            int id1, id2;
            if (!safeIntInput("Start P_ID (P_ID1): ", &id1) || id1 <= 0) {
                printf("ERROR: Invalid P_ID1. Must be a positive integer.\n"); continue;
            }
            if (!safeIntInput("End   P_ID (P_ID2): ", &id2) || id2 <= 0) {
                printf("ERROR: Invalid P_ID2. Must be a positive integer.\n"); continue;
            }
            rangeSearchPassengers(&PT, id1, id2);
        }

        /* ---- 12. Save ---- */
        else if (ch == 12) {
            saveDrivers(&DT);
            savePassengers(&PT);
            saveBookings(&BT);
            printf("SUCCESS: All data saved to files.\n");
        }

        /* ---- 13. Exit ---- */
        else if (ch == 13) {
            saveDrivers(&DT);
            savePassengers(&PT);
            saveBookings(&BT);
            printf("SUCCESS: Data saved. Goodbye!\n");
            break;
        }

        else {
            printf("ERROR: Invalid choice! Please enter a number between 1 and 13.\n");
        }
    }

    return 0;
}
