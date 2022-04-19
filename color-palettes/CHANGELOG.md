# _Parameter Space Exploration_ changelog

## Libraries

### `pse`

#### pre-alpha 0.7.0 to 0.8.0

* Big step forward on the documentation with a specific documented example;
* Now Solve() return RES_NOT_FOUND when no optimization was possible;
* It's now possible to give the epsilon value that will be used by the
  automatic differentials computation. See pse_cpspace_exploration_options_t.

#### pre-alpha 0.6.0 to 0.7.0

* Add convergence mode in the solver for the Solve() function
* Iterations/costs calls counters are now filled and are no more optional

#### pre-alpha 0.5.0 to 0.6.0

* Remove the `dev` parameter of the unused
  `pseConstrainedParameterSpaceExplorationContextCreate`;
* Rename `pseConstrainedParameterSpaceParameterSpacesVariationsRemove` to
  `pseConstrainedParameterSpaceParameterSpacesVariationsRemoveIfExist` to
  explicit the behavior;
* Introduce the concept of state for relationships and group of relationships.
  This talks about the enable/disable flag;
* Rename `pseConstrainedParameterSpaceRelationshipsSwitch*` to
  `pseConstrainedParameterSpaceRelationshipsStateSet*` to explicitly talk about
  the concept of state;
* Add `pseConstrainedParameterSpaceRelationshipsSameStateSet`;
* Add `pseConstrainedParameterSpaceRelationshipsStateGet`;
* Add `pseConstrainedParameterSpaceRelationshipsGroupStateGet`;
* Rename `PSE_DEVICE_PARAMS_DEFAULT` to `PSE_DEVICE_PARAMS_NULL`;
* Add `pseConstrainedParameterSpaceParametricPointsCountGet`;
* Add `pseConstrainedParameterSpaceRelationshipsCountGet`;
* Fix memory management on destruction of the device: everything is destroyed;
* Fix memory management for `values` and `exploration context` that now keep a
  reference on the CPS to ensure it will be kept alive until their release;
* Fix cost functors ID reuse;
* Fix relationships ID reuse;
* Fix `pseConstrainedParameterSpaceRelationshipsParamsGet` returning success on
  bad argument;
* Fix `stb_sb_delnat` that was not moving the right number of elements;
* Fix all warnings with GCC 7.4;
* Fix some warnings with MSVC 19.21.27702.2 x64;

### `pse-clt`

#### pre-alpha 0.1.0 to 0.2.0

* Add filtering management for L1 distance computation

### `pse-clt-space-color`

#### pre-alpha 0.2.0 to 0.3.0

* Add the convergence mode management: now, solve can return RES_NOT_CONVERGED
* Fix to/from HSV color space conversion
* Fix bad XYZ color space configuration

#### pre-alpha 0.1.0 to 0.2.0

* Add OLED screen energy consumption constraint, meaningful only in RGB color
  space;
* Add Rasche2005 CVD algorithm for CVD variations;
* Add XYZ color space;
* Fix Gamut constraint to work only with RGB color space;
* Fix the distance threshold use to make it generates good costs;

