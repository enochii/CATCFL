//
// Created by kisslune on 6/5/23.
//

#ifndef POCR_SVF_CFLBASICUTILS_H
#define POCR_SVF_CFLBASICUTILS_H

#include <SVFIR/SVFType.h>


namespace SVF
{
    /// basic types for CFL-reachability
    typedef unsigned CFGSymbTy;
    typedef std::pair<CFGSymbTy, unsigned> Label;
    typedef std::unordered_set<NodeID> HashSet;
    typedef std::vector<NodeID> Vector;

    /// basic methods for CFL-reachability
    void processArgs(int argc, char** argv, int& arg_num, char** arg_vec, std::vector<std::string>& inFileVec);
    //std::vector<std::string> split(std::string str, char s);
    std::string strip(std::string& str);

    template <class K, class V>
    u32_t getSize(std::map<K, V>& mp) {
        u32_t sz = 0;
        for (auto& it: mp) {
            sz += getSize(it.second);
        }
        return sz;
    }

    template <class K, class V>
    u32_t getSize(std::unordered_map<K, V>& mp) {
        u32_t sz = 0;
        for (auto& it: mp) {
            sz += getSize(it.second);
        }
        return sz;
    }

    template <unsigned int N>
    inline u32_t getSize(const SVF::SparseBitVector<N>& bs) {
        return bs.count();
    }

    template <unsigned int N>
    inline void setBit(SVF::SparseBitVector<N>& bs, SVF::NodeID id) {
        bs.set(id);
    }

    template <unsigned int N>
    inline bool trySetBit(SparseBitVector<N>& bs, SVF::NodeID id) {
        return bs.test_and_set(id);
    }

    template <unsigned int N>
    inline bool testBit(const SVF::SparseBitVector<N>& bs, SVF::NodeID id) {
        return bs.test(id);
    }

    template <unsigned int N>
    inline bool addDelta(SVF::SparseBitVector<N>& lhs,
                         const SVF::SparseBitVector<N>& rhs) {
        return lhs |= rhs;
    }

    template <unsigned int N>
    inline void setMinusInPlace(SVF::SparseBitVector<N>& lhs,
                        const SVF::SparseBitVector<N>& rhs) {
        lhs.intersectWithComplement(rhs);
    }

    inline void setBit(std::vector<SVF::NodeID>& bs, SVF::NodeID id) {
        bs.emplace_back(id);
    }

    template <class SetType>
    inline void setBit(SetType& bs, SVF::NodeID id) {
        bs.insert(id);
    }

    template <class SetType>
    inline bool testBit(const SetType& bs, SVF::NodeID id) {
        return bs.find(id) != bs.end();
    }

    template <class SetType>
    inline bool trySetBit(SetType& bs, SVF::NodeID id) {
        return bs.insert(id).second;
    }

    template <class SetType>
    inline void addDelta(SetType& lhs, const SetType& rhs) {
        for (auto n : rhs)
            setBit(lhs, n);
    }
}

#endif //POCR_SVF_CFLBASICUTILS_H
