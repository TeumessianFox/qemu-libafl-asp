project('libvduse', 'c',
        license: 'GPL-2.0-or-later',
        default_options: ['c_std=gnu99'])

keyval = import('keyval')
config_host = keyval.load(meson.global_build_root() / 'config-host.mak')

libvduse = static_library('vduse',
                          files('libvduse.c'),
                          c_args: '-D_GNU_SOURCE',
                          pic: 'AS_SHARED_LIB' in config_host)

libvduse_dep = declare_dependency(link_with: libvduse,
                                  include_directories: include_directories('.'))
