#include "ride.h"

/* ============================================================
   GLOBAL BOOKING COUNTER (restored from file on load)
   ============================================================ */
static int bookingCounter = 1;

/* ============================================================
   UTILITY
   ============================================================ */
static float euclidean(int x1, int y1, int x2, int y2) {
    return sqrtf((float)((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)));
}

/* ============================================================
   B+ TREE – GENERIC NODE CREATION
   ============================================================ */
BPNode *bpNewNode(int is_leaf) {
    BPNode *n = calloc(1, sizeof(BPNode));
    if (!n) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    n->is_leaf  = is_leaf;
    n->num_keys = 0;
    n->next     = NULL;
    return n;
}

/* ============================================================
   B+ TREE – DRIVER  (keyed on d_ID)
   ============================================================ */

/* Find the leaf that should contain key k */
static BPNode *drvFindLeaf(BPNode *root, int k) {
    BPNode *cur = root;
    while (!cur->is_leaf) {
        int i = 0;
        while (i < cur->num_keys && k >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    return cur;
}

/* Insert into a non-full leaf */
static void drvLeafInsert(BPNode *leaf, int key, Driver *d) {
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i+1]    = leaf->keys[i];
        leaf->drv_data[i+1]= leaf->drv_data[i];
        i--;
    }
    leaf->keys[i+1]    = key;
    leaf->drv_data[i+1]= d;
    leaf->num_keys++;
}

/* Split a full child and push median up to parent */
static void drvSplitChild2(BPNode *parent, int ci, BPNode *child) {
    int mid = ORDER / 2;
    BPNode *sibling = bpNewNode(child->is_leaf);

    if (child->is_leaf) {
        /* Copy right half to sibling */
        sibling->num_keys = child->num_keys - mid;
        for (int i = 0; i < sibling->num_keys; i++) {
            sibling->keys[i]    = child->keys[mid + i];
            sibling->drv_data[i]= child->drv_data[mid + i];
        }
        child->num_keys = mid;
        /* Link leaves */
        sibling->next = child->next;
        child->next   = sibling;
        /* Push up first key of sibling */
        int pushKey = sibling->keys[0];
        /* Shift parent */
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]     = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]     = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    } else {
        /* Internal node split */
        int pushKey = child->keys[mid];
        sibling->num_keys = child->num_keys - mid - 1;
        for (int i = 0; i < sibling->num_keys; i++)
            sibling->keys[i] = child->keys[mid + 1 + i];
        for (int i = 0; i <= sibling->num_keys; i++)
            sibling->children[i] = child->children[mid + 1 + i];
        child->num_keys = mid;
        /* Shift parent */
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]     = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]     = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    }
}

/* Recursive insert – returns 1 if node is now full (needs split from above) */
static void drvInsertRec(BPNode **nodePtr, int key, Driver *d) {
    BPNode *node = *nodePtr;
    if (node->is_leaf) {
        drvLeafInsert(node, key, d);
    } else {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        drvInsertRec(&node->children[i], key, d);
        if (node->children[i]->num_keys == ORDER) {
            drvSplitChild2(node, i, node->children[i]);
        }
    }
}

void drvInsert(DriverTree *T, Driver *d) {
    if (!T->root) {
        T->root = bpNewNode(1);
    }
    /* Check if root is full */
    if (T->root->num_keys == ORDER - 1) {
        BPNode *newRoot = bpNewNode(0);
        newRoot->children[0] = T->root;
        drvSplitChild2(newRoot, 0, T->root);
        T->root = newRoot;
    }
    /* Now root is guaranteed not full */
    drvInsertRec(&T->root, d->d_ID, d);
}

Driver *drvSearch(DriverTree *T, int id) {
    if (!T->root) return NULL;
    BPNode *leaf = drvFindLeaf(T->root, id);
    for (int i = 0; i < leaf->num_keys; i++)
        if (leaf->keys[i] == id) return leaf->drv_data[i];
    return NULL;
}

