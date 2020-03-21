from plots import *

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
#M = 0
N = 25000
data = plot_keys(keys, M, N, "plots.png")
