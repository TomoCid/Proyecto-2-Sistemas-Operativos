#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <limits>
#include <stdexcept>

using namespace std;

class HashTable {
    static const int SIZE = 100;
    vector<list<pair<int, int>>> table;

    int hash(int key) const {
        return key % SIZE;
    }

public:
    HashTable() : table(SIZE) {}

    void addPage(int page, int frame) {
        int index = hash(page);
        for (auto& pair : table[index]) {
            if (pair.first == page) {
                pair.second = frame;
                return;
            }
        }
        table[index].emplace_back(page, frame);
    }

    void removePage(int page) {
        int index = hash(page);
        table[index].remove_if([page](const pair<int, int>& p) {
            return p.first == page;
        });
    }

    int getFrame(int page) const {
        int index = hash(page);
        for (const auto& pair : table[index]) {
            if (pair.first == page) {
                return pair.second;
            }
        }
        return -1;
    }
};

class VMSimulator {
private:
    HashTable pageTable;
    vector<int> pageReferences;
    deque<int> frames;
    string replacementAlgorithm;
    int numFrames;
    int pageFaultCount = 0;

public:
    VMSimulator(int numFrames, const string& replacementAlgorithm)
        : numFrames(numFrames), replacementAlgorithm(replacementAlgorithm) {}

    void loadPageReferences(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Error al abrir el archivo.");
        }
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
            cout << "Ingrese un algoritmo válido."<< endl;
        }
    }

    void simulateOptimal() {
        vector<int> optimalFrames(numFrames, -1);

        for (size_t i = 0; i < pageReferences.size(); ++i) {
            int page = pageReferences[i];

            if (find(optimalFrames.begin(), optimalFrames.end(), page) == optimalFrames.end()) {
                pageFaultCount++;
                int replaceIndex = -1;
                int farthest = i;

                for (int j = 0; j < numFrames; ++j) {
                    if (optimalFrames[j] == -1) {
                        replaceIndex = j;
                        break;
                    }

                    size_t nextUse = distance(pageReferences.begin(), 
                        find(pageReferences.begin() + i + 1, pageReferences.end(), optimalFrames[j]));
                    if (nextUse > farthest) {
                        farthest = nextUse;
                        replaceIndex = j;
                    }
                }
                if (replaceIndex != -1) {
                    optimalFrames[replaceIndex] = page;
                }
            }
        }
    }

    void simulateFIFO() {
        for (int page : pageReferences) {
            if (find(frames.begin(), frames.end(), page) == frames.end()) {
                pageFaultCount++;
                if (frames.size() == numFrames) {
                    frames.pop_front();
                }
                frames.push_back(page);
            }
        }
    }

    void simulateLRU() {
        list<int> lruCache;

        for (int page : pageReferences) {
            auto it = find(lruCache.begin(), lruCache.end(), page);
            if (it == lruCache.end()) {
                pageFaultCount++;
                if (lruCache.size() == numFrames) {
                    lruCache.pop_front();
                }
            } else {
                lruCache.erase(it);
            }
            lruCache.push_back(page);
        }
    }

    void simulateClockLRU() {
        vector<int> clock(numFrames, -1);
        vector<int> useBit(numFrames, 0);
        int hand = 0;

        for (int page : pageReferences) {
            bool pageFound = false;

            for (int i = 0; i < numFrames; ++i) {
                if (clock[i] == page) {
                    useBit[i] = 1;
                    pageFound = true;
                    break;
                }
            }

            if (!pageFound) {
                pageFaultCount++;
                while (useBit[hand] == 1) {
                    useBit[hand] = 0;
                    hand = (hand + 1) % numFrames;
                }
                clock[hand] = page;
                useBit[hand] = 1;
                hand = (hand + 1) % numFrames;
            }
        }
    }

    int getPageFaultCount() const {
        return pageFaultCount;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Uso: " << argv[0] << " -m <num_frames> -a <algorithm> -f <references_file>\n";
        return 1;
    }

    int numFrames = 0;
    string algorithm, referencesFile;

    for (int i = 1; i < argc; i += 2) {
        string arg = argv[i];
        if (arg == "-m") numFrames = stoi(argv[i + 1]);
        else if (arg == "-a") algorithm = argv[i + 1];
        else if (arg == "-f") referencesFile = argv[i + 1];
    }

    try {
        VMSimulator vm(numFrames, algorithm);
        vm.loadPageReferences(referencesFile);
        vm.startSimulation();
        cout << "Fallos de página: " << vm.getPageFaultCount() << '\n';
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}