/* In-order traversal of all leaf nodes */
void drvInorder(BPNode *node, void(*fn)(Driver*)) {
    if (!node) return;
    if (node->is_leaf) {
        for (int i = 0; i < node->num_keys; i++)
            fn(node->drv_data[i]);
        return;
    }
    drvInorder(node->children[0], fn);
}

/* Delete from leaf layer (simple scan + compact) */
void drvDelete(DriverTree *T, int id) {
    if (!T->root) return;
    BPNode *leaf = drvFindLeaf(T->root, id);
    int found = -1;
    for (int i = 0; i < leaf->num_keys; i++)
        if (leaf->keys[i] == id) { found = i; break; }
    if (found == -1) return;
    free(leaf->drv_data[found]);
    for (int i = found; i < leaf->num_keys - 1; i++) {
        leaf->keys[i]    = leaf->keys[i+1];
        leaf->drv_data[i]= leaf->drv_data[i+1];
    }
    leaf->num_keys--;
}

/* ============================================================
   B+ TREE – PASSENGER  (keyed on p_ID)
   ============================================================ */

static BPNode *paxFindLeaf(BPNode *root, int k) {
    BPNode *cur = root;
    while (!cur->is_leaf) {
        int i = 0;
        while (i < cur->num_keys && k >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    return cur;
}

static void paxLeafInsert(BPNode *leaf, int key, Passenger *p) {
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i+1]    = leaf->keys[i];
        leaf->pax_data[i+1]= leaf->pax_data[i];
        i--;
    }
    leaf->keys[i+1]    = key;
    leaf->pax_data[i+1]= p;
    leaf->num_keys++;
}

static void paxSplitChild(BPNode *parent, int ci, BPNode *child) {
    int mid = ORDER / 2;
    BPNode *sibling = bpNewNode(child->is_leaf);
    if (child->is_leaf) {
        sibling->num_keys = child->num_keys - mid;
        for (int i = 0; i < sibling->num_keys; i++) {
            sibling->keys[i]    = child->keys[mid + i];
            sibling->pax_data[i]= child->pax_data[mid + i];
        }
        child->num_keys  = mid;
        sibling->next    = child->next;
        child->next      = sibling;
        int pushKey      = sibling->keys[0];
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]      = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]      = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    } else {
        int pushKey = child->keys[mid];
        sibling->num_keys = child->num_keys - mid - 1;
        for (int i = 0; i < sibling->num_keys; i++)
            sibling->keys[i] = child->keys[mid + 1 + i];
        for (int i = 0; i <= sibling->num_keys; i++)
            sibling->children[i] = child->children[mid + 1 + i];
        child->num_keys = mid;
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]      = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]      = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    }
}

static void paxInsertRec(BPNode **nodePtr, int key, Passenger *p) {
    BPNode *node = *nodePtr;
    if (node->is_leaf) {
        paxLeafInsert(node, key, p);
    } else {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        paxInsertRec(&node->children[i], key, p);
        if (node->children[i]->num_keys == ORDER) {
            paxSplitChild(node, i, node->children[i]);
        }
    }
}

void paxInsert(PassengerTree *T, Passenger *p) {
    if (!T->root) T->root = bpNewNode(1);
    if (T->root->num_keys == ORDER - 1) {
        BPNode *newRoot = bpNewNode(0);
        newRoot->children[0] = T->root;
        paxSplitChild(newRoot, 0, T->root);
        T->root = newRoot;
    }
    paxInsertRec(&T->root, p->p_ID, p);
}

Passenger *paxSearch(PassengerTree *T, int id) {
    if (!T->root) return NULL;
    BPNode *leaf = paxFindLeaf(T->root, id);
    for (int i = 0; i < leaf->num_keys; i++)
        if (leaf->keys[i] == id) return leaf->pax_data[i];
    return NULL;
}

