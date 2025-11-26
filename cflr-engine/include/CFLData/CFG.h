//
// Created by kisslune on 7/5/22.
//

#ifndef POCR_SVF_CFG_H
#define POCR_SVF_CFG_H

#include "SVF-LLVM/BasicTypes.h"
#include "BasicUtils.h"
#include "CFGSym.h"

namespace SVF
{
    class CFLGraph;
    /*!
 * Context-free grammar container
 */

struct SymbolTuple {
    CFGSymbTy rightTerminal, lhsSymbol;
    SymbolTuple(CFGSymbTy rightTerminal, CFGSymbTy lhsSymbol):
        rightTerminal(rightTerminal), lhsSymbol(lhsSymbol) {}
};

class CFG
{
public:
    enum LineTy
    {
        Production,     // production rule
        Insert,         // insert non-terminals
        Follow,         // follow non-terminals
        Count,          // count non-terminals
        Order,          // ordering

        PredCond,
        SuccCond,
        PredStarCond,
        SuccStarCond,

        LinearCond,
        RevCond,
    };

    LineTy lineTy;      // used to track the type of the current line

    /// number of symbols
    CFGSymbTy numOfSymbols;

    /// mapping string symbol to int
    Map<std::string, CFGSymbTy> symbToIntMap;
    Map<CFGSymbTy, std::string> intToSymbMap;
    /// the IDs of symbols with variant subscript
    Set<CFGSymbTy> variableSymbols, transitiveSymbols, insertSymbols, followSymbols,
                    countSymbols, nonterminals;

    Set<CFGSymbTy> succStarSymbols, predStarSymbols, succFreeStmbols, predFreeSymbols;
    Map<CFGSymbTy, Set<CFGSymbTy>> succCondition, predCondition;
    Map<CFGSymbTy, SetTy> taintedSrcs, taintedDsts, srcs, dsts;
    const NodeBS emptyData;

    /// Sets of rules
    Set<CFGSymbTy> emptyRules;                                          // X ::= epsilon
    Map<CFGSymbTy, Set<CFGSymbTy>> unaryRules;                          // X ::= Y
    Map<std::pair<CFGSymbTy, CFGSymbTy>, Set<CFGSymbTy>> binaryRules;   // X ::= Y Z

    const Set<CFGSymbTy> emptySet;

    std::vector<CFGSymbol> symbols;
    std::map<CFGSymbTy, std::pair<CFGSymbTy, CFGSymbTy>> linearPairMap;
    std::map<CFGSymbTy, CFGSymbTy> pairedSymMap;
    Set<CFGSymbTy> barSyms, symmSyms;

    enum SymIndex {
        UNKNOWN = -1,
        ZERO,
        EXISTING,
    };
    /*
     * X = Y Z. let Z be the existing symbol (with an index), X and Y also possess a corresponding index,
     *  indicated by `SymbolIndex`:
     * - ZERO: 0;
     * - UNKNOWN: let the processing edge be (Z, u, v), for each (Y, i)-predecessor of Node u, the index is `i`.
     * - EXISTING: using the existing index, i.e., Z's index
     */
    // determining the index for unary productions is similar
//    using IndexedSym = std::pair<CFGSymbTy, SymIndex>;
    struct IndexedSym {
        CFGSymbTy sym;
        SymIndex idxTy;

        IndexedSym(CFGSymbTy sym, SymIndex idxTy): sym(sym), idxTy(idxTy) {}

        bool isUnknown() const {
            return idxTy == UNKNOWN;
        }

        bool isExisting() const {
            return idxTy == EXISTING;
        }

        bool isZero() const {
            return idxTy == ZERO;
        }
    };
    struct PartBinProd {
        IndexedSym lhs, other;

        PartBinProd(IndexedSym lhs, IndexedSym other): lhs(lhs), other(other) {}
    };
    Map<CFGSymbTy, std::vector<IndexedSym>> unaryIndexes;
    Map<CFGSymbTy, std::vector<PartBinProd>> xzIndexes, xyIndexes;

    struct BinProdRhs {
        IndexedSym r1, r2;

