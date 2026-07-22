#include "voting.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5

typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;

struct District {
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[PARTIES_SZ];
};

struct Station {
    int sid;
    int did;
    int registered;
    Voter* voters;
    Station* next;
};
struct Voter {
    int vid;
    bool voted;
    Voter* parent;
    Voter* lc;
    Voter* rc;
};

struct Party {
    int pid;
    int electedCount;
    Candidate* candidates;
};
struct Candidate {
    int cid;
    int did;
    int votes;
    bool isElected;
    Candidate* lc;
    Candidate* rc;
};

struct ElectedCandidate {
    int cid;
    int did;
    int pid;
    ElectedCandidate* next;
};

District Districts[DISTRICTS_SZ];
Station** StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate* Parliament;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123
};
int MaxStationsCount;
int MaxSid;

/* ===================== extra state used internally by voting.c ===================== */
/* none of this is part of the required structs, it's just bookkeeping I needed
   to get the required complexities (see report). */

static int HashP;          // prime modulus for the a*x+b universal hash family
static int HashTableSz;    // actual capacity of StationsHT (a prime >= MaxStationsCount)
static int HashA, HashB;   // hash coefficients

static int FreeDistrictSlots[DISTRICTS_SZ]; // min-heap of unused indices in Districts[]
static int FreeDistrictSlotsCount;

typedef struct {
    int cid;
    int pid;
    int votes;
} ElectedRow; // just used to build the printout for the M event

/* ===================== small generic helpers ===================== */

static int NextPrimeAtLeast(int n) {
    for (int i = 0; i < PRIMES_SZ; i++) {
        if (Primes[i] >= 2 && Primes[i] >= n) return Primes[i];
    }
    return Primes[PRIMES_SZ - 1];
}

static int NextPrimeGreaterThan(int n) {
    for (int i = 0; i < PRIMES_SZ; i++) {
        if (Primes[i] >= 2 && Primes[i] > n) return Primes[i];
    }
    return Primes[PRIMES_SZ - 1];
}

static int HashSid(int sid) {
    long long v = ((long long)HashA * sid + HashB) % HashP;
    return (int)(v % HashTableSz);
}

static void PrintCommaList(const int* vals, int n) {
    for (int i = 0; i < n; i++) {
        if (i > 0) printf(", ");
        printf("%d", vals[i]);
    }
    printf("\n");
}

/* ---- min-heap of free Districts[] slots, used so 'D' can find an empty
   cell in O(log n) instead of scanning the array. Since the slots start
   out equal to their own index (0,1,2,...), the array is already a valid
   min-heap right after Announce Elections, we only ever extract-min. ---- */

static void SiftDownFreeSlots(int i) {
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < FreeDistrictSlotsCount && FreeDistrictSlots[l] < FreeDistrictSlots[smallest]) smallest = l;
        if (r < FreeDistrictSlotsCount && FreeDistrictSlots[r] < FreeDistrictSlots[smallest]) smallest = r;
        if (smallest == i) return;
        int tmp = FreeDistrictSlots[i];
        FreeDistrictSlots[i] = FreeDistrictSlots[smallest];
        FreeDistrictSlots[smallest] = tmp;
        i = smallest;
    }
}

static int ExtractFreeDistrictSlot(void) {
    int min = FreeDistrictSlots[0];
    FreeDistrictSlotsCount--;
    FreeDistrictSlots[0] = FreeDistrictSlots[FreeDistrictSlotsCount];
    SiftDownFreeSlots(0);
    return min;
}

static District* FindDistrictByDid(int did) {
    for (int i = 0; i < DISTRICTS_SZ; i++) {
        if (Districts[i].did == did) return &Districts[i];
    }
    return NULL;
}

static Station* FindStationBySid(int sid) {
    Station* cur = StationsHT[HashSid(sid)];
    while (cur != NULL && cur->sid != sid) cur = cur->next;
    return cur;
}

