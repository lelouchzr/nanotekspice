# Component engine contract

`IComponent` is the graph boundary. The circuit stores and calls only `IComponent`
references/pointers. Factory implementations inherit `Component`, which owns connection
management, cache invalidation and evaluation guards. Components never look up circuit
names or downcast other chips.

Pins are one-based. Constructors call `Component(type, pinCount)` and `definePin()` for
all meaningful pins. Power and unused pins compute U and cannot be linked. Pins can
have any number of bidirectional links; connecting existing nets merges the entire net.
Inputs and sinks never drive a net. Matching known drivers resolve to that value;
conflicting or unknown drivers resolve to U. High impedance is ignored during net
resolution and displays as U when no active driver remains.

`read(pin)` resolves the net excluding this same endpoint, which allows a RAM data pin
to sample an external writer. `evaluate(pin)` returns this component's own `Drive` only;
it must not resolve its output net. `compute(pin)` resolves the visible pin value,
including its own driver. Cyclic evaluations return U. Acyclic recursion beyond the
safety depth raises `ComponentError`, rather than silently producing a wrong value.

Every tick has four global passes:

1. `beginTick(t)` invalidates cached values and commits pending source assignments or
   clock inversion. `onBeginTick` must not read links.
2. `prepare(t)` / `onPrepare(t)` captures all sequential inputs in pending state, without
   changing externally visible registers.
3. `simulate(t)` / `onSimulate(t)` commits pending registers. These hooks must not read
   links: all sampling already occurred in the second pass.
4. `endTick(t)` / `onEndTick(t)` completes cleanup and invalidates caches after every
   component has committed.

The circuit must complete each pass before starting the next. Repeated simulation at
the same tick is idempotent. Standalone `simulate(t)` supplies the missing phases for
that component only; it does not advance linked components automatically.

After source assignments and after the commit/cleanup passes, the circuit settles
registered asynchronous state to a fixed point. `hasAsynchronousState()` opts a
component in; `settle()` reports actual state changes and invalidates its caches.
It must not advance clocks or the circuit tick. The 4040 uses this phase for reset
feedback, including the supplied modulo-16 counter. A 1,024-pass convergence limit
rejects unstable asynchronous networks. Circuits without these components skip the
settling work.

`setInput` only records a pending assignment. `currentInput` reports committed state.
A clock assigned 0 remains U before tick 1, becomes 0 at tick 1, then becomes 1 at tick 2.

Under `NTS_BONUS`, visualization and program loading remain virtual interface calls;
the generic graph never requires concrete display or CPU component references.
