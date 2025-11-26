/*
 // Author: Kisslune
 */

#include "CFLSolver/CFLSolver.h"
#include "CFLSolver/CATCFL.h"

using namespace SVF;


static Option<bool> Default_CFL("std", "Standard CFL-reachability analysis", false);
static Option<bool> Pocr_CFL("pocr", "POCR CFL-reachability analysis", false);
static Option<bool> Focr_CFL("focr", "Uni-directional CFL-reachability analysis", false);
static Option<bool> SkewedStd_CFL("skewedstd", "Skewed CFL-reachability analysis based on standard solver", false);
static Option<bool> SkewedOCR_CFL("skewed", "Skewed CFL-reachability analysis based on POCR solver", false);

static Option<bool> CAT_CFL("cat", "CAT", false);

int main(int argc, char** argv)
{
    int arg_num = 0;
    char** arg_vec = new char* [argc];
    std::vector<std::string> moduleNameVec;
    std::vector<std::string> inFileVec;
    processArgs(argc, argv, arg_num, arg_vec, inFileVec);
    OptionBase::parseOptions(arg_num, arg_vec, "CFL-reachability analysis\n", "[options] <input>");

    StdCFL* cfl;
    string cfg = argv[1], graph = argv[2];

    if (Default_CFL())
    {
        cfl = new StdCFL(cfg, graph);
        cfl->analyze();
    }
    else if (Pocr_CFL())
    {
        cfl = new PocrCFL(cfg, graph);
        cfl->analyze();
    }
    else if (Focr_CFL())
    {
        cfl = new FocrCFL(cfg, graph);
        cfl->analyze();
    }
    else if (SkewedStd_CFL())
    {
        CFLOpt::ucfl.setValue(true);
        cfl = new SkewedCFL(cfg, graph);
        cfl->analyze();
    }
    else if (SkewedOCR_CFL())
    {
        CFLOpt::ucfl.setValue(true);
        cfl = new SkewedOcrCFL(cfg, graph);
        cfl->analyze();
    }
    else if (CAT_CFL())
    {
        cfl = new CATmCFL(cfg, graph);
        cfl->analyze();
    }
    else
    {
        std::cout << "No analysis specified, exiting..." << std::endl;
    }

    return 0;
}