#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <queue>

using namespace std;

typedef vector<string> str_list;
struct word_node {
        //// NODE CONTENT
        string s;
        vector<word_node*> adjacency;
        // get rid of loops
        bool visited;

        //// PRUNING RECORDS
        int level;
        // record all its parent, in order to mark the path
        vector<word_node*> from;

        word_node(const string& s_) : s(s_), visited(false),
                level(0), from(0) { }
};

class Solution {
public:
        void retrievePaths(word_node& node, vector<str_list>& paths) {
                paths.back().push_back(node.s);

                if(node.from.empty()) {
                        paths.push_back(paths.back());
                } else {
                        for(word_node* p : node.from)
                                retrievePaths(*p, paths);
                }

                paths.back().pop_back();
        }

        vector<str_list> findLadders(string start,
                string end, unordered_set<string> &dict) {
                size_t len = start.length();

                vector<word_node> nodes = { word_node(start), word_node(end) };
                for(auto s: dict) nodes.push_back(word_node(s));

                //// build the graph
                for(auto cur = nodes.begin(); ; ++cur) {
                        auto cmp = cur;
                        if(++cmp == nodes.end()) break;

                        for(; cmp != nodes.end(); ++cmp) {
                                size_t difference = 0;
                                for(size_t i = 0; i < len; i++) {
                                        if(cur->s[i] != cmp->s[i])
                                                ++difference;
                                        if(difference >= 2) break;
                                }
                                
                                if(difference == 1) {
                                        cur->adjacency.push_back(&*cmp);
                                        cmp->adjacency.push_back(&*cur);
                                }
                        }
                }

                //// search until all shortest solutions are found
                // once a solution is found, all paths that have reached that
                // level while not having reached the endpoint will be forced
                // stop.
                int min_suclevel = -1; // -1 stands for invalid
                queue<word_node*> bfs;

                nodes.front().visited = true;
                nodes.front().level = 1;
                bfs.push(&nodes.front());

                while(bfs.size()) {
                        word_node& cur = *bfs.front();
                        bfs.pop();
                        //cout << cur.s << ": ";
                        // endpoint will never show up in queue so I can use
                        if(min_suclevel > 0 && cur.level >= min_suclevel)
                                continue;

                        for(word_node* next : cur.adjacency) {
                                if(next->s == end) {
                                        next->from.push_back(&cur);
                                        if(!next->visited) {
                                                next->visited = true;
                                                min_suclevel = cur.level + 1;
                                        }
                                }

                                if(next->visited) {
                                        // because of the property of bfs, the
                                        // the difference between adjencies'
                                        // level and the current is at most 1
                                        if(next->level > cur.level)
                                                next->from.push_back(&cur);
                                } else {
                                        //cout << next->s << " ";
                                        next->visited = true;
                                        next->level = cur.level + 1;
                                        next->from.push_back(&cur);
                                        bfs.push(next);
                                }
                        }

                        //cout << endl;
                }

                /*
                for(word_node& n : nodes) {
                        cout << n.s << ": ";
                        for(word_node* f : n.from) {
                                cout << f->s << " ";
                        } cout << endl;
                }
                */

                // retrieve paths
                vector<str_list> paths, temps;
                temps.push_back(str_list());
                retrievePaths(nodes[1], temps);

                for(str_list& s : temps) {
                        if(s.size() >= 2) paths.push_back(
                                str_list(s.rbegin(), s.rend()));
                }

                return paths;
        }
};

void printResult(const vector<str_list>& r)
{
        for(auto sl: r) {
                for(auto s : sl) cout << s << ' ';
                cout << endl;
        }
}

int main()
{
        unordered_set<string> dict = {"hot","dot","dog","lot","log"};
        printResult(Solution().findLadders("hit", "cog", dict));
}
