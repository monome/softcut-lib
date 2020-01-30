def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')

def build(bld):
    softcut_sources = [
        'softcut/src/FadeCurves.cpp',
        'softcut/src/ReadWriteHead.cpp',
        'softcut/src/SubHead.cpp',
        'softcut/src/Svf.cpp',
        'softcut/src/Voice.cpp',
    ]    
    
    bld.stlib(
        target = 'softcut',
        features = 'cxx cxxstlib',
        source = softcut_sources,
        includes = ['softcut/include'],
        cflags = ['-O3', '-Wall', '-Wextra'],
        cxxflags = ['--std=c++14']
    )
