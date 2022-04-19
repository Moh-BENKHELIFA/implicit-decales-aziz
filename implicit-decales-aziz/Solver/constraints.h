#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H
#include "pse.h"
#include "../Decale/DecaleScalarField2D.h"
#include "../Deformer2D/Deformer2D.h"
#include "mydecalsolver.h"
#include "../GUI-Decale/QWidgetMyDecale.h"



class Constraints
{
public:
    Constraints();



    static enum pse_res_t gamut_constratint
            (const struct pse_eval_ctxt_t* ,
             const struct pse_eval_coordinates_t* eval_coords,
             struct pse_eval_relshps_t* eval_relshps,
             pse_real_t* costs);
    static enum pse_res_t min_dist_constratint
            (const struct pse_eval_ctxt_t* eval_ctxt,           //current state
             const struct pse_eval_coordinates_t* eval_coords,  //coordinates of points
             struct pse_eval_relshps_t* eval_relshps,           //cost of the relationship// 1 cost per relship
             pse_real_t* costs);
    static enum pse_res_t max_dist_constratint
            (const struct pse_eval_ctxt_t* eval_ctxt,
             const struct pse_eval_coordinates_t* eval_coords,
             struct pse_eval_relshps_t* eval_relshps,
             pse_real_t* costs);
    static enum pse_res_t alignement(
            const struct pse_eval_ctxt_t* eval_ctxt,
                         const struct pse_eval_coordinates_t* eval_coords,
                         struct pse_eval_relshps_t* eval_relshps,
                         pse_real_t* costs);
protected:
    static bool isInsideTheImage(int i,int j, int w, int h);


};

#endif // CONSTRAINTS_H
