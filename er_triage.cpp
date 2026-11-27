// Author: Aun Haider Niazi
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <limits>
#include <algorithm>
using namespace std;

struct EndOfInput {};   // graceful EOF: lets destructors save state

struct Patient {
    int id;
    string name;
    string condition;
    int severity;     // 1 (Minor) to 10 (Critical)
    int arrivalOrder; // Tie-breaker so equal severity stays FIFO
    string status;    // "Waiting", "Admitted", "Discharged"
};

struct TriageCompare {
    bool operator()(const Patient& a, const Patient& b) const {
        if (a.severity != b.severity) return a.severity < b.severity;
        return a.arrivalOrder > b.arrivalOrder;
    }
};

struct WardNode {
    Patient data;
    WardNode* prev;
    WardNode* next;
    WardNode(const Patient& p) : data(p), prev(nullptr), next(nullptr) {}
};

struct Action {
    string type;
    Patient prevState;
};

class HospitalER {
    priority_queue<Patient, vector<Patient>, TriageCompare> waitingHeap;
    unordered_map<int, Patient> registry;
    stack<Action> undoStack;

    WardNode* wardHead = nullptr;
    WardNode* wardTail = nullptr;
    int wardCount = 0;
    const int MAX_BEDS = 5;
    int nextId = 101;
    int arrivalCounter = 0;