/* Range search on passenger IDs [id1, id2] using leaf-level linked list */
void paxRangeSearch(PassengerTree *T, int id1, int id2) {
    if (!T->root) { printf("Passenger database is empty.\n"); return; }
    if (id1 > id2) { printf("Invalid range: P_ID1 must be <= P_ID2.\n"); return; }
    /* Find first leaf that could contain id1 */
    BPNode *leaf = paxFindLeaf(T->root, id1);
    int found = 0;
    printf("%-6s %-20s %-15s %-8s\n", "P_ID", "Name", "Mobile", "Rides");
    printf("------------------------------------------------------------\n");
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            if (leaf->keys[i] > id2) goto done;
            if (leaf->keys[i] >= id1) {
                Passenger *p = leaf->pax_data[i];
                printf("%-6d %-20s %-15s %-8d\n",
                       p->p_ID, p->name, p->mobile, p->frequency);
                found++;
            }
        }
        leaf = leaf->next;
    }
done:
    if (!found) printf("No passengers found in range [%d, %d].\n", id1, id2);
    else printf("------------------------------------------------------------\n");
}

/* ============================================================
   B+ TREE – BOOKING  (keyed on booking_id)
   ============================================================ */

static BPNode *bkFindLeaf(BPNode *root, int k) {
    BPNode *cur = root;
    while (!cur->is_leaf) {
        int i = 0;
        while (i < cur->num_keys && k >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    return cur;
}

static void bkLeafInsert(BPNode *leaf, int key, Booking *b) {
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i+1]   = leaf->keys[i];
        leaf->bk_data[i+1]= leaf->bk_data[i];
        i--;
    }
    leaf->keys[i+1]   = key;
    leaf->bk_data[i+1]= b;
    leaf->num_keys++;
}

static void bkSplitChild(BPNode *parent, int ci, BPNode *child) {
    int mid = ORDER / 2;
    BPNode *sibling = bpNewNode(child->is_leaf);
    if (child->is_leaf) {
        sibling->num_keys = child->num_keys - mid;
        for (int i = 0; i < sibling->num_keys; i++) {
            sibling->keys[i]   = child->keys[mid + i];
            sibling->bk_data[i]= child->bk_data[mid + i];
        }
        child->num_keys   = mid;
        sibling->next     = child->next;
        child->next       = sibling;
        int pushKey       = sibling->keys[0];
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]      = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]      = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    } else {
        int pushKey = child->keys[mid];
        sibling->num_keys = child->num_keys - mid - 1;
        for (int i = 0; i < sibling->num_keys; i++)
            sibling->keys[i] = child->keys[mid + 1 + i];
        for (int i = 0; i <= sibling->num_keys; i++)
            sibling->children[i] = child->children[mid + 1 + i];
        child->num_keys = mid;
        for (int i = parent->num_keys; i > ci; i--) {
            parent->keys[i]      = parent->keys[i-1];
            parent->children[i+1]= parent->children[i];
        }
        parent->keys[ci]      = pushKey;
        parent->children[ci+1]= sibling;
        parent->num_keys++;
    }
}

static void bkInsertRec(BPNode **nodePtr, int key, Booking *b) {
    BPNode *node = *nodePtr;
    if (node->is_leaf) {
        bkLeafInsert(node, key, b);
    } else {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        bkInsertRec(&node->children[i], key, b);
        if (node->children[i]->num_keys == ORDER) {
            bkSplitChild(node, i, node->children[i]);
        }
    }
}

void bkInsert(BookingTree *T, Booking *b) {
    if (!T->root) T->root = bpNewNode(1);
    if (T->root->num_keys == ORDER - 1) {
        BPNode *newRoot = bpNewNode(0);
        newRoot->children[0] = T->root;
        bkSplitChild(newRoot, 0, T->root);
        T->root = newRoot;
    }
    bkInsertRec(&T->root, b->booking_id, b);
}

Booking *bkSearch(BookingTree *T, int id) {
    if (!T->root) return NULL;
    BPNode *leaf = bkFindLeaf(T->root, id);
    for (int i = 0; i < leaf->num_keys; i++)
        if (leaf->keys[i] == id) return leaf->bk_data[i];
    return NULL;
}

/* ============================================================
   APPLICATION LAYER
   ============================================================ */

