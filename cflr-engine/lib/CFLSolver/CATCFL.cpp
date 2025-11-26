//
// Created by enochii on 2024/7/15.
//

#include "CFLSolver/CATCFL.h"

#include "CFLData/Consts.h"
#include "CFLData/Debug.h"

using namespace SVF;


void CATfCFL::initSolver()
{
    /// add all edges into adjacency list and worklist
    for (auto edge : graph()->getCFLEdges())
    {
        NodeID src = edge->getSrcID(), dst = edge->getDstID();
        u32_t sym = edge->getEdgeKind(), idx = edge->getEdgeIdx();
        processSingle(src, dst, make_pair(sym, idx));
    }

    /// processing empty rules, i.e., X ::= epsilon
    for (auto nIter = graph()->begin(); nIter != graph()->end(); ++nIter)
    {
        NodeID nodeId = nIter->first;
        for (auto lhs : grammar()->getEmptyRules())
        {
            auto lbl = std::make_pair(lhs, 0);
            // self-referencing edges are secondary-edges for a transitive relation
            processSingle(nodeId, nodeId, lbl);
        }
    }

    FocrCFL::postInitSolver();
}

void CATfCFL::processCFLItem(CFLItem item) {
    stat->wlIteration ++;
    if (CFLOpt::LStat()) {
        cout << item.src() << " (" << grammar()->getSymbolString(item.label().first) << ") " << item.dst() << endl;
    }

    processUnary(item);
    iterateSuccs(item);
    iteratePreds(item);
}


void CATfCFL::processUnary(const CFLItem &item) {
    if (!item.isUnary())
        return;

    for (Label newTy : unarySumm(item.label()))
        processSingle(item.src(), item.dst(), newTy);
}

NodeBS CATfCFL::checkAndAddSrcs(const NodeBS &srcSet, NodeID dst, const Label &lbl) {
    if (!lbl.first)
        return emptyBS;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return srcSet;
    stat->checks += srcSet.count();

    return cflData()->checkAndAddSrcs(srcSet, dst, lbl);
}

NodeBS CATfCFL::checkAndAddDsts(NodeID src, const NodeBS& dstSet, const Label& lbl) {
    if (!lbl.first)
        return emptyBS;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return dstSet;
    stat->checks += dstSet.count();

    return cflData()->checkAndAddDsts(src, dstSet, lbl);
}

void CATfCFL::pushWL(NodeID src, NodeID dst, Label lbl, bool pred, bool succ, bool unary, bool primary) {
    CFLBase::pushIntoWorklist(CFLItem(src, dst, lbl, primary, pred, succ, unary));
}

u64_t CATfCFL::getSCnt() {
    for (auto& it: cflData()->getPredMap()) {
        NodeID dst = it.first;
        auto& ref = it.second[Label(grammar()->getStartSymbol(), 0)];
        SPred[dst] |= ref;
    }
    return getSize(SPred);
}

u64_t CATfCFL::getTotalGDegree() {
    return cflData()->getGraphDegree() + getSize(SPred);
}

void CATfCFL::iteratePreds(const CFLItem &item) {
    if (!item.isSucc())
        return;

    for (auto& iter : cflData()->getPreds(item.src()))
    {
        Label lty = iter.first;
        for (Label newTy : binarySumm(lty, item.label()))
            processSrcs(iter.second, item.dst(), newTy);
    }
}

void CATfCFL::iterateSuccs(const CFLItem &item) {
    if (!item.isPred())
        return;

    for (auto& iter : cflData()->getSuccs(item.dst()))
    {
        Label rty = iter.first;
        for (Label newTy : binarySumm(item.label(), rty))
            processDsts(item.src(), iter.second, newTy);

    }
}

void CATfCFL::processSrcs(const NodeBS &srcs, NodeID dst, const Label &newTy) {
    auto symbol = grammar()->getSymbol(newTy.first);

    bool pred = symbol->needPred(dst);
    if (!symbol->succFree) {
        NodeBS diffSrcs = checkAndAddEdges(srcs, dst, newTy);
        for (NodeID diffSrc : diffSrcs) {
            bool succ = symbol->needSucc(diffSrc);
            if (pred || succ || symbol->unary)
                pushWL(diffSrc, dst, newTy, pred, succ, symbol->unary);
        }
    } else if (pred) {
        NodeBS diffSrcs = checkAndAddSrcs(srcs, dst, newTy);
        for (NodeID diffSrc : diffSrcs)
            pushWL(diffSrc, dst, newTy, true, false, symbol->unary);
    } else if (symbol->start) {
        SPred[dst] |= srcs;
    } else if (symbol->unary) {
        cout << "unexpected cases" << endl;
        exit(0);
    }
}

