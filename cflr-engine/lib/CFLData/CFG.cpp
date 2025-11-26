//
// Created by kisslune on 7/5/22.
//

#include "CFLSolver/CFLBase.h"
#include "CFLData/CFG.h"
#include <iostream>
#include <CFLData/CFLGraph.h>

using namespace SVF;
using namespace SVFUtil;


void CFG::parseGrammar(std::string fname)
{
    readGrammarFile(fname);
    detectTransitiveSymbol();
//    printCFGStat();
}


void CFG::readGrammarFile(std::string fname)
{
    std::vector<std::string> linearPairs;
    std::vector<std::string> revPairs;

    std::ifstream gFile;
    gFile.open(fname, std::ios::in);
    if (!gFile.is_open())
    {
        std::cout << "error opening " << fname << std::endl;
        exit(0);
    }

    std::string line;
    while (getline(gFile, line))
    {
        line = strip(line);

        /// Switch line types
        //@{
        if (line == "Order:") {
            lineTy = Order;
        } else if (line == "Production:") {
            lineTy = Production;
            continue;
        } else if (line == "Insert:") {
            lineTy = Insert;
            continue;
        } else if (line == "Follow:") {
            lineTy = Follow;
            continue;
        } else if (line == "Count:") {
            lineTy = Count;
            continue;
        } else if (line == "Pred:") {
            lineTy = PredCond;
            continue;
        } else if (line == "Succ:") {
            lineTy = SuccCond;
            continue;
        } else if (line == "Pred*:") {
            lineTy = PredStarCond;
            continue;
        } else if (line == "Succ*:") {
            lineTy = SuccStarCond;
            continue;
        } else if (line == "Conn:") {
            lineTy = LinearCond;
            continue;
        } else if (line == "Rev:") {
            lineTy = RevCond;
            continue;
        }
        //@}
        if (lineTy == Order) {
            readUCFLSymbol(line, lineTy);
        } else if (lineTy == Production)
            readProduction(line);
        else if (lineTy == PredCond)
            readPrePostConditions(line, predCondition);
        else if (lineTy == SuccCond)
            readPrePostConditions(line, succCondition);
        else if (lineTy == LinearCond)
            linearPairs = SVFUtil::split(line, '\t');
        else if (lineTy == RevCond)
            revPairs = SVFUtil::split(line, '\t');
        else
            readUCFLSymbol(line, lineTy);
    }

    if (insertSymbols.empty()) {
        for (auto& it : intToSymbMap) {
            auto sym = it.first;
            insertSymbols.insert(sym);
        }
    } else {
        for (auto& it : intToSymbMap) {
            auto sym = it.first;
            if (!isNonTerminal(sym))
                insertSymbols.insert(sym);
        }
    }

    if (insertSymbols.size() + followSymbols.size() != intToSymbMap.size()) {
        std::cout << "The total symbols does not match." << std::endl;
        exit(0);
    }

    for (auto i = 0; i < linearPairs.size(); i += 3) {
        linearPairMap[getSymbolId(linearPairs[i])] = std::make_pair(getSymbolId(linearPairs[i+1]), getSymbolId(linearPairs[i+2]));
    }

    for (auto i = 0; i < revPairs.size(); i += 2) {
        auto rev = getSymbolId(revPairs[i]);
        auto ori = getSymbolId(revPairs[i + 1]);
        if (rev != ori) {
            pairedSymMap[ori] = rev;
            pairedSymMap[rev] = ori;

            barSyms.insert(rev);
        } else {
            symmSyms.insert(ori);
        }
    }

    gFile.close();

    indexProductions();
}


void CFG::readProduction(std::string& line)
{
    std::vector<std::string> vec = split(line, '\t');
    if (vec.empty())
        return;

    if (vec.size() == 1)
    {
        addSymbol(vec[0]);
        emptyRules.insert(getSymbolId(vec[0]));
    }
    else if (vec.size() == 2)
    {
        addSymbol(vec[0]);
        addSymbol(vec[1]);
        unaryRules[getSymbolId(vec[1])].insert(getSymbolId(vec[0]));
    }
    else if (vec.size() == 3)
    {
        addSymbol(vec[0]);
        addSymbol(vec[1]);
        addSymbol(vec[2]);
        binaryRules[std::make_pair(getSymbolId(vec[1]), getSymbolId(vec[2]))].insert(getSymbolId(vec[0]));
    }

    nonterminals.insert(getSymbolId(vec[0]));
}


void CFG::readUCFLSymbol(std::string& line, LineTy ty)
{
    Set<CFGSymbTy>* ucflSymbolSet = nullptr;
    if (ty == Insert)
        ucflSymbolSet = &insertSymbols;
    else if (ty == Follow)
        ucflSymbolSet = &followSymbols;
    else if (ty == Count)
        ucflSymbolSet = &countSymbols;
    else if (ty == SuccStarCond)
        ucflSymbolSet = &succStarSymbols;
    else if (ty == PredStarCond)
        ucflSymbolSet = &predStarSymbols;

    if (!ucflSymbolSet && ty != Order) {
        std::cout << "invalid symbol type" << std::endl;
        exit(0);
    }

    std::vector<std::string> vec = split(line, ',');
    for (auto& it : vec)
    {
        std::string symbStr = strip(it);
        addSymbol(symbStr);
        if (ucflSymbolSet) {
            ucflSymbolSet->insert(getSymbolId(symbStr));
        }
    }
}


