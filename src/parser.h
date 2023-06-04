#ifndef PARSER_H
#define PARSER_H

#include <clan/clan.h>
#include <cloog/cloog.h>
#include <osl/osl.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#define SPLIT 1
#define REORDER 2
#define INTERCHANGE 3
#define FUSE 4
#define SKEW 5
#define TILE 6
#define UNROLL 7

using namespace std;

class BaseArg {
   public:
    bool type;
    BaseArg(bool type) : type(type) {}
};

class SingleIntArg : public BaseArg {
   public:
    int arg = 0;
    SingleIntArg() : BaseArg(0) {}
    void display() { cout << arg << endl; }
};

class VectorArg : public BaseArg {
   public:
    std::vector<int> arg;
    VectorArg() : BaseArg(1) {}
    void display() {
        cerr << arg.size() << " : ";
        for (auto i : arg) cout << i << " ";
        cout << endl;
    }
};

bool isdigit(char c) { return c >= '0' && c <= '9'; }

string get_trans(osl_generic_p extension) {
    string arg;
    for (auto p = extension; p != NULL; p = p->next) {
        if (strcmp(p->interface->URI, "clay") == 0) {
            arg = ((char **)p->data)[0];
        }
    }
    return arg;
}

SingleIntArg *read_int(string &str, int &p) {
    while (p < str.length() && !isdigit(str[p]) && str[p] != '-') ++p;

    SingleIntArg *ret = new SingleIntArg();

    int flag = 0;
    if (str[p] == '-') {
        flag = 1;
        ++p;
    } else
        flag = 0;

    while (p < str.length() && isdigit(str[p])) {
        ret->arg = ret->arg * 10 + str[p] - '0';
        ++p;
    }
    ret->arg = flag ? -ret->arg : ret->arg;

    return ret;
}

VectorArg *read_vector(string &str, int &p) {
    VectorArg *ret = new VectorArg();
    while (p < str.length() && str[p] != ']') {
        while (p < str.length() && !isdigit(str[p]) && str[p] != '-' &&
               str[p] != ']')
            ++p;
        if (p >= str.length() || str[p] == ']') break;
        ret->arg.push_back(read_int(str, p)->arg);
    }
    return ret;
}

int parser(osl_scop_p scop, vector<BaseArg *> &args) {
    string str = get_trans(scop->extension);
    // some preprocessing
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find(';'), str.length());

    string func_name = str.substr(0, str.find('('));
    int ret = 0;

    if (func_name == "split") {
        ret = SPLIT;
    } else if (func_name == "reorder") {
        ret = REORDER;
    } else if (func_name == "interchange") {
        ret = INTERCHANGE;
    } else if (func_name == "fuse") {
        ret = FUSE;
    } else if (func_name == "skew") {
        ret = SKEW;
    } else if (func_name == "tile") {
        ret = TILE;
    } else if (func_name == "unroll") {
        ret = UNROLL;
    } else {
        cerr << "Unknown transformation: " << func_name << endl;
        exit(1);
    }

    args.clear();
    int p = str.find('(') + 1;

    while (p < str.length()) {
        while (p < str.length() && !isdigit(str[p]) && str[p] != '-' &&
               str[p] != '[')
            ++p;
        if (p >= str.length()) break;
        if (str[p] == '[') {
            args.push_back(read_vector(str, p));
        } else
            args.push_back(read_int(str, p));
    }

    return ret;
}

#endif