void addDriver(DriverTree *T, int id, char name[], int type, int x, int y) {
    if (type != 0 && type != 1) {
        printf("ERROR: Invalid vehicle type! Use 0 for Cab, 1 for Bike.\n");
        return;
    }
    if (drvSearch(T, id)) {
        printf("ERROR: Driver with ID %d already exists!\n", id);
        return;
    }
    Driver *d = malloc(sizeof(Driver));
    if (!d) { printf("ERROR: Memory allocation failed!\n"); return; }
    d->d_ID          = id;
    strncpy(d->name, name, 49); d->name[49] = '\0';
    d->vehicle_type  = type;
    d->x             = x;
    d->y             = y;
    d->status        = 0;
    d->total_earnings= 0.0f;
    drvInsert(T, d);
    printf("SUCCESS: Driver '%s' (ID:%d, %s) added at location (%d,%d).\n",
           d->name, d->d_ID, type==0?"Cab":"Bike", x, y);
}

void addPassenger(PassengerTree *T, int id, char name[], char mobile[]) {
    if (paxSearch(T, id)) {
        printf("ERROR: Passenger with ID %d already exists!\n", id);
        return;
    }
    /* Check duplicate mobile by scanning all passengers */
    if (T->root) {
        BPNode *leaf = T->root;
        while (!leaf->is_leaf) leaf = leaf->children[0];
        while (leaf) {
            for (int i = 0; i < leaf->num_keys; i++)
                if (strcmp(leaf->pax_data[i]->mobile, mobile) == 0) {
                    printf("ERROR: Mobile number '%s' is already registered!\n", mobile);
                    return;
                }
            leaf = leaf->next;
        }
    }
    if (strlen(mobile) == 0) {
        printf("ERROR: Mobile number cannot be empty!\n");
        return;
    }
    Passenger *p = malloc(sizeof(Passenger));
    if (!p) { printf("ERROR: Memory allocation failed!\n"); return; }
    p->p_ID     = id;
    strncpy(p->name, name, 49);   p->name[49]   = '\0';
    strncpy(p->mobile, mobile, 19); p->mobile[19] = '\0';
    p->frequency = 0;
    paxInsert(T, p);
    printf("SUCCESS: Passenger '%s' (ID:%d, Mobile:%s) added.\n",
           p->name, p->p_ID, p->mobile);
}

Driver *findNearestVehicle(DriverTree *T, int px, int py, int pref) {
    if (!T->root) return NULL;
    Driver *best  = NULL;
    float   minD  = 1e9f;
    /* Scan all leaf-level drivers */
    BPNode *leaf = T->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Driver *d = leaf->drv_data[i];
            if (d->status != 0) continue;
            if (pref != -1 && d->vehicle_type != pref) continue;
            float dist = euclidean(px, py, d->x, d->y);
            if (dist <= 5.0f && dist < minD) {
                minD = dist; best = d;
            }
        }
        leaf = leaf->next;
    }
    return best;
}

int requestRide(DriverTree *DT, PassengerTree *PT, BookingTree *BT,
                int p_id, int px, int py, int pref) {
    if (pref != -1 && pref != 0 && pref != 1) {
        printf("ERROR: Invalid preference! Use -1 (any), 0 (cab), 1 (bike).\n");
        return -1;
    }
    Passenger *p = paxSearch(PT, p_id);
    if (!p) {
        printf("ERROR: Passenger with ID %d not found in database!\n", p_id);
        return -1;
    }
    Driver *d = findNearestVehicle(DT, px, py, pref);
    if (!d) {
        if (pref == -1)
            printf("ERROR: No free vehicle found within 5 km of (%d,%d).\n", px, py);
        else
            printf("ERROR: No free %s found within 5 km of (%d,%d).\n",
                   pref==0?"cab":"bike", px, py);
        return -1;
    }
    Booking *b = malloc(sizeof(Booking));
    if (!b) { printf("ERROR: Memory allocation failed!\n"); return -1; }
    b->booking_id   = bookingCounter++;
    b->d_ID         = d->d_ID;
    b->p_ID         = p_id;
    b->vehicle_type = d->vehicle_type;
    b->distance     = 0.0f;
    b->fare         = 0.0f;
    d->status       = 1;
    bkInsert(BT, b);
    printf("SUCCESS: Ride booked! Booking ID: %d | Driver: %s (ID:%d, %s) | Passenger: %s (ID:%d).\n",
           b->booking_id, d->name, d->d_ID,
           d->vehicle_type==0?"Cab":"Bike",
           p->name, p_id);
    return b->booking_id;
}

