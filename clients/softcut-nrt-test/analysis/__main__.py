from plots import *

# start point in samples
M = 446758
N = M + 1000
#N = M + 200

keys = [
    'output',
    'fade0', 'rec0', 'pre0', 'wrIdx0', 'state0',
    'fade1', 'rec1', 'pre1', 'wrIdx1', 'state1',
]

plot_keys(keys, M, N)
