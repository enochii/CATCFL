/* -------------------- CFLSolver.cpp ------------------ */
//
// Created by kisslune on 7/5/22.
//

#include "CFLSolver/CFLSolver.h"
#include "CFLData/Consts.h"
#include "FileUtil.h"

using namespace SVF;

void StdCFL::initialize()
{
    _grammar = new CFG();
    _grammar->parseGrammar(grammarName);

    _graph = new CFLGraph(_grammar);
    _graph->readGraph(graphName);

    stat = new CFLStat(this);
    stat->setMemUsageBefore();

//    initSolver();
}


void StdCFL::finalize()
{
    stat->setMemUsageAfter();

    if (CFLOpt::srcSnkCheck())
        analyzeSrcSnk();

    dumpStat();
    if (!CFLOpt::outGraphFName().empty())
        graph()->writeGraph(CFLOpt::outGraphFName());
}


void StdCFL::analyze()
{
    std::thread th(StdCFL::timer);      // timer thread

    initialize();

    /// start solving
    double propStart = stat->getClk();

    if (CFLOpt::tailor()) {
        grammar()->computeContext(graph());
        if (CFLOpt::ctxStat())
            stat->setStatFloat("#ctxTimeMs", stat->getClk() - propStart);
    }

    initSolver();

    do
    {
        stat->numOfIteration++;
        reanalyze = false;
        if (CFLOpt::solveCFL())
            solve();
    } while (reanalyze);

    double propEnd = stat->getClk();
    stat->timeOfSolving += (propEnd - propStart) / TIMEINTERVAL;

    finalize();

    pthread_cancel(th.native_handle());     // kill timer
    th.join();
}


Set<Label> StdCFL::unarySumm(Label lty)
{
    Set<Label> retVal;
    auto& lhsSet = grammar()->getLhs(lty.first);

    for (auto lhs : lhsSet)
    {
        if (!lhs)
            continue;

        if (grammar()->isaVariantSymbol(lhs) && grammar()->isaVariantSymbol(lty.first))
            retVal.insert(Label(lhs, lty.second));
        else
            retVal.insert(Label(lhs, 0));
    }

    return retVal;
}


Set<Label> StdCFL::binarySumm(Label lty, Label rty)
{
    Set<Label> retVal;
    auto lhsSet = grammar()->getLhs(std::make_pair(lty.first, rty.first));

    for (auto lhs : lhsSet)
    {
        if (!lhs)       // a fault label
            continue;

        if (grammar()->isaVariantSymbol(lty.first))
        {
            if ((grammar()->isaVariantSymbol(rty.first) && lty.second == rty.second)
                || !grammar()->isaVariantSymbol(rty.first))
            {
                if (grammar()->isaVariantSymbol(lhs))
                    retVal.insert(Label(lhs, lty.second));
                else
                    retVal.insert(Label(lhs, 0));
            }
        }
        else if (grammar()->isaVariantSymbol(rty.first))
        {
            if (grammar()->isaVariantSymbol(lhs))
                retVal.insert(Label(lhs, rty.second));
            else
                retVal.insert(Label(lhs, 0));
        }
        else
            retVal.insert(Label(lhs, 0));
    }

    return retVal;
}


void StdCFL::initSolver()
{
    /// add all edges into adjacency list and worklist
    for (auto edge : graph()->getCFLEdges())
    {
        NodeID src = edge->getSrcID(), dst = edge->getDstID();
        auto lbl = std::make_pair(edge->getEdgeKind(), edge->getEdgeIdx());
//        cflData()->addEdge(src, dst, lbl);
//        pushIntoWorklist(src, dst, lbl);
        if (checkAndAddEdge(src, dst, lbl))
            pushIntoWorklist(src, dst, lbl);
    }

    /// processing empty rules, i.e., X ::= epsilon
    for (auto nIter = graph()->begin(); nIter != graph()->end(); ++nIter)
    {
        NodeID nodeId = nIter->first;
        for (auto lhs : grammar()->getEmptyRules())
        {
            auto lbl = std::make_pair(lhs, 0);
//            cflData()->addEdge(nodeId, nodeId, lbl);
//            pushIntoWorklist(nodeId, nodeId, lbl);
            if (checkAndAddEdge(nodeId, nodeId, lbl))
                pushIntoWorklist(nodeId, nodeId, lbl, false);
        }
    }
}