    int readInt(const string& prompt, int minVal, int maxVal) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) {
                if (val >= minVal && val <= maxVal) return val;
            } else if (cin.eof()) {
                throw EndOfInput{};
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input (" << minVal << "-" << maxVal << "). Try again.\n";
        }
    }

    string readWord(const string& prompt) {
        string val;
        while (true) {
            cout << prompt;
            if (cin >> val) return val;
            if (cin.eof()) throw EndOfInput{};
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
        }
    }

    void addToWard(const Patient& p) {
        WardNode* node = new WardNode(p);
        if (!wardHead) {
            wardHead = wardTail = node;
        } else {
            wardTail->next = node;
            node->prev = wardTail;
            wardTail = node;
        }
        wardCount++;
    }

    bool removeFromWard(int id, Patient& removedOut) {
        WardNode* cur = wardHead;
        while (cur) {
            if (cur->data.id == id) {
                removedOut = cur->data;
                if (cur->prev) cur->prev->next = cur->next;
                else wardHead = cur->next;
                if (cur->next) cur->next->prev = cur->prev;
                else wardTail = cur->prev;
                delete cur;
                wardCount--;
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    void rebuildHeap() {
        while (!waitingHeap.empty()) waitingHeap.pop();
        for (const auto& kv : registry) {
            if (kv.second.status == "Waiting") {
                waitingHeap.push(kv.second);
            }
        }
    }

public:
    HospitalER() {
        // Preload sample patients for quick testing
        seedPatient("Ali_Raza", "Fractured_Arm", 5);
        seedPatient("Sara_Khan", "Chest_Pain", 9);
        seedPatient("Bilal_Ahmed", "Mild_Fever", 2);
    }

    ~HospitalER() {
        saveReport();
        while (wardHead) {
            WardNode* tmp = wardHead;
            wardHead = wardHead->next;
            delete tmp;
        }
    }

    void seedPatient(const string& name, const string& cond, int sev) {
        Patient p{nextId++, name, cond, sev, ++arrivalCounter, "Waiting"};
        registry[p.id] = p;
        waitingHeap.push(p);
    }

    void registerPatient() {
        string name, cond;
        name = readWord("Patient Name (use _ for spaces): ");
        cond = readWord("Condition (e.g. Cardiac_Arrest, Burns, Fever): ");
        int sev = readInt("Severity (1=Minor to 10=Critical): ", 1, 10);

        Patient p{nextId++, name, cond, sev, ++arrivalCounter, "Waiting"};
        registry[p.id] = p;
        waitingHeap.push(p);
        undoStack.push({"REGISTER", p});

        cout << "Registered #" << p.id << " (" << p.name << ") | Severity: " << p.severity << "/10\n";
    }

    void admitNext() {
        if (wardCount >= MAX_BEDS) {
            cout << "Ward is full (" << wardCount << "/" << MAX_BEDS << " beds occupied). Discharge a patient first.\n";
            return;
        }

        // Skip stale heap copies left behind by escalations or undos
        while (!waitingHeap.empty()) {
            Patient top = waitingHeap.top();
            waitingHeap.pop();
            auto it = registry.find(top.id);
            if (it != registry.end() && it->second.status == "Waiting" &&
                it->second.severity == top.severity) {
                undoStack.push({"ADMIT", it->second});
                it->second.status = "Admitted";
                addToWard(it->second);
                cout << "ADMITTED #" << it->second.id << " " << it->second.name
                     << " [" << it->second.condition << ", Sev " << it->second.severity << "] to Ward Bed.\n";
                return;
            }
        }
        cout << "No patients in waiting queue.\n";
    }

    void escalateSeverity() {
        int id = readInt("Enter Waiting Patient ID: ", 100, numeric_limits<int>::max());
        auto it = registry.find(id);
        if (it == registry.end() || it->second.status != "Waiting") {
            cout << "Active waiting patient not found.\n";
            return;
        }
        int newSev = readInt("New Severity (1-10): ", 1, 10);
        undoStack.push({"ESCALATE", it->second});
        it->second.severity = newSev;
        waitingHeap.push(it->second);
        cout << "Updated #" << id << " (" << it->second.name << ") severity to " << newSev << "/10.\n";
    }

    void dischargePatient() {
        if (!wardHead) {
            cout << "No patients currently admitted in ward.\n";
            return;
        }
        int id = readInt("Enter Admitted Patient ID to discharge: ", 100, numeric_limits<int>::max());
        Patient removed;
        if (!removeFromWard(id, removed)) {
            cout << "Patient #" << id << " is not in the ward.\n";
            return;
        }
        undoStack.push({"DISCHARGE", removed});
        registry[id].status = "Discharged";
        cout << "Discharged #" << id << " (" << removed.name << "). Bed freed.\n";
    }

    void searchPatient() {
        int id = readInt("Search Patient ID: ", 100, numeric_limits<int>::max());
        auto it = registry.find(id);
        if (it == registry.end()) {
            cout << "Patient #" << id << " not found.\n";
            return;
        }
        const Patient& p = it->second;
        cout << "\n[Record #" << p.id << "] " << p.name
             << " | Condition: " << p.condition
             << " | Severity: " << p.severity << "/10"
             << " | Status: " << p.status << "\n";
    }

    void undoLastAction() {
        if (undoStack.empty()) {
            cout << "Nothing to undo.\n";
            return;
        }
        Action act = undoStack.top();
        undoStack.pop();
        int id = act.prevState.id;

        if (act.type == "REGISTER") {
            registry.erase(id);
            rebuildHeap();
            cout << "Undid registration of #" << id << " (" << act.prevState.name << ").\n";
        } else if (act.type == "ADMIT") {
            Patient dummy;
            removeFromWard(id, dummy);
            registry[id] = act.prevState;
            waitingHeap.push(act.prevState);
            cout << "Undid admission of #" << id << " (" << act.prevState.name << ") -> returned to queue.\n";
        } else if (act.type == "DISCHARGE") {
            registry[id] = act.prevState;
            addToWard(act.prevState);
            cout << "Undid discharge of #" << id << " (" << act.prevState.name << ") -> restored to ward.\n";
        } else if (act.type == "ESCALATE") {
            registry[id] = act.prevState;
            rebuildHeap();
            cout << "Undid severity change for #" << id << " (restored to " << act.prevState.severity << "/10).\n";
        }
    }

    void showDashboard() const {
        cout << "\n==================== ER LIVE DASHBOARD ====================\n";
        cout << "Ward Beds Occupied: " << wardCount << "/" << MAX_BEDS << "\n";
        cout << "Admitted Patients: ";
        if (!wardHead) {
            cout << "None\n";
        } else {
            WardNode* cur = wardHead;
            while (cur) {
                cout << "[#" << cur->data.id << " " << cur->data.name << " (Sev " << cur->data.severity << ")]";
                if (cur->next) cout << " <-> ";
                cur = cur->next;
            }
            cout << "\n";
        }

        cout << "\nWaiting Triage Queue:\n";
        cout << left << setw(6) << "ID" << setw(16) << "Name" << setw(18) << "Condition"
             << setw(10) << "Severity" << "Est. Wait\n";
        cout << string(60, '-') << "\n";

        auto copyHeap = waitingHeap;
        unordered_set<int> shown;   // escalations leave stale heap copies behind
        int pos = 0;
        while (!copyHeap.empty()) {
            Patient p = copyHeap.top();
            copyHeap.pop();
            auto it = registry.find(p.id);
            if (it == registry.end() || it->second.status != "Waiting" ||
                it->second.severity != p.severity) continue;
            if (!shown.insert(p.id).second) continue;

            cout << setw(6) << p.id << setw(16) << p.name << setw(18) << p.condition
                 << setw(10) << (to_string(p.severity) + "/10")
                 << (pos * 10) << " mins\n";
            pos++;
        }
        if (pos == 0) cout << "(Queue Empty)\n";
        cout << "===========================================================\n";
    }

    void saveReport() const {
        ofstream f("er_records.txt", ios::trunc);
        if (!f) { cerr << "Warning: could not write er_records.txt\n"; return; }
        f << "Hospital ER Triage Log - by Aun Haider Niazi\n";
        for (const auto& kv : registry) {
            const Patient& p = kv.second;
            f << p.id << " " << p.name << " " << p.condition
              << " Severity:" << p.severity << " Status:" << p.status << "\n";
        }
    }
};

int main() {
    cout << "Smart Hospital ER Triage System - by Aun Haider Niazi\n";
    HospitalER er;
    int choice;

    try {
    do {
        er.showDashboard();
        cout << "1. Register Patient   2. Admit Highest-Priority   3. Escalate Severity\n"
             << "4. Discharge Patient  5. Search Patient           6. Undo Last Action\n"
             << "7. Exit & Save\nChoice: ";

        if (!(cin >> choice)) {
            if (cin.eof()) { cout << "\nEnd of input. Session saved.\n"; break; }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number 1-7.\n";
            continue;
        }

        if (choice == 1) er.registerPatient();
        else if (choice == 2) er.admitNext();
        else if (choice == 3) er.escalateSeverity();
        else if (choice == 4) er.dischargePatient();
        else if (choice == 5) er.searchPatient();
        else if (choice == 6) er.undoLastAction();
    } while (choice != 7);
    } catch (const EndOfInput&) {
        cout << "\nEnd of input. Session saved.\n";
    }

    cout << "Session saved to er_records.txt. Goodbye!\n";
    return 0;
}
