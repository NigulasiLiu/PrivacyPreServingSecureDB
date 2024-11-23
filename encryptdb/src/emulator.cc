#include "emulator_struct.h"
#include "merkel.h"
#include "tools.h"
#include <cstdio>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include <pbc/pbc_field.h>
#include <string>
#include "fhe.h"
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

FHE::PHE phe;
GlobalParams global_params;
DO data_owner;
CS cloud;
QU client;
string filename = "../dataset/n100-byte2.json";
Dataset dataset;
vector<MerkelTree>  row_merkel_tree;
G1 commit_val;
hash_func hash_function = sha256_wrapper;
void Setup(int sqrt_n)
{
    FILE *sys_param_file = fopen("../params/pairing.param", "r");
    global_params.e.init(Pairing(sys_param_file).get_pbc_param_t());
    global_params.g = G1(global_params.e, false);
    // global_params.gt = GT(global_params.e, false);
    global_params.gt = global_params.e(global_params.g, global_params.g);
    vector<Zr> z;
    z.reserve(sqrt_n);
    global_params.h.resize(sqrt_n);
    for (int i(0); i < sqrt_n; ++i) {
        z.emplace_back(global_params.e, true);
        global_params.h[i] = global_params.g ^ z[i];
    }
    global_params.hh.setN(sqrt_n*sqrt_n);
    for (int i(0); i < sqrt_n; ++i) {
        for (int j(0); j < sqrt_n; ++j) {
            if (i != j) {
                global_params.hh.set(i, j, global_params.g^(z[i] * z[j]));
            }
        }
    }

    data_owner.sk.x1 = Zr(global_params.e, true);
    data_owner.sk.x2 = Zr(global_params.e, true);
    data_owner.pk.zx = global_params.gt ^ data_owner.sk.x1;
    data_owner.pk.gx = global_params.g ^ data_owner.sk.x2;

    client.sk.x1 = Zr(global_params.e, true);
    client.sk.x2 = Zr(global_params.e, true);
    client.pk.zx = global_params.gt ^ client.sk.x1;
    client.pk.gx = global_params.g ^ client.sk.x2;

    cloud.rk = client.pk.gx ^ data_owner.sk.x1;
}

void Preprocess()
{
    dataset.RetrieveDataset(filename);
    mpz_t gt_mpz;
    mpz_init(gt_mpz);
    for (int i(0); i < dataset.getN(); ++i) {
        long val = dataset.db_get(i);
        // Zr db_ij(global_params.e, val);
        mpz_class db_ij(val);
        Zr s_ij(global_params.e, true);
        EdbElement cipher;
        cipher.gs = global_params.g ^ s_ij;
        GT gt_num = global_params.gt ^ (data_owner.sk.x1 * s_ij);
        element_to_mpz(gt_mpz, const_cast<element_s*>(gt_num.getElement()));
        mpz_class gt_mpz_class(gt_mpz);
        // if (i == 0) cout << "obf = " << gt_mpz_class.get_str(16) << endl;
        db_ij *= gt_mpz_class;
        cipher.num = db_ij;
        // if (i < 100000) {
        //     auto cipher_mpz = cipher.toMpzClass();
        //     long cipher_bitlen = mpz_sizeinbase(cipher_mpz.get_mpz_t(), 2);
        //     cout << "cipher bitlen = " << cipher_bitlen << endl;
        //     EdbElement recover_element;
        //     recover_element.fromMpzClass(global_params.e, cipher_mpz);
        //     if (cipher.gs == recover_element.gs && cipher.num == recover_element.num) 
        //     {
        //         cout << "fromMpzClass() correct!" << endl;
        //     } else {
        //         cout << "Wrong!" << endl;
        //         if (!(cipher.gs == recover_element.gs)) {
        //             cout << "gs incorrect" << endl;
        //             cipher.gs.dump(stdout, "cipher.gs");
        //             recover_element.gs.dump(stdout, "recover_element");
        //         }
        //         if (cipher.num != recover_element.num)
        //         {
        //             cout << "num incorrect" << endl;
        //         }
        //     }
        // }
        dataset.edb_set(i, cipher);
    }
    mpz_clear(gt_mpz);
}

