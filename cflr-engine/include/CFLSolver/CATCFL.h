//
// Created by sch on 2024/8/9.
//

#ifndef LIGHTCFL_H
#define LIGHTCFL_H

#include "CFLSolver.h"

class CATfCFL: public FocrCFL {
public:
    CATfCFL(std::string& _grammarName, std::string& _graphName) : FocrCFL(_grammarName, _graphName) {
        CFLOpt::ucfl.setValue(true);
        CFLOpt::tailor.setValue(true);
    }

    void initSolver() override;
    void countSumEdges() override;
protected:
    // S-Invariant: an S-edge is stored in SPred or cflData().predMap
    unordered_map<NodeID, NodeBS> SPred;

    void processCFLItem(CFLItem item) override;

    void iteratePreds(const CFLItem& item);
    void iterateSuccs(const CFLItem& item);
    void processUnary(const CFLItem& item);

    virtual void processSrcs(const NodeBS& srcs, NodeID dst, const Label& newTy);
    virtual void processDsts(NodeID src, const NodeBS& dsts, const Label& newTy);
    virtual void processSingle(NodeID src, NodeID dst, const Label& newTy);

    NodeBS checkAndAddSrcs(const NodeBS& srcSet, NodeID dst, const Label& lbl);
    NodeBS checkAndAddDsts(NodeID src, const NodeBS& dstSet, const Label& lbl);

    virtual u64_t getSCnt();
    virtual u64_t getTotalGDegree();

    void pushWL(NodeID src, NodeID dst, Label lbl, bool pred, bool succ, bool unary, bool primary = true);
};


class CATtCFL: public CATfCFL {
public:
    CATtCFL(std::string& _grammarName, std::string& _graphName) : CATfCFL(_grammarName, _graphName) { }

protected:
    unordered_map<NodeID, NodeBS> SSucc;

    void processSrcs(const NodeBS& srcs, NodeID dst, const Label& newTy) override;
    void processDsts(NodeID src, const NodeBS& dsts, const Label& newTy) override;

    virtual NodeBS checkAndAddSrcs(const NodeBS& srcSet, NodeID dst, const Label& lbl, bool start, bool pred);
    virtual NodeBS checkAndAddDsts(NodeID src, const NodeBS& dstSet, const Label& lbl, bool start, bool succ);

    void countSumEdges() override;

    u64_t getSCnt() override;
    u64_t getTotalGDegree() override;
};

class CATmCFL: public CATtCFL {
public:
    CATmCFL(std::string& _grammarName, std::string& _graphName) : CATtCFL(_grammarName, _graphName) { }

    bool query(NodeID src, NodeID dst, const Label& lbl) override;
protected:
    CFLData minData;

    NodeBS checkAndAddSrcs(const NodeBS& srcSet, NodeID dst, const Label& lbl, bool start, bool pred) override;
    NodeBS checkAndAddDsts(NodeID src, const NodeBS& dstSet, const Label& lbl, bool start, bool succ) override;

    u64_t getTotalGDegree() override;
};


#endif //LIGHTCFL_H
