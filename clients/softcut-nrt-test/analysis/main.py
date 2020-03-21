from plots import *
#
# keys = [
#     'output', 'frameInBlock', 'rate',
#     'wrIdx0',
#     # 'wrIdx1',
#     # 'phase0', 'phase1',
#     # ['pre0', 'pre1'],
#     # ['rec0', 'rec1'],
#     # ['fade0', 'fade1'],
#     # ['state0', 'state1'],
#     # ['action0', 'action1']
# ]
#
# M = 40000 + 400
# N = M + 200
# data = plot_keys(keys, M, N, "rising-rate-glitch-1.png")
#
#
# M = 40000
# N = M + 1000
# data = plot_keys(keys, M, N, "plots.png")


#
# M = 17065
# N = M + 72
# data = plot_keys(keys, M, N, "loop_1.png")
# M = 34160
# N = M + 72
# data = plot_keys(keys, M, N, "loop_2.png")
# M = 51278
# N = M + 72
# data = plot_keys(keys, M, N, "loop_3.png")


keys = [
    'output',
    'frameInBlock',
    'rate',
    'phase0', 'phase1',
    'wrIdx0',
    'wrIdx1',
    ['fade0', 'fade1']
]

M = 20000
N = 25000
data = plot_keys(keys, M, N, "plots.png")
