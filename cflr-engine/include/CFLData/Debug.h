//
// Created by enochii on 2025/7/3.
//

#ifndef POCR_SVF_DEBUG_H
#define POCR_SVF_DEBUG_H

#include "BasicUtils.h"

using namespace SVF;

void dumpResult(unordered_map<NodeID, NodeBS>& mp) {
    for (auto& it: mp) {
        cout << it.first << " -> {";
        for (auto dst: it.second)
            cout << dst << ", ";
        cout << "}\n";
    }
}


void dumpResult(unordered_map<NodeID, map<const Label, NodeBS>>& mp, CFG* cfg, bool startonly = false) {
    for (auto& it1: mp) {
        cout << it1.first << " -> {";
        for (auto& it2: it1.second) {
            auto symID = it2.first.first;
            auto sym = cfg->getSymbolString(symID);
            if (startonly && symID != cfg->getStartSymbol()) continue;
            cout << sym << ": {";
            for (auto dst: it2.second)
                cout << dst << ", ";
            cout << "}\n";
        }
        cout << "}\n";
    }
}

void dumpFinalG(unordered_map<NodeID, map<const Label, NodeBS>>& mp, CFG* cfg, bool succ) {
    cout << (succ? "Out":"In") << "\n";
    for (auto& it1: mp) {
        auto key = it1.first;
        for (auto& it2: it1.second) {
            auto symID = it2.first.first;
            auto sym = cfg->getSymbolString(symID);
            for (auto val: it2.second)
                if (succ)
                    cout << key << "\t" << sym << "\t" << val << "\n";
                else
                    cout << val << "\t" << sym << "\t" << key << "\n";
        }
    }
}

#endif //POCR_SVF_DEBUG_H
