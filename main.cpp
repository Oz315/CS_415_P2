#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <queue>

using namespace std;

struct client {
    int startTime = 0;
    int endTime = 1;
    int money = 0;
};


void readFile(const string& inputFile, vector<client>& clients) {
    ifstream data;
    string fullInput = "data/data/" + inputFile;
    data.open(fullInput);
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

vector<int> topologicalSort(const vector<vector<int>>& matrix) {

    // Get the total number of vertices, including Start and End
    int size = matrix.size();

    // Keep track of incoming edges for each vertex
    vector<int> inDegree(size, 0);

    // This will hold the sorted order
    vector<int> topoOrder;
    queue<int> q;

    // Count how many edges are going into each vertex
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] == 1) {
                inDegree[j]++;
            }
        }
    }

    // Put the vertices with no incoming edges into the queue
    for (int i = 0; i < size; i++) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    // Keep taking vertices out until the queue is empty
    while (!q.empty()) {

        // Get the first vertex and remove it from the queue
        int current = q.front();
        q.pop();

        // Add this vertex to the sorted list
        topoOrder.push_back(current);

        // Look at the edges leaving this vertex
        for (int j = 0; j < size; j++) {
            if (matrix[current][j] == 1) {

                // Subtract one incoming edge without changing the matrix
                inDegree[j]--;

                // If nothing points to this vertex anymore, add it
                if (inDegree[j] == 0) {
                    q.push(j);
                }
            }
        }
    }

    // Check if we were able to sort all the vertices
    if (topoOrder.size() != matrix.size()) {
        cout << "Could not finish topological sorting." << endl;
        return {};
    }

    // Send back the finished topological order
    return topoOrder;
}

void calculateRevenue(const vector<client>& clients,
     const vector<vector<int>>& matrix,
     const vector<int>& topoOrder,
     vector<long long>& F,
     vector<int>& opt) {

    int size = matrix.size();
    int end = clients.size() + 1;

    // Make space for the F and opt values
    F.assign(size, 0);
    opt.assign(size, -1);

    // End doesn't have any revenue
    F[end] = 0;

    // Go backwards through the topological order
    for (int i = (int)topoOrder.size() - 1; i >= 0; i--) {

        int current = topoOrder[i];

        // We already know the F value for End
        if (current == end) {
            continue;
        }

        // Start doesn't have any money
        int money = 0;

        // Get the money for the current client
        if (current != 0) {
            money = clients[current - 1].money;
        }

        long long best = -1;

        // Check all the vertices that current can go to
        for (int j = 0; j < size; j++) {

            if (matrix[current][j] == 1) {

                // Add this client's money to the next F value
                long long total = money + F[j];

                // If this gives more money, save it
                if (total > best) {
                    best = total;
                    opt[current] = j;
                }
            }
        }

        // Save the highest revenue we found for current
        if (best != -1) {
            F[current] = best;
        }
    }
}

vector<int> computeOptimalPath(const vector<int>& opt,
    const vector<client>& clients) {

    // End is after all the clients in the matrix
    int end = clients.size() + 1;

    // This will hold the clients that give the best revenue
    vector<int> chosenClients;

    // Start at the first client saved in opt
    int current = opt[0];

    // Keep going until we reach End
    while (current != end && current != -1) {

        // Save the client number in our list
        chosenClients.push_back(current);

        // Move to the next best client from opt
        current = opt[current];
    }

    // Return the list of clients we picked
    return chosenClients;
}

void writeFiles(const string& filename, const string& output) {
    ofstream out;
    string outputFileName = "output/out" + filename;
    out.open(outputFileName);
    if (!out.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }
    out << output;
}

int main() {
    vector<client> clients;
    vector<vector<int>> matrix;
    string filename;
    string outText; // This is for writing output to outFile
    cout << "Enter the file to read data:";
    cin >> filename;

    outText += "Enter the file to read data: " + filename + "\n";
    readFile(filename,clients);

    cout << "\nThere are " << clients.size() << " clients in this file\n\n";
    outText += "\nThere are " + to_string(clients.size()) + " clients in this file\n\n";
    buildDAG(clients, matrix);
    
    // Get the topological order from the matrix
    vector<int> topoOrder = topologicalSort(matrix);

   // Check if topological sorting worked before moving on
    if (topoOrder.size() != matrix.size()) {
        return 1;
    }

    /*
    // Print it just to see if it worked
    cout << "Topological order: ";

    int totalVertices = topoOrder.size();
    int end = clients.size() + 1;

    for (int i = 0; i < totalVertices; i++) {

        // Check if the current vertex is Start
        if (topoOrder[i] == 0) {
            cout << "Start";
        }

        // Check if we reached the End vertex
        else if (topoOrder[i] == end) {
            cout << "End";
        }

        // Otherwise print the client number
        else {
            cout << "C" << topoOrder[i];
        }

        if (i + 1 < totalVertices) {
            cout << ", ";
        }
    }

    cout << endl;
    */
    // These will hold the revenue and best next vertices
    vector<long long> F;
    vector<int> opt;

    // Calculate the highest revenue using the topological order
    calculateRevenue(clients, matrix, topoOrder, F, opt);

    // Print the highest revenue starting from Start
    cout << "Optimal revenue earned is " << F[0] << endl;
    outText += "Optimal revenue earned is " + to_string(F[0]) + "\n";
    // Get the clients that give us the highest revenue
    vector<int> chosenClients = computeOptimalPath(opt, clients);

    // Print the clients we picked
    cout << "Clients contributing to this optimal revenue: ";
    outText += "Clients contributing to this optimal revenue: ";
    for (int i = 0; i < chosenClients.size(); i++) {

        // Put commas between the client numbers
        if (i > 0) {
            cout << ", ";
            outText += ", ";
        }

        // Print the client number
        cout << chosenClients[i];
        outText += to_string(chosenClients[i]);
    }

    cout << endl;
    outText += "\n";

    writeFiles(filename, outText);
    return 0;

}