void CATfCFL::processDsts(NodeID src, const NodeBS &dsts, const Label &newTy) {
    auto symbol = grammar()->getSymbol(newTy.first);

    bool succ = symbol->needSucc(src);
    if (!symbol->predFree) {
        NodeBS diffDsts = checkAndAddEdges(src, dsts, newTy);
        for (NodeID diffDst : diffDsts) {
            bool pred = symbol->needPred(diffDst);
            if (succ || pred || symbol->unary)
                pushWL(src, diffDst, newTy, pred, succ, symbol->unary);
        }
    } else if (succ) {
        NodeBS diffDsts = checkAndAddDsts(src, dsts, newTy);
        for (NodeID diffDst : diffDsts)
            pushWL(src, diffDst, newTy, false, true, symbol->unary);
    } else if (symbol->unary) {
        for (NodeID dst : dsts)
            pushWL(src, dst, newTy, false, false, true);
    } else if (symbol->start) {
        cout << "unexpected cases" << endl;
        exit(0);
    }
}

void CATfCFL::processSingle(NodeID src, NodeID dst, const Label& newTy) {
    auto symbol = grammar()->getSymbol(newTy.first);

    bool pred = symbol->needPred(dst);
    bool succ = symbol->needSucc(src);

    if (CFLOpt::ucfl() && !symbol->insert) {
        // if (symbol->unary || (!symbol->terminal && (pred || succ)))
        if (symbol->unary || (pred || succ))
            pushWL(src, dst, newTy, pred, succ, symbol->unary);
    } else {
        stat->checks ++;

        bool changed = false;
        if (succ) {
            changed = cflData()->checkAndAddDst(src, dst, newTy);
            if (!changed) return;
        }
        if (pred) {
            changed = cflData()->checkAndAddSrc(src, dst, newTy);
        } else if (symbol->start) {
            setBit(SPred[dst], src);
        }
        // changed is true -> (pred || succ)
        // if (symbol->unary || (changed && (!symbol->terminal)))
        if (symbol->unary || changed)
            pushWL(src, dst, newTy, pred, succ, symbol->unary);
    }
}

void CATfCFL::countSumEdges() {
    stat->degree = cflData()->getLabelDegree();
    stat->numOfSumEdges = cflData()->getEdgeCnt(false);
    stat->setStatInt(GRAPH_DEGREE, cflData()->getGraphDegree());
    stat->setStatInt(TOTAL_G_DEGREE, getTotalGDegree());
    stat->numOfCountEdges = getSCnt() - graph()->getTotalNodeNum();

    if (CFLOpt::dumpG()) {
        dumpFinalG(cflData()->getPredMap(), grammar(), false);
        dumpFinalG(cflData()->getSuccMap(), grammar(), true);
    }
}

void CATtCFL::processSrcs(const NodeBS& srcs, NodeID dst, const Label& newTy) {
    auto symbol = grammar()->getSymbol(newTy.first);

    bool pred = symbol->needPred(dst);
    auto diffSrcs = checkAndAddSrcs(srcs, dst, newTy, symbol->start, pred);

    if (!pred && !symbol->unary && symbol->succFree)
        return;

    for (auto src: diffSrcs) {
        bool succ = !symbol->succFree && symbol->needSucc(src);
        if (CFLOpt::ucfl() && symbol->insert) {
            if (succ && !cflData()->checkAndAddDst(src, dst, newTy))
                continue;

//            if (succ) cflData()->addDst(src, dst, newTy);
//            else if (symbol->start) setBit(SSucc[src], dst);
        }
        if (pred || succ || symbol->unary)
            pushWL(src, dst, newTy, pred, succ, symbol->unary);
    }
}


