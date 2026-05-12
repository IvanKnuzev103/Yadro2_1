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
const vector<string> res_order = {"iron", "gold", "gems", "exp"};

struct Room {
    int id;
    vector<int> neighbors;
    map<string, int> res;

    Room(string s) {
        for (size_t i = 0; i < s.length(); i++) if (s[i] == ',') s[i] = ' ';
        stringstream ss(s);
        vector<int> nums; string word;
        while (ss >> word) {
            bool is_num = true;
            for (char c : word) if (!isdigit(c)) is_num = false;
            if (is_num){
                nums.push_back(stoi(word));   
            }
        }
        
        if (!nums.empty()) {
            id = nums[0];
            if (nums.size() >= 5) {
                for (size_t i = 1; i < nums.size() - 4; i++) neighbors.push_back(nums[i]);
                res["iron"] = nums[nums.size() - 4];
                res["gold"] = nums[nums.size() - 3];
                res["gems"] = nums[nums.size() - 2];
                res["exp"] = nums[nums.size() - 1];
            }
        }
    }
};

void state_out(int n, vector<Room>& rs, ofstream& out) {
    out << "state " << n;
    for (const string& k : res_order) {
        if (n == 0) {
            out << " 0";
        }
        else{
            out << " " << (rs[n].res[k] > 0 ? to_string(rs[n].res[k]) : "_");
        }
    } 
    out << endl;
}

bool collect_f(Room& r, string tar, map<string, int>& local_col, ofstream& out, vector<Room>& rooms) {
    string best = ""; int max_v = -1;
    for (const string& k : res_order) {
        if (r.res[k] > 0) {
            int v = vals[k] * (k == tar ? 2 : 1);
            if (v > max_v) { 
                max_v = v; best = k; 
            }
        }
    }

    if (best != "") {
        local_col[best]++; r.res[best]--;
        out << "collect " << best << endl;
        state_out(r.id, rooms, out);
        return true;
    }
    return false;
}

int get_dist(int start, int target, const vector<vector<int>>& adj) {
    if (start == target) {
        return 0;
    }
    queue<pair<int, int>> q;
    q.push({start, 0});
    set<int> v = {start};
    
    while (!q.empty()) {
        int curr = q.front().first; int d = q.front().second; q.pop();
        for (int n : adj[curr]) {
            if (n == target) {
                return d + 1;
            }   
            if (v.find(n) == v.end()) { 
                v.insert(n);
                q.push({n, d + 1}); 
            }
        }
    }
    return 999;
}

vector<int> get_path(int start, int target, const vector<vector<int>>& adj) {
    if (start == target) {
        return {};
    }

    queue<vector<int>> q;
    q.push({start});
    set<int> v = {start};
    while (!q.empty()) {

        vector<int> p = q.front(); q.pop();
        int curr = p.back();
        if (curr == target) {
            vector<int> res_p;
            for (size_t i = 1; i < p.size(); i++){
                res_p.push_back(p[i]);
            } 
            return res_p;
        }

        vector<int> nbs = adj[curr];
        sort(nbs.begin(), nbs.end());
        for (int n : nbs) {
            if (v.find(n) == v.end()) { 
                v.insert(n);
                vector<int> np = p;
                np.push_back(n);
                q.push(np); 
            }
        }
        
    }
    return {};
}

long long run_bot(int mode, int M, string tar, vector<Room> rooms, const vector<vector<int>>& adj, string out_name) {
    ofstream out(out_name);
    map<string, int> local_col = {{"iron", 0}, {"gold", 0}, {"gems", 0}, {"exp", 0}};
    vector<int> way = {0};
    int cur_f = M, curr = 0;
    state_out(0, rooms, out);

    while (true) {
        int d_h = get_dist(curr, 0, adj);
        bool can_move = (mode == 1) ? (cur_f > M / 2) : (cur_f > d_h + 1);
        int nxt = -1;
        if (can_move) {
            set<int> seen(way.begin(), way.end());
            vector<int> nbs = adj[curr]; sort(nbs.begin(), nbs.end());
            for (int n : nbs) {
                if (seen.find(n) == seen.end()) { 
                    nxt = n; break; 
                }
            }    
        }

        if (nxt != -1) {
            curr = nxt; cur_f--;
            out << "go " << curr << endl; way.push_back(curr);
            state_out(curr, rooms, out);
            
            bool first = true;
            while (true) {
                int dist = get_dist(curr, 0, adj);
                if (first) {
                    if (collect_f(rooms[curr], tar, local_col, out, rooms)){
                        first = false;
                    }
                    else break;
                } else if (mode == 2 && cur_f > dist) {
                    if (collect_f(rooms[curr], tar, local_col, out, rooms)) {
                        cur_f--;
                    }    
                    else break;
                } else break;
            }
        } else break;
    }

    vector<int> back = get_path(curr, 0, adj);
    for (int n : back) { 
        cur_f--;
        out << "go " << n << endl;
        state_out(n, rooms, out); 
    }
    
    long long total = 0;
    for (const string& k : res_order) {
        total += (long long)local_col[k] * vals[k] * (k == tar ? 2 : 1);
    }

    out << "result " << local_col["iron"] << " " << local_col["gold"] << " " << local_col["gems"] << " " << local_col["exp"] << " " << total << endl;
    return total;
}

int main(int argc, char* argv[]) {
    ofstream res1("result.txt");
    if (argc < 2) { 
        res1 << "Invalid input" << endl;
        return 0; 
    }

    ifstream in(argv[1]);
    if (!in) { 
        res1 << "Invalid input" << endl;
        return 0; 
    }

    string l;
    if (!getline(in, l) || l.empty()) {
        res1 << "Invalid input" << endl;
        return 0; 
    }

    int N = stoi(l); vector<Room> rooms;
    for (int i = 0; i <= N; i++) {
        if (!getline(in, l)) { 
            res1 << "Invalid input" << endl; return 0; 
        }
        for (char c : l){
            if (!isdigit(c) && !isspace(c) && c != ',' && c != '\r') { 
                res1 << l << endl;
                return 0; 
            }
        } 
        rooms.push_back(Room(l));
    }

    int M; 
    string tar; 
    in >> M >> tar;
    vector<vector<int>> adj(N + 1);
    for (int i = 0; i <= N; i++) adj[i] = rooms[i].neighbors;

    long long r1 = run_bot(1, M, tar, rooms, adj, "result.txt");
    long long r2 = run_bot(2, M, tar, rooms, adj, "result2.txt");
    long long r3 = run_bot(3, M, tar, rooms, adj, "result3.txt");

    ofstream cmp("compare.txt");
    cmp << "Strategy 1: " << r1 << " pts\nStrategy 2: " << r2 << " pts (Greedy)\nStrategy 3: " << r3 << " pts (Economic)\n";
    return 0;
}
