#!/usr/bin/env python

import setuptools
from distutils.core import setup, Extension

VLIB='/usr/local/varnish/lib'

module1 = Extension('vsl', sources = ['vsl.c'], 
        include_dirs = ['/usr/local/varnish/include'],
        libraries = ['varnishapi'],
        library_dirs = [VLIB],
        runtime_library_dirs = [VLIB])

setup (name = 'vsl',
       version = '0.7.3',
       author = "lgx",
       author_email = "lgx@live.com",
       description = 'Varnish VSL interface',
       ext_modules = [module1])



