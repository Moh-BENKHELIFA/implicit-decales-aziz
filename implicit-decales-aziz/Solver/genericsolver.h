#ifndef GENERICSOLVER_H
#define GENERICSOLVER_H
#include <pse.h>
#include <vector>
#include "../Decale/DecaleScalarField2D.h"
#include "../Deformer2D/Deformer2D.h"
#include <QString>
#include <map>


class GenericSolver
{
public:
    GenericSolver();

    //functions
    void init();
    void solve();


protected:
    virtual void internal_preupdate_assignconstraints() = 0;
    virtual void internal_preupdate_constraintrelationships(size_t nRelshsps, std::vector< std::array<pse_ppoint_id_t, 2> > pppairs) = 0;
    virtual void internal_preupdate_assignnpoints() = 0;
    virtual void internal_preupdate_mapping(std::vector<pse_ppoint_id_t> ppids) = 0;
    virtual void internal_preupdate_setHandlers() = 0;



    //initialization of pse_library
    struct pse_device_params_t devp = PSE_DEVICE_PARAMS_NULL;

    //init
    struct pse_device_t* dev = NULL;
    struct pse_cpspace_params_t cpsp = PSE_CPSPACE_PARAMS_NULL;
    struct pse_cpspace_t* cps = NULL;
    struct pse_cpspace_exploration_ctxt_t* ctxt = NULL;
    struct pse_pspace_params_t psp = PSE_PSPACE_PARAMS_NULL;
    struct pse_pspace_point_attrib_component_t r2_coords_comp[2] = {
      PSE_TYPE_REAL, PSE_TYPE_REAL,
    };
    struct pse_pspace_point_attrib_component_t lockstatus_comp[1] = {
      PSE_TYPE_BOOL_8
    };
    pse_clt_pspace_uid_t r2_uid = 376847;
    std::vector<pse_relshp_cost_func_params_t> fp;

    std::vector<pse_relshp_cost_func_id_t> fids;
    void* gamut_config[1] = { this };

    struct pse_cpspace_values_data_t coords_data = PSE_CPSPACE_VALUES_DATA_NULL;
    struct pse_cpspace_values_t* coords = NULL;



    //update
    struct pse_cpspace_exploration_ctxt_params_t ecp = PSE_CPSPACE_EXPLORATION_CTXT_PARAMS_NULL;
    size_t nPoints;
    std::vector<pse_ppoint_id_t> ppids;

    //solve
    struct pse_cpspace_exploration_samples_t smpls = PSE_CPSPACE_EXPLORATION_SAMPLES_NULL;



};

#endif // GENERICSOLVER_H