void CFG::detectTransitiveSymbol()
{
    for (auto& rule : binaryRules)
    {
        for (auto lhs : rule.second)
            if (lhs == rule.first.first && lhs == rule.first.second)
                transitiveSymbols.insert(lhs);
    }
}


void CFG::addSymbol(std::string& s)
{
    if (hasSymbol(s))
        return;

    numOfSymbols++;
    symbToIntMap[s] = numOfSymbols;
    intToSymbMap[numOfSymbols] = s;
    /// check whether the label has an index
    if (s.find("_i") == s.size() - 2 && s.find("_i") != -1)
        variableSymbols.insert(numOfSymbols);
}


void CFG::printCFGStat()
{
    u32_t numOfVariantSymbols = variableSymbols.size();

    u32_t numOfRules = 0;

    numOfRules += emptyRules.size();
    for (auto& rule : unaryRules)
        numOfRules += rule.second.size();

    for (auto& rule : binaryRules)
        numOfRules += rule.second.size();

    std::cout << "#Symbol = " << numOfSymbols << ":\t";
    for (auto& it : intToSymbMap)
        std::cout << it.second << "->" << it.first << ", ";
    std::cout << std::endl;

    std::cout << "Insert:" << "\t\t";
    for (auto it : insertSymbols)
        std::cout << getSymbolString(it) << ", ";
    std::cout << std::endl;

    std::cout << "Follow:" << "\t\t";
    for (auto it : followSymbols)
        std::cout << getSymbolString(it) << ", ";
    std::cout << std::endl;

    std::cout << "Count:" << "\t\t";
    for (auto it : countSymbols)
        std::cout << getSymbolString(it) << ", ";
    std::cout << std::endl;

    std::cout << "Trans:" << "\t\t";
    for (auto it : transitiveSymbols)
        std::cout << getSymbolString(it) << ", ";
    std::cout << std::endl;

    std::cout << "#VariantSymbol = " << numOfVariantSymbols << std::endl;
    std::cout << "#Rule = " << numOfRules << std::endl;

//    printProdIndexes();

    std::cout << std::endl;
}

void CFG::indexProductions() {
    for (auto& it: unaryRules) {
        auto rhs = it.first;
        for (auto lhs: it.second) {
            unaryIndexes[rhs].emplace_back(lhs, isaVariantSymbol(lhs)? EXISTING:ZERO);

            SymIndex si = ZERO;
            if (isaVariantSymbol(rhs)) {
                if (isaVariantSymbol(lhs)) si = EXISTING;
                else si = UNKNOWN;
            }
            unaryRhs[lhs].emplace_back(rhs, si);
        }
    }
    for (auto& it: binaryRules) {
        auto r1 = it.first.first, r2 = it.first.second;
        bool isR1Var = isaVariantSymbol(r1), isR2Var = isaVariantSymbol(r2);
        for (auto lhs: it.second) {
            bool isLhsVar = isaVariantSymbol(lhs);
            if (isR1Var && isR2Var) {
                if (!isLhsVar) {
                    xzIndexes[r1].emplace_back(IndexedSym(lhs, ZERO), IndexedSym(r2, EXISTING));
                    xyIndexes[r2].emplace_back(IndexedSym(lhs, ZERO), IndexedSym(r1, EXISTING));

                    binaryRhs[lhs].emplace_back(IndexedSym(r1, UNKNOWN), IndexedSym(r2, UNKNOWN));
                }
            } else if (isR1Var) {
                xzIndexes[r1].emplace_back(IndexedSym(lhs, isLhsVar? EXISTING:ZERO), IndexedSym(r2, ZERO));
                xyIndexes[r2].emplace_back(IndexedSym(lhs, isLhsVar? UNKNOWN:ZERO), IndexedSym(r1, UNKNOWN));

                binaryRhs[lhs].emplace_back(IndexedSym(r1, isLhsVar? EXISTING:UNKNOWN), IndexedSym(r2, ZERO));
            } else if (isR2Var) {
                xzIndexes[r1].emplace_back(IndexedSym(lhs, isLhsVar? UNKNOWN:ZERO), IndexedSym(r2, UNKNOWN));
                xyIndexes[r2].emplace_back(IndexedSym(lhs, isLhsVar? EXISTING:ZERO), IndexedSym(r1, ZERO));

                binaryRhs[lhs].emplace_back(IndexedSym(r1, ZERO), IndexedSym(r2, isLhsVar? EXISTING:UNKNOWN));
            } else {
                if (!isLhsVar) {
                    xzIndexes[r1].emplace_back(IndexedSym(lhs, ZERO), IndexedSym(r2, ZERO));
                    xyIndexes[r2].emplace_back(IndexedSym(lhs, ZERO), IndexedSym(r1, ZERO));

                    binaryRhs[lhs].emplace_back(IndexedSym(r1, ZERO), IndexedSym(r2, ZERO));
                }
            }
        }
    }
}

