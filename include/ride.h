#ifndef RIDE_H
#define RIDE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ===== B+ TREE ORDER ===== */
#define ORDER 4   /* max children per internal node; max keys = ORDER-1 */

/* ===== DATA STRUCTURES ===== */

typedef struct Driver {
    int    d_ID;
    char   name[50];
    int    vehicle_type;     /* 0=Cab, 1=Bike */
    int    x, y;
    int    status;           /* 0=free, 1=booked */
    float  total_earnings;
} Driver;

typedef struct Passenger {
    int  p_ID;
    char name[50];
    char mobile[20];
    int  frequency;
} Passenger;

typedef struct Booking {
    int   booking_id;
    int   d_ID;
    int   p_ID;
    int   vehicle_type;
    float distance;
    float fare;
} Booking;

/* ===== B+ TREE NODE ===== */
/* Leaf nodes hold actual records; internal nodes hold keys + child pointers. */

typedef struct BPNode {
    int          is_leaf;
    int          num_keys;
    int          keys[ORDER];          /* separator/record keys          */
    struct BPNode *children[ORDER+1];  /* children (internal) or NULL    */
    struct BPNode *next;               /* next leaf (leaf level only)    */

    /* Payloads – only one union field is valid per tree type */
    Driver    *drv_data[ORDER];   /* Driver leaf records   */
    Passenger *pax_data[ORDER];   /* Passenger leaf records*/
    Booking   *bk_data[ORDER];    /* Booking leaf records  */
} BPNode;

/* ===== B+ TREE ROOT WRAPPERS ===== */
typedef struct { BPNode *root; } DriverTree;
typedef struct { BPNode *root; } PassengerTree;
typedef struct { BPNode *root; } BookingTree;

/* ===== FUNCTION DECLARATIONS ===== */

/* --- Node helpers --- */
BPNode *bpNewNode(int is_leaf);

/* --- Driver B+ tree operations --- */
void     drvInsert(DriverTree *T, Driver *d);
Driver  *drvSearch(DriverTree *T, int id);
void     drvDelete(DriverTree *T, int id);
void     drvInorder(BPNode *node, void(*fn)(Driver*));

/* --- Passenger B+ tree operations --- */
void      paxInsert(PassengerTree *T, Passenger *p);
Passenger *paxSearch(PassengerTree *T, int id);
void      paxRangeSearch(PassengerTree *T, int id1, int id2);

/* --- Booking B+ tree operations --- */
void     bkInsert(BookingTree *T, Booking *b);
Booking *bkSearch(BookingTree *T, int id);
void     bkInorder(BPNode *node, void(*fn)(Booking*));

/* ===== APPLICATION FUNCTIONS ===== */

void    addDriver(DriverTree *T, int id, char name[], int type, int x, int y);
void    addPassenger(PassengerTree *T, int id, char name[], char mobile[]);
Driver *findNearestVehicle(DriverTree *T, int px, int py, int pref);
int     requestRide(DriverTree *DT, PassengerTree *PT, BookingTree *BT,
                    int p_id, int px, int py, int pref);
void    completeRide(DriverTree *DT, PassengerTree *PT, BookingTree *BT,
                     int bid, float distance);
float   calculateDriverEarnings(BookingTree *BT, int d_id);
void    displayTopDrivers(DriverTree *DT, BookingTree *BT);
void    displayFrequentPairs(DriverTree *DT, PassengerTree *PT, BookingTree *BT);
void    displayAvailableVehicles(DriverTree *DT);
void    updateDriverLocation(DriverTree *DT, int id, int x, int y);
void    deleteDriver(DriverTree *DT, int id);
void    displayBookingHistory(BookingTree *BT);
void    rangeSearchPassengers(PassengerTree *PT, int id1, int id2);

/* --- Persistence --- */
void saveDrivers(DriverTree *DT);
void loadDrivers(DriverTree *DT);
void savePassengers(PassengerTree *PT);
void loadPassengers(PassengerTree *PT);
void saveBookings(BookingTree *BT);
void loadBookings(BookingTree *BT);

#endif /* RIDE_H */
