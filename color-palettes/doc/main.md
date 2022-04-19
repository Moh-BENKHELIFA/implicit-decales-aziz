# Parameter Space Exploration {#mainpage}

This project introduces a set of libraries dedicated to the description of
constrained parameter spaces on which we can apply optimizations. The only
optimization available now is the minimization of the costs of all specified
constraints.

We first describe all the [high-level concepts](#concepts) available through
this project. We then explain the [architecture](#architecture) of the project
along with some key points on the [coding style](#codingstyle). Finally, we
propose a [concrete and documented example](#example).

Keep in mind that it still misses [some features](#missing) that we have to
implement before having the planned consistent set of libraries.

# Concepts {#concepts}

The project introduces many concepts for many reasons:

* To allow a proper and fine description of the data.
* To allow optimizers to have a better knowledge on what are the data, how they
  are related to each other and thus, to allow better performances.
* To manage properly the exchange between the libraries and the user by using
  dedicated concepts.
* To propose flexibility to the users in order to manage many use cases.

## Overview

Here, we define all the concepts quickly to provide a good overview. Next
section gives more details on these concepts:

* _Parameter Space_: a space that can be parametrized, i.e. in which we can
  define parametric points with coordinates that can be _optimized_ and
  thus updated by the implemented optimization algorithms.
* _Parametric Point_: a point in a parameter space.
* _Constrained Parameter Space_: it's a parameter space in which we add explicit
  constrained relationships on a finite set of explicitly defined parametric
  points.
* _Relationship_: represent a relationship between parametric points or between
  parametric points and other user-specific concepts.
* _Cost Functor_: a function that is able to compute the cost of a constraint of 
  a relationship.
* _Relationship Constraint_: it's the association of a relationship with a
  specific cost functor. A relationship may have many constraints.

For advanced usage, we also have these concepts:

* _Variations_: a parameter space can have variations, that can also be seen as
  possible transformations. These variations allows to transform the parametric
  points and to apply the optimizer on these transformed parametric points. This
  allow to optimize in the parameter space and its variations **at the same
  time**. In addition, we can explicitly provide the variations in which the
  relationships are valid.

We also have more technical concepts. They are related to the architecture of
the project and how things are implemented:

* _Device_: the main object representing the root node of all other objects
  created through the API. It allows more than one instance of the libraries in
  a same program, but it also allows to manage finer configuration requests,
  along with its validation, before starting to use the library.
* _Parametric Point Values_: the description of the parametric points and the
  relationships of a constrained parameter space **do not need** to know the
  concrete values of the parametric points. But when comes the optimization, the
  user has to give them. This concept allows the user to describe how the values
  data will be accessed.
* _Exploration Context_: a context of optimization allows to freeze the current
  state of a constrained parameter space before its optimization. Whatever will
  be the changes made on the constrained parameter space later, the context will
  stay valid and will work on the frozen state. Mainly, this allows to have
  concurrent edition and computation on the same constrained parameter space.
* _Exploration Samples_: as we optimize using specific values for the parametric
  points, the samples represent the values that must be used during the
  optimization. This concept will also be able to carry more information in the
  future.
* _Exploration Results_: the results of the optimization is not only made of the
  new coordinates of the parametric points, but also some extra results that are
  represented by this concept.
* _Evaluation Context_: a cost functor is evaluated with regards to a specific
  evaluation context, including the current parametric points values, but also
  the relationships for which we must evaluate the cost functor.

## More details on... {#details}

### Parameter space {#pspace}

In this project, a parameter space is described as simple as possible:

1. How a parametric point is represented in this parameter space;
2. Which are the possible variations of this parameter space.

The representation of a parametric point is simply a list of attributes. Each
attribute is defined by its number of components. Each component is defined by
its specific type. For now, there is only two possible attributes:

* **Coordinates** (::PSE_POINT_ATTRIB_COORDINATES): describes how the
  coordinates of the parametric points are represented. **This attribute is
  mandatory**. This attribute must have at least one component, and can be read
  and written by optimizers. Type of coordinates must be ::PSE_TYPE_REAL defined
  in ::pse_type_t.
* **Lock status** (::PSE_POINT_ATTRIB_LOCK_STATUS): says if a parametric point
  is locked for optimization. If it's locked, an optimizer will not be able to
  modify its coordinates. This attribute is optional and must have only one
  component of type ::PSE_TYPE_BOOL_8 defined in ::pse_type_t. By default, all
  parametric points are considered unlocked.

See the type ::pse_pspace_params_t to have more technical details.

#### Conversion between parameter space {#pspace_conversion}

The conversion between different parameter space is something managed at the
optimization context creation time. Check @ref exploration_ctxt to have more
information.

Putting this setup as late as possible allows to use different conversion
functions without changing either the parameter space nor the constrained
parameter space. That allows to create many optimization contexts on the same
constrained parameter space with different conversion functions.

### Parametric point {#ppoint}

When the user add a parametric point, some parameters must be defined. For now,
there is no specific parameter but the type has been defined and required by the
API. This will be simpler to integrate changes when future parameters will come.

See the type ::pse_ppoint_params_t to have more technical details.

#### Parameter point attributes {#ppoint_attribs}

Parametric points attributes describe how is represented a parametric point.
They are defined in the parameter space as this is a constant definition for all
parametric points of a same parameter space. See @ref pspace for more
information.

### Cost functor {#cfunc}

A cost functor is mainly there to say to the optimizers how to compute the costs
of the constraints of the relationships. But in order to do the computation,
many other information must be provided:

* A unique identifier defined by the client. As cost functors must be referenced
  in relationships, we use such unique and user-defined identifiers to allow
  pre-declaration of relationships.
* The parameter space in which it expects to receive the coordinates of the
  parametric points.
* The number of costs it will compute, and if it will be per relationship of per
  parametric point.
* The type information for its evaluation context, if any. See @ref cfunc_ctxt
  for more information on this parameter.
* A user configuration for the cost functor. This configuration will be passed
  during the initialization and the cleaning of each evaluation context created
  using the type information described above.

See the type ::pse_relshp_cost_func_params_t to have implementation details.

#### Arity {#cfunc_arity}

Some words on the arity of a cost functor, i.e. the number of costs generated by
the cost functor. It is very important to understand that it's only the cost
functor that knows how may costs it will compute. That's why it's not an
information carried by the relationships.

#### Contexts for evaluation during optimization {#cfunc_ctxt}

During optimization, the computation of cost functors is called. In order to let
the cost functor callback has the possibility to get or store information per
relationship, the evaluation context has been introduced. This way, the user can
store specific information per relationship to be used during the evaluation.

#### Computation {#cfunc_compute}

The computation of a cost functor is done through a callback of type
::pse_clt_relshp_cost_func_compute_cb. In order to allow best performance and
batch computation on the user side, when possible, the function is called with
many relationships to evaluate at once.

### Relationship {#relshp}

A relationship allows to define how some parametric points are related with
each other or with an user-defined concept. Constraints can be attached to a
relationship. For now, a constraint is made of a cost functor used to compute
the cost of the constraint during optimization.

See ::pse_cpspace_relshp_cnstrs_t for more technical information.

#### Reference to parametric points {#relshp_ppoints}

To reference the parametric points, we have two kind of relationship right now:

* *Inclusive* relationship, which includes all the parametric points referenced
  in the list provided by the user.
* *Exclusive* relationship, which excludes all the parametric points referenced
  in the list provided by the user. In this case, the list of parametric points
  that are included in the relationship at the end is deduced as late as
  possible, i.e. when no more modification is possible on the list of parametric
  points available in the constrained parameter space.

Note that with exclusive relationships, it's possible to have constraint on all
parametric points at once, without knowing their list, by giving an empty list
of excluded parametric point.

#### Constraints {#relshp_cnstrs}

For a relationship, in this project, a constraint means a "cost": the
relationship will cost something, and this cost will be computed during
optimization using the cost functor provided. It's possible to give more than
one cost functor. In the end, the number of costs computed for a relationship
will be the sum of the number of costs computed by each cost functor.

#### Enabling/disabling relationship {#relshp_toggle}

The API allows to enable and disable relationships. A disabled relationship act
as if it was not existing: it will generate no cost and thus will not influence
the optimization.

See ::pseConstrainedParameterSpaceRelationshipsStateGet and
::pseConstrainedParameterSpaceRelationshipsStateSet for more technical
information on this.

#### Grouping relationships {#relshp_groups}

The API allows to group relationships in order to manipulate them as a whole.
To that purpose, it's possible to provide a user defined unique identifier at
the relationships creation time. All relationships that use the same identifier
will be grouped together. It's then possible to remove, enable or disable all
relationships of a group at once.

### Constrained parameter space {#cpspace}

A constrained parameter space is the main concept for the description of the
data. It's a parameter space in which we have declared parametric points with
possibly relationships.

For now, a constrained parameter space do not have specific parameters at
creation time.

### Constrained parameter space values {#values}

To let the optimizers get and set values from and to the parametric points, the
concept of constrained parameter space values has been introduced in the API.
This allows the user to describe how he wants the data to be accessed. This
includes the possibility to lock/unlock the use of data: if the user wants to
make modifications, it must lock the values to make the optimizer aware and then
wait for the unlock event before doing anything else.

#### Values data {#values_data}

This is were are defined the accessors for the data. See
::pse_cpspace_values_data_t for more technical information.

Data must say in which parameter space it is expressed in. It also gives the
accessors to get/set the values. Here, there are two possibilities:

* *Global* accessors: there are only one get/set for all attributes of the
  parametric points.
* *Per attribute* accessors: there are one get/set per attribute of the
  parametric points.

It's up to the user to choose the best solution for his use case. This mechanism
allows to avoid the duplication of the data when possible.

### Exploration {#exploration}

Exploration of a constrained parameter space is in fact the process of
minimization of the costs of all constraints of the constrained parameter space.

#### Exploration context {#exploration_ctxt}

We force the use of a context for the exploration. This allows us to freeze the
current state of the constrained parameter space we want to explore. This frozen
state will be used by all consequent exploration done through this context, even
if the constrained parameter space is later modified through the API.

#### Exploration options {#exploration_opts}

Exploration as many options. The most important one is the parametric space in
which the exploration will be done. It's also here that the conversion function
is provided, allowing the optimizer to convert the coordinates of the parametric
points from one parameter space to another.

Concerning variations, check @ref variations_exploration for more information.

Finally, the use can provide some low-level hints regarding the convergence
mechanism of the minimization. **It's a work in progress API and will be better
described later**.

#### Exploration samples {#exploration_smpls}

To explore a constrained parameter space, we need initial coordinates. The
samples give thes initial values from which the optimization will start. Right
now, it's only the initial coordinates of the parametric points given as
constrained parameter space values.

#### Exploration results {#exploration_results}

The exploration results are of two kinds:

* The optimized coordinates of the parametric points;
* The extra results provided by the optimizer.

The optimized coordinates are sent to the user-space by using the constrained
parameter space values `set` callback. The extra results are only internal
counters for now, allowing the user to have some statistics on what happened.

#### Initialization of the evaluation contexts of the relationships {#exploration_relshps_ctxts_init}

The evaluation of the relationships is the computation of the costs using the
cost functors making the constraints. These cost functors work with evaluation
contexts that may need to be initialized. We provide a way to call automatically
the initialization function on evaluation context, by passing a user data to
them. See ::pseConstrainedParameterSpaceExplorationRelationshipsAllContextsInit
function for more technical information.

### Evaluation {#eval}

Evaluation occurs during optimization, when it's time to compute the costs of
the relationships. Each cost functor has a `compute` callback that respect a
specific definition that we copy here:

```{.c}
typedef enum pse_res_t
(*pse_clt_relshp_cost_func_compute_cb)
  (const struct pse_eval_ctxt_t* eval_ctxt,
   const struct pse_eval_coordinates_t* eval_coords,
   struct pse_eval_relshps_t* eval_relshps,
   pse_real_t* costs);
```

We can see that a cost functor must return a result code. We can also see that
we have four parameters: the evaluation context, the current coordinates of the
parametric points, the relationships to evaluate and the buffer of costs to
fill.

* `eval_ctxt`, _the evaluation context_

  It provides the device, the constrained parameter space and the exploration
  context for which we are currently evaluating the cost functor;

* `eval_coords`, _the coordinates of the parametric points_

  It provides the parameter space in which the coordinates are expressed, and
  the **number of scalars** that represent the coordinates. **The parameter
  space defines the number of components for the coordinate attribute of its
  parametric points, thus the number of scalars provided here is equal to the
  number of components of the coordinate attribute multiplied by the number of
  parametric points. In addition, note that the optimizer has converted *ALL*
  components to the type `pse_real_t`.**

* `eval_relshps`, _the relationships to evaluate_

  The relationships to evaluate, i.e. the list of relationships that have this
  cost functor as a constraint. Relationships come along with their data: the
  list of parametric points involved in each relationship, the evaluation
  context and the specific configuration provided for this cost functor during
  the creation of the relationships.

* `costs`, _the costs_

  The buffer in which the callback must store the costs it has computed. **The
  number of costs to fill is equal to the number of costs declared by the cost
  functor multiplied by either the number of relationships or the number of
  parametric points depending on the cost functor arity mode.**

### Variations {#variations}

The variations of a parametric space allow to provide transformation that can be
used during optimization. At creation time, we list all possible variations for
a specific parametric space. During the definition of the relationships, we can
say, per relationship, for which additional variations they must be evaluated.
Finally, during the creation of the optimization context, we can provide the
variations that must also be used during the optimization. This way, the
optimizer will optimize coordinates in the provided parameter space without
variations, but also using the provided variations.

#### Variations application {#variations_apply}

During optimization, the optimizer will have to apply the variation starting
from the main representation of the coordinates of the parametric points in the
parametric space, to their transformed representation in a given variation.

This `apply` callback is provided by the user when he creates the optimization
context. That means the user can use different conversion callbacks for the same
variations but in different optimization context.

See ::pse_ppoint_variation_apply_cb for more technical information.

#### Relationships and variations {#variations_relshps}

By default, a relationship is not directly associated to a parametric space.
It's the constraints, i.e. cost functors, that are defined for a specific
parametric space. However, a relationship can specify in which additional
variations it is valid.

For example, taking a relationship \f$R_1\f$ using two cost functors \f$F_1\f$
and \f$F_2\f$, respectively defined for the parameters spaces \f$\Omega_1\f$ and
\f$\Omega_2\f$. If we do nothing else, the relationship \f$R_1\f$ will be
evaluated twice, once using \f$F_1\f$ callback in \f$\Omega_1\f$ and once using
\f$F_2\f$ callback in \f$\Omega_2\f$.

This time, we add three variations \f$v_{1,1}\f$, \f$v_{1,2}\f$ and
\f$v_{2,1}\f$, the first two being variations of the parameter space
\f$\Omega_1\f$ and the last one a variation of the parameter space
\f$\Omega_2\f$. We allow all variations for all relationships. And we assume
that the optimization context allows all these variations. In this context,
\f$R_1\f$ will be evaluated five times:

- Once in \f$\Omega_1\f$ without variation, using the \f$F_1\f$ callback;
- Once in \f$\Omega_1\f$ with variation \f$v_{1,1}\f$, using the \f$F_1\f$
  callback;
- Once in \f$\Omega_1\f$ with variation \f$v_{1,2}\f$, using the \f$F_1\f$
  callback;
- Once in \f$\Omega_2\f$ without variation, using the \f$F_2\f$ callback;
- Once in \f$\Omega_2\f$ with variation \f$v_{2,1}\f$, using the \f$F_2\f$
  callback.

If the user allows variations on relationships that are not applicable on the
parameter spaces of the cost functors of the constraints, these variations will
not be used. In our example, if we have now a relationship \f$R_2\f$ that uses
only the cost functor \f$F_2\f$, if we allow all variations for all
relationships like before, i.e. \f$R_2\f$ will allow \f$v_{1,1}\f$,
\f$v_{1,2}\f$ and \f$v_{2,1}\f$, then in this context, \f$R_2\f$ will be
evaluated two times:

- Once in \f$\Omega_2\f$ without variation, using the \f$F_2\f$ callback;
- Once in \f$\Omega_2\f$ with variation \f$v_{2,1}\f$, using the \f$F_2\f$
  callback.

#### Exploration and variations {#variations_exploration}

During optimization, we explicitly set the parameter space in which we want to
optimize. If another parameter space is needed by one or more cost functors, the
conversion callback provided by the user is called to convert coordinates from
the main parameter space to this needed parameter space. For variations, we have
exactly the same mechanism: the user explicitly gives the variations in which he
wants to optimize **in addition** of the parameter space without variation. When
it will be needed, the optimizer will apply the variation on coordinates by
using the callback provided by the user.

See ::pse_cpspace_exploration_ctxt_params_t and
::pse_cpspace_exploration_variations_params_t for more technical information.

# Architecture {#architecture}

Here is the architecture section. **More or less deprecated. Must be reworked to
fit the evolutions**.

\dotfile "architecture.dot" "High-level architecture description"

# Coding style {#codingstyle}

C ANSI is used for portability and its simplicity. It will also allow to
implement bindings to other languages more easily.

Here are some key points of the coding style in use in this project:

* We use prefix all the time (`pse`, `PSE`) to ensure the namespace is clearly
  named and not avoided;
* Name of the types end with `_t`;
* We avoid the use of `typedef` with `struct` and `enum` when possible to force
  the user to **explicitly** says what is talking about;
* We use the same name for constants hidden under macros and for `static const`
  values. The only difference is the underscore `_` at the end of the name for
  macros. To avoid duplication, the `static const` values are initialized by the
  associated macro.
* Name of the functions respect the namespace scheme: start from the global
  namespace and go to the local namespace. Then we use a verb to describe the
  action and possibly some "modifiers" to **explicitly** provide information on
  the _how_. For example with the function
  ::pseConstrainedParameterSpaceRelationshipsRemoveByGroup:
  * `pse` > `ConstrainedParameterSpace` > `Relationships`, the namespaces global
    to local that could be read as "In PSE, I want to work on a Constrained
    Parameter Space, and more precisely on its Relationshps".
  * `Remove` for the action, so we know that we want to remove a relationship
    from a constrained parameter space in PSE.
  * `ByGroup` to explain how it will be removed. Here, it will be by group, so
    by using a group identifier.
  * Parameters names and types give more information on the _how_.
  * And if it's not enough to understand the behavior, a comment is added.
* For performance reasons, we try to declare callbacks that work on list of
  objects instead of one object only. This allows less function calls and this
  allows batch computations on client side. As these callbacks are the
  "channels" by which the client code and the optimizers will communicate, it's
  important to limit their number at runtime because they could be expensive.
* Nearly all functions return a `enum pse_res_t` which is a result code. This
  way, we let the caller manage the result as he wants, and **react consistently
  in his context, that the libraries cannot know or understand**.
* Some concepts use reference counting mechanism. In this case, the associated
  `Create` function return an object with a reference, given to the caller. It's
  then up to the caller to manage properly the references by calling the
  associated `RefAdd` and `RefSub` functions. Some functions of the API may keep
  references (i.e. increment by one the number of references) on objects passed
  as parameters. The last `RefSub` will destroy the object.
* We try to fit the code to 80 columns. This constraint has lots of good
  properties. One of them is the ability to let developers use the IDE they
  love, even those only usable from command line. Another one is to force the
  developers to keep the code clean, forcing them to think about the readability
  of their code by others.

For the implementation, we can add more key points:

* We try to avoid `include` when it's possible, by limiting the types,
  functions, etc. that are made public to other parts of the code. If needed, we
  try to separate types into many files in order to avoid the inclusion of a big
  monolithic file.
* To respect the previous point, the libraries have public API, we often use the
  naming file convention below, when it's needed of course:
  * Public API use file names of the form `name.h`.
  * Private headers, **used only by the implementation**, use file names of the
    form `name_p.h`, with the `_p` for `private`.
  * Implementation code use file names of the form `name.c`.
* We use the `goto` mechanism as a try/catch mechanism, allowing, for example,
  the functions to restore the previous state on error.

# Concrete examples to get started {#example}

* \ref example1_basics : this first example use all basic concepts and should be
  the first example to check.


# Missing features {#missing}

All the features described here must be implemented to finalize a first version
of this project. They are needed to finalize the expected and consistent
behavior of the libraries:

* Parallel computation, along with a thread-safe API;
* Serialization to save/restore, at least, data described through the API;
* Interpolation optimizer, allowing to optimize the path between two set of
  values of parametric points for one constrained parameter space. We could also
  allow the interpolation between two different constrained parameter space if
  they have the same number of parametric points.

