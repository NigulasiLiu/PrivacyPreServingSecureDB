#include "merkel.h"
#include "tools.h"
#include <cmath>
#include <queue>
#include <string>
#include <utility>
#include <iostream>
#include <vector>
using namespace std;

MerkelTree::MerkelTree(hash_func hfunc)
    : hfunc(hfunc), level(0), leaf_bgn_id(0), n_leaves(0)
{}

void MerkelTree::BuildTree(const std::vector<std::string>& leaves)
{
    DeleteTree();
    n_leaves = leaves.size();
    int bottom = 1;
    while ((bottom << 1) < n_leaves) {
        bottom <<= 1;
    }
    level = 0;
    for (int i(bottom); i != 0; i >>= 1) level++;
    leaf_bgn_id = n_leaves;
    // leaf_bgn_id = 1;
    // for (int i(1); i < level; ++i) leaf_bgn_id <<= 1;
    int nid = leaf_bgn_id;
    priority_queue<pair<int, string>, vector<pair<int, string>>, pq_cmp_func> pq;
    for (auto& leaf : leaves) {
        pair<int, string> node{nid, hfunc(leaf)};
        tree.emplace(node);
        if ((nid & 1) == 0) {
            pq.emplace(node);
        }
        nid++;
    }
    while (!pq.empty()) {
        const auto& left_node = pq.top();
        if (tree.count(left_node.first+1) == 0) {
            LOG_FATAL("right node does not exist. left node's id is " + to_string(left_node.first));
            break;
        }
        const auto& right_node_hash = tree[left_node.first+1];
        nid = left_node.first >> 1;
        pair<int, string> node{nid, hfunc(left_node.second + right_node_hash)};
        pq.pop();
        tree.emplace(node);
        if ((nid & 1) == 0) {
            pq.emplace(node);
        }
    }
}

void MerkelTree::DeleteTree()
{
    tree.clear();
    level = -1;
    leaf_bgn_id = 0;
    n_leaves = 0;
}

string MerkelTree::ComputeRootHash(std::vector<std::pair<int, std::string>> &nodes, hash_func hfunc)
{
    priority_queue<pair<int, string>, vector<pair<int, string>>, pq_cmp_func> pq;
    for (const auto& i : nodes) {
        pq.emplace(i);
    }
    int has_right_node = 0;
    pair<int, string> right_node;
    while (!pq.empty()) {
        if (has_right_node) {
            auto& left_node = pq.top();
            if (right_node.first != left_node.first+1) {
                LOG_WARN("left node's id does match with right node. left id is "
                    + to_string(left_node.first)
                    + ", right id is " + to_string(right_node.first));
            }
            int nid = left_node.first >> 1;
            string hash_str = hfunc(left_node.second + right_node.second);
            pq.emplace(nid, hash_str);
        }
        else {
            right_node = pq.top();
        }
        pq.pop();
        has_right_node ^= 1;
    }
    if (right_node.first != 1) {
        LOG_WARN("root's id is wrong, the result may be incorrect");
    }
    return right_node.second;
}

vector<vector<int>> MerkelTree::TreeMask(int id)
{
    vector<vector<int>> ret(level+1);
    int should_tag_id = BrotherNodeId(id);
    for (int l = level; l >= 0; --l) {
        int n_nodes = 1 << l;
        if (l == level)
            ret[l].resize(tree.size() - n_nodes + 1);
        else
            ret[l].resize(n_nodes);
        if (should_tag_id >= n_nodes) {
            // LOG_INFO("tag node: " + to_string(should_tag_id));
            ret[l][should_tag_id-n_nodes] = 1;
            should_tag_id >>= 1;
            should_tag_id = BrotherNodeId(should_tag_id);
        }
    }
    return ret;
}