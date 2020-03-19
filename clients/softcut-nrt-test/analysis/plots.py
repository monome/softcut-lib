import numpy as np
import matplotlib.pyplot as plt

def load_file(key):
    return np.load('../output/{}.npy'.format(key))

def plot_keys(keys, M, N):
    nkeys = len(keys)
    fix, ax = plt.subplots(nkeys, 1, figsize=(16, nkeys * 4))

    data = {}
    for i in range(nkeys):
        k = keys[i]
        if isinstance(k, list):
            for j in range(len(k)):
                a = (load_file(k[j]))[0]
                ax[i].set_title(k)
                ax[i].grid(True)
                ax[i].plot(a[M:N], marker='.', linewidth=1, markersize=2)
        else:
            arr = (load_file(k))[0]
            min = np.amin(arr)
            max = np.amax(arr)
            ax[i].grid(True)
            ax[i].set_title('{} [{}, {}]'.format(k, min, max))
            ax[i].plot(arr[M:N], marker='.', linewidth=1, markersize=2)
            data[k] = arr
    plt.savefig('../output/charts.png')
    # plt.show()

    return data