void CFG::printProdIndexes() {
    auto dumpIndexSym = [&](const IndexedSym& sym) {
        std::cout << getSymbolString(sym.sym) << "(" << sym.idxTy << ")";
    };

    auto dumpPartProd = [&](const PartBinProd& prod) {
        std::cout << "[";
        dumpIndexSym(prod.lhs);
        std::cout << " ";
        dumpIndexSym(prod.other);
        std::cout << "]";
    };

    std::cout << "----unary:\n";
    for (auto& it: unaryIndexes) {
        std::cout << getSymbolString(it.first) << "\t|->\t{";
        for (auto& lhsSym: it.second) {
            dumpIndexSym(lhsSym);
            std::cout << " ";
        }
        std::cout << "}\n";
    }

    std::cout << "----xz:\n";
    for (auto& it: xzIndexes) {
        std::cout << getSymbolString(it.first) << "\t|->\t{";
        for (auto& partProd: it.second) {
            dumpPartProd(partProd);
            std::cout << " ";
        }
        std::cout << "}\n";
    }

    std::cout << "----xy:\n";
    for (auto& it: xyIndexes) {
        std::cout << getSymbolString(it.first) << "\t|->\t{";
        for (auto& partProd: it.second) {
            dumpPartProd(partProd);
            std::cout << " ";
        }
        std::cout << "}\n";
    }
}

void CFG::readPrePostConditions(std::string& line, Map<CFGSymbTy, Set<CFGSymbTy>>& cond) {
    std::vector<std::string> vec = split(line, '\t');
    if (vec.empty())
        return;
    auto keyStr = vec[0];
    auto keyID = getSymbolId(keyStr);
    for (u32_t i = 1; i < vec.size(); ++i)
    {
        auto valID = getSymbolId(vec[i]);
        cond[keyID].insert(valID);
    }
}

static void contextHelper(const Map<CFGSymbTy, Set<CFGSymbTy>>& condition,
                 const Map<CFGSymbTy, SetTy>& nodes,
                 Map<CFGSymbTy, SetTy>& taintedNodes) {
    for (auto& it : condition) {
        auto keyID = it.first;
        for (auto valID : it.second) {
            auto vit = nodes.find(valID);
            if (vit != nodes.end())
                addDelta(taintedNodes[keyID], vit->second);
        }
    }
};

void CFG::computeContext(CFLGraph* graph) {
    for (auto edge: graph->getCFLEdges()) {
        NodeID src = edge->getSrcID(), dst = edge->getDstID();
        u32_t sym = edge->getEdgeKind();

        setBit(srcs[sym], src);
        setBit(dsts[sym], dst);
    }

    contextHelper(predCondition, srcs, taintedDsts);
    contextHelper(succCondition, dsts, taintedSrcs);

    // succ/pred-free
    for (auto& it: intToSymbMap) {
        auto sym = it.first;
        if (!(isPredStarSym(sym) || hasTaintedDsts(sym)))
            predFreeSymbols.insert(sym);
        if (!(isSuccStarSym(sym) || hasTaintedSrcs(sym)))
            succFreeStmbols.insert(sym);
    }

    u32_t sz = intToSymbMap.size();
    for (u32_t i = 0; i < sz; ++ i) {
        CFGSymbTy s = i+1;
        symbols.emplace_back(getSymbolString(s), isPredFreeSym(s), isSuccFreeSym(s),
                             isPredStarSym(s), isSuccStarSym(s), isaVariantSymbol(s),
                             isCountSymbol(s), hasUnaryProd(s), isInsertSymbol(s),
                             isFollowSymbol(s), isTerminal(s));
        auto& ref = symbols.back();

        if (isPairedSym(s))
            ref.setPairedSym(getMatchSymPair(s));

        if (hasTaintedSrcs(s)) {
            ref.taintedSrcs = taintedSrcs.at(s);
        }

        if (hasTaintedDsts(s)) {
            ref.taintedDsts = taintedDsts.at(s);
        }
    }
}


bool CFG::isPredStarSym(CFGSymbTy sym) const {
    return predStarSymbols.find(sym) != predStarSymbols.end();
}

bool CFG::isSuccStarSym(CFGSymbTy sym) const {
    return succStarSymbols.find(sym) != succStarSymbols.end();
}

bool CFG::isPredFreeSym(CFGSymbTy sym) const {
    return predFreeSymbols.find(sym) != predFreeSymbols.end();
}

bool CFG::isSuccFreeSym(CFGSymbTy sym) const {
    return succFreeStmbols.find(sym) != succFreeStmbols.end();
}

bool CFG::hasTaintedSrcs(CFGSymbTy sym) const {
    return taintedSrcs.find(sym) != taintedSrcs.end();
}

bool CFG::hasTaintedDsts(CFGSymbTy sym) const {
    return taintedDsts.find(sym) != taintedDsts.end();
}

CFGSymbTy CFG::getPairedSym(CFGSymbTy sym) {
    auto it = pairedSymMap.find(sym);
    if (it != pairedSymMap.end())
        return it->second;
    return 0; // fault symbol
}