void completeRide(DriverTree *DT, PassengerTree *PT, BookingTree *BT,
                  int bid, float distance) {
    if (distance < 0) {
        printf("ERROR: Distance cannot be negative!\n");
        return;
    }
    Booking *b = bkSearch(BT, bid);
    if (!b) {
        printf("ERROR: Booking ID %d not found!\n", bid);
        return;
    }
    if (b->distance > 0) {
        printf("ERROR: Booking ID %d has already been completed (distance=%.2f km).\n",
               bid, b->distance);
        return;
    }
    if (distance == 0) {
        printf("WARNING: Distance is 0 km — ride completed with zero fare.\n");
    }
    b->distance = distance;
    b->fare     = distance * (b->vehicle_type == 0 ? 10.0f : 5.0f);

    Driver    *d = drvSearch(DT, b->d_ID);
    Passenger *p = paxSearch(PT, b->p_ID);

    if (d) { d->total_earnings += b->fare; d->status = 0; }
    else   printf("WARNING: Driver ID %d not found for earnings update.\n", b->d_ID);

    if (p) p->frequency++;
    else   printf("WARNING: Passenger ID %d not found for frequency update.\n", b->p_ID);

    printf("SUCCESS: Ride (ID:%d) completed! Distance=%.2f km | Fare=%.2f | Driver=%s | Passenger=%s.\n",
           bid, distance, b->fare,
           d ? d->name : "Unknown",
           p ? p->name : "Unknown");
}

float calculateDriverEarnings(BookingTree *BT, int d_id) {
    float total = 0.0f;
    if (!BT->root) return total;
    BPNode *leaf = BT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++)
            if (leaf->bk_data[i]->d_ID == d_id)
                total += leaf->bk_data[i]->fare;
        leaf = leaf->next;
    }
    return total;
}

/* Helpers for displayTopDrivers */
typedef struct { Driver *d; float earn; } DE;

void displayTopDrivers(DriverTree *DT, BookingTree *BT) {
    if (!DT->root) { printf("No drivers in database.\n"); return; }

    /* Collect all drivers into array */
    DE arr[1024]; int cnt = 0;
    BPNode *leaf = DT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf && cnt < 1024) {
        for (int i = 0; i < leaf->num_keys && cnt < 1024; i++) {
            arr[cnt].d    = leaf->drv_data[i];
            arr[cnt].earn = calculateDriverEarnings(BT, arr[cnt].d->d_ID);
            /* Also sync total_earnings field */
            arr[cnt].d->total_earnings = arr[cnt].earn;
            cnt++;
        }
        leaf = leaf->next;
    }
    if (cnt == 0) { printf("No drivers to display.\n"); return; }

    /* Partial selection sort – pick top 3 */
    int top = cnt < 3 ? cnt : 3;
    printf("\n=== TOP %d DRIVER(S) BY EARNINGS ===\n", top);
    printf("%-4s %-20s %-10s %-10s\n", "Rank", "Name", "Type", "Earnings");
    printf("------------------------------------------------\n");
    int used[1024] = {0};
    for (int r = 0; r < top; r++) {
        int bi = -1; float mx = -1.0f;
        for (int i = 0; i < cnt; i++)
            if (!used[i] && arr[i].earn > mx) { mx = arr[i].earn; bi = i; }
        if (bi == -1) break;
        used[bi] = 1;
        printf("%-4d %-20s %-10s Rs.%.2f\n", r+1,
               arr[bi].d->name,
               arr[bi].d->vehicle_type==0?"Cab":"Bike",
               arr[bi].earn);
    }
}

