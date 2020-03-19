from plots import *

# start point in samples
#M = 446758
#N = M + 1000
#N = M + 200
M = 1000
N = M + 50



keys = [
    'output', 'frameInBlock',
    'phase0', 'phase1',
    'wrIdx0', 'wrIdx1',
    ['pre0', 'pre1'],
    ['rec0', 'rec1'],
    ['fade0', 'fade1'],
    ['state0', 'state1']
#    'fade0', 'rec0', 'pre0', 'wrIdx0', 'state0',
#    'fade1', 'rec1', 'pre1', 'wrIdx1', 'state1',
]

data = plot_keys(keys, M, N)
