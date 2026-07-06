# Ride Hailing System using B+ Trees

## Overview

This project implements a console-based Ride Hailing System in C that simulates the core functionalities of ride-sharing platforms such as Uber and Ola.

The system uses B+ Trees as the primary indexing structure for managing drivers, passengers, and bookings efficiently while supporting fast search, insertion, and range queries.

The project demonstrates concepts from:

- Data Structures
- Database Indexing
- File Handling
- Algorithm Design
- Systems Programming

---

## Features

### Driver Management
- Add Driver
- Delete Driver
- Update Driver Location
- Display Available Vehicles

### Passenger Management
- Add Passenger
- Passenger Range Search

### Ride Management
- Request Ride
- Complete Ride
- Booking History
- Driver Earnings Calculation

### Analytics
- Top Drivers by Earnings
- Most Frequent Driver-Passenger Pair

### Persistence
- Save all records to files
- Load existing records automatically

---

## Data Structures Used

| Entity | Data Structure |
|---------|---------------|
| Drivers | B+ Tree |
| Passengers | B+ Tree |
| Bookings | B+ Tree |
| Persistence | Text Files |

---

## Why B+ Trees?

B+ Trees are widely used in database indexing systems because they provide:

- O(log n) search complexity
- O(log n) insertion complexity
- Efficient range queries
- Sequential traversal using linked leaves

In this project:

### Driver Tree
```
Driver ID → Driver Record
```

### Passenger Tree
```
Passenger ID → Passenger Record
```

### Booking Tree
```
Booking ID → Booking Record
```

---

## B+ Tree Structure

```text
               [200]
              /     \
         [100]       [300]
        /    \       /    \
      Leaf  Leaf   Leaf  Leaf
```

The leaf nodes are linked together:

```text
Leaf -> Leaf -> Leaf -> Leaf
```

This linked structure allows efficient range search operations.

---

## Ride Allocation Algorithm

When a passenger requests a ride:

1. Search passenger database.
2. Traverse driver B+ Tree leaves.
3. Find the nearest available driver.
4. Verify distance ≤ 5 km.
5. Create booking.
6. Update driver status.

---

## Project Structure

```text
Ride-Hailing-System/

├── main.c
├── ride.c
├── ride.h
├── drivers.txt
├── passengers.txt
├── bookings.txt
└── README.md
```

---

## Sample Features

- Nearest vehicle search
- Driver earnings tracking
- Frequent customer analysis
- Booking history management
- Passenger range queries
- Persistent storage

---

## Time Complexity

| Operation | Complexity |
|-----------|------------|
| Search | O(log n) |
| Insert | O(log n) |
| Delete | O(log n) |
| Range Search | O(log n + k) |
| Ride Allocation | O(n) |

---

## Compilation

```bash
gcc main.c ride.c -lm -o ride
```

Run:

```bash
./ride
```

---

## Concepts Demonstrated

- B+ Trees
- Range Queries
- File Handling
- Dynamic Memory Allocation
- Database Indexing
- Searching Algorithms
- Sorting Algorithms
- Persistent Storage
- System Design

---

## Author

Raghav Nimgaonkar

B.Tech Computer Science and Engineering
VNIT Nagpur