#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

struct client {
    int startTime = 0;
    int endTime = 1;
    int money = 0;
};


void readFile(const string& inputFile, vector<client>& clients) {
    ifstream data;
    data.open(inputFile);
    if (!data.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }

    client temp;
    while (data >> temp.startTime >> temp.endTime >> temp.money) {
        clients.push_back(temp);
    }
    data.close();
}

void buildDAG(vector<client>& clients, vector<vector<int>>& matrix) {

    //We can only make the matrix once we have clients size
    matrix.resize(clients.size() + 2, vector<int>(clients.size() + 2, 0));

    // This for loop is a bit weird with the indices but hopefully it makes enough sense
    for (int i = 1; i <= clients.size(); i++) {
        for (int j = 1; j <= clients.size(); j++) {
            if (i != j && clients[i - 1].endTime <= clients[j - 1].startTime)
                matrix[i][j] = 1;
        }
    }

    // Basically this first for loop is looking for any outgoing edges, if a client doesn't have one, it MUST point to END
    bool anyEdges = false;
    for (int i = 1; i <= clients.size(); i++) {
        anyEdges = false;
        for (int j = 1; j <= clients.size(); j++) {
            if (matrix[i][j] == 1) {
                anyEdges = true;
                break;
            }
        }
        if (!anyEdges) {
            matrix[i][clients.size() + 1] = 1;
        }
    }

    // This is the same thing but we are looking for any vertex without an incoming edge, so that START can point to it
    for (int j = 1; j <= clients.size(); j++) {
        anyEdges = false;
        for (int i = 1; i <= clients.size(); i++) {
            if (matrix[i][j] == 1) {
                anyEdges = true;
                break;
            }
        }
        if (!anyEdges) {
            matrix[0][j] = 1;
        }
    }
}

int main() {
    vector<client> clients;
    vector<vector<int>> matrix;
    // Didn't say anything about user input so I just slapped it on like this
    string filename = "data1.txt";

    readFile(filename,clients);
    buildDAG(clients, matrix);
}