void Commit()
{
    auto sqrt_n = dataset.getSqrtN();
    row_merkel_tree.resize(sqrt_n);
    vector<string> leaves(sqrt_n);
    for (int rid(0); rid < sqrt_n; ++rid) {
        for (int i(0); i < sqrt_n; ++i) {
            leaves[i] = dataset.edb_get(rid, i).toString();
        }
        row_merkel_tree[rid].setHashFunc(hash_function);
        row_merkel_tree[rid].BuildTree(leaves);
        string hash_str = row_merkel_tree[rid].getRootHash();
        Zr root_num(global_params.e, (unsigned char*)&hash_str[0], hash_str.size(), 16);
        G1 val = global_params.h[rid] ^ root_num;
        if (rid == 0) {
            commit_val = val;
        } else {
            commit_val *= val;
        }
    }
}

// return [cipher_mask, encrypted_tree_mask]
pair<vector<mpz_class>, vector<vector<mpz_class>>> EncodeQuery(int rid, int cid)
{
    phe.set_params_from_file("../params/phe.json");
    vector<mpz_class> cipher_mask;
    cipher_mask.reserve(dataset.getSqrtN());
    for (int i(0); i < dataset.getSqrtN(); ++i) {
        long bit = i == cid;
        cipher_mask.emplace_back(phe.encrypt(bit));
    }
    auto& rm_tree = row_merkel_tree[rid];
    auto leaf_id = rm_tree.getLeafBgnId() + cid;
    auto tree_mask = rm_tree.TreeMask(leaf_id);

    // cout << "tree_mask = " << endl;
    // for (auto& arr : tree_mask) {
    //     for (auto i : arr) {
    //         cout << i << ", ";
    //     }
    //     cout << endl;
    // }

    vector<vector<mpz_class>> encrypted_tree_mask(tree_mask.size());
    for (int i(0); i < tree_mask.size(); ++i) {
        int n_nodes = tree_mask[i].size();
        encrypted_tree_mask[i].resize(n_nodes);
        for (int j(0); j < n_nodes; ++j) {
            encrypted_tree_mask[i][j] = phe.encrypt(tree_mask[i][j]);
        }
    }
    return make_pair(std::move(cipher_mask), std::move(encrypted_tree_mask));
}

// return [pi, cipher, tree_nodes]
tuple<G1, mpz_class, vector<mpz_class>> Query(int rid, vector<mpz_class>& cipher_mask, vector<vector<mpz_class>>& encrypted_tree_mask)
{
    string hash_str;
    Zr root_num;
    G1 pi;
    bool is_first = true;
    for (int i(0); i < dataset.getSqrtN(); ++i) {
        if (i == rid) continue;
        hash_str = row_merkel_tree[i].getRootHash();
        root_num = Zr(global_params.e, (unsigned char*)&hash_str[0], hash_str.size(), 16);
        if (is_first) {
            pi = global_params.hh.get(rid, i) ^ root_num;
            is_first = false;
        } else {
            pi *= global_params.hh.get(rid, i) ^ root_num;
        }
    }

    mpz_class cipher;
    vector<mpz_class> tree_nodes;
    // compute cipher
    for (int i(0); i < cipher_mask.size(); ++i) {
        auto item = phe.mul(cipher_mask[i], dataset.edb_get(rid, i).toMpzClass());
        if (i == 0) {
            cipher = item;
        } else {
            cipher = phe.add(cipher, item);
        }
    }

    // compute tree_nodes
    auto& rm_tree = row_merkel_tree[rid];
    tree_nodes.reserve(tree_nodes.size());
    for (int i(0); i < encrypted_tree_mask.size(); ++i) {
        mpz_class encrypted_hash;
        for (int j(0); j < encrypted_tree_mask[i].size(); ++j) {
            auto node_hash = mpz_class(rm_tree.getHash((1 << i)+j), 16);
            auto item = phe.mul(encrypted_tree_mask[i][j], node_hash);
            // cout << (1 << i)+j << ": node_hash = " << node_hash.get_str(16) 
            //     << ", decrypt hash = " << phe.decrypt(item).get_str(16) << endl;

            if (j == 0) {
                encrypted_hash = item;
            } else {
                encrypted_hash = phe.add(encrypted_hash, item);
            }
        }
        // cout << "encrypted_hash = " << phe.decrypt(encrypted_hash).get_str(16) << endl;
        tree_nodes.emplace_back(std::move(encrypted_hash));
    }

    return make_tuple(std::move(pi), std::move(cipher), std::move(tree_nodes));
}