void CATtCFL::processDsts(NodeID src, const NodeBS &dsts, const Label &newTy) {
    auto symbol = grammar()->getSymbol(newTy.first);

    bool succ = symbol->needSucc(src);
    auto diffDsts = checkAndAddDsts(src, dsts, newTy, symbol->start, succ);

    if (!succ && !symbol->unary && symbol->predFree)
        return;

    for (auto dst: diffDsts) {
        bool pred = !symbol->predFree && symbol->needPred(dst);
        if (CFLOpt::ucfl() && symbol->insert) {
            if (pred && !cflData()->checkAndAddSrc(src, dst, newTy))
                continue;
//            if (pred) cflData()->addSrc(src, dst, newTy);
//            else if (symbol->start) setBit(SPred[dst], src); // uncomment this, need to change `getSCnt()`; see strategy (1)
        }

        if (pred || succ || symbol->unary)
            pushWL(src, dst, newTy, pred, succ, symbol->unary);
    }
}

NodeBS CATtCFL::checkAndAddSrcs(const NodeBS &srcSet, NodeID dst, const Label &lbl, bool start, bool pred) {
    if (start && !pred) {
        stat->checks += srcSet.count();
        auto& bs = SPred[dst];
        NodeBS newSrcs;
        newSrcs.intersectWithComplement(srcSet, bs);
        bs |= newSrcs;
        return newSrcs;
    }
    return CATfCFL::checkAndAddSrcs(srcSet, dst, lbl);
}

NodeBS CATtCFL::checkAndAddDsts(NodeID src, const NodeBS &dstSet, const Label &lbl, bool start, bool succ) {
    if (start && !succ) {
        stat->checks += dstSet.count();
        auto& bs = SSucc[src];
        NodeBS newDsts;
        newDsts.intersectWithComplement(dstSet, bs);
        bs |= newDsts;
        return newDsts;
    }
    return CATfCFL::checkAndAddDsts(src, dstSet, lbl);
}

void CATtCFL::countSumEdges() {
    stat->setStatInt("#ExtraSEdges", getSize(SPred) + getSize(SSucc));
    // side-effect... do not change the ordering
    CATfCFL::countSumEdges();
}

u64_t CATtCFL::getSCnt() {
    if (CFLOpt::verify()) {
        for (auto& it: cflData()->getSuccMap()) {
            NodeID src = it.first;
            auto& ref = it.second[Label(grammar()->getStartSymbol(), 0)];
            SSucc[src] |= ref;
        }

        for (auto& it: SSucc) {
            auto src = it.first;
            for (auto dst: it.second)
                setBit(SPred[dst], src);
        }
    }

    for (auto& it: cflData()->getPredMap()) {
        NodeID dst = it.first;
        auto& ref = it.second[Label(grammar()->getStartSymbol(), 0)];
        SPred[dst] |= ref;
    }

    return getSize(SPred);
// // `strategy (1)`
//    auto minorS = getSize(SPred) - SPred.size();
//    return cflData()->getEdgeCnt(grammar()->getStartSymbol()) + minorS;
}

u64_t CATtCFL::getTotalGDegree() {
    return CATfCFL::getTotalGDegree() + getSize(SSucc);
}

NodeBS CATmCFL::checkAndAddSrcs(const NodeBS &srcSet, NodeID dst, const Label &lbl, bool start, bool pred) {
    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return srcSet;
    stat->checks += srcSet.count();

    if (pred) {
        return cflData()->checkAndAddSrcs(srcSet, dst, lbl);
    } else if (start) {
        auto& bs = SPred[dst];
        NodeBS newSrcs;
        newSrcs.intersectWithComplement(srcSet, bs);
        bs |= newSrcs;
        return newSrcs;
    } else {
        return minData.checkAndAddSrcs(srcSet, dst, lbl);
    }
}

NodeBS CATmCFL::checkAndAddDsts(NodeID src, const NodeBS &dstSet, const Label &lbl, bool start, bool succ) {
    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return dstSet;

    stat->checks += dstSet.count();

    if (succ) {
        return cflData()->checkAndAddDsts(src, dstSet, lbl);
    } else if (start) {
        auto& bs = SSucc[src];
        NodeBS newDsts;
        newDsts.intersectWithComplement(dstSet, bs);
        bs |= newDsts;
        return newDsts;
    } else {
        return minData.checkAndAddDsts(src, dstSet, lbl);
    }
}

u64_t CATmCFL::getTotalGDegree() {
    return CATtCFL::getTotalGDegree() + minData.getGraphDegree();
}

bool CATmCFL::query(NodeID src, NodeID dst, const Label &lbl) {
    return testBit(SPred[dst], src)
            || testBit(SSucc[src], dst)
            || cflData()->getPredMap()[dst][lbl].test(src);
}