        BinProdRhs(IndexedSym r1, IndexedSym r2): r1(r1), r2(r2) {}
    };
    Map<CFGSymbTy, std::vector<IndexedSym>> unaryRhs;
    Map<CFGSymbTy, std::vector<BinProdRhs>> binaryRhs;

    void indexProductions();
    void printProdIndexes();

public:
    CFG() : numOfSymbols(0),
            lineTy(Production)
    {}

    bool hasSymbol(const std::string& s)
    { return symbToIntMap.find(s) != symbToIntMap.end(); }

    void addSymbol(std::string& s);

    CFGSymbTy getSymbolId(const std::string& s)
    {
        assert(hasSymbol(s) && "Attempting to access a non-existing symbol!!");
        return symbToIntMap.at(s);
    }

    std::string getSymbolString(CFGSymbTy c)
    { return intToSymbMap.at(c); }

    bool isaVariantSymbol(CFGSymbTy c)
    { return variableSymbols.find(c) != variableSymbols.end(); }

    const Set<CFGSymbTy>& getLhs(CFGSymbTy rhs) const
    {
        auto it = unaryRules.find(rhs);
        if (it == unaryRules.end())
            return emptySet;
        return it->second;
    }

    const Set<CFGSymbTy>& getLhs(std::pair<CFGSymbTy, CFGSymbTy> rhs) const
    {
        auto it = binaryRules.find(rhs);
        if (it == binaryRules.end())
            return emptySet;
        return it->second;
    }

    Set<CFGSymbTy>& getEmptyRules()
    { return emptyRules; }

    bool isTransitive(CFGSymbTy s)
    { return transitiveSymbols.find(s) != transitiveSymbols.end(); }

    bool isInsertSymbol(CFGSymbTy s)
    { return insertSymbols.find(s) != insertSymbols.end(); }

    bool isFollowSymbol(CFGSymbTy s)
    { return followSymbols.find(s) != followSymbols.end(); }

    bool isCountSymbol(CFGSymbTy s)
    { return countSymbols.find(s) != countSymbols.end(); }

    CFGSymbTy getStartSymbol()
    { return *countSymbols.begin(); }

    bool isStartNullable() {
        return getEmptyRules().count(getStartSymbol()) != 0;
    }

    bool isTerminal(CFGSymbTy sym)
    { return nonterminals.find(sym) == nonterminals.end(); }

    bool isNonTerminal(CFGSymbTy sym)
    { return nonterminals.find(sym) != nonterminals.end(); }

    bool hasUnaryProd(CFGSymbTy sym) {
        return unaryRules.find(sym) != unaryRules.end();
    }

    bool isPairedSym(CFGSymbTy sym) {
        return linearPairMap.find(sym) != linearPairMap.end();
    }

    std::pair<CFGSymbTy, CFGSymbTy>& getMatchSymPair(CFGSymbTy sym) {
        return linearPairMap.at(sym);
    }

    CFGSymbTy getPairedSym(CFGSymbTy sym);

    bool isBarSym(CFGSymbTy sym) { return barSyms.find(sym) != barSyms.end(); }

    bool isSymmSym(CFGSymbTy sym) { return symmSyms.find(sym) != symmSyms.end(); }

    const CFGSymbol* getSymbol(CFGSymbTy sym) const { return &symbols.at(sym - 1); }

    void parseGrammar(std::string fname);
    void readGrammarFile(std::string fname);
    void readProduction(std::string& line);
    void readPrePostConditions(std::string& line, Map<CFGSymbTy, Set<CFGSymbTy>>& cond);
    void readUCFLSymbol(std::string& line, LineTy ty);
    void detectTransitiveSymbol();
    void printCFGStat();

    void computeContext(CFLGraph* graph);

    bool isPredStarSym(CFGSymbTy sym) const;
    bool isSuccStarSym(CFGSymbTy sym) const;
    bool isPredFreeSym(CFGSymbTy sym) const;
    bool isSuccFreeSym(CFGSymbTy sym) const;
    bool hasTaintedSrcs(CFGSymbTy sym) const;
    bool hasTaintedDsts(CFGSymbTy sym) const;
};

}

#endif //POCR_SVF_CFG_H
