#include <iostream>
#include <cassert>
#include <vector>
#include "trie.hpp"
#include "patricia.hpp"
#include "splay_tree.hpp"
#include "treap.hpp"
#include "kd_tree.hpp"
#include "bst.hpp"
#include "avl_tree.hpp"

void testarTrie() {
    Trie trie;

    trie.insert("casa");
    trie.insert("casamento");
    trie.insert("casaco");
    trie.insert("carro");
    trie.insert("dado");

    assert(trie.size() == 5);
    assert(trie.search("casa") == true);
    assert(trie.search("casamento") == true);
    assert(trie.search("casaco") == true);
    assert(trie.search("carro") == true);
    assert(trie.search("dado") == true);
    assert(trie.search("cas") == false);
    assert(trie.search("computador") == false);

    assert(trie.startsWith("cas") == true);
    assert(trie.startsWith("car") == true);
    assert(trie.startsWith("x") == false);

    auto sugestoes = trie.autocomplete("casa");
    assert(sugestoes.size() == 3);

    assert(trie.remove("casamento") == true);
    assert(trie.search("casamento") == false);
    assert(trie.search("casa") == true);
    assert(trie.size() == 4);

    assert(trie.remove("casa") == true);
    assert(trie.search("casa") == false);
    assert(trie.search("casaco") == true);
    assert(trie.size() == 3);

    assert(trie.remove("inexistente") == false);
    std::cout << "[OK] Trie" << std::endl;
}

void testarPatricia() {
    PatriciaTree pat;

    pat.insert("computador");
    pat.insert("computacao");
    pat.insert("computar");
    pat.insert("compilador");
    pat.insert("dado");

    assert(pat.size() == 5);
    assert(pat.search("computador") == true);
    assert(pat.search("computacao") == true);
    assert(pat.search("computar") == true);
    assert(pat.search("compilador") == true);
    assert(pat.search("dado") == true);
    assert(pat.search("computa") == false);

    pat.insert("computa");
    assert(pat.size() == 6);
    assert(pat.search("computa") == true);

    assert(pat.remove("computacao") == true);
    assert(pat.search("computacao") == false);
    assert(pat.search("computador") == true);
    assert(pat.search("computa") == true);
    assert(pat.size() == 5);

    assert(pat.remove("computa") == true);
    assert(pat.search("computa") == false);
    assert(pat.search("computador") == true);
    assert(pat.size() == 4);

    assert(pat.remove("inexistente") == false);
    std::cout << "[OK] Patricia" << std::endl;
}

void testarSplay() {
    SplayTree<int> splay;

    std::vector<int> valores = {50, 30, 70, 20, 40, 60, 80};
    for (int v : valores) {
        splay.insert(v);
    }
    assert(splay.size() == 7);
    assert(splay.getRoot()->chave == 80);

    assert(splay.search(30) == true);
    assert(splay.getRoot()->chave == 30);

    assert(splay.search(999) == false);

    assert(splay.remove(30) == true);
    assert(splay.search(30) == false);
    assert(splay.size() == 6);

    assert(splay.remove(999) == false);
    std::cout << "[OK] Splay Tree" << std::endl;
}

void testarTreap() {
    Treap<int> treap(12345);

    std::vector<int> valores = {15, 10, 20, 5, 12, 18, 25};
    for (int v : valores) {
        treap.insert(v);
    }
    assert(treap.size() == 7);

    for (int v : valores) {
        assert(treap.search(v) == true);
    }
    assert(treap.search(100) == false);

    assert(treap.remove(10) == true);
    assert(treap.search(10) == false);
    assert(treap.size() == 6);

    assert(treap.remove(15) == true);
    assert(treap.search(15) == false);
    assert(treap.size() == 5);

    assert(treap.remove(999) == false);
    std::cout << "[OK] Treap" << std::endl;
}

void testarKDTree() {
    KDTree<2> kd2;

    Ponto<2> p1({3.0, 6.0});
    Ponto<2> p2({17.0, 15.0});
    Ponto<2> p3({13.0, 15.0});
    Ponto<2> p4({6.0, 12.0});
    Ponto<2> p5({9.0, 1.0});
    Ponto<2> p6({2.0, 7.0});
    Ponto<2> p7({10.0, 19.0});

    kd2.insert(p1);
    kd2.insert(p2);
    kd2.insert(p3);
    kd2.insert(p4);
    kd2.insert(p5);
    kd2.insert(p6);
    kd2.insert(p7);

    assert(kd2.size() == 7);
    assert(kd2.search(p3) == true);
    assert(kd2.search(Ponto<2>({99.0, 99.0})) == false);

    auto resIntervalo = kd2.rangeSearch(Ponto<2>({0.0, 0.0}), Ponto<2>({10.0, 10.0}));
    assert(resIntervalo.size() == 3);

    Ponto<2> maisProximo = kd2.nearestNeighbor(Ponto<2>({10.0, 2.0}));
    assert(maisProximo == p5);

    std::cout << "[OK] KD-Tree (2D)" << std::endl;
}

void testarBaselines() {
    BST<int> bst;
    AVLTree<int> avl;

    for (int i = 1; i <= 10; ++i) {
        bst.insert(i);
        avl.insert(i);
    }
    assert(bst.size() == 10);
    assert(avl.size() == 10);
    assert(avl.height() <= 4);

    assert(bst.remove(5) == true);
    assert(avl.remove(5) == true);
    assert(bst.size() == 9);
    assert(avl.size() == 9);
    std::cout << "[OK] Baselines (BST e AVL)" << std::endl;
}

int main() {
    std::cout << "Executando testes unitarios..." << std::endl;

    testarTrie();
    testarPatricia();
    testarSplay();
    testarTreap();
    testarKDTree();
    testarBaselines();

    std::cout << "\nTodos os testes passaram com sucesso." << std::endl;
    return 0;
}