/* ---- voters tree: complete, unsorted binary tree. To keep it complete on
   insert/delete we address nodes by their position in level order (1 =
   root, 2/3 = its children, ...) using the binary representation of that
   position: after dropping the leading 1 bit, each remaining bit says
   "go left" (0) or "go right" (1) to reach the parent of that position. ---- */

static int HighestSetBit(int v) {
    int pos = 0;
    while (v >>= 1) pos++;
    return pos;
}

static Voter* GetParentAtIndex(Voter* root, int idx) {
    int top = HighestSetBit(idx);
    Voter* cur = root;
    for (int b = top - 1; b >= 1; b--) {
        cur = ((idx >> b) & 1) ? cur->rc : cur->lc;
    }
    return cur;
}

static void CollectVoterVids(Voter* root, int* vids, int* count) {
    if (root == NULL) return;
    CollectVoterVids(root->lc, vids, count);
    vids[(*count)++] = root->vid;
    CollectVoterVids(root->rc, vids, count);
}

static Voter* FindVoterAnywhere(Voter* root, int vid) {
    if (root == NULL) return NULL;
    if (root->vid == vid) return root;
    Voter* found = FindVoterAnywhere(root->lc, vid);
    if (found != NULL) return found;
    return FindVoterAnywhere(root->rc, vid);
}

static void FreeVoterTree(Voter* root) {
    if (root == NULL) return;
    FreeVoterTree(root->lc);
    FreeVoterTree(root->rc);
    free(root);
}

static void PrintVotersInorderHelper(Voter* root, int* idx, int total) {
    if (root == NULL) return;
    PrintVotersInorderHelper(root->lc, idx, total);
    (*idx)++;
    printf("  %d %d%s\n", root->vid, root->voted, (*idx < total) ? "," : "");
    PrintVotersInorderHelper(root->rc, idx, total);
}

/* ---- candidates tree: plain BST on cid ---- */

static void InsertCandidateBST(Candidate** root, Candidate* node) {
    if (*root == NULL) {
        *root = node;
        return;
    }
    Candidate* cur = *root;
    for (;;) {
        if (node->cid < cur->cid) {
            if (cur->lc == NULL) { cur->lc = node; return; }
            cur = cur->lc;
        } else {
            if (cur->rc == NULL) { cur->rc = node; return; }
            cur = cur->rc;
        }
    }
}

static Candidate* FindCandidateByCid(Candidate* root, int cid) {
    while (root != NULL) {
        if (cid == root->cid) return root;
        root = (cid < root->cid) ? root->lc : root->rc;
    }
    return NULL;
}

static int CountCandidates(Candidate* root) {
    if (root == NULL) return 0;
    return 1 + CountCandidates(root->lc) + CountCandidates(root->rc);
}

static int CountCandidatesInDistrict(Candidate* root, int did) {
    if (root == NULL) return 0;
    int here = (root->did == did) ? 1 : 0;
    return here + CountCandidatesInDistrict(root->lc, did) + CountCandidatesInDistrict(root->rc, did);
}

static void PrintCandidatesInorderHelper(Candidate* root, int* idx, int total) {
    if (root == NULL) return;
    PrintCandidatesInorderHelper(root->lc, idx, total);
    (*idx)++;
    printf("  %d %d%s\n", root->cid, root->did, (*idx < total) ? "," : "");
    PrintCandidatesInorderHelper(root->rc, idx, total);
}

static int CountElectedCandidates(Candidate* root) {
    if (root == NULL) return 0;
    int here = root->isElected ? 1 : 0;
    return here + CountElectedCandidates(root->lc) + CountElectedCandidates(root->rc);
}

static void PrintElectedCandidatesHelper(Candidate* root, int* idx, int total) {
    if (root == NULL) return;
    PrintElectedCandidatesHelper(root->lc, idx, total);
    if (root->isElected) {
        (*idx)++;
        printf("  %d %d%s\n", root->cid, root->votes, (*idx < total) ? "," : "");
    }
    PrintElectedCandidatesHelper(root->rc, idx, total);
}

