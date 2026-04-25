#include <iostream>
#include <string>
#include "deque.hpp"

using namespace std;

int main() {
    int n;
    cin >> n;
    
    sjtu::deque<int> dq;
    
    for (int i = 0; i < n; ++i) {
        string cmd;
        cin >> cmd;
        
        if (cmd == "push_back") {
            int value;
            cin >> value;
            dq.push_back(value);
        } else if (cmd == "push_front") {
            int value;
            cin >> value;
            dq.push_front(value);
        } else if (cmd == "pop_back") {
            dq.pop_back();
        } else if (cmd == "pop_front") {
            dq.pop_front();
        } else if (cmd == "front") {
            cout << dq.front() << endl;
        } else if (cmd == "back") {
            cout << dq.back() << endl;
        } else if (cmd == "size") {
            cout << dq.size() << endl;
        } else if (cmd == "empty") {
            cout << (dq.empty() ? "true" : "false") << endl;
        } else if (cmd == "at") {
            size_t pos;
            cin >> pos;
            cout << dq.at(pos) << endl;
        } else if (cmd == "clear") {
            dq.clear();
        }
    }
    
    return 0;
}
