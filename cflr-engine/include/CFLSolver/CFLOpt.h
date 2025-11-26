//===- Options.h -- Command line options ------------------------//

#ifndef CFLOpt_H_
#define CFLOpt_H_

#include <Util/Options.h>

namespace SVF
{
/// Carries around command line options.
class CFLOpt
{
public:
    CFLOpt(void) = delete;

    static std::string getSrc() { return "src" + sfx(); }
    static std::string getSnk() { return "snk" + sfx(); }
    static u32_t getTimeOutInSec() { return timeOutInHour() * 3600; }

    /// CFL Options
    static const Option<u32_t> timeOutInHour;
    static const Option<std::string> sPairsFName, outGraphFName, sfx;
    static const Option<bool> PStat, LStat, solveCFL, graphStat, cubic, ecgSCC;

    static Option<bool> ucfl, tailor, verify, dumpG, ctxStat, srcSnkCheck;
};

}  // namespace SVF

#endif
