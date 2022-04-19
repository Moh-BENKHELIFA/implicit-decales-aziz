#include "genericsolver.h"
#include <vector>
#include <map>
#include <iostream>

GenericSolver::GenericSolver()
{
    std::cout<<"generic solver: constructor"<<std::endl;
    /* Create the device that will load the driver */
#ifdef __gnu_linux__
    devp.backend_drv_filepath = PSE_LIB_NAME("pse-drv-eigen-ref");
#else
    devp.backend_drv_filepath = PSE_LIB_NAME("libpse-drv-eigen-ref");
#endif
    //devp.logger = &PSE_LOGGER_STDOUT;
    PSE_CALL(pseDeviceCreate(&devp, &dev));

    //this->init(dev);
}

void GenericSolver::init()
{
    /* Create the CPS and fill it */
   //this->dev = dev;
   std::cout<<"solver init"<<std::endl;

   //destroy curent state of cps
   if(ctxt!=nullptr){
       PSE_CALL(pseConstrainedParameterSpaceExplorationContextRefSub(ctxt));
   }
   if( cps != nullptr ) {
       PSE_CALL(pseConstrainedParameterSpaceRefSub(cps));
   }
   PSE_CALL(pseConstrainedParameterSpaceCreate(dev, &cpsp, &cps));
   psp.ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components_count = 2;
   psp.ppoint_params.attribs[PSE_POINT_ATTRIB_COORDINATES].components = r2_coords_comp;
   psp.ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components_count = 1;
   psp.ppoint_params.attribs[PSE_POINT_ATTRIB_LOCK_STATUS].components = lockstatus_comp;
   PSE_CALL(pseConstrainedParameterSpaceParameterSpacesDeclare(cps, 1, &r2_uid, &psp));

   fp.clear();
   fids.clear();

   //cost functors: - add constraints
   internal_preupdate_assignconstraints();


   PSE_CALL(pseConstrainedParameterSpaceRelationshipCostFunctorsRegister(cps, fp.size(), fp.data(), fids.data()));

   coords_data.pspace = r2_uid;
   coords_data.storage = PSE_CPSPACE_VALUES_STORAGE_ACCESSORS_GLOBAL;

   internal_preupdate_setHandlers();

   coords_data.as.global.accessors.ctxt = this;//send current state to the context
   PSE_CALL(pseConstrainedParameterSpaceValuesCreate(cps, &coords_data, &coords));

   internal_preupdate_assignnpoints();//size_t nPoints = decalDataArray.size();
   size_t nRelshsps = (nPoints*nPoints-nPoints) / 2;// according to adjacenecy matrix

   //starting over the state of the cps
   std::vector<pse_ppoint_params_t> ppp(nPoints, PSE_PPOINT_PARAMS_NULL);
   std::vector<pse_ppoint_id_t> ppids(nPoints, PSE_PPOINT_ID_INVALID_);

   std::vector< std::array<pse_ppoint_id_t, 2> > pppairs(nRelshsps);

   PSE_CALL(pseConstrainedParameterSpaceParametricPointsAdd(cps,nPoints,ppp.data(),ppids.data()));
   //mapping
   internal_preupdate_mapping(ppids);

   //1 to 1 relation
   //c - to change number of pairs 0 and 1
   //iterate over hald of adjacency matrix - undirected graph
   //making the pairs
   size_t c = 0;

   for(size_t i = 0; i <nPoints-1; i++){
       for(size_t j = i+1; j <nPoints; j++){
           pppairs[c][0] = ppids[i];
           pppairs[c][1] = ppids[j];
           c++;
       }

   }
   //making the relationships between points
   internal_preupdate_constraintrelationships(nRelshsps, pppairs);
   std::cout<<"nRelshsps:::  "<<nRelshsps<<std::endl;


   /* To call on the CPS when its structure change */
   ecp.pspace.explore_in = r2_uid;
   ecp.options.auto_df_epsilon = //0.00000001
                                   0.00001
                                 ;
   ecp.options.max_convergence_tries = //1000000
                                         100000
                                       ;
   ecp.options.until_convergence = true;
   PSE_CALL(pseConstrainedParameterSpaceExplorationContextCreate(cps, &ecp, &ctxt));
}

void GenericSolver::solve()
{
    //std::cout<<"solve"<<std::endl;
    smpls.values = coords;
    PSE_CALL(pseConstrainedParameterSpaceExplorationSolve(ctxt, &smpls));
    PSE_CALL(pseConstrainedParameterSpaceExplorationLastResultsRetreive(ctxt, coords, NULL));
}






