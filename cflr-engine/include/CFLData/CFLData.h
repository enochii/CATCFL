/*
 * CFLSolver.h
 *
 *  Created on: Nov 22, 2019
 *      Author: Yuxiang Lei
 */

#ifndef CFGDATA_H_
#define CFGDATA_H_

#include "Util/WorkList.h"
#include "BasicUtils.h"
#include "CFLSolver/CFLOpt.h"
#include "CFG.h"

// #define CUBIC

namespace SVF
{
/*!
 * Adjacency-list graph representation
 */
class CFLData
{
public:
    typedef std::map<const Label, NodeBS> TypeMap;
    typedef std::unordered_map<NodeID, TypeMap> DataMap;
    typedef typename DataMap::iterator iterator;
    typedef typename DataMap::const_iterator const_iterator;

protected:
    DataMap succMap;
    DataMap predMap;
    const NodeBS emptyData;


public:
    // Constructor
    CFLData() {}

    // Destructor
    virtual ~CFLData() = default;

    virtual void clear()
    {
        succMap.clear();
        predMap.clear();
    }

    inline const_iterator begin() const
    { return succMap.begin(); }

    inline const_iterator end() const
    { return succMap.end(); }

    inline iterator begin()
    { return succMap.begin(); }

    inline iterator end()
    { return succMap.end(); }

    inline DataMap& getSuccMap()
    { return succMap; }

    inline DataMap& getPredMap()
    { return predMap; }

    inline TypeMap& getSuccs(const NodeID key)
    { return succMap[key]; }

    inline TypeMap& getPreds(const NodeID key)
    { return predMap[key]; }

    inline NodeBS& getSuccs(const NodeID key, const Label lbl)
    { return succMap[key][lbl]; }

    inline NodeBS& getPreds(const NodeID key, const Label lbl)
    { return predMap[key][lbl]; }

    inline const NodeBS& getSuccsIfExist(const NodeID key, const Label lbl) {
        auto it = succMap[key].find(lbl);
        if (it != succMap[key].end())
            return it->second;
        return emptyData;
    }

    inline const NodeBS& getPredsIfExist(const NodeID key, const Label lbl) {
        auto it = predMap[key].find(lbl);
        if (it != predMap[key].end())
            return it->second;
        return emptyData;
    }

    inline u32_t getEdgeCnt(CFGSymbTy sym, bool nontrivial = true, bool pred = true) {
        u32_t cnt = 0;
        for (auto& nit: (pred? predMap : succMap)) {
            for (auto& lit: nit.second) {
                if (lit.first.first == sym) {
                    auto& bv = lit.second;
                    if (nontrivial)
                        bv.reset(nit.first);
                    cnt += bv.count();
                }
            }
        }
        return cnt;
    }

    // Alias data operations
    //@{
    inline void addEdge(const NodeID src, const NodeID dst, const Label lbl)
    {
        succMap[src][lbl].set(dst);
        predMap[dst][lbl].set(src);
    }

    inline void addEdges(const NodeID src, const NodeBS& dstSet, const Label lbl)
    {
        if ((succMap[src][lbl] |= dstSet))
        {
            for (const NodeID dst : dstSet)
                predMap[dst][lbl].set(src);
        }
    }

    inline void addEdges(const NodeBS& srcSet, const NodeID dst, const Label lbl)
    {
        if ((predMap[dst][lbl] |= srcSet))
        {
            for (const NodeID src : srcSet)
                succMap[src][lbl].set(dst);
        }
    }

    inline bool checkAndAddEdge(const NodeID src, const NodeID dst, const Label lbl)
    {
        succMap[src][lbl].test_and_set(dst);
        return predMap[dst][lbl].test_and_set(src);
    }

    inline NodeBS checkAndAddEdges(const NodeID src, const NodeBS& dstSet, const Label lbl)
    {
         NodeBS newDsts;
#ifdef CUBIC
        // cubic
         if (CFLOpt::cubic()) {
             for (const NodeID dst : dstSet)
                 if (predMap[dst][lbl].test_and_set(src))
                     newDsts.set(dst);
             succMap[src][lbl] |= newDsts;
         } else {
             if ((succMap[src][lbl] |= dstSet))
             {
                 for (const NodeID dst : dstSet)
                     if (predMap[dst][lbl].test_and_set(src))
                         newDsts.set(dst);
             }
         }
#else
        // sub-cubic
        newDsts.intersectWithComplement(dstSet, succMap[src][lbl]);

        if ((succMap[src][lbl] |= newDsts))
        {
            for (NodeID dst : newDsts)
                predMap[dst][lbl].set(src);
        }
#endif
        return newDsts;
    }

    inline NodeBS checkAndAddEdges(const NodeBS& srcSet, const NodeID dst, const Label lbl)
    {
        NodeBS newSrcs;
#ifdef CUBIC
        // cubic
        if (CFLOpt::cubic()) {
            for (const NodeID src: srcSet)
                if (succMap[src][lbl].test_and_set(dst))
                    newSrcs.set(src);
            predMap[dst][lbl] |= newSrcs;
        } else {
            if ((predMap[dst][lbl] |= srcSet)) {
                for (const NodeID src: srcSet)
                    if (succMap[src][lbl].test_and_set(dst))
                        newSrcs.set(src);
            }
        }
#else
        // sub-cubic
        newSrcs.intersectWithComplement(srcSet, predMap[dst][lbl]);

        if ((predMap[dst][lbl] |= newSrcs))
        {
            for (NodeID src : newSrcs)
                succMap[src][lbl].set(dst);
        }
#endif
        return newSrcs;
    }

