
# Class-Parallel Tsetlin Machine

Feedback decision tree visualised:  
[tsetlin_decision_tree-combined.pdf](https://github.com/ashurrafiev/TsetlinTutorial/tree/master/tsetlin_decision_tree-combined.pdf)

## Usage

```
./tm [options]
```

| option | type | description |
| :--- | :--- | :--- |
| <span style="white-space: nowrap;">**-step-size**</span> | `int` | number of inputs per training step |
| **-steps** | `int` | total number of training steps |
| **-s** | `float` | learning rate _s_ |
| <span style="white-space: nowrap;">**-boost-pos**</span> | `0` or `1` | boost positive feedback; default is `0` |
| **-t** | `float` | threshold _T_ |
| **-ts** | `float,float,...` | comma-separated threshold list; this way you can set different _T_ values for each class |
| **-tnorm** | `0` or `1` | _T_ values are normalized; default is `1` |
| <span style="white-space: nowrap;">**-rand-seed**</span> | `int` | use specific random seed or timer if `0` (default) |
| <span style="white-space: nowrap;">**-acc-eval-train**</span> | `int` | _currently disabled, dooes nothing_ |
| <span style="white-space: nowrap;">**-acc-eval-test**</span> | `int` | use test dataset to evaluate accuracy: `0` - don't evaluate, `-1` evaluate once at the end of training, `n` - evaluate every `n` steps |
| <span style="white-space: nowrap;">**-log-tastates**</span> | | if specified, enable logging of the TA state spectrum |
| <span style="white-space: nowrap;">**-log-status**</span> | | if specified, enable logging of the TM status variables and events |
| <span style="white-space: nowrap;">**-log-acc**</span> | | if specified, enable logging of the accuracy evaluation results |
| <span style="white-space: nowrap;">**-log-append**</span> | | if specified, append to the existing log files; default is rewrite |
| <span style="white-space: nowrap;">**-load-state**</span> | `string` | if specified, load previously saved state of the TM; the string value is path format, where `%d` is replaced by a class index |
| <span style="white-space: nowrap;">**-save-state**</span> | `string` | if specified, the state of TM is saved after training; the string value is path format, where `%d` is replaced by a class index |
| <span style="white-space: nowrap;">**-train-mask**</span> | `string` | binary mask to enable training per class; default is `11111...`, i.e., every class is training |
| **-par** | `0` or `1` | enable parallel execution; default is `1` |

Example:

```
./tm -step-size 12000 -steps 5 -acc-eval-test 1 -log-acc
```

Train MNIST for 5 epochs, evaluate accuracy after each epoch (step) and log it.

## TsetlinOptions.h

Some parameters are hard-coded in `TsetlinOptions.h` and require recompilation when changed.

| option | type | value | description |
| :--- | :--- | :--- | :--- |
| `FEATURES` | `int` | (28*28) | number of input features; defaulted to MNIST |
| `CLASSES` | `int` | 10 | number of classes; defaulted to 10 for MNIST |
| `CLAUSES` | `int` | 200 | number of clauses per class |
| `NUM_STATES` | `int` | 100 | number of TA states per decision; exclude states are `(-NUM_STATES+1) .. 0`, include states are `1 .. NUM_STATES` |
| `LIT_LIMIT` | <span style="white-space: nowrap;">`0` or `1`</span> | `0` | toggle **literal-limiting** feedback algorithm |
| `INPUT_DATA_PATH` | `string` | <span style="white-space: nowrap;">"../../poets/tsetlin/pkbits"</span> | path to input data directory |
| `TRAIN_DATA_FMT` | `string` | <span style="white-space: nowrap;">"/mnist-train-cls%d.bin"</span> | format of the train data input file (per-class, `%d` is replaced with the class index); the file is in [pkbits](https://github.com/ashurrafiev/AuxTsetlinTools#packed-bits-format) format |
| `TEST_DATA` | `string` | <span style="white-space: nowrap;">"/mnist-test.bin"</span> | test data file name; the file is in [pkbits](https://github.com/ashurrafiev/AuxTsetlinTools#packed-bits-format) format |

## Build instructions

### Using precompiled logger files

To build:
```
make quick
```

To clean:
```
make clean
```

### Recompile logger files

* Requires Java (JDK) and [Logger generator](https://github.com/ashurrafiev/AuxTsetlinTools)
* [Logger XML specification](https://github.com/ashurrafiev/AuxTsetlinTools/loggerxml.md)

To build:
```
make all
```

To clean:
```
make cleanall
```

## Plotting diagrams

... (**TO DO**)

