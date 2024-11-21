#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <map>
#include <climits>

using namespace std;

class PageTable {
    unordered_map<int, int> table; // Maps each page to its frame

public:
    void addPage(int page, int frame) {
        table[page] = frame; // Updates or adds the entry
    }

    void removePage(int page) {
        table.erase(page); // Directly removes the page
    }

    int getFrame(int page) const {
        auto it = table.find(page);
        return (it != table.end()) ? it->second : -1;
    }
};

class VMSimulator {
private:
    int numFrames;
    string replacementAlgorithm;
    PageTable pageTable;
    deque<int> frames;
    vector<int> optimalFrames;
    unordered_map<int, list<int>> frameMap;
    int pageFaultCount = 0;
    vector<int> pageReferences;

public:
    VMSimulator(int numFrames, string replacementAlgorithm)
        : numFrames(numFrames), replacementAlgorithm(replacementAlgorithm) {}

    void loadPageReferences(const string &filename) {
        ifstream file(filename);
        int page;
        while (file >> page) {
            pageReferences.push_back(page);
        }
    }

    void startSimulation() {
        if (replacementAlgorithm == "Optimal") {
            simulateOptimal();
        } else if (replacementAlgorithm == "FIFO") {
            simulateFIFO();
        } else if (replacementAlgorithm == "LRU") {
            simulateLRU();
        } else if (replacementAlgorithm == "ClockLRU") {
            simulateClockLRU();
        } else {
            cout << "Algorithm " << replacementAlgorithm << " is not supported." << endl;
        }
    }

    void displayFrames(const vector<int>& frames) const {
        cout << "FRAMES: ";
        for (int frame : frames) {
            if (frame != -1) cout << frame << ' ';
        }
        cout << '\n';
    }

    void simulateOptimal() {
        optimalFrames.assign(numFrames, -1);
        map<int, vector<int>> pageOccurrences;

        for (int i = 0; i < pageReferences.size(); ++i) {
            pageOccurrences[pageReferences[i]].push_back(i);
        }

        for (int i = 0; i < pageReferences.size(); ++i) {
            int page = pageReferences[i];
            if (find(optimalFrames.begin(), optimalFrames.end(), page) == optimalFrames.end()) {
                pageFaultCount++;
                int idx = 0, maxDistance = 0;

                for (int j = 0; j < optimalFrames.size(); ++j) {
                    int framePage = optimalFrames[j];
                    if (framePage == -1) {
                        idx = j;
                        break;
                    }
                    auto nextOccurrence = upper_bound(pageOccurrences[framePage].begin(), pageOccurrences[framePage].end(), i);
                    if (nextOccurrence == pageOccurrences[framePage].end()) {
                        maxDistance = INT_MAX;
                        idx = j;
                    } else {
                        int distance = *nextOccurrence - i;
                        if (distance > maxDistance) {
                            maxDistance = distance;
                            idx = j;
                        }
                    }
                }
                pageTable.removePage(optimalFrames[idx]);
                pageTable.addPage(page, optimalFrames.size() - 1);
                optimalFrames[idx] = page;
            }
            displayFrames(optimalFrames);
        }
    }

    void simulateFIFO() {
        for (int page : pageReferences) {
            if (find(frames.begin(), frames.end(), page) == frames.end()) {
                pageFaultCount++;
                if (frames.size() == numFrames) {
                    int removedPage = frames.front();
                    frames.pop_front();
                    pageTable.removePage(removedPage);
                }
                frames.push_back(page);
                pageTable.addPage(page, frames.size() - 1);
            }
            displayFrames({frames.begin(), frames.end()});
        }
    }

    void simulateLRU() {
        for (int page : pageReferences) {
            auto it = find(frames.begin(), frames.end(), page);
            if (it == frames.end()) {
                pageFaultCount++;
                if (frames.size() == numFrames) {
                    int removedPage = frames.front();
                    frames.pop_front();
                    pageTable.removePage(removedPage);
                }
                frames.push_back(page);
            } else {
                frames.erase(it);
                frames.push_back(page);
            }
            pageTable.addPage(page, frames.size() - 1);
            displayFrames({frames.begin(), frames.end()});
        }
    }

    void simulateClockLRU() {
        vector<int> useBit(numFrames, 0);
        int clockHand = 0;

        for (int page : pageReferences) {
            if (find(frames.begin(), frames.end(), page) == frames.end()) {
                pageFaultCount++;
                while (useBit[clockHand] == 1) {
                    useBit[clockHand] = 0;
                    clockHand = (clockHand + 1) % numFrames;
                }
                if (frames.size() == numFrames) {
                    pageTable.removePage(frames[clockHand]);
                    frames[clockHand] = page;
                } else {
                    frames.push_back(page);
                }
                useBit[clockHand] = 1;
                pageTable.addPage(page, clockHand);
                clockHand = (clockHand + 1) % numFrames;
            } else {
                int idx = distance(frames.begin(), find(frames.begin(), frames.end(), page));
                useBit[idx] = 1;
            }
            displayFrames({frames.begin(), frames.end()});
        }
    }

    int getPageFaultCount() const {
        return pageFaultCount;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " -m <num_frames> -a <algorithm> -f <references_file>\n";
        return 1;
    }

    int numFrames = 0;
    string algorithm, referencesFile;

    for (int i = 1; i < argc; i += 2) {
        string arg = argv[i];
        if (arg == "-m") numFrames = stoi(argv[i + 1]);
        else if (arg == "-a") algorithm = argv[i + 1];
        else if (arg == "-f") referencesFile = argv[i + 1];
        else {
            cerr << "Usage: " << argv[0] << " -m <num_frames> -a <algorithm> -f <references_file>\n";
            return 1;
        }
    }

    VMSimulator vm(numFrames, algorithm);
    vm.loadPageReferences(referencesFile);
    vm.startSimulation();
    cout << "Page Faults: " << vm.getPageFaultCount() << '\n';

    return 0;
}
