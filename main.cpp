#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <queue>

using namespace std;

struct Room {
    int id = -1;
    vector<int> neighbors;
    int res[4] = {0, 0, 0, 0}; // iron, gold, gems, exp
    bool visited = false;
    bool collected_once = false; 
};

const string res_names[] = {"iron", "gold", "gems", "exp"};
const int base_vals[] = {7, 11, 23, 1};

void print_state(ostream& out, const Room& r) {
    out << "state " << r.id;
    for (int i = 0; i < 4; ++i) {
        if (r.res[i] < 0) out << " _";
        else out << " " << r.res[i];
    }
    out << endl;
}

int get_best_res(const Room& r, int target_idx, const int doubled_vals[]) {
    int best = -1;
    int max_v = -1;
    for (int i = 0; i < 4; ++i) {
        if (r.res[i] > 0) {
            if (doubled_vals[i] > max_v) {
                max_v = doubled_vals[i];
                best = i;
            }
        }
    }
    return best;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    ifstream in(argv[1]);
    ofstream out("result.txt"); // Открываем сразу для вывода ошибок
    if (!in) return 1;

    int N;
    string line;
    if (!(in >> N)) return 0;
    getline(in, line); // Очистка буфера после чтения N

    map<int, Room> rooms;
    for (int i = 0; i <= N; ++i) {
        if (!getline(in, line) || line.empty()) break;
        
        // Валидация строки перед обработкой
        stringstream ss(line);
        string id_s, adj_s, r0, r1, r2, r3, extra;
        
        // Проверка структуры: должно быть ровно 6 элементов
        if (!(ss >> id_s >> adj_s >> r0 >> r1 >> r2 >> r3) || (ss >> extra)) {
            out << line << endl;
            return 0;
        }

        // Проверка списка смежности (только цифры и запятые)
        for (char c : adj_s) {
            if (!isdigit(c) && c != ',') {
                out << line << endl;
                return 0;
            }
        }

        // Парсинг данных после успешной валидации
        int id = stoi(id_s);
        Room r; 
        r.id = id;
        r.res[0] = stoi(r0); r.res[1] = stoi(r1); r.res[2] = stoi(r2); r.res[3] = stoi(r3);

        // Обработка списка смежности (логика не меняется)
        string adj_temp = adj_s;
        for (char &c : adj_temp) if (!isdigit(c)) c = ' ';
        stringstream ss_adj(adj_temp);
        int nb;
        while (ss_adj >> nb) r.neighbors.push_back(nb);
        
        rooms[id] = r;
    }

    // Делаем связи двусторонними (ваша логика)
    for (auto const& [id, r] : rooms) {
        for (int nb_id : r.neighbors) {
            if (rooms.count(nb_id)) {
                bool exists = false;
                for (int b : rooms[nb_id].neighbors) if (b == id) exists = true;
                if (!exists) rooms[nb_id].neighbors.push_back(id);
            }
        }
    }

    int M; string target_name;
    if (!(in >> M >> target_name)) return 0;
    int target_idx = -1;
    for (int i = 0; i < 4; ++i) if (res_names[i] == target_name) target_idx = i;

    int doubled_vals[4];
    for (int i = 0; i < 4; ++i) {
        doubled_vals[i] = base_vals[i] * (i == target_idx ? 2 : 1);
    }

    int cur = 0;
    int food = M;
    rooms[0].visited = true;
    long long collected[4] = {0, 0, 0, 0};

    // --- ФАЗА ИССЛЕДОВАНИЯ ---
    while (food > M / 2) {
        int next = -1;
        int min_adj = 1e9;
        for (int n : rooms[cur].neighbors) {
            if (rooms.count(n) && !rooms[n].visited && n < min_adj) min_adj = n;
        }

        if (min_adj != 1e9) {
            next = min_adj;
        } else {
            queue<int> q; q.push(cur);
            map<int, int> parent; parent[cur] = -1;
            int found_target = -1;
            while(!q.empty()){
                int u = q.front(); q.pop();
                if (rooms.count(u) && !rooms[u].visited) { found_target = u; break; }
                if (rooms.count(u)) {
                    vector<int> nbs = rooms[u].neighbors;
                    sort(nbs.begin(), nbs.end());
                    for(int v : nbs) {
                        if (parent.find(v) == parent.end()) {
                            parent[v] = u; q.push(v);
                        }
                    }
                }
            }
            if (found_target != -1) {
                int step = found_target;
                while (parent[step] != cur) step = parent[step];
                next = step;
            }
        }

        if (next == -1) break;

        cur = next;
        food--;
        rooms[cur].visited = true;
        out << "go " << cur << endl;
        print_state(out, rooms[cur]);

        int b = get_best_res(rooms[cur], target_idx, doubled_vals);
        if (b != -1) {
            out << "collect " << res_names[b] << endl;
            collected[b] += rooms[cur].res[b];
            rooms[cur].res[b] = -1;
            rooms[cur].collected_once = true;
            print_state(out, rooms[cur]);
        }
    }

    // --- ФАЗА ВОЗВРАЩЕНИЯ ---
    auto get_path = [&](int start) {
        queue<int> q; q.push(0);
        map<int, int> dist; dist[0] = 0;
        while(!q.empty()){
            int u = q.front(); q.pop();
            if (rooms.count(u)) {
                for (int v : rooms[u].neighbors) {
                    if (rooms.count(v) && rooms[v].visited && dist.find(v) == dist.end()) {
                        dist[v] = dist[u] + 1; q.push(v);
                    }
                }
            }
        }
        vector<int> p;
        int c = start;
        while (c != 0) {
            int best_p = -1;
            if (rooms.count(c)) {
                for (int v : rooms[c].neighbors) {
                    if (dist.count(v) && dist[v] == dist[c] - 1) {
                        if (best_p == -1 || v < best_p) best_p = v;
                    }
                }
            }
            if (best_p == -1) break;
            c = best_p;
            p.push_back(c);
        }
        return p;
    };

    vector<int> home_path = get_path(cur);
    for (size_t i = 0; i < home_path.size(); ++i) {
        cur = home_path[i];
        food--;
        out << "go " << cur << endl;
        if (cur != 0) print_state(out, rooms[cur]);
        
        int dist_left = home_path.size() - 1 - i;
        while (true) {
            int b = get_best_res(rooms[cur], target_idx, doubled_vals);
            if (b == -1) break;
            if (rooms[cur].collected_once) {
                if (food <= dist_left) break;
                food--;
            }
            out << "collect " << res_names[b] << endl;
            collected[b] += rooms[cur].res[b];
            rooms[cur].res[b] = -1;
            rooms[cur].collected_once = true;
            print_state(out, rooms[cur]);
        }
    }

    long long score = 0;
    for (int i = 0; i < 4; ++i) {
        score += collected[i] * (i == target_idx ? base_vals[i] * 2 : base_vals[i]);
    }

    out << "result " << collected[0] << " " << collected[1] << " " << collected[2] << " " << collected[3] << " " << score << endl;

    return 0;
}