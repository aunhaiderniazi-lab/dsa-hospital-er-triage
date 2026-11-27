# Smart Hospital ER Triage System (C++ DSA)

A console-based **Emergency Room Triage & Patient Queue System** built in C++11, combining four core data structures to manage patient intake, urgency prioritization, ward bed allocation, and instant action rollback.

**Author:** Aun Haider Niazi  
**Program:** Bachelor of Engineering Technology (Information) — Foundation University (FUSST)

---

## Features

- **Priority Triage Queue:** Automatically orders waiting patients from severity `10` (Critical) down to `1` (Minor), using arrival order as a FIFO tie-breaker.
- **Live ER Dashboard:** Displays occupied ward beds, the sorted waiting queue, and estimated wait times (`10 mins` per position ahead).
- **Dynamic Urgency Escalation:** Updates a waiting patient's severity in real time if their condition worsens.
- **Ward Bed Management:** Admits highest-priority patients up to the 5-bed ward limit and frees beds upon discharge.
- **1-Click Undo (`O(1)`):** Reverts the most recent registration, admission, discharge, or severity escalation.
- **Instant Patient Lookup (`O(1)`):** Retrieves any patient's condition, severity, and current status by ID.
- **Session Persistence:** Exports the complete patient log to `er_records.txt` upon exit.

---

## Data Structures Used

| Data Structure | Role in System | Time Complexity |
| :--- | :--- | :--- |
| **Max-Heap (`priority_queue`)** | Orders waiting patients by severity with FIFO tie-breaking and lazy deletion | `O(log n)` insert / extract |
| **Doubly Linked List (`WardNode`)** | Tracks active ward beds with bidirectional links (`prev` / `next`) | `O(1)` insert / delete at node |
| **Stack (`undoStack`)** | Stores state snapshots for reverting recent operations | `O(1)` push / pop |
| **Hash Map (`unordered_map`)** | Maps Patient IDs to full records for instant lookup and stale-entry checks | `O(1)` average lookup |

---

## Build & Run

```bash
g++ -std=c++11 -Wall -Wextra er_triage.cpp -o er_triage
./er_triage
```

*(On Windows PowerShell or Command Prompt, run `er_triage.exe` after compiling.)*

---

## Sample Walkthrough

```text
Smart Hospital ER Triage System - by Aun Haider Niazi

==================== ER LIVE DASHBOARD ====================
Ward Beds Occupied: 0/5
Admitted Patients: None

Waiting Triage Queue:
ID    Name            Condition         Severity  Est. Wait
------------------------------------------------------------
102   Sara_Khan       Chest_Pain        9/10      0 mins
101   Ali_Raza        Fractured_Arm     5/10      10 mins
103   Bilal_Ahmed     Mild_Fever        2/10      20 mins
===========================================================
1. Register Patient   2. Admit Highest-Priority   3. Escalate Severity
4. Discharge Patient  5. Search Patient           6. Undo Last Action
7. Exit & Save
Choice: 2
ADMITTED #102 Sara_Khan [Chest_Pain, Sev 9] to Ward Bed.
```
