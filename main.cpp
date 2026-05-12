#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <sstream>
#include <algorithm>

using namespace std;

map<string, int> vals = {{"iron", 7}, {"gold", 11}, {"gems", 23}, {"exp", 1}};
map<string, int> collected = {{"iron", 0}, {"gold", 0}, {"gems", 0}, {"exp", 0}};

struct Room {
    int id;
    vector<int> neighbors;
    map<string, int> res;

    Room(string s) {
        for (char &c : s) {
            if (c == ','){
                c = ' ';
            } 
        }
        stringstream ss(s);
        if (!(ss >> id)){
            return;
        }
        vector<int> nums;
        int n;
        while (ss >> n){
            nums.push_back(n);
        } 
        if (nums.size() >= 4) {
            for (size_t i = 0; i < nums.size() - 4; ++i) neighbors.push_back(nums[i]);
            res["iron"] = nums[nums.size() - 4];
            res["gold"] = nums[nums.size() - 3];
            res["gems"] = nums[nums.size() - 2];
            res["exp"] = nums[nums.size() - 1];
        }
    }
};

void state(int n, vector<Room>& rs, ofstream& out) {
    out << "state " << n;
    vector<string> order = {"iron", "gold", "gems", "exp"};
    for (auto& k : order) {
        if (n == 0) {
            out << " 0";
        }    
        else {
            out << " " << (rs[n].res[k] > 0 ? to_string(rs[n].res[k]) : "_");
        }   
    }
    out << endl;
}

void collect(Room& r, string tar, ofstream& out) {
    string best = "";
    int max_v = -1;
    vector<string> order = {"iron", "gold", "gems", "exp"};
    for (auto& k : order) {
        if (r.res[k] > 0) {
            int v = vals[k] * (k == tar ? 2 : 1);
            if (v > max_v) {
                max_v = v; best = k;
            }
        }
    }
    if (!best.empty()) {
        collected[best]++; 
        r.res[best]--;
        out << "collect " << best << endl;
    }
}

int next_step(int start, const vector<int>& way, const vector<vector<int>>& adj) {
    set<int> seen(way.begin(), way.end());
    vector<int> nbs = adj[start];
    sort(nbs.begin(), nbs.end());
    for (int n : nbs){
        if (seen.find(n) == seen.end()){
            return n;
        } 
    } 
    
    queue<pair<int, int>> q;
    set<int> visited = {start};
    for (int n : nbs) { q.push({n, n}); visited.insert(n); }
    while (!q.empty()) {
        int curr = q.front().first; int first = q.front().second; q.pop();
        vector<int> next_nbs = adj[curr]; sort(next_nbs.begin(), next_nbs.end());
        for (int n : next_nbs) {
            if (seen.find(n) == seen.end()){
                return first;
            } 
            if (visited.find(n) == visited.end()) {
                visited.insert(n); q.push({n, first}); 
            }
        }
    }
    return -1;
}

vector<int> get_path_to_zero(int start, const vector<vector<int>>& adj) {
    if (start == 0) return {};
    queue<vector<int>> q;
    q.push({start});
    set<int> v = {start};
    while(!q.empty()){
        vector<int> p = q.front(); q.pop();
        int curr = p.back();
        if (curr == 0) {
            return vector<int>(p.begin() + 1, p.end());
        }
        vector<int> nbs = adj[curr]; sort(nbs.begin(), nbs.end());
        for(int n : nbs){
            if(v.find(n) == v.end()){
                v.insert(n);
                vector<int> np = p;
                np.push_back(n);
                q.push(np);
            }
        } 
    }
    return {};
}

int main(int argc, char* argv[]) {
    ofstream out("result.txt");
    if (argc < 2) {
        out << "Invalid input" << endl; return 0; 
    }
    
    ifstream in(argv[1]);
    if (!in.is_open()) { 
        out << "Invalid input" << endl; return 0; 
    }
    
    string l; 
    if (!getline(in, l) || l.empty()) {
         out << "Invalid input" << endl; return 0; 
    }
    
    int N;
    try { 
        N = stoi(l);
    } catch (...) { 
        out << "Invalid input" << endl; return 0; 
    }

    vector<Room> rooms;
    for (int i = 0; i <= N; ++i) {
        if (!getline(in, l) || l.empty()) {
            out << "Invalid input" << endl; return 0; 
        }
        for (char c : l) {
            if (!isdigit(c) && !isspace(c) && c != ',' && c != '\r') {
                out << l << endl; 
                return 0;
            }
        }
        rooms.emplace_back(l);
    }
    
    int M; string tar; 
    if (!(in >> M >> tar)) { 
        out << "Invalid input" << endl; return 0; 
    }

    vector<vector<int>> adj(N + 1);
    for (int i = 0; i <= N; ++i){
        adj[i] = rooms[i].neighbors;
    }

    vector<int> way;
    int cur_f = M, curr = 0;
    way.push_back(0); 
    state(0, rooms, out);

    while (cur_f > M / 2) {
        int nxt = next_step(curr, way, adj);
        if (nxt == -1) break;
        curr = nxt; cur_f--;
        out << "go " << curr << endl;
        way.push_back(curr);
        state(curr, rooms, out);

        bool has_res = false;
        for(auto const& [name, count] : rooms[curr].res) {
            if(count > 0) {
                has_res = true;
            }
        }

        if (has_res) {
            collect(rooms[curr], tar, out);
            state(curr, rooms, out);
        }
    }

    vector<int> back = get_path_to_zero(curr, adj);
    for (int n : back) { 
        out << "go " << n << endl; 
        state(n, rooms, out); 
    }

    long long total = 0;
    vector<string> order = {"iron", "gold", "gems", "exp"};
    for (auto& k : order) {
        total += (long long)collected[k] * vals[k] * (k == tar ? 2 : 1);
    }
    out << "result " << collected["iron"] << " " << collected["gold"] << " " << collected["gems"] << " " << collected["exp"] << " " << total << endl;

    return 0;
}
