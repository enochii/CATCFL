//===- Options.cpp -- Command line options ------------------------//

#include "CFLSolver/CFLOpt.h"

namespace SVF
{
const Option<u32_t> CFLOpt::timeOutInHour(
        "toh",
        "time out in hours",
        6
);

const Option<bool> CFLOpt::PStat(
        "pstat",
        "Print statistics",
        true
);

const Option<bool> CFLOpt::LStat(
        "lstat",
        "light cfl statistics",
        false
);


const Option<bool> CFLOpt::solveCFL(
        "solve",
        "Perform dynamic CFL-reachability solving",
        true
);

const Option<std::string> CFLOpt::outGraphFName(
        "write-graph",
        "Write the graph into file",
        ""
);

const Option<std::string> CFLOpt::sfx(
        "sfx",
        "file suffix of simplified graphs",
        "" //
);

Option<bool> CFLOpt::ucfl(
        "ucfl",
        "Enable uni-directional CFL-reachability summarization scheme",
        false
);

Option<bool> CFLOpt::tailor(
        "prune",
        "prune useless edges",
        false
);


const Option<bool> CFLOpt::cubic(
        "cub",
        "truly cubic",
        false
);

Option<bool> CFLOpt::verify(
        "verify",
        "verify the cfl-reachability solution",
        true
);

Option<bool> CFLOpt::dumpG(
        "dumpg",
        "dump the summary edges",
        false
);

Option<bool> CFLOpt::ctxStat(
        "ctx",
        "context analysis stat",
        false
);

Option<bool> CFLOpt::srcSnkCheck(
        "sscheck",
        "src snk check",
        false
);


const Option<std::string> CFLOpt::sPairsFName(
        "write-spairs",
        "Write S pairs into specified file",
        ""
);

const Option<bool> CFLOpt::ecgSCC(
        "ecgscc",
        "Simplify cycles in ECG",
        false
);

} // namespace SVF.
