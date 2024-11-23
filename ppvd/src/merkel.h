#pragma once
#include "tools.h"
#include <cstddef>
#include <vector>
#include <map>
using hash_func = std::string(*)(const std::string&);

// struct MerkelNode {
//     int nid;
//     std::string hash_code;
//     MerkelNode *parent;
//     MerkelNode *left;
//     MerkelNode *right;
    
//     MerkelNode(): left(nullptr), right(nullptr) {}
//     bool isRoot() { return parent == nullptr; }
//     bool isLeaf() { return left == nullptr && right == nullptr; }
// };

class MerkelTree {
    hash_func hfunc;
    std::map<int, std::string> tree;
    int level;
    int leaf_bgn_id;
    int n_leaves;

public:
    // return the number of leaves
    MerkelTree(): MerkelTree(sha1_wrapper) {}
    MerkelTree(hash_func hfunc);
    void setHashFunc(hash_func hfunc) {this->hfunc = hfunc;}
    inline int size() { return n_leaves; }
    inline hash_func getHashFunc() { return hfunc; }
    inline std::string getRootHash() {
        return tree.count(1) ? tree[1].substr(0, 4*2) : "";
    }
    void BuildTree(const std::vector<std::string>& leaves);
    void DeleteTree();
    static std::string ComputeRootHash(std::vector<std::pair<int, std::string>>& nodes, hash_func hfunc);
private:
    struct pq_cmp_func {
        bool operator()(std::pair<int, std::string>& a, std::pair<int, std::string>& b) {
            return a.first < b.first;
        }
    };
};