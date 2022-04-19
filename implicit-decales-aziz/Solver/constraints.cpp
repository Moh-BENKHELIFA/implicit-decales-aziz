#include "constraints.h"
#include <iostream>
#include <map>

int costFactor = 1;

Constraints::Constraints()
{


}

bool Constraints::isInsideTheImage(int i,int j, int w, int h){
    return i>=0 and i<w &&  j>=0 and j<h;
}


pse_res_t Constraints::gamut_constratint(const pse_eval_ctxt_t *, const pse_eval_coordinates_t *eval_coords, pse_eval_relshps_t *eval_relshps, pse_real_t *costs)
{
    assert(eval_relshps->count == 1);
    const struct pse_eval_relshp_data_t* data = eval_relshps->data[0];

    auto gamut = MyDecalSolver::mygamut;


    size_t height = gamut->getIHeight();
    size_t width  = gamut->getIWidth();

    for(size_t i = 0; i < data->ppoints_count; ++i) {
        costs[i] = 0;

        const pse_ppoint_id_t ppidx = data->ppoints[i];
        const pse_real_t x = eval_coords->coords[i*2+0];
        const pse_real_t y = eval_coords->coords[i*2+1];

        int m = int(floor(x-2.5) + 1); //floor = arrondi au plus petit
        int n = int(floor(y-2.5) + 1);

        // compute cost for out of gamut positions
        auto computeCost = [](double distance) {
          const double stepValue = 10; // distance between the decale and the gamut after repositionning
          return costFactor * (stepValue + distance );
        };

        double signedDistance = 0.0;

        auto getGamutValue = [gamut](int u, int v, double xx, double yy ){
            double result = 0.;
            if ( u == 0 or v == 0) {
                result = double(gamut->getDistanceField()->eval(u,v));
            }
            else{
                result =( (u-xx)    *( ((v-yy)     * gamut->getDistanceField()->eval(u-1,v-1))   +
                                       ((yy-(v-1)) * gamut->getDistanceField()->eval(u-1, v)) )) +
                        ( (xx-(u-1))*( ((v-yy)     * gamut->getDistanceField()->eval(u,v-1))     +
                                       ((yy-(v-1)) * gamut->getDistanceField()->eval(u, v))   ));
            }
            return result;
        };

        if(isInsideTheImage(m,n,int(width),int(height))) {
            std::map<pse_ppoint_id_t, DecaleScalarField2D*>::iterator it;
            it = MyDecalSolver::mydecalsmap.find(ppidx);
            signedDistance = std::max(
                        0.0,
                        getGamutValue(m, n, x, y ) +
                        MyDecalSolver::mydecalsmap.at(it->first)->getSize() // radius
                                      );
            costs[i] = ( signedDistance > 0. ) ? computeCost( signedDistance ) : 0.0;

        }else{
            // 1. get value on the boundary of distance field
            {
                int mPrime, nPrime;

                if(m >= int(width))  mPrime = int(width) - 1;
                else if(m < 0)  mPrime = 0;
                else            mPrime = m;
                if(n >= int(height)) nPrime = int(height) - 1;
                else if(n < 0)  nPrime = 0;
                else            nPrime = n;

                signedDistance = gamut->getDistanceField()->eval(mPrime,nPrime);
            }

            // 2. add distance to boundary
            double distToBoundary = 0.;
            if(y < 0 and x > 0 and x <= width){//top
                distToBoundary = y;
            }else if(y >=height and x > 0 and x <= width){//bottom
                distToBoundary = y - height;
            }else if(x < 0 and y > 0 and y <= height){//left
                distToBoundary = x;
            }else if(x >=width and y > 0 and y <= height){//right
                distToBoundary = x - width;
            }else if(x < 0 and y < 0){//top left
                distToBoundary = std::sqrt(std::pow(x-0, 2) + std::pow(y-0, 2));
            }else if(x >=width and y < 0){//top right
                distToBoundary = std::sqrt(std::pow(x-width, 2) + std::pow(y-0, 2));
            }else if(x < 0 and y >= height){//bottom left
                distToBoundary = std::sqrt(std::pow(x-0, 2) + std::pow(y-height, 2));
            }else if(x >=width and y >= height){//bottom right
                distToBoundary = std::sqrt(std::pow(x-width, 2) + std::pow(y-height, 2));
            }else{
                std::cout<<"outside of the mat "<<std::endl;
            }
            costs[i] += computeCost( signedDistance + distToBoundary );
        }

    }
    return RES_OK;
}

