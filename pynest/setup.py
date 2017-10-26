# -*- coding: utf-8 -*-
#
# setup.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.
from __future__ import absolute_import, print_function

import os
from subprocess import PIPE, Popen
import sys

from setuptools import Extension, find_packages, setup
from setuptools.command.install import install

# get NEST installation specific stuff
def query_nest_config(kind):
    try:
        session = Popen(['nest-config', '--'+kind], stdout=PIPE, stderr=PIPE, universal_newlines=True)
        stdout, stderr = session.communicate()
        if session.returncode != 0 or stderr:
            raise ValueError('Some error occured querying nest-config --{} :\n'
                             '  stdout={}\n'
                             '  stderr={}'.format(kind, stdout, stderr))

        return stdout.strip()
    except FileNotFoundError:
        print('\n!!! Make sure nest is installed and `nest-config` is in the PATH !!!\n', file=sys.stderr)
        raise

compiler = query_nest_config('compiler')
version = query_nest_config('version')
include_dirs = query_nest_config('includes').replace('-I', '').split()
libdirs = query_nest_config('libdir').split()
libs = [l[2:] for l in query_nest_config('libs').split() if l.startswith('-l')]

# Define extension module
extra_link_args = []
if sys.platform == 'darwin':
    for l in libdirs:
        extra_link_args.append('-Wl,-rpath,{}'.format(l))

os.environ['CC'] = compiler
os.environ['CXX'] = compiler

# enable compilation of pre-cythonized pynestkernel, in case no
# cython is available on the system
do_cythonize = True
sources = ['nest/pynestkernel.pyx']
if '--no-cythonize' in sys.argv:
    sources = ['nest/pynestkernel.cpp']
    sys.argv.remove('--no-cythonize')
    do_cythonize = False

extensions = [Extension(
    name='nest.pynestkernel',
    sources=sources,
    include_dirs=include_dirs,
    library_dirs=libdirs,
    runtime_library_dirs=libdirs,
    libraries=libs,
    extra_link_args=extra_link_args,
    language='c++',
    define_macros = [('CYTHON_DEREF( x )', '( *x )'),
                     ('CYTHON_ADDR( x )', '( &x )')],
)]


if do_cythonize:
    try:
        from Cython.Build import cythonize
        extensions = cythonize(extensions)
    except ImportError:
        print('\n!!! Installing PyNEST requires Cython >= 0.19.2 !!!\n', file=sys.stderr)
        raise

# Convert markdown README into RST, s.t. it displays nice on pypi.
# Requires installation of pandoc (https://pandoc.org/) and
# python wrapper (https://pypi.python.org/pypi/pypandoc)
try:
    import pypandoc
    long_description = pypandoc.convert('README.md', 'rst')
except(IOError, ImportError):
    long_description = open('README.md').read()

setup(
    name='PyNEST',
    version=version,
    description='PyNEST provides Python bindings for NEST',
    long_description=long_description,
    author='The NEST Initiative',
    url='http://www.nest-simulator.org',
    license='GPLv2+',
    packages=find_packages(),
    ext_modules=extensions,
    keywords=['nest-simulator', 'nest', 'brain', 'simulator', 'neurons', 'synapses'],
    install_requires=[
        # 'cython', # build dependancy
        'matplotlib',
        'numpy',
        'scipy',
    ],
    include_package_data=True,
    classifiers=[
        # Trove classifiers
        # Full list: https://pypi.python.org/pypi?%3Aaction=list_classifiers
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: Implementation :: CPython'
    ],
)
