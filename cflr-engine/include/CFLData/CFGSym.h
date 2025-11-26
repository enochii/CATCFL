//
// Created by sch on 2024/8/5.
//

#ifndef CFGSYM_H
#define CFGSYM_H

#include <string>
#include "SVF-LLVM/BasicTypes.h"

using SetTy = SVF::Set<SVF::NodeID>;

struct CFGSymbol {
    using NodeBS = SVF::NodeBS;
    using u32_t = SVF::u32_t;
    using NodeID = SVF::NodeID;
    using SetTy = SVF::Set<NodeID>;

    std::string id;
    bool predFree, succFree, predStar, succStar;
    bool variant, start, unary, insert, terminal, follow, hasPair;
    std::pair<u32_t, u32_t> pairedSym;

    SetTy taintedSrcs, taintedDsts;
//    SetTy taintedSrcSet, taintedDstSet;

    void setPairedSym(std::pair<u32_t, u32_t> ps) {
        hasPair = true;
        pairedSym = ps;
    }

    inline bool isDstTainted(NodeID dst) const { return testBit(taintedDsts, dst); }
    inline bool isSrcTainted(NodeID src) const { return testBit(taintedSrcs, src); }

    inline bool needPred(NodeID dst) const { return predStar || isDstTainted(dst); }
    inline bool needSucc(NodeID src) const { return succStar || isSrcTainted(src); }

    CFGSymbol(const std::string& id, bool predFree, bool succFree, bool predStar, bool succStar,
              bool variant, bool start, bool unary, bool insert, bool follow, bool terminal):
              id(id), predFree(predFree), succFree(succFree), predStar(predStar), succStar(succStar),
              variant(variant), start(start), unary(unary), insert(insert), follow(follow), terminal(terminal),
              hasPair(false) {}
};

#endif //CFGSYM_H
