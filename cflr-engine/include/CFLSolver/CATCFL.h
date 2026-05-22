//
// Created by sch on 2024/8/9.
//

#ifndef LIGHTCFL_H
#define LIGHTCFL_H

#include "CFLSolver.h"

class CATmCFL: public FocrCFL {
public:
    CATmCFL(std::string& _grammarName, std::string& _graphName) : FocrCFL(_grammarName, _graphName) {
        CFLOpt::ucfl.setValue(true);
        CFLOpt::tailor.setValue(true);
    }

    bool query(NodeID src, NodeID dst, const Label& lbl) override;

protected:
    // S-Invariant: an S-edge is stored in SPred or cflData().predMap
    unordered_map<NodeID, NodeBS> SPred, SSucc;
    CFLData minData;

    void initSolver() override;
    void countSumEdges() override;

    void processCFLItem(CFLItem item) override;
    void iteratePreds(const CFLItem& item);
    void iterateSuccs(const CFLItem& item);
    void processUnary(const CFLItem& item);

    virtual void processSingle(NodeID src, NodeID dst, const Label& newTy);
    virtual void processSrcs(const NodeBS& srcs, NodeID dst, const Label& newTy);
    virtual void processDsts(NodeID src, const NodeBS& dsts, const Label& newTy);

    void pushWL(NodeID src, NodeID dst, Label lbl, bool pred, bool succ, bool unary, bool primary = true);


    NodeBS checkAndAddSrcs(const NodeBS& srcSet, NodeID dst, const Label& lbl, bool start, bool pred);
    NodeBS checkAndAddDsts(NodeID src, const NodeBS& dstSet, const Label& lbl, bool start, bool succ);

    u64_t getTotalGDegree();
    virtual u64_t getSCnt();
};


#endif //LIGHTCFL_H