pse_res_t Constraints::min_dist_constratint(const pse_eval_ctxt_t *eval_ctxt,
                                            const pse_eval_coordinates_t *eval_coords,
                                            pse_eval_relshps_t *eval_relshps,
                                            pse_real_t *costs)
{
    size_t i;
    (void)eval_ctxt;

    for(i = 0; i < eval_relshps->count; ++i) {
        const struct pse_eval_relshp_data_t* data = eval_relshps->data[i];
        assert(data->ppoints_count == 2);

        const pse_ppoint_id_t ppidx1 = data->ppoints[0];
        const pse_ppoint_id_t ppidx2 = data->ppoints[1];

        const pse_real_t x1 = eval_coords->coords[ppidx1*2+0];
        const pse_real_t y1 = eval_coords->coords[ppidx1*2+1];

        const pse_real_t x2 = eval_coords->coords[ppidx2*2+0];
        const pse_real_t y2 = eval_coords->coords[ppidx2*2+1];

        double dist = std::max(std::abs(x2-x1), std::abs(y2-y1));

        std::map<pse_ppoint_id_t, DecaleScalarField2D*>::iterator it1;
        std::map<pse_ppoint_id_t, DecaleScalarField2D*>::iterator it2;
        it1 = MyDecalSolver::mydecalsmap.find(ppidx1);
        it2 = MyDecalSolver::mydecalsmap.find(ppidx2);

        double r1 = MyDecalSolver::mydecalsmap.at(it1->first)->getSize();
        double r2 = MyDecalSolver::mydecalsmap.at(it2->first)->getSize();

        costs[i] = costFactor * std::min(0.0, dist-r1-r2);

    }


    return RES_OK;

}

pse_res_t Constraints::max_dist_constratint(const pse_eval_ctxt_t *eval_ctxt,
                                            const pse_eval_coordinates_t *eval_coords,
                                            pse_eval_relshps_t *eval_relshps,
                                            pse_real_t *costs){
    size_t i;
    (void)eval_ctxt;

    for(i = 0; i < eval_relshps->count; ++i) {
        const struct pse_eval_relshp_data_t* data = eval_relshps->data[i];
        assert(data->ppoints_count == 2);

        const pse_ppoint_id_t ppidx1 = data->ppoints[0];
        const pse_ppoint_id_t ppidx2 = data->ppoints[1];

        const pse_real_t x1 = eval_coords->coords[ppidx1*2+0];
        const pse_real_t y1 = eval_coords->coords[ppidx1*2+1];

        const pse_real_t x2 = eval_coords->coords[ppidx2*2+0];
        const pse_real_t y2 = eval_coords->coords[ppidx2*2+1];

        //std::cout<<i<<"relship: "<<ppidx1<<"-"<<ppidx2<<std::endl;

        double dist = std::sqrt(std::pow(x2-x1, 2) + std::pow(y2-y1, 2));//L2 dist;

        //double dist = std::max(std::abs(x2-x1), std::abs(y2-y1));//L1 dist

        std::map<pse_ppoint_id_t, DecaleScalarField2D*>::iterator it1;
        std::map<pse_ppoint_id_t, DecaleScalarField2D*>::iterator it2;
        it1 = MyDecalSolver::mydecalsmap.find(ppidx1);
        it2 = MyDecalSolver::mydecalsmap.find(ppidx2);

        int id1 = it1->first;
        int id2 = it2->first;

        double maxDist = 150; //if <150 collision doesn't work
        double f =  std::max(0.0, dist - maxDist);

        //2 decals relation

        //C
        if( id1 == 0 and id2 == 2)
            costs[i] = f;
        else
            costs[i] = 0;

        costs[i] *= costFactor;
    }

    return RES_OK;
}

static enum pse_res_t alignement(
        const struct pse_eval_ctxt_t* eval_ctxt,
                     const struct pse_eval_coordinates_t* eval_coords,
                     struct pse_eval_relshps_t* eval_relshps,
        pse_real_t* costs){

    return RES_OK;
}




