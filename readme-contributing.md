## repository structure

### current branches:

- `main`: currently active development branch. pretty much every PR should be opened against `main` (with possible exception of hotfixes for norns.) may have bugs. 

- `norns-latest`: this will attempt to track the commit pointed at by the `softcut` submodule in the main branch of `monome/norns` repo.

- `v1.5`: current version branch. `main` should be merged into here whenever it's deemed stable enough for testing.

- `v2-wip`: this branch contains "backlogged" changes intended for a v2 overhaul. involves extensive and complicated changes to the processing architecture. it's not yet working, hence the backlog, but it reopresents enough work to be worth saving. will be "salvaging" changes from here to release as v1.5.

###  working branches:

other working branches branches may exist, and should be named using a prefix and a slash. one of:

`fix/` - for bugfixes and performance improvements. 
`feature/`- for new user-facing features
`dev/` for primarily developer-facing work, like refactoring or architecture changes.

if you are part of the `monome` organization, go ahead and make your development branch in the main repo.
if you're not, make your working branch on your own fork.
