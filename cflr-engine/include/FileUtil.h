//
// Created by enochii on 2024/5/23.
//

#ifndef POCR_SVF_FILEUTIL_H
#define POCR_SVF_FILEUTIL_H

#include <SVFIR/SVFType.h>
#include <fstream>

using namespace SVF;

inline std::ifstream openRFile(const std::string& fname) {
    std::ifstream inFile(fname, std::ios::in);
    if (!inFile.is_open()) {
//        std::cout << "error opening " << fname << std::endl;
        return {};
    }
    return std::move(inFile);
}

inline NodeBS readSetFromFile(const std::string& fname) {
    NodeBS st;
    auto f = openRFile(fname);
    std::string line;
    while (getline(f, line)) {
        if (line.empty()) continue;

        NodeID src = stoi(line);
        st.set(src);
    }
    return st;
}

inline std::string replaceSuffix(const std::string& origin, const std::string& newSuffix) {
    auto dotPos = origin.find_last_of('.');
    if (dotPos == std::string::npos) {
        std::cout << "the given file name does not have a dot" << std::endl;
        exit(0);
    }

    return origin.substr(0, dotPos+1) + newSuffix;
}



#endif //POCR_SVF_FILEUTIL_H
