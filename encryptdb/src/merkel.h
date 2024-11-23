#pragma once
#include "tools.h"
#include <cstddef>
#include <vector>
#include <map>
using hash_func = std::string(*)(const std::string&);
inline int BrotherNodeId(int id) {
    id += (id&1) ? -1 : 1;
    return std::max(1, id);
}
class MerkelTree {
    hash_func hfunc;
    // id -> hash
    std::map<int, std::string> tree;
    // begin with 0, from top to bottom
    int level;
    // leaf_bgn_id = n_leaves, because the property: n1 + n2 = n0
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
        return getHash(1);
    }
    inline int getLeafBgnId() const {return leaf_bgn_id;}
    inline std::string getHash(int id) {return tree.count(id) ? tree[id] : "";}
    inline int getLevel() {return level;}
    std::vector<std::vector<int>> TreeMask(int id);
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