void StdCFL::dumpStat()
{
    if (CFLOpt::PStat() && stat)
        stat->performStat();
}


void StdCFL::countSumEdges()
{
    /// calculate summary edges
    stat->numOfSumEdges = 0;
    for (auto it1 = cflData()->begin(); it1 != cflData()->end(); ++it1)
        for (auto& it2 : it1->second)
            stat->numOfSumEdges += it2.second.count();

    /// calculate S edges
    stat->sEdgeSet.clear();
    for (auto& it1 : cflData()->getSuccMap())
        for (auto& it2 : it1.second)
            if (grammar()->isCountSymbol(it2.first.first))
                stat->sEdgeSet[it1.first] |= it2.second;

    for (auto& it : stat->sEdgeSet)
        it.second.reset(it.first);

    stat->numOfCountEdges = 0;
    for (auto& it1 : stat->sEdgeSet) {
        stat->numOfCountEdges += it1.second.count();
    }

    stat->degree = cflData()->getLabelDegree();
    stat->PTNumStatMap[GRAPH_DEGREE] = cflData()->getEdgeCnt(true) + cflData()->getEdgeCnt(false);
}


/// ---------------- CFL data methods with dynamic skewing option ----------------------------

void StdCFL::addEdge(NodeID src, NodeID dst, Label lbl)
{
    if (!lbl.first)
        return;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return;

    cflData()->addEdge(src, dst, lbl);
}


bool StdCFL::checkAndAddEdge(NodeID src, NodeID dst, Label lbl)
{
    if (!lbl.first)
        return false;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return true;
    stat->checks++;
    return cflData()->checkAndAddEdge(src, dst, lbl);
}


NodeBS StdCFL::checkAndAddEdges(NodeID src, const NodeBS& dstSet, Label lbl)
{
    if (!lbl.first)
        return emptyBS;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return dstSet;
    stat->checks += dstSet.count();

    return cflData()->checkAndAddEdges(src, dstSet, lbl);
}


NodeBS StdCFL::checkAndAddEdges(const NodeBS& srcSet, NodeID dst, Label lbl)
{
    if (!lbl.first)
        return emptyBS;

    if (CFLOpt::ucfl() && !grammar()->isInsertSymbol(lbl.first))
        return srcSet;
    stat->checks += srcSet.count();

    return cflData()->checkAndAddEdges(srcSet, dst, lbl);
}

bool StdCFL::query(NodeID src, NodeID dst, const Label& lbl) {
    return cflData()->getSuccs(src, lbl).test(dst);
}


vector<CFLItem> StdCFL::getStartEdges() {
    vector<CFLItem> sEdges;
    for (auto& it1: cflData()->getSuccMap()) {
        auto src = it1.first;
        for (auto& it2: it1.second) {
            if (grammar()->isCountSymbol(it2.first.first)) {
                for (auto dst: it2.second) {
                    sEdges.emplace_back(src, dst, it2.first);
                }
            }
        }
    }
    return sEdges;
}

void StdCFL::analyzeSrcSnk() {
    NodeBS sources = readSetFromFile(replaceSuffix(graphName, CFLOpt::getSrc()));
    NodeBS sinks = readSetFromFile(replaceSuffix(graphName, CFLOpt::getSnk()));

    u64_t noSnkCnt = 0, pathCnt = 0;

    auto S = std::make_pair(grammar()->getStartSymbol(), 0);
    for (auto src : sources) {
        bool reachable = false;
        for (auto dst: sinks) {
            if (src == dst) continue;

            if (query(src, dst, S)) {
                reachable = true;
                pathCnt++;
            }
        }
        if (!reachable) {
            noSnkCnt++;
        }
    }
    stat->PTNumStatMap["#Src"] = getSize(sources);
    stat->PTNumStatMap["#Snk"] = getSize(sinks);
    stat->PTNumStatMap["#NoSnk"] = noSnkCnt;
    stat->PTNumStatMap["#SrcSnkPath"] =  pathCnt;
}
