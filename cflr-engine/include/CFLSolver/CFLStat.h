//
// Created by kisslune on 7/5/22.
//

#ifndef POCR_SVF_CFLSTAT_H
#define POCR_SVF_CFLSTAT_H

#include "SVF-LLVM/BasicTypes.h"
#include "SVFIR/SVFType.h"

namespace SVF
{
class StdCFL;

class CFLGraph;

/*!
 * Statistics of Andersen's analysis
 */
class CFLStat
{
public:
    typedef std::map<const char*, u64_t> NUMStatMap;
    typedef std::map<const char*, double> TIMEStatMap;

    /// num counters
    u32_t numOfIteration;
    u32_t wlIteration;
    u64_t checks;
    u32_t numOfSumEdges;
    u32_t numOfCountEdges;
    u32_t numOfNodes;
    u32_t numOfEdges;
    u32_t degree;
    u32_t pathCnt;

    /// time counters
    double timeOfSolving;
    double startTime;
    double endTime;
    double gsTime;

    /// A set for S edges
    std::map<NodeID, NodeBS> sEdgeSet;

    NUMStatMap generalNumMap;
    NUMStatMap PTNumStatMap;
    TIMEStatMap timeStatMap;
private:
    StdCFL* cfl;

    /// Memory usage, in KB
    u32_t _vmrssUsageBefore;
    u32_t _vmrssUsageAfter;
    u32_t _vmsizeUsageBefore;
    u32_t _vmsizeUsageAfter;

public:
    CFLStat(StdCFL* p) : cfl(p),
                         numOfIteration(0),
                         wlIteration(0),
                         checks(0),
                         numOfSumEdges(0),
                         numOfCountEdges(0),
                         numOfNodes(0),
                         numOfEdges(0),
                         timeOfSolving(0),
                         degree(0),
                         pathCnt(0)
    {
        startClk();
    };

    virtual ~CFLStat()
    {}

    virtual inline void startClk()
    { startTime = CLOCK_IN_MS(); }

    virtual inline void endClk()
    { endTime = CLOCK_IN_MS(); }

    static inline double getClk()
    { return CLOCK_IN_MS(); }

    void setMemUsageBefore();
    void setMemUsageAfter();
    double getMemUsage() const { return (_vmrssUsageAfter - _vmrssUsageBefore) / 1024.0; }

    void setStatInt(const char* key, u64_t val);
    void setStatFloat(const char* key, double val);

    void performStat();
    void graphStat();
    virtual void printStat(std::string str = "");
    void writeSPairsIntoFile(std::string fName);
};
}

#endif //POCR_SVF_CFLSTAT_H