static void CollectElectedRows(Candidate* node, int pid, int did, ElectedRow* rows, int* count) {
    if (node == NULL) return;
    CollectElectedRows(node->lc, pid, did, rows, count);
    if (node->isElected && node->did == did) {
        rows[*count].cid = node->cid;
        rows[*count].pid = pid;
        rows[*count].votes = node->votes;
        (*count)++;
    }
    CollectElectedRows(node->rc, pid, did, rows, count);
}

static void FreeCandidateTree(Candidate* root) {
    if (root == NULL) return;
    FreeCandidateTree(root->lc);
    FreeCandidateTree(root->rc);
    free(root);
}

/* ---- election of a party's seats in one district: a min-heap keyed on
   votes, of exactly the size we need to elect. The first candidates we
   meet fill up the heap, then every later candidate only gets in if it
   beats the current smallest, which then gets kicked out. ---- */

static void SiftDownCandidateHeap(Candidate** heap, int size, int i) {
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < size && heap[l]->votes < heap[smallest]->votes) smallest = l;
        if (r < size && heap[r]->votes < heap[smallest]->votes) smallest = r;
        if (smallest == i) return;
        Candidate* tmp = heap[i]; heap[i] = heap[smallest]; heap[smallest] = tmp;
        i = smallest;
    }
}

static void CollectAndElect(Candidate* node, int did, int k, Candidate** heap, int* count) {
    if (node == NULL) return;
    CollectAndElect(node->lc, did, k, heap, count);
    if (node->did == did) {
        if (*count < k) {
            heap[*count] = node;
            (*count)++;
            if (*count == k) {
                for (int i = k / 2 - 1; i >= 0; i--) SiftDownCandidateHeap(heap, k, i);
            }
        } else if (node->votes > heap[0]->votes) {
            heap[0] = node;
            SiftDownCandidateHeap(heap, k, 0);
        }
    }
    CollectAndElect(node->rc, did, k, heap, count);
}

static void ElectPartyCandidatesInDistrict(int pid, int did, int k) {
    Candidate** heap = malloc(k * sizeof(Candidate*));
    int count = 0;
    CollectAndElect(Parties[pid].candidates, did, k, heap, &count);
    for (int i = 0; i < count; i++) heap[i]->isElected = true;
    free(heap);
}

/* ---- Parliament: a descending sorted list, built by merging each party's
   (already sorted) elected list into it. Only PARTIES_SZ merges happen in
   total, so even though each merge is a simple O(size) pass, the whole
   thing stays O(n) overall. ---- */

static void CollectPartyElectedDescending(Candidate* node, int pid, ElectedCandidate** headPtr) {
    if (node == NULL) return;
    CollectPartyElectedDescending(node->lc, pid, headPtr);
    if (node->isElected) {
        ElectedCandidate* e = malloc(sizeof(ElectedCandidate));
        e->cid = node->cid;
        e->did = node->did;
        e->pid = pid;
        e->next = *headPtr;
        *headPtr = e; // visiting cid's in ascending order + always prepending -> descending list
    }
    CollectPartyElectedDescending(node->rc, pid, headPtr);
}

static ElectedCandidate* MergeDescending(ElectedCandidate* a, ElectedCandidate* b) {
    ElectedCandidate dummy;
    dummy.next = NULL;
    ElectedCandidate* tail = &dummy;
    while (a != NULL && b != NULL) {
        if (a->cid >= b->cid) { tail->next = a; a = a->next; }
        else { tail->next = b; b = b->next; }
        tail = tail->next;
    }
    tail->next = (a != NULL) ? a : b;
    return dummy.next;
}

/* ===================== event handlers ===================== */

