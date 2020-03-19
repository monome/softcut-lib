from plots import *

# start point in samples
M = 17065
N = M + 72


keys = [
    'output', 'frameInBlock',
    'phase0', 'phase1',
    'wrIdx0', 'wrIdx1',
    ['pre0', 'pre1'],
    ['rec0', 'rec1'],
    ['fade0', 'fade1'],
    ['state0', 'state1'],
    ['action0', 'action1']
#    'fade0', 'rec0', 'pre0', 'wrIdx0', 'state0',
#    'fade1', 'rec1', 'pre1', 'wrIdx1', 'state1',
]

data = plot_keys(keys, M, N)