bool Verify(int rid, int cid, G1 pi, mpz_class& cipher, vector<mpz_class>& tree_nodes)
{
    auto sqrt_n = dataset.getSqrtN();
    // MerkelTree mt(hash_function);
    // vector<string> leaves(sqrt_n);
    // for (int i(0); i < sqrt_n; ++i) {
    //     leaves[i] = dataset.edb_get(rid, i).toString();
    // }
    // mt.BuildTree(leaves);
    // string hash_str = mt.getRootHash();
    // printf("%s\n%s\n", row_merkel_tree[rid].getRootHash().c_str(), hash_str.c_str());

    // recover edb[rid][cid]
    EdbElement edb;
    mpz_class content = phe.decrypt(cipher);
    if (content != dataset.edb_get(rid, cid).toMpzClass()) {
        LOG_FATAL("decrypt cipher wrong!");
    }
    edb.fromMpzClass(global_params.e, content);
    if (edb != dataset.edb_get(rid, cid)) {
        dataset.edb_get(rid, cid).toMpzClass();
        LOG_FATAL("cipher is wrong!");
    }

    // recover merkel tree root node's hash
    string hash_str;
    vector<pair<int, string>> nodes;
    nodes.reserve(tree_nodes.size());
    auto node_id = sqrt_n + cid;
    auto now_id = BrotherNodeId(node_id);
    for (int i(tree_nodes.size() - 1); i >= 0; --i) {
        if (now_id < 2) break;
        if (now_id >= (1 << i)) {
            auto node_hash = phe.decrypt(tree_nodes[i]).get_str(16);
            // cout << "[" << to_string(now_id) << "] node_hash = " << node_hash 
            //     << ", real hash = " << row_merkel_tree[rid].getHash(now_id) << endl; 
            nodes.emplace_back(now_id, node_hash);
            now_id = BrotherNodeId(now_id >> 1);
        }
    }
    nodes.emplace_back(node_id, sha256_wrapper(edb.toString()));
    hash_str = MerkelTree::ComputeRootHash(nodes, hash_function);
    if (hash_str != row_merkel_tree[rid].getRootHash())
    {
        // LOG_FATAL("hash_str is wrong! \n\thash_str = " + hash_str + 
        //     "\n\treal root hash = " + row_merkel_tree[rid].getRootHash());
    }

    Zr root_num(global_params.e, (unsigned char*)&hash_str[0], hash_str.size(), 16);
    G1 left1 = commit_val / (global_params.h[rid] ^ root_num);
    bool result = 
        global_params.e(left1, global_params.h[rid]) == global_params.e(pi, global_params.g);
    return result;
}

mpz_class Decrypt(int rid, int cid)
{
    Zr r(global_params.e, true);
    auto gsr = dataset.edb_get(rid, cid).gs ^ r;
    
    auto rk_result = global_params.e(gsr, cloud.rk);

    rk_result ^= r.inverse();

    rk_result ^= client.sk.x2.inverse();

    mpz_t tmp;
    mpz_init(tmp);
    element_to_mpz(tmp, const_cast<element_s*>(rk_result.getElement()));
    mpz_class result(tmp);
    // cout << "obf = " << result.get_str(16) << endl;
    result = dataset.edb_get(rid, cid).num / result;
    
    mpz_clear(tmp);
    return result;
}

