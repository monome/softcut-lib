to build and use `softcut_jack_osc` on macOS, JACK must be installed and running. also some packages dependencies need to be installed via macports or homebrew:

```
sudo port install jack
sudo port install libsndfile1-dev
sudo port install liblo
```

to use the software, `jackd` must be running:

```
jackd -d coreaudio 
```

and the selected audio device must have at least 2 channels of I/O. (i find it easiest to use an aggregate device on mac laptops.)