    inline NodeBS checkAndAddDsts(const NodeID src, const NodeBS& dstSet, const Label lbl)
    {
        NodeBS newDsts;
        newDsts.intersectWithComplement(dstSet, succMap[src][lbl]);
        succMap[src][lbl] |= newDsts;
        return newDsts;
    }

    inline NodeBS checkAndAddSrcs(const NodeBS& srcSet, const NodeID dst, const Label lbl)
    {
        NodeBS newSrcs;
        newSrcs.intersectWithComplement(srcSet, predMap[dst][lbl]);
        predMap[dst][lbl] |= newSrcs;
        return newSrcs;
    }

    inline bool checkAndAddDst(const NodeID src, const NodeID dst, const Label lbl)
    {
        return succMap[src][lbl].test_and_set(dst);
    }

    inline bool checkAndAddSrc(const NodeID src, const NodeID dst, const Label lbl)
    {
        return predMap[dst][lbl].test_and_set(src);
    }

    inline void addDst(const NodeID src, const NodeID dst, const Label lbl)
    {
        succMap[src][lbl].set(dst);
    }

    inline void addSrc(const NodeID src, const NodeID dst, const Label lbl)
    {
        predMap[dst][lbl].set(src);
    }

    inline bool hasEdge(const NodeID src, const NodeID dst, const Label lbl)
    {
        auto it1 = succMap.find(src);
        if (it1 == succMap.end())
            return false;

        auto it2 = it1->second.find(lbl);
        if (it2 == it1->second.end())
            return false;

        return it2->second.test(dst);
    }

    /* This is a dataset version, to be modified to a cflData version */
    inline void clearEdges(const NodeID key)
    {
        succMap[key].clear();
        predMap[key].clear();
    }
    //@}

    u32_t getLabelDegree() {
        u32_t degree = 0;
        for (auto& it1 : getSuccMap()) {
            degree += it1.second.size();
        }
        for (auto& it1 : getPredMap()){
            degree += it1.second.size();
        }
        return degree;
    }

    u32_t getGraphDegree() {
        return getEdgeCnt(true) + getEdgeCnt(false);
    }

    u32_t getEdgeCnt(bool succ = false) {
        u32_t cnt = 0;
        for (auto& it : succ? succMap:predMap) {
            for (auto& it1 : it.second) {
                cnt += it1.second.count();
            }
        }
        return cnt;
    }
};

/*!
 * Hybrid graph representation for transitive relations
 */
class HybridData
{
public:
    struct TreeNode
    {
        NodeID id;
        std::unordered_set<TreeNode*> children;

        TreeNode(NodeID nId) : id(nId)
        {}

        inline bool operator==(const TreeNode& rhs) const
        { return id == rhs.id; }

        inline bool operator<(const TreeNode& rhs) const
        { return id < rhs.id; }
    };

    u32_t checks;

    Map<NodeID, std::unordered_map<NodeID, TreeNode*>> indMap;   // indMap[v][u] points to node v in tree(u)

protected:
    std::unordered_map<NodeID, NodeBS> newEdgeMap;

public:
    HybridData() : checks(0)
    {}

    ~HybridData()
    {
        for (auto iter1 : indMap)
        {
            for (auto iter2 : iter1.second)
                delete iter2.second;
        }
    }

    inline bool hasInd(NodeID src, NodeID dst)
    {
        checks++;
        auto it = indMap.find(dst);
        if (it == indMap.end())
            return false;
        return (it->second.find(src) != it->second.end());
    }

    /// Add a node dst to tree(src)
    inline TreeNode* addInd(NodeID src, NodeID dst)
    {
        checks++;
        auto resIns = indMap[dst].insert(std::make_pair(src, new TreeNode(dst)));
        if (resIns.second)
            return resIns.first->second;
        return nullptr;
    }

    /// Get the node dst in tree(src)
    inline TreeNode* getNode(NodeID src, NodeID dst)
    { return indMap[dst][src]; }

    /// add v into desc(x) as a child of u
    inline void insertTreeEdge(TreeNode* u, TreeNode* v)
    { u->children.insert(v); }

    std::unordered_map<NodeID, NodeBS>& addArc(NodeID src, NodeID dst)
    {
        newEdgeMap.clear();

        if (!hasInd(src, dst))
        {
            for (auto iter : indMap[src])
                meld(iter.first, getNode(iter.first, src), getNode(dst, dst));
        }

        return newEdgeMap;
    }

    void meld(NodeID x, TreeNode* uNode, TreeNode* vNode)
    {
        TreeNode* newVNode = addInd(x, vNode->id);
        if (!newVNode)
            return;

        insertTreeEdge(uNode, newVNode);
        newEdgeMap[x].set(vNode->id);

        for (TreeNode* vChild : vNode->children)
            meld(x, newVNode, vChild);
    }
};

}   // end namespace SVF

#endif