void Update(int rid, int cid, int v)
{
    dataset.db_set(rid, cid, v);
    auto sqrt_n = dataset.getSqrtN();
    mpz_t gt_mpz;
    mpz_init(gt_mpz);

    for (int j(0); j < sqrt_n; ++j) {
        long val = dataset.db_get(rid, j);
        // Zr db_ij(global_params.e, val);
        mpz_class db_ij(val);
        Zr s_ij(global_params.e, true);
        EdbElement cipher;
        cipher.gs = global_params.g ^ s_ij;
        GT gt_num = global_params.gt ^ (data_owner.sk.x1 * s_ij);
        element_to_mpz(gt_mpz, const_cast<element_s*>(gt_num.getElement()));
        db_ij *= mpz_class(gt_mpz);
        cipher.num = db_ij;
        
        dataset.edb_set(rid, j, cipher);
    }

    mpz_clear(gt_mpz);
    string old_hash_str = row_merkel_tree[rid].getRootHash();
    Zr old_root_num(global_params.e, (unsigned char*)&old_hash_str[0], old_hash_str.size(), 16);
    vector<string> leaves(sqrt_n);
    for (int i(0); i < sqrt_n; ++i) {
        leaves[i] = dataset.edb_get(rid, i).toString();
    }
    row_merkel_tree[rid].BuildTree(leaves);
    string hash_str = row_merkel_tree[rid].getRootHash();
    Zr root_num(global_params.e, (unsigned char*)&hash_str[0], hash_str.size(), 16);
    G1 val = global_params.h[rid] ^ (root_num - old_root_num);
    commit_val *= val;
}

void emulator(const EmulatorParams* params)
{
    int rid = 1;
    int cid = 2;
    TimeCounter tc;
    if (params != nullptr) {
        filename = params->filename;
    }
    int n = get_n_from_filename(filename);
    LOG_INFO("n = " + to_string(n));

    tc.Start();
    Setup(sqrt(n));
    tc.EndAndPrintSec("Setup");

    tc.Start();
    Preprocess();
    tc.EndAndPrintSec("Preprocess");

    tc.Start();
    Commit();
    tc.EndAndPrintSec("Commit");

    tc.Start();
    auto [cipher_mask, encrypted_tree_mask] = EncodeQuery(rid, cid);
    tc.EndAndPrintMs("EncodeQuery");

    tc.Start();
    auto [pi, cipher, tree_nodes] = Query(rid, cipher_mask, encrypted_tree_mask);
    tc.EndAndPrintMs("Query");

    tc.Start();
    bool v = Verify(rid, cid, pi, cipher, tree_nodes);
    tc.EndAndPrintMs("Verify");
    if (v) {
        LOG_INFO("verification passed");
    } else {
        LOG_FATAL("verification failed");
    }

    tc.Start();
    auto result = Decrypt(rid, cid);
    tc.EndAndPrintMs("Decrypt"); 

    if (result.get_si() == dataset.db_get(rid, cid)) {
        LOG_INFO("decrypt result is correct");
    } else {
        LOG_FATAL(string("decrypt result is wrong. src is ") + to_string(dataset.db_get(rid, cid)) + ". result is " + result.get_str());
    }

    tc.Start();
    Update(rid, cid, 1234);
    tc.EndAndPrintMs("Update");


    // test
    // pi = Query(rid);
    // result = Decrypt(rid, cid);
    // if (result.get_si() == dataset.db_get(rid, cid)) {
    //     LOG_INFO("update decrypt result is correct");
    // } else {
    //     LOG_FATAL(string("update decrypt result is wrong. src is ") + to_string(dataset.db_get(rid, cid)) + ". result is " + result.get_str());
    // }
}

void testcode()
{
    auto& e = global_params.e;
    auto& g = global_params.g;
    Zr a(e, true), b(e, true);
    auto ga = g ^ a;
    auto gab = g ^ (a + b);
    gab = (g^a) * (g^b);
    if (ga == (gab / (g^b))) {
        cout << "div can do" << endl;
    } else {
        cout << "div is wrong" << endl;
    }
    if (ga == (gab ^ (b.inverse()))) {
        cout << "yes!!!" << endl;
    } else {
        cout << "Oh no" << endl;
    }
}
