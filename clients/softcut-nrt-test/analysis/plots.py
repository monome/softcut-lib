import numpy as np
import matplotlib.pyplot as plt

data_location = '../../../cmake-build-debug/clients/softcut-nrt-test/'
def load_file(key):
    #return np.load('../output/{}.npy'.format(key))
    return np.load('{}/{}.npy'.format(data_location, key))

def plot_keys(keys, M, N, output_name="output.png"):
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
    plt.savefig('{}/{}'.format(data_location, output_name))
    #plt.show()

    return data
