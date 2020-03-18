from plots import *

# start pointin samples
M = 436603 + 2400 + 50
# end point in samples
N = M + 64

print([M, N])

# everything:
# keys = [
#     'buffer', 'output', 'frameInBlock',
#     'active', 'rate',
#     ['state0', 'state1'],
#     ['action0', 'action1'],
#     ['phase0', 'phase1'],
#     ['wrIdx0', 'wrIdx1'],
#     ['pre0', 'pre1'],
#     ['rec0', 'rec1'],
#     ['fade0', 'fade1'],
#     ['dir0', 'dir1'],
# ]

keys = [
    'output',
    'frameInBlock',
    #['wrIdx0', 'wrIdx1'],
    'wrIdx0',
    'wrIdx1',
    ['state0', 'state1'],
    'fade0', 'fade1',
    ['rec0', 'rec1'],
    ['pre0', 'pre1'],
    'phase0', 'phase1'
]

plot_keys(keys, M, N)
