# Example using basic features {#example1_basics}

This example use the basics of PSE. Its purpose is to force 2D points to keep a
minimal distance between each other. To keep the example simple, we add 3 points
programmatically, we lock the first one and we ask the optimizer to move the
other ones to respect this minimal distance.

## Device creation

In order to setup and create a device, we have to delcare device parameters and
a device handle.

\snippet example1_basics.c device_decl

We fill the device parameters with simple values. We want to use the default
allocator, we want the logging mechanism to print on `stdout` and we want to use
the backend driver called `pse-drv-eigen-ref`.

\snippet example1_basics.c device_setup

After that, we can create the device with this call. If something goes wrong
here, we will not be able to go farther. Most of the time, an error to this call
is due to the impossibility to find the driver dynamic library. Please check the
[troubleshooting](#troubleshooting) section for more information.

\snippet example1_basics.c device_create

At the end of the program, we can destroy the device with this command. This
will destroy everything that have been created through this device.

\snippet example1_basics.c device_destroy

## Describing our constrained parameter space

For now, we have nothing else than the device ready. We have to describe our
constrained parameter space before doing optimization using it. We have many
steps to write before having our constrained parameter space ready.

Even if in this example we focus on a very simple constrained parameter space
structure, you will see with experience that most of the work done in this
simple case will have to be done for more complex cases. The reason is simple:
we have many concepts to manage, so we have many steps to code. These steps
reflect all the strong concepts of PSE and allow flexibility for many different
use cases.

### Step 1: creation of the constrained parameter space

Now we have the device, we can create a constrained parameter space. To that
purpose, we have to first declare the constrained parameter space parameters and
a constrained parameter space handle.

\snippet example1_basics.c cps_decl

Then we have to setup the constrained parameter space. For now, there is nothing
to do.

\snippet example1_basics.c cps_setup

We can now create the constrained parameter space in the device using this
command.

\snippet example1_basics.c cps_create

At the end of the program, we can manually release our reference on the
constrained parameter space using this command. If this is the last reference,
the constrained parameter space will be destroyed.

\snippet example1_basics.c cps_destroy

### Step 2: describing the parameter space

Our constrained parameter space is defined, we now have to define which
parameter spaces are related to it. In this example, we need only one and it
will be \f$\mathbb{R}^2\f$. We will have to reference this parameter space with a
unique identifier. We define a constant for that purpose.

\snippet example1_basics.c ps_constants

> NOTE: the UID is the only way to reference the parameter space. As it's a
> user defined value, the UID can be known before adding the parameter space to
> any constrained parameter space. 

Then we have to declare the parameter space parameters, its unique identifier
variable and the components for each attribute, i.e. the coordinates and the
lock status attributes. We use the lock status attribute in this example to
ensure that the first parametric point will not move.

For the coordinates, we use two real numbers as we are in \f$\mathbb{R}^2\f$.

For the lock status, we use one boolean.

\snippet example1_basics.c ps_decl

We can now setup the parametric space by using the variables we have just
declared. Here, we fill the parameter space parameters with the components
information.

\snippet example1_basics.c ps_setup

We use this properly filled parameters to declare the main parameter space of
the constrained parameter space. In this call, we can see that we declare only
one parameter space, using the unique identifier we defined above. This UID will
be used later to reference this parameter space.

\snippet example1_basics.c ps_create

At the end of the program, we can manually remove this parameter space from the
constrained parameter space with the command below.

\snippet example1_basics.c ps_destroy

### Step 3: adding parametric points

At this step, we have now a constrained parameter space defined, with its main
parameter space defined, thus describing the structure of the parametric point.
They are points in \f$\mathbb{R}^2\f$ with a lock status.

We will now add three parametric points in our constrained parameter space. To
that purpose we have to declare their parameters and the buffer that will
receive their identifiers at creation time.

\snippet example1_basics.c pp_decl

For now, there is no specific parameter for parametric points, so we will do
with the default values.

\snippet example1_basics.c pp_setup

Then, we can add the parametric points to the constrained parameter space with
this command. As you can see, we have to give their number, 3, their parameters
and the buffer that will received their identifiers.

\snippet example1_basics.c pp_create

> NOTE: for now, we don't have defined values for the coordinates of these
> parametric points: we are describing the structure of the constrained
> parameter space! Later, we will give specific values/samples during the
> optimization process in order to optimize a concrete situation.

At the end of the program, we can manually remove all parametric points using
this command.

\snippet example1_basics.c pp_destroy

### Step 4: defining the cost functor

At this step, we have a constrained parameter space with three parametric
points. We want to define relationships between these points in order to make
them separated with at least a minimal distance.

But before creating the relationships, we have to define the cost functors that
will compute a cost, for each pair of points. This cost will represent how it's
hard to keep the points at the current distance. If they are nearer than the
minimal distance, we want our system to say "it's hard to keep them at this
distance, I would prefer to have them farther from each other". If they are
farther than the minimal distance, we want our system to say "it's ok, they can
say at this distance, it's enough for me". Saying the same thing using
mathematics will give for two points \f$x_1\f$ and \f$x_2\f$:

\f[
  \begin{cases}
    cost\left(x_1,x_2\right) > 0 & \text{if } dist\left(x_1,x_2\right) < \text{MIN_DISTANCE} \\
    cost\left(x_1,x_2\right) = 0 & \text{oterwise}
  \end{cases}
\f]

And this can be written:

\f[
  cost\left(x_1,x_2\right) = \max\left(0, \text{MIN_DISTANCE} - dist\left(x_1,x_2\right)\right)
\f]

We will first define the constant that will represent this minimal distance.

\snippet example1_basics.c cfunc_constants

And then we define the function that will compute the cost described above. This
function will be used as the `compute` callback of the cost functor.

> NOTE: when the structure of the constrained parameter space is well defined,
> the compute functions of cost functors are where the behavior of the
> optimization is written: the costs will drive the optimization.

\snippet example1_basics.c cfunc_compute

Now we have define the behavio of our cost functor, we will have to register it
to our constrained parameter space in order to use it in our relationships.

We first start by the declaration of the cost functor parameters and the
variable that will store its identifier once it will be created.

\snippet example1_basics.c cfunc_decl

Then we setup the cost functor by providing the parameter space in which it
expects to received the coordinates of the parametric points, the function to
use to compute the costs and, of course, how many costs it will generate. Here,
we have written the function to generate one cost per relationship.

\snippet example1_basics.c cfunc_setup

Now we can register this cost functor to the constrained parameter space.

\snippet example1_basics.c cfunc_create

At the end of the program, we can manually unregister the cost functor from the
constrained parameter space by using this command.

\snippet example1_basics.c cfunc_destroy

### Step 5: building the relationships between parametric points

At this step, the constrained parameter space is waiting for the relationships.
We have defined the cost functor we want to use for our relationships, so it's
time to create them.

We first declare a buffer that will help us to store the pairs of parametric
points. We also have the parameters of each relationships and the buffer where
will be stored the identifiers of the newly created relationships.

\snippet example1_basics.c relshps_decl

We then have to setup the relationships. Here, we first build the pairs of
parametric points. We can do this step only **after** having created them as we
have to know their identifiers. Once done, we can then fill the parameters of
the three relationships we have, by giving them the pairs, one for each. Note
that we have inclusive relationships, i.e. the parametric points identifiers
given are the ones we want to bind with this relationship. We also define the
same constraint for each relationship, i.e. the cost functor defined previously.

\snippet example1_basics.c relshps_setup

Everything is read so we can create the relationships with this simple call.
Note the specific parameter where we give 0: it's a group identifier, but we
will not cover this concept in this example.

\snippet example1_basics.c relshps_create

At the end of the program, we can manually remove all the relationships from the
constrained parameter space by using this command.

\snippet example1_basics.c relshps_destroy

### Conclusion

We sum-up the five steps that were needed to build our constrained parameter
space:

1. Creation of the constrained parameter space it-self, as the container;
2. Definition of the parameter space we wanted to use;
3. Creation of the parametric points;
4. Definition of the cost functor we wanted to use;
5. Creation of the relationships between points.

As you can see, all these steps are mandatory to build a complete constrained
parameter space. That makes a long code but this is the compromise to have
everything written explicitly.

In the end, once the description of the constrained parameter space is done
properly, most of the work is done in the computation functions of the cost
functors, allowing the user to focus on only a little subset of lines of code.

## Exploration of the constrained parameter space

Now we have our constrained parameter space properly described, we can optimize
concrete situations using it. To do so, we have to first say how to get and set
the concrete values of the parametric points. From these values, we will be able
to launch the optimization and get the results.

### Storage of concrete parametric points values

We will use two different values object: one to represent the samples, i.e. the
initial values of the parametric points from which will start the optimization,
and another one to represent the result, i.e. the optimized values of the
parametric points.

In this example, parametric points have two kind of attributes: coordinates and
lock status. The values of the lock status is implicitly stored as we want to
lock only the first parametric point. For the coordinates, we will have to store
the values somewhere, and we will simply have a buffer for that purpose.

But for now, we will define two functions to get and set the values of the
parametric points attributes.

#### Getting values of parametric points attributes

This function is quite simple and must be changed only when the user internal
structure used to store the values has changed. We use only a simple buffer of
real numbers for the coordinates.

\snippet example1_basics.c vals_attrib_get

The `ctxt` parameter is a pointer on the user internal object where the values
are stored. As the user, we know exactly which is the type under this pointer.
As you will see later when we will provide this pointer during the setup of the
values, it's only a pointer on a buffer of real numbers.

The `attrib_values` parameter is the buffer where to store the values of the
attributes.  By the asserts, we ensure that we store the right types. In fact,
right now, the PSE libraries can only store coordinates as real number and lock
status as boolean on 1 byte, that's why we can keep this code simple.

The `values_idx` parameter stores the identifiers of the parametric points for
which we are asking for attribute values. Keep in mind that in some situation,
the PSE libraries may ask for only a subset of the parametric points values.

Note that for the lock status attribute, we check the index number to say if the
parametric point is locked or not. It's the dirty way to do the job as the first
parametric point in the list may not be the first parametric point of the
constrained parameter space. For simplicity, we will keep the code like this.

#### Setting values of parametric points attributes

This function has the same explanation than the previous section. The main
difference here is the absence of the lock status management which is a
read-only attribute.

\snippet example1_basics.c vals_attrib_set

#### Creation of constrained parameter space values

Having the get and set functions, we can now create the constrained parameter
space values objects that will serve as the communication channels between the
user space and the PSE libraries space. Using such mechanism ensure that the PSE
libraries will only duplicate values when it's necessary, whatever the reason.

First, we need to declare the values and their data accessors. We also declare
the buffer where are stored the values of the coordinates of all parametric
points.

\snippet example1_basics.c vals_decl

Data accessors must be properly setup. For the samples, we want to create read
only values. We defined them in \f$\mathbb{R}^2\f$. We use global accessors
because we have one get function for all attributes. We fill only the get
callback and we give our buffer for the coordinates values. For the result, we
want to create write only values. We obviously defined them in
\f$\mathbb{R}^2\f$ too. We fill only the set callback and we give the save
buffer for the coordinates values. This way, the optimization will overwrite the
initial values when we will get results. We could have used a different buffer
to keep the initial values.

\snippet example1_basics.c vals_setup

The we can simply create the values objects using these calls.

\snippet example1_basics.c vals_create

At the end of the program, we can manually release our references on these
values by using these commands.

\snippet example1_basics.c vals_destroy

### Optimization

Here we are. The constrained parameter space is properly described. We have the
communication channels ready to get and set the concrete values of the
parametric points through the constrained parameter space values, we can now
focus on the optimization it-self.

#### Preparation of the exploration context

An optimization will work through an optimization context. When we create an
optimization context for constrained parameter space, it's like if it was
instanced. That means that whatever the changes will be made to the constrained
parameter space **after** the creation of the exploration context, the later
will work with the state at instantiation time of the former. This behavior
allows the edition of the constrained parameter space at the same time it is
used for an optimization. As the later could take time, it's a very important
feature. It's also important to note that we can do many optimizations through
the same optimization context.

Back to the code. We need to declare the exploration context parameters and the
handle of the exploration context.

\snippet example1_basics.c explc_decl

The setup of the exploration context is quite simple, we just say that we want
to optimize in \f$\mathbb{R}^2\f$.

\snippet example1_basics.c explc_setup

The creation is also straight-forward by using this command.

\snippet example1_basics.c explc_create

At the end of the program, we can manually release our reference on the
exploration context by using this command.

\snippet example1_basics.c explc_destroy

#### Exploring the constrained parameter space starting from concrete samples

We have our exploration context ready, we can now do the concrete optimization
on concrete values. We need to declare exploration samples that will be used to
launch the optimization on concrete values.

\snippet example1_basics.c expl_decl

Then, the setup is quite simple. We just initialize the concrete values of the
coordinates of the parametric points in our internal buffer, and we give the
constrained parameter space values associated to the samples to the exploration
samples.

\snippet example1_basics.c expl_setup

We print the initial values to check the difference between the initial values
and the optimized ones.

\snippet example1_basics.c expl_print_initial

And finally, we can launch optimization using this simple code. This function
will return when the optimization will be finished, or when the convergence will
not be possible in reasonable time. It will not be the case in our simple
example.

\snippet example1_basics.c expl_solve

It's now the time to get the results. We do this simple code to that purpose.
This will call the `set` callback on the constrained parameter space values
provided as the second parameter. The third parameter is for extra results we
don't mind in this example.

\snippet example1_basics.c expl_get_results

We then print the optimized values to compare them to the initial ones.

\snippet example1_basics.c expl_print_optimized

### Conclusion

When we go through the full code, at first, it can be seen as a lot of code to
do something simple. In fact, when we look more carefully, all this code is
needed because of the explicit concepts that must be managed by the user.
High-level contexts of use of these libraries may hide some of these steps. If
you look at each section, you will not find lots of lines of code in each, that
means that each concept is quite simple. But we have many concepts.

In any case, most of the code written here will not change with time. The
"smartness" of the optimization is embedded in the computation functions of the
cost functors and in the structure of the relationships between parametric
points. This way, changing how the cost are computed or what are the
relationships will be enough to have very different behaviors. And this is a
small subset of the lines of code.

## Remarks on this code

* The goal of this example was not to provide a well structured user internal
  format of the data. Using a raw buffer for the values without any structure is
  obviously not the good way to go if you have complex situations.
* We provide all the destroy/release/remove functions to call for each concept
  in order to make the complete description. But keep in mind that the final
  call that do the device destruction will be enough to destroy everything.
  Other functions are useful when you manage dynamically the objects.

## Troubleshooting

### The device creation fail. How is it possible?

On linux, do not forget to use the `LD_LIBRARY_PATH` environment variable to add
the path to the dynamic libraries of the drivers:

```
cd build/examples
LD_LIBRARY_PATH=../libs ./example1_basics
```

## Full example code

When we put everthing at the right place, we get the full code below.

\includelineno example1_basics.c

