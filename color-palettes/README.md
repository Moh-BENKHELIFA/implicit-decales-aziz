# Parameter Space Exploration (PSE) libraries

![pipeline status](https://gitlab.com/Storm-IRIT/private-projects/color-palettes/badges/master/pipeline.svg)

## Installing

### On Linux

Here is the procedure to get the code, build it, launch the test and run the
documented example provided by the project:

```
git clone git@gitlab.com:Storm-IRIT/private-projects/color-palettes.git
# Ensure that you have Eigen3 installed, with e.g.:
# apt install libeigen3-dev
cd color-palettes
mkdir build
cd build
cmake -DPSE_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release ../cmake
make -j`nproc`
make test
LD_LIBRARY_PATH=./libs ./examples/example1_basics
```

## Description of the software

### Core libraries

* `pse`: main library that manage the API and the common internal behavior;
* `pse-drv-eigen-ref`: driver providing a default Eigen implementation of the
   exploration, using a Levenberg-Marquardt solver;
* `pse-drv-eigen`: ***This driver is broken right now!*** driver providing an
  Eigen implementation of the exploration, using a Levenberg-Marquardt with
  tuned differential computation, that provide extra performances.

### Client libraries

This project comes with some libraries allowing the use of PSE in specific
contexts, or to help the user to implement a layer above PSE to fit his needs:

* `pse-clt`: a library with utilities for client implementation of parameter
  spaces;
* `pse-clt-space-color`: the color space layer above `pse` that manage color
   palettes and their exploration. This library offers a higher vision of the
   concepts to keep its use as simple as possible.

### Tests

These tests are used to validate the behavior of the libraries provided by this
project.

* `test_api`: unit test for the `pse` API;
* `test_api_exploration`: test more further the exploration API, by using a more
  sophisticated implementation of parameter spaces;
* `test_clt_api`: test the utility library for client implementation;
* `test_clt_space_color_api`: unit test for the `pse-clt-space-color` API.
  **Work in progress**.

### Command Line Instructions

These CLI are provided as tools that can be used through command line. They are
wrappers above the libraries offered in this project.

* `pse_space_color_exploration`: command line tool to do explorations in
  color space. This command line instruction use the `pse-clt-space-color`
  client library;

Ideas are:

* Explore color space through command line. Use the serialization mechanism to
  read/write the results for example, or as images for the results;
* Transform an image to its related version using CVD algorithms.

## Remaining tasks

- [ ] Serialization, with at least the YAML driver and, if not too long, the
      binary driver.
- [X] Weighted constraints in `pse-clt-space-color`. We need to find a way to
      give a kind of `user_data` per relationship during the context
      initialization. Probably not hard, but it's missing right now.
- [ ] Add again all the missing cost functors in `pse-clt` or
    `pse-clt-space-color`.
- [X] Add again all the missing (or partially implemented) color spaces in
    `pse-clt-space-color`: XYZ, HSV, LUV and Cat02 LMS.
- [X] Add again color space LUV to `pse-clt-space-color`.
- [X] Rasche2005 algorithm for CVD.
- [ ] Add again utilities related to colors in `pse-clt-space-color`: palette
      processing to sort colors regarding a specific criteria, palettized
      raster, palettizer and probably forgotten little things.
- [ ] Interpolation solver! That means to add a new concept in the API to allow
      to declare an interpolable CPS (ICPS) and to call an interpolation solver
      on it.  This ICPS will take parameters explaining how to bind parametric
      points and relationships, and how to interpolate.
- [ ] Tests, tests and tests!
