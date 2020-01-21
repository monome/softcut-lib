idea for new voice computation structure,

should allow "duck" and "follow" featurees without compromising performance.
implementation complexity is reasonable;
the cost is some extra RAM usage (perhaps several KB per voice.)

1. allocate a number of buffers at least as big as the largest expected blocksize. each buffer will contain the value of a parameter for a whole block:
- active subhead index
- each subhead state:
  - read phase
  - write phase
  - pre+fade level
  - rec+fade level

2. each process block can be decomposed without looping over all voices on each sample. 

## normal mode:

same as present operation, with some potential speed gain due to memory locality:

- update positions
- update fade levels
- peek+poke, mix

## "follow" mode:

here, one voice is always hard-synced to another, but with different buffer data and mix parameters. this allows stereo operation among other things.

- copy positions
- copy fade levels
- peek+poke, mix

## duck mode:

one voice uses another's write index as a reference; output level is attenuated when this voice's read position crosses that voice's record position.

- update positions
- update fade levels, comparing with reference position
- peek+poke, mix


### details

- can use `std::array` and `std::copy` for state buffers
- 