void displayFrequentPairs(DriverTree *DT, PassengerTree *PT, BookingTree *BT) {
    if (!BT->root) { printf("No bookings in history.\n"); return; }

    /* Collect all bookings */
    Booking *bArr[4096]; int bn = 0;
    BPNode *leaf = BT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf && bn < 4096) {
        for (int i = 0; i < leaf->num_keys && bn < 4096; i++)
            bArr[bn++] = leaf->bk_data[i];
        leaf = leaf->next;
    }
    if (bn == 0) { printf("No bookings found.\n"); return; }

    int max = 0, bd = -1, bp = -1;
    for (int i = 0; i < bn; i++) {
        int cnt = 0;
        for (int j = 0; j < bn; j++)
            if (bArr[i]->d_ID == bArr[j]->d_ID && bArr[i]->p_ID == bArr[j]->p_ID) cnt++;
        if (cnt > max) { max = cnt; bd = bArr[i]->d_ID; bp = bArr[i]->p_ID; }
    }
    Driver    *d = drvSearch(DT, bd);
    Passenger *p = paxSearch(PT, bp);
    printf("\n=== MOST FREQUENT PAIR ===\n");
    printf("Driver   : %s (ID:%d)\n", d ? d->name : "Unknown", bd);
    printf("Passenger: %s (ID:%d)\n", p ? p->name : "Unknown", bp);
    printf("Rides Together: %d\n", max);
}

void displayAvailableVehicles(DriverTree *DT) {
    if (!DT->root) { printf("No drivers in database.\n"); return; }
    printf("\n=== AVAILABLE VEHICLES ===\n");
    printf("%-6s %-20s %-6s %-12s\n", "D_ID", "Name", "Type", "Location");
    printf("--------------------------------------------------\n");
    int found = 0;
    BPNode *leaf = DT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Driver *d = leaf->drv_data[i];
            if (d->status == 0) {
                printf("%-6d %-20s %-6s (%d,%d)\n",
                       d->d_ID, d->name,
                       d->vehicle_type==0?"Cab":"Bike",
                       d->x, d->y);
                found++;
            }
        }
        leaf = leaf->next;
    }
    if (!found) printf("No available vehicles at this time.\n");
}

void updateDriverLocation(DriverTree *DT, int id, int x, int y) {
    Driver *d = drvSearch(DT, id);
    if (!d) { printf("ERROR: Driver with ID %d not found!\n", id); return; }
    if (d->status == 1) {
        printf("ERROR: Driver '%s' (ID:%d) is currently on a ride. Cannot update location.\n",
               d->name, id);
        return;
    }
    d->x = x; d->y = y;
    printf("SUCCESS: Driver '%s' (ID:%d) location updated to (%d,%d).\n",
           d->name, id, x, y);
}

void deleteDriver(DriverTree *DT, int id) {
    Driver *d = drvSearch(DT, id);
    if (!d) { printf("ERROR: Driver with ID %d not found!\n", id); return; }
    if (d->status == 1) {
        printf("ERROR: Cannot delete Driver '%s' (ID:%d) — currently on a ride!\n",
               d->name, id);
        return;
    }
    printf("SUCCESS: Driver '%s' (ID:%d) removed from the system.\n", d->name, id);
    drvDelete(DT, id);
}

void displayBookingHistory(BookingTree *BT) {
    if (!BT->root) { printf("No booking history available.\n"); return; }
    printf("\n=== BOOKING HISTORY ===\n");
    printf("%-8s %-8s %-8s %-6s %-10s %-10s\n",
           "Bk_ID", "D_ID", "P_ID", "Type", "Distance", "Fare");
    printf("------------------------------------------------------\n");
    int cnt = 0;
    BPNode *leaf = BT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Booking *b = leaf->bk_data[i];
            printf("%-8d %-8d %-8d %-6s %-10.2f Rs.%-8.2f\n",
                   b->booking_id, b->d_ID, b->p_ID,
                   b->vehicle_type==0?"Cab":"Bike",
                   b->distance, b->fare);
            cnt++;
        }
        leaf = leaf->next;
    }
    if (!cnt) printf("No bookings found.\n");
    else printf("------------------------------------------------------\n");
}