void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid) {
    DebugPrint("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);

    MaxStationsCount = parsedMaxStationsCount;
    MaxSid = parsedMaxSid;

    for (int i = 0; i < DISTRICTS_SZ; i++) {
        Districts[i].did = DefaultDid;
        Districts[i].seats = 0;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        for (int j = 0; j < PARTIES_SZ; j++) Districts[i].partyVotes[j] = 0;
        FreeDistrictSlots[i] = i;
    }
    FreeDistrictSlotsCount = DISTRICTS_SZ;

    // Universal hashing family h(k) = ((a*k + b) mod p) mod m.
    // p has to be a prime bigger than any sid we'll ever hash, m is the
    // actual table size (also a prime, to spread out chains evenly).
    HashP = NextPrimeGreaterThan(MaxSid > 0 ? MaxSid : 1);
    HashTableSz = NextPrimeAtLeast(MaxStationsCount > 0 ? MaxStationsCount : 1);
    HashA = 1 + rand() % (HashP - 1);
    HashB = rand() % HashP;

    StationsHT = malloc(HashTableSz * sizeof(Station*));
    for (int i = 0; i < HashTableSz; i++) StationsHT[i] = NULL;

    for (int i = 0; i < PARTIES_SZ; i++) {
        Parties[i].pid = i;
        Parties[i].electedCount = 0;
        Parties[i].candidates = NULL;
    }
    Parliament = NULL;

    printf("A %d %d\n", MaxStationsCount, MaxSid);
    printf("DONE\n");
}

void EventCreateDistrict(int did, int seats) {
    DebugPrint("D %d %d\n", did, seats);

    int slot = ExtractFreeDistrictSlot();
    Districts[slot].did = did;
    Districts[slot].seats = seats;
    Districts[slot].blanks = 0;
    Districts[slot].invalids = 0;
    for (int j = 0; j < PARTIES_SZ; j++) Districts[slot].partyVotes[j] = 0;

    printf("D %d %d\n", did, seats);
    printf("  Districts\n  ");
    int first = 1;
    for (int i = 0; i < DISTRICTS_SZ; i++) {
        if (Districts[i].did == DefaultDid) continue;
        if (!first) printf(", ");
        printf("%d", Districts[i].did);
        first = 0;
    }
    printf("\n");
    printf("DONE\n");
}

void EventCreateStation(int sid, int did) {
    DebugPrint("S %d %d\n", sid, did);

    Station* node = malloc(sizeof(Station));
    node->sid = sid;
    node->did = did;
    node->registered = 0;
    node->voters = NULL;
    node->next = NULL;

    int h = HashSid(sid);
    if (StationsHT[h] == NULL || sid < StationsHT[h]->sid) {
        node->next = StationsHT[h];
        StationsHT[h] = node;
    } else {
        Station* cur = StationsHT[h];
        while (cur->next != NULL && cur->next->sid < sid) cur = cur->next;
        node->next = cur->next;
        cur->next = node;
    }

    printf("S %d %d\n", sid, did);
    printf("  Stations[%d]\n  ", h);
    int first = 1;
    for (Station* cur = StationsHT[h]; cur != NULL; cur = cur->next) {
        if (!first) printf(", ");
        printf("%d", cur->sid);
        first = 0;
    }
    printf("\n");
    printf("DONE\n");
}

void EventRegisterVoter(int vid, int sid) {
    DebugPrint("R %d %d\n", vid, sid);

    Voter* node = malloc(sizeof(Voter));
    node->vid = vid;
    node->voted = false;
    node->parent = NULL;
    node->lc = NULL;
    node->rc = NULL;

    Station* station = FindStationBySid(sid);
    int n = station->registered;
    if (n == 0) {
        station->voters = node;
    } else {
        int idx = n + 1;
        Voter* parent = GetParentAtIndex(station->voters, idx);
        node->parent = parent;
        if (idx & 1) parent->rc = node; else parent->lc = node;
    }
    station->registered++;

    printf("R %d %d\n", vid, sid);
    printf("  Voters[%d]\n  ", sid);
    int vids[station->registered];
    int count = 0;
    CollectVoterVids(station->voters, vids, &count);
    PrintCommaList(vids, count);
    printf("DONE\n");
}

void EventRegisterCandidate(int cid, int pid, int did) {
    DebugPrint("C %d %d %d\n", cid, pid, did);

    Candidate* node = malloc(sizeof(Candidate));
    node->cid = cid;
    node->did = did;
    node->votes = 0;
    node->isElected = false;
    node->lc = NULL;
    node->rc = NULL;

    InsertCandidateBST(&Parties[pid].candidates, node);

    printf("C %d %d %d\n", cid, pid, did);
    printf("  Candidates[%d]\n", pid);
    int total = CountCandidates(Parties[pid].candidates);
    int idx = 0;
    PrintCandidatesInorderHelper(Parties[pid].candidates, &idx, total);
    printf("DONE\n");
}

void EventVote(int vid, int sid, int cid, int pid) {
    DebugPrint("V %d %d %d %d\n", vid, sid, cid, pid);

    Station* station = FindStationBySid(sid);
    Voter* voter = FindVoterAnywhere(station->voters, vid);
    voter->voted = true;

    int printDid;
    District* dist;
    if (cid == BlankDid) {
        dist = FindDistrictByDid(station->did);
        dist->blanks++;
        printDid = station->did;
    } else if (cid == InvalidDid) {
        dist = FindDistrictByDid(station->did);
        dist->invalids++;
        printDid = station->did;
    } else {
        // votes for a real candidate are tallied in the candidate's own
        // district, not the voter's station district (see report)
        Candidate* cand = FindCandidateByCid(Parties[pid].candidates, cid);
        cand->votes++;
        dist = FindDistrictByDid(cand->did);
        dist->partyVotes[pid]++;
        printDid = cand->did;
    }

    printf("V %d %d %d %d\n", vid, sid, cid, pid);
    printf("  District[%d]\n", printDid);
    printf("  blanks %d\n", dist->blanks);
    printf("  invalids %d\n", dist->invalids);
    printf("  partyVotes\n");
    for (int p = 0; p < PARTIES_SZ; p++) {
        printf("  %d %d%s\n", p, dist->partyVotes[p], (p < PARTIES_SZ - 1) ? "," : "");
    }
    printf("DONE\n");
}

void EventCountVotes(int did) {
    DebugPrint("M %d\n", did);

    District* dist = FindDistrictByDid(did);
    int originalSeats = dist->seats;

    int totalValidVotes = 0;
    for (int p = 0; p < PARTIES_SZ; p++) totalValidVotes += dist->partyVotes[p];
    int quota = (originalSeats > 0) ? (totalValidVotes / originalSeats) : 0;

    int partyElected[PARTIES_SZ];
    for (int p = 0; p < PARTIES_SZ; p++) {
        int e = (quota > 0) ? (dist->partyVotes[p] / quota) : 0;
        Parties[p].electedCount += e;
        dist->seats -= e;

        int available = CountCandidatesInDistrict(Parties[p].candidates, did);
        if (e > available) {
            int diff = e - available;
            e = available;
            Parties[p].electedCount -= diff;
            dist->seats += diff;
        }
        partyElected[p] = e;
    }

    for (int p = 0; p < PARTIES_SZ; p++) {
        if (partyElected[p] > 0) ElectPartyCandidatesInDistrict(p, did, partyElected[p]);
    }

    printf("M %d\n", did);
    printf("  seats\n");
    int cap = (originalSeats > 0) ? originalSeats : 1;
    ElectedRow rows[cap];
    int count = 0;
    for (int p = 0; p < PARTIES_SZ; p++) {
        CollectElectedRows(Parties[p].candidates, p, did, rows, &count);
    }
    for (int i = 0; i < count; i++) {
        printf("  %d %d %d%s\n", rows[i].cid, rows[i].pid, rows[i].votes, (i < count - 1) ? "," : "");
    }
    printf("DONE\n");
}

void EventFormParliament(void) {
    DebugPrint("N\n");

    for (int p = 0; p < PARTIES_SZ; p++) {
        ElectedCandidate* partyList = NULL;
        CollectPartyElectedDescending(Parties[p].candidates, p, &partyList);
        Parliament = MergeDescending(Parliament, partyList);
    }

    printf("N\n");
    printf("  members\n");
    for (ElectedCandidate* cur = Parliament; cur != NULL; cur = cur->next) {
        printf("  %d %d %d%s\n", cur->cid, cur->pid, cur->did, (cur->next != NULL) ? "," : "");
    }
    printf("DONE\n");
}

void EventPrintDistrict(int did) {
    DebugPrint("I %d\n", did);

    District* dist = FindDistrictByDid(did);
    printf("I %d\n", did);
    printf("  seats %d\n", dist->seats);
    printf("  blanks %d\n", dist->blanks);
    printf("  invalids %d\n", dist->invalids);
    printf("  partyVotes\n");
    for (int p = 0; p < PARTIES_SZ; p++) {
        printf("  %d %d%s\n", p, dist->partyVotes[p], (p < PARTIES_SZ - 1) ? "," : "");
    }
    printf("DONE\n");
}

void EventPrintStation(int sid) {
    DebugPrint("J %d\n", sid);

    Station* station = FindStationBySid(sid);
    printf("J %d\n", sid);
    printf("  registered %d\n", station->registered);
    printf("  voters\n");
    if (station->registered > 0) {
        int idx = 0;
        PrintVotersInorderHelper(station->voters, &idx, station->registered);
    }
    printf("DONE\n");
}

void EventPrintParty(int pid) {
    DebugPrint("K %d\n", pid);

    printf("K %d\n", pid);
    printf("  elected\n");
    int total = CountElectedCandidates(Parties[pid].candidates);
    int idx = 0;
    PrintElectedCandidatesHelper(Parties[pid].candidates, &idx, total);
    printf("DONE\n");
}

void EventPrintParliament(void) {
    DebugPrint("L\n");

    printf("L\n");
    printf("  members\n");
    for (ElectedCandidate* cur = Parliament; cur != NULL; cur = cur->next) {
        printf("  %d %d %d%s\n", cur->cid, cur->pid, cur->did, (cur->next != NULL) ? "," : "");
    }
    printf("DONE\n");
}

void EventBonusUnregisterVoter(int vid, int sid) {
    DebugPrint("BU %d %d\n", vid, sid);

    Station* station = FindStationBySid(sid);
    Voter* target = FindVoterAnywhere(station->voters, vid);

    int n = station->registered;
    if (n == 1) {
        free(station->voters);
        station->voters = NULL;
    } else {
        // move the last node (in level order) into the spot we're deleting,
        // then just drop the (now duplicated) last node - keeps the tree complete
        Voter* lastParent = GetParentAtIndex(station->voters, n);
        Voter* last;
        if (n & 1) { last = lastParent->rc; lastParent->rc = NULL; }
        else       { last = lastParent->lc; lastParent->lc = NULL; }
        target->vid = last->vid;
        target->voted = last->voted;
        free(last);
    }
    station->registered--;

    printf("BU %d %d\n", vid, sid);
    printf("  Voters[%d]\n  ", sid);
    if (station->registered > 0) {
        int vids[station->registered];
        int count = 0;
        CollectVoterVids(station->voters, vids, &count);
        PrintCommaList(vids, count);
    } else {
        printf("\n");
    }
    printf("DONE\n");
}

void EventBonusFreeMemory(void) {
    DebugPrint("BF\n");

    for (int i = 0; i < HashTableSz; i++) {
        Station* cur = StationsHT[i];
        while (cur != NULL) {
            Station* next = cur->next;
            FreeVoterTree(cur->voters);
            free(cur);
            cur = next;
        }
    }
    free(StationsHT);
    StationsHT = NULL;

    for (int p = 0; p < PARTIES_SZ; p++) {
        FreeCandidateTree(Parties[p].candidates);
        Parties[p].candidates = NULL;
    }

    ElectedCandidate* cur = Parliament;
    while (cur != NULL) {
        ElectedCandidate* next = cur->next;
        free(cur);
        cur = next;
    }
    Parliament = NULL;

    printf("BF\n");
    printf("DONE\n");
}
