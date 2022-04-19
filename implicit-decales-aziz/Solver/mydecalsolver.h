#ifndef MYDECALSOLVER_H
#define MYDECALSOLVER_H

#include "genericsolver.h"
#include "../Decale/DecaleScalarField2D.h"
#include "../Deformer2D/Deformer2D.h"
#include "../Gamut/GamutField2D.h"
#include <map>

class MyDecalSolver : public GenericSolver
{
public:
    MyDecalSolver();
    void setDecales (const VectorOfDecaleFields &decales);
    void setGamut(GamutField2D *gamut);

    void internal_preupdate_assignconstraints() override;
    void internal_preupdate_constraintrelationships(size_t nRelshsps, std::vector< std::array<pse_ppoint_id_t, 2> > pppairs) override;
    void internal_preupdate_assignnpoints() override;
    void internal_preupdate_mapping(std::vector<pse_ppoint_id_t> ppids) override;
    void internal_preupdate_setHandlers() override;


    static enum pse_res_t getAttribs(void* ctxt,
                              const enum pse_point_attrib_t attrib,
                              const enum pse_type_t as_type,
                              const size_t count,
                              const pse_ppoint_id_t* values_idx,
                              void* attrib_values);
    static enum pse_res_t setAttribs(void* ctxt,
                              const enum pse_point_attrib_t attrib,
                              const enum pse_type_t as_type,
                              const size_t count,
                              const pse_ppoint_id_t* values_idx,
                              const void* attrib_values);


    VectorOfDecaleFields decales;
    static std::map<pse_ppoint_id_t, DecaleScalarField2D*> mydecalsmap;

    static GamutField2D *mygamut;


protected:

    int gamut_cosntr_ID = -1;
    int min_dist_cosntr_ID = -1;
    int max_dist_cosntr_ID = -1;


};

#endif // MYDECALSOLVER_H