void rangeSearchPassengers(PassengerTree *PT, int id1, int id2) {
    printf("\n=== RANGE SEARCH: Passengers with P_ID in [%d, %d] ===\n", id1, id2);
    paxRangeSearch(PT, id1, id2);
}

/* ============================================================
   PERSISTENCE
   ============================================================ */

void saveDrivers(DriverTree *DT) {
    FILE *fp = fopen("drivers.txt", "w");
    if (!fp) { printf("ERROR: Cannot open drivers.txt for writing!\n"); return; }
    if (!DT->root) { fclose(fp); return; }
    BPNode *leaf = DT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Driver *d = leaf->drv_data[i];
            fprintf(fp, "%d %s %d %d %d %d %.2f\n",
                    d->d_ID, d->name, d->vehicle_type,
                    d->x, d->y, d->status, d->total_earnings);
        }
        leaf = leaf->next;
    }
    fclose(fp);
}

void loadDrivers(DriverTree *DT) {
    FILE *fp = fopen("drivers.txt", "r");
    if (!fp) return;
    int id, t, x, y, s; float e; char name[50];
    while (fscanf(fp, "%d %49s %d %d %d %d %f", &id, name, &t, &x, &y, &s, &e) == 7) {
        Driver *d = malloc(sizeof(Driver));
        if (!d) break;
        d->d_ID = id; strcpy(d->name, name);
        d->vehicle_type = t; d->x = x; d->y = y;
        d->status = s; d->total_earnings = e;
        drvInsert(DT, d);
    }
    fclose(fp);
}

void savePassengers(PassengerTree *PT) {
    FILE *fp = fopen("passengers.txt", "w");
    if (!fp) { printf("ERROR: Cannot open passengers.txt for writing!\n"); return; }
    if (!PT->root) { fclose(fp); return; }
    BPNode *leaf = PT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Passenger *p = leaf->pax_data[i];
            fprintf(fp, "%d %s %s %d\n", p->p_ID, p->name, p->mobile, p->frequency);
        }
        leaf = leaf->next;
    }
    fclose(fp);
}

void loadPassengers(PassengerTree *PT) {
    FILE *fp = fopen("passengers.txt", "r");
    if (!fp) return;
    int id, f; char name[50], mob[20];
    while (fscanf(fp, "%d %49s %19s %d", &id, name, mob, &f) == 4) {
        Passenger *p = malloc(sizeof(Passenger));
        if (!p) break;
        p->p_ID = id; strcpy(p->name, name);
        strcpy(p->mobile, mob); p->frequency = f;
        paxInsert(PT, p);
    }
    fclose(fp);
}

void saveBookings(BookingTree *BT) {
    FILE *fp = fopen("bookings.txt", "w");
    if (!fp) { printf("ERROR: Cannot open bookings.txt for writing!\n"); return; }
    fprintf(fp, "%d\n", bookingCounter);
    if (!BT->root) { fclose(fp); return; }
    BPNode *leaf = BT->root;
    while (!leaf->is_leaf) leaf = leaf->children[0];
    while (leaf) {
        for (int i = 0; i < leaf->num_keys; i++) {
            Booking *b = leaf->bk_data[i];
            fprintf(fp, "%d %d %d %d %.2f %.2f\n",
                    b->booking_id, b->d_ID, b->p_ID,
                    b->vehicle_type, b->distance, b->fare);
        }
        leaf = leaf->next;
    }
    fclose(fp);
}

void loadBookings(BookingTree *BT) {
    FILE *fp = fopen("bookings.txt", "r");
    if (!fp) { bookingCounter = 1; return; }
    if (fscanf(fp, "%d", &bookingCounter) != 1) bookingCounter = 1;
    int bid, did, pid, vt; float dist, fare;
    while (fscanf(fp, "%d %d %d %d %f %f", &bid, &did, &pid, &vt, &dist, &fare) == 6) {
        Booking *b = malloc(sizeof(Booking));
        if (!b) break;
        b->booking_id  = bid; b->d_ID = did; b->p_ID = pid;
        b->vehicle_type= vt; b->distance = dist; b->fare = fare;
        bkInsert(BT, b);
    }
    fclose(fp);
}
