import numpy as np
import matplotlib.pyplot as plt

keys = [
    'buffer', 'output', 'active',
    # 'rate',
    'state0', 'state1',
    'action0', 'action1',
    'phase0', 'phase1',
    'fade0', 'fade1',
    # 'rec0', 'rec1',
    # 'pre0', 'pre1',
    'wrIdx0', 'wrIdx1',
    'dir0', 'dir1',
]

nkeys = len(keys)


def load_file(key):
    return np.load('../output/{}.npy'.format(key))


fix, ax = plt.subplots(nkeys, 1, figsize=(16, nkeys * 8))

for i in range(nkeys):
    k = keys[i]
    arr = (load_file(k))[0]
    # print(len(arr))
    # print(arr)
    min = np.amin(arr)
    max = np.amax(arr)
    ax[i].grid(True)
    ax[i].set_title('{} [{}, {}]'.format(k, min, max))
    ax[i].plot(arr)
plt.savefig('../output/charts.pdf')
plt.show()